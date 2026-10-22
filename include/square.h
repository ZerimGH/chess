#ifndef SQUARE_H
#define SQUARE_H

#include <stdint.h>

typedef struct {
    uint8_t col; /* 0 = a, 7 = h */
    uint8_t row; /* 0 = 1, 7 = 8 */
} square_t;

uint8_t square_to_idx(square_t square);
int square_equals(square_t a, square_t b);

#endif /* SQUARE_H */
