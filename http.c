#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int request_line(const char* request, HttpRequest* req, char* cursor) {

    char method[50];
    char path[2048];
    char version[50];
    int res = sscanf(request, "%49s %2047s %49s", method, path, version);

    if (res != 3 || strlen(method) == 0 || strlen(path) == 0 || strlen(version) == 0) {
        return -1;
    }

    req->method = strdup(method);
    req->path = strdup(path);
    req->version = strdup(version);
    return 0;
}

int parse_headers_only(const char* request, HttpRequest* req) {
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

/*
Parses the request. Returns 0 if good, -1 if protocol failure
*/
int parse_headers(const char *request, HttpRequest *req) {
    char *startpos = (char*) request;

    if (request_line(request, req, startpos) == -1) {
        return -1;
    }

    if (parse_headers_only(request, req) == -1) {
        return -1;
    }

    return 0;
}

HttpHeader* get_header(HttpRequest request, char* key) {
    for (int i = 0; i < request.header_count; i++) {
        char* hkey = request.headers[i].key;
        if (strcmp(hkey, key) == 0) {
            return &request.headers[i];
        }
    }

    return NULL;
}

char* resptostr(HttpResponse resp) {
    char tmp[4096];
    char *ptmp = tmp;

    // Response line
    sprintf(ptmp, "%s %d %s\r\n", resp.version, resp.status_code, resp.status_desc);
    ptmp += strlen(ptmp);

    // Any headers
    for (int i = 0; i < resp.header_count; i++) {
        HttpHeader hdr = resp.headers[i];
        sprintf(ptmp, "%s: %s\r\n", hdr.key, hdr.value);
        ptmp += strlen(ptmp);
    }
    // End of headers
    strcpy(ptmp, "\r\n");

    return strdup(tmp);
}
