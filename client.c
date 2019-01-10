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
    char say[3]={'s','a','y'};
    int sd = socket(AF_INET, SOCK_STREAM, 0); // socket descryptor
    if(sd < 0)
    {
        perror("Błąd tworzenia gniazda");
        exit(-1);
    }
    else
        printf("Utworzono gniazdo klienta\n");

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
            printf("Nie znaleziono hosta ");
            close(sd);
            exit(-1);
        }
        memcpy(&serveraddr.sin_addr, hostp->h_addr, sizeof(serveraddr.sin_addr));
    }

    else
    {
        if(strcmp(say,msg_buf))
        {
            if((rc = connect(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) < 0)
            {
                perror("Błąd połączenia");
                close(sd);
                exit(-1);
            }
            else
                printf("Ustanowiono połączenie\n");
        }

    }

    memset(msg_buf, 0x00, sizeof(msg_buf));
    struct msg *message = (struct msg *) msg_buf;
    printf("Podaj swoj nickname");
    scanf("%20s",msg_buf);
    message->type = JOIN;
    strcpy(message->join.nickname, msg_buf);
    message->len = strlen(message->join.nickname)+1+HDR_SIZE;
    send(sd, message, message->len, 0);
    if(msg_buf[0]!='/')
    {
        rc = send(sd, msg_buf, sizeof(msg_buf), 0);

    }

    return 0;
}