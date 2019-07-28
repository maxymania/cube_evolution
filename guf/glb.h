#pragma once

#define USE_DESKTOP_GL

#ifdef USE_DESKTOP_GL
#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glext.h>
#else
#include <GLES2/gl2.h>
#endif

