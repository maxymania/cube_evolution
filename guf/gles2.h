#include "glb.h"

#define locValid(x) ((x)>=0)
namespace gufgles2 {
	void init(void);
	void init2(void);
	
	struct Program {
		GLuint program;
		/* Per-Object Uniform-Colors can be supplied as Generic Per-Vertex Attrib as well. */
		struct {
			GLint mvp; /* Model-View-Projection */
			GLint tex; /* Texture */
			GLint color; /* Per-Object color. */
		} u;
		struct {
			GLint position;
			GLint texCoord;
			GLint color; /* Per-Vertex color. */
		} a;
	};
}

#define DEF_PROGRAMS gufgles2::Program gufgles2_programs[5];
extern DEF_PROGRAMS;
