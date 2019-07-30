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
#include "glb.h"

#ifndef USE_DESKTOP_GL
#include <SDL2/SDL_syswm.h>

#define caseof(x,y) case x:return (EGLNativeDisplayType)y
EGLNativeDisplayType geoSDL_EGL_nativeDisplay(SDL_Window *window) {
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (!SDL_GetWindowWMInfo(window,&info)) return 0;
	switch(info.subsystem){
	#ifdef SDL_VIDEO_DRIVER_WINDOWS
	caseof(SDL_SYSWM_WINDOWS,GetDC(info.info.win.hdc)); /* .win.hdc in >=SDL 2.0.4 */
	/*caseof(SDL_SYSWM_WINDOWS,GetDC(info.info.win.window));*/
	// TODO: implement GetDC();
	#endif
	#ifdef SDL_VIDEO_DRIVER_X11
	caseof(SDL_SYSWM_X11,info.info.x11.display);
	#endif
	#ifdef SDL_VIDEO_DRIVER_WAYLAND
	caseof(SDL_SYSWM_WAYLAND,info.info.wl.display); /* TODO: is that right? */
	#endif
	}
	return EGL_DEFAULT_DISPLAY;
}
#undef caseof

#define caseof(x,y) case x:return (EGLNativeWindowType)y
EGLNativeWindowType geoSDL_EGL_nativeWindow(SDL_Window *window) {
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (!SDL_GetWindowWMInfo(window,&info)) return 0;
	switch(info.subsystem){
	#ifdef SDL_VIDEO_DRIVER_WINDOWS
	caseof(SDL_SYSWM_WINDOWS,info.info.win.window);
	#endif
	#ifdef SDL_VIDEO_DRIVER_X11
	caseof(SDL_SYSWM_X11,info.info.x11.window);
	#endif
	#ifdef SDL_VIDEO_DRIVER_DIRECTFB
	caseof(SDL_SYSWM_DIRECTFB,info.info.dfb.window);
	#endif
	#ifdef SDL_VIDEO_DRIVER_COCOA
	caseof(SDL_SYSWM_COCOA,info.info.cocoa.window);
	#endif
	#ifdef SDL_VIDEO_DRIVER_UIKIT
	caseof(SDL_SYSWM_UIKIT,info.info.uikit.window);
	#endif
	#ifdef SDL_VIDEO_DRIVER_WAYLAND
	caseof(SDL_SYSWM_WAYLAND,info.info.wl.surface); /* TODO: is that right? */
	#endif
	#ifdef SDL_VIDEO_DRIVER_MIR
	/* I'd like to shoot MIR to the moon! */
	caseof(SDL_SYSWM_MIR,info.info.mir.surface);
	#endif
	}
	return 0;
}
#undef caseof

EGLNativeWindowType geoSDL_EGL_nativeWindow2(SDL_Window *window) {
	return 0;
}

#endif

