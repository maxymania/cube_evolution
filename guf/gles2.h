#include "glb.h"

#define locValid(x) ((x)>=0)
namespace gufgles2 {
	void init(void);
	
	struct Program {
		GLuint program;
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
	Program programs[4]; /*
	[0] -> level geometry.
	[1] -> objects.
	[2] -> ...
	*/
}
