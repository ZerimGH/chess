TARGET = ./main
SRC = src/main.c
OBJ = $(SRC:.c=.o)

CFLAGS = -Wall -Wextra -pedantic -std=c23 -I../../include
LDFLAGS = -L../..
LDLIBS = -lchess

LIBCHESS = ../../libchess.a

PERFT_DEPTH ?= 5

.PHONY: all
all: $(TARGET)

.PHONY: run
run: all
	$(TARGET) $(PERFT_DEPTH)

.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)

$(TARGET): $(OBJ) $(LIBCHESS)
	$(CC) $(LDFLAGS) $(OBJ) -o $@ $(LDLIBS)

$(LIBCHESS):
	$(MAKE) -C ../..

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
