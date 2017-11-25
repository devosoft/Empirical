#ifndef EMP_OPENGL_GL_H
#define EMP_OPENGL_GL_H

#ifdef __EMSCRIPTEN__

#define GLFW_INCLUDE_ES3
#include <GLES2/gl2.h>

#else

#include <GL/glew.h>

#endif  // __EMSCRIPTEN__

#include <GLFW/glfw3.h>

#endif  // EMP_OPENGL_GL_H
