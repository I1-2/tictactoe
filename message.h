#ifndef TICTACTOE_MESSAGE_H
#define TICTACTOE_MESSAGE_H

#define HDR_SIZE 2

typedef enum
{
    CIRCLE,
    CROSS
} player_t;

typedef enum
{
    WIN_CIRCLE,
    DRAW,
    WIN_CROSS,
    JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE
} result_t;

typedef enum
{
    JOIN,
    CHAT,
    MOVE,
    MOVE_YOUR_ASS,
    FINISH
} msg_type_t;

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
            char nickname[20];
            char msg[160];
        } chat;

        struct
        {
            uint8_t x;
            uint8_t y;
            uint8_t player;
        } move;

        struct
        {
            uint8_t result;
        } finish;

        struct
        {
            uint8_t you;
        } move_your_ass;
    };

}  __attribute__ ((packed));

#endif //TICTACTOE_MESSAGE_H
