#include "gles2.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef USE_DESKTOP_GL

DEF_PROGRAMS;
// see also https://stackoverflow.com/questions/42231698/how-to-convert-glsl-version-330-core-to-glsl-es-version-100

/*

[1] -> objects.
[2] -> simpleTex
[3] -> noTex
[4] -> level geometry.
*/

using namespace gufgles2;

namespace {
	const char vsLevel[] =
	"#version 100\n"
	"attribute vec3 aPosition;\n"
	"attribute vec4 aColor;\n"
	"attribute vec2 aTexCoord;\n"
	"uniform mat4 uMvp;\n"
	"varying vec4 vColor;\n"
	"varying vec2 vTexCoord;\n"
	"void main()\n"
	"{\n"
		"gl_Position = uMvp * vec4(aPosition,1.0);\n"
		"vColor = aColor;\n"
		"vTexCoord= aTexCoord;\n"
	"}\n";
	
	const char vsObject[] =
	"#version 100\n"
	"attribute vec3 aPosition;\n"
	"attribute vec2 aTexCoord;\n"
	"uniform mat4 uMvp;\n"
	"uniform vec4 uColor;\n"
	"varying vec4 vColor;\n"
	"varying vec2 vTexCoord;\n"
	"void main()\n"
	"{\n"
		"gl_Position = uMvp * vec4(aPosition,1.0);\n"
		"vColor = uColor;\n"
		"vTexCoord= aTexCoord;\n"
	"}\n";
	
	const char fsGeom[] =
	"#version 100\n"
	"precision mediump float;\n"
	"uniform sampler2D uTex;\n"
	"varying vec4 vColor;\n"
	"varying vec2 vTexCoord;\n"
	"void main() {\n"
		"gl_FragColor = vColor * texture2D(uTex,vTexCoord);\n"
	"}\n";
	;
	
	const char fsSimpleTex[] =
	"#version 100\n"
	"precision mediump float;\n"
	"uniform sampler2D uTex;\n"
	"varying vec4 vColor;\n"
	"varying vec2 vTexCoord;\n"
	"void main() {\n"
		"gl_FragColor = vColor * texture2D(uTex,vTexCoord);\n"
	"}\n";
	;
	
	const char vsNoTex[] =
	"attribute vec3 aPosition;\n"
	"uniform mat4 uMvp;\n"
	"void main()\n"
	"{\n"
		"gl_Position = uMvp * vec4(aPosition,1.0);\n"
	"}\n";
	const char fsNoTex[] =
	"#version 100\n"
	"precision mediump float;\n"
	"uniform vec4 uColor;\n"
	"varying vec2 vTexCoord;\n"
	"void main() {\n"
		"gl_FragColor = uColor;\n"
	"}\n";
	;
	
	GLuint makeShader(GLenum t,const char* data) {
		GLint compiled;
		GLuint object = glCreateShader(t);
		glShaderSource(object, 1, &data, NULL);
		
		
		/* Compile the shader program */
		glCompileShader(object);
	
		/* Check the compile status */
		glGetShaderiv(object, GL_COMPILE_STATUS, &compiled);
		if(compiled) return object;
		
		GLint infoLen = 0;
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &infoLen);
		
		char* infoLog = new char[infoLen+1];
		infoLog[infoLen]=0;
		
		glGetShaderInfoLog(object, infoLen, NULL, infoLog);
		printf("Shader: %s\n",infoLog);
		printf("Source:\n%s\n",data);
		
		glDeleteShader(object);
		_Exit(1);
		return object;
	}
	void makeProgram(Program &program,GLuint vs,GLuint fs,const char* why) {
		GLuint object = glCreateProgram();
		glAttachShader(object,vs);
		glAttachShader(object,fs);
		
		GLint linked;
		glLinkProgram(object);
		glGetProgramiv(object, GL_LINK_STATUS, &linked);
		if(!linked) {
			printf("Shader program not linked!\nAnnotation: %s\n",why);
			_Exit(1);
			return;
		}
		
		//printf("program = %d\n",object);
		program.program = object;
		
		program.u.mvp = glGetUniformLocation(object,"uMvp");
		program.u.tex = glGetUniformLocation(object,"uTex");
		program.u.color = glGetUniformLocation(object,"uColor");
		
		program.a.position = glGetAttribLocation(object,"aPosition");
		program.a.texCoord = glGetAttribLocation(object,"aTexCoord");
		program.a.color = glGetAttribLocation(object,"aColor");
	}
}

void gufgles2::init(void)
{
	static GLuint shaders[10];
	shaders[0] = makeShader(GL_VERTEX_SHADER  ,vsLevel);
	shaders[1] = makeShader(GL_VERTEX_SHADER  ,vsObject);
	shaders[2] = makeShader(GL_FRAGMENT_SHADER,fsGeom);
	shaders[3] = makeShader(GL_FRAGMENT_SHADER,fsSimpleTex);
	shaders[4] = makeShader(GL_FRAGMENT_SHADER,fsNoTex);
	shaders[5] = makeShader(GL_VERTEX_SHADER  ,vsNoTex);
	
	// Position 0 gets corrputed somehow!!
	gufgles2_programs[0].program = 0;
	
	makeProgram(gufgles2_programs[1],shaders[1],shaders[2],"vsObject + fsGeom");
	makeProgram(gufgles2_programs[2],shaders[1],shaders[3],"vsObject + fsSimpleTex");
	makeProgram(gufgles2_programs[3],shaders[5],shaders[4],"vsNoTex + fsNoTex");
	makeProgram(gufgles2_programs[4],shaders[0],shaders[2],"vsLevel + fsGeom");
	//for(int i=0;i<4;++i) printf("gufgles2_programs[%d].program = %d\n",i,gufgles2_programs[i].program);
	//_Exit(1);
}

#endif