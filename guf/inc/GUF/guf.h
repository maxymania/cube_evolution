#pragma once

enum {
	GUF_MODELVIEW,
	GUF_PROJECTION,
};
enum {
	GUF_OUTSIDE = 0,
	GUF_INSIDE = 0x1,
};

typedef unsigned int GUFenum;
typedef unsigned int int_u;

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
/*
The inverse of glLoadMatrixf.
*/
void gufDumpMatrixf(float * m);
void gufMultMatrixf(const float * m);
void gufMatrixMode(GUFenum mode);

/* OpenGL 1.1 or OpenGL ES 2.0 Utility functions. */
bool gufCorrectTextureSize(int &xs,int &ys);

