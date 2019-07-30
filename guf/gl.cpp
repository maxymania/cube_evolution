#include <string.h>
#include <GUF/guf.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#if 0
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/constants.hpp> // glm::pi
#include <glm/gtc/type_ptr.hpp>
#endif
#include "glb.h"

#ifdef USE_DESKTOP_GL
namespace {
// stupid function to cater for stupid ATI linux drivers that return incorrect depth values
float depthcorrect(float d)
{
	return (d<=1/256.0f) ? d*256 : d;
}
}

void gufReadDepthBuffer(int x,int y,float &depth){
	float localDepth;
	glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &localDepth);
	depth = depthcorrect(localDepth);
}

/* Matrix-stuff */
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
void gufDumpMatrixd(double * m){
	GLint v;
	glGetIntegerv(GL_MATRIX_MODE,&v);
	switch(v) {
	case GL_MODELVIEW:  glGetDoublev(GL_MODELVIEW_MATRIX ,m); break;
	case GL_PROJECTION: glGetDoublev(GL_PROJECTION_MATRIX,m); break;
	case GL_TEXTURE:    glGetDoublev(GL_TEXTURE_MATRIX   ,m); break;
	case GL_COLOR:      glGetDoublev(GL_COLOR_MATRIX     ,m); break;
	}
}
void gufDumpTypeMatrixf(int_u type, float * m){
	switch(type) {
	case GUF_MODELVIEW:  glGetFloatv(GL_MODELVIEW_MATRIX ,m); break;
	case GUF_PROJECTION: glGetFloatv(GL_PROJECTION_MATRIX,m); break;
	}
}
void gufDumpTypeMatrixd(int_u type, double * m){
	switch(type) {
	case GUF_MODELVIEW:  glGetDoublev(GL_MODELVIEW_MATRIX ,m); break;
	case GUF_PROJECTION: glGetDoublev(GL_PROJECTION_MATRIX,m); break;
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
bool gufCorrectTextureSize(int &xs,int &ys) { return false; }
void gufLoadIdentity(void) { glLoadIdentity(); }
void gufRotatef(float angle, float x, float y, float z) {
	// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glRotate.xml
	glRotatef(angle,x,y,z);
}
void gufTranslatef(float x, float y, float z) {
	// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glTranslate.xml
	glTranslatef(x,y,z);
}
void gufScalef(float x, float y, float z) {
	// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glScale.xml
	glScalef(x,y,z);
}
void gufOrtho(float left,  float right,  float bottom,  float top,  float nearVal,  float farVal){
	// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glOrtho.xml
	glOrtho(left,right,bottom,top,nearVal,farVal);
}
void gufPushMatrix( void) {
	// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glPushMatrix.xml
	glPushMatrix();
}
void gufPopMatrix( void) {
	glPopMatrix();
}

/* OpenGL Basic API. */
void gufBasicClearDepth(float f){
	glClearDepth(f);
}
void gufBasicEnableSmoothShadeModel(void){
	glShadeModel(GL_SMOOTH);
}
void gufBasicSmoothLineHint(void) {
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

/* OpenGL 1.1 Draw API for per-object lit geometry. */
void gufGeometrySetColor(float r,float g,float b,float a) {
	glColor4f(r,g,b,a);
}
void gufGeometryBegin(int_u gl_primitive){
	glBegin(gl_primitive);
}
void gufGeometryTexCoord2f(float u,float v){
	glTexCoord2f(u,v);
}
void gufGeometryNormal3f(float x,float y,float z){
	// We can silently ignore normals in this engine!
	//glNormal3f(x,y,z);
}
void gufGeometryVertex3f(float x,float y,float z){
	glVertex3f(x,y,z);
}
void gufGeometryEnd(void){
	glEnd();
}
/* List equivalents. */
void gufGeometryNewList(int_u list){
	glNewList(list, GL_COMPILE);
}
void gufGeometryEndList(void){
	glEndList();
}
void gufGeometryCallList(int_u list){
	glCallList(list);
}


/* OpenGL 1.1 / ES1 API for per-vertex lit level geometry. */
void gufLevelEnableClientState(void) {
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);	
}
void gufLevelVertexPtr3f(int_u stride,const void* ptr) {
	glVertexPointer(3, GL_FLOAT, stride, ptr);
}
void gufLevelColorPtr4b(int_u stride,const void* ptr) {
	glColorPointer(4, GL_UNSIGNED_BYTE, stride, ptr);
}
void gufLevelTexCoordPtr2f(int_u stride,const void* ptr) {
	glTexCoordPointer(2, GL_FLOAT, stride, ptr);
}
void gufLevelDrawArrays(int_u mode,  int first,  int_u count){
	glDrawArrays(mode,first,count);
}

/* OpenGL 1.1 Enable/Disable substitute */
#define ENDIS(x) if(enabled) glEnable(x); else glDisable(x); break
void gufSetEnabled(int_u feature,bool enabled) {
	switch(feature) {
	case GUF_TEXTURE_2D: ENDIS(GL_TEXTURE_2D);
	case GUF_FOG: ENDIS(GL_FOG);
	case GUF_LINE_SMOOTH: ENDIS(GL_LINE_SMOOTH);
	case GUF_POLYGON_OFFSET_LINE: ENDIS(GL_POLYGON_OFFSET_LINE);
	case GUF_WIREFRAME: glPolygonMode(GL_FRONT_AND_BACK,enabled?GL_LINE:GL_FILL);
	}
}
#undef ENDIS

/* Overbright*/
bool gufOverbrightSupported(void){
	const char *exts = (const char *)glGetString(GL_EXTENSIONS);
	if(strstr(exts, "GL_EXT_texture_env_combine")) return true;
	return false;
}
void gufOverbrightWorldInit(void) {
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PRIMARY_COLOR_EXT);
}
void gufOverbrightTextureEnable(void) {
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}
void gufOverbright(float amount) {
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, amount );
}

