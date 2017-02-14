CC = gcc
CFLAGS = -Wall -g -Wno-unused-function
LDLIBS = -lSDL2 -lGL -lGLU -lm

test_heap: test_heap.c heap.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm test_heap heap
