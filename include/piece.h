#ifndef PIECE_H
#define PIECE_H

#include <stdint.h>

/* We have 6 piece types, pawn, knight, bishop, rook, queen, king */
/* 2 colours, white, black */
/* 1 bit for colour, 3 bits for piece type */
/* 4 bits total, round up to a byte */

typedef uint8_t piece_t;

/* The pieces type will be decided by the low 3 bits */
#define PIECE_TYPE(PIECE) (PIECE & 0x07)

enum {
    PIECE_TYPE_PAWN,
    PIECE_TYPE_KNIGHT,
    PIECE_TYPE_BISHOP,
    PIECE_TYPE_ROOK,
    PIECE_TYPE_QUEEN,
    PIECE_TYPE_KING,
    PIECE_TYPE_NONE,
};

enum {
    PIECE_COLOR_BLACK = 0x0,
    PIECE_COLOR_WHITE = 0x01,
};

/* The pieces color will be decided by the 4th bit */
#define PIECE_COLOR(PIECE) ((PIECE >> 3) & PIECE_COLOR_WHITE)

/* Create a piece */
#define PIECE(COLOR, TYPE)                                                       \
    (((COLOR & PIECE_COLOR_WHITE) << 3)                                          \
        | (TYPE > PIECE_TYPE_KING ? PIECE_TYPE_NONE : TYPE & 0x07))

char piece_to_char(piece_t piece);
piece_t piece_from_char(char c);
int piece_is_color(piece_t piece, uint8_t col);
int piece_is_type(piece_t piece, uint8_t type);

#endif /* PIECE_H */
