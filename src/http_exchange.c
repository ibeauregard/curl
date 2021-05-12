#include "http_exchange.h"
#include "socket.h"
#include "buffered_response_printer.h"

#include <stdbool.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define CONTENT_LENGTH_LABEL "content-length:"

static void make_exchange(Uri* uri);
const struct http_exchange HttpExchange = {
        .withUri = &make_exchange
};

static void send_request();
static void print_response();
static void close_();
static struct exchange {
    Socket* socket;
    Uri* uri;
    bool failed;
} exchange = {
        .socket = NULL,
        .uri = NULL,
        .failed = false
};

void make_exchange(Uri* uri)
{
    exchange.uri = uri;
    send_request();
    print_response();
    close_();
}

static void create_connected_socket();
static void write_to_socket();
void send_request()
{
    create_connected_socket();
    if (!exchange.failed) write_to_socket();
}

static void print_response_();
void print_response()
{
    if (!exchange.failed) print_response_();
}

static size_t get_content_length(BufferedResponsePrinter* printer);
void print_response_()
{
    BufferedResponsePrinter* printer = BufferedResponsePrinterClass.fromSocket(exchange.socket);
    printf("%zu\n", get_content_length(printer));
    if (exchange.failed) return;
}

static void attempt_connection(struct addrinfo* addr_list);
void create_connected_socket()
{
    static const struct addrinfo hints = {
            .ai_family = AF_UNSPEC,
            .ai_socktype = SOCK_STREAM,
            .ai_protocol = IPPROTO_TCP
    };
    struct addrinfo* addr_info;
    int getaddrinfo_return_value;
    Uri* uri = exchange.uri;
    if ((getaddrinfo_return_value = getaddrinfo(uri->host, uri->port ? uri->port : "80", &hints, &addr_info)) != 0) {
        fprintf(stderr, "my_curl: %s\n", gai_strerror(getaddrinfo_return_value));
        exchange.failed = true;
        return;
    }
    attempt_connection(addr_info);
    if (!exchange.socket) {
        fprintf(stderr, "Could not connect to %s\n", uri->host);
        exchange.failed = true;
    }
}

void write_to_socket()
{
    char request_text[BUFSIZ];
    Uri* uri = exchange.uri;
    int num_printed = snprintf(request_text, BUFSIZ,
                               "GET %s HTTP/1.1\r\n"
                               "Host: %s\r\n\r\n",
                               uri->resource && strlen(uri->resource) ? uri->resource : "/", uri->host);
    if (num_printed == -1) {
        fprintf(stderr, "%s\n", "my_curl: encoding error in snprintf");
        exchange.failed = true;
    } else if (num_printed == BUFSIZ) {
        fprintf(stderr, "%s\n", "my_curl: cannot withUri a get request: URI too long");
        exchange.failed = true;
    }
    Socket* socket = exchange.socket;
    if (socket->write(request_text) == -1) {
        fprintf(stderr, "my_curl: error from write: %s\n", strerror(errno));
        exchange.failed = true;
    }
}

void attempt_connection(struct addrinfo* addr_list)
{
    for (struct addrinfo* addr_info = addr_list; addr_info; addr_info = addr_info->ai_next) {
        Socket* socket = SocketClass.fromAddrInfo(addr_info);
        if (!socket) continue;
        if (socket->connect(addr_info) != -1) {
            exchange.socket = socket;
            break;
        }
        socket->close(socket);
    }
    freeaddrinfo(addr_list);
}

static size_t parse_headers_for_content_length(char* headers);
size_t get_content_length(BufferedResponsePrinter* printer)
{
    char* headers = NULL;
    while (!headers && !printer->isFull()) {
        printer->load(printer);
        headers = printer->getHeaders();
    }
    if (!headers && printer->isFull()) {
        fprintf(stderr, "%s\n", "my_curl: Can't parse headers; too large");
        exchange.failed = true;
    }
    return parse_headers_for_content_length(headers);
}

size_t parse_headers_for_content_length(char* headers)
{
    static size_t label_length = strlen(CONTENT_LENGTH_LABEL);
    size_t content_length_index, header_length = strlen(headers);
    for (content_length_index = label_length; content_length_index < header_length; content_length_index++) {
        if (!strncasecmp(&headers[content_length_index - label_length], CONTENT_LENGTH_LABEL, label_length)) {
            break;
        }
    }
    if (content_length_index == header_length) {
        fprintf(stderr, "%s\n", "my_curl doesn't support chunked encoding; headers have to specify content length");
        exchange.failed = true;
    }
    long content_length = strtol(&headers[content_length_index], NULL, 10);
    free(headers);
    return content_length;
}

void close_()
{
    if (exchange.socket) exchange.socket->close(exchange.socket);
}
