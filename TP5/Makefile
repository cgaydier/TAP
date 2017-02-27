CC = gcc
CFLAGS = -Wall -g -Wno-unused-function -Wno-deprecated-declarations
LDFLAGS = -lm
.PHONY: clean

ifeq ($(shell uname -s), Darwin)
    LF = Library/Frameworks
    LDFLAGS += -framework OpenGL -framework GLUT -framework SDL2
    LDFLAGS += -F$(HOME)/$(LF)	
    LDFLAGS += -F/$(LF)
    IFLAGS += -I$(HOME)/$(LF)/SDL2.framework/Headers/
    IFLAGS += -I/$(LF)/SDL2.framework/Headers/
else
    LDFLAGS += -lglut -lGLU -lGL -lSDL2
endif

a_star: a_star.c utils.c heap.c
	$(CC) $(CFLAGS) $(IFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm a_star
