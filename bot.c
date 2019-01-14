#include "client.h"
#include "message.h"

#define SERVER "127.0.0.1"

#define SERVPORT 9876

int sign_move_n(board_t *board, figure_t player, int place_number_x, int place_number_y)
{
    if (board->moves[place_number_x][place_number_y] != NONE)
        return 0;
    board->moves[place_number_x][place_number_y] = (player == CIRCLE) ? CIRCLE : CROSS;
    return 1;
}

int remove_move_n(board_t *board, int place_number_x, int place_number_y)
{
    if (board->moves[place_number_x][place_number_y] == NONE)
        return 0;
    board->moves[place_number_x][place_number_y] = NONE;
    return 1;

}

void change_player(figure_t * player)
{
    *player = ((*player == CIRCLE) ? CROSS : CIRCLE);
}

result_t result_game(board_t *board)
{
    //Checking verses
    for (int y=0; y<3; y++)
    {
        for(int j=0; j<3; j++) {
            if ((board->moves[y][j] == board->moves[y][j+1]) && (board->moves[y][j] == board->moves[y][j+ 2])) {
                if (board->moves[y][j] == CIRCLE)
                    return WIN_CIRCLE;
                if (board->moves[y][j] == CROSS)
                    return WIN_CROSS;
            }
        }
    }
    //Checking columns
    for (int x=0; x<3; x++)
    {
        for(int j=0; j<3; j++) {
            if ((board->moves[x][j] == board->moves[x + 1][j]) && (board->moves[x][j] == board->moves[x + 2][j])) {
                if (board->moves[x][j] == CIRCLE)
                    return WIN_CIRCLE;
                if (board->moves[x][j] == CROSS)
                    return WIN_CROSS;
            }
        }
    }
    //Checking diagonals
    if ((board->moves[0][0] == board->moves[1][1]) && (board->moves[0][0] == board->moves[2][2]))
    {
        if (board->moves[1][1] == CIRCLE)
            return WIN_CIRCLE;
        if (board->moves[1][1] == CROSS)
            return WIN_CROSS;
    }
    if ((board->moves[0][2] == board->moves[1][1]) && (board->moves[0][2] == board->moves[2][0]))
    {
        if (board->moves[1][1] == CIRCLE)
            return WIN_CIRCLE;
        if (board->moves[1][1] == CROSS)
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

int check_result(figure_t player, result_t result)
{
    if (result == DRAW) return 0;

    if (((player == CIRCLE) && (result == WIN_CIRCLE)) || ((player == CROSS) && (WIN_CROSS)))
        return 1;

    return -1;
}

int do_move(board_t *board, figure_t player, int *place_number_x, int *place_number_y)
{
    int best_move_x = -1;
    int best_move_y = -1;
    int best_move_pt = -2;

    int temp_result;
    for (int i=0; i<3; i++)
    {
        for(int i2; i2<3; i2++) {
            if (sign_move_n(board, player, i, i2) == 0) continue;

            result_t tmpRes = result_game(board);
            if (tmpRes != JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE)
                if (tmpRes != JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE) {
                    temp_result = check_result(player, tmpRes);
                } else {
                    figure_t enemy = player;
                    change_player(&enemy);
                    temp_result = (-1) * do_move(board, enemy, NULL, NULL);
                }
            if (temp_result > best_move_pt) {
                best_move_pt = temp_result;
                best_move_x = i;
                best_move_y = i2;
            }
            remove_move_n(board, i,i2);
        }
    }

    if ((place_number_x != NULL) && (place_number_y != NULL))
    {
        *place_number_x = best_move_x;
        *place_number_y = best_move_y;
    }
    return best_move_pt;
}

int main(int argc, char *argv[])
{
    int rc;
    int serverPort = SERVPORT;
    struct sockaddr_in serveraddr;
    char msg_buf[255];
    char server[255];
    int receive;

    int sd = socket(AF_INET, SOCK_STREAM, 0); // socket descryptor
    if(sd < 0)
    {
        perror("Creating socket error\n");
        exit(-1);
    }
    else
        printf("Client socket is set\n");

    if(argc > 1)
        strcpy(server, argv[1]);
    else
        strcpy(server, SERVER);

    if (argc > 2)
        serverPort = atoi(argv[2]);

    memset(&serveraddr, 0x00, sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(serverPort);

    if((serveraddr.sin_addr.s_addr = inet_addr(server)) == (unsigned long)INADDR_NONE)
    {
        struct hostent *hostp = gethostbyname(server);
        if(hostp == (struct hostent *)NULL)
        {
            printf("404 Host not found \n");
            close(sd);
            exit(-1);
        }
        memcpy(&serveraddr.sin_addr, hostp->h_addr, sizeof(serveraddr.sin_addr));
    }
    if((rc = connect(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) < 0)
    {
        if((rc = connect(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) < 0)
        {
            perror("Connection error\n");
            close(sd);
            exit(-1);
        }
        else
            printf("Connection is set\n");
    }
    strcpy(msg_buf,"RABIN");
    struct msg *message = (struct msg *) msg_buf;
    message->join.nickname;
    message->type = JOIN;
    message->len = strlen(message->join.nickname)+1+HDR_SIZE;
    if(send(sd, message, message->len, 0) == -1){
        perror("Socket send error\n");
        exit(-1);
    }
    board_t moves;

    while(1)
    {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(fileno(stdin), &rfds);
        FD_SET(sd, &rfds);
        select((fileno(stdin) > sd? fileno(stdin) : sd)+1, &rfds, NULL, NULL, NULL);
        if(FD_ISSET(sd, &rfds))
        {
            receive = recv(sd, msg_buf, 255, 0);
            if(receive<1)
            {
                printf("CONNECTION ERROR --> EXITING");
                close(sd);
                exit(0);
            }
            struct msg *message = (struct msg *) msg_buf;
            switch(message->type) {
                case MOVE:
                    if (message->move.x < 0 || message->move.x > 2 || message->move.y < 0 || message->move.y > 2)
                        break;
                    if (message->move.player == CIRCLE)
                        moves.moves[message->move.x][message->move.y] = 'O';
                    else
                        moves.moves[message->move.x][message->move.y] = 'X';
                    break;
                case MOVE_YOUR_ASS:
                    if (message->move_your_ass.you == 1) { // 1 for CIRCLE and 2 for CROSS
                        struct msg *message = (struct msg *) msg_buf;
                        int n = -1;
                        int n2 = -1;
                        figure_t pl = CIRCLE;
                        do_move(&moves, pl, &n, &n2);
                        message->type = MOVE;
                        message->move.x = n;
                        message->move.y = n2;
                        message->move.player = 1;
                        message->len = 3 + HDR_SIZE;
                        if (send(sd, message, message->len, 0) == -1) {
                            perror("Socket send error\n");
                            exit(-1);
                        }
                    }
                    else {
                        struct msg *message = (struct msg *) msg_buf;
                        int n = -1;
                        int n2 = -1;
                        figure_t pl = CROSS;
                        do_move(&moves, pl, &n, &n2);
                        message->type = MOVE;
                        message->move.x = n;
                        message->move.y = n2;
                        message->move.player = 2; // 2 = CROSS
                        message->len = 3 + HDR_SIZE;
                        if (send(sd, message, message->len, 0) == -1) {
                            perror("Socket send error\n");
                            exit(-1);
                        }
                    }
                    break;
                case FINISH:
                    close(sd);
                    exit(0);
                    break;
            }
        }



    }




    close(sd);
    return 0;
}

