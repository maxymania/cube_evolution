#pragma once

enum {
	GUF_MODELVIEW,
	GUF_PROJECTION,
};
enum {
	GUF_OUTSIDE = 0,
	GUF_INSIDE = 0x1,
};

/* gufSetEnabled() */
enum {
	GUF_TEXTURE_2D,
	GUF_FOG,
	GUF_LINE_SMOOTH,
	GUF_POLYGON_OFFSET_LINE,
};

typedef unsigned int GUFenum;
#ifndef HAS_INT_U
#define HAS_INT_U
typedef unsigned int int_u;
#endif

/* GLU (OpenGL Utility Library) substitutes.*/
bool gufUnProject(double winx, double winy, double winz,
	const double modelMatrix[16], 
	const double projMatrix[16],
	const int viewport[4],
	double *objx, double *objy, double *objz
);


void gufPerspective(double fovy, double aspect, double zNear, double zFar);
void gufPerspectivef(float fovy, float aspect, float zNear, float zFar);

void gufSphere(unsigned int flags,float radius,int slices,int stacks);

void gufScaleImage(int channels,  int wIn,  int hIn,  const void * dataIn,  int wOut,  int hOut,  void* dataOut);

int gufBuild2DMipmaps(int_u target,  int internalFormat,  int_u width,  int_u height,  int_u format,  int_u type,  const void * data);

/* OpenGL 1.1 substitutes. */

/* glLoadMatrixf(...) */
void gufLoadMatrixf(const float * m);
void gufLoadMatrixd(const double * m);
/*
The inverse of glLoadMatrixf.
*/
void gufDumpMatrixf(float * m);
void gufDumpMatrixd(double * m);
void gufDumpTypeMatrixf(int_u type, float * m);
void gufDumpTypeMatrixd(int_u type, double * m);

void gufMultMatrixf(const float * m);
void gufMatrixMode(GUFenum mode);
void gufLoadIdentity(void);
void gufRotatef(float angle, float x, float y, float z);
void gufTranslatef(float x, float y, float z);
void gufScalef(float x, float y, float z);
/*
void glOrtho(GLdouble left,  GLdouble right,  GLdouble bottom,  GLdouble top,  GLdouble nearVal,  GLdouble farVal);
*/
void gufOrtho(float left,  float right,  float bottom,  float top,  float nearVal,  float farVal);
void gufPushMatrix( void);
void gufPopMatrix( void);

/* OpenGL 1.1 Enable/Disable substitute */
void gufSetEnabled(int_u feature,bool enabled);

/* OpenGL 1.1 or OpenGL ES 2.0 Utility functions. */
bool gufCorrectTextureSize(int &xs,int &ys);

/* Render API Extensions/Effects */

/* Overbright */
bool gufOverbrightSupported(void);
void gufOverbrightWorldInit(void);
void gufOverbrightTextureEnable(void);
void gufOverbright(float amount);

/* GL Fog */
void gufFogSetup(void);
void gufFogStart(int start);
void gufFogEnd(int end);
void gufFogColor(const float *fv4);

/* Render-Extras */
int gufExtraLine(float x1,float y1,float z1,float x2,float y2,float z2); /* NOTE: y and z are swapped! */

// Wire-frame box.
void gufExtraBoxBegin(void);
void gufExtraBoxVertex(float x,float y,float z);
void gufExtraBoxEnd(void);

void gufExtraDot(float x,float y,float z,float DOF);

void gufExtraBlendboxFill(int x1,int y1,int x2,int y2,float r,float g,float b);
void gufExtraBlendboxSurround(int x1,int y1,int x2,int y2,float r,float g,float b);

void gufExtraIconBegin(void);
void gufExtraIconSetRGB(int r,int g,int b);
void gufExtraIconVertex(float tx,float ty,int x,int y);
void gufExtraIconEnd(void);

void gufExtraBlendScreen(int virtw,int virth,float r,float g,float b);

