#include "buffered_socket_printer.h"

#include <stdio.h>

static BufferedSocketPrinter* from_socket(Socket* socket);
const struct buffered_socket_printer_class BufferedSocketPrinterClass = {
        .fromSocket = &from_socket
};

static struct internals {
    Socket* socket;
    char buffer[BUFSIZ];
    size_t index;
    size_t length;
} internals = {
        .socket = NULL,
        .buffer = {0},
        .index = 0,
        .length = 0
};

static ssize_t load(BufferedSocketPrinter* this);
static ssize_t unload(BufferedSocketPrinter* this);
static BufferedSocketPrinter printer = {
        ._internals = &internals,
        .load = &load,
        .unload = &unload
};

BufferedSocketPrinter* from_socket(Socket* socket)
{
    printer._internals->socket = socket;
    return &printer;
}

ssize_t load(BufferedSocketPrinter* this)
{
    (void) this;
    return 0;
}

ssize_t unload(BufferedSocketPrinter* this)
{
    (void) this;
    return 0;
}
