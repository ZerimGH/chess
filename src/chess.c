#include "chess.h"
#include "move.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ABS(a) (a < 0 ? -a : a)
#define INIT_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

static void chess_generate_moves(chess_t *chess);

chess_t chess_new(void) {
    chess_t chess;
    memset(&chess, 0, sizeof(chess_t));
    chess.board      = board_empty();
    chess.en_passant = (square_t){0xFF, 0xFF};
    /* This should probably never fail */
    chess_load_fen(&chess, INIT_FEN);
    return chess;
}

int chess_load_fen(chess_t *chess, const char *fen) {
    if (!chess || !fen) return 1;

    board_t new_board = board_empty();
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
    if (v > INT_MAX || v < 0) return 1;
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
    if (v > INT_MAX || v < 1) return 1;
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

    chess_generate_moves(chess);

    return 0;
}

static int square_is_valid(square_t square) {
    return square.row < 8 && square.col < 8;
}

static int square_is_empty(chess_t *chess, square_t square) {
    if (!chess) return 0;
    piece_t piece = board_piece_at(&chess->board, square);
    return piece_is_type(piece, PIECE_TYPE_NONE);
}

static int square_is_color(chess_t *chess, square_t square, int col) {
    if (!chess) return 0;
    piece_t piece = board_piece_at(&chess->board, square);
    return piece_is_color(piece, col);
}

static square_t chess_find_king(chess_t *chess) {
    int found         = 0;
    piece_t king      = PIECE(chess->color_to_move, PIECE_TYPE_KING);
    square_t king_pos = (square_t){0xFF, 0xFF};
    for (king_pos.col = 0; king_pos.col < 8; king_pos.col++) {
        for (king_pos.row = 0; king_pos.row < 8; king_pos.row++) {
            piece_t piece = board_piece_at(&chess->board, king_pos);
            if (piece == king) {
                found = 1;
                break;
            }
        }
        if (found) break;
    }

    if (!found) return (square_t){0xFF, 0xFF};
    return king_pos;
}

static int pawn_attacks(
    chess_t *chess, square_t square, square_t attacker) {
    int d      = chess->color_to_move ? -1 : 1;
    square_t l = (square_t){attacker.col - 1, attacker.row + d};
    square_t r = (square_t){attacker.col + 1, attacker.row + d};
    return (square_is_valid(l) && square_equals(l, square))
           || (square_is_valid(r) && square_equals(r, square));
}

static int knight_attacks(chess_t *chess, square_t square, square_t attacker) {
    (void)chess;

    int a1 = 1;
    int b1 = 2;
    int a2 = 2;
    int b2 = 1;

    for (int i = 0; i < 4; i++) {
        square_t new1 = (square_t){attacker.col + a1, attacker.row + b1};
        square_t new2 = (square_t){attacker.col + a2, attacker.row + b2};

        if (square_is_valid(new1) && square_equals(new1, square)) return 1;
        if (square_is_valid(new2) && square_equals(new2, square)) return 1;

        int oa1 = a1;
        a1      = -b1;
        b1      = oa1;

        int oa2 = a2;
        a2      = -b2;
        b2      = oa2;
    }

    return 0;
}

static int slide_attacks(
    chess_t *chess, square_t square, square_t attacker, int a, int b) {
    for (int i = 0; i < 4; i++) {
        square_t new = attacker;
        do {
            new.col += a;
            new.row += b;
            if (square_is_valid(new)) {
                if (!square_is_empty(chess, new)) {
                    if (square_equals(new, square)) return 1;
                    break;
                }

                if (square_equals(new, square)) return 1;
            }
        } while (square_is_valid(new));

        int oa = a;
        a      = -b;
        b      = oa;
    }

    return 0;
}

static int sliding_piece_attacks(chess_t *chess, square_t square,
    square_t attacker, int straight, int diagonal) {
    if (straight && slide_attacks(chess, square, attacker, 1, 0)) return 1;
    if (diagonal && slide_attacks(chess, square, attacker, 1, 1)) return 1;

    return 0;
}

static int king_attacks(chess_t *chess, square_t square, square_t attacker) {
    (void)chess;
    int dx = (int)square.col - (int)attacker.col;
    int dy = (int)square.row - (int)attacker.row;

    if (dx == 0 && dy == 0) return 0;

    return ABS(dx) < 2 && ABS(dy) < 2;
}

