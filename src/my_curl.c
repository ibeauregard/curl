#include "my_curl.h"
#include "htparse.h"
#include "http_exchange.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

static bool parse_uri(char* raw_uri, Uri* uri);
int my_curl(char* raw_uri)
{
    Uri uri;
    if (!parse_uri(raw_uri, &uri)) return EXIT_FAILURE;
    HttpExchange exchange = HttpExchangeClass.fromUri(&uri);
    exchange.send(&exchange);
    exchange.printResponse(&exchange);
    exchange.delete(&exchange);
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
