#ifndef __DRIVERS_OPENGL_H
#define __DRIVERS_OPENGL_H

//#define GL_GLEXT_LEGACY
//#ifdef HAVE_APPLE_OPENGL_FRAMEWORK
//#include <OpenGL/gl.h>
//#include <OpenGL/glext.h>
//#include <OpenGL/glu.h>
//#else
//#include <GL/gl.h>
//#include <GL/glext.h>
//#include <GL/glu.h>
//#endif

#include <SDL_opengl.h>


#ifndef GLAPIENTRY
 #ifdef APIENTRY
  #define GLAPIENTRY APIENTRY
 #else
  #define GLAPIENTRY
 #endif
#endif

#if GL_VERSION_1_3 && GL_ARB_vertex_program && GL_ARB_fragment_program
#define MDFN_WANT_OPENGL_SHADERS 1
#else

#endif


typedef void GLAPIENTRY (*glBindTexture_Func)(GLenum target,GLuint texture);
typedef void GLAPIENTRY (*glColorTableEXT_Func)(GLenum target,
        GLenum internalformat,  GLsizei width, GLenum format, GLenum type,
        const GLvoid *table);
typedef void GLAPIENTRY (*glTexImage2D_Func)(GLenum target, GLint level,
        GLint internalFormat,
        GLsizei width, GLsizei height, GLint border,
        GLenum format, GLenum type,
        const GLvoid *pixels);
typedef void GLAPIENTRY (*glBegin_Func)(GLenum mode);
typedef void GLAPIENTRY (*glVertex2f_Func)(GLfloat x, GLfloat y);
typedef void GLAPIENTRY (*glTexCoord2f_Func)(GLfloat s, GLfloat t);
typedef void GLAPIENTRY (*glEnd_Func)(void);
typedef void GLAPIENTRY (*glEnable_Func)(GLenum cap);
typedef void GLAPIENTRY (*glBlendFunc_Func)(GLenum sfactor, GLenum dfactor);
typedef const GLubyte* GLAPIENTRY (*glGetString_Func)(GLenum name);
typedef void GLAPIENTRY (*glViewport_Func)(GLint x, GLint y,GLsizei width,
         GLsizei height);
typedef void GLAPIENTRY (*glGenTextures_Func)(GLsizei n, GLuint *textures);
typedef void GLAPIENTRY (*glDeleteTextures_Func)(GLsizei n,
         const GLuint *textures);
typedef void GLAPIENTRY (*glTexParameteri_Func)(GLenum target, GLenum pname,
         GLint param);
typedef void GLAPIENTRY (*glClearColor_Func)(GLclampf red, GLclampf green,
         GLclampf blue, GLclampf alpha);
typedef void GLAPIENTRY (*glLoadIdentity_Func)(void);
typedef void GLAPIENTRY (*glClear_Func)(GLbitfield mask);
typedef void GLAPIENTRY (*glMatrixMode_Func)(GLenum mode);
typedef void GLAPIENTRY (*glDisable_Func)(GLenum cap);

typedef void GLAPIENTRY (*glPixelStorei_Func)(GLenum pname, GLint param);
typedef void GLAPIENTRY (*glTexSubImage2D_Func)(GLenum target, GLint level,
                                       GLint xoffset, GLint yoffset,
                                       GLsizei width, GLsizei height,
                                       GLenum format, GLenum type,
                                       const GLvoid *pixels);
typedef void GLAPIENTRY (*glFinish_Func)(void);
typedef void GLAPIENTRY (*glOrtho_Func)(GLdouble left, GLdouble right,
                                 GLdouble bottom, GLdouble top,
                                 GLdouble near_val, GLdouble far_val);
typedef void GLAPIENTRY (*glPixelTransferf_Func)(GLenum pname, GLfloat param);
typedef void GLAPIENTRY (*glColorMask_Func)(GLboolean, GLboolean, GLboolean, GLboolean);
typedef void GLAPIENTRY (*glTexEnvf_Func)(GLenum, GLenum, GLfloat);
typedef void GLAPIENTRY (*glGetIntegerv_Func)(GLenum, GLint *);
typedef void GLAPIENTRY (*glTexGend_Func)(GLenum, GLenum, GLdouble);
typedef void GLAPIENTRY (*glRasterPos2i_Func)(GLint x, GLint y);
typedef void GLAPIENTRY (*glDrawPixels_Func)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void GLAPIENTRY (*glPixelZoom_Func)(GLfloat, GLfloat);
typedef void GLAPIENTRY (*glGetTexLevelParameteriv_Func)(GLenum target, GLint level, GLenum pname, GLint *params);
typedef void GLAPIENTRY (*glAccum_Func)(GLenum, GLfloat);
typedef void GLAPIENTRY (*glClearAccum_Func)(GLfloat, GLfloat, GLfloat, GLfloat);