static int square_attacked(chess_t *chess, square_t square) {
    if (!chess) return 0;

    square_t attacker = (square_t){0x00, 0x00};
    for (attacker.col = 0; attacker.col < 8; attacker.col++) {
        for (attacker.row = 0; attacker.row < 8; attacker.row++) {
            piece_t piece = board_piece_at(&chess->board, attacker);
            if (piece_is_type(piece, PIECE_TYPE_NONE)) continue;
            if (piece_is_color(piece, chess->color_to_move)) continue;
            if (square_equals(attacker, square)) continue;

            switch (PIECE_TYPE(piece)) {
            case PIECE_TYPE_PAWN:
                if (pawn_attacks(chess, square, attacker)) return 1;
                break;
            case PIECE_TYPE_KNIGHT:
                if (knight_attacks(chess, square, attacker)) return 1;
                break;
            case PIECE_TYPE_BISHOP:
                if (sliding_piece_attacks(chess, square, attacker, 0, 1))
                    return 1;
                break;
            case PIECE_TYPE_ROOK:
                if (sliding_piece_attacks(chess, square, attacker, 1, 0))
                    return 1;
                break;
            case PIECE_TYPE_QUEEN:
                if (sliding_piece_attacks(chess, square, attacker, 1, 1))
                    return 1;
                break;
            case PIECE_TYPE_KING:
                if (king_attacks(chess, square, attacker)) return 1;
                break;
            }
        }
    }

    return 0;
}

static int chess_is_check(chess_t *chess) {
    if (!chess) return 0;
    square_t king_pos = chess_find_king(chess);
    if (!square_is_valid(king_pos)) return 0;
    return square_attacked(chess, king_pos);
}

static chess_t chess_copy_cheap(chess_t *chess) {
    chess_t copy;
    copy.board           = chess->board;
    copy.color_to_move   = chess->color_to_move;
    copy.wcq             = chess->wcq;
    copy.wck             = chess->wck;
    copy.bcq             = chess->bcq;
    copy.bck             = chess->bck;
    copy.en_passant      = chess->en_passant;
    copy.halfmove_clock  = chess->halfmove_clock;
    copy.fullmove_number = chess->fullmove_number;
    copy.moves_len       = 0;
    return copy;
}

/*
static chess_t chess_copy_deep(chess_t *chess) {
    chess_t copy;
    memcpy(&copy, chess, sizeof(chess_t));
    return copy;
}
*/

static void append_move(chess_t *chess, move_t move) {
    if (!chess) return;
    if (chess->moves_len >= CHESS_MAX_MOVES) return;

    /* Check if move results in check, don't append if it does */
    chess_t copy = chess_copy_cheap(chess);
    if (chess_make_pseudo_move(&copy, move)) return;
    if (chess_is_check(&copy)) return;

    chess->moves[chess->moves_len++] = move;
}

static void generate_pawn_moves(chess_t *chess, square_t square) {
    if (!chess) return;
    int d           = chess->color_to_move ? 1 : -1;
    int start_row   = chess->color_to_move ? 1 : 6;
    int promote_row = chess->color_to_move ? 7 : 0;

    if (square.row == promote_row) return;

    /* I think overflows or underflows are fine here */
    square_t f1 = (square_t){square.col, square.row + d};
    square_t f2 = (square_t){square.col, square.row + d * 2};
    square_t l  = (square_t){square.col - 1, square.row + d};
    square_t r  = (square_t){square.col + 1, square.row + d};

    /* Forwards pushes */
    if (square_is_valid(f1)) {
        if (square_is_empty(chess, f1)) {
            /* Forward 1 */
            if (f1.row == promote_row) {
                append_move(chess, (move_t){square, f1, PIECE_TYPE_KNIGHT});
                append_move(chess, (move_t){square, f1, PIECE_TYPE_BISHOP});
                append_move(chess, (move_t){square, f1, PIECE_TYPE_ROOK});
                append_move(chess, (move_t){square, f1, PIECE_TYPE_QUEEN});
            } else {
                append_move(chess, (move_t){square, f1, PIECE_TYPE_NONE});
            }

            /* Forward 2 */
            if (square.row == start_row && square_is_valid(f2)) {
                if (square_is_empty(chess, f2)) {
                    append_move(chess, (move_t){square, f2, PIECE_TYPE_NONE});
                }
            }
        }
    }

    /* Diagonal captures */
    if (square_is_valid(l)) {
        if ((!square_is_empty(chess, l)
                && square_is_color(chess, l, !chess->color_to_move))
            || square_equals(l, chess->en_passant)) {
            if (l.row == promote_row) {
                append_move(chess, (move_t){square, l, PIECE_TYPE_KNIGHT});
                append_move(chess, (move_t){square, l, PIECE_TYPE_BISHOP});
                append_move(chess, (move_t){square, l, PIECE_TYPE_ROOK});
                append_move(chess, (move_t){square, l, PIECE_TYPE_QUEEN});
            } else {
                append_move(chess, (move_t){square, l, PIECE_TYPE_NONE});
            }
        }
    }

    if (square_is_valid(r)) {
        if ((!square_is_empty(chess, r)
                && square_is_color(chess, r, !chess->color_to_move))
            || square_equals(r, chess->en_passant)) {
            if (r.row == promote_row) {
                append_move(chess, (move_t){square, r, PIECE_TYPE_KNIGHT});
                append_move(chess, (move_t){square, r, PIECE_TYPE_BISHOP});
                append_move(chess, (move_t){square, r, PIECE_TYPE_ROOK});
                append_move(chess, (move_t){square, r, PIECE_TYPE_QUEEN});
            } else {
                append_move(chess, (move_t){square, r, PIECE_TYPE_NONE});
            }
        }
    }
}

