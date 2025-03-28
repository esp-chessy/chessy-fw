#include <stdlib.h>
#include <stdio.h>
#include "moves.h"
#include "board.h"
#include <ctype.h>
#include <stdbool.h>
typedef int (*char_check_fn)(int);

static char_check_fn get_enemy_check(char piece)
{
    return islower(piece) ? isupper : islower;
}

static bool is_valid_position(int x, int y)
{
    return x >= 0 && x < 8 && y >= 0 && y < 8;
}

static int pawn_moves(const char board[8][8], int x, int y, Position_t *moves)
{
    printf("pawn_moves\n");
    int move_count = 0;
    char piece = board[x][y];
    int direction = (piece == 'p') ? -1 : 1;
    int start_rank = (piece == 'p') ? 6 : 1;
    char_check_fn is_enemy = get_enemy_check(piece);

    // Two square advance from starting position
    if (x == start_rank) {
        int two_ahead = x + (2 * direction);
        if (two_ahead >= 0 && two_ahead < 8 && board[two_ahead][y] == ' ') {
            moves[move_count++] = (Position_t) {
                two_ahead, y
            };
        }
    }

    // One square advance
    int one_ahead = x + direction;
    if (one_ahead >= 0 && one_ahead < 8 && board[one_ahead][y] == ' ') {
        moves[move_count++] = (Position_t) {
            one_ahead, y
        };
    }
    // Diagonal captures
    for (int dy = -1; dy <= 1; dy += 2) {
        if (y + dy >= 0 && y + dy < 8 && one_ahead >= 0 && one_ahead < 8) {
            if (is_enemy(board[one_ahead][y + dy])) {
                moves[move_count++] = (Position_t) {
                    one_ahead, y + dy
                };
            }
        }
    }
    return move_count;
}

static int rook_moves(const char board[8][8], int x, int y, Position_t *moves)
{
    // TODO: add a check for castling
    printf("rook_moves\n");
    int move_count = 0;
    char_check_fn is_enemy = get_enemy_check(board[x][y]);
    // Direction arrays for horizontal and vertical movement
    const int dx[] = {0, 0, -1, 1};  // Left, right, up, down
    const int dy[] = {-1, 1, 0, 0};

    // Check all four directions
    for (int dir = 0; dir < 4; dir++) {
        for (int i = 1; ; i++) {
            int new_x = x + dx[dir] * i;
            int new_y = y + dy[dir] * i;

            if (!is_valid_position(new_x, new_y)) {
                break;  // Out of bounds
            }

            if (board[new_x][new_y] != ' ') {
                if (is_enemy(board[new_x][new_y])) {
                    moves[move_count++] = (Position_t) {
                        new_x, new_y
                    };
                }
                break;  // Hit a piece
            }

            moves[move_count++] = (Position_t) {
                new_x, new_y
            };
        }
    }
    return move_count;
}

static int knight_moves(const char board[8][8], int x, int y, Position_t *moves)
{
    printf("knight_moves\n");
    int move_count = 0;
    char_check_fn is_enemy = get_enemy_check(board[x][y]);
    // Direction arrays for the 8 possible knight moves
    const int dx[] = {2, 2, -2, -2, 1, 1, -1, -1};
    const int dy[] = {1, -1, 1, -1, 2, -2, 2, -2};

    // Check all possible knight moves
    for (int i = 0; i < 8; i++) {
        int new_x = x + dx[i];
        int new_y = y + dy[i];
        if (is_valid_position(new_x, new_y) &&
                (board[new_x][new_y] == ' ' || is_enemy(board[new_x][new_y]))) {
            moves[move_count++] = (Position_t) {
                new_x, new_y
            };
        }
    }
    return move_count;
}

static int bishop_moves(const char board[8][8], int x, int y, Position_t *moves)
{
    printf("bishop_moves\n");
    int move_count = 0;
    char_check_fn is_enemy = get_enemy_check(board[x][y]);

    // Direction arrays for the four diagonals
    const int dx[] = {-1, -1, 1, 1};
    const int dy[] = {-1, 1, -1, 1};

    // Check all four diagonal directions
    for (int dir = 0; dir < 4; dir++) {
        for (int i = x + dx[dir], j = y + dy[dir]; is_valid_position(i, j); i += dx[dir], j += dy[dir]) {
            if (board[i][j] == ' ') {
                moves[move_count++] = (Position_t) {
                    i, j
                };
            } else if (is_enemy(board[i][j])) {
                moves[move_count++] = (Position_t) {
                    i, j
                };
                break;
            } else {
                break;
            }
        }
    }
    return move_count;
}

static int king_moves(const char board[8][8], int x, int y, Position_t *moves)
{
    printf("king_moves\n");
    int move_count = 0;
    char_check_fn is_enemy = get_enemy_check(board[x][y]);

    // Direction arrays for the eight possible king moves
    const int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    const int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

    // Check all 8 possible king moves
    for (int i = 0; i < 8; i++) {
        int new_x = x + dx[i];
        int new_y = y + dy[i];

        if (is_valid_position(new_x, new_y) && (board[new_x][new_y] == ' ' || is_enemy(board[new_x][new_y]))) {
            moves[move_count++] = (Position_t) {
                new_x, new_y
            };
        }
    }
    return move_count;
}

static int queen_moves(const char board[8][8], int x, int y, Position_t *moves)
{
    printf("queen_moves\n");
    int move_count = 0;
    char_check_fn is_enemy = get_enemy_check(board[x][y]);

    // Direction arrays for all 8 directions (horizontal, vertical, diagonal)
    const int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    const int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

    // Check all eight directions
    for (int dir = 0; dir < 8; dir++) {
        for (int i = 1; ; i++) {
            int new_x = x + dx[dir] * i;
            int new_y = y + dy[dir] * i;

            if (!is_valid_position(new_x, new_y)) {
                break;  // Out of bounds
            }

            if (board[new_x][new_y] != ' ') {
                if (is_enemy(board[new_x][new_y])) {
                    moves[move_count++] = (Position_t) {
                        new_x, new_y
                    };
                }
                break;  // Hit a piece
            }

            moves[move_count++] = (Position_t) {
                new_x, new_y
            };
        }
    }
    return move_count;
}

int get_available_moves(const char board[8][8], int x, int y, Position_t *moves)
{
    switch (board[x][y]) {
    case 'P':
    case 'p':
        return pawn_moves(board, x, y, moves);
    case 'R':
    case 'r':
        return rook_moves(board, x, y, moves);
    case 'N':
    case 'n':
        return knight_moves(board, x, y, moves);
    case 'B':
    case 'b':
        return bishop_moves(board, x, y, moves);
    case 'Q':
    case 'q':
        return queen_moves(board, x, y, moves);
    case 'K':
    case 'k':
        return king_moves(board, x, y, moves);
    default:
        return 0;
    }
}

bool is_valid_move(Position_t end, Position_t *moves, int move_count)
{
    // Check if the end position matches any of the valid moves
    for (int i = 0; i < move_count; i++) {
        if (moves[i].x == end.x && moves[i].y == end.y) {
            return true;
        }
    }
    return false;
}
