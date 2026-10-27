TARGET = ./main
SRC = src/main.c
OBJ = $(SRC:.c=.o)

TEXTURE_SRC_DIR = ./cburnett
TEXTURES = ./textures
TEXTURE_SRCS = $(shell find $(TEXTURE_SRC_DIR) -type f -name '*.svg')
TEXTURE_TARGETS = $(patsubst $(TEXTURE_SRC_DIR)/%.svg,$(TEXTURES)/%.png,$(TEXTURE_SRCS))

CFLAGS = -Wall -Wextra -pedantic -std=c99 -I../../include
LDFLAGS = -L../..
LDLIBS = -lchess -lraylib

LIBCHESS = ../../libchess.a

.PHONY: all
all: $(TARGET) $(TEXTURE_TARGETS)

.PHONY: run
run: all
	$(TARGET)

.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET) $(TEXTURE_TARGETS)

$(TARGET): $(OBJ) $(LIBCHESS)
	$(CC) $(LDFLAGS) $(OBJ) -o $@ $(LDLIBS)

$(LIBCHESS):
	$(MAKE) -C ../..

$(TEXTURES):
	mkdir -p $@

$(TEXTURES)/%.png: $(TEXTURE_SRC_DIR)/%.svg | $(TEXTURES)
	magick -background none -density 384 $< -resize 512x512 $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
