#pragma once

enum {
	GUF_MODELVIEW,
	GUF_PROJECTION,
};
typedef unsigned int GUFenum;

/* GLU (OpenGL Utility Library) substitutes.*/
bool gufUnProject(double winx, double winy, double winz,
	const double modelMatrix[16], 
	const double projMatrix[16],
	const int viewport[4],
	double *objx, double *objy, double *objz
);


void gufPerspective(double fovy, double aspect, double zNear, double zFar);
void gufPerspectivef(float fovy, float aspect, float zNear, float zFar);


/* OpenGL 1.1 substitutes. */

/* glLoadMatrixf(...) */
void gufLoadMatrixf(const float * m);
/*
The inverse of glLoadMatrixf.
*/
void gufDumpMatrixf(float * m);
void gufMultMatrixf(const float * m);
void gufMatrixMode(GUFenum mode);

