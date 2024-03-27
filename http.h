#ifndef HTTP_H
#define HTTP_H

typedef struct {
    char *key;
    char *value;
} http_header_t;

typedef struct {
    char method[10];
    char path[1024];
    char version[10];
    http_header_t *headers;
    int header_count;
    char *body;
} http_request_t;

int http_request_parse(const char *request, http_request_t* req);

#endif