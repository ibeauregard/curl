#include "socket.h"

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

static Socket* from_addr_info(struct addrinfo* addrInfo);
const struct socket_class SocketClass = {
        .fromAddrInfo = &from_addr_info
};

static struct internals {
    int fd;
} internals;

static int get_fd();
static int connect_(struct addrinfo* addr_info);
static ssize_t read_(char* buffer, size_t num);
static ssize_t write_(char* text);
static void close_();
static Socket socket_ = {
        ._internals = &internals,
        .getFd = &get_fd,
        .connect = &connect_,
        .read = &read_,
        .write = &write_,
        .close = &close_
};

Socket* from_addr_info(struct addrinfo* addrInfo)
{
    int socket_fd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (socket_fd == -1) return NULL;
    socket_._internals->fd = socket_fd;
    return &socket_;
}

int get_fd()
{
    return internals.fd;
}

int connect_(struct addrinfo* addr_info)
{
    return connect(internals.fd, addr_info->ai_addr, addr_info->ai_addrlen);
}

ssize_t read_(char* buffer, size_t num)
{
    return read(internals.fd, buffer, num);
}

ssize_t write_(char* text)
{
    return write(internals.fd, text, strlen(text));
}

void close_()
{
    if (close(socket_._internals->fd) == -1)
        fprintf(stderr, "my_curl: error when closing socket: %s\n", strerror(errno));
}
