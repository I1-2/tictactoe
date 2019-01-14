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
#define bool uint8_t

#define BUFLEN 255
#define MAX_CONNECTIONS 10
#define MAX_GAMES MAX_CONNECTIONS/2
#define BIND_PORT 9876

typedef struct {
    int fd;
    char nickname[20];
} player_t;


typedef struct {
    player_t players[3];
    uint8_t player_whose_turn_is_now;
    board_t board;
} game_t;

bool send_chat_msg(int fd, char nickname[20], char msg[160]){
    struct msg message;
    message.type = CHAT;
    strcpy(message.chat.msg, msg);
    strcpy(message.chat.nickname, nickname);
    message.len = HDR_SIZE + strlen(message.chat.msg) + 21; // null terminator and nickname
    return send(fd, &message, message.len, 0) != message.len;
    // return true if message sent and false if not
}

int main() {
    game_t games[MAX_GAMES];
    memset(&games, 0, sizeof(games));

    for(int i = 0; i < MAX_GAMES; ++i){
        games[i].players[CIRCLE].fd = -1;
        games[i].players[CROSS].fd = -1;
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

    #pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {
        int maxFd = fdListen;

        FD_ZERO(&rfds);
        FD_SET(fdListen, &rfds);

        for(int i = 0; i < MAX_GAMES; ++i){
            if(games[i].players[CIRCLE].fd != -1){
                maxFd = (games[i].players[CIRCLE].fd > maxFd) ? games[i].players[CIRCLE].fd : maxFd;
                FD_SET(games[i].players[CIRCLE].fd, &rfds);
            }
            if(games[i].players[CROSS].fd != -1){
                maxFd = (games[i].players[CROSS].fd != -1 > maxFd) ? games[i].players[CROSS].fd != -1 : maxFd;
                FD_SET(games[i].players[CROSS].fd != -1, &rfds);
            }
        }

        if (FD_ISSET(fdListen, &rfds)) {
            // handle new connection
            int i;
            for(i = 0; i < MAX_GAMES; ++i){
                if(games[i].players[CIRCLE].fd == -1){
                    games[i].players[CIRCLE].fd = accept(fdListen, NULL, NULL);
                    if(games[i].players[CROSS].fd != -1){
                        struct msg message;
                        message.move_your_ass.you = CIRCLE;
                        message.type = MOVE_YOUR_ASS;
                        message.len = HDR_SIZE+1;
                        if(send(games[i].players[CIRCLE].fd, &message, message.len, 0) != message.len){
                            perror("Sending through socket failed");
                            exit(EXIT_FAILURE);
                        }
                        games[i].player_whose_turn_is_now = CIRCLE;
                    }
                    break;
                } else if(games[i].players[CROSS].fd == -1){
                    games[i].players[CROSS].fd = accept(fdListen, NULL, NULL);
                    if(games[i].players[CIRCLE].fd != -1){
                        struct msg message;
                        message.move_your_ass.you = CIRCLE;
                        message.type = MOVE_YOUR_ASS;
                        message.len = HDR_SIZE+1;
                        if(send(games[i].players[CIRCLE].fd, &message, message.len, 0) != message.len){
                            perror("Sending through socket failed");
                            exit(EXIT_FAILURE);
                        }
                        games[i].player_whose_turn_is_now = CIRCLE;
                    }
                    break;
                }
            }
            if (i == MAX_GAMES) {
                // no games available, server is full, disconnecting
                close(accept(fdListen, NULL, NULL));
            }
        }

        // handle incomming messages/disconnections
        for(int i = 0; i < MAX_GAMES; ++i) {
            for (int player_figure = 1; player_figure <= 2; ++player_figure) {
                if (FD_ISSET(games[i].players[player_figure].fd, &rfds)) {
                    game_t game = games[i];
                    player_t player = game.players[player_figure];

                    int len = recv(player.fd, &buf, BUFLEN, 0);

                    // if(len == 0)
                    // TODO

                    struct msg *recved_msg = (struct msg*) &buf;
                    switch (recved_msg->type){
                        case JOIN:
                            strcpy(player.nickname, recved_msg->join.nickname);
                            break;
                        case CHAT:
                            for (int dest_player_figure = 1; dest_player_figure <= 2; ++dest_player_figure){
                                if(!send_chat_msg(game.players[dest_player_figure].fd, player.nickname, recved_msg->chat.msg)){
                                    perror("Sending through socket failed");
                                    exit(EXIT_FAILURE);
                                }
                            }
                            break;
                        case MOVE:
                            if(&player != &(game.players[game.player_whose_turn_is_now])) {
                                // player tried to do his move on not his turn
                                if (!send_chat_msg(player.fd, "INFO", "That was not your turn motherducker..")) {
                                    perror("Sending through socket failed");
                                    exit(EXIT_FAILURE);
                                }
                                break;
                            }
                            if(recved_msg->move.x > 2 || recved_msg->move.x < 0 || recved_msg->move.y > 2 || recved_msg->move.y < 0) {
                                // player tried to move off the board
                                if (!send_chat_msg(player.fd, "INFO", "Idiot.. You tried to move off the board..")) {
                                    perror("Sending through socket failed");
                                    exit(EXIT_FAILURE);
                                }
                                break;
                            }
                            if(game.board.moves[recved_msg->move.x][recved_msg->move.y] != NONE_FIGURE) {
                                // player tried to move to occupied field
                                if (!send_chat_msg(player.fd, "INFO", "Idiot.. You tried to move to occupied field")) {
                                    perror("Sending through socket failed");
                                    exit(EXIT_FAILURE);
                                }
                                break;
                            }
                            // do actual move
                            game.board.moves[recved_msg->move.x][recved_msg->move.y] = game.player_whose_turn_is_now;
                            for (int dest_player_figure = 1; dest_player_figure <= 2; ++dest_player_figure) {
                                struct msg message;
                                message.type = MOVE;
                                message.len = HDR_SIZE + 3;
                                message.move.x = recved_msg->move.x;
                                message.move.y = recved_msg->move.y;
                                message.move.player = game.player_whose_turn_is_now;

                                if(send(game.players[dest_player_figure].fd, &message, message.len, 0) != message.len){
                                    perror("Sending through socket failed");
                                    exit(EXIT_FAILURE);
                                }
                            }

                            game.player_whose_turn_is_now = game.player_whose_turn_is_now == CIRCLE ? CROSS : CIRCLE;

                            // TODO: check if it isn't finish of the game

                            struct msg message;
                            message.type = MOVE_YOUR_ASS;
                            message.len = HDR_SIZE + 1;
                            message.move_your_ass.you = game.player_whose_turn_is_now;

                            if(send(game.players[game.player_whose_turn_is_now].fd, &message, message.len, 0) != message.len){
                                perror("Sending through socket failed");
                                exit(EXIT_FAILURE);
                            }



                        default:
                            perror("Got unknown message");
                            exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
}