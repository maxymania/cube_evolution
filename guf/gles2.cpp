#include <stdio.h>
#include <string.h>
#include <GUF/guf.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <vector>
#if 1
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/constants.hpp> // glm::pi
#include <glm/gtc/type_ptr.hpp>
#endif
#include "gles2.h"

#ifndef USE_DESKTOP_GL

//#define DEBUG_ENTER printf("%s\n",__PRETTY_FUNCTION__)
#define DEBUG_ENTER

namespace {
	struct MatrixStack {
		std::vector<glm::mat4> stack;
		glm::mat4 top;
	};
	struct ObjectVertex {
		glm::vec3 position;
		glm::vec2 texCoord;
	};
	struct ObjectDrawCall {
		GLenum mode;
		std::vector<ObjectVertex> vertices;
	};
	inline void recycle(ObjectDrawCall* call) {
		if(!call) return;
		delete call;
	}
	struct ObjectDrawList {
		std::vector<ObjectDrawCall*> calls;
	};
	inline void recycle(ObjectDrawList* list) {
		if(!list) return;
		for(int i(0),n(list->calls.size());i<n;++i)
			recycle(list->calls[i]);
		delete list;
	}
	inline void unset(ObjectDrawList* &list) {
		recycle(list);
		list = 0;
	}
	
	struct StridePointer {
		GLuint stride;
		const void* pointer;
	};
	inline void SP(StridePointer &sp,GLuint stride,const void* pointer){
		sp.stride = stride;
		sp.pointer = pointer;
	}
	MatrixStack mat_modelview;
	MatrixStack mat_perspective;
	MatrixStack *mat_selected;
	bool mat_dirty;
	glm::mat4 mat_mvp;
	void mat_mvp_calc(void) {
		if(mat_dirty) {
			mat_mvp = mat_perspective.top * mat_modelview.top;
			mat_dirty = false;
		}
	}
	
	inline void dump_mat4(float * dst,const float * src){
		for(int i(0);i<16;++i) dst[i] = src[i];
	}
	inline void dump_mat4d(double * dst,const float * src){
		for(int i(0);i<16;++i) dst[i] = src[i];
	}
	float previousClearDepth;
	glm::vec4 uColor;
	float uOverbright;
	glm::vec4 uFogInfo; /*
	.x -> Start
	.y -> End
	.z -> Factor (1.0f if enabled or 0.0f if disabled)
	.w -> unused...
	*/
	glm::vec4 uFogColor;
	void init_u() {
		uColor = glm::vec4(1,1,1,1);
		uOverbright = 1.0f;
		uFogInfo = glm::vec4(0,0,0,0);
		uFogColor = glm::vec4(1,1,1,1);
	}
	
	StridePointer pointers[3]; /*
	[0] -> Vertex 3 GL_FLOAT
	[1] -> Color 4 GL_UNSIGNED_BYTE
	[2] -> TexCoord 2 GL_FLOAT
	*/
	
	ObjectVertex obj_cur_vertex;
	std::vector<ObjectDrawList*> drawlists;
	ObjectDrawList *create_new_drawlist(int_u list) {
		for(int_u i(drawlists.size());i<=list;++i) drawlists.push_back(0);
		unset(drawlists[list]);
		ObjectDrawList *inst (new ObjectDrawList);
		drawlists[list] = inst;
		return inst;
	}
	ObjectDrawList *get_drawlist(int_u list) {
		if(drawlists.size()<=list) return 0;
		return drawlists[list];
	}
	ObjectDrawList * drawlist;
	ObjectDrawCall * drawcall;
	ObjectDrawCall * drawinst;
	
