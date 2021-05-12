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

static int connect_(Socket* this, struct addrinfo* addr_info);
static ssize_t read_(Socket* this, char* buffer, size_t num);
static ssize_t write_(Socket* this, char* text);
static void close_(Socket* this);
static Socket socket_ = {
        ._internals = &internals,
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

int connect_(Socket* this, struct addrinfo* addr_info)
{
    return connect(this->_internals->fd, addr_info->ai_addr, addr_info->ai_addrlen);
}

ssize_t read_(Socket* this, char* buffer, size_t num)
{
    return read(this->_internals->fd, buffer, num);
}

ssize_t write_(Socket* this, char* text)
{
    return write(this->_internals->fd, text, strlen(text));
}

void close_(Socket* this)
{
    (void) this;
    if (close(socket_._internals->fd) == -1)
        fprintf(stderr, "my_curl: error when closing socket: %s\n", strerror(errno));
}
