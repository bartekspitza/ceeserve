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

char* read_file_into_mem(const char* filename);
void log_request(struct sockaddr_in client, const HttpRequest *req, const HttpResponse *response);
HttpResponse create_response(int status, char* statusdesc, char *body);

void handle_conn(int socket, struct sockaddr_in client_addr) {
    size_t buffer_size = 2048;
    char requestraw[buffer_size];
    int n = read(socket, requestraw, buffer_size-1);
    if (n < 0) error("ERROR reading from socket");
    requestraw[n]  = '\0';

    HttpRequest req;
    HttpResponse resp;
    int res = http_request_parse(requestraw, &req);
    if (res == -1) {
        memset(&req, 0, sizeof(HttpRequest));
        resp = create_response(400, "Bad Request", NULL);
    } else if (strcmp(req.method, "GET") != 0) {
        resp = create_response(400, "Bad Request", "Method not supported");
    } else {
        char *path = req.path;
        path++; // Skip leading '/'

        char *fname;
        if (strlen(path) == 0) {
            fname = "index.html";
        } else {
            fname = path;
        }

        char *fcontent = read_file_into_mem(fname);
        if (fcontent == NULL) {
            resp = create_response(404, "Not Found", NULL);
        } else {
            resp = create_response(200, "OK", fcontent);
        }
    }

    log_request(client_addr, &req, &resp);
    char resptext[5000];
    resptostr(resp, resptext);
    send(socket, resptext, strlen(resptext),0); 
    close(socket);
}

HttpResponse create_response(int status, char *statusdesc, char *body) {
    HttpResponse resp = {
        .version = "HTTP/1.1",
        .status_code = status,
        .status_desc = statusdesc,
        .body = body,
        .header_count = 0,
        .headers = NULL,
    };
    return resp;
}

char* read_file_into_mem(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        return NULL;
    }

    // Find size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(fileSize + 1);

    // Read the file into the buffer
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead < fileSize) {
        fclose(file);
        error("Failed to read the file entirely");
    }

    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}

void log_request(struct sockaddr_in client_addr, const HttpRequest *req, const HttpResponse *response) {
    char client[200];
    strcpy(client, inet_ntoa(client_addr.sin_addr));
    char logmsg[3000];
    sprintf(logmsg, "%s %s %s %d %s", req->version, client, req->method, response->status_code, req->path);
    __log(logmsg);
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}
