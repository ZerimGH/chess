#include "chess.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define INIT_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

chess_t chess_new(void) {
    chess_t chess;
    chess.board = board_empty();
    /* This should probably never fail */
    chess_load_fen(&chess, INIT_FEN);
    return chess;
}

int chess_load_fen(chess_t *chess, const char *fen) {
    if (!chess || !fen) return 1;

    board_t new_board = chess->board;
    if (board_load_fen(&new_board, fen)) return 1;

    const char *space = strchr(fen, ' ');
    if (!space) return 1;
    space++;
    if (!*space) return 1;

    int color_to_move = 1;
    int wcq = 0, wck = 0, bcq = 0,
        bck             = 0; /* White can castle queenside, kingside, black... */
    square_t en_passant = (square_t){0xFF, 0xFF};
    int halfmove_clock  = 0;
    int fullmove_number = 0;

    /* This is gonna get ugly */
    /* Active color */
    if (*space == 'w') color_to_move = 1;
    else if (*space == 'b') color_to_move = 0;
    else return 1;

    /* Castling availability */
    space++;
    if (*space != ' ') return 1;
    space++;
    if (!*space) return 1;

    while (*space && *space != ' ') {
        if (*space == '-') {
            space++;
            break;
        }
        switch (*space) {
        case 'K':
            if (wck) return 1;
            wck = 1;
            break;
        case 'Q':
            if (wcq) return 1;
            wcq = 1;
            break;
        case 'k':
            if (bck) return 1;
            bck = 1;
            break;
        case 'q':
            if (bcq) return 1;
            bcq = 1;
            break;
        default: return 1;
        }
        space++;
    }

    /* En passant target square */
    if (*space != ' ') return 1;
    space++;
    if (!*space) return 1;
    if (*space == '-') space++;
    else {
        if (*space < 'a' || *space > 'h') return 1;
        en_passant.col = *space - 'a';
        space++;
        if (!*space) return 1;
        if (*space < '1' || *space > '8') return 1;
        en_passant.row = *space - '1';
    }

    /* Halfmove clock */
    space++;
    if (!*space) return 1;
    errno        = 0;
    char *endptr = NULL;
    long v       = strtol(space, &endptr, 10);
    if (errno) return 1;
    if (v > 50 || v < 0) return 1;
    halfmove_clock = (int)v;
    space          = endptr;

    /* Fullmove number */
    if (!*space) return 1;
    space++;
    if (!*space) return 1;
    errno  = 0;
    endptr = NULL;
    v      = strtol(space, &endptr, 10);
    if (errno) return 1;
    if (v > INT_MAX || v < 0) return 1;
    fullmove_number = (int)v;
    space           = endptr;

    if (*space) return 1;

    chess->board           = new_board;
    chess->color_to_move   = color_to_move;
    chess->wcq             = wcq;
    chess->wck             = wck;
    chess->bcq             = bcq;
    chess->bck             = bck;
    chess->en_passant      = en_passant;
    chess->halfmove_clock  = halfmove_clock;
    chess->fullmove_number = fullmove_number;

    return 0;
}

int chess_make_move(chess_t *chess, move_t move) {
    if (!chess) return 1;

    /* Validate color to move */
    piece_t piece_from = board_piece_at(&chess->board, move.from);
    if (piece_is_type(piece_from, PIECE_TYPE_NONE)) return 1;
    if (!piece_is_color(piece_from, chess->color_to_move)) return 1;

    if (board_make_move(&chess->board, move)) return 1;
    chess->color_to_move = !chess->color_to_move;
    return 0;
}
