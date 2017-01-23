CC = gcc
CFLAGS = -Wall -g
LDLIBS = -lSDL2 -lGL -lGLU -lm

all: tsp

tsp: tsp.c utils.c

clean: rm tsp
