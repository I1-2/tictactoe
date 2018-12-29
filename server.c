#include "server.h"


struct polaczenie {
    int aktywne;
    int fd;
};

int fdListen;

if ((
fdListen = socket(AF_INET, SOCK_STREAM, 0)
)==-1)
{
perror("socket");
exit(EXIT_FAILURE);
}


si_local.
sin_family = AF_INET;
si_local.
sin_port = htons(port);
if (
bind(fdListen,
(const struct sockaddr *)&si_local, sizeof(si_local))==-1)
{
perror("bind");
exit(EXIT_FAILURE);
}
listen(fdListen,
10);

fd_set rfds;

int maxFd = fdListen;

FD_ZERO(&rfds);
FD_SET(fdListen, &rfds
);

for (
i = 0;
i<10; i++)
{
if (mojePolaczenia[i].aktywne)
{
maxFd = (mojePolaczenia[i].fd > maxFd) ? mojePolaczenia[i].fd : maxFd;
FD_SET(mojePolaczenia[i]
.fd, &rfds);
}
}
// WYWOLANIE FUNKCJI SELECT

/* Czekanie nie dłużej niż sekund. */
tv.
tv_sec = 5;
tv.
tv_usec = 0;
retval = select(maxFd + 1, &rfds, NULL, NULL, &tv);

if (
FD_ISSET(fdListen, &rfds
))
{
int polIdx = 0;
for (
polIdx = 0;
polIdx<10; polIdx++)
{
if (mojePolaczenia[polIdx].aktywne == 0)
{ //Przyjmowanie nowego połaczenia
mojePolaczenia[polIdx].
aktywne = 1;
mojePolaczenia[polIdx].
fd = accept(fdListen, NULL, NULL);
printf("Nowe polaczenie\n");
break;
}
}
if (polIdx == 10)
{  //Serwer obsługuje 10 połączeń, odrzucanie połączenia
int tmpFd = accept(fdListen, NULL, NULL);
send(tmpFd,
"Odmowa\n", 7, 0);
close(tmpFd);
}

if (FD_ISSET(mojePolaczenia[i].fd, &rfds))
{
memset(buf, 0, sizeof(char)*BUFLEN);
int odczytano = recv(mojePolaczenia[i].fd, buf, BUFLEN, 0);
if (odczytano < 1)
{ //Zakończenie połączenia
mojePolaczenia[i].aktywne=0;
close(mojePolaczenia[i].fd);
mojePolaczenia[i].fd=-1;
}
else
{
printf("Odebrałem od klienta %d: %s", i, buf);
int polIdx;
for (polIdx=0; polIdx<10; polIdx++)
{ //Rozsyłanie wiadomości do pozostałych połączeń
if(polIdx == i)
continue;
if (mojePolaczenia[polIdx].aktywne)
send(mojePolaczenia[polIdx].fd, buf, odczytano, 0);
}
}
}

