#ifndef MY_CURL_HTTP_EXCHANGE_H
#define MY_CURL_HTTP_EXCHANGE_H

#include "htparse.h"

typedef struct http_exchange HttpExchange;
struct http_exchange {
    struct internals* _internals;
    void (*send)(HttpExchange* this);
    void (*printResponse)(HttpExchange* this);
    void (*close)(HttpExchange* this);
};

extern const struct http_exchange_class {
    HttpExchange* (*fromUri)(Uri* uri);
} HttpExchangeClass;

#endif
