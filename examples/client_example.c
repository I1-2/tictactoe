//Serwer domyślny
#define SERVER "127.0.0.1"

/* Server's port number */
#define SERVPORT 9876

/*
 * Argumenty a.out serverIpOrName serverPort
 */
int main(int argc, char *argv[])
{
    int sd;
    int rc;
    int serverPort = SERVPORT;
    struct sockaddr_in serveraddr;
    char server[255];

    char data[] = "To jest testowa wiadomosc";

    if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Błąd tworzenia gniazda");
        exit(-1);
    }
    else
        printf("Utworzono gniazdo klienta\n");

    /* Jako argument podano adres serwera */
    if(argc > 1)
        strcpy(server, argv[1]);
    else
        strcpy(server, SERVER);    /*Use the default server name or IP*/

    if (argc > 2)
        serverPort = atoi(argv[2]);

    printf("Łączenie z %s, port %d\n", server, serverPort);

    memset(&serveraddr, 0x00, sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(serverPort);

    if((serveraddr.sin_addr.s_addr = inet_addr(server)) == (unsigned long)INADDR_NONE)
    {
        struct hostent *hostp = gethostbyname(server);
        if(hostp == (struct hostent *)NULL)
        {
            printf("Nie znaleziono hosta -> ");
            printf("h_errno = %d\n", h_errno);
            printf("---This is a client program---\n");
            printf("Command usage: %s <server name or IP> <PORT>\n", argv[0]);
            close(sd);
            exit(-1);
        }
        memcpy(&serveraddr.sin_addr, hostp->h_addr, sizeof(serveraddr.sin_addr));
    }

    if((rc = connect(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) < 0)
    {
        perror("Błąd połączenia");
        close(sd);
        exit(-1);
    }
    else
        printf("Ustanowiono połączenie\n");

    rc = send(sd, data, sizeof(data), 0);

    close(sd);
    exit(0);

    return 0;
}
