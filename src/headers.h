#ifndef MY_CURL_HEADERS_H
#define MY_CURL_HEADERS_H

#include <stddef.h>

typedef enum transfer_encoding {
    REGULAR,
    CHUNKED
} TransferEncoding;

typedef struct {
    TransferEncoding transfer_encoding;
    size_t content_length;
} Headers;

extern const struct headers_class {
    Headers* (*fromRawString)(char* raw_headers);
} HeadersClass;

#endif
