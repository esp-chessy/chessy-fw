#include <stdio.h>
#include <string.h>
#include "board.h"

char board[8][8];


void init_board()
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

void print_board()
{
    printf("  a b c d e f g h\n");
    printf(" ┌────────────────┐\n");
    for (int i = 0; i < 8; i++) {
        printf("%d│", 8 - i);
        for (int j = 0; j < 8; j++) {
            printf("%c ", board[i][j]);
        }
        printf("│%d\n", 8 - i);
    }
    printf(" └────────────────┘\n");
    printf("  a b c d e f g h\n");
}
