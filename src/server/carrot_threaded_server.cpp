/*
* Copyright (C) Xinjing Cho
*/
#include <thread>

#include "carrot_threaded_server.h"

namespace carrot {
    static void destroy_connection(int client_fd) {
        TEMP_FAILURE_RETRY(::close(client_fd));
    }

    static void respond_to_client(packet ** p, uint32_t *psize, int client_fd,
                                  ::google::protobuf::Message * resp,
                                  rpc::Response * rpc_resp) {
        uint32_t rpc_len = 0;
        uint32_t resp_len = 0;
        uint32_t n;

        if (rpc_resp)
            rpc_len += rpc_resp->ByteSize();

        if (resp)
            resp_len = resp->ByteSize();

        n = sizeof(carrot::packet) + rpc_len + resp_len;

        CARROT_LOG("send packet back to client:\n%s", resp->DebugString().c_str());
        if (expand_packet(p, psize, n) == -1) {
            CARROT_LOG("failed to allocate memory for outgoing packet: %s", ::strerror(errno));
            destroy_connection(client_fd);
            return;
        }

        (*p)->rpc_len = htonl(rpc_len);
        (*p)->req_len = htonl(0);
        (*p)->resp_len = htonl(resp_len);

        if (rpc_resp && !rpc_resp->SerializeToArray((*p)->data, rpc_len)) {
            CARROT_LOG("failed to serialize response(metadata)");
            destroy_connection(client_fd);
            return;
        }

        if (resp && !resp->SerializeToArray((*p)->data + rpc_len, resp_len)) {
            CARROT_LOG("failed to serialize response");
            destroy_connection(client_fd);
            return;
        }

        if (carrot::write_n(client_fd, (char *)(*p), n) == -1) {
            CARROT_LOG("failed to send response back to client");
            destroy_connection(client_fd);
            return;
        }
    }

    void CarrotThreadedServer::handle_client(int client_fd) {
        packet * p = NULL;
        uint32_t size = 0;

        while (!this->stopped) {
            uint32_t rpc_len;
            uint32_t req_len;
            uint32_t resp_len;
            uint32_t n;

            if (carrot::read_n(client_fd, (char *)&rpc_len, sizeof(uint32_t)) == -1) {
                CARROT_LOG("failed to read request metadata size from client: %s", ::strerror(errno));
                destroy_connection(client_fd);
                break;
            }

            if (carrot::read_n(client_fd, (char *)&req_len, sizeof(uint32_t)) == -1) {
                CARROT_LOG("failed to read request parameter size from client: %s", ::strerror(errno));
                destroy_connection(client_fd);
                break;
            }

            if (carrot::read_n(client_fd, (char *)&resp_len, sizeof(uint32_t)) == -1) {
                CARROT_LOG("failed to read response parameter size from client: %s", ::strerror(errno));
                destroy_connection(client_fd);
                break;
            }

            rpc_len  =  ntohl(rpc_len);
            req_len  =  ntohl(req_len);
            resp_len =  ntohl(resp_len);

            n = sizeof(carrot::packet) + rpc_len + req_len + resp_len;

            CARROT_LOG("packet from client: rpc_len: %d, req_len: %d, resp_len: %d", rpc_len, req_len, resp_len);
            if (expand_packet(&p, &size, n) == -1) {
                CARROT_LOG("failed to allocate memory for incoming packet: %s", ::strerror(errno));
                destroy_connection(client_fd);
                break;
            }

            p->rpc_len = rpc_len;
            p->req_len = req_len;
            p->resp_len = resp_len;

            if (carrot::read_n(client_fd, p->data, n - sizeof(carrot::packet)) == -1) {
                CARROT_LOG("failed to read incoming packet from client:%s", ::strerror(errno));
                destroy_connection(client_fd);
                break;
            }

            ::carrot::rpc::Request rpc_req;
            ::carrot::rpc::Response rpc_resp;

            /*As callee, ignores reponse parameter delivered by client.*/
            if (!rpc_req.ParseFromArray(p->data, rpc_len)) {
                CARROT_LOG("failed to read request parameter from client");
                destroy_connection(client_fd);
                continue;
            }

            std::string method_identity = rpc_req.method_identity();

            size_t pos = method_identity.find_first_of('.');

            CarrotController controller;

            if (pos == std::string::npos) {
                rpc_resp.set_status(::carrot::rpc::NOT_FOUND);
                rpc_resp.set_message("invalid method method identity");
                respond_to_client(&p, &size, client_fd, NULL, &rpc_resp);
                continue;
            }

            std::string service_name = std::string(method_identity.begin(),
                                                   method_identity.begin() + pos);
            std::string method_name = std::string(method_identity.begin() + pos + 1, // 1 for '.'
                                                  method_identity.end());
            std::map<std::string, struct carrot_service>::iterator it 
                = this->services.find(service_name);

            upm request;
            upm response;

            if (it == services.end()) {
                rpc_resp.set_status(::carrot::rpc::NOT_FOUND);
                rpc_resp.set_message("service not found");
                respond_to_client(&p, &size,  client_fd, NULL, &rpc_resp);
                continue;
            }
            

            struct carrot_service &       cs = it->second;
            ::google::protobuf::Service * s = cs.s;

            const ::google::protobuf::MethodDescriptor * md = s->GetDescriptor()->FindMethodByName(method_name);

            if (md == NULL) {
                rpc_resp.set_status(::carrot::rpc::NOT_FOUND);
                rpc_resp.set_message("method not found");
                respond_to_client(&p, &size, client_fd,  NULL, &rpc_resp);
                continue;
            }

            CARROT_LOG("client invoking method %s", (service_name + "." + method_name).c_str());

            request = upm(s->GetRequestPrototype(md).New());
            response = upm(s->GetResponsePrototype(md).New());

            if (!request->ParseFromArray(p->data + rpc_len, req_len)) {
                CARROT_LOG("failed to read request parameter from client");
                destroy_connection(client_fd);
                continue;
            }

            s->CallMethod(md, &controller, request.get(), response.get(), cs.c);

            if (controller.Failed()) {
                rpc_resp.set_status(rpc::FAILED);
                rpc_resp.set_message("server side failure");
            } else {
                rpc_resp.set_status(rpc::SUCCESS);
            }

            respond_to_client(&p, &size,  client_fd, response.get(), &rpc_resp);
        }
        destroy_connection(client_fd);
        free(p);
    }

    void CarrotThreadedServer::Run() {
        this->stopped = false;
        
        while (!this->stopped) {
            struct sockaddr addr;
            socklen_t       len = sizeof(struct sockaddr);

            int client_fd = ::accept(this->listen_sock, &addr, &len);

            if (client_fd == -1) {
                CARROT_LOG("failed to accept a client socket: %s", ::strerror(errno));
                return;
            }

            CARROT_LOG("got a connection from %s", inet_ntoa(((struct sockaddr_in*)&addr)->sin_addr));

            std::thread(&CarrotThreadedServer::handle_client, this, client_fd).detach();
        }
    }

}