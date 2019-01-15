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
#include <netinet/tcp.h>
#include "message.h"

#define SERVER "127.0.0.1"

#define SERVPORT 9876

int main(int argc, char *argv[])
{
    int rc;
    int serverPort = SERVPORT;
    struct sockaddr_in serveraddr;
    char msg_buf[255];
    char server[255];
    int receive;
    int one = 1;
    int sd = socket(AF_INET, SOCK_STREAM, 0); // socket descryptor
    setsockopt(sd,SOL_TCP,TCP_NODELAY,&one,sizeof(one));
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
    memset(moves.moves,NONE_FIGURE,9);

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
                        moves.moves[message->move.x][message->move.y] = message->move.player;
                    break;
                case MOVE_YOUR_ASS:
                    if (message->move_your_ass.you == CIRCLE) {
                        printf("CIRCLE MOVE\n");
                        struct msg *message = (struct msg *) msg_buf;
                        int n;
                        int n2;
                        for(int i=0;i<3;i++)
                        {
                            if(moves.moves[i][0] == moves.moves[i][1] && moves.moves[i][2]==NONE_FIGURE)
                            {
                                n=i;
                                n2=2;
                                break;
                            }
                            else if(moves.moves[i][0] == moves.moves[i][2] && moves.moves[i][1]==NONE_FIGURE)
                            {
                                n=i;
                                n2=1;
                                break;
                            }
                            else if(moves.moves[i][1] == moves.moves[i][2] && moves.moves[i][0]==NONE_FIGURE)
                            {
                                n=i;
                                n2=0;
                                break;
                            }
                        }
                        for(int i=0;i<3;i++) {
                            if (moves.moves[0][i] == moves.moves[1][i] && moves.moves[2][i] == NONE_FIGURE) {
                                n = 2;
                                n2 = i;
                                break;
                            } else if (moves.moves[0][i] == moves.moves[2][i] && moves.moves[1][i] == NONE_FIGURE) {
                                n = 1;
                                n2 = i;
                                break;
                            } else if (moves.moves[1][i] == moves.moves[2][i] && moves.moves[0][i] == NONE_FIGURE) {
                                n = 0;
                                n2 = i;
                                break;
                            }

                        }
                        if(moves.moves[1][1] == moves.moves[2][2] && moves.moves[0][0]==NONE_FIGURE)
                        {
                            n = 0;
                            n2 = 0;
                        }
                        else if(moves.moves[2][2] == moves.moves[0][0] && moves.moves[1][1]==NONE_FIGURE)
                        {
                            n = 1;
                            n2 = 1;
                        }
                        else if(moves.moves[1][1] == moves.moves[0][0] && moves.moves[2][2]==NONE_FIGURE)
                        {
                            n = 2;
                            n2 = 2;
                        }
                        else if(moves.moves[0][2] == moves.moves[1][1] && moves.moves[2][0]==NONE_FIGURE)
                        {
                            n = 2;
                            n2 = 0;
                        }
                        else if(moves.moves[0][2] == moves.moves[2][0] && moves.moves[1][1]==NONE_FIGURE)
                        {
                            n = 1;
                            n2 = 1;
                        }
                        else if(moves.moves[2][0] == moves.moves[1][1] && moves.moves[0][2]==NONE_FIGURE)
                        {
                            n = 0;
                            n2 = 2;
                        }
                        else
                        {
                            for(int j=0;j<3;j++)
                            {
                                for(int j2=0;j2<3;j2++)
                                {
                                    if(moves.moves[j][j2]==NONE_FIGURE)
                                    {
                                        n = j;
                                        n2 = j2;
                                    }
                                }
                            }

                        }
                        message->type = MOVE;
                        message->move.x = n;
                        message->move.y = n2;
                        message->move.player = CIRCLE;
                        message->len = 3 + HDR_SIZE;
                        if (send(sd, message, message->len, 0) == -1) {
                            perror("Socket send error\n");
                            exit(-1);
                        }
                    }
                    else {
                        printf("Wykonuje ruch jako krzyzyk\n");
                        struct msg *message = (struct msg *) msg_buf;
                        int n;
                        int n2;
                        for(int i=0;i<3;i++) {
                            if (moves.moves[i][0] == moves.moves[i][1] && moves.moves[i][2] == NONE_FIGURE) {
                                n = i;
                                n2 = 2;
                                break;
                            } else if (moves.moves[i][0] == moves.moves[i][2] && moves.moves[i][1] == NONE_FIGURE) {
                                n = i;
                                n2 = 1;
                                break;
                            } else if (moves.moves[i][1] == moves.moves[i][2] && moves.moves[i][0] == NONE_FIGURE) {
                                n = i;
                                n2 = 0;
                                break;
                            }
                        }
                        for(int i=0;i<3;i++) {
                            if (moves.moves[0][i] == moves.moves[1][i] && moves.moves[2][i] == NONE_FIGURE) {
                                n = 2;
                                n2 = i;
                                break;
                            } else if (moves.moves[0][i] == moves.moves[2][i] && moves.moves[1][i] == NONE_FIGURE) {
                                n = 1;
                                n2 = i;
                                break;
                            } else if (moves.moves[1][i] == moves.moves[2][i] && moves.moves[0][i] == NONE_FIGURE) {
                                n = 0;
                                n2 = i;
                                break;
                            }

                        }
                        if(moves.moves[1][1] == moves.moves[2][2] && moves.moves[0][0]==NONE_FIGURE)
                        {
                            n = 0;
                            n2 = 0;
                        }
                        else if(moves.moves[2][2] == moves.moves[0][0] && moves.moves[1][1]==NONE_FIGURE)
                        {
                            n = 1;
                            n2 = 1;
                        }
                        else if(moves.moves[1][1] == moves.moves[0][0] && moves.moves[2][2]==NONE_FIGURE)
                        {
                            n = 2;
                            n2 = 2;
                        }
                        else if(moves.moves[0][2] == moves.moves[1][1] && moves.moves[2][0]==NONE_FIGURE)
                        {
                            n = 2;
                            n2 = 0;
                        }
                        else if(moves.moves[0][2] == moves.moves[2][0] && moves.moves[1][1]==NONE_FIGURE)
                        {
                            n = 1;
                            n2 = 1;
                        }
                        else if(moves.moves[2][0] == moves.moves[1][1] && moves.moves[0][2]==NONE_FIGURE)
                        {
                            n = 0;
                            n2 = 2;
                        }
                        else
                        {
                            for(int j=0;j<3;j++)
                            {
                                for(int j2=0;j2<3;j2++)
                                {
                                    if(moves.moves[j][j2]==NONE_FIGURE)
                                    {
                                        n = j;
                                        n2 = j2;
                                        break;
                                    }
                                }
                            }

                        }
                        message->type = MOVE;
                        message->move.x = n;
                        message->move.y = n2;
                        message->move.player = CROSS; // 2 = CROSS
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

