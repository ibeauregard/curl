#include "socket.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

static Socket* from_addr_info(struct addrinfo* addrInfo);
const struct socket_class SocketClass = {
        .fromAddrInfo = &from_addr_info
};

struct internals {
    int fd;
};

static int connect_(Socket* this, struct addrinfo* addr_info);
static ssize_t read_(Socket* this, char* buffer, size_t num);
static ssize_t write_(Socket* this, char* text);
static void delete(Socket** this);
Socket* from_addr_info(struct addrinfo* addrInfo)
{
    int socket_fd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (socket_fd == -1) return NULL;
    Socket* this = malloc(sizeof (Socket));
    this->_internals = malloc(sizeof (struct internals));
    this->_internals->fd = socket_fd;
    this->connect = &connect_;
    this->read = &read_;
    this->write = &write_;
    this->delete = &delete;
    return this;
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

void delete(Socket** this)
{
    if (close((*this)->_internals->fd) == -1)
        fprintf(stderr, "my_curl: error when closing socket: %s\n", strerror(errno));
    free((*this)->_internals);
    free(*this); *this = NULL;
}
