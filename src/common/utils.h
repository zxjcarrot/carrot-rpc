/*
* Copyright (C) Xinjing Cho
*/
#ifndef CARROT_RPC_UTILS_H_
#define CARROT_RPC_UTILS_H_
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <errno.h>

#include "packet.h"

namespace carrot {

#ifdef LOGON
#define CARROT_LOG _stderr_print
#else
#define CARROT_LOG (void)
#endif 

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(expression)\
                            ({ long int __result;\
                            do __result = (long int)(expression);\
                            while(__result == -1L&& errno == EINTR);\
                            __result;})
#endif


/*
* connect to @hostname and fills up @addr structure
* based on the hostname info provided by @hostname and @port;
* Return socket fd if successful, -1 if some errors occurred and addr stays untouched.
*/
int connect_to(const char * hostname, const char * servname);

/*
* create a server TCP socket binds&listens on it.
* @type: type of the socket(SOCK_DE)
* @protofamily: protocol family used to create this socket, one of PF_INET and PF_INET6
* Return socket fd if successful, -1 if some errors occurred
*/
int create_server_tcp_socket(short port, int protofamily);

/*
* resolve @hostname to ip address.
* return 0 if successful and addr is filled, -1 otherwise and addr stays untouched.
*/
int resolve_hostname(const char * hostname, const char * servname, struct sockaddr_storage * addr, socklen_t *addr_len);

/*
* create a nonblocking TCP socket on it.
* @type: type of the socket(SOCK_STREAM, SOCK_DGRAM)
* @protofamily: protocol family used to create this socket, one of PF_INET and PF_INET6
* Return socket fd if successful, -1 if some errors occurred
*/
int create_nonblocking_tcp_socket(int protofamily, int port);

int set_fd_nonblocking(int fd);

int read_n(int sock, char * buf, size_t n);

int write_n(int sock, char * buf, size_t n);

void _stderr_print(const char *fmt, ...);

/*
* expand the packet to be big enough to contain at least @n bytes of payload
*/
int expand_packet(packet ** p, uint32_t * size, uint32_t n);
}
#endif