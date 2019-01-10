#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "../message.h"


uint8_t bufor[256];

//przykład tworzenia wiadomości tekstowej
int message_example()
{
    struct msg *message = (struct msg *) bufor;

    message->type = CHAT;
    strcpy(message->chat.msg, "To jest wiadomosc");
    message->len = strlen(message->chat.msg)+1+HDR_SIZE;
    //              chat string length + null terminator + header size;

    //wysyłanie wiadomosci
    send(fd_socket, message, message->len, 0);
    return 0;
}
