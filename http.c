#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Internal funcs
int __parse_headers(const char* request, HttpRequest* req);
int __request_line(const char* request, HttpRequest* req, char* cursor);
// Internal funcs

void resptostr(HttpResponse resp, char *str) {
    sprintf(str, "%s %d %s\r\n", resp.version, resp.status_code, resp.status_desc);
    str += strlen(str);

    for (int i = 0; i < resp.header_count; i++) {
        HttpHeader hdr = resp.headers[i];
        sprintf(str, "%s: %s\r\n", hdr.key, hdr.value);
        str += strlen(str);
    }

    strcpy(str, "\r\n");
}

/*
Parses the request. Returns 0 if good, -1 if protocol failure
*/
int http_request_parse(const char *request, HttpRequest *req) {
    char *startpos = (char*) request;

    if (__request_line(request, req, startpos) == -1) {
        return -1;
    }

    if (__parse_headers(request, req) == -1) {
        return -1;
    }

    return 0;
}

int __request_line(const char* request, HttpRequest* req, char* cursor) {
    int i = 0;
    char c;
    int part = 0;
    int partidx = 0;
    while ((c = request[i++]) != '\0') {
        if (c == ' ') {
            part++; 
            partidx = 0;
            continue;
        } else if (c == '\r') {
            continue;
        } else if (c == '\n') {
            cursor += i+1;
            break;
        }

        if (part == 0) {
            req->method[partidx] = c;
        } else if (part == 1) {
            req->path[partidx] = c;
        } else if (part == 2) {
            req->version[partidx] = c;
        }

        partidx++;
    }

    return part == 2 ? 0 : -1;
}

int __parse_headers(const char* request, HttpRequest* req) {
    HttpHeader *ar = malloc(sizeof(HttpHeader) * 1000);
    memset(ar, 0, sizeof(HttpHeader) * 1000);

    int currline = 0;
    int lastOffset = 0;
    bool firstColon = false;
    char *currPointer = NULL;
    bool encounteredblank = false;

    char c;
    int i = 0;
    while ((c = request[i]) != '\0') {
        if (c == '\n') {
            currline++;
            i++;

            if (request[i] == '\r' && request[i+1] == '\n') { // blank line, headers end
                encounteredblank = true;
                break; 
            } else {
                ar[currline-1].key = (char*) malloc(sizeof(char)*256);
                ar[currline-1].value = (char*) malloc(sizeof(char)*256);
                currPointer = ar[currline-1].key;
                lastOffset = i;
                firstColon = false;
                continue;
            }
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
    
    if (!encounteredblank) return -1;

    req->header_count = currline-1;
    req->headers = realloc(ar, sizeof(HttpRequest)*req->header_count);

    return 0;
}