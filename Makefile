.POSIX:

SRC = src/piece.c src/square.c src/board.c src/chess.c src/move.c
OBJ = $(SRC:.c=.o)

CFLAGS = -Wall -Wextra -pedantic -Iinclude -O3
AR = ar
ARFLAGS = rcs

.PHONY: all
all: libchess.a

.PHONY: clean
clean:
	rm -f $(OBJ) libchess.a
	make -C demos/raylib clean 

.PHONY: run
run: all
	make -C demos/raylib run

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

libchess.a: $(OBJ)
	$(AR) $(ARFLAGS) $@ $(OBJ)
