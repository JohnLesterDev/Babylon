CC = gcc
BASE_CFLAGS = -Wall -Wextra `sdl2-config --cflags` -I./src -I./include -std=c11
LDFLAGS = `sdl2-config --libs`
RELEASE_CFLAGS = $(BASE_CFLAGS) -Werror -march=native -flto -O3 -DNDEBUG
CFLAGS = $(BASE_CFLAGS) -g

SRC_DIR = src
BIN_DIR = bin
BUILD_DIR = build

PROJECT_NAME = babylon

OBJ_DIR = $(BUILD_DIR)/obj
ASM_DIR = $(BUILD_DIR)/asm

SRC = $(shell find $(SRC_DIR) -name '*.c')
OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))
OUT = $(BIN_DIR)/$(PROJECT_NAME)

all: $(OUT)

$(OUT): $(OBJ)
	mkdir -p $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

release: CFLAGS = $(RELEASE_CFLAGS)
release: clean $(OUT)
	strip $(OUT)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

asm: CFLAGS += -S
asm: clean
asm: $(patsubst $(SRC_DIR)/%.c, $(ASM_DIR)/%.s, $(SRC))

$(ASM_DIR)/%.s: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $<

run: all
	./$(OUT) $(ARGS)

.PHONY: all release clean asm run
