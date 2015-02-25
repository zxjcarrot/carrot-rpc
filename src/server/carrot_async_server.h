/*
* Copyright (C) Xinjing Cho
*/
#ifndef CARROT_RPC_ASYNC_SERVER_H_
#define CARROT_RPC_ASYNC_SERVER_H_

#include <errno.h>
#include <stdlib.h>
#include <assert.h>

#include <memory>
#include <utility>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "../protob/rpc.pb.h"
#include "../common/utils.h"
#include "../common/carrot_controller.h"
#include "carrot_server.h"

namespace carrot {

using boost::asio::ip::tcp;

class CarrotAsyncServer;
typedef boost::shared_ptr<CarrotAsyncServer> async_server_sptr;

class RpcServerConnection
    : public boost::enable_shared_from_this<RpcServerConnection> {
public:
    typedef boost::shared_ptr<RpcServerConnection> pointer;

    static pointer create(CarrotAsyncServer * server, boost::asio::io_service& ios) 
    {
        return pointer(new RpcServerConnection(server, ios));
    }

    ~RpcServerConnection() {
        if (p_ && size_) {
            ::free(p_);
            size_ = 0;
        }
    }

    boost::asio::ip::tcp::socket& socket()
    {
        return socket_;
    }

    void destroy();

    void serve();

    void server(CarrotAsyncServer * server)
    {
        server_ = server;
    }
private:
    
    RpcServerConnection(CarrotAsyncServer * server, boost::asio::io_service& ios)
    : server_(server), socket_(ios), p_(NULL), size_(0)
    {}

    void handle_read_header(const boost::system::error_code& error,
        std::size_t bytes_transferred);
    void handle_read_payload(const boost::system::error_code& error,
        std::size_t bytes_transferred);
    void handle_write_response(const boost::system::error_code& error,
        std::size_t bytes_transferred);
    void respond_to_client(::google::protobuf::Message * resp,
        rpc::Response * rpc_resp);

    CarrotAsyncServer*              server_;
    tcp::socket                     socket_;
    packet*                         p_;
    uint32_t                        size_; /* size of p */
    uint32_t                        n_;

};

class CarrotAsyncServer {
public:
    CarrotAsyncServer(boost::asio::io_service & ios, std::string bind_ip , unsigned short port)
        : acceptor_(ios, tcp::endpoint(boost::asio::ip::address::from_string(bind_ip), port))
    {
        start_accept();
    }

    CarrotAsyncServer(boost::asio::io_service & ios, unsigned short port)
        : acceptor_(ios, tcp::endpoint(tcp::v4(), port))
    {
        start_accept();
    }
    
    ~CarrotAsyncServer() {
        for (auto it : conns_) {
            RpcServerConnection::pointer c = it.second;
            /* nullify the back pointer to server of every server*/
            c->server(nullptr);
        }
    }

    std::map<std::string, carrot_service>& services()
    {
        return services_;
    }

    std::map<uint64_t, RpcServerConnection::pointer>& conns()
    {
        return conns_;
    }

    /*
    * Register a server on the server, this function should be called before starting the server.
    */
    bool RegisterService(::google::protobuf::Service * service,
                         ::google::protobuf::Closure * closure);
private:

    void start_accept();

    void handle_accept(RpcServerConnection::pointer c, const boost::system::error_code& error);
    
    std::map<uint64_t, RpcServerConnection::pointer>  conns_;
    std::map<std::string, carrot_service>       services_;
    tcp::acceptor                               acceptor_;
};


}
#endif