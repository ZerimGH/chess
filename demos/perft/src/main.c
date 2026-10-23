#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "chess.h"

#define DEFAULT_DEPTH 5

uint64_t perft(chess_t *chess, int depth) {
    if (!chess) return 0;
    if (depth == 0) return 1;

    uint64_t tot = 0;

    for (size_t i = 0; i < chess->moves_len; i++) {
        move_t move = chess->moves[i];
        chess_t copy;
        memcpy(&copy, chess, sizeof(chess_t));
        if (chess_make_move(&copy, move)) continue;
        tot += perft(&copy, depth - 1);
    }

    return tot;
}

int main(int argc, char **argv) {
    int depth = DEFAULT_DEPTH;
    if (argc > 1)  {
        depth = atoi(argv[1]);
        if (depth < 1) depth = DEFAULT_DEPTH;
    }
    chess_t chess = chess_new();
    printf("perft(%d) = %lu\n", depth, perft(&chess, depth));
    return 0;
}
