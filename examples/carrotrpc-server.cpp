#include <iostream>
#include <cassert>
#include <cstdint>
#include <thread>
#include <chrono>

#include <boost/asio.hpp>

#include <google/protobuf/service.h>
#include <carrot-rpc/server/carrot_async_server.h>
#include <carrot-rpc/common/carrot_controller.h>

#include "arithmetic.pb.h"


class ArithServiceImpl: public rpc::arith::ArithService {
public:
    ArithServiceImpl() {}
    ~ArithServiceImpl() {}

    void compute(::google::protobuf::RpcController* controller,
           const ::rpc::arith::ArithRequest* request,
           ::rpc::arith::ArithResponse* response,
           ::google::protobuf::Closure* done) {
        assert(request != NULL);
        assert(response != NULL);

        std::cout << "request: \n" << request->DebugString() << std::endl;
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        switch (request->type()) {
            case ::rpc::arith::Addition:
                response->set_res(request->op1() + request->op2());
                break;
            case ::rpc::arith::Subtraction:
                response->set_res(request->op1() - request->op2());
                break;
            case ::rpc::arith::Multiplication:
                response->set_res(request->op1() * request->op2());
                break;
            case ::rpc::arith::Division:
                if (request->op2() == 0) {
                    controller->SetFailed("divisor can't be 0");
                    response->set_res(0);
                } else {
                    response->set_res(request->op1() / request->op2());
                }
                break;
        }
        done->Run();
        std::cout << "response: \n" << response->DebugString() << std::endl;
    }
};

void Done() {
    std::cout << "got called" << std::endl;
}
int main(int argc, char const *argv[]) {
    boost::asio::io_service ios;
    boost::asio::io_service::work work(ios);

    carrot::CarrotAsyncServer server(ios, 8888);
    ArithServiceImpl s;
    server.RegisterService(&s, ::google::protobuf::NewPermanentCallback(&Done));
    
    for (;;) {
        try{
            ios.run();
        } catch (std::exception & e) {
            std::cerr << e.what() << std::endl;
        }
    }

    return 0;
}