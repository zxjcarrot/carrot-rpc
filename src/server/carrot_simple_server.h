/*
* Copyright (C) Xinjing Cho
*/
#ifndef CARROT_RPC_SIMPLE_SERVER_H_
#define CARROT_RPC_SIMPLE_SERVER_H_

#include <errno.h>
#include <stdlib.h>
#include <assert.h>

#include <memory>
#include <utility>

#include <cheetah/reactor.h>

#include "../protob/rpc.pb.h"
#include "../common/utils.h"
#include "carrot_server.h"
#include "../common/carrot_controller.h"
namespace carrot {

class CarrotSimpleServer;

typedef std::function<void(struct rpc_connection*)> ClientHandler;
typedef std::function<void(void)>                   AcceptHandler;

struct rpc_connection {
    int             client_fd;
    bool            alive;
    struct event    e;
    ClientHandler   handle_client_func;
    packet *        p;
    uint32_t        size; /* size of p */
};


/* single-threaded reactor-based rpc server implementaion */
class CarrotSimpleServer: public CarrotServer {
public:    
    CarrotSimpleServer(short port, int protocol = PROTOCOL_IP4);
    ~CarrotSimpleServer();

    /*
    * Start the server and processing requests.
    */
    void Run();

    void Stop() { reactor_get_out(&this->r); }
private:
    void handle_client(struct rpc_connection * c);
    void handle_listen_sock();
    void respond_to_client(struct rpc_connection * c,
                           ::google::protobuf::Message * resp,
                           rpc::Response * rpc_resp);
    void destroy_rpc_connection(struct rpc_connection * c);
    std::vector<struct rpc_connection *>    conns_unused; /* resusable connections */
    std::map<uint64_t, rpc_connection *>    conns_in_use;
    AcceptHandler                           accept_handler;
    struct event                            listen_event;
    struct reactor                          r;
};

}
#endif