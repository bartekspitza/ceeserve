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
#include "logger.h"

void __log(const char* msg) {
    logger("staticserver", msg);
}

void handle_malformed(int socket, char* request);

void handle_conn(int socket, struct sockaddr_in client_addr) {
    size_t buffer_size = 2048;
    char requestraw[buffer_size];
    memset(requestraw, 0, buffer_size);
    int n = read(socket, requestraw, buffer_size - 1);
    if (n < 0) error("ERROR reading from socket");

    char client[200], logmsg[2048];
    strcpy(client, inet_ntoa(client_addr.sin_addr));

    HttpRequest req;
    int res = http_request_parse(requestraw, &req);
    if (http_request_parse(requestraw, &req) == -1) {
        sprintf(logmsg, "%s MALFORMED", client);
        handle_malformed(socket, requestraw);
    } else {
        sprintf(logmsg, "%s %s %s %s", client, req.method, req.path, req.version);
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

    __log(logmsg);
    close(socket);
}

void handle_malformed(int socket, char* request) {
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
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}
