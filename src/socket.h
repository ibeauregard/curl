#ifndef MY_CURL_SOCKET_H
#define MY_CURL_SOCKET_H

#include <netdb.h>

typedef struct socket Socket;
struct socket {
    int (*getFd)();
    int (*connect)(struct addrinfo* addrInfo);
    ssize_t (*read)(char* buffer, size_t num);
    ssize_t (*write)(char* text);
    void (*close)();
};

extern const struct socket_class {
    Socket* (*fromAddrInfo)(struct addrinfo* addrInfo);
} SocketClass;

#endif
