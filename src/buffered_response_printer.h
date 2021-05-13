#ifndef MY_CURL_BUFFERED_RESPONSE_PRINTER_H
#define MY_CURL_BUFFERED_RESPONSE_PRINTER_H

#include "socket.h"
#include <stdbool.h>

typedef struct buffered_response_printer BufferedResponsePrinter;
struct buffered_response_printer {
    void (*print)();
};

extern const struct buffered_response_printer_class {
    BufferedResponsePrinter* (*fromSocket)(Socket* socket);
} BufferedResponsePrinterClass;

#endif
