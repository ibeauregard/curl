#include "buffered_response_printer.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

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
} internals = {
        .socket = NULL,
        .buffer = {0},
        .index = 0,
        .length = 0,
        .capacity = CAPACITY
};

static size_t load();
static size_t unload();
static bool is_empty();
static bool is_full();
static char* get_headers();
static BufferedResponsePrinter printer = {
        ._internals = &internals,
        .load = &load,
        .unload = &unload,
        .isEmpty = &is_empty,
        .isFull = &is_full,
        .getHeaders = &get_headers
};

BufferedResponsePrinter* from_socket(Socket* socket)
{
    printer._internals->socket = socket;
    return &printer;
}

static size_t min(size_t n1, size_t n2);
size_t load()
{
    size_t free_capacity = internals.capacity - internals.length;
    size_t insert_index = (internals.index + internals.length) % internals.capacity;
    size_t read_potential = min(free_capacity, internals.capacity - insert_index);
    size_t num_read = read(internals.socket->getFd(), &internals.buffer[insert_index], read_potential);
    if (num_read == read_potential && read_potential < free_capacity) {
        num_read += read(internals.socket->getFd(), internals.buffer, free_capacity - read_potential);
    }
    internals.length += num_read;
    return num_read;
}

inline size_t min(size_t n1, size_t n2)
{
    return n1 <= n2 ? n1 : n2;
}

size_t unload()
{
    return 0;
}

bool is_empty()
{
    return internals.length == 0;
}

bool is_full()
{
    return internals.length == internals.capacity;
}

char* get_headers()
{
    static size_t body_index = 4; // length of "\r\n\r\n"
    for (; body_index <= internals.length; body_index++) {
        if (!strncmp(&internals.buffer[body_index - 4], "\r\n\r\n", 4)) {
            break;
        }
    }
    if (body_index > internals.length) return NULL;
    internals.index += body_index;
    internals.length -= body_index;
    return strndup(internals.buffer, body_index);
}