	void bindUniforms(const gufgles2::Program &program) {
		mat_mvp_calc();
		
		glUseProgram(program.program);
		if(locValid(program.u.mvp))   glUniformMatrix4fv(program.u.mvp,1,GL_FALSE,glm::value_ptr(mat_mvp));
		if(locValid(program.u.tex))   glUniform1i       (program.u.tex,0);
		if(locValid(program.u.color)) glUniform4f       (program.u.color,uColor.x,uColor.y,uColor.z,uColor.w);
		
	}
	void perform(ObjectDrawCall * call) {
		DEBUG_ENTER;
		if(call->vertices.empty()) return; // nothing to draw!
		const gufgles2::Program &program = gufgles2_programs[1];
		bindUniforms(program);
		if(locValid(program.a.position))
			glVertexAttribPointer(
				program.a.position, // index
				3, // size
				GL_FLOAT, // type
				GL_FALSE, // normalized
				sizeof(ObjectVertex), // stride
				glm::value_ptr(call->vertices[0].position)); // pointer
		if(locValid(program.a.texCoord))
			glVertexAttribPointer(
				program.a.texCoord, // index
				2, // size
				GL_FLOAT, // type
				GL_FALSE, // normalized
				sizeof(ObjectVertex), // stride
				glm::value_ptr(call->vertices[0].texCoord)); // pointer
		glDrawArrays(call->mode,0,call->vertices.size());
	}
	struct my_state_t {
		bool texture_2d;
		bool polygon_offset_line;
		bool wireframe;
	} state;
	void init_state(void) {
		state.texture_2d = false;
		state.polygon_offset_line = false;
		state.wireframe = false;
	}
}

void gufgles2::init2(void)
{
	mat_dirty = true;
	mat_modelview.top = glm::mat4(1.0f);
	mat_perspective.top = glm::mat4(1.0f);
	mat_selected = &mat_modelview;
	previousClearDepth = 1.0f;
	obj_cur_vertex = ObjectVertex();
	drawlists = std::vector<ObjectDrawList*>();
	drawlist = 0;
	drawcall = 0;
	drawinst = new ObjectDrawCall;
	init_u();
	init_state();
}

void gufReadDepthBuffer(int x,int y,float &depth){
	DEBUG_ENTER;
	// TODO: implement.
	depth = 0.99999f;
}

/* Matrix-stuff */
void gufDumpMatrixf(float * m){
	DEBUG_ENTER;
	dump_mat4(m,glm::value_ptr(mat_selected->top));
}
void gufDumpMatrixd(double * m){
	DEBUG_ENTER;
	dump_mat4d(m,glm::value_ptr(mat_selected->top));
}
void gufDumpTypeMatrixf(int_u type, float * m){
	DEBUG_ENTER;
	switch(type) {
	case GUF_MODELVIEW:  dump_mat4(m,glm::value_ptr(mat_modelview.top)); break;
	case GUF_PROJECTION: dump_mat4(m,glm::value_ptr(mat_perspective.top)); break;
	}
}
void gufDumpTypeMatrixd(int_u type, double * m){
	DEBUG_ENTER;
	switch(type) {
	case GUF_MODELVIEW:  dump_mat4d(m,glm::value_ptr(mat_modelview.top)); break;
	case GUF_PROJECTION: dump_mat4d(m,glm::value_ptr(mat_perspective.top)); break;
	}
}

