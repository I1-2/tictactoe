#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
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
    uint8_t moves_counter;
} game_t;

bool send_chat_msg(int fd, char nickname[20], char msg[160]){
    struct msg message;
    message.type = CHAT;
    strcpy(message.chat.msg, msg);
    strcpy(message.chat.nickname, nickname);
    message.len = HDR_SIZE + strlen(message.chat.msg) + 21; // null terminator and nickname
    printf("Sending chat message to fd %d: nickname=%s, msg=%s", fd, nickname, msg);
    return send(fd, &message, message.len, 0) == message.len;
    // return true if message sent and false if not
}

bool send_move_msg(int fd, uint8_t x, uint8_t y, figure_t player){
    struct msg message;
    message.type = MOVE;
    message.len = HDR_SIZE + 3;
    message.move.x = x;
    message.move.y = y;
    message.move.player = player;
    printf("Sending MOVE message to fd %d: x=%d, y=%d, player=%d", fd, x, y, player);
    return send(fd, &message, message.len, 0) == message.len;
}

bool send_move_your_ass_msg(int fd, figure_t player){
    struct msg message;
    message.type = MOVE_YOUR_ASS;
    message.len = HDR_SIZE + 1;
    message.move_your_ass.you = player;
    printf("Sending MOVE_YOUR_ASS message to fd %d: you=%d", fd, player);
    return send(fd, &message, message.len, 0) == message.len;
}

bool send_finish_msg(int fd, result_t result){
    struct msg message;
    message.type = FINISH;
    message.len = HDR_SIZE + 1;
    message.finish.result = result;
    printf("Sending FINISH message to fd %d: result=%d", fd, result);
    return send(fd, &message, message.len, 0) == message.len;
}

void clean_game_up(game_t* game){
    memset(game, 0, sizeof(*game));
    shutdown(game->players[CIRCLE].fd, 0);
    game->players[CIRCLE].fd = -1;
    shutdown(game->players[CROSS].fd, 0);
    game->players[CROSS].fd = -1;
}

result_t result_game(board_t *board);

