##Overview
a rpc libray written in C++ leveraging boost.asio, cheetah, protobuf.
###Features
Blocking RPC client.  
Async RPC client.  
Blocking RPC server.  
Async RPC server.  
Threaded RPC server.  
###Future work
Thread pool RPC server.
###Prerequsites
[protobuf 2.6.0](https://developers.google.com/protocol-buffers/)  
[boost.asio boost.log](http://www.boost.org/users/history/version_1_57_0.html)  
[cheetah](https://github.com/zxjcarrot/cheetah)  
##Installation
    git clone https://github.com/zxjcarrot/carrot-rpc
    cd carrot-rpc
    make
    sudo make install  

##Examples  
A async rpc client and async rpc server [example](https://github.com/zxjcarrot/carrot-rpc/tree/master/examples).
