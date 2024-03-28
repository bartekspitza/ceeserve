#ifndef HTTP_H
#define HTTP_H

typedef struct {
    char *key;
    char *value;
} HttpHeader;

typedef struct {
    char *method;
    char *path;
    char *version;
    HttpHeader *headers;
    int header_count;
    char *body;
} HttpRequest;

typedef struct {
    char *version;
    char *status_desc;
    char *body;
    long body_length;
    int status_code;
    HttpHeader *headers;
    int header_count;
} HttpResponse;

int http_request_parse(const char *request, HttpRequest* req);

char* resptostr(HttpResponse resp, long *bytes);

#endif