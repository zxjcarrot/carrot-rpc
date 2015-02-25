/*
* Copyright (C) Xinjing Cho
*/
#include "carrot_simple_server.h"

namespace carrot {

static int expand_connection_packet(struct rpc_connection * c, uint32_t size) {
    assert(c != NULL);

    if (c->size < size || c->p == NULL) {
        if ((c->p = (carrot::packet *)::realloc(c->p, size)) == NULL) {
            return -1;
        }
        c->size = size;
    }

    return 0;
}

void CarrotSimpleServer::destroy_rpc_connection(struct rpc_connection * c){
    assert(c != NULL);

    reactor_remove_event(&this->r, &c->e);

    if (c->alive) {
        TEMP_FAILURE_RETRY(::close(c->client_fd));
        c->alive = false;
    }
    /* reserve c->p for resue */

    this->conns_unused.push_back(c);
    this->conns_in_use.erase((uint64_t)c);
}

void CarrotSimpleServer::respond_to_client(struct rpc_connection * c,
                                    ::google::protobuf::Message * resp,
                                    rpc::Response * rpc_resp) {
    size_t rpc_len = 0;
    size_t resp_len = 0;
    size_t n;

    if (rpc_resp)
        rpc_len += rpc_resp->ByteSize();

    if (resp)
        resp_len = resp->ByteSize();

    n = sizeof(carrot::packet) + rpc_len + resp_len;

    CARROT_LOG("send packet back to client:\n%s", resp->DebugString().c_str());
    if (expand_connection_packet(c, n) == -1) {
        CARROT_LOG("failed to allocate memory for outgoing packet: %s", ::strerror(errno));
        destroy_rpc_connection(c);
        return;
    }

    c->p->rpc_len = htonl(rpc_len);
    c->p->req_len = htonl(0);
    c->p->resp_len = htonl(resp_len);

    if (rpc_resp && !rpc_resp->SerializeToArray(c->p->data, rpc_len)) {
        CARROT_LOG("failed to serialize response(metadata)");
        destroy_rpc_connection(c);
        return;
    }

    if (resp && !resp->SerializeToArray(c->p->data + rpc_len, resp_len)) {
        CARROT_LOG("failed to serialize response");
        destroy_rpc_connection(c);
        return;
    }

    if (carrot::write_n(c->client_fd, (char *)c->p, n) == -1) {
        CARROT_LOG("failed to send response back to client");
        destroy_rpc_connection(c);
        return;
    }
}
void CarrotSimpleServer::handle_client(struct rpc_connection * c) {
    uint32_t rpc_len;
    uint32_t req_len;
    uint32_t resp_len;
    uint32_t n;

    if (carrot::read_n(c->client_fd, (char *)&rpc_len, sizeof(uint32_t)) == -1) {
        CARROT_LOG("failed to read request metadata size from client: %s", ::strerror(errno));
        destroy_rpc_connection(c);
        return;
    }

    if (carrot::read_n(c->client_fd, (char *)&req_len, sizeof(uint32_t)) == -1) {
        CARROT_LOG("failed to read request parameter size from client: %s", ::strerror(errno));
        destroy_rpc_connection(c);
        return;
    }

    if (carrot::read_n(c->client_fd, (char *)&resp_len, sizeof(uint32_t)) == -1) {
        CARROT_LOG("failed to read response parameter size from client: %s", ::strerror(errno));
        destroy_rpc_connection(c);
        return;
    }

    rpc_len  =  ntohl(rpc_len);
    req_len  =  ntohl(req_len);
    resp_len =  ntohl(resp_len);

    n = sizeof(carrot::packet) + rpc_len + req_len + resp_len;

    CARROT_LOG("packet from client: rpc_len: %d, req_len: %d, resp_len: %d", rpc_len, req_len, resp_len);
    if (expand_connection_packet(c, n) == -1) {
        CARROT_LOG("failed to allocate memory for incoming packet: %s", ::strerror(errno));
        destroy_rpc_connection(c);
        return;
    }

    c->p->rpc_len = rpc_len;
    c->p->req_len = req_len;
    c->p->resp_len = resp_len;

    if (carrot::read_n(c->client_fd, c->p->data, n - sizeof(carrot::packet)) == -1) {
        CARROT_LOG("failed to read incoming packet from client:%s", ::strerror(errno));
        destroy_rpc_connection(c);
        return;
    }

    ::carrot::rpc::Request rpc_req;
    ::carrot::rpc::Response rpc_resp;

    /*As callee, ignores reponse parameter delivered by client.*/
    if (!rpc_req.ParseFromArray(c->p->data, rpc_len)) {
        CARROT_LOG("failed to read request parameter from client");
        destroy_rpc_connection(c);
        return;
    }

    std::string method_identity = rpc_req.method_identity();

    size_t pos = method_identity.find_first_of('.');

    CarrotController controller;

    if (pos == std::string::npos) {
        rpc_resp.set_status(::carrot::rpc::NOT_FOUND);
        rpc_resp.set_message("invalid method method identity");
        respond_to_client(c, NULL, &rpc_resp);
        return;
    }

    std::string service_name = std::string(method_identity.begin(),
                                           method_identity.begin() + pos);
    std::string method_name = std::string(method_identity.begin() + pos + 1, // 1 for '.'
                                          method_identity.end());
    std::map<std::string, struct carrot_service>::iterator it 
        = services.find(service_name);

    upm request;
    upm response;

    if (it == services.end()) {
        rpc_resp.set_status(::carrot::rpc::NOT_FOUND);
        rpc_resp.set_message("service not found");
        respond_to_client(c, NULL, &rpc_resp);
        return;
    }
    

    struct carrot_service &       cs = it->second;
    ::google::protobuf::Service * s = cs.s;

    const ::google::protobuf::MethodDescriptor * md = s->GetDescriptor()->FindMethodByName(method_name);

    if (md == NULL) {
        rpc_resp.set_status(::carrot::rpc::NOT_FOUND);
        rpc_resp.set_message("method not found");
        respond_to_client(c, NULL, &rpc_resp);
        return;
    }

    CARROT_LOG("client invoking method %s", (service_name + "." + method_name).c_str());

    request = upm(s->GetRequestPrototype(md).New());
    response = upm(s->GetResponsePrototype(md).New());

    if (!request->ParseFromArray(c->p->data + rpc_len, req_len)) {
        CARROT_LOG("failed to read request parameter from client");
        destroy_rpc_connection(c);
        return;
    }

    s->CallMethod(md, &controller, request.get(), response.get(), cs.c);

    if (controller.Failed()) {
        rpc_resp.set_status(rpc::FAILED);
        rpc_resp.set_message("server side failure");
    } else {
        rpc_resp.set_status(rpc::SUCCESS);
    }

    respond_to_client(c, response.get(), &rpc_resp);
}

static void client_callback(el_socket_t fd, short res_flags, void *arg) {
    if (res_flags != E_READ)
        return;
    struct rpc_connection * c = static_cast<struct rpc_connection *>(arg);
    c->handle_client_func(c);
}

static void listen_callback(el_socket_t fd, short res_flags, void *arg) {
    if (res_flags != E_READ)
        return;
    std::function<void(void)> * f = (std::function<void(void)> *)arg;
    (*f)();
}

void CarrotSimpleServer::handle_listen_sock() {
    struct sockaddr addr;
    socklen_t       len = sizeof(struct sockaddr);

    int client_sock = ::accept(listen_sock, &addr, &len);
    
    if (client_sock == -1) {
        CARROT_LOG("failed to accept a client socket: %s", ::strerror(errno));
        return;
    }

    CARROT_LOG("got a connection from %s", inet_ntoa(((struct sockaddr_in*)&addr)->sin_addr));

    struct rpc_connection * conn = NULL;

    if (!this->conns_unused.empty()) {
        conn = this->conns_unused.back();
        this->conns_unused.pop_back();
    } else {
        conn = new rpc_connection();
    }

    if (conn == NULL) {
        CARROT_LOG("failed to allocate memory for rpc_connection: %s", ::strerror(errno));
        return;
    }


    conn->client_fd = client_sock;
    conn->alive = true;
    conn->handle_client_func = std::bind(&CarrotSimpleServer::handle_client, this, std::placeholders::_1);
    
    event_set(&conn->e, client_sock, E_READ, client_callback, conn);

    if (reactor_add_event(&this->r, &conn->e)) {
        CARROT_LOG("failed to add client event to reactor");
        delete conn;
        return;
    }

    this->conns_in_use.insert(std::make_pair((uint64_t)conn, conn));
}

CarrotSimpleServer::CarrotSimpleServer(short port, int protocol):
    CarrotServer(port, protocol) {
    accept_handler = std::bind(&CarrotSimpleServer::handle_listen_sock, this);

    reactor_init_with_mt_timer(&this->r, NULL);

}

CarrotSimpleServer::~CarrotSimpleServer() {
    reactor_destroy(&this->r);

    for(auto c : conns_unused) {
        if (c->alive) {
            TEMP_FAILURE_RETRY(::close(c->client_fd));
            c->alive = false;
        }

        if (c->p && c->size)
            ::free(c->p);

        c->p = NULL;
        c->size = 0;

        delete c;
    }

    for (auto it : conns_in_use) {
        struct rpc_connection * c = it.second;

        if (c->alive) {
            TEMP_FAILURE_RETRY(::close(c->client_fd));
            c->alive = false;
        }

        if (c->p && c->size)
            ::free(c->p);

        c->p = NULL;
        c->size = 0;

        delete c;
    }
}


void CarrotSimpleServer::Run() {
    event_set(&this->listen_event, listen_sock, E_READ, listen_callback, &accept_handler);

    if (reactor_add_event(&this->r, &this->listen_event) != 0)
        throw std::runtime_error("failed to add event to the reactor");

    struct timeval timeout = {1, 0};
    
    reactor_loop(&this->r, &timeout, 0);

    reactor_clean_events(&this->r);
}

}