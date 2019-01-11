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

#include "message.h"

#define true 1
#define false 0
#define bool int

#define BUFLEN 255
#define MAX_CONNECTIONS 10
#define MAX_GAMES MAX_CONNECTIONS/2
#define BIND_PORT = 9876;

typedef struct {
    int circle_player_fd;
    char circle_player_nickname[20];
    int cross_player_fd;
    char cross_player_nickname[20];
    player_t turn;
} game_t;

int main() {
    game_t games[MAX_GAMES];
    memset(&games, 0, sizeof(games));

    for(int i = 0; i < MAX_GAMES; ++i){
        games[i].circle_player_fd = -1;
        games[i].cross_player_fd = -1;
    }

    int fdListen;

    if ((fdListen = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in si_local;
    bzero(&si_local, sizeof(struct sockaddr_in));
    si_local.sin_family = AF_INET;
    si_local.sin_port = htons(BIND_PORT);
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

        for(int i = 0; i < MAX_GAMES; ++i){
            if(games[i].circle_player_fd != -1){
                maxFd = (games[i].circle_player_fd > maxFd) ? games[i].circle_player_fd : maxFd;
                FD_SET(games[i].circle_player_fd, &rfds);
            }
            if(games[i].cross_player_fd != -1){
                maxFd = (games[i].cross_player_fd != -1 > maxFd) ? games[i].cross_player_fd != -1 : maxFd;
                FD_SET(games[i].cross_player_fd != -1, &rfds);
            }
        }

        if (FD_ISSET(fdListen, &rfds)) {
            // handle new connection
            int i;
            for(int i = 0; i < MAX_GAMES; ++i){
                if(games[i].circle_player_fd == -1){
                    games[i].circle_player_fd = accept(fdListen, NULL, NULL);
                    // TODO
                    break;
                } else if(games[i].cross_player_fd == -1){
                    games[i].cross_player_fd = accept(fdListen, NULL, NULL);
                    // TODO
                    break;
                }
            }
            if (i == MAX_GAMES) {
                // no games available, server is full
                close(accept(fdListen, NULL, NULL));
            }
        }

        // TODO: handle incomming messages/disconnections
    }
}
