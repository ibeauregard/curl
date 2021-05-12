#ifndef MY_CURL_BUFFERED_SOCKET_PRINTER_H
#define MY_CURL_BUFFERED_SOCKET_PRINTER_H

#include "socket.h"

typedef struct buffered_socket_printer BufferedSocketPrinter;
struct buffered_socket_printer {
    struct internals* _internals;
    ssize_t (*load)(BufferedSocketPrinter* this);
    ssize_t (*unload)(BufferedSocketPrinter* this);
};

extern const struct buffered_socket_printer_class {
    BufferedSocketPrinter* (*fromSocket)(Socket* socket);
} BufferedSocketPrinterClass;

#endif
