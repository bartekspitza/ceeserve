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

void send_response(int socket, int status, char* statusdesc);
char* readFileIntoMemory(const char* filename, size_t* length);

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
    if (res == -1) {
        sprintf(logmsg, "%s MALFORMED", client);
        send_response(socket, 400, "Bad Request");
    } else if (strcmp(req.method, "GET") != 0) {
        puts("zxcv");
        puts(req.method);
        puts("zxcv");
        sprintf(logmsg, "%s %s %s %s", client, req.method, req.path, req.version);
        send_response(socket, 400, "Bad Request");
    } else {
        puts("here");
        sprintf(logmsg, "%s %s %s %s", client, req.method, req.path, req.version);
        char *fname;
        char *path = req.path;
        path++; // Skip first /

        if (strlen(path) == 0) {
            fname = "index.html";
        } else {
            fname = path;
        }

        char *fcontent = readFileIntoMemory(fname, NULL);
        if (fcontent == NULL) {
            send_response(socket, 404, "Not Found");
        } else {
            HttpResponse resp = {
                .version = "HTTP/1.1",
                .status_code = 200,
                .status_desc = "OK",
                .headers = NULL,
                .header_count = 0,
                .body = fcontent,
            };

            char resptext[sizeof(resp)*2];
            resptostr(resp, resptext);
            send(socket, resptext, strlen(resptext),0); 
        }
    }

    __log(logmsg);
    close(socket);
}

void send_response(int socket, int status, char *statusdesc) {
    HttpResponse resp = {
        .version = "HTTP/1.1",
        .status_code = status,
        .status_desc = statusdesc,
        .header_count = 0,
        .headers = NULL,
        .body = NULL,
    };

    char resptext[sizeof(resp)*2];
    resptostr(resp, resptext);
    send(socket, resptext, strlen(resptext), 0);
}

char* readFileIntoMemory(const char* filename, size_t* length) {
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

void error(const char *msg) {
    perror(msg);
    exit(1);
}
