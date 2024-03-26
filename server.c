#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    printf("PID: %d\n", getpid());

    // Create a socket
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0) {
        error("ERROR opening socket");
    }

    // Bind the socket to an address
    int port = 8080;
    struct sockaddr_in listen_addr = {0};
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(port);
    int bindres = bind(listen_socket, (struct sockaddr *) &listen_addr, sizeof(listen_addr));
    if (bindres < 0) {
        printf("ERROR on binding with errno: %d\n", errno);
        exit(1);
    }

    // Listen
    listen(listen_socket, 5);
    printf("Listening on %d\n", port);

    // Accept
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int serving_socket = accept(listen_socket, (struct sockaddr*) &client_addr, &client_len);
    if (serving_socket < 0) {
        printf("Error: %d\n", errno);
        exit(1);
    } else {
        printf("Accepted connection from %s\n", inet_ntoa(client_addr.sin_addr));
    }

    // Read
    char buffer[256];
    memset(buffer,0,256);
    int n = read(serving_socket, &buffer, 255);
    if (n < 0) error("ERROR reading from socket");

    printf("Here is the message: %s\n",buffer);
    close(serving_socket);
    close(listen_socket);

    return 0; 
}
