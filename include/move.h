#ifndef MOVE_H
#define MOVE_H

#include "piece.h"
#include "square.h"

typedef struct {
    square_t from;
    square_t to;
    uint8_t promote_to; /* type of piece */
} move_t;

#endif /* MOVE_H */