void gufLoadMatrixf(const float * m) {
	DEBUG_ENTER;
	mat_selected->top = glm::make_mat4(m);
}
void gufMultMatrixf(const float * m) {
	DEBUG_ENTER;
	mat_selected->top = mat_selected->top * glm::make_mat4(m);
}
void gufMatrixMode(GUFenum mode) {
	DEBUG_ENTER;
	switch(mode) {
	case GUF_MODELVIEW:  mat_selected = &mat_modelview; break;
	case GUF_PROJECTION: mat_selected = &mat_perspective; break;
	}
}
bool gufCorrectTextureSize(int &xs,int &ys) {
	DEBUG_ENTER;
	// TODO: implement.
	return false;
}
void gufLoadIdentity(void) {
	DEBUG_ENTER;
	mat_selected->top = glm::mat4(1.0f);
}
void gufRotatef(float angle, float x, float y, float z) {
	DEBUG_ENTER;
	float nang = angle * glm::pi<float>()/180.f;
	glm::mat4 rot = glm::rotate(glm::mat4(1.0f),nang,glm::vec3(x,y,z));
	mat_selected->top = mat_selected->top * rot;
	// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glRotate.xml
}
void gufTranslatef(float x, float y, float z) {
	DEBUG_ENTER;
	glm::mat4 rot = glm::translate(glm::mat4(1.0f),glm::vec3(x,y,z));
	mat_selected->top = mat_selected->top * rot;
	// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glTranslate.xml
}
void gufScalef(float x, float y, float z) {
	DEBUG_ENTER;
	glm::mat4 rot = glm::scale(glm::mat4(1.0f),glm::vec3(x,y,z));
	mat_selected->top = mat_selected->top * rot;
	// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glScale.xml
}
void gufOrtho(float left,  float right,  float bottom,  float top,  float nearVal,  float farVal){
	DEBUG_ENTER;
	glm::mat4 rot = glm::ortho(left,right,bottom,top,nearVal,farVal);
	mat_selected->top = mat_selected->top * rot;
	// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glOrtho.xml
}
void gufPushMatrix( void) {
	DEBUG_ENTER;
	// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glPushMatrix.xml
	mat_selected->stack.push_back(mat_selected->top);
}
void gufPopMatrix( void) {
	DEBUG_ENTER;
	if(mat_selected->stack.empty()) return;
	mat_selected->top = mat_selected->stack.back();
	mat_selected->stack.pop_back();
}

/* OpenGL Basic API. */
void gufBasicClearDepth(float f){
	DEBUG_ENTER;
	//if(f!=previousClearDepth) glClearDepthf(f);
	//previousClearDepth = f;
	glClear(GL_DEPTH_BUFFER_BIT);
}
void gufBasicEnableSmoothShadeModel(void){
	DEBUG_ENTER;
	// Ignore!
}
void gufBasicSmoothLineHint(void) {
	DEBUG_ENTER;
	// Ignore!
}

/* OpenGL 1.1 Draw API for per-object lit geometry. */
void gufGeometrySetColor(float r,float g,float b,float a) {
	DEBUG_ENTER;
	uColor = glm::vec4(r,g,b,a);
}
void gufGeometryBegin(int_u gl_primitive){
	DEBUG_ENTER;
	// drawinst = new ObjectDrawCall;
	drawcall = drawlist ? new ObjectDrawCall : drawinst;
	if(!drawcall->vertices.empty()) drawcall->vertices.clear();
	drawcall->mode = gl_primitive;
}
void gufGeometryTexCoord2f(float u,float v){
	DEBUG_ENTER;
	obj_cur_vertex.texCoord = glm::vec2(u,v);
}

// Calls Ignored!!!
void gufGeometryNormal3f(float x,float y,float z){}


void gufGeometryVertex3f(float x,float y,float z){
	DEBUG_ENTER;
	obj_cur_vertex.position = glm::vec3(x,y,z);
	drawcall->vertices.push_back(obj_cur_vertex);
}
void gufGeometryEnd(void){
	DEBUG_ENTER;
	if(drawinst==drawcall) {
		perform(drawcall);
	} else if(drawlist) {
		drawlist->calls.push_back(drawcall);
	}
	// Set drawcall-ptr to NULL!
	drawcall = 0;
}




/* List equivalents. */
void gufGeometryNewList(int_u list){
	DEBUG_ENTER;
	drawlist = create_new_drawlist(list);
}
void gufGeometryEndList(void){
	DEBUG_ENTER;
	drawlist = 0;
	drawcall = 0;
}
void gufGeometryCallList(int_u list){
	DEBUG_ENTER;
	ObjectDrawCall *dcll;
	ObjectDrawList *dlst = get_drawlist(list);
	if(!dlst) return;
	for(int i(0),n(dlst->calls.size());i<n;++i) {
		dcll = dlst->calls[i];
		if(!dcll) continue;
		perform(dcll);
	}
}



