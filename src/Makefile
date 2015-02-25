#sources
CARROT_SRC = common/utils.cpp server/carrot_async_server.cpp server/carrot_server.cpp server/carrot_simple_server.cpp server/carrot_threaded_server.cpp common/carrot_controller.cpp client/carrot_blocking_channel.cpp client/carrot_async_channel.cpp protob/rpc.pb.cc
#object files
CARROT_OBJS = $(CARROT_SRC:.cpp=.o)
#executable
PROGRAM = libcarrotrpc.so
#compiler
CC = g++

#includes
INCLUDE = -Icommon -Iserver -Iclient -I.
#linker params
LINKPARAMS = -lcheetah -lprotobuf -lboost_system -lboost_log -lpthread
#options for development
CFLAGS = --std=c++11 -g -O0 -Wall -fpic -shared -Wno-unused-value
#options for release
#CFLAGS = --std=c++11 -g -O2 -Wall -Werror

all:
	$(CC) -o $(PROGRAM) $(CFLAGS) $(INCLUDE) $(CARROT_SRC) $(LINKPARAMS)

rpc-client:
	$(CC) -o carrotrpc-client $(CFLAGS) $(INCLUDE) carrotrpc-client.cpp arithmetic.pb.cc $(CARROT_SRC) $(LINKPARAMS)
rpc-server:
	$(CC) -o carrotrpc-server $(CFLAGS) $(INCLUDE) carrotrpc-server.cpp arithmetic.pb.cc $(CARROT_SRC) $(LINKPARAMS)
install:
	rm -rf /usr/include/carrot-rpc
	mkdir /usr/include/carrot-rpc
	mkdir /usr/include/carrot-rpc/client
	mkdir /usr/include/carrot-rpc/common
	mkdir /usr/include/carrot-rpc/server
	mkdir /usr/include/carrot-rpc/protob

	cp -R client/*.h /usr/include/carrot-rpc/client
	cp -R common/*.h /usr/include/carrot-rpc/common
	cp -R server/*.h /usr/include/carrot-rpc/server
	cp -R protob/*.h /usr/include/carrot-rpc/protob
	cp $(PROGRAM) 	 /usr/lib

uninstall:
	rm -rf /usr/include/carrot-rpc
	rm /usr/lib/$(PROGRAM)
clean:
	-rm $(PROGRAM)
	-rm *.o
