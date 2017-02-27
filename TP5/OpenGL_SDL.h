#pragma once
#if defined (__APPLE__)
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <GLUT/glut.h>
	#include <SDL.h>
#else
	#include <GL/glu.h>
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_opengl.h>
#endif
