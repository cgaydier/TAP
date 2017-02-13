CC = gcc
CFLAGS = -Wall -g -Wno-unused-function
LDLIBS = -lSDL2 -lGL -lGLU -lm

tsp: tsp.c utils.c
	$(CC) $(CFLAGS) $(LDLIBS) $^ -o $@

tsp_approx: tsp_approx.c utils.c
	$(CC) $(CFLAGS) $(LDLIBS) $^ -o $@

test_heap: test_heap.c heap.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm tsp
	rm tsp_approx
	rm test_heap heap
