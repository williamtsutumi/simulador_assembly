CC = gcc
CFLAGS = -Iheaders -O3
DEBUG_CFLAGS = -O2 -Wall -fsanitize=address -fsanitize=undefined -fno-sanitize-recover -fstack-protector -D_FORTIFY_SOURCE=2 -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -Wcast-align -Wlogical-op -Wshadow -Wformat=2 -Wunused-parameter
SRC_DIR = src
OBJ_DIR = obj

# List all source files in the src directory
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)

# Generate object file names based on source file names
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

# Main target
all: simulator

# Rule to compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to build the simulator using object files
simulator: $(OBJ_FILES) simulator.c
	$(CC) $(CFLAGS) $^ -o $@

# Debug target to build with debug flags
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: all

# Clean rule to remove object files and the simulator executable
clean:
	rm -f $(OBJ_DIR)/*.o simulator

.PHONY: all debug clean
