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

#define SERVER "127.0.0.1"
#define SERVPORT 9876

#define BUFLEN 1024

int main(int argc, char *argv[])
{
    int serverPort = SERVPORT;
    struct sockaddr_in serveraddr;
    char msg_buf[BUFLEN];
    char server[255];
    char chat_msg[160];
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

    if((serveraddr.sin_addr.s_addr = inet_addr(server)) == (unsigned long)INADDR_NONE)
    {
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
    printf("Insert your nickname\n");
    fgets(message->join.nickname, 20, stdin);
    message->join.nickname[strlen(message->join.nickname)-1] = 0; // remove \n from the end
    message->type = JOIN;
    message->len = strlen(message->join.nickname)+1+HDR_SIZE;

    if(connect(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0){
        perror("Connection error");
        shutdown(sd, 0);
        exit(EXIT_FAILURE);
    }

    if(send(sd, message, message->len, 0) == -1){
        perror("Socket send error");
        exit(EXIT_FAILURE);
    }
    printf("**Connected, prepend your message with / to chat, write :q to quit\n");
    printf("**When it`s your turn insert x and y coordinate with space between them - eg. \"1 0\"\n");
    char moves[3][3];
    memset(moves,'_',9);

    while(1){
        // handling console input and network socket at the same time without threading
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(sd, &rfds);
        select((STDIN_FILENO > sd ? STDIN_FILENO : sd)+1, &rfds, NULL, NULL, NULL);

        // handle networking
        if(FD_ISSET(sd, &rfds)){
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
                        if(message->move.player == CIRCLE)
                            moves[message->move.x][message->move.y] = 'O';
                        else
                            moves[message->move.x][message->move.y] = 'X';
                        for (int x=0; x<3; x++)
                        {
                            for (int y=0; y<3; y++)
                            {
                                printf("%c", moves[x][y]);
                            }
                            printf("\n");
                        }
                        printf("\n");
                        break;
                    case MOVE_YOUR_ASS:
                        if(message->move_your_ass.you == CIRCLE)
                        printf("YOUR TURN, CIRCLE \n");
                        else if(message->move_your_ass.you == CROSS)
                            printf("YOUR TURN, CROSS \n");
                        break;
                    case FINISH:
                        switch(message->finish.result){
                            case WIN_CIRCLE:
                                printf("CIRCLE WINS\n");
                                break;
                            case WIN_CROSS:
                                printf("CROSS WINS\n");
                                break;
                            case DRAW:
                                printf("DRAW\n");
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

        // handle console input
        if(FD_ISSET(STDIN_FILENO, &rfds)){
            message = (struct msg *) msg_buf;
            fgets(chat_msg, 160, stdin);
            if(strlen(chat_msg) == 3 && chat_msg[0]==':' && chat_msg[1]=='q'){
                printf("Bye!");
                shutdown(sd, 0);
                exit(EXIT_SUCCESS);
            } else if(chat_msg[0]!='/'){
                sscanf(chat_msg,"%d %d",&(message->move.x),&(message->move.y));
                message->type = MOVE;
                message->len = 3+HDR_SIZE;
                if(send(sd, message, message->len, 0) == -1){
                    perror("Socket send error");
                    exit(EXIT_FAILURE);
                }
            } else {
                message->type = CHAT;
                chat_msg[strlen(chat_msg)-1] = 0; // remove \n from the end
                strcpy(message->chat.msg, &(chat_msg[1]));
                message->len = strlen(message->chat.msg)+21+HDR_SIZE;
                if(send(sd, message, message->len, 0) == -1){
                    perror("Socket send error");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}