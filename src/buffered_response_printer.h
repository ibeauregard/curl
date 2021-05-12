#ifndef MY_CURL_BUFFERED_RESPONSE_PRINTER_H
#define MY_CURL_BUFFERED_RESPONSE_PRINTER_H

#include "socket.h"
#include <stdbool.h>

typedef struct buffered_response_printer BufferedResponsePrinter;
struct buffered_response_printer {
    struct internals* _internals;
    size_t (*load)();
    size_t (*unload)();
    bool (*isEmpty)();
    bool (*isFull)();
    char* (*getHeaders)();
};

extern const struct buffered_response_printer_class {
    BufferedResponsePrinter* (*fromSocket)(Socket* socket);
} BufferedResponsePrinterClass;

#endif
