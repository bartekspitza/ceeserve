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

/**
 * Parses the data until the blank line is encountered, i.e. the request line
 * and all the headers.
*/
int parse_headers(const char *data, HttpRequest* req);

HttpHeader* get_header(HttpRequest request, char* key);

char* resptostr(HttpResponse resp, long *bytes);

#endif