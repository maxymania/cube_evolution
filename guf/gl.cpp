#include <GUF/guf.h>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/constants.hpp> // glm::pi
#include <glm/gtc/type_ptr.hpp>
#include "glb.h"

#ifdef USE_DESKTOP_GL
void gufDumpMatrixf(float * m){
	GLint v;
	glGetIntegerv(GL_MATRIX_MODE,&v);
	switch(v) {
	case GL_MODELVIEW:  glGetFloatv(GL_MODELVIEW_MATRIX ,m); break;
	case GL_PROJECTION: glGetFloatv(GL_PROJECTION_MATRIX,m); break;
	case GL_TEXTURE:    glGetFloatv(GL_TEXTURE_MATRIX   ,m); break;
	case GL_COLOR:      glGetFloatv(GL_COLOR_MATRIX     ,m); break;
	}
}
void gufLoadMatrixf(const float * m) {
	glLoadMatrixf(m);
}
void gufMultMatrixf(const float * m) {
	glMultMatrixf(m);
}
void gufMatrixMode(GUFenum mode) {
	switch(mode) {
	case GUF_MODELVIEW:  glMatrixMode(GL_MODELVIEW); break;
	case GUF_PROJECTION: glMatrixMode(GL_PROJECTION); break;
	}
}
#endif