static void slide(chess_t *chess, square_t square, int a, int b) {
    for (int i = 0; i < 4; i++) {
        square_t new = square;
        do {
            new.col += a;
            new.row += b;
            if (square_is_valid(new)) {
                if (!square_is_empty(chess, new)) {
                    if (square_is_color(chess, new, !chess->color_to_move)) {
                        append_move(
                            chess, (move_t){square, new, PIECE_TYPE_NONE});
                    }
                    break;
                }

                append_move(chess, (move_t){square, new, PIECE_TYPE_NONE});
            }
        } while (square_is_valid(new));

        int oa = a;
        a      = -b;
        b      = oa;
    }
}

static void generate_sliding_moves(
    chess_t *chess, square_t square, int straight, int diagonal) {
    if (!chess) return;

    if (straight) slide(chess, square, 1, 0);
    if (diagonal) slide(chess, square, 1, 1);
}

static void generate_knight_moves(chess_t *chess, square_t square) {
    if (!chess) return;
    int a1 = 1;
    int b1 = 2;
    int a2 = 2;
    int b2 = 1;

    for (int i = 0; i < 4; i++) {
        square_t new1 = (square_t){square.col + a1, square.row + b1};
        square_t new2 = (square_t){square.col + a2, square.row + b2};

        if (square_is_valid(new1)) {
            if (square_is_empty(chess, new1)
                || square_is_color(chess, new1, !chess->color_to_move)) {
                append_move(chess, (move_t){square, new1, PIECE_TYPE_NONE});
            }
        }

        if (square_is_valid(new2)) {
            if (square_is_empty(chess, new2)
                || square_is_color(chess, new2, !chess->color_to_move)) {
                append_move(chess, (move_t){square, new2, PIECE_TYPE_NONE});
            }
        }

        int oa1 = a1;
        a1      = -b1;
        b1      = oa1;

        int oa2 = a2;
        a2      = -b2;
        b2      = oa2;
    }
}

static void generate_king_moves(chess_t *chess, square_t square) {
    if (!chess) return;

    /* Regular moves */
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            square_t new = (square_t){square.col + i, square.row + j};
            if (!square_is_valid(new)) continue;
            if (!(square_is_empty(chess, new)
                    || square_is_color(chess, new, !chess->color_to_move)))
                continue;
            append_move(chess, (move_t){square, new, PIECE_TYPE_NONE});
        }
    }

    /* Castle */
    if (chess_is_check(chess)) return; /* Can't castle when in check */

    uint8_t row = chess->color_to_move ? 0 : 7;
    int ck      = chess->color_to_move ? chess->wck : chess->bck;
    int cq      = chess->color_to_move ? chess->wcq : chess->bcq;

    /* Kingside, check squares f -> g */
    square_t ck_square = (square_t){5, row};
    if (ck) {
        for (; ck_square.col < 7; ck_square.col++) {
            if (!square_is_empty(chess, ck_square)
                || square_attacked(chess, ck_square)) {
                ck = 0;
                break;
            }
        }

        ck_square.col--;
    }

    /* Queenside, check squares c -> d */
    square_t cq_square = (square_t){3, row};
    if (cq) {
        for (; cq_square.col > 1; cq_square.col--) {
            if (!square_is_empty(chess, cq_square)
                || square_attacked(chess, cq_square)) {
                cq = 0;
                break;
            }
        }

        cq_square.col++;
    }

    if (ck) append_move(chess, (move_t){square, ck_square, PIECE_TYPE_NONE});
    if (cq) append_move(chess, (move_t){square, cq_square, PIECE_TYPE_NONE});
}

static void chess_generate_moves(chess_t *chess) {
    if (!chess) return;

    chess->moves_len = 0;

    square_t square;
    for (square.col = 0; square.col < 8; square.col++) {
        for (square.row = 0; square.row < 8; square.row++) {
            piece_t piece = board_piece_at(&chess->board, square);
            if (PIECE_TYPE(piece) == PIECE_TYPE_NONE) continue;
            if (PIECE_COLOR(piece) != chess->color_to_move) continue;

            switch (PIECE_TYPE(piece)) {
            case PIECE_TYPE_PAWN:   generate_pawn_moves(chess, square); break;
            case PIECE_TYPE_KNIGHT: generate_knight_moves(chess, square); break;
            case PIECE_TYPE_BISHOP:
                generate_sliding_moves(chess, square, 0, 1);
                break;
            case PIECE_TYPE_ROOK:
                generate_sliding_moves(chess, square, 1, 0);
                break;
            case PIECE_TYPE_QUEEN:
                generate_sliding_moves(chess, square, 1, 1);
                break;
            case PIECE_TYPE_KING: generate_king_moves(chess, square); break;
            }
        }
    }
}

