#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define true 1
#define false 0
#define bool int

#define BUFLEN 1024
#define MAX_CONNECTIONS 10

typedef struct {
    int circle_player_fd;
    char circle_player_nickname[20];
    int cross_player_fd;
    char cross_player_nickname[20];
} game_t;

int main() {
    int connections[MAX_CONNECTIONS];
    for (int i = 0; i < MAX_CONNECTIONS; ++i) connections[i] = -1;
    // -1 is inactive connection

    int port = 9876;

    int fdListen;

    if ((fdListen = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in si_local;
    bzero(&si_local, sizeof(struct sockaddr_in));
    si_local.sin_family = AF_INET;
    si_local.sin_port = htons(port);
    if (bind(fdListen, (const struct sockaddr *) &si_local, sizeof(si_local)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    listen(fdListen, MAX_CONNECTIONS);

    // fd set with fds to be watched for read
    fd_set rfds;

    char buf[BUFLEN];

    while (true) {

        int maxFd = fdListen;

        FD_ZERO(&rfds);
        FD_SET(fdListen, &rfds);

        for (int i = 0; i < MAX_CONNECTIONS; ++i) {
            if (connections[i] != -1) {
                maxFd = (connections[i] > maxFd) ? connections[i] : maxFd;
                FD_SET(connections[i], &rfds);
            }
        }
    }
}
