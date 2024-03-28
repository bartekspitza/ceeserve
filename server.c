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

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Signal handler for SIGCHLD to clean up zombie processes
void sigchld_handler(int s) {
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[]) {
    printf("PID: %d\n", getpid());

    // Create a socket
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0) {
        error("ERROR opening socket");
    }

    // Set SO_REUSEADDR to allow immediate reuse of the port
    int reuse = 1;
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        error("setsockopt");
    }

    // Bind the socket to an address
    int port = 8080;
    struct sockaddr_in listen_addr = {0};
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(port);
    printf("Listening on %d\n", port);
    if (bind(listen_socket, (struct sockaddr *) &listen_addr, sizeof(listen_addr)) < 0) {
        error(NULL);
    }
    listen(listen_socket, 5);

    // Register signal handler for SIGCHLD to clean up zombie processes
    struct sigaction act = {0};
    act.sa_handler = sigchld_handler;
    act.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &act, NULL);

    // Accept connections
    while(1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int serving_socket = accept(listen_socket, (struct sockaddr*) &client_addr, &client_len);
        if (serving_socket < 0) {
            error("accept");
            continue;
        }

        int cpid = fork();
        if (cpid == -1) {
            error("fork");
            exit(1);
        } else if (cpid == 0) { // Child
            // Let child handle the connection
            close(listen_socket);
            printf("Accepted connection from %s\n", inet_ntoa(client_addr.sin_addr));

            size_t buffer_size = 2048;
            char buffer[buffer_size];
            memset(buffer, 0, buffer_size);
            int n = read(serving_socket, buffer, buffer_size - 1);
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
                send(serving_socket, resptext, strlen(resptext), 0);
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
                send(serving_socket, resptext, strlen(resptext), 0);
            }

            close(serving_socket);
            exit(0);
        } else { // Parent
            close(serving_socket);
        }
    }

    close(listen_socket); // This line will never be reached in this example
    return 0; 
}