/* Render-Extras */

void gufExtraLineColor(int r,int g,int b){
	glColor3f(r/255.0f,g/255.0f,b/255.0f);
}
int gufExtraLine(float x1,float y1,float z1,float x2,float y2,float z2){
	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f((float)x1, z1, (float)y1);
	glVertex3f((float)x1, z1, y1+0.01f);
	glVertex3f((float)x2, z2, (float)y2);
	glVertex3f((float)x2, z2, y2+0.01f);
	glEnd();
	return 4;
}
void gufExtraBoxBegin(void) {
	glBegin(GL_LINE_LOOP);
}
void gufExtraBoxVertex(float x,float y,float z) {
	glVertex3f(x,y,z);
}
void gufExtraBoxEnd(void) {
	glEnd();
}
void gufExtraDot(float x,float y,float z,float DOF) {
	glBegin(GL_LINE_LOOP);
	glVertex3f(x-DOF, (float)y, z-DOF);
	glVertex3f(x+DOF, (float)y, z-DOF);
	glVertex3f(x+DOF, (float)y, z+DOF);
	glVertex3f(x-DOF, (float)y, z+DOF);
	glEnd();
}
void gufExtraBlendboxFill(int x1,int y1,int x2,int y2,float r,float g,float b){
	glBegin(GL_TRIANGLE_STRIP);
	glColor3d(r, g, b);
	glVertex2f(x1, y1);
	glVertex2f(x2, y1);
	glVertex2f(x1, y2);
	glVertex2f(x2, y2);
	glEnd();
}
void gufExtraBlendboxSurround(int x1,int y1,int x2,int y2,float r,float g,float b){
	glBegin(GL_LINE_LOOP);
	glColor3d(r, g, b);
	glVertex2f(x1, y1);
	glVertex2f(x2, y1);
	glVertex2f(x2, y2);
	glVertex2f(x1, y2);
	glEnd();
}
void gufExtraIconBegin(void){
	glBegin(GL_QUADS);
}
void gufExtraIconSetRGB(int r,int g,int b) {
	glColor3f(r/255.0f,g/255.0f,b/255.0f);
}
void gufExtraIconVertex(float tx,float ty,int x,int y){
	glTexCoord2f(tx,ty); glVertex2f(x,y);
}
void gufExtraIconEnd(void){
	glEnd();
}
void gufExtraBlendScreen(int virtw,int virth,float r,float g,float b) {
	glBegin(GL_QUADS);
	glColor3f(r, g, b);
	glVertex2f(0, 0);
	glVertex2f(virtw, 0);
	glVertex2f(virtw, virth);
	glVertex2f(0, virth);
	glEnd();
}

/* Gl Fog*/
// see https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glFog.xml
void gufFogSetup(void){
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_DENSITY, 0.25);
	glHint(GL_FOG_HINT, GL_NICEST);
}
void gufFogStart(int start){
	glFogi(GL_FOG_START, start);
}
void gufFogEnd(int end){
	glFogi(GL_FOG_END, end);
}
void gufFogColor(const float *fv4){
	glFogfv(GL_FOG_COLOR, fv4);
}

#endif

void gufLoadMatrixd(const double * m) {
	float fm[16];
	for(int i(0);i<16;++i) fm[i] = (float)(m[i]);
	gufLoadMatrixf(fm);
}

