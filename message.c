#include "message.h"
#define HDR_SIZE 3

typedef enum
{
    DOLACZ,
    POTWIERDZENIE,
    WIAD_TEKST,
    RUCH
} type_t;

struct wiadomosc
{
    uint8_t sync;
    uint8_t type;
    uint8_t len;

    union
    {
        struct
        {
            char imie[20];
        } dolacz;

        struct
        {
            uint8_t rezultat;
        } potw;

        struct
        {
            char napis[80];
        } wiadomosc;

        struct
        {
            uint8_t x;
            uint8_t y;
        } ruch;
    } dane;

}  __attribute__ ((packed));

uint8_t bufor[256];

int main()
{
    struct wiadomosc *mojaWiad = (struct wiadomosc *) bufor;

    //Przykład tworzenia wiadomości tekstowej
    mojaWiad->type = WIAD_TEKST;
    strcpy(mojaWiad->dane.wiadomosc.napis, "To jest wiadomosc");
    mojaWiad->len = strlen(mojaWiad->dane.wiadomosc.napis)+1+HDR_SIZE;

    //wysyłanie
    //send(nrGniazda, mojaWiad, mojaWiad->len, 0);
    return 0;
}