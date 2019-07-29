#pragma once

#define USE_DESKTOP_GL

#ifdef USE_DESKTOP_GL
#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glext.h>
#else
#include <GLES2/gl2.h>
// This feature is specific to OpenGL ES 2.0 (or later) and OpenGL 3.0 (or later).
#define HAS_glGenerateMipmap
#endif

