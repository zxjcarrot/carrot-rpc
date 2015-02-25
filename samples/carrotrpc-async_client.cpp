#include <iostream>
#include <thread>
#include <cmath>
#include <random>

#include <carrot-rpc/client/carrot_async_channel.h>
#include <carrot-rpc/common/carrot_controller.h>

#include "arithmetic.pb.h"

int c = 0;
std::atomic<int> n;
const char * ip;

boost::asio::io_service ios;
boost::asio::io_service::work work(ios);
carrot::CarrotAsyncChannel * channel;
rpc::arith::ArithService* service;

std::default_random_engine generator;

void Done(carrot::CarrotController * controller, rpc::arith::ArithResponse * resp) {
    
    if (controller->Failed()) {
        std::cout << " ErrorCode: " << controller->ErrorCode() 
                  << ", ErrorText: " << controller->ErrorText() << std::endl;
    }
    std::cout << resp->DebugString() << std::endl;
}

void DoArith(int count) {
    std::uniform_int_distribution<int> dis(0,120);
    for (int i = 0; i < count; ++i) {
        carrot::CarrotController * controller = new carrot::CarrotController(ip, "8888", 1000);
        rpc::arith::ArithRequest * request = new rpc::arith::ArithRequest;
        rpc::arith::ArithResponse* response = new rpc::arith::ArithResponse;

        // Set up the request.
        request->set_type(rpc::arith::ArithType(dis(generator) % 4 + 1));
        request->set_op1(dis(generator));
        request->set_op2(dis(generator));

        //std::cout << "requqest:\n "  << request.DebugString() << std::endl;
        // Execute the RPC.
        service->compute(controller, request, response, ::google::protobuf::NewCallback(&Done, controller, response));

        if (controller->Failed()) {
            std::cout<< controller->ErrorText() << std::endl;
        }
        //std::cout << "response: " << response.DebugString() << std::endl;
    }
}

void worker() {
    DoArith(ceil((double)n / c));
}

void reactor_thread() {
    // You provide classes MyRpcChannel and MyRpcController, which implement
    // the abstract interfaces protobuf::RpcChannel and protobuf::RpcController.
    channel = new carrot::CarrotAsyncChannel(ios);

    // The protocol compiler generates the SearchService class based on the
    // definition given above.
    service = new rpc::arith::ArithService::Stub(channel);

    for (;;) {
        try {
            ios.run();
        } catch (std::exception & e) {
            std::cerr << e.what() << std::endl;
        }
    }
    

    std::cout << "async channel stopped" << std::endl;
    delete channel;
    delete service;
}

int main(int argc, char const *argv[]) {
    if (argc != 4) {
        std::cout << "usage: ./carrotrpc-client ip-address-of-server number-of-invocations number-of-concurrent-requests." << std::endl;
        return -1;
    }
    ip = argv[1];
    n = atoi(argv[2]);
    c = atoi(argv[3]);
    
    auto t = std::thread(reactor_thread);

    char buf[1024];
    while (fgets(buf, 1024, stdin)) {
        std::thread(worker).detach();
    }
    std::cout << "exiting" << std::endl;

    t.join();
    return 0;
}