/*
* Copyright (C) Xinjing Cho
*/
#include "carrot_async_server.h"

namespace carrot {

void RpcServerConnection::destroy()
{
    if (server_)
        server_->conns().erase((uint64_t)this);
    if (socket_.is_open()) {
        boost::system::error_code error;
        socket_.shutdown(tcp::socket::shutdown_receive, error);
        if (error)
            CARROT_LOG("carrot-rpc destroy shutdown error: %s", error.message().c_str());
        socket_.close(error);
        if (error)
            CARROT_LOG("carrot-rpc destroy close error: %s", error.message().c_str());
    } else {
        CARROT_LOG("socket already closed");
    }
}

void RpcServerConnection::serve()
{
    if (expand_packet(&p_, &size_, sizeof(packet)) == -1) {
        throw std::bad_alloc();
    }

    /* read header */
    boost::asio::async_read(socket_, boost::asio::buffer((char*)p_, sizeof(packet)),
        boost::bind(&RpcServerConnection::handle_read_header, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void RpcServerConnection::handle_read_header(const boost::system::error_code& error,
        std::size_t bytes_transferred)
{
    if (!error) {
        CONVERT_TO_HOST_ORDER(*p_);
        CARROT_LOG("packet from client: rpc_len: %d, req_len: %d, resp_len: %d", p_->rpc_len, p_->req_len, p_->resp_len);
    
        n_ = sizeof(packet) + p_->rpc_len + p_->req_len + p_->resp_len;

        if (expand_packet(&p_, &size_, n_) == -1) {
            throw std::bad_alloc();
        }

        boost::asio::async_read(socket_, boost::asio::buffer(p_->data, n_ - sizeof(packet)),
            boost::bind(&RpcServerConnection::handle_read_payload, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    } else {
        destroy();
        throw boost::system::system_error(error);
    }
}

void RpcServerConnection::handle_read_payload(const boost::system::error_code& error,
        std::size_t bytes_transferred)
{
    if (!error) {
        ::carrot::rpc::Request rpc_req;
        ::carrot::rpc::Response rpc_resp;

        /*As callee, ignores reponse parameter delivered by client.*/
        if (!rpc_req.ParseFromArray(p_->data, p_->rpc_len)) {
            destroy();
            throw std::runtime_error(std::string("failed to read incoming packet from client: ") + ::strerror(errno));
        }

        CARROT_LOG("request metadata: %s", rpc_req.DebugString().c_str());
        std::string method_identity = rpc_req.method_identity();

        size_t pos = method_identity.find_first_of('.');

        CarrotController controller;

        if (pos == std::string::npos) {
            rpc_resp.set_id(rpc_req.id());
            rpc_resp.set_status(::carrot::rpc::NOT_FOUND);
            rpc_resp.set_message("invalid method method identity");
            respond_to_client(NULL, &rpc_resp);
            return;
        }

        std::string service_name = std::string(method_identity.begin(),
                                               method_identity.begin() + pos);
        std::string method_name = std::string(method_identity.begin() + pos + 1, // 1 for '.'
                                              method_identity.end());
        std::map<std::string, struct carrot_service>::iterator it 
            = server_->services().find(service_name);

        upm request;
        upm response;

        if (it == server_->services().end()) {
            rpc_resp.set_id(rpc_req.id());
            rpc_resp.set_status(::carrot::rpc::NOT_FOUND);
            rpc_resp.set_message("service not found");
            respond_to_client(NULL, &rpc_resp);
            return;
        }
        
        struct carrot_service &       cs = it->second;
        ::google::protobuf::Service * s = cs.s;

        const ::google::protobuf::MethodDescriptor * md = s->GetDescriptor()->FindMethodByName(method_name);

        if (md == NULL) {
            rpc_resp.set_id(rpc_req.id());
            rpc_resp.set_status(::carrot::rpc::NOT_FOUND);
            rpc_resp.set_message("method not found");
            respond_to_client(NULL, &rpc_resp);
            return;
        }

        CARROT_LOG("client invoking method %s, id: %ld", (service_name + "." + method_name).c_str(), rpc_req.id());

        request = upm(s->GetRequestPrototype(md).New());
        response = upm(s->GetResponsePrototype(md).New());

        if (!request->ParseFromArray(p_->data + p_->rpc_len, p_->req_len)) {
            CARROT_LOG("failed to read request parameter from client");
            destroy();
            throw std::runtime_error(std::string("failed to read request parameter from client"));
        }

        try {
            s->CallMethod(md, &controller, request.get(), response.get(), cs.c);
        } catch(std::exception & e) {
            controller.SetFailed(std::string("Server Internal Error ") + e.what());
        }

        if (controller.Failed()) {
            rpc_resp.set_status(rpc::FAILED);
            rpc_resp.set_message(controller.ErrorText());
        } else {
            rpc_resp.set_status(rpc::SUCCESS);
        }
        rpc_resp.set_id(rpc_req.id());

        respond_to_client(response.get(), &rpc_resp);
    } else {
        destroy();
        throw boost::system::system_error(error);
    }
}

void RpcServerConnection::respond_to_client(::google::protobuf::Message * resp,
        rpc::Response * rpc_resp)
{
    size_t rpc_len = 0;
    size_t resp_len = 0;

    if (rpc_resp)
        rpc_len += rpc_resp->ByteSize();

    if (resp)
        resp_len = resp->ByteSize();

    size_t n = sizeof(carrot::packet) + rpc_len + resp_len;

    CARROT_LOG("send packet back to client:\n%s", resp->DebugString().c_str());
    if (expand_packet(&p_, &size_, n_) == -1) {
        throw std::bad_alloc();
    }

    p_->rpc_len = htonl(rpc_len);
    p_->req_len = htonl(0);
    p_->resp_len = htonl(resp_len);

    if (rpc_resp && !rpc_resp->SerializeToArray(p_->data, rpc_len)) {
        CARROT_LOG("failed to serialize response(metadata)");
        destroy();
        throw std::runtime_error(std::string("failed to serialize response(metadata)"));
    }

    if (resp && !resp->SerializeToArray(p_->data + rpc_len, resp_len)) {
        CARROT_LOG("failed to serialize response");
        destroy();
        throw std::runtime_error(std::string("failed to serialize response"));
    }

    boost::asio::async_write(socket_, boost::asio::buffer((char*)p_, n),
        boost::bind(&RpcServerConnection::handle_write_response, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
}

void RpcServerConnection::handle_write_response(const boost::system::error_code& error,
        std::size_t bytes_transferred) {
    if (!error) {
        /* start sering next call */
        serve();
    } else {
        destroy();
        throw boost::system::system_error(error);
    }
}

void CarrotAsyncServer::start_accept()
{
    RpcServerConnection::pointer pointer = RpcServerConnection::create(this, acceptor_.get_io_service());

    conns_.insert(std::make_pair((uint64_t)pointer.get(), pointer));
    
    acceptor_.async_accept(pointer->socket(), 
        boost::bind(&CarrotAsyncServer::handle_accept, this, pointer,
            boost::asio::placeholders::error));
}

void CarrotAsyncServer::handle_accept(RpcServerConnection::pointer c, const boost::system::error_code& error)
{
    if (!error) {
        c->serve();
    }
    start_accept();
}

/*
* Register a server on the server, this function should be called before starting the server.
*/
bool CarrotAsyncServer::RegisterService(::google::protobuf::Service * service,
                     ::google::protobuf::Closure * closure) {
    std::string key = service->GetDescriptor()->name();
    if (services_.find(key) != services_.end()) {
        return false;
    }

    CARROT_LOG("Registering service %s.", key.c_str());
    struct carrot_service cs;
    cs.s = service;
    cs.c = closure;
    services_.insert(std::make_pair(key, cs));

    return true;
}

}