/* OpenGL 1.1 / ES1 API for per-vertex lit level geometry. */
void gufLevelEnableClientState(void) {
}
void gufLevelVertexPtr3f(int_u stride,const void* ptr) {
	DEBUG_ENTER;
	SP(pointers[0],stride,ptr);
}
void gufLevelColorPtr4b(int_u stride,const void* ptr) {
	DEBUG_ENTER;
	SP(pointers[1],stride,ptr);
}
void gufLevelTexCoordPtr2f(int_u stride,const void* ptr) {
	DEBUG_ENTER;
	SP(pointers[2],stride,ptr);
}
void gufLevelDrawArrays(int_u mode, int first, int_u count){
	DEBUG_ENTER;
	// TODO: Do some more work!
	//glDrawArrays(mode,first,count);
}



/* OpenGL 1.1 Enable/Disable substitute */
void gufSetEnabled(int_u feature,bool enabled) {
	DEBUG_ENTER;
	switch(feature) {
	case GUF_TEXTURE_2D: state.texture_2d = enabled; break;
	case GUF_FOG: uFogInfo.z = enabled?1.0f:0.0f; break;
	case GUF_POLYGON_OFFSET_LINE: state.polygon_offset_line = enabled; break;
	case GUF_WIREFRAME: state.wireframe = enabled; break;
	}
}


/* Overbright*/
bool gufOverbrightSupported(void){
	DEBUG_ENTER;
	/*
	const char *exts = (const char *)glGetString(GL_EXTENSIONS);
	if(strstr(exts, "GL_EXT_texture_env_combine")) return true;
	*/
	return false;
}
void gufOverbrightWorldInit(void) { }
void gufOverbrightTextureEnable(void) { }

void gufOverbright(float amount)
{
	DEBUG_ENTER;
	uOverbright = amount;
}



/* Render-Extras */

void gufExtraLineColor(int r,int g,int b){
	DEBUG_ENTER;
	uColor = glm::vec4(r/255.0f,g/255.0f,b/255.0f,1.0f);
}

static int extraQuad_q2ts[] = { 0,1,3,2 };
static int extraQuad_i;
static glm::vec3 extraQuad[4];
static glm::vec2 extraQuadTx[2];
static void extraQuadDraw(int_u mode) {
	const gufgles2::Program &program = gufgles2_programs[3]; // noTex
	
	bindUniforms(program);
	
	if(locValid(program.a.position))
		glVertexAttribPointer(
			program.a.position, // index
			3, // size
			GL_FLOAT, // type
			GL_FALSE, // normalized
			sizeof(glm::vec3), // stride
			extraQuad); // pointer
	
	glDrawArrays(mode,0,4);
}
static void extraQuadDrawTx(int_u mode) {
	const gufgles2::Program &program = gufgles2_programs[2]; // simpleTex
	
	bindUniforms(program);
	
	if(locValid(program.a.position))
		glVertexAttribPointer(
			program.a.position, // index
			3, // size
			GL_FLOAT, // type
			GL_FALSE, // normalized
			sizeof(glm::vec3), // stride
			extraQuad); // pointer
	
	if(locValid(program.a.texCoord))
		glVertexAttribPointer(
			program.a.texCoord, // index
			2, // size
			GL_FLOAT, // type
			GL_FALSE, // normalized
			sizeof(glm::vec2), // stride
			extraQuadTx); // pointer
	
	glDrawArrays(mode,0,4);
}


int gufExtraLine(float x1,float y1,float z1,float x2,float y2,float z2){
	DEBUG_ENTER;
	extraQuad[0] = glm::vec3((float)x1, z1, (float)y1);
	extraQuad[1] = glm::vec3((float)x1, z1, y1+0.01f);
	extraQuad[2] = glm::vec3((float)x2, z2, (float)y2);
	extraQuad[3] = glm::vec3((float)x2, z2, y2+0.01f);
	extraQuadDraw(GL_TRIANGLE_STRIP);
	return 4;
}

