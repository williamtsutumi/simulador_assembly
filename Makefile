CC=gcc
CFLAGS=-O2 -fsanitize=address -fsanitize=undefined -fno-sanitize-recover -fstack-protector -D_FORTIFY_SOURCE=2 -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -Wcast-align -Wlogical-op -Wshadow -Wno-unused-result -Wformat=2

simulator: simulator.c
	$(CC) $(CFLAGS) simulator.c -o simulator

.PHONY: clean

clean:
	rm -f simulator
