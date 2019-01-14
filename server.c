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
#define BIND_PORT 9876

typedef enum
{
    G_CIRCLE = 0,
    G_CROSS  = 1
} player_t_;

typedef enum
{
    WIN_CIRCLE_         = -1,
    DRAW_               =  0,
    WIN_CROSS_          =  1,
    JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE_    =  2
} result_t_;

typedef enum
{
    NONE    = 0,
    CIRCLE_   = 1,
    CROSS_ = 2
} move_t;

typedef struct
{
    uint8_t moves[3][3];
} board_t;


int sign_move_n(board_t *board, player_t_ player, int place_number_x, int place_number_y)
{
    if (board->moves[place_number_x][place_number_y] != NONE)
        return 0;
    board->moves[place_number_x][place_number_y] = (player == G_CIRCLE) ? CIRCLE : CROSS;
    return 1;
}

int sign_move(board_t *board, player_t player, int x_number, int y_number)
{
    return sign_move_n(board, player, x_number, y_number);
}

int remove_move_n(board_t *board, int place_number_x, int place_number_y)
{
    if (board->moves[place_number_x][place_number_y] == NONE)
        return 0;
    board->moves[place_number_x][place_number_y] = NONE;
    return 1;

}

void change_player(player_t_ * player)
{
    *player = ((*player == G_CIRCLE) ? G_CROSS : G_CIRCLE);
}

result_t result_game(board_t *board)
{
    //Checking verses
    for (int y=0; y<3; y++)
    {
        for(int j=0; j<3; j++) {
            if ((board->moves[y][j] == board->moves[y][j+1]) && (board->moves[y][j] == board->moves[y][j+ 2])) {
                if (board->moves[y][j] == CIRCLE_)
                    return WIN_CIRCLE;
                if (board->moves[y][j] == CROSS_)
                    return WIN_CROSS;
            }
        }
    }
    //Checking columns
    for (int x=0; x<3; x++)
    {
        for(int j=0; j<3; j++) {
            if ((board->moves[x][j] == board->moves[x + 1][j]) && (board->moves[x][j] == board->moves[x + 2][j])) {
                if (board->moves[x][j] == CIRCLE_)
                    return WIN_CIRCLE;
                if (board->moves[x][j] == CROSS_)
                    return WIN_CROSS;
            }
        }
    }
    //Checking diagonals
    if ((board->moves[0][0] == board->moves[1][1]) && (board->moves[0][0] == board->moves[2][2]))
    {
        if (board->moves[1][1] == CIRCLE_)
            return WIN_CIRCLE;
        if (board->moves[1][1] == CROSS_)
            return WIN_CROSS;
    }
    if ((board->moves[0][2] == board->moves[1][1]) && (board->moves[0][2] == board->moves[2][0]))
    {
        if (board->moves[1][1] == CIRCLE_)
            return WIN_CIRCLE;
        if (board->moves[1][1] == CROSS_)
            return WIN_CROSS;
    }
    //Checking posibility of move

    for (int i=0; i<3; i++) {
        for(int j=0; j<3; j++) {
            if (board->moves[i][j] == NONE)
                return JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE;
        }
    }
    return DRAW;
}

