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

void register_zombie_killer();
struct sockaddr_in listen_address(int port);

int main(int argc, char *argv[]) {
    printf("PID: %d\n", getpid());

    int port = atoi(argv[1]);
    char *path = argv[2];

    // Init
    int listen_socket;
    int reusesocket = 1;
    struct sockaddr_in listen_addr = listen_address(port);
    if (chdir(path) == -1) {
        error("Couldn't set web root");
    }
    if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        error("Error opening socket");
    }
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &reusesocket, sizeof(reusesocket)) == -1) {
        error("Error setsockopt");
    }
    if (bind(listen_socket, (struct sockaddr *) &listen_addr, sizeof(listen_addr)) == -1) {
        error("Error binding");
    }
    if (listen(listen_socket, 5) == -1) {
        error("Error listen");
    }

    register_zombie_killer();

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
            close(listen_socket);
            handle_conn(serving_socket, client_addr);
            exit(0);
        } else { // Parent
            close(serving_socket);
        }
    }

    close(listen_socket); // This line will never be reached in this example
    return 0; 
}

void sigchld_handler(int s) {
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void register_zombie_killer() {
    struct sigaction act = {0};
    act.sa_handler = sigchld_handler;
    act.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &act, NULL);
}

struct sockaddr_in listen_address(int port) {
    struct sockaddr_in listen_addr = {0};
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(port);
    return listen_addr;
}
