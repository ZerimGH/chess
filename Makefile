.POSIX:

SRC = src/piece.c src/square.c src/board.c src/chess.c src/move.c
OBJ = $(SRC:.c=.o)

CFLAGS = -Wall -Wextra -pedantic -Iinclude -O3
AR = ar
ARFLAGS = rcs

PERFT_DEPTH ?= 5 

.PHONY: all
all: libchess.a

.PHONY: clean
clean:
	rm -f $(OBJ) libchess.a
	make -C demos/raylib clean 
	make -C demos/perft clean 

.PHONY: run
run: all
	make -C demos/raylib run

.PHONY: perft
perft: all
	time make -C demos/perft run PERFT_DEPTH=$(PERFT_DEPTH)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

libchess.a: $(OBJ)
	$(AR) $(ARFLAGS) $@ $(OBJ)
