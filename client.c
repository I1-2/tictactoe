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
        perror("Creating socket error");
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
            printf("404 Host not found ");
            close(sd);
            exit(-1);
        }
        memcpy(&serveraddr.sin_addr, hostp->h_addr, sizeof(serveraddr.sin_addr));
    }
    if((rc = connect(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) < 0)
    {
        if((rc = connect(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) < 0)
        {
            perror("Connection error");
            close(sd);
            exit(-1);
        }
        else
            printf("Connection is set\n");
    }

    while(1)
    {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(fileno(stdin), &rfds);
        FD_SET(sd, &rfds);
        select((fileno(stdin) > sd: fileno(stdin) ? sd)+1, &rfds, NULL, NULL, NULL);
        if(FD_ISSET(sd,&rfds))
        {
            receive = recv(sd, msg_buf, 255, 0);
        }
        if(FD_ISSET(fileno(stdin),&rfds)
        {

            struct msg *message = (struct msg *) msg_buf;
            printf("Insert your nickname");
            fgets(message->join.nickname, 20, stdin);
            message->type = JOIN;
            message->len = strlen(message->join.nickname)+1+HDR_SIZE;
            if(send(sd, message, message->len, 0) == -1){
                perror("Socket send error");
                exit(-1);
            }
            printf("Insert x and y coordinate with between them: ");
            fgets(chat_msg, 160, stdin);
            if(chat_msg[0]!='/')
            {
                sscanf(chat_msg,"%d %d",&(message->move.x),&(message->move.y));
                message->type = MOVE;
                message->len = 2+HDR_SIZE;
                if(send(sd, message, message->len, 0) == -1){
                    perror("Socket send error");
                    exit(-1);
                }
            }
            else if(chat_msg[0]==':' && chat_msg[1]=='q')
            {
                close(sd);
            }
            else
            {
                message->type = CHAT;
                strcpy(message->chat.msg,chat_msg);
                message->len = strlen(message->chat.msg)+1+HDR_SIZE;
                if(send(sd, message, message->len, 0) == -1){
                    perror("Socket send error");
                    exit(-1);
                }
            }

        }
    }




    close(sd);
    return 0;
}