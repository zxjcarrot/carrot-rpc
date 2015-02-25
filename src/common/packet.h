/*
* Copyright (C) Xinjing Cho
*/
#ifndef CARROT_RPC_PACKET_H_
#define CARROT_RPC_PACKET_H_
#include <stdint.h>

#include <arpa/inet.h>

namespace carrot {

/* packet sent over the network containing nesessary metadata */
struct packet {
    uint32_t  rpc_len;  /* the sum of length of Request and Response */
    uint32_t  req_len;  /* size of the request parameter */
    uint32_t  resp_len; /* sizeof the response paramerter */
    char      data[];
};

#define CONVERT_TO_NETWORK_ORDER(p)do {\
    (p).rpc_len = htonl((p).rpc_len);  \
    (p).req_len = htonl((p).req_len);  \
    (p).resp_len = htonl((p).resp_len);\
}while(0);

#define CONVERT_TO_HOST_ORDER(p)do {\
    (p).rpc_len = ntohl((p).rpc_len);  \
    (p).req_len = ntohl((p).req_len);  \
    (p).resp_len = ntohl((p).resp_len);\
}while(0);

}
#endif