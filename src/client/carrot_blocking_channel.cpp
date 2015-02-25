/*
* Copyright (C) Xinjing Cho
*/
#include <exception>

#include <arpa/inet.h>
#include <errno.h>

#include "../protob/rpc.pb.h"
#include "carrot_blocking_channel.h"
#include "../common/packet.h"

namespace carrot {

CarrotBlockingChannel::CarrotBlockingChannel(std::string hostname, int port) {
    this->sock = carrot::connect_to(hostname.c_str(), std::to_string(port).c_str());
    if (this->sock < 0) 
        throw std::runtime_error("failed to connect to server");
}

CarrotBlockingChannel::~CarrotBlockingChannel() {
    if (this->sock >= 0) {
        ::close(this->sock);
        this->sock = -1;
    }
}

void CarrotBlockingChannel::CallMethod(const ::google::protobuf::MethodDescriptor * method,
                                ::google::protobuf::RpcController * controller_,
                                const ::google::protobuf::Message * request,
                                ::google::protobuf::Message * response,
                                ::google::protobuf::Closure * done) {
    assert(method != NULL);
    assert(controller_ != NULL);
    assert(request != NULL);
    assert(response != NULL);
    CarrotController * controller = static_cast<CarrotController*>(controller_);
    carrot::rpc::Request rpc_req;
    carrot::rpc::Response rpc_resp;

    rpc_req.set_method_identity(method->service()->name() + "." + method->name());
    /* As caller, the size of Response(metadata) and response(parameter) are left out.*/
    uint32_t rpc_len = rpc_req.ByteSize();
    uint32_t req_len = request->ByteSize();
    uint32_t resp_len = 0;

    size_t  n = sizeof(carrot::packet) + rpc_len + req_len;

    CARROT_LOG("calling %s, rpc_len: %d, req_len: %d, resp_len: %d", rpc_req.method_identity().c_str(), rpc_len, req_len, resp_len);

    carrot::packet * p = (carrot::packet *)::malloc(n);

    if (p == NULL) {
        controller->SetFailed("failed to malloc for outgoing packet", EC_OOM);
        goto run_callback;
    }

    p->rpc_len = rpc_len;
    p->req_len = req_len;
    p->resp_len = resp_len;

    if (!rpc_req.SerializeToArray(p->data, rpc_len + req_len)) {
       controller->SetFailed("failed to serialize Request metadata into byte array", EC_SERIALIZATION_ERROR);
       goto run_callback;
    }

    if (!request->SerializeToArray(p->data + rpc_len, req_len)) {
        controller->SetFailed("failed to serialize request paramerter into byte array", EC_SERIALIZATION_ERROR);
        goto run_callback;
    }

    CONVERT_TO_NETWORK_ORDER(*p);

    if (carrot::write_n(this->sock, (char *)p, n) == -1) {
        controller->SetFailed(std::string("failed to send packet to server:") + ::strerror(errno), EC_TRANSPORT_ERROR);
        goto run_callback;
    }
    
    if (carrot::read_n(this->sock, (char *)&rpc_len, sizeof(uint32_t)) == -1) {
        controller->SetFailed(std::string("failed to read request metadata size from server:")
                             + ::strerror(errno), EC_TRANSPORT_ERROR);
        goto run_callback;
    }

    if (carrot::read_n(this->sock, (char *)&req_len, sizeof(uint32_t)) == -1) {
        controller->SetFailed(std::string("failed to read request parameter size from server:")
                             + ::strerror(errno), EC_TRANSPORT_ERROR);
        goto run_callback;
    }

    if (carrot::read_n(this->sock, (char *)&resp_len, sizeof(uint32_t)) == -1) {
        controller->SetFailed(std::string("failed to read response parameter size from server:")
                             + ::strerror(errno), EC_TRANSPORT_ERROR);
        goto run_callback;
    }

    rpc_len  =  ntohl(rpc_len);
    req_len  =  ntohl(req_len);
    resp_len =  ntohl(resp_len);

    if ((rpc_len + req_len + resp_len) > n - sizeof(carrot::packet)) {
        n = rpc_len + req_len + resp_len + sizeof(carrot::packet);
        p = (carrot::packet *)::realloc(p, n);

        if (p == NULL) {
            controller->SetFailed("failed to malloc for incoming packet", EC_OOM);
            goto run_callback;
        }
    }

    p->rpc_len = rpc_len;
    p->req_len = req_len;
    p->resp_len = resp_len;

    CARROT_LOG("got metadata from server: rpc_len: %d, req_len: %d, resp_len: %d.", rpc_len, req_len, resp_len);
    if (carrot::read_n(this->sock, p->data, rpc_len + req_len + resp_len) == -1) {
        controller->SetFailed(std::string("failed to read incoming packet from server:")
                             + ::strerror(errno), EC_TRANSPORT_ERROR);
        goto run_callback;
    }

    if (!rpc_resp.ParseFromArray(p->data, rpc_len)) {
        controller->SetFailed("failed to read Response metadata from server", EC_SERIALIZATION_ERROR);
        goto run_callback;
    }

    switch(rpc_resp.status()) {
        case carrot::rpc::NOT_FOUND:
            controller->SetFailed("The remote method called not found on the server side: "  + rpc_resp.message(), EC_METHOD_NOT_FOUND);
        break;
        case carrot::rpc::ERROR:
            controller->SetFailed("Some errors occurred during the rpc: " + rpc_resp.message(), EC_ERROR);
        break;
        case carrot::rpc::FAILED:
            controller->SetFailed("Failed to call rpc: " +  rpc_resp.message(), EC_FAILED);
        break;
        case carrot::rpc::SUCCESS:
        break;
    }

    /*As caller, ignores request parameter returned from server side.*/
    if (!response->ParseFromArray(p->data + rpc_len, resp_len)) {
        controller->SetFailed("failed to read response parameter from server", EC_SERIALIZATION_ERROR);
        goto run_callback;
    }

run_callback:
    done->Run();

    ::free(p);
}

}