#ifndef TICTACTOE_MESSAGE_H
#define TICTACTOE_MESSAGE_H

#define HDR_SIZE 2

typedef enum
{
    JOIN,
    ACK,
    CHAT,
    MOVE
} type_t;

struct msg
{
    uint8_t type;
    uint8_t len;

    union
    {
        struct
        {
            char nickname[20];
        } join;

        struct
        {
            uint8_t result;
        } ack;

        struct
        {
            char msg[80];
        } chat;

        struct
        {
            uint8_t x;
            uint8_t y;
        } move;
    };

}  __attribute__ ((packed));

#endif //TICTACTOE_MESSAGE_H
