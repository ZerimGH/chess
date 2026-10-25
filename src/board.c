#include "board.h"
#include <stdio.h>
#include <string.h>

board_t board_empty(void) {
    board_t board = {0};
    for (uint8_t i = 0; i < 64; i++) {
        board.pieces[i] = PIECE(PIECE_COLOR_WHITE, PIECE_TYPE_NONE);
    }

    return board;
}

piece_t board_piece_at(board_t *board, square_t square) {
    if (!board) return PIECE(PIECE_COLOR_WHITE, PIECE_TYPE_NONE);

    uint8_t idx = square_to_idx(square);
    if (idx == 0xFF) return PIECE(PIECE_COLOR_WHITE, PIECE_TYPE_NONE);
    return board->pieces[idx];
}

void board_set_square(board_t *board, square_t square, piece_t piece) {
    if (!board) return;

    uint8_t idx = square_to_idx(square);
    if (idx == 0xFF) return;

    board->pieces[idx] = piece;
}

int board_make_move(board_t *board, move_t move) {
    if (!board) return 1;

    uint8_t idx_from = square_to_idx(move.from);
    uint8_t idx_to   = square_to_idx(move.to);
    if (idx_from == 0xFF || idx_to == 0xFF || idx_from == idx_to) return 1;

    board->pieces[idx_to]   = board->pieces[idx_from];
    board->pieces[idx_from] = PIECE(PIECE_COLOR_WHITE, PIECE_TYPE_NONE);
    return 0;
}

int board_load_fen(board_t *board, const char *fen) {
    if (!board || !fen) return 1;

    board_t new      = board_empty();
    uint8_t rank     = 0;
    uint8_t file     = 0;
    const char *read = fen;
    char c;

    while ((c = *read++) && c != ' ') {
        if (c == '/') {
            if (file != 8 || rank >= 7) return 1;
            rank++;
            file = 0;
            continue;
        }

        if (c >= '1' && c <= '8') {
            uint8_t count = c - '0';
            if (file + count > 8) return 1;
            file += count;
            continue;
        }

        piece_t piece = piece_from_char(c);
        uint8_t type  = PIECE_TYPE(piece);

        if (type > PIECE_TYPE_KING) return 1;
        if (rank >= 8 || file >= 8) return 1;

        uint8_t idx     = (7 - rank) + file * 8;
        new.pieces[idx] = piece;
        file++;
    }

    if (rank != 7 || file != 8) return 1;

    *board = new;
    return 0;
}
