/*
  Copyright (C) 2019 Simon Schmidt

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include <GUF/sdl.h>
#include "gles2.h"
#include <stdlib.h>

#ifndef USE_DESKTOP_GL

static EGLDisplay egl_display;
static EGLSurface egl_surface;
static EGLContext egl_context;

extern EGLNativeDisplayType geoSDL_EGL_nativeDisplay(SDL_Window *window);
extern EGLNativeWindowType geoSDL_EGL_nativeWindow(SDL_Window *window);
extern EGLNativeWindowType geoSDL_EGL_nativeWindow2(SDL_Window *window);

static bool initializeEgl(SDL_Window *window)
{
	EGLint attribList[]={
		EGL_CONFORMANT  , EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_DEPTH_SIZE, 24,
		EGL_NONE,
	};
	EGLint contextAttribs[]={
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE,
	};
	
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	
	EGLConfig config;
	EGLint numConfigs,majorVersion,minorVersion;
	
	display = eglGetDisplay(geoSDL_EGL_nativeDisplay(window));
	if(!display) return false;
	if(!eglInitialize(display, &majorVersion, &minorVersion)) return false;
	
	if(!eglGetConfigs(display, NULL, 0, &numConfigs)) return false;
	if(!eglChooseConfig(display, attribList, &config, 1, &numConfigs)) return false;
	if(numConfigs<1) return false;
	
	surface = eglCreateWindowSurface(display, config, geoSDL_EGL_nativeWindow(window), NULL);
	if(surface == EGL_NO_SURFACE)
		surface = eglCreateWindowSurface(display, config, geoSDL_EGL_nativeWindow2(window), NULL);
	
	if(surface == EGL_NO_SURFACE) return false;
	
	context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
	if(!context) return false;
	if(!eglMakeCurrent(display, surface, surface, context)) return false;
	
	egl_display = display;
	egl_surface = surface;
	egl_context = context;
	
	return true;
}

void gufSDL_GL_Context(SDL_Window *window) {
	if(!initializeEgl(window)) { _Exit(1); return; } // TODO: catch!
	gufgles2::init();
	gufgles2::init2();
	// Don't need to. Is already current!
	//eglMakeCurrent(egl_display,egl_surface,egl_surface,egl_context);
	
	//sdl_context = SDL_GL_CreateContext(window);
	//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
}

void gufSDL_GL_SwapBuffers(SDL_Window *window) {
	eglWaitGL();
	eglSwapBuffers(egl_display,egl_surface);
}

#endif

