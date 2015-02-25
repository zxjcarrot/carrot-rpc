#compiler
CC = g++

#linker params
LINKPARAMS = -lcheetah -lprotobuf -lboost_system -lboost_log -lpthread -lcarrotrpc
#options for development
CFLAGS = --std=c++11 -g -O0 -Wall -Wno-unused-value

rpc-client:
	$(CC) -o carrotrpc-client $(CFLAGS) carrotrpc-async_client.cpp arithmetic.pb.cc $(LINKPARAMS)
rpc-server:
	$(CC) -o carrotrpc-server $(CFLAGS) carrotrpc-server.cpp arithmetic.pb.cc $(LINKPARAMS)
