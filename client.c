#include "client.h"
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
    char chat_msg[160];
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
            shutdown(sd,0);
            exit(-1);
        }
        memcpy(&serveraddr.sin_addr, hostp->h_addr, sizeof(serveraddr.sin_addr));
    }
    if((rc = connect(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) < 0)
    {
        if((rc = connect(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) < 0)
        {
            perror("Connection error\n");
            shutdown(sd,0);
            exit(-1);
        }
        else
            printf("Connection is set\n");
    }

    struct msg *message = (struct msg *) msg_buf;
    printf("Insert your nickname\n");
    fgets(message->join.nickname, 20, stdin);
    message->join.nickname[strlen(message->join.nickname)-1] = 0; // remove \n from the end
    message->type = JOIN;
    message->len = strlen(message->join.nickname)+1+HDR_SIZE;
    if(send(sd, message, message->len, 0) == -1){
        perror("Socket send error\n");
        exit(-1);
    }
    printf("****Prepend your message with / to chat, write :q to quit****\n");
    printf("When it`s your turn insert x and y coordinate with between them: \n");
    char moves[3][3];
    memset(moves,'_',9);

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
                printf("CONNECTION ERROR --> EXITING\n");
                shutdown(sd,0);
                exit(0);
            }
            struct msg *message = (struct msg *) msg_buf;
            switch(message->type)
            {
                case CHAT:
                    printf("%s: %s\n",message->chat.nickname, message->chat.msg);
                    break;
                case MOVE:
                    if(message->move.x < 0 || message->move.x > 2 || message->move.y < 0 || message->move.y > 2)
                        break;
                    if(message->move.player==CIRCLE)
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
                    printf("YOUR TURN\n");
                    break;
                case FINISH:
                    switch(message->finish.result)
                    {
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
                            printf("JEDEN RABIN POWIE TAK DRUGI RABIN POWIE NIE\n");
                            break;
                    }
                    shutdown(sd,0);
                    exit(0);
                    break;
            }
        }
        if(FD_ISSET(fileno(stdin), &rfds))
        {

            struct msg *message = (struct msg *) msg_buf;
            fgets(chat_msg, 160, stdin);
            if(chat_msg[0]==':' && chat_msg[1]=='q')
            {
                shutdown(sd,0);
                exit(0);
            }
            else if(chat_msg[0]!='/')
            {
                sscanf(chat_msg,"%d %d",&(message->move.x),&(message->move.y));
                message->type = MOVE;
                message->len = 3+HDR_SIZE;
                if(send(sd, message, message->len, 0) == -1){
                    perror("Socket send error\n");
                    exit(-1);
                }
            }
            else
            {
                message->type = CHAT;
                chat_msg[strlen(chat_msg)-1] = 0; // remove \n from the end
                strcpy(message->chat.msg,&(chat_msg[1]));
                message->len = strlen(message->chat.msg)+21+HDR_SIZE;
                if(send(sd, message, message->len, 0) == -1){
                    perror("Socket send error\n");
                    exit(-1);
                }
            }

        }
    }




    shutdown(sd,0);
    return 0;
}