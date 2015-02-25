/*
* Copyright (C) Xinjing Cho
*/
#ifndef CARROT_RPC_ASYNC_CHANNEL_H_
#define CARROT_RPC_ASYNC_CHANNEL_H_
#include <string>
#include <memory>
#include <queue>
#include <mutex>
#include <atomic>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/descriptor.h>

#include "../protob/rpc.pb.h"
#include "../common/packet.h"
#include "../common/carrot_controller.h"
#include "../common/utils.h"

namespace carrot {

using boost::asio::ip::tcp;

#define STATE_CONNECT_TO_SERVER 0
#define STATE_PREPARE_TO_WRITE_TO_SERVER 1
#define STATE_WRITE_TO_SERVER 2
#define STATE_READ_METADATA 3
#define STATE_READ_DATA 4
#define STATE_TIMEOUT 5
#define STATE_DONE 6



typedef std::shared_ptr<carrot::rpc::Request>  rpc_request_sptr;
typedef std::shared_ptr<carrot::rpc::Response> rpc_response_sptr;

struct call_task{
    call_task(boost::asio::io_service& ios, uint64_t call_id)
        : timer(ios), p(nullptr), size(0), id(call_id), called(false), state(0)
        {}
    
    ~call_task()
    {
        if (p && size) {
            ::free(p);
            p = nullptr;
            size = 0;
        }
        method = nullptr;
        controller = nullptr;
        request = nullptr;
        response = nullptr;
        done = nullptr;
    }

    const ::google::protobuf::MethodDescriptor * method;
    CarrotController * controller;
    const ::google::protobuf::Message * request;
    ::google::protobuf::Message * response;
    /* client might provide self-deleting Closure */
    ::google::protobuf::Closure * done;
    uint32_t timeout;
    boost::asio::deadline_timer timer;
    packet*  p;
    uint32_t size;
    uint64_t id;
    /* if @done has been called */
    bool called;
    int  state;
};


typedef boost::shared_ptr<call_task> call_task_sptr;

class CarrotAsyncChannel;
class RpcClientConnection
    : public boost::enable_shared_from_this<RpcClientConnection> {
public:
    typedef boost::shared_ptr<RpcClientConnection> pointer;

    static pointer create(CarrotAsyncChannel * chann, boost::asio::io_service& ios,
        const std::string & host, const std::string & service ) {
        return pointer(new RpcClientConnection(chann, ios, host, service));
    }

    ~RpcClientConnection();

    void issue(const ::google::protobuf::MethodDescriptor * method,
                    CarrotController * controller,
                    const ::google::protobuf::Message * request,
                    ::google::protobuf::Message * response,
                    ::google::protobuf::Closure * done);

private:
    RpcClientConnection(CarrotAsyncChannel * chann, boost::asio::io_service& ios,
        const std::string & host, const std::string & service )
        : chann_(chann), id_(0), socket_(ios), resolver_(ios), timer_(ios),
          host_(host), service_(service), p_(nullptr), size_(0), n_(0), connecting_(false)
        {}

    void destroy(const boost::system::error_code reason);
    void setup_task_timer(call_task_sptr task);
    void handle_timeout(call_task_sptr task, const boost::system::error_code& error);
    void handle_resolve(call_task_sptr task,
            const boost::system::error_code& error,
            tcp::resolver::iterator endpoint_iterator);
    void handle_connect(call_task_sptr task,
            const boost::system::error_code& error,
            tcp::resolver::iterator endpoint_iterator);
    void finish_task(call_task_sptr task);
    void start_call(call_task_sptr task);
    void start_read();
    void handle_read_header(const boost::system::error_code& error,
        std::size_t bytes_transferred);
    void handle_read_payload(const boost::system::error_code& error,
        std::size_t bytes_transferred);
    void handle_write(call_task_sptr task,
        const boost::system::error_code& error,
        std::size_t bytes_transferred);
    
    CarrotAsyncChannel*                     chann_;
    /* id for each individual call, incremented for every call */
    uint64_t                                id_;
    tcp::socket                             socket_;
    /* every outgoing task is mapped by call id */
    std::map<uint64_t, call_task_sptr>      outgoing_tasks;
    tcp::resolver                           resolver_;
    boost::asio::deadline_timer             timer_;
    std::string                             host_;
    std::string                             service_;
    packet*                                 p_;
    uint32_t                                size_;
    uint32_t                                n_;
    bool                                    connecting_;
};

class CarrotAsyncChannel
    : public ::google::protobuf::RpcChannel {
public:
    CarrotAsyncChannel(boost::asio::io_service& ios)
        : ios_(ios)
        {}

    ~CarrotAsyncChannel()
        {}

    /**
    * Call the method described by @method on the server side with
    * @request. After the call returned, response is stored
    * @reponse and error conditions during the call
    * are stored in @contorller if any.
    * Note: currently @controller has to be carrot::CarrotController
    * and @controller_->host() and @controller->service() should contain
    * the hostname and the service of the rpc server being called.
    */
    void CallMethod(const ::google::protobuf::MethodDescriptor * method,
                    ::google::protobuf::RpcController * controller_,
                    const ::google::protobuf::Message * request,
                    ::google::protobuf::Message * response,
                    ::google::protobuf::Closure * done);
    
    std::map<std::string, RpcClientConnection::pointer> conns()
    {
        return conns_;
    }

private:
    boost::asio::io_service& ios_;
    /* every rpc connection is mapped by 'hostname:service' of a rpc server */
    std::map<std::string, RpcClientConnection::pointer> conns_;
};


}

#endif