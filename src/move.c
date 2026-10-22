#include "move.h"

int move_equals(move_t a, move_t b) {
    return square_equals(a.from, b.from) && square_equals(a.to, b.to)
           && a.promote_to == b.promote_to;
}
