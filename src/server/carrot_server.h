/*
* Copyright (C) Xinjing Cho
*/
#ifndef CARROT_RPC_SERVER_H_
#define CARROT_RPC_SERVER_H_

#include <map>
#include <string>
#include <functional>
#include <memory>

#include <google/protobuf/service.h>

#include "../protob/rpc.pb.h"
#include "../common/packet.h"
#include "../common/utils.h"

namespace carrot {

#define PROTOCOL_IP4    0
#define PROTOCOL_IP6    1

struct carrot_service {
    ::google::protobuf::Service * s;
    ::google::protobuf::Closure * c;
};

typedef std::unique_ptr<google::protobuf::Message> upm;

class CarrotServer {
public:
    /*
    * Create a rpc server that listens on @port with protocol @protocol and address type @address_type.
    * std::runtime_error is thrown is the creation failed.
    */
    CarrotServer(short port, int protocol = PROTOCOL_IP4);
    CarrotServer() {}
    virtual ~CarrotServer() {}
    virtual void Run() = 0;
    virtual void Stop() = 0;
    /*
    * Register a server on the server, this function should be called before starting the server.
    */
    bool RegisterService(::google::protobuf::Service * service,
                         ::google::protobuf::Closure * closure);
protected:
    std::map<std::string, carrot_service>   services;
    int                                     listen_sock;
};

}
#endif