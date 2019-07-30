#include <GUF/sdl.h>
#include "glb.h"

#ifdef USE_DESKTOP_GL

static SDL_GLContext sdl_context;

void gufSDL_GL_Context(SDL_Window *window) {
	sdl_context = SDL_GL_CreateContext(window);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
}

void gufSDL_GL_SwapBuffers(SDL_Window *window) {
	SDL_GL_SwapWindow(window);
}

#endif