void gufExtraBoxBegin(void) {
	DEBUG_ENTER;
	extraQuad_i = 0;
}
void gufExtraBoxVertex(float x,float y,float z) {
	DEBUG_ENTER;
	extraQuad[extraQuad_i] = glm::vec3(x,y,z);
	extraQuad_i++;
}
void gufExtraBoxEnd(void) {
	DEBUG_ENTER;
	extraQuadDraw(GL_LINE_LOOP);
}


void gufExtraDot(float x,float y,float z,float DOF) {
	DEBUG_ENTER;
	extraQuad[0] = glm::vec3(x-DOF, (float)y, z-DOF);
	extraQuad[1] = glm::vec3(x+DOF, (float)y, z-DOF);
	extraQuad[2] = glm::vec3(x+DOF, (float)y, z+DOF);
	extraQuad[3] = glm::vec3(x-DOF, (float)y, z+DOF);
	extraQuadDraw(GL_LINE_LOOP);
}


void gufExtraBlendboxFill(int x1,int y1,int x2,int y2,float r,float g,float b){
	DEBUG_ENTER;
	uColor = glm::vec4(r, g, b, 1.0f);
	extraQuad[0] = glm::vec3(x1, y1, 0.0f);
	extraQuad[1] = glm::vec3(x2, y1, 0.0f);
	extraQuad[2] = glm::vec3(x1, y2, 0.0f);
	extraQuad[3] = glm::vec3(x2, y2, 0.0f);
	extraQuadDraw(GL_TRIANGLE_STRIP);
}


void gufExtraBlendboxSurround(int x1,int y1,int x2,int y2,float r,float g,float b){
	DEBUG_ENTER;
	uColor = glm::vec4(r, g, b, 1.0f);
	extraQuad[0] = glm::vec3(x1, y1, 0.0f);
	extraQuad[1] = glm::vec3(x2, y1, 0.0f);
	extraQuad[2] = glm::vec3(x2, y2, 0.0f);
	extraQuad[3] = glm::vec3(x1, y2, 0.0f);
	extraQuadDraw(GL_LINE_LOOP);
}


void gufExtraIconBegin(void){
	DEBUG_ENTER;
	extraQuad_i = 0;
}
void gufExtraIconSetRGB(int r,int g,int b) {
	DEBUG_ENTER;
	uColor = glm::vec4(r/255.0f,g/255.0f,b/255.0f,1.0f);
}
void gufExtraIconVertex(float tx,float ty,int x,int y){
	DEBUG_ENTER;
	int i = extraQuad_q2ts[extraQuad_i];
	extraQuadTx[i] = glm::vec2(tx,ty);
	extraQuad[i] = glm::vec3(x,y,0.0f);
	extraQuad_i++;
}
void gufExtraIconEnd(void){
	DEBUG_ENTER;
	extraQuadDrawTx(GL_TRIANGLE_STRIP);
}

void gufExtraBlendScreen(int virtw,int virth,float r,float g,float b) {
	DEBUG_ENTER;
	uColor = glm::vec4(r, g, b, 1.0f);
	extraQuad[0] = glm::vec3(0, 0, 0);
	extraQuad[1] = glm::vec3(virtw, 0, 0);
	extraQuad[2] = glm::vec3(0, virth, 0);
	extraQuad[3] = glm::vec3(virtw, virth, 0);
	extraQuadDraw(GL_TRIANGLE_STRIP);
}

/* Gl Fog*/
// see https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glFog.xml
void gufFogSetup(void){
}
void gufFogStart(int start){
	DEBUG_ENTER;
	uFogInfo.x = start;
}
void gufFogEnd(int end){
	DEBUG_ENTER;
	uFogInfo.y = end;
}
void gufFogColor(const float *fv4){
	DEBUG_ENTER;
	uFogColor = glm::make_vec4(fv4);
}

#endif
