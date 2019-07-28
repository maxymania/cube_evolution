### TODOs in order to migrate to OpenGL ES 2.0

* Get rid of `GLU` (especially `gluSphere()`)
* Replace `glPushMatrix`, `glPopMatrix` and other matrix-functions with their GUF- counterparts.
* Deal with `glDrawArrays`, catch `glVertexPointer`, `glColorPointer` and `glTexCoordPointer` and convert them.

Terms:

GUF = OpenGL ES 2.0 Utility Framework (my purpose-built library/component.)

### TODOs after the migration.

* Replace `GL_EXT_texture_env_combine` with corresponding shader code.