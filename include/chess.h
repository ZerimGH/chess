#ifndef CHESS_H
#define CHESS_H

#include "board.h"
#include "move.h"

#include <stddef.h>

#define CHESS_MAX_MOVES 218

typedef struct {
    board_t board;
    int color_to_move;
    int wcq, wck, bcq, bck;
    square_t en_passant;
    int halfmove_clock;
    int fullmove_number;
    move_t moves[CHESS_MAX_MOVES];
    size_t moves_len;
} chess_t;

chess_t chess_new(void);
int chess_load_fen(chess_t *chess, const char *fen);
int chess_make_move(chess_t *chess, move_t move);

#endif /* CHESS_H */
