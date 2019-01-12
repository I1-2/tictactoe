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
