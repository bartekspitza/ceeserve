#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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

char* send_file(const char* filename);
void log_request(struct sockaddr_in client, const HttpRequest *req, const HttpResponse *response);
void create_file_response(const char* filename, HttpResponse* response);
HttpResponse create_response(int status, char *statusdesc, char *body);
void add_header(HttpResponse *response, char* key, char* value);

const size_t bufsize = 4096;

/**
 * Num bytes of where last char of \r\n\r\n is encountered, -1 if not found
*/
int end_of_headers(char* data) {
    for (int i = 0; i < bufsize-4; i++) {
        if (data[i]=='\r' && data[i+1]=='\n' && data[i+2]=='\r' && data[i+3]=='\n') {
            return i+4;
        }
    }

    return -1;
}

/**
 * HTTP/1.1 Connection, persistent. Headers max size is 4k.
*/
void handle_conn(int socket, struct sockaddr_in client_addr) {
    HttpRequest req;
    HttpResponse resp;
    char data[bufsize];
    int bytes_read = 0;
    bool reading_headers = true;

    while (1) {
        // Read correct amount from socket
        int bytes_to_read = bufsize - bytes_read;
        int bytes = read(socket, data, bufsize);
        if (bytes < 0) exit(1);
        bytes_read += bytes;
        
        // See if we are done reading headers
        if (reading_headers) {
            int eof = end_of_headers(data);
            if (eof == -1) continue;
            reading_headers = false;
            if (parse_headers(data, &req) == -1) exit(1);
        }

        // Buffer is full
        if (bytes_read == bufsize) {
            // handle this
        }




        int res = parse_headers(data, &req);

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

            create_file_response(fname, &resp);
        }

        log_request(client_addr, &req, &resp);
        long bytes; 
        char *respdata = resptostr(resp, &bytes);

        send(socket, respdata, bytes, 0); 
        sleep(1000);
    }
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

void create_file_response(const char* filename, HttpResponse* response) {
    response->version = "HTTP/1.1";
    response->headers = malloc(sizeof(HttpHeader) * 100);
    response->header_count = 0;

    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        response->status_code = 404;
        response->status_desc = "Not Found";
        return;
    }

    // Find size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Content-Length
    char* clength = malloc(sizeof(char)*30);
    sprintf(clength, "%ld", fileSize);
    add_header(response, "Content-Length", clength);

    // Read the file into the buffer
    char* fcontent = malloc(fileSize);
    size_t bytesRead = fread(fcontent, 1, fileSize, file);
    fclose(file);
    if (bytesRead < fileSize) {
        response->status_code = 500;
        response->status_desc = "Internal Server Error";
        return;
    }

    if (strcmp(filename, "image.png") == 0) {
        add_header(response, "Content-Type", "image/png");
    }

    response->body_length = fileSize;
    response->body = fcontent;
    response->status_code = 200;
    response->status_desc = "OK";
}

/*
This assumes there is memory allocated for the headers array
*/
void add_header(HttpResponse *response, char* key, char* value) {
    response->headers[response->header_count].key = key;
    response->headers[response->header_count].value = value;
    response->header_count++;
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
