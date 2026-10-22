#include "piece.h"
#include <ctype.h>
#include <string.h>

static const char *chars = {"pnbrqk"};

char piece_to_char(piece_t piece) {
    int color = PIECE_COLOR(piece);
    int type  = PIECE_TYPE(piece);
    if (type >= (int)strlen(chars)) return ' ';
    char c = chars[type];
    return color ? toupper(c) : c;
}

piece_t piece_from_char(char c) {
    if (!c) return PIECE(PIECE_COLOR_WHITE, PIECE_TYPE_NONE);
    int is_white = isupper(c) ? PIECE_COLOR_WHITE : PIECE_COLOR_BLACK;
    char *pos    = strchr(chars, tolower(c));
    if (!pos) return PIECE(PIECE_COLOR_WHITE, PIECE_TYPE_NONE);
    return PIECE(is_white, (uint8_t)(pos - chars));
}

int piece_is_color(piece_t piece, uint8_t col) {
    if (col) col = 0x01;
    return PIECE_COLOR(piece) == col;
}

int piece_is_type(piece_t piece, uint8_t type) {
    uint8_t pt = PIECE_TYPE(piece);
    if (pt > PIECE_TYPE_KING) pt = PIECE_TYPE_NONE;
    return pt == type;
}