int main() {
    int one = 1;
    game_t games[MAX_GAMES];

    for(int i = 0; i < MAX_GAMES; ++i){
        clean_game_up(&(games[i]));
    }

    int fd_listen;

    if ((fd_listen = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    setsockopt(fd_listen, SOL_TCP, TCP_NODELAY, &one, sizeof(one));

    struct sockaddr_in si_local;
    bzero(&si_local, sizeof(struct sockaddr_in));
    si_local.sin_family = AF_INET;
    si_local.sin_port = htons(BIND_PORT);
    if (bind(fd_listen, (const struct sockaddr *) &si_local, sizeof(si_local)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    listen(fd_listen, MAX_CONNECTIONS);
    printf("Server listening for new connections\n");

    // fd set with fds to be watched for read
    fd_set rfds;

    char buf[BUFLEN];

    #pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {
        int max_fd = fd_listen;

        FD_ZERO(&rfds);
        FD_SET(fd_listen, &rfds);

        for(int i = 0; i < MAX_GAMES; ++i){
            for (int figure = 1; figure <= 2; ++figure) {
                if (games[i].players[figure].fd != -1) {
                    max_fd = (games[i].players[figure].fd > max_fd) ? games[i].players[figure].fd : max_fd;
                    FD_SET(games[i].players[figure].fd, &rfds);
                }
            }
        }

        select(max_fd+1, &rfds, NULL, NULL, NULL);

        if (FD_ISSET(fd_listen, &rfds)) {
            // handle new connection
            printf("New incomming connection\n");
            int i;
            for(i = 0; i < MAX_GAMES; ++i){
                if(games[i].players[CIRCLE].fd == -1){
                    games[i].players[CIRCLE].fd = accept(fd_listen, NULL, NULL);
                    setsockopt(games[i].players[CIRCLE].fd, SOL_TCP, TCP_NODELAY, &one, sizeof(one));
                    // if game is full, start it
                    if(games[i].players[CROSS].fd != -1){
                        if(!send_move_your_ass_msg(games[i].players[CIRCLE].fd, CIRCLE)){
                            perror("Sending through socket failed");
                            exit(EXIT_FAILURE);
                        }
                        games[i].player_whose_turn_is_now = CIRCLE;
                    }
                    break;
                } else if(games[i].players[CROSS].fd == -1){
                    games[i].players[CROSS].fd = accept(fd_listen, NULL, NULL);
                    setsockopt(games[i].players[CROSS].fd, SOL_TCP, TCP_NODELAY, &one, sizeof(one));
                    // if game is full, start it
                    if(games[i].players[CIRCLE].fd != -1){
                        if(!send_move_your_ass_msg(games[i].players[CIRCLE].fd, CIRCLE)){
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
                shutdown(accept(fd_listen, NULL, NULL), 0);
            }
        }

        // handle incomming messages/disconnections
        for(int i = 0; i < MAX_GAMES; ++i) {
            for (int player_figure = 1; player_figure <= 2; ++player_figure) {
                if (FD_ISSET(games[i].players[player_figure].fd, &rfds)) {
                    game_t *game = &games[i];
                    player_t *player = &game->players[player_figure];

                    int len = recv(player->fd, buf, BUFLEN, 0);

                    if(len <= 0){
                        printf("Disconnecting fd: %d\n", player->fd);
                        // handling disconnection or read error
                        if(!send_finish_msg(game->players[player_figure == CIRCLE ? CROSS : CIRCLE].fd, JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE)){
                            perror("Sending through socket failed");
                            exit(EXIT_FAILURE);
                        };

                        clean_game_up(game);
                    }

                    struct msg *recved_msg = (struct msg*) buf;
                    printf("Got message to fd %d with type: %d\n", player->fd, recved_msg->type);
                    switch (recved_msg->type){
                        case JOIN:
                            strcpy(player->nickname, recved_msg->join.nickname);
                            break;
                        case CHAT:
                            for (int dest_player_figure = 1; dest_player_figure <= 2; ++dest_player_figure){
                                if(!send_chat_msg(game->players[dest_player_figure].fd, player->nickname, recved_msg->chat.msg)){
                                    perror("Sending through socket failed");
                                    exit(EXIT_FAILURE);
                                }
                            }
                            break;
                        case MOVE:
                            if(player != &(game->players[game->player_whose_turn_is_now])) {
                                // player tried to do his move on not his turn
                                if (!send_chat_msg(player->fd, "INFO", "That was not your turn motherducker..")) {
                                    perror("Sending through socket failed");
                                    exit(EXIT_FAILURE);
                                }
                                break;
                            }
                            if(recved_msg->move.x > 2 || recved_msg->move.x < 0 || recved_msg->move.y > 2 || recved_msg->move.y < 0) {
                                // player tried to move off the board
                                if (!send_chat_msg(player->fd, "INFO", "Idiot.. You tried to move off the board..")) {
                                    perror("Sending through socket failed");
                                    exit(EXIT_FAILURE);
                                }
                                break;
                            }
                            if(game->board.moves[recved_msg->move.x][recved_msg->move.y] != NONE_FIGURE) {
                                // player tried to move to occupied field
                                if (!send_chat_msg(player->fd, "INFO", "Idiot.. You tried to move to occupied field")) {
                                    perror("Sending through socket failed");
                                    exit(EXIT_FAILURE);
                                }
                                break;
                            }
                            // do actual move
                            game->board.moves[recved_msg->move.x][recved_msg->move.y] = game->player_whose_turn_is_now;
                            ++(game->moves_counter);

                            for (int dest_player_figure = 1; dest_player_figure <= 2; ++dest_player_figure) {
                                if(!send_move_msg(game->players[dest_player_figure].fd, recved_msg->move.x, recved_msg->move.y, game->player_whose_turn_is_now)){
                                    perror("Sending through socket failed");
                                    exit(EXIT_FAILURE);
                                }
                            }

                            // switch players
                            game->player_whose_turn_is_now = game->player_whose_turn_is_now == CIRCLE ? CROSS : CIRCLE;

                            result_t result = result_game(&(game->board));
                            if(result == JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE && game->moves_counter>=9)
                                result = DRAW;
                            if(result != JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE){
                                for (int dest_player_figure = 1; dest_player_figure <= 2; ++dest_player_figure) {
                                    if(!send_finish_msg(game->players[dest_player_figure].fd, result)){
                                        perror("Sending through socket failed");
                                        exit(EXIT_FAILURE);
                                    }
                                }

                                clean_game_up(game);
                                break;
                            }

                            if(!send_move_your_ass_msg(game->players[game->player_whose_turn_is_now].fd, game->player_whose_turn_is_now)){
                                perror("Sending through socket failed");
                                exit(EXIT_FAILURE);
                            }

                            break;
                        default:
                            perror("Got unknown message");
                            exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
}

result_t result_game(board_t *board) {
    //Checking verses
    for (int y = 0; y < 3; y++) {
        if ((board->moves[y][0] == board->moves[y][1]) && (board->moves[y][0] == board->moves[y][2])) {
            if (board->moves[y][0] == CIRCLE)
                return WIN_CIRCLE;
            if (board->moves[y][0] == CROSS)
                return WIN_CROSS;
        }
    }
    //Checking columns
    for (int j = 0; j < 3; j++) {
        if ((board->moves[0][j] == board->moves[1][j]) && (board->moves[0][j] == board->moves[2][j])) {
            if (board->moves[0][j] == CIRCLE)
                return WIN_CIRCLE;
            if (board->moves[0][j] == CROSS)
                return WIN_CROSS;
        }
    }
    //Checking diagonals
    if ((board->moves[0][0] == board->moves[1][1]) && (board->moves[0][0] == board->moves[2][2])) {
        if (board->moves[1][1] == CIRCLE)
            return WIN_CIRCLE;
        if (board->moves[1][1] == CROSS)
            return WIN_CROSS;
    }
    if ((board->moves[0][2] == board->moves[1][1]) && (board->moves[0][2] == board->moves[2][0])) {
        if (board->moves[1][1] == CIRCLE)
            return WIN_CIRCLE;
        if (board->moves[1][1] == CROSS)
            return WIN_CROSS;
    }

    return JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE;
}