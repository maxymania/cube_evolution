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
bool gufCorrectTextureSize(int &xs,int &ys) { return false; }
#endif

struct countloop {
	int low,high,step;
	inline void set(int N,bool reversed=false) {
		if(reversed){
			low = N;
			high = 0;
			step = -1;
		} else {
			low = 0;
			high = N;
			step = 1;
		}
	}
	inline bool check(int i) {
		if(step<0) return i>=0;
		return i<=high;
	}
};
struct vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texcoord;
	inline void point(){
		glTexCoord2f(texcoord.x,texcoord.y);
		glNormal3f(normal.x,normal.y,normal.z);
		glVertex3f(pos.x,pos.y,pos.z);
	}
};
struct quadstrip {
	vertex vtx[4];
	unsigned int pos;
	void push(const vertex &v) {
		vtx[pos] = v;
		pos++;
		if(pos==4) {
			vtx[0].point();
			vtx[1].point();
			vtx[2].point();
			vtx[0].point();
			vtx[2].point();
			vtx[3].point();
			vtx[0] = vtx[2];
			vtx[1] = vtx[3];
			pos = 2;
		}
	}
};

namespace {
	inline glm::vec3 genpos(float x,float y,float r,float z) {
		return glm::vec3(x*r,y*r,z);
	}
	inline glm::vec3 v3xs(const glm::vec3 &base,float scalar) {
		return glm::vec3(base.x*scalar,base.y*scalar,base.z*scalar);
	}
	inline void setvert(vertex &vert,glm::vec3 abs,glm::vec2 tc,float pf,float nf) {
		vert.pos = abs*pf;
		vert.normal = abs*nf;
		vert.texcoord = tc;
	}
	inline float gentexc(int i,int n) {
		return ((float)i)/((float)n);
	}
}

#define CACHE_SIZE 30
#define LOOPIE(n) for(int i(0);i<=n;++i)

#define POS(i,j) genpos(sinc1[i],cosc1[i],sinc2[j],cosc2[j])
#define TEXC(i,j) glm::vec2( gentexc(i,slices),gentexc(j,stacks) )
#define VERTEX(i,j) setvert(cur_vert,POS(i,j),TEXC(i,j),radius,nf)

void gufSphere(unsigned int flags,float radius,int slices,int stacks)
{
	countloop f_slices;
	quadstrip q_strip;
	vertex    cur_vert;
	f_slices.set(slices,!!(flags&GUF_INSIDE));
	float sinc1[CACHE_SIZE];
	float cosc1[CACHE_SIZE];
	float sinc2[CACHE_SIZE];
	float cosc2[CACHE_SIZE];
	float angle;
	float nf = 1.0f;
	
	if(slices >= CACHE_SIZE) slices = CACHE_SIZE-1;
	if(stacks >= CACHE_SIZE) stacks = CACHE_SIZE-1;
	
	LOOPIE(slices) {
		angle = 2 * M_PI * i / slices;
		sinc1[i] = SDL_sinf(angle);
		cosc1[i] = SDL_cosf(angle);
	}
	LOOPIE(stacks) {
		angle = 2 * M_PI * i / stacks;
		sinc2[i] = SDL_sinf(angle);
		cosc2[i] = SDL_cosf(angle);
	}
	
	glBegin(GL_TRIANGLES);
	for(int j(1);j<stacks;++j){
		q_strip.pos = 0;
		for(int i(f_slices.low);f_slices.check(i); i+=f_slices.step) {
			VERTEX(i,j); q_strip.push(cur_vert);
			VERTEX(i,j-1); q_strip.push(cur_vert);
		}
		VERTEX(f_slices.low,j); q_strip.push(cur_vert);
		VERTEX(f_slices.low,j-1); q_strip.push(cur_vert);
	}
	glEnd();
}

