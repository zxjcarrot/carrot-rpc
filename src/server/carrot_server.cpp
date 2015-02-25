/*
* Copyright (C) Xinjing Cho
*/
#include "carrot_server.h"

namespace carrot {

    CarrotServer::CarrotServer(short port, int protocal) {
        this->listen_sock = carrot::create_server_tcp_socket(port,
                            protocal == PROTOCOL_IP4 ? AF_INET : AF_INET6);
        if (listen_sock == -1) {
            throw std::runtime_error("failed to create a server socket");
        }
    }
    /*
    * Register a service on the server, this function should be called before starting the server.
    */
    bool CarrotServer::RegisterService(::google::protobuf::Service * service,
                                       ::google::protobuf::Closure * closure) {
        std::string key = service->GetDescriptor()->name();
        if (services.find(key) != services.end()) {
            return false;
        }

        CARROT_LOG("Registering service %s.", key.c_str());
        struct carrot_service cs;
        cs.s = service;
        cs.c = closure;
        services.insert(std::make_pair(key, cs));

        return true;
    }
}