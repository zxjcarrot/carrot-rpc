/*
* Copyright (C) Xinjing Cho
*/
#ifndef CARROT_RPC_CHANNEL_H_
#define CARROT_RPC_CHANNEL_H_
#include <string>

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/descriptor.h>

#include "../common/utils.h"
#include "../common/carrot_controller.h"

namespace carrot {

/*
* simple blocking rpc channel implementation.
*/
class CarrotBlockingChannel : public ::google::protobuf::RpcChannel {
public:
    /**
    * Build a blocking channel bewtween client and server.
    * @hostname: the hostname of the server, could be domain name or ip address.
    * @port: the port to connect to.
    */
    CarrotBlockingChannel(std::string hostname, int port);
    ~CarrotBlockingChannel();

    /**
    * Call the method described by @method on the server side with
    * @request. After the call returned, response is stored
    * @reponse and error conditions during the call
    * are stored in @contorller if any.
    * Note: currently @controller has to be carrot::CarrotController.
    */
    void CallMethod(const ::google::protobuf::MethodDescriptor * method,
                    ::google::protobuf::RpcController * controller,
                    const ::google::protobuf::Message * request,
                    ::google::protobuf::Message * response,
                    ::google::protobuf::Closure * done);
private:
    int             sock;
};

}
#endif