#ifndef HTTP_H
#define HTTP_H

typedef struct {
    char *key;
    char *value;
} HttpHeader;

typedef struct {
    char method[10];
    char path[1024];
    char version[10];
    HttpHeader *headers;
    int header_count;
    char *body;
} HttpRequest;

typedef struct {
    char version[10];
    int status_code;
    char status_desc[30];
    HttpHeader *headers;
    int header_count;
    char *body;
} HttpResponse;

int http_request_parse(const char *request, HttpRequest* req);

void resptostr(HttpResponse response, char* str);

#endif