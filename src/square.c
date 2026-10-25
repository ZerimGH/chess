#include "square.h"

uint8_t square_to_idx(square_t square) {
    if (square.row >= 8 || square.col >= 8) return 0xFF;
    return square.row + square.col * 8;
}

int square_equals(square_t a, square_t b) {
    return a.col == b.col && a.row == b.row;
}
