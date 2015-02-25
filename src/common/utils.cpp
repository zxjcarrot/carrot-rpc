/*
* Copyright (C) Xinjing Cho
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include <fcntl.h>
#include <errno.h>
#include <netdb.h>

#include "utils.h"

namespace carrot{
    int connect_to(const char * hostname, const char * servname) {
        struct addrinfo hints, *res, *res0;
        int             error;
        int             s;
        const  char *   cause = NULL;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = PF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        error = getaddrinfo(hostname, servname, &hints, &res0);

        if (error) {
            CARROT_LOG("conect_to: %s", gai_strerror(error));
            return -1;
        }

        s = 1;

        for (res = res0; res; res = res->ai_next) {
            s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

            if (s < 0) {
                cause = "socket";
                continue;
            }

            if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
                cause = "connect";
                close(s);
                s = -1;
                continue;
            }
            break;
        }

        if (s < 0) {
            CARROT_LOG("%s: can't connect to server due to %s: %s", __func__, cause, strerror(errno));
        }

        return s;
    }

    int resolve_hostname(const char * hostname, const char * servname, struct sockaddr_storage * addr, socklen_t *addr_len) {
        struct addrinfo hints, *res, *res0;
        int             error;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = PF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        error = getaddrinfo(hostname, servname, &hints, &res0);

        if (error) {
            CARROT_LOG("failed in get_ipaddr: %s", gai_strerror(error));
            return -1;
        }

        for (res = res0; res; res = res->ai_next) {
            memcpy(addr, res->ai_addr, res->ai_addrlen);
            *addr_len = res->ai_addrlen;
            break;
        }

        if (res == NULL) {
            CARROT_LOG("couldn't resolve hosename %s to ip address", hostname);
            return -1;
        }

        freeaddrinfo(res0);
        
        return 0;
    }
    int create_server_tcp_socket(short port, int protofamily) {
        int sock = ::socket(protofamily, SOCK_STREAM, 0);

        if (sock < 0) {
            CARROT_LOG("failed to create a socket: %s", ::strerror(errno));
            return -1;
        }
        CARROT_LOG("created a socket: %d", sock);

        struct sockaddr addr;
        ::memset(&addr, 0, sizeof(sockaddr));
        
        if (protofamily == AF_INET) {
            CARROT_LOG("using ipv4.");
            ((struct sockaddr_in *)&addr)->sin_family = AF_INET;
            ((struct sockaddr_in *)&addr)->sin_addr.s_addr = htonl(INADDR_ANY);
            ((struct sockaddr_in *)&addr)->sin_port = htons(port);
        } else {
            CARROT_LOG("using ipv6.");
            ((struct sockaddr_in6 *)&addr)->sin6_family = AF_INET6;
            ::memcpy(&((struct sockaddr_in6 *)&addr)->sin6_addr.s6_addr, &in6addr_any, sizeof(in6addr_any));
             ((struct sockaddr_in6 *)&addr)->sin6_port = htons(port);
        }

        if (::bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            CARROT_LOG("failed to bind the socket on %d: %s", port, ::strerror(errno));
            return -1;
        }

        if (::listen(sock, 128) == -1) {
            CARROT_LOG("failed to listen on socket: %s", ::strerror(errno));
            return -1;
        }

        return sock;
    }

    int create_nonblocking_tcp_socket(int protofamily, int type) {
        int sock = ::socket(protofamily, type, 0);

        if (sock < 0) {
            CARROT_LOG("failed to create a socket: %s", ::strerror(errno));
            return -1;
        }

        if (set_fd_nonblocking(sock) == -1) {
            CARROT_LOG("failed to set the socket %d to nonblocking mode", sock);
            ::close(sock);
            return -1;
        }

        return sock;
    }

    int write_n(int sock, char * buf, size_t n) {
        ssize_t nwrite;
        size_t  n0 = 0;

        while(n0 < n) {
            nwrite = ::write(sock, buf + n0, n - n0);
            if (nwrite <= 0) {
                if (errno == EINTR) {
                    continue;
                }
                return -1;
            }
            n0 += nwrite;
        }

        return 0;    
    }

    int read_n(int sock, char * buf, size_t n) {
        ssize_t nread;
        size_t  n0 = 0;

        while (n0 < n) {
            nread = ::read(sock, buf + n0, n - n0);
            if (nread <= 0) {
                if (errno == EINTR) {
                    continue;
                }
                return -1;
            }
            n0 += nread;
        }

        return 0;
    }

    void _stderr_print(const char *fmt, ...) {
        va_list list;

        va_start(list, fmt);
        vfprintf(stderr, fmt, list);
        va_end(list);

        fputc('\n', stderr);
    }

    int set_fd_nonblocking(int fd) {
        int flags = 0;

        if ((flags = fcntl(fd, F_GETFL)) == -1) {
            CARROT_LOG("failed to fcntl(fd, F_GETFL) on fd[%d]: %s",
                      fd, ::strerror(errno));
            return -1;
        }

        if ((flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK)) == -1){
            CARROT_LOG("failed to set nonblocking mode for fd[%d]: %s",
                      fd, ::strerror(errno));
            return -1;
        }

        return 0;
    }

    int expand_packet(packet ** p, uint32_t * size, uint32_t n) {
        assert(p != NULL);

        if (*size < n || *p == NULL) {
            if (*size == 0)
                *size = 1;

            while (*size < n) {
                *size *= 2;
            }

            if ((*p = (carrot::packet *)::realloc(*p, *size)) == NULL) {
                return -1;
            }
        }

        return 0;
    }

}