#include "server.h"

#define true 1
#define false 0
#define bool int

#define BUFLEN 1024
#define MAX_CONNECTIONS 10

int main() {
    int connections[MAX_CONNECTIONS];
    for(int i; i<MAX_CONNECTIONS; ++i){
        connections[i] = -1;
        // -1 is inactive connection
    }

    int port = 9876;

    int fdListen;

    if ((fdListen = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in si_local;
    bzero(&si_local, sizeof(struct sockaddr_in));
    si_local.sin_family = AF_INET;
    si_local.sin_port = htons(port);
    if (bind(fdListen, (const struct sockaddr *) &si_local, sizeof(si_local)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    listen(fdListen, MAX_CONNECTIONS);

    // fd set with fds to be watched for read
    fd_set rfds;

    char buf[BUFLEN];

    while(true) {

        int maxFd = fdListen;

        FD_ZERO(&rfds);
        FD_SET(fdListen, &rfds);

        for (int i = 0; i < MAX_CONNECTIONS; ++i) {
            if (connections[i] != -1) {
                maxFd = (connections[i] > maxFd) ? connections[i] : maxFd;
                FD_SET(connections[i], &rfds);
            }
        }
// WYWOLANIE FUNKCJI SELECT

        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        int retval = select(maxFd + 1, &rfds, NULL, NULL, &tv);
        // select obserwujacy rfds i czekajacy max 5 sekund

        if (FD_ISSET(fdListen, &rfds)) {
            int i = 0;
            for (i = 0; i < MAX_CONNECTIONS; ++i) {
                if (connections[i] == -1) {
                    // przyjmowanie nowego połaczenia
                    connections[i] = accept(fdListen, NULL, NULL);
                    printf("Nowe polaczenie\n");
                    break;
                }
            }
            if (i == MAX_CONNECTIONS) {
                // zamkniecie nowego polaczenia, nie ma juz dla niego miejsca
                int tmpFd = accept(fdListen, NULL, NULL);
                close(tmpFd);
            }
        }

        for(int i = 0; i < MAX_CONNECTIONS; ++i) {
            if (FD_ISSET(connections[i], &rfds)) {
                memset(buf, 0, sizeof(char) * BUFLEN);
                int received = recv(connections[i], buf, BUFLEN, 0);
                if (received < 1) {
                    // zakończenie połączenia
                    close(connections[i]);
                    connections[i] = -1;
                } else {
                    printf("Odebrałem od klienta %d: %s", i, buf);
                    int j;
                    for (j = 0; j < MAX_CONNECTIONS; ++j) {
                        // rozsyłanie wiadomości do pozostałych połączeń
                        if (j == i)
                            continue;
                        if (connections[j] != -1)
                            send(connections[j], buf, received, 0);
                    }
                }
            }
        }

    }
    return 0;
}