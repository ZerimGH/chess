#include "chess.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* https://chessprogramming.org/Perft_Results */

#define DEFAULT_DEPTH 5

typedef struct {
    const char *name;
    const char *fen;
    uint64_t expected[5]; /* Only 5, since that's
                             the maximum of each test listed */
} perft_t;

const perft_t perfts[] = {
    {"Initial position",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        {20, 400, 8902, 197281, 4865609}},
    {"Position 2",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        {48, 2039, 97862, 4085603, 193690690}},
    {"Position 3",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        {14, 191, 2812, 43238, 674624}},
    {"Position 4",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        {6, 264, 9467, 422333, 15833292}},
    {"Position 5",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        {44, 1486, 62379, 2103487, 89941194}},
    {"Position 6",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 "
        "10",
        {46, 2079, 89890, 3894594, 164075551}},
};

uint64_t perft(chess_t *chess, int depth) {
    if (!chess) return 0;
    if (depth == 1) return chess->moves_len;

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
    if (argc > 1) {
        depth = atoi(argv[1]);
        if (depth < 1) depth = DEFAULT_DEPTH;
    }

    for (size_t i = 0; i < sizeof(perfts) / sizeof(perft_t); i++) {
        perft_t test = perfts[i];
        printf("Running test %s\n", test.name);
        printf("Depth | Nodes          | Time taken (ms)\n");
        for (int j = 1; j <= depth; j++) {
            chess_t chess = chess_new();
            if (chess_load_fen(&chess, test.fen)) {
                fprintf(stderr, "Failed to load position %s\n", test.fen);
                return 1;
            }

            struct timespec start, end;
            timespec_get(&start, TIME_UTC);
            uint64_t nodes = perft(&chess, j);
            timespec_get(&end, TIME_UTC);

            long ms_taken = (end.tv_sec - start.tv_sec) * 1000L
                            + (end.tv_nsec - start.tv_nsec) / 1000000L;

            if (nodes != test.expected[j - 1]) {
                fprintf(stderr,
                    "Perft test failed on position %s, %s at depth %d\n",
                    test.name,
                    test.fen,
                    j);
                fprintf(stderr,
                    "Expected: %lu, got %lu\n",
                    test.expected[j - 1],
                    nodes);
                return 1;
            }

            printf("%-5d | %-14lu | %ld\n", j, nodes, ms_taken);
        }
        printf("Passed! :)\n\n");
    }
    return 0;
}
