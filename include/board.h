#ifndef BOARD_H
#define BOARD_H

#include "move.h"
#include "piece.h"
#include "square.h"
#include <stddef.h>

typedef struct {
    piece_t pieces[64];
} board_t;

board_t board_empty(void);
piece_t board_piece_at(board_t *board, square_t square);
void board_set_square(board_t *board, square_t square, piece_t piece);
int board_make_move(board_t *board, move_t move);
int board_load_fen(board_t *board, const char *fen);

#endif /* BOARD_H */
