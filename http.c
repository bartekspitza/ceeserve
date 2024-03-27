#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Internal funcs
void __parse_headers(const char* request, int *hcount, http_header_t* ar);
// Internal funcs

http_request_t http_request_parse(const char *request) {
    char method[16], path[256], protocol[16];

    http_request_t req;

    // parse start line
    sscanf(request, "%s %s %s", method, path, protocol);
    req.method = method;
    req.path = path;
    req.protocol = protocol;

    http_header_t *headers = malloc(sizeof(http_header_t) * 1000);
    __parse_headers(request, &req.header_count, headers);
    req.headers = realloc(headers, sizeof(http_header_t)*req.header_count);

    return req;
}

void __parse_headers(const char* request, int *hcount, http_header_t* ar) {
    int currline = 0;
    int i = 0;
    int lineoffset = 0;
    char c;
    char key[256];
    char value[256];
    bool readingKey = true;
    bool firstColon = false;

    while ((c = request[i]) != '\0') {
        if (c == '\n') {
            if (currline >= 1 && strlen(key) > 0 && strcmp("\r", key)) {
                ar[currline-1].key = strdup(key);
                ar[currline-1].value = strdup(value);
            }

            memset(key, 0, sizeof(key));
            memset(value, 0, sizeof(value));
            i++;
            currline++;
            lineoffset = i;
            readingKey = true;
            firstColon = false;
            continue;
        }

        if (c == ':' && !firstColon) {
            i++; // Always an extra space after the :, skip it
            readingKey = !readingKey;
            firstColon = true;
            continue;
        }

        if (readingKey) {
            key[i-lineoffset] = c;
        } else {
            value[i-(lineoffset+2+strlen(key))] = c;
        }

        i++;
    }

    *hcount = currline-2;
}