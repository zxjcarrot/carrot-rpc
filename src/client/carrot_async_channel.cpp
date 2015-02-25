/*
* Copyright (C) Xinjing Cho
*/
#include "carrot_async_channel.h"

namespace carrot {

static const char* state_strs[] = {
    "STATE_CONNECT_TO_SERVER",
    "STATE_PREPARE_TO_WRITE_TO_SERVER",
    "STATE_WRITE_TO_SERVER",
    "STATE_READ_METADATA",
    "STATE_READ_DATA",
    "STATE_TIMEOUT",
    "STATE_DONE",
    ""
};

RpcClientConnection::~RpcClientConnection() {
    if (p_ && size_) {
        ::free(p_);
        p_ = nullptr;
        size_ = 0;
    }
}

void RpcClientConnection::destroy(const boost::system::error_code reason) {
    CARROT_LOG("destroy got called : %s ", reason.message().c_str());
    boost::system::error_code error;
    
    resolver_.cancel();
    
    socket_.cancel(error);
    if (error)
        CARROT_LOG("destroy cancel events waiting on the socket with error: %s ", error.message().c_str());
    
    socket_.shutdown(tcp::socket::shutdown_both, error);
    if (error)
        CARROT_LOG("destroy shutdown socket with error: %s ", error.message().c_str());
    
    socket_.close(error);
    if (error)
        CARROT_LOG("destroy closed socket with error: %s ", error.message().c_str());
    
    chann_->conns().erase(host_ + ":" + service_);

    for (auto it : outgoing_tasks) {
        call_task_sptr task = it.second;
        task->timer.cancel(error);
        if (error)
            CARROT_LOG("destroy cancel timer with error: %s ", error.message().c_str());
        
        if (!task->called){
            task->controller->SetFailed(std::string("rpc failed: ") + reason.message());
            try {
                task->done->Run();
            } catch (std::exception &e) {
                task->controller->SetFailed(std::string("rpc failed: ") + e.what());
            }
            task->called = true;
        }
    }
    outgoing_tasks.clear();
}

void RpcClientConnection::setup_task_timer(call_task_sptr task)
{
    if (task->controller->timeout() != TIMEOUT_NEVER) {
        task->timer.cancel();
        task->timer.expires_from_now(boost::posix_time::milliseconds(task->controller->timeout()));
        task->timer.async_wait(boost::bind(&RpcClientConnection::handle_timeout, shared_from_this(), task, 
            boost::asio::placeholders::error));
    }
}

void RpcClientConnection::handle_timeout(call_task_sptr task, const boost::system::error_code& error)
{
    if (!error) {
        outgoing_tasks.erase(task->id);
        if (!task->called){
            task->controller->SetFailed("rpc timed out at state " + std::string(state_strs[task->state]), EC_RPC_TIMEOUT);
            try {
                task->done->Run();
            } catch (std::exception &e) {
                task->controller->SetFailed(std::string("rpc failed: ") + e.what());
            }
            task->called = true;
        }

        /* if failed on connectting to peer, destroy self */
        if (connecting_)
            destroy(boost::system::errc::make_error_code(boost::system::errc::timed_out));
    } else {
        CARROT_LOG("handle_timeout error: %s", error.message().c_str());
    }
}

void RpcClientConnection::issue(const ::google::protobuf::MethodDescriptor * method,
                    CarrotController * controller,
                    const ::google::protobuf::Message * request,
                    ::google::protobuf::Message * response,
                    ::google::protobuf::Closure * done)
{
    call_task_sptr task = call_task_sptr(new call_task(socket_.get_io_service(), ++id_));

    task->method = method;
    task->controller = controller;
    task->request = request;
    task->response = response;
    task->done = done;

    CARROT_LOG("issuing task %p method %p controller %p request %p response %p done %p", task.get(), method, controller, request, response, done);
    if (!socket_.is_open()) {
        /* first call, try to establish connection */
        CARROT_LOG("try to resolve %s:%s", task->controller->host().c_str(), task->controller->service().c_str());
        tcp::resolver::query q(task->controller->host(), task->controller->service());

        resolver_.async_resolve(q,
            boost::bind(&RpcClientConnection::handle_resolve, shared_from_this(), task,
                boost::asio::placeholders::error,
                boost::asio::placeholders::iterator));
        connecting_ = true;
        task->state = STATE_CONNECT_TO_SERVER;
        setup_task_timer(task);
    } else {
        start_call(task);
    }
}

void RpcClientConnection::handle_resolve(call_task_sptr task,
            const boost::system::error_code& error,
            tcp::resolver::iterator endpoint_iterator)
{
    if (!error) {
        CARROT_LOG("resolved, try to connect to %s:%d", 
            endpoint_iterator->endpoint().address().to_string().c_str(),
             endpoint_iterator->endpoint().port());
        boost::asio::async_connect(socket_, endpoint_iterator, 
            boost::bind(&RpcClientConnection::handle_connect, shared_from_this(), task,
                boost::asio::placeholders::error,
                boost::asio::placeholders::iterator));
    } else {
        CARROT_LOG("handle_resolve error: %s", error.message().c_str());
        if (!task->called){
            task->controller->SetFailed("failed to connect, " + error.message(), EC_ERROR);
            try {
                task->done->Run();
            } catch (std::exception &e) {
                task->controller->SetFailed(std::string("rpc failed: ") + e.what());
            }
            task->called = true;
        }
        destroy(error);
        throw boost::system::system_error(error);
    }
}

void RpcClientConnection::handle_connect(call_task_sptr task,
            const boost::system::error_code& error,
            tcp::resolver::iterator endpoint_iterator)
{
    if (!error) {
        CARROT_LOG("connected to %s:%d, calling...", 
            endpoint_iterator->endpoint().address().to_string().c_str(),
             endpoint_iterator->endpoint().port());
        connecting_ = false;
        start_call(task);
        start_read();
    } else {
        if (!task->called){
            task->controller->SetFailed("failed to connect, " + error.message(), EC_CONNECTION_FAILED);
            try {
            task->done->Run();
        } catch (std::exception &e) {
            task->controller->SetFailed(std::string("rpc failed: ") + e.what());
        }
            task->called = true;
        }
        CARROT_LOG("handle_resolve error: %s", error.message().c_str());
        destroy(error);
        throw boost::system::system_error(error);
    }

}

void RpcClientConnection::finish_task(call_task_sptr task)
{
    boost::system::error_code error;

    task->timer.cancel(error);
    CARROT_LOG("calling task %d callback %p", task->id, task->done);
    if (error)
        CARROT_LOG("finish_task cancelled timer with error: %s ", error.message().c_str());
    if (!task->called){
        try {
            task->done->Run();
        } catch (std::exception &e) {
            task->controller->SetFailed(std::string("rpc failed: ") + e.what());
        }
        task->called = true;
    }
}

void RpcClientConnection::start_call(call_task_sptr task)
{
    carrot::rpc::Request rpc_req;

    rpc_req.set_id(task->id);
    rpc_req.set_method_identity(task->method->service()->name() + "." + task->method->name());
    
    /* As caller, the size of Response(metadata) and response(parameter) are left out.*/
    uint32_t rpc_len = rpc_req.ByteSize();
    uint32_t req_len = task->request->ByteSize();
    uint32_t resp_len = 0;
    size_t  n = sizeof(carrot::packet) + rpc_len + req_len;

    if (expand_packet(&task->p, &task->size, n) == -1){
        task->controller->SetFailed("failed to allocate memory for rpc packet", EC_OOM);
        finish_task(task);
        return;
    }

    if (!rpc_req.SerializeToArray(task->p->data, rpc_len + req_len)) {
       task->controller->SetFailed("failed to serialize Request metadata into byte array", EC_SERIALIZATION_ERROR);
       finish_task(task);
       return;
    }

    if (!task->request->SerializeToArray(task->p->data + rpc_len, req_len)) {
        task->controller->SetFailed("failed to serialize request paramerter into byte array", EC_SERIALIZATION_ERROR);
        finish_task(task);
        return;
    }

    task->p->rpc_len = rpc_len;
    task->p->req_len = req_len;
    task->p->resp_len = resp_len;

    CONVERT_TO_NETWORK_ORDER(*task->p);

    outgoing_tasks.insert(std::make_pair(task->id, task));

    setup_task_timer(task);

    CARROT_LOG("calling %s, rpc_len: %d, req_len: %d, resp_len: %d, reuqest metadata: %s", rpc_req.method_identity().c_str(), rpc_len, req_len, resp_len, rpc_req.DebugString().c_str());
    task->state = STATE_WRITE_TO_SERVER;
    boost::asio::async_write(socket_, boost::asio::buffer((char*)task->p, n),
        boost::bind(&RpcClientConnection::handle_write, shared_from_this(), task,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

/* start reading next reponse */
void RpcClientConnection::start_read()
{
    if (expand_packet(&p_, &size_, sizeof(packet)) == -1){
        destroy(boost::system::errc::make_error_code(boost::system::errc::not_enough_memory));
        throw std::bad_alloc();
    }

    boost::asio::async_read(socket_, boost::asio::buffer(p_, sizeof(packet)),
        boost::bind(&RpcClientConnection::handle_read_header, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void RpcClientConnection::handle_read_header(const boost::system::error_code& error,
        std::size_t bytes_transferred)
{
    if (!error) {
        CONVERT_TO_HOST_ORDER(*p_);

        n_ = p_->rpc_len + p_->req_len+ p_->resp_len;

        if (expand_packet(&p_, &size_, n_ + sizeof(packet)) == -1){
            destroy(boost::system::errc::make_error_code(boost::system::errc::not_enough_memory));
            throw std::bad_alloc();
        }

        CARROT_LOG("got metadata from server: rpc_len: %d, req_len: %d, resp_len: %d.",
                    p_->rpc_len, p_->req_len, p_->resp_len);

        boost::asio::async_read(socket_, boost::asio::buffer(p_->data, n_),
            boost::bind(&RpcClientConnection::handle_read_payload, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    } else {
        CARROT_LOG("handle_read_header error: %s", error.message().c_str());
        destroy(error);
        throw boost::system::system_error(error);
    }
}

void RpcClientConnection::handle_read_payload(const boost::system::error_code& error,
        std::size_t bytes_transferred)
{
    if (!error || (n_ == bytes_transferred && error == boost::asio::error::eof)) {
        carrot::rpc::Response rpc_resp;

        if (!rpc_resp.ParseFromArray(p_->data, p_->rpc_len)) {
            destroy(boost::system::errc::make_error_code(boost::system::errc::invalid_argument));
            throw std::runtime_error("carrot-rpc: failed to parse rpc request metadata");
        }

        auto it = outgoing_tasks.find(rpc_resp.id());

        if (it == outgoing_tasks.end()) {
            CARROT_LOG("call id %ld not found in the outgoing_tasks, discarding...", rpc_resp.id());
            /* start reading next response */
            start_read();
            return;
        }

        call_task_sptr task = it->second;

        task->state = STATE_DONE;
        switch(rpc_resp.status()) {
            case carrot::rpc::NOT_FOUND:
                task->controller->SetFailed("The remote method called not found on the server side: "  + rpc_resp.message(), EC_METHOD_NOT_FOUND);
            break;
            case carrot::rpc::ERROR:
                task->controller->SetFailed("Some errors occurred during the rpc: " + rpc_resp.message(), EC_ERROR);
            break;
            case carrot::rpc::FAILED:
                task->controller->SetFailed("Failed to call rpc: " +  rpc_resp.message(), EC_FAILED);
            break;
            case carrot::rpc::SUCCESS:
            break;
        }

        /*As caller, ignores request parameter returned from server side.*/
        if (!task->response->ParseFromArray(p_->data + p_->rpc_len, p_->resp_len)) {
            task->controller->SetFailed("failed to read response parameter from server", EC_SERIALIZATION_ERROR);
        }

        /* start reading next response */
        start_read();

        outgoing_tasks.erase(it);

        finish_task(task);
    } else {
        CARROT_LOG("handle_read_payload error: %s", error.message().c_str());
        destroy(error);
        throw boost::system::system_error(error);
    }
}

void RpcClientConnection::handle_write(call_task_sptr task,
        const boost::system::error_code& error,
        std::size_t bytes_transferred)
{
    if (!error) {
        task->state = STATE_READ_DATA;
        CARROT_LOG("wrote out %d bytes", bytes_transferred);
    } else {
        if (!task->called){
            task->controller->SetFailed("failed write rpc packet out, " + error.message(), EC_TRANSPORT_ERROR);
            try {
                task->done->Run();
            } catch (std::exception &e) {
                task->controller->SetFailed(std::string("rpc failed: ") + e.what());
            }
            task->called = true;
        }
        CARROT_LOG("handle_write error: %s", error.message().c_str());
        destroy(error);
        throw boost::system::system_error(error);
    }
}

void CarrotAsyncChannel::CallMethod(const ::google::protobuf::MethodDescriptor * method,
                    ::google::protobuf::RpcController * controller_,
                    const ::google::protobuf::Message * request,
                    ::google::protobuf::Message * response,
                    ::google::protobuf::Closure * done)
{
    CarrotController * ctl = static_cast<CarrotController *>(controller_);

    auto it = conns_.find(ctl->host() + ":" + ctl->service());

    RpcClientConnection::pointer c;

    if(it == conns_.end()) { /* connectino hasn't been established, create a new connection */
        c = RpcClientConnection::create(this, ios_, ctl->host(), ctl->service());
        conns_.insert(std::make_pair(ctl->host() + ":" + ctl->service(), c));
    } else {
        c = it->second;
    }

    c->issue(method, ctl, request, response, done);
}

}