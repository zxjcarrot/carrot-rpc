/*
* Copyright (C) Xinjing Cho
*/
#ifndef CARROT_RPC_THREADED_SERVER_H_
#define CARROT_RPC_THREADED_SERVER_H_

#include <errno.h>
#include <stdlib.h>
#include <assert.h>

#include <memory>
#include <utility>

#include "../protob/rpc.pb.h"
#include "../common/utils.h"
#include "carrot_server.h"
#include "../common/carrot_controller.h"

namespace carrot {

/**
* Simple request-per-thread server
*/
class CarrotThreadedServer:public CarrotServer {
public:
    CarrotThreadedServer(short port, int protocol = PROTOCOL_IP4):
        CarrotServer(port, protocol), stopped(true) {}
    CarrotThreadedServer() {}
    ~CarrotThreadedServer() {}
    
    /*
    * Start the server and processing requests.
    */
    void Run();

    void Stop() { stopped = true; }

private:
    void handle_client(int fd);
    bool stopped;
};
}
#endif