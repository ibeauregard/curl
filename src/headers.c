#include "headers.h"
#include "math.h"
#include <string.h>
#include <stdlib.h>

#define CONTENT_LENGTH_LABEL "content-length:"
#define CHUNKED_ENCODING_LABEL "transfer-encoding:"

static Headers* from_raw_string(char* raw_headers);
const struct headers_class HeadersClass = {
        .fromRawString = &from_raw_string
};

static Headers headers = {
        .transfer_encoding = REGULAR,
        .content_length = 0
};

Headers* from_raw_string(char* raw_headers)
{
    static size_t content_length_len = 15, // strlen(CONTENT_LENGTH_LABEL)
                chunked_encoding_len = 18; // strlen(CHUNKED_ENCODING_LABEL);
    size_t header_len = strlen(raw_headers);
    for (size_t i = min(content_length_len, chunked_encoding_len); i < header_len; i++) {
        if (i >= content_length_len && !strncasecmp(&raw_headers[i - content_length_len],
                                                    CONTENT_LENGTH_LABEL, content_length_len)) {
            headers.content_length = strtol(&raw_headers[i], NULL, 10);
            break;
        } if (i >= chunked_encoding_len && !strncasecmp(&raw_headers[i - chunked_encoding_len],
                                                        CHUNKED_ENCODING_LABEL, chunked_encoding_len)) {
            headers.transfer_encoding = CHUNKED;
            break;
        }
    }
    free(raw_headers);
    return &headers;
}
