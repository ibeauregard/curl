#include "buffered_response_printer.h"
#include "headers.h"
#include "math.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define CAPACITY 1 << 13 // 8 k

static BufferedResponsePrinter* from_socket(Socket* socket);
const struct buffered_response_printer_class BufferedResponsePrinterClass = {
        .fromSocket = &from_socket
};

static struct internals {
    Socket* socket;
    char buffer[CAPACITY];
    size_t index;
    size_t length;
    size_t capacity;
} printer = {
        .socket = NULL,
        .buffer = {0},
        .index = 0,
        .length = 0,
        .capacity = CAPACITY
};

static void print();
static BufferedResponsePrinter printer_interface = {
        .print = &print
};

BufferedResponsePrinter* from_socket(Socket* socket)
{
    printer.socket = socket;
    return &printer_interface;
}

static bool is_full();
static size_t load();
static char* get_raw_headers();
static void print_chunked_transfer();
static void print_chunk(size_t chunk_size);
void print()
{
    char* raw_headers = NULL;
    while (!raw_headers && !is_full()) {
        load();
        raw_headers = get_raw_headers();
    }
    if (!raw_headers) {
        fprintf(stderr, "%s\n", "my_curl: Can't parse headers; too large");
    } else {
        Headers* headers = HeadersClass.fromRawString(raw_headers);
        if (headers->transfer_encoding == CHUNKED) {
            print_chunked_transfer();
        } else {
            print_chunk(headers->content_length);
        }
    }
}

void print_chunked_transfer()
{
    fprintf(stderr, "%s\n", "my_curl: Transfer-Encoding header not yet supported; Content-Length must be specified");
}

static bool is_empty();
static size_t unload();
void print_chunk(size_t chunk_size)
{
    size_t write_count = 0, read_count = printer.length;
    int socket_fd = printer.socket->getFd();
    fd_set read_set, write_set;
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    while (write_count < chunk_size) {
        FD_SET(socket_fd, &read_set);
        FD_SET(STDOUT_FILENO, &write_set);
        select(socket_fd + 1, &read_set, &write_set, NULL, NULL);
        if (!is_empty() && FD_ISSET(STDOUT_FILENO, &write_set)) {
            write_count += unload();
        } if (read_count < chunk_size && !is_full() && FD_ISSET(socket_fd, &read_set)) {
            read_count += load();
        }
    }
}

size_t load()
{
    size_t free_capacity = printer.capacity - printer.length;
    size_t insert_index = (printer.index + printer.length) % printer.capacity;
    size_t read_potential = min(free_capacity, printer.capacity - insert_index);
    size_t read_count = read(printer.socket->getFd(), &printer.buffer[insert_index], read_potential);
    if (read_count == read_potential && read_potential < free_capacity) {
        read_count += read(printer.socket->getFd(), printer.buffer, free_capacity - read_potential);
    }
    printer.length += read_count;
    return read_count;
}

size_t unload()
{
    size_t print_potential = printer.index + printer.length <= printer.capacity ?
                             printer.length : printer.capacity - printer.index;
    size_t print_count = write(STDOUT_FILENO, &printer.buffer[printer.index], print_potential);
    if (print_count == print_potential && print_potential < printer.length) {
        print_count += write(STDOUT_FILENO, printer.buffer, printer.length - print_potential);
    }
    printer.index = (printer.index + print_count) % printer.capacity;
    printer.length -= print_count;
    return print_count;
}

char* get_raw_headers()
{
    static size_t body_index = 4; // length of "\r\n\r\n"
    for (; body_index <= printer.length; body_index++) {
        if (!strncmp(&printer.buffer[body_index - 4], "\r\n\r\n", 4)) {
            break;
        }
    }
    if (body_index > printer.length) return NULL;
    printer.index = (printer.index + body_index) % printer.capacity;
    printer.length -= body_index;
    return strndup(printer.buffer, body_index);
}

bool is_empty()
{
    return printer.length == 0;
}

bool is_full()
{
    return printer.length == printer.capacity;
}
