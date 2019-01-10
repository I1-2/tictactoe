#ifndef TICTACTOE_MESSAGE_H
#define TICTACTOE_MESSAGE_H

#define HDR_SIZE 2

typedef enum
{
    WIN_CIRCLE                                      = -1,
    DRAW                                            =  0,
    WIN_CROSS                                       =  1,
    JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE     =  2
} result_t;

typedef enum
{
    JOIN,
    CHAT,
    MOVE,
    FINISH
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
            char msg[80];
        } chat;

        struct
        {
            uint8_t x;
            uint8_t y;
        } move;

        struct
        {
            result_t result;
        } finish;
    };

}  __attribute__ ((packed));

#endif //TICTACTOE_MESSAGE_H
