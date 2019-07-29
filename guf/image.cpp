#include <stdlib.h>
#include <GUF/guf.h>
#include "glb.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

#define ucv (unsigned char*)
#define ucc (const unsigned char*)

namespace {
	inline int gl2channels(int_u format) {
		switch(format){
		case GL_ALPHA:
		case GL_LUMINANCE: return 1;
		case GL_LUMINANCE_ALPHA: return 2;
		case GL_RGB: return 3;
		case GL_RGBA: return 4;
		}
		return 0;
	}
	inline int why(int r,const char* whystr) {
		return r;
	}
	
	#ifndef HAS_glGenerateMipmap
	void *imgbuffer = NULL;
	int imgbuflen = 0;
	inline void* imgbufget(int size) {
		if(imgbuflen>size) return imgbuffer;
		imgbuffer = realloc(imgbuffer,size);
		imgbuflen = size;
		return imgbuffer;
	}
	inline bool imgbufinit(int origSiz,void* &a,void* &b){
		/*
		void* buf = imgbufget((origSiz/2)+(origSiz/4));
		*/
		char* buf = (char*)imgbufget(origSiz);
		if(!buf) return false;
		a = buf;
		b = &(buf[(origSiz/2)]);
		return true;
	}
	#endif
}

void gufScaleImage(int channels,  int wIn,  int hIn,  const void * dataIn,  int wOut,  int hOut,  void* dataOut){
	stbir_resize_uint8(
		ucc dataIn ,wIn ,hIn ,0,
		ucv dataOut,wOut,hOut,0,
		channels);
}

int gufBuild2DMipmaps(int_u target,  int internalFormat,  int_u width,  int_u height,  int_u format,  int_u type,  const void * data){
	glTexImage2D(target,0,internalFormat,width,height,0,format,type,data);
	#ifdef HAS_glGenerateMipmap
		glGenerateMipmap(target);
	#else
		int channels = gl2channels(format);
		int level = 0;
		if(target!=GL_TEXTURE_2D) return 1;
		if(type!=GL_UNSIGNED_BYTE) return 1;
		void *ndata2[] = {0,0};
		if(!imgbufinit(width*height*channels,ndata2[0],ndata2[1])) return 1;
		
		int_u w1,h1,w2,h2;
		w1 = width;
		h1 = height;
		const void *plast = data;
		void *pcur;
		for(;;) {
			if( (w1<2) && (h1<2) ) break;
			w2 = w1/2;
			h2 = h1/2;
			pcur = ndata2[level&1];
			
			level++;
			if(!w2) w2 = 1;
			if(!h2) h2 = 1;
			
			gufScaleImage(channels,w1,h1,plast,w2,h2,pcur);
			glTexImage2D(target,level,internalFormat,w2,h2,0,format,type,pcur);
			
			plast = pcur;
			w1 = w2;
			h1 = h2;
		}
	#endif
	return 0;
}

