#include <stdio.h>
#include <string.h>
#include "board.h"

void init_board(char board[8][8])
{
    char temp[8][8] = {
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'},
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'}
    };
    memcpy(board, temp, sizeof(temp));
}

void print_board(const char board[8][8])
{

    printf("  a b c d e f g h\n");
    printf(" ┌────────────────┐\n");
    for (int i = 0; i < 8; i++) {
        printf("%d│", 8 - i);
        // hall matrix and leds has reversed y axis compared to board
        for (int j = 7; j >= 0; j--) {
            printf("%c ", board[i][j]);
        }
        printf("│%d\n", 8 - i);
    }
    printf(" └────────────────┘\n");
    printf("  a b c d e f g h\n");
}
