#include "my_curl.h"
#include "htparse.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>

static bool parse_uri(char* raw_uri, Uri* uri);
static int get_connected_socket(Uri* uri);
static void make_exchange(Uri* uri, int socket_fd);
static void close_socket(int socket_fd);
int my_curl(char* raw_uri)
{
    Uri uri;
    int socket_fd = -1;
    if (parse_uri(raw_uri, &uri)) socket_fd = get_connected_socket(&uri);
    if (socket_fd != -1) make_exchange(&uri, socket_fd);
    if (socket_fd != -1) close_socket(socket_fd);
    return EXIT_SUCCESS;
}

bool parse_uri(char* raw_uri, Uri* uri)
{
    scan(raw_uri, uri);
    if (uri->scheme && strcmp(uri->scheme, "http") != 0) {
        fprintf(stderr, "%s\n", "my_curl only supports http");
        return false;
    }
    if (uri->port && strcmp(uri->port, "80") != 0) {
        fprintf(stderr, "%s\n", "my_curl only supports http: if specified, the port should be 80");
        return false;
    }
    return true;
}

static int attempt_connection(struct addrinfo** addr_list);
int get_connected_socket(Uri* uri)
{
    struct addrinfo *result, hints = {
            .ai_family = AF_UNSPEC,
            .ai_socktype = SOCK_STREAM,
            .ai_protocol = IPPROTO_TCP
    };
    int getaddrinfo_return_value;
    if ((getaddrinfo_return_value = getaddrinfo(uri->host, uri->port ? uri->port : "80", &hints, &result)) != 0) {
        fprintf(stderr, "my_curl: error from getaddrinfo: %s\n", gai_strerror(getaddrinfo_return_value));
        return -1;
    }
    int socket_fd = attempt_connection(&result);
    if (socket_fd == -1) fprintf(stderr, "Could not connect to %s\n", uri->host);
    return socket_fd;
}

static ssize_t make_get_request(Uri* uri, int socket_fd);
static void print_response(int socket_fd);
void make_exchange(Uri* uri, int socket_fd)
{
    if (make_get_request(uri, socket_fd) != -1) print_response(socket_fd);
}

void close_socket(int socket_fd)
{
    if (close(socket_fd) == -1) {
        fprintf(stderr, "my_curl: error when closing socket: %s\n", strerror(errno));
    }
}

int attempt_connection(struct addrinfo** addr_list)
{
    int socket_fd;
    struct addrinfo* result_ptr;
    for (result_ptr = *addr_list; result_ptr; result_ptr = result_ptr->ai_next) {
        socket_fd = socket(result_ptr->ai_family, result_ptr->ai_socktype, result_ptr->ai_protocol);
        if (socket_fd == -1) continue;
        if (connect(socket_fd, result_ptr->ai_addr, result_ptr->ai_addrlen) != -1) break;
        close(socket_fd);
    }
    freeaddrinfo(*addr_list);
    return result_ptr ? socket_fd : -1;
}

ssize_t make_get_request(Uri* uri, int socket_fd)
{
    char request_text[BUFSIZ];
    int num_printed = snprintf(request_text, BUFSIZ,
                                   "GET %s HTTP/1.1\r\n"
                                   "Host: %s\r\n\r\n", uri->resource ? uri->resource : "/", uri->host);
    if (num_printed == -1) {
        fprintf(stderr, "%s\n", "my_curl: encoding error in snprintf");
        return -1;
    } else if (num_printed == BUFSIZ) {
        fprintf(stderr, "%s\n", "my_curl: cannot make a get request: URI too long");
        return -1;
    }

    size_t request_text_length = strlen(request_text);
    ssize_t num_written = write(socket_fd, request_text, request_text_length);
    if (num_written == -1) fprintf(stderr, "my_curl: error from write: %s\n", strerror(errno));
    return num_written;
}

static size_t get_content_length(int socket_fd);
void print_response(int socket_fd)
{
    char print_buffer[BUFSIZ];
    size_t content_length = get_content_length(socket_fd);
    ssize_t num_read;
    for (size_t i = 0; i < content_length; i += num_read) {
        if ((num_read = read(socket_fd, print_buffer, BUFSIZ)) == -1) {
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

size_t get_content_length(int socket_fd)
{
    (void) socket_fd;
    size_t content_length = 1256;


    return content_length;
}
