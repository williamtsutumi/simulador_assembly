CC=gcc
DBG_FLAGS=-O2 -fsanitize=address -fsanitize=undefined -fno-sanitize-recover -fstack-protector -D_FORTIFY_SOURCE=2 -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -Wcast-align -Wlogical-op -Wshadow -Wno-unused-result -Wformat=2
OPT_FLAGS=-O3

.PHONY: all clean

all: clean simulator

simulator: simulator.c
	$(CC) $(OPT_FLAGS) $^ -o $@

debug: simulator.c
	$(CC) $(DBG_FLAGS) $^ -o simulator

clean:
	rm -f simulator
