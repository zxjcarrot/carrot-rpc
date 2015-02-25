/*
* Copyright (C) Xinjing Cho
*/
#ifndef CARROT_RPC_CONTROLLER_H_
#define CARROT_RPC_CONTROLLER_H_
#include <string>

#include <google/protobuf/service.h>

namespace carrot {

enum error_code {
    EC_SUCCESS,
    EC_ERROR,
    EC_FAILED,
    EC_CONNECTION_FAILED,
    EC_OOM,
    EC_SERIALIZATION_ERROR,
    EC_TRANSPORT_ERROR,
    EC_INVALID_INTERNAL_STATE,
    EC_METHOD_NOT_FOUND,
    EC_RPC_TIMEOUT
};


#define TIMEOUT_NEVER 0

class CarrotController: public ::google::protobuf::RpcController {
public:
    CarrotController()
        : failed_(false), cancelled_(false), code_(EC_SUCCESS) 
        {}
    CarrotController(std::string host, std::string service, uint32_t timeout)
        : failed_(false), cancelled_(false), code_(EC_SUCCESS),
          host_(host), service_(service), timeout_(timeout)
        {}
    virtual ~CarrotController(){}

    virtual void SetFailed(const std::string & reason) {
        error_text_ = reason;
        failed_ = true;
        code_ = EC_FAILED;
    }

    void SetFailed(const std::string & reason, error_code ec) {
        SetFailed(reason);
        code_ = ec;
    }

    virtual bool IsCanceled() const {
        return cancelled_;
    }

    void Reset() {
        error_text_ = "";
        cancelled_ = false;
        failed_ = false;
        code_ = EC_SUCCESS;
    }

    virtual bool Failed() const {
        return failed_;
    }

    virtual void StartCancel() {
        cancelled_ = true;
    }

    virtual std::string ErrorText() const {
        return error_text_;
    }

    error_code ErrorCode() const {
        return code_;
    }

    virtual void NotifyOnCancel(::google::protobuf::Closure * callback) {
        cancel_callback_ = callback;
    }   

    void host(const std::string & host) {
        host_ = host;
    }

    void service(const std::string & service) {
        service_ = service;
    }
        
    void timeout(uint32_t timeout) {
        timeout_ = timeout;
    }
    std::string host() {
        return host_;
    }

    std::string service() {
        return service_;
    }

    uint32_t timeout() {
        return timeout_;
    }
private:
    bool                            failed_;
    bool                            cancelled_;
    std::string                     error_text_;
    ::google::protobuf::Closure *   cancel_callback_;
    error_code                      code_;

    /* used by async channel to issue multiple rpc to different server */ 
    /*  hostname of the rpc server(ip address, domain name etc) */
    std::string                     host_;
    /* @service is either a decimal port number or a well-known service name listed in /etc/services*/
    std::string                     service_;
    /* timeout of for a call */
    uint32_t                        timeout_;
};

}

#endif