int chess_make_pseudo_move(chess_t *chess, move_t move) {
    if (!chess) return 1;
    if (!square_is_valid(move.from) || !square_is_valid(move.to)) return 1;

    piece_t piece_moved = board_piece_at(&chess->board, move.from);
    if (piece_is_type(piece_moved, PIECE_TYPE_NONE)) return 1;

    if (board_make_move(&chess->board, move)) return 1;

    square_t new_en_passant = (square_t){0xFF, 0xFF};

    /* Special handling for pawn moves */
    if (piece_is_type(piece_moved, PIECE_TYPE_PAWN)) {
        /* Set en passant square on 2 push */
        int dy = (int)move.to.row - (int)move.from.row;
        int dx = (int)move.to.col - (int)move.from.col;
        if (ABS(dy) == 2) {
            int sign       = dy > 0 ? 1 : -1;
            new_en_passant = (square_t){move.from.col, move.from.row + sign};
        }

        /* Remove piece below for en passant */
        if (ABS(dx) == 1 && ABS(dy) == 1
            && square_equals(move.to, chess->en_passant)) {
            square_t captured = (square_t){
                chess->en_passant.col, chess->en_passant.row - dy};
            if (square_is_valid(captured))
                board_set_square(&chess->board,
                    captured,
                    PIECE(PIECE_COLOR_WHITE, PIECE_TYPE_NONE));
        }
    }

    chess->en_passant = new_en_passant;

    /* Special handling for castling */
    if (piece_is_type(piece_moved, PIECE_TYPE_KING)) {
        int dx      = (int)move.to.col - (int)move.from.col;
        uint8_t row = PIECE_COLOR(piece_moved) ? 0 : 7;
        if (ABS(dx) > 1) {
            int sign = dx > 0;
            if (sign) {
                /* Kingside */
                board_set_square(&chess->board,
                    (square_t){7, row},
                    PIECE(PIECE_COLOR_WHITE, PIECE_TYPE_NONE));
                board_set_square(&chess->board,
                    (square_t){5, row},
                    PIECE(PIECE_COLOR(piece_moved), PIECE_TYPE_ROOK));
            } else {
                /* Queenside */
                board_set_square(&chess->board,
                    (square_t){0, row},
                    PIECE(PIECE_COLOR_WHITE, PIECE_TYPE_NONE));
                board_set_square(&chess->board,
                    (square_t){3, row},
                    PIECE(PIECE_COLOR(piece_moved), PIECE_TYPE_ROOK));
            }
        }
    }

    /* Special handling for moves that will remove castling rights */
    /* Just checking move from / to for rooks should be enough */
    if (PIECE_COLOR(piece_moved)) {
        /* King moves */
        if (piece_is_type(piece_moved, PIECE_TYPE_KING)) {
            chess->wcq = 0;
            chess->wck = 0;
        }

        /* Rook moves / captures */
        if (move.from.row == 0 && move.from.col == 0) chess->wcq = 0;
        if (move.from.row == 0 && move.from.col == 7) chess->wck = 0;
        if (move.to.row == 7 && move.from.col == 0) chess->bcq = 0;
        if (move.to.row == 7 && move.from.col == 7) chess->bck = 0;
    } else {
        /* King moves */
        if (piece_is_type(piece_moved, PIECE_TYPE_KING)) {
            chess->bcq = 0;
            chess->bck = 0;
        }

        /* Rook moves / captures */
        if (move.from.row == 7 && move.from.col == 0) chess->bcq = 0;
        if (move.from.row == 7 && move.from.col == 7) chess->bck = 0;
        if (move.to.row == 0 && move.from.col == 0) chess->wcq = 0;
        if (move.to.row == 0 && move.from.col == 7) chess->wck = 0;
    }

    return 0;
}

int chess_make_move(chess_t *chess, move_t move) {
    if (!chess) return 1;

    /* Check with legal moves */
    int found = 0;
    for (size_t i = 0; i < chess->moves_len; i++) {
        move_t check = chess->moves[i];
        if (move_equals(move, check)) {
            found = 1;
            break;
        }
    }
    if (!found) return 1;

    if (chess_make_pseudo_move(chess, move)) return 1;

    chess->color_to_move = !chess->color_to_move;

    chess_generate_moves(chess);

    return 0;
}
