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
    if (bind(listen_socket, (struct sockaddr *) &listen_addr, sizeof(listen_addr)) < 0) {
        error("ERROR on binding");
    }

    // Listen
    listen(listen_socket, 5);
    printf("Listening on %d\n", port);


    while(1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int serving_socket = accept(listen_socket, (struct sockaddr*) &client_addr, &client_len);
        if (serving_socket < 0) {
            perror("accept");
            continue;
        }

        int pid = fork();
        if (pid == 0) { // This is the child process
            //close(listen_socket); // Child doesn't need the listener
            printf("Accepted connection from %s\n", inet_ntoa(client_addr.sin_addr));

            char buffer[256];
            memset(buffer,0,256);
            int n = read(serving_socket, buffer, 255);
            if (n < 0) error("ERROR reading from socket");

            printf("Here is the message: %s\n", buffer);
            close(serving_socket);
            exit(0);
        } else if (pid < 0) {
            perror("fork");
            exit(1);
        } else {
            close(serving_socket);  // Parent doesn't need this
        }
    }

    close(listen_socket); // This line will never be reached in this example
    return 0; 
}
