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

const size_t BSIZE = 4096;
const char* FILES_FOLDER = "files";

/**
 * Num bytes of where last char of \r\n\r\n is encountered, -1 if not found
*/
int end_of_headers(char* data) {
    for (int i = 0; i < BSIZE-4; i++) {
        if (data[i]=='\r' && data[i+1]=='\n' && data[i+2]=='\r' && data[i+3]=='\n') {
            return i+4;
        }
    }

    return -1;
}

void send_response(int socket, HttpResponse resp) {
    char *data = resptostr(resp);
    send(socket, data, strlen(data), 0); 
}

void serve_file(int socket, HttpRequest req) {
    char *path = req.path;
    path++; // Skip leading '/'

    char *fname;
    if (strlen(path) == 0) {
        fname = "index.html";
    } else {
        fname = path;
    }

    // Open file
    FILE* file = fopen(fname, "rb");
    if (file == NULL) {
        HttpResponse resp = create_response(404, "Not Found", NULL);
        resp.headers = malloc(sizeof(HttpHeader));
        add_header(&resp, "Content-Length", "0");
        send_response(socket, resp);
        return;
    }

    // Find size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Content-Length
    char* clength = malloc(sizeof(char)*30);
    sprintf(clength, "%ld", fileSize);

    // Send start of response
    HttpResponse resp = create_response(200, "OK", NULL);
    resp.headers = malloc(sizeof(HttpHeader)*1000);
    resp.header_count = 0;
    add_header(&resp, "Content-Length", clength);

    if (strcmp(fname, "image.png") == 0) {
        add_header(&resp, "Content-Type", "image/png");
    }
    if (strncmp(fname, FILES_FOLDER, strlen(FILES_FOLDER)) == 0) {
        add_header(&resp, "Content-Disposition", "attachment");
    }

    send_response(socket, resp);

    // Send body
    while (feof(file) == 0) {
        char buffer[BSIZE];
        size_t bytesread = fread(buffer, 1, BSIZE, file);
        send(socket, buffer, bytesread, 0);
    }
}

/**
 * HTTP/1.1 Connection, persistent. Headers max size is 4k.
*/
void handle_conn(int socket, struct sockaddr_in client_addr) {
    HttpRequest req;
    HttpResponse resp;
    char data[BSIZE];
    int bytes_read = 0;
    bool reading_headers = true;

    while (1) {
        // Read correct amount from socket
        int bytes_to_read = BSIZE - bytes_read;
        int bytes = read(socket, data, bytes_to_read);
        if (bytes <= 0) exit(1);
        bytes_read += bytes;
        
        // See if we are done reading headers, if buffer is full here we're kinda fked
        int body_offset;
        if (reading_headers) {
            body_offset = end_of_headers(data);
            if (body_offset == -1) continue;
            reading_headers = false;
            if (parse_headers(data, &req) == -1) exit(1);
            log_request(client_addr, &req, &resp);
        }

        if (strcmp(req.method, "GET") == 0) { // Assume GET never has a body
            serve_file(socket, req);
            memset(data, 0, BSIZE);
            memset(&req, 0, BSIZE);
            memset(&resp, 0, BSIZE);
            reading_headers = true;
            bytes_read = 0;
        } else if (strcmp(req.method, "POST")) {
            //TODO
            // Allow upload of arbitrary files
            // HttpHeader* cl = get_header(req, "Content-Length");
        }
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

