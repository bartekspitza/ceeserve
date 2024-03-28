#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include "http.h"
#include "static.h"

void handle_conn(int socket) {
    size_t buffer_size = 2048;
    char buffer[buffer_size];
    memset(buffer, 0, buffer_size);
    int n = read(socket, buffer, buffer_size - 1);
    puts(buffer);
    if (n < 0) error("ERROR reading from socket");

    HttpRequest req;
    int res = http_request_parse(buffer, &req);
    if (res == -1) {
        HttpResponse resp = {
            .version = "HTTP/1.1",
            .status_code = 400,
            .status_desc = "Bad Request",
            .headers = NULL,
            .header_count = 0,
            .body = NULL,
        };

        char resptext[1024];
        resptostr(resp, resptext);
        send(socket, resptext, strlen(resptext), 0);
    } else {
        HttpResponse resp = {
            .version = "HTTP/1.1",
            .status_code = 200,
            .status_desc = "OK",
            .headers = NULL,
            .header_count = 0,
            .body = "Hello World!",
        };
        HttpHeader h1 = {
            .key = "Host",
            .value = "localhost:8080",
        };
        HttpHeader ar[] = {h1};
        resp.header_count = 1;
        resp.headers = ar;

        char resptext[1024];
        resptostr(resp, resptext);
        send(socket, resptext, strlen(resptext), 0);
    }

    close(socket);
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}
