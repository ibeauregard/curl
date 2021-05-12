#ifndef MY_CURL_HTTP_EXCHANGE_H
#define MY_CURL_HTTP_EXCHANGE_H

#include "htparse.h"

extern const struct http_exchange {
    void (*withUri)(Uri* uri);
} HttpExchange;

#endif
