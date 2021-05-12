#ifndef MY_CURL_SOCKET_H
#define MY_CURL_SOCKET_H

#include <netdb.h>

typedef struct socket Socket;
struct socket {
    struct internals* _internals;
    int (*connect)(Socket* this, struct addrinfo* addrInfo);
    ssize_t (*read)(Socket* this, char* buffer, size_t num);
    ssize_t (*write)(Socket* this, char* text);
    void (*close)(Socket* this);
};

extern const struct socket_class {
    Socket* (*fromAddrInfo)(struct addrinfo* addrInfo);
} SocketClass;

#endif
