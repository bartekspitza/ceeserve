#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Internal funcs
void __parse_headers(const char* request, http_request_t* req);
// Internal funcs

/*
Parses the request. Returns 0 if good, -1 if protocol failure
*/
int http_request_parse(const char *request, http_request_t *req) {
    char method[16], path[256], version[16];

    // parse start line
    sscanf(request, "%s %s %s", method, path, version);
    req->method = method;
    req->path = path;
    req->version = version;
    if (*req->method == '\0' || *req->path == '\0' || *req->version == '\0') {
        return -1;
    }

    __parse_headers(request, req);

    return 0;
}

void __parse_headers(const char* request, http_request_t* req) {
    http_header_t *ar = malloc(sizeof(http_header_t) * 1000);
    memset(ar, 0, sizeof(http_header_t) * 1000);

    int currline = 0;
    int lastOffset = 0;
    bool firstColon = false;
    char *currPointer = NULL;

    char c;
    int i = 0;
    while ((c = request[i]) != '\0') {
        if (c == '\n') {

            // Trim
            if (currPointer != NULL) {

            }

            currline++;
            ar[currline-1].key = (char*) malloc(sizeof(char)*256);
            ar[currline-1].value = (char*) malloc(sizeof(char)*256);
            currPointer = ar[currline-1].key;
            lastOffset = i+1;
            firstColon = false;
            i++;
            continue;
        }

        if (c == '\r' || currline == 0) {
            i++;
            continue;
        }

        if (c == ':' && !firstColon) {
            firstColon = true;
            currPointer = ar[currline-1].value;
            i++;
            if (request[i] == ' ') i++; // skip first whitespace if exists

            lastOffset = i;
            continue;
        }

        currPointer[i-lastOffset] = c;

        i++;
    }

    req->header_count = currline-1;
    req->headers = realloc(ar, sizeof(http_request_t)*req->header_count);
}