#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include "message.h"

#define true 1
#define false 0
#define bool uint8_t

#define SERVER "127.0.0.1"
#define SERVPORT 9876

#define BUFLEN 1024

typedef struct {
    uint8_t board[9];
} local_board_t;

int do_move(local_board_t *board, figure_t player, uint8_t *field);

bool send_move_msg(int fd, uint8_t x, uint8_t y, figure_t player){
    struct msg message;
    message.type = MOVE;
    message.len = HDR_SIZE + 3;
    message.move.x = x;
    message.move.y = y;
    message.move.player = player;
    printf("Sending MOVE message to fd %d: x=%d, y=%d, player=%d\n", fd, x, y, player);
    return send(fd, &message, message.len, 0) == message.len;
}

int main(int argc, char *argv[])
{
    int serverPort = SERVPORT;
    struct sockaddr_in serveraddr;
    char msg_buf[BUFLEN];
    char server[255];
    int unread;
    int sd = socket(AF_INET, SOCK_STREAM, 0); // socket descriptor
    if(sd < 0)
    {
        perror("Creating socket error");
        exit(-1);
    }

    if(argc > 1)
        strcpy(server, argv[1]);
    else
        strcpy(server, SERVER);

    if (argc > 2)
        serverPort = atoi(argv[2]);

    memset(&serveraddr, 0x00, sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(serverPort);

    if((serveraddr.sin_addr.s_addr = inet_addr(server)) == (unsigned long)INADDR_NONE){
        struct hostent *hostp = gethostbyname(server);
        if(hostp == (struct hostent *)NULL)
        {
            printf("404 Host not found \n");
            shutdown(sd,0);
            exit(-1);
        }
        memcpy(&serveraddr.sin_addr, hostp->h_addr, sizeof(serveraddr.sin_addr));
    }

    struct msg *message = (struct msg *) msg_buf;
    printf("Insert bot's nickname:\n");
    fgets(message->join.nickname, 20, stdin);
    message->join.nickname[strlen(message->join.nickname)-1] = 0; // remove \n from the end
    message->type = JOIN;
    message->len = strlen(message->join.nickname)+1+HDR_SIZE;

    if(connect(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0){
        perror("Connection error");
        shutdown(sd, 0);
        exit(EXIT_FAILURE);
    } else printf("Bot connected to server\n");

    if(send(sd, message, message->len, 0) == -1){
        perror("Socket send error");
        exit(EXIT_FAILURE);
    }

    figure_t player = NONE_FIGURE;
    local_board_t board;
    memset(board.board, NONE_FIGURE, 9);

    while(1){
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sd, &rfds);
        select(sd+1, &rfds, NULL, NULL, NULL);

        // handle networking
        unread = recv(sd, msg_buf, BUFLEN, 0);
        message = (struct msg *) msg_buf;
        if(unread < 1){
            perror("Connection error, exiting");
            shutdown(sd, 0);
            exit(0);
        }

        while(unread > 0){
            switch(message->type){
                case CHAT:
                    printf("[CHAT] %s: %s\n", message->chat.nickname, message->chat.msg);
                    break;
                case MOVE:
                    // no need to check if < 0, we use unsigned type
                    if(message->move.x > 2 || message->move.y > 2)
                        break;
                    board.board[3*message->move.y+message->move.x] = message->move.player;
                    break;
                case MOVE_YOUR_ASS:
                    if(player == NONE_FIGURE) {
                        player = message->move_your_ass.you;
                        if(player == CIRCLE) printf("I'm a circle!\n");
                        else printf("I'm a cross!\n");
                    }

                    uint8_t field = 0;
                    do_move(&board, player, &field);
                    if (!send_move_msg(sd, field % 3, field / 3, player)) {
                        perror("Sending through socket failed");
                        exit(EXIT_FAILURE);
                    }
                    break;
                case FINISH:
                    switch(message->finish.result){
                        case WIN_CIRCLE:
                            printf("Circle wins\n");
                            break;
                        case WIN_CROSS:
                            printf("Cross wins\n");
                            break;
                        case DRAW:
                            printf("Draw\n");
                            break;
                        case JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE:
                            printf("JEDEN RABIN POWIE TAK, DRUGI RABIN POWIE NIE\n");
                            break;
                    }
                    shutdown(sd, 0);
                    exit(EXIT_SUCCESS);
            }
            // handling of multiple messages in one buffer
            int len = message->len;
            unread -= len;
            message = (struct msg *)(((char*) message) + len);
        }
    }
}

int set_move(local_board_t *board, figure_t player, uint8_t field) {
    if (board->board[field] != NONE_FIGURE)
        return 0;
    board->board[field] = (player == CIRCLE) ? CIRCLE : CROSS;
    return 1;
}

int unset_move(local_board_t *board, uint8_t field)
{
    if (board->board[field] == NONE_FIGURE)
        return 0;
    board->board[field] = NONE_FIGURE;
    return 1;
}

void change_player(figure_t *player)
{
    *player = (*player == CIRCLE) ? CROSS : CIRCLE;
}

result_t check_game(local_board_t *board) {
    //Sprawdzanie wierszy
    for (int y=0; y<3; y++)
    {
        if ((board->board[3*y] == board->board[3*y + 1]) && (board->board[3*y] == board->board[3*y + 2]))
        {
            if (board->board[3*y] == CIRCLE)
                return WIN_CIRCLE;
            if (board->board[3*y] == CROSS)
                return WIN_CROSS;
        }
    }
    //Sprawdzanie kolumn
    for (int x=0; x<3; x++)
    {
        if ((board->board[x] == board->board[x + 3]) && (board->board[x] == board->board[x + 6]))
        {
            if (board->board[x] == CIRCLE)
                return WIN_CIRCLE;
            if (board->board[x] == CROSS)
                return WIN_CROSS;
        }
    }
    //Sprawdzanie przekątnych
    if ((board->board[0] == board->board[4]) && (board->board[0] == board->board[8]))
    {
        if (board->board[4] == CIRCLE)
            return WIN_CIRCLE;
        if (board->board[4] == CROSS)
            return WIN_CROSS;
    }
    if ((board->board[2] == board->board[4]) && (board->board[2] == board->board[6]))
    {
        if (board->board[4] == CIRCLE)
            return WIN_CIRCLE;
        if (board->board[4] == CROSS)
            return WIN_CROSS;
    }
    //sprawdzanie możliwości wykonania ruchu

    for (int i=0; i<9; i++)
        if (board->board[i] == NONE_FIGURE)
            return JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE;

    return DRAW;
}

int review_result(figure_t player, result_t result) {
    if (result == DRAW) return 0;

    if (((player == CIRCLE) && (result == WIN_CIRCLE)) || ((player == CROSS) && (result == WIN_CROSS)))
        return 1;

    return -1;
}

/**
 * @return -1 przegrana, 0 remis, 1 wygrana
 */
int do_move(local_board_t *board, figure_t player, uint8_t *field) {
    uint8_t best_field = 0;
    int best_result = -2;

    int result;
    for (uint8_t i=0; i<9; i++)
    {
        if (set_move(board, player, i) == 0) continue;

        result_t tmp_result = check_game(board);
        if (tmp_result != JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE)
        {
            result = review_result(player, tmp_result);
        }
        else
        {
            figure_t enemy = player;
            change_player(&enemy);
            result = (-1) * do_move(board, enemy, NULL);
        }
        if (result > best_result)
        {
            best_result = result;
            best_field = i;
        }
        unset_move(board, i);
    }

    if (field != NULL)
    {
        *field = best_field;
    }
    return best_result;
}