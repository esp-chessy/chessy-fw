#ifndef MOVES_H
#define MOVES_H

#include <stdbool.h>
#include "board.h"

/**
 * @brief A position on the board
 *
 */
typedef struct {
    int x;
    int y;
} Position_t;

/**
 * @brief A move on the board
 *
 */
typedef struct {
    Position_t start;
    Position_t end;
} Move_t;

/**
 * @brief Get the available moves for a piece based on the piece's position
 *
 * @param board The current board state
 * @param x The x coordinate of the piece
 * @param y The y coordinate of the piece
 * @param moves Array to store valid moves
 * @return int Number of valid moves found
 */
int get_available_moves(const char board[8][8], int x, int y, Position_t *moves);

/**
 * @brief Check if a move is valid
 *
 * @param end The end position of the move
 * @param moves Array of valid moves
 * @param move_count Number of valid moves
 * @return true if the move is valid
 * @return false if the move is invalid
 */
bool is_valid_move(Position_t end, Position_t *moves, int move_count);

#endif
