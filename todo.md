### TODOs in order to migrate to OpenGL ES 2.0

* ~~Get rid of `GLU` (especially `gluSphere()`)~~ Done!
* ~~Replace `glPushMatrix`, `glPopMatrix` and other matrix-functions with their GUF- counterparts.~~ Done!
* ~~Find `glBegin` - `glEnd` braces for particles and special effects and replace them.~~ (c.a. 90% Done)
* Find `glBegin` - `glEnd` braces for geometry.
* Deal with `glDrawArrays`, catch `glVertexPointer`, `glColorPointer` and `glTexCoordPointer` and convert them.

Terms:

GUF = OpenGL ES 2.0 Utility Framework (my purpose-built library/component.)

### TODOs after the migration.

* Replace `GL_EXT_texture_env_combine` with corresponding shader code.