int check_result(player_t_ player, result_t_ result)
{
    if (result == DRAW) return 0;

    if (((player == G_CIRCLE) && (result == WIN_CIRCLE)) || ((player == G_CROSS) && (WIN_CROSS)))
        return 1;

    return -1;
}


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
    player_t_ player;
    int n1;
    int n2;

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
    board_t moves;

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
        int recieve;
        if (FD_ISSET(fdListen, &rfds)) {
            // handle new connection
            int i;
            for(int i = 0; i < MAX_GAMES; ++i){
                if(games[i].circle_player_fd == -1){
                    games[i].circle_player_fd = accept(fdListen, NULL, NULL);
                    recieve = recv(games[i].circle_player_fd, buf, 255, 0);
                    if(recieve<1)
                    {
                        printf("CONNECTION ERROR --> EXITING\n");
                        close(games[i].circle_player_fd);
                        exit(0);
                    }
                    struct msg *message = (struct msg *) buf;
                    switch(message->type)
                    {
                        case JOIN:
                            strcpy(games->circle_player_nickname,message->join.nickname);
                            break;
                        case CHAT:
                            message->type = CHAT;
                            strcpy(message->chat.msg,buf);
                            message->len = strlen(message->chat.msg)+21+HDR_SIZE;
                            if(send(games[i].cross_player_fd, message, message->len, 0) == -1){
                                perror("Socket send error\n");
                                exit(-1);
                            }
                            break;
                        case MOVE:
                            player = G_CIRCLE;
                            if(result_game(&moves)!=JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE)
                            {
                                if(result_game(&moves)==DRAW_)
                                {
                                    message->type = FINISH;
                                    message->finish.result = DRAW;
                                    message->len = strlen(message->finish.result)+1+HDR_SIZE;
                                    if((send(games[i].cross_player_fd, message, message->len, 0)&& send(games[i].circle_player_fd,message,message->len,0)) == -1){
                                        perror("Socket send error\n");
                                        exit(-1);
                                    }
                                    close(games[i].cross_player_fd);
                                    close(games[i].circle_player_fd);
                                    exit(0);

                                }
                                else if(result_game(&moves)==WIN_CIRCLE_)
                                {
                                        message->type = FINISH;
                                        message->finish.result = WIN_CIRCLE;
                                        message->len = strlen(message->finish.result)+1+HDR_SIZE;
                                        if((send(games[i].cross_player_fd, message, message->len, 0)&& send(games[i].circle_player_fd,message,message->len,0)) == -1){
                                            perror("Socket send error\n");
                                            exit(-1);
                                        }
                                        close(games[i].cross_player_fd);
                                        close(games[i].circle_player_fd);
                                        exit(0);
                                }
                                else if(result_game(&moves)==WIN_CROSS_)
                                {
                                        message->type = FINISH;
                                        message->finish.result = WIN_CROSS;
                                        message->len = strlen(message->finish.result)+1+HDR_SIZE;
                                        if((send(games[i].cross_player_fd, message, message->len, 0)&& send(games[i].circle_player_fd,message,message->len,0)) == -1){
                                            perror("Socket send error\n");
                                            exit(-1);
                                        }
                                        close(games[i].cross_player_fd);
                                        close(games[i].circle_player_fd);
                                        exit(0);
                                }
                            }
                            else
                            {
                                n1 = message->move.x;
                                n2 = message->move.y;
                                if(sign_move(&moves,player,n1,n2) == 0)
                                {
                                    moves.moves[n1][n2] = 1;
                                    message->type = MOVE_YOUR_ASS;
                                    message->move_your_ass.you = 1;
                                    message->len = strlen(message->move_your_ass.you)+1+HDR_SIZE;
                                    if(send(games[i].cross_player_fd, message, message->len, 0) == -1){
                                        perror("Socket send error\n");
                                        exit(-1);
                                    }
                                    message->type = MOVE;
                                    message->move.x = n1;
                                    message->move.y = n2;
                                    message->len = 3+HDR_SIZE;
                                    if(send(games[i].cross_player_fd, message, message->len, 0) == -1){
                                        perror("Socket send error\n");
                                        exit(-1);
                                    }
                                }
                                else
                                {
                                    message->type = MOVE_YOUR_ASS;
                                    message->move_your_ass.you = 2;
                                    message->len = strlen(message->move_your_ass.you)+1+HDR_SIZE;
                                    if(send(games[i].circle_player_fd, message, message->len, 0) == -1){
                                        perror("Socket send error\n");
                                        exit(-1);
                                    }
                                }
                            }
                            break;
                    }

                    break;
                } else if(games[i].cross_player_fd == -1){
                    games[i].cross_player_fd = accept(fdListen, NULL, NULL);
                    recieve = recv(games[i].cross_player_fd, buf, 255, 0);
                    if(recieve<1)
                    {
                        printf("CONNECTION ERROR --> EXITING\n");
                        close(games[i].cross_player_fd);
                        exit(0);
                    }
                    struct msg *message = (struct msg *) buf;
                    switch(message->type)
                    {
                        case JOIN:
                            strcpy(games->cross_player_nickname,message->join.nickname);
                            break;
                        case CHAT:
                            message->type = CHAT;
                            strcpy(message->chat.msg,buf);
                            message->len = strlen(message->chat.msg)+21+HDR_SIZE;
                            if(send(games[i].circle_player_fd, message, message->len, 0) == -1){
                                perror("Socket send error\n");
                                exit(-1);
                            }
                            break;
                        case MOVE:
                            player = G_CROSS;
                            if(result_game(&moves)!=JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE)
                            {
                                if(result_game(&moves)==DRAW_)
                                {
                                    message->type = FINISH;
                                    message->finish.result = DRAW;
                                    message->len = strlen(message->finish.result)+1+HDR_SIZE;
                                    if((send(games[i].cross_player_fd, message, message->len, 0)&& send(games[i].circle_player_fd,message,message->len,0)) == -1){
                                        perror("Socket send error\n");
                                        exit(-1);
                                    }
                                    close(games[i].cross_player_fd);
                                    close(games[i].circle_player_fd);
                                    exit(0);

                                }
                                else if(result_game(&moves)==WIN_CIRCLE_)
                                {
                                    message->type = FINISH;
                                    message->finish.result = WIN_CIRCLE;
                                    message->len = strlen(message->finish.result)+1+HDR_SIZE;
                                    if((send(games[i].cross_player_fd, message, message->len, 0)&& send(games[i].circle_player_fd,message,message->len,0)) == -1){
                                        perror("Socket send error\n");
                                        exit(-1);
                                    }
                                    close(games[i].cross_player_fd);
                                    close(games[i].circle_player_fd);
                                    exit(0);
                                }
                                else if(result_game(&moves)==WIN_CROSS_)
                                {
                                    message->type = FINISH;
                                    message->finish.result = WIN_CROSS;
                                    message->len = strlen(message->finish.result)+1+HDR_SIZE;
                                    if((send(games[i].cross_player_fd, message, message->len, 0)&& send(games[i].circle_player_fd,message,message->len,0)) == -1){
                                        perror("Socket send error\n");
                                        exit(-1);
                                    }
                                    close(games[i].cross_player_fd);
                                    close(games[i].circle_player_fd);
                                    exit(0);
                                }
                            }
                            else
                            {
                                n1 = message->move.x;
                                n2 = message->move.y;
                                if(sign_move(&moves,player,n1,n2) == 0)
                                {
                                    moves.moves[n1][n2] = 1;
                                    message->type = MOVE_YOUR_ASS;
                                    message->move_your_ass.you = 1;
                                    message->len = strlen(message->move_your_ass.you)+1+HDR_SIZE;
                                    if(send(games[i].circle_player_fd, message, message->len, 0) == -1){
                                        perror("Socket send error\n");
                                        exit(-1);
                                    }
                                    message->type = MOVE;
                                    message->move.x = n1;
                                    message->move.y = n2;
                                    message->len = 3+HDR_SIZE;
                                    if(send(games[i].cross_player_fd, message, message->len, 0) == -1){
                                        perror("Socket send error\n");
                                        exit(-1);
                                    }
                                }
                                else
                                {
                                    message->type = MOVE_YOUR_ASS;
                                    message->move_your_ass.you = 2;
                                    message->len = strlen(message->move_your_ass.you)+1+HDR_SIZE;
                                    if(send(games[i].cross_player_fd, message, message->len, 0) == -1){
                                        perror("Socket send error\n");
                                        exit(-1);
                                    }
                                }
                            }
                            break;
                    }
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
