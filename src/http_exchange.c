#include "http_exchange.h"
#include "socket.h"
#include "buffered_socket_printer.h"

#include <stdbool.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

static HttpExchange* from_uri(Uri* uri);
const struct http_exchange_class HttpExchangeClass = {
        .fromUri = &from_uri
};

static struct internals {
    Socket* socket;
    Uri* uri;
    bool failed;
} internals = {
        .socket = NULL,
        .uri = NULL,
        .failed = false
};

static void send_(HttpExchange* this);
static void print_response(HttpExchange* this);
static void close_(HttpExchange* this);
static HttpExchange exchange = {
        ._internals = &internals,
        .send = &send_,
        .printResponse = &print_response,
        .close = &close_
};

HttpExchange* from_uri(Uri* uri)
{
    exchange._internals->uri = uri;
    return &exchange;
}

static void create_connected_socket(HttpExchange* this);
static void write_to_socket(HttpExchange* this);
void send_(HttpExchange* this)
{
    create_connected_socket(this);
    if (!this->_internals->failed) write_to_socket(this);
}

static void print_response_(HttpExchange* this);
void print_response(HttpExchange* this)
{
    if (!this->_internals->failed) print_response_(this);
}

void print_response_(HttpExchange* this)
{
    char print_buffer[BUFSIZ];
    size_t content_length = 1256;
    ssize_t num_read;
    Socket* socket = this->_internals->socket;
    for (size_t i = 0; i < content_length; i += num_read) {
        if ((num_read = socket->read(socket, print_buffer, BUFSIZ)) == -1) {
            fprintf(stderr, "my_curl: error from read: %s\n", strerror(errno));
            break;
        }
        ssize_t num_written = write(STDOUT_FILENO, print_buffer, num_read);
        if (num_written != num_read) {
            fprintf(stderr, "my_curl: error while writing to stdout %s\n",
                    num_written == -1 ? strerror(errno) : "");
        }
    }
}

static void attempt_connection(HttpExchange* this, struct addrinfo* addr_list);
void create_connected_socket(HttpExchange* this)
{
    static const struct addrinfo hints = {
            .ai_family = AF_UNSPEC,
            .ai_socktype = SOCK_STREAM,
            .ai_protocol = IPPROTO_TCP
    };
    struct addrinfo* addr_info;
    int getaddrinfo_return_value;
    Uri* uri = this->_internals->uri;
    if ((getaddrinfo_return_value = getaddrinfo(uri->host, uri->port ? uri->port : "80", &hints, &addr_info)) != 0) {
        fprintf(stderr, "my_curl: %s\n", gai_strerror(getaddrinfo_return_value));
        this->_internals->failed = true;
        return;
    }
    attempt_connection(this, addr_info);
    if (!this->_internals->socket) {
        fprintf(stderr, "Could not connect to %s\n", uri->host);
        this->_internals->failed = true;
    }
}

void write_to_socket(HttpExchange* this)
{
    char request_text[BUFSIZ];
    Uri* uri = this->_internals->uri;
    int num_printed = snprintf(request_text, BUFSIZ,
                               "GET %s HTTP/1.1\r\n"
                               "Host: %s\r\n\r\n", uri->resource ? uri->resource : "/", uri->host);
    if (num_printed == -1) {
        fprintf(stderr, "%s\n", "my_curl: encoding error in snprintf");
        this->_internals->failed = true;
    } else if (num_printed == BUFSIZ) {
        fprintf(stderr, "%s\n", "my_curl: cannot make a get request: URI too long");
        this->_internals->failed = true;
    }
    Socket* socket = this->_internals->socket;
    if (socket->write(socket, request_text) == -1) {
        fprintf(stderr, "my_curl: error from write: %s\n", strerror(errno));
        this->_internals->failed = true;
    }
}

void attempt_connection(HttpExchange* this, struct addrinfo* addr_list)
{
    for (struct addrinfo* addr_info = addr_list; addr_info; addr_info = addr_info->ai_next) {
        Socket* socket = SocketClass.fromAddrInfo(addr_info);
        if (!socket) continue;
        if (socket->connect(socket, addr_info) != -1) {
            this->_internals->socket = socket;
            break;
        }
        socket->close(socket);
    }
    freeaddrinfo(addr_list);
}

void close_(HttpExchange* this)
{
    (void) this;
    if (internals.socket) internals.socket->close(internals.socket);
}