#if MDFN_WANT_OPENGL_SHADERS
typedef GLhandleARB GLAPIENTRY (*glCreateShaderObjectARB_Func)(GLenum);
typedef void GLAPIENTRY (*glShaderSourceARB_Func)(GLhandleARB, GLsizei, const GLcharARB* *, const GLint *);
typedef void GLAPIENTRY (*glCompileShaderARB_Func)(GLhandleARB);
typedef GLhandleARB GLAPIENTRY (*glCreateProgramObjectARB_Func)(void);
typedef void GLAPIENTRY (*glAttachObjectARB_Func)(GLhandleARB, GLhandleARB);
typedef void GLAPIENTRY (*glLinkProgramARB_Func)(GLhandleARB);
typedef void GLAPIENTRY (*glUseProgramObjectARB_Func)(GLhandleARB);
typedef void GLAPIENTRY (*glUniform1fARB_Func)(GLint, GLfloat);
typedef void GLAPIENTRY (*glUniform2fARB_Func)(GLint, GLfloat, GLfloat);
typedef void GLAPIENTRY (*glUniform3fARB_Func)(GLint, GLfloat, GLfloat, GLfloat);
typedef void GLAPIENTRY (*glUniform1iARB_Func)(GLint, GLint);
typedef void GLAPIENTRY (*glUniform2iARB_Func)(GLint, GLint, GLint);
typedef void GLAPIENTRY (*glUniform3iARB_Func)(GLint, GLint, GLint, GLint);
typedef void GLAPIENTRY (*glActiveTextureARB_Func)(GLenum);
typedef void GLAPIENTRY (*glGetInfoLogARB_Func)(GLhandleARB, GLsizei, GLsizei *, GLcharARB *);
typedef GLint GLAPIENTRY (*glGetUniformLocationARB_Func)(GLhandleARB, const GLcharARB *);
typedef void GLAPIENTRY (*glDeleteObjectARB_Func)(GLhandleARB);
typedef void GLAPIENTRY (*glDetachObjectARB_Func)(GLhandleARB, GLhandleARB);
#endif

extern glBindTexture_Func p_glBindTexture;
extern glColorTableEXT_Func p_glColorTableEXT;
extern glTexImage2D_Func p_glTexImage2D;
extern glBegin_Func p_glBegin;
extern glVertex2f_Func p_glVertex2f;
extern glTexCoord2f_Func p_glTexCoord2f;
extern glEnd_Func p_glEnd;
extern glEnable_Func p_glEnable;
extern glBlendFunc_Func p_glBlendFunc;
extern glGetString_Func p_glGetString;
extern glViewport_Func p_glViewport;
extern glGenTextures_Func p_glGenTextures;
extern glDeleteTextures_Func p_glDeleteTextures;
extern glTexParameteri_Func p_glTexParameteri;
extern glClearColor_Func p_glClearColor;
extern glLoadIdentity_Func p_glLoadIdentity;
extern glClear_Func p_glClear;
extern glMatrixMode_Func p_glMatrixMode;
extern glDisable_Func p_glDisable;

extern glPixelStorei_Func p_glPixelStorei;
extern glTexSubImage2D_Func p_glTexSubImage2D;
extern glFinish_Func p_glFinish;
extern glOrtho_Func p_glOrtho;
extern glPixelTransferf_Func p_glPixelTransferf;
extern glColorMask_Func p_glColorMask;
extern glTexEnvf_Func p_glTexEnvf;
extern glGetIntegerv_Func p_glGetIntegerv;
extern glTexGend_Func p_glTexGend;
extern glGetTexLevelParameteriv_Func p_glGetTexLevelParameteriv;
extern glAccum_Func p_glAccum;
extern glClearAccum_Func p_glClearAccum;

#if MDFN_WANT_OPENGL_SHADERS
extern glCreateShaderObjectARB_Func p_glCreateShaderObjectARB;
extern glShaderSourceARB_Func p_glShaderSourceARB;
extern glCompileShaderARB_Func p_glCompileShaderARB;
extern glCreateProgramObjectARB_Func p_glCreateProgramObjectARB;
extern glAttachObjectARB_Func p_glAttachObjectARB;
extern glLinkProgramARB_Func p_glLinkProgramARB;
extern glUseProgramObjectARB_Func p_glUseProgramObjectARB;
extern glUniform1fARB_Func p_glUniform1fARB;
extern glUniform2fARB_Func p_glUniform2fARB;
extern glUniform3fARB_Func p_glUniform3fARB;
extern glUniform1iARB_Func p_glUniform1iARB;
extern glUniform2iARB_Func p_glUniform2iARB;
extern glUniform3iARB_Func p_glUniform3iARB;
extern glActiveTextureARB_Func p_glActiveTextureARB;
extern glGetInfoLogARB_Func p_glGetInfoLogARB;
extern glGetUniformLocationARB_Func p_glGetUniformLocationARB;
extern glDeleteObjectARB_Func p_glDeleteObjectARB;
extern glDetachObjectARB_Func p_glDetachObjectARB;

#endif

#include <string>

void BlitOpenGLRaw(SDL_Surface *surface, const SDL_Rect *rect, const SDL_Rect *dest_rect);
void BlitOpenGL(SDL_Surface *src_surface, const SDL_Rect *src_rect, const SDL_Rect *dest_rect, const SDL_Rect *original_src_rect, bool alpha_blend);
void KillOpenGL(void);
int InitOpenGL(int ipolate, int scanlines, std::string pixshader, SDL_Surface *screen);
void ClearBackBufferOpenGL(SDL_Surface *screen);
void FlipOpenGL(void);

#endif
