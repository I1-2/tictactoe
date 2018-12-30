#include "server.h"

//****************************************************************************


#define BOT 1

typedef enum
{
    NONE    = 0,
    CIRCLE   = 1,
    CROSS = 2
} move_t;

typedef enum
{
    G_CIRCLE = 0,
    G_CROSS  = 1
} player_t;

typedef enum
{
    WIN_CIRCLE         = -1,
    DRAW               =  0,
    WIN_CROSS          =  1,
    JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE    =  2
} result_t;

typedef struct
{
    uint8_t moves[9];
} board_t;

void clear_board(board_t *board)
{
    memset(board, NONE, sizeof(board_t));
}

char show_place(move_t place)
{
    char result;
    switch (place)
    {

        case CIRCLE:
            result = 'O';
            break;

        case CROSS:
            result = 'X';
            break;

        default:
            result = '-';
            break;
    }
    return result;
}

void show_board(board_t *board)
{
    for (int y=0; y<3; y++)
    {
        for (int x=0; x<3; x++)
        {
            printf("%c", show_place(board->moves[3*y+x]));
        }
        printf("\n");
    }
}

int sign_move_n(board_t *board, player_t player, int place_number)
{
    if (board->moves[place_number] != NONE)
        return 0;
    board->moves[place_number] = (player == G_CIRCLE) ? CIRCLE : CROSS;
    return 1;
}

int sign_move(board_t *board, player_t player, int x_number, int y_number)
{
    x_number--;
    y_number--;
    int place_number = 3*x_number + y_number;
    return sign_move_n(board, player, place_number);
}

int remove_move_n(board_t *board, int place_number)
{
    if (board->moves[place_number] == NONE)
        return 0;
    board->moves[place_number] = NONE;
    return 1;

}

void change_player(player_t * player)
{
    *player = ((*player == G_CIRCLE) ? G_CROSS : G_CIRCLE);
}

result_t result_game(board_t *board)
{
    //Sprawdzanie wierszy
    for (int y=0; y<3; y++)
    {
        if ((board->moves[3*y] == board->moves[3*y + 1]) && (board->moves[3*y] == board->moves[3*y + 2]))
        {
            if (board->moves[3*y] == CIRCLE)
                return WIN_CIRCLE;
            if (board->moves[3*y] == CROSS)
                return WIN_CROSS;
        }
    }
    //Sprawdzanie kolumn
    for (int x=0; x<3; x++)
    {
        if ((board->moves[x] == board->moves[x + 3]) && (board->moves[x] == board->moves[x + 6]))
        {
            if (board->moves[x] == CIRCLE)
                return WIN_CIRCLE;
            if (board->moves[x] == CROSS)
                return WIN_CROSS;
        }
    }
    //Sprawdzanie przekątnych
    if ((board->moves[0] == board->moves[4]) && (board->moves[0] == board->moves[8]))
    {
        if (board->moves[4] == CIRCLE)
            return WIN_CIRCLE;
        if (board->moves[4] == CROSS)
            return WIN_CROSS;
    }
    if ((board->moves[2] == board->moves[4]) && (board->moves[2] == board->moves[6]))
    {
        if (board->moves[4] == CIRCLE)
            return WIN_CIRCLE;
        if (board->moves[4] == CROSS)
            return WIN_CROSS;
    }
    //sprawdzanie możliwości wykonania ruchu

    for (int i=0; i<9; i++)
        if (board->moves[i] == NONE)
            return JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE;

    return DRAW;
}

int check_result(player_t player, result_t result)
{
    if (result == DRAW) return 0;

    if (((player == G_CIRCLE) && (result == WIN_CIRCLE)) || ((player == G_CROSS) && (WIN_CROSS)))
        return 1;

    return -1;
}

/**
 * @return -1 przegrana, 0 DRAW, 1 wygrana
 */
int do_move(board_t *board, player_t player, int *place_number)
{
    int best_move = -1;
    int best_move_pt = -2;

    int temp_result;
    for (int i=0; i<9; i++)
    {
        if (sign_move_n(board, player, i) == 0) continue;

        result_t tmpRes = result_game(board);
        if (tmpRes != JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE)
        {
            temp_result = check_result(player, tmpRes);
        }
        else
        {
            player_t enemy = player;
            change_player(&enemy);
            temp_result = (-1) * do_move(board, enemy, NULL);
        }
        if (temp_result > best_move_pt)
        {
            best_move_pt = temp_result;
            best_move = i;
        }
        remove_move_n(board, i);
    }

    if (place_number != NULL)
    {
        *place_number = best_move;
    }
    return best_move_pt;
}

int main()
{
    board_t my_board;
    player_t aktplayer = G_CIRCLE;

    clear_board(&my_board);
    while (result_game(&my_board) == JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE)
    {
        int x_number;
        int y_number;
        printf("Ruch wykonuje %s\n", (aktplayer == G_CIRCLE) ? "Kółko" : "Krzyżyk");
        printf("\t podaj nr wiersza (1-3): ");
        scanf("%d", &x_number);
        printf("\tpodaj nr kolumny (1-3): ");
        scanf("%d", &y_number);
        if (sign_move(&my_board, aktplayer, x_number, y_number) == 0)
            continue;
        change_player(&aktplayer);
        show_board(&my_board);

#ifdef BOT
        if (result_game(&my_board) != JEDEN_RABIN_POWIE_TAK_DRUGI_RABIN_POWIE_NIE)
            break;

        int n = -1;
        do_move(&my_board, aktplayer, &n);
        sign_move_n(&my_board, aktplayer, n);
        change_player(&aktplayer);
        show_board(&my_board);
#endif /*BOT*/
    }
    printf("Koniec gry, result");
    //TODO dopisać odpowiedni komunikat
    printf("\n");

    return 0;
}

//***************************************************************************
/*
int main() {
    int connections[MAX_CONNECTIONS];
    for(int i = 0; i < MAX_CONNECTIONS; ++i) connections[i] = -1;
        // -1 is inactive connection

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
            int i;
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
 */