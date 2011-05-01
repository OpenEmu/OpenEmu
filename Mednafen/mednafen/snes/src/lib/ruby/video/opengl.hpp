#include <GL/gl.h>

#if defined(PLATFORM_X)
  #include <GL/glx.h>
  #define glGetProcAddress(name) (*glXGetProcAddress)((const GLubyte*)(name))
#elif defined(PLATFORM_WIN)
  #include <GL/glext.h>
  #define glGetProcAddress(name) wglGetProcAddress(name)
#else
  #error "ruby::OpenGL: unsupported platform"
#endif

PFNGLCREATEPROGRAMPROC glCreateProgram = 0;
PFNGLUSEPROGRAMPROC glUseProgram = 0;
PFNGLCREATESHADERPROC glCreateShader = 0;
PFNGLDELETESHADERPROC glDeleteShader = 0;
PFNGLSHADERSOURCEPROC glShaderSource = 0;
PFNGLCOMPILESHADERPROC glCompileShader = 0;
PFNGLATTACHSHADERPROC glAttachShader = 0;
PFNGLDETACHSHADERPROC glDetachShader = 0;
PFNGLLINKPROGRAMPROC glLinkProgram = 0;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = 0;
PFNGLUNIFORM1IPROC glUniform1i = 0;
PFNGLUNIFORM2FVPROC glUniform2fv = 0;
PFNGLUNIFORM4FVPROC glUniform4fv = 0;

class OpenGL {
public:
  GLuint gltexture;
  GLuint glprogram;
  GLuint fragmentshader;
  GLuint vertexshader;
  bool shader_support;

  uint32_t *buffer;
  unsigned iwidth, iheight;

  void resize(unsigned width, unsigned height) {
    if(iwidth >= width && iheight >= height) return;

    if(gltexture) glDeleteTextures(1, &gltexture);
    iwidth  = max(width,  iwidth );
    iheight = max(height, iheight);
    if(buffer) delete[] buffer;
    buffer = new uint32_t[iwidth * iheight];

    glGenTextures(1, &gltexture);
    glBindTexture(GL_TEXTURE_2D, gltexture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, iwidth);
    glTexImage2D(GL_TEXTURE_2D,
      /* mip-map level = */ 0, /* internal format = */ GL_RGB,
      iwidth, iheight, /* border = */ 0, /* format = */ GL_BGRA,
      GL_UNSIGNED_INT_8_8_8_8_REV, buffer);
  }

  bool lock(uint32_t *&data, unsigned &pitch) {
    pitch = iwidth * sizeof(uint32_t);
    return data = buffer;
  }

  void clear() {
    memset(buffer, 0, iwidth * iheight * sizeof(uint32_t));
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
  }

  void refresh(bool smooth, unsigned inwidth, unsigned inheight, unsigned outwidth, unsigned outheight) {
    if(shader_support) {
      glUseProgram(glprogram);
      GLint location;

      float inputSize[2] = { inwidth, inheight };
      location = glGetUniformLocation(glprogram, "rubyInputSize");
      glUniform2fv(location, 1, inputSize);

      float outputSize[2] = { outwidth, outheight };
      location = glGetUniformLocation(glprogram, "rubyOutputSize");
      glUniform2fv(location, 1, outputSize);

      float textureSize[2] = { iwidth, iheight };
      location = glGetUniformLocation(glprogram, "rubyTextureSize");
      glUniform2fv(location, 1, textureSize);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, smooth ? GL_LINEAR : GL_NEAREST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, outwidth, 0, outheight, -1.0, 1.0);
    glViewport(0, 0, outwidth, outheight);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPixelStorei(GL_UNPACK_ROW_LENGTH, iwidth);
    glTexSubImage2D(GL_TEXTURE_2D,
      /* mip-map level = */ 0, /* x = */ 0, /* y = */ 0,
      inwidth, inheight, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer);

    //OpenGL projection sets 0,0 as *bottom-left* of screen.
    //therefore, below vertices flip image to support top-left source.
    //texture range = x1:0.0, y1:0.0, x2:1.0, y2:1.0
    //vertex range = x1:0, y1:0, x2:width, y2:height
    double w = double(inwidth)  / double(iwidth);
    double h = double(inheight) / double(iheight);
    int u = outwidth;
    int v = outheight;
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0); glVertex3i(0, v, 0);
    glTexCoord2f(w, 0); glVertex3i(u, v, 0);
    glTexCoord2f(0, h); glVertex3i(0, 0, 0);
    glTexCoord2f(w, h); glVertex3i(u, 0, 0);
    glEnd();

    glFlush();

    if(shader_support) {
      glUseProgram(0);
    }
  }

  void set_fragment_shader(const char *source) {
    if(!shader_support) return;

    if(fragmentshader) {
      glDetachShader(glprogram, fragmentshader);
      glDeleteShader(fragmentshader);
      fragmentshader = 0;
    }

    if(source) {
      fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fragmentshader, 1, &source, 0);
      glCompileShader(fragmentshader);
      glAttachShader(glprogram, fragmentshader);
    }

    glLinkProgram(glprogram);
  }

  void set_vertex_shader(const char *source) {
    if(!shader_support) return;

    if(vertexshader) {
      glDetachShader(glprogram, vertexshader);
      glDeleteShader(vertexshader);
      vertexshader = 0;
    }

    if(source) {
      vertexshader = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vertexshader, 1, &source, 0);
      glCompileShader(vertexshader);
      glAttachShader(glprogram, vertexshader);
    }

    glLinkProgram(glprogram);
  }

  void init() {
    //disable unused features
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_STENCIL_TEST);

    //enable useful and required features
    glEnable(GL_DITHER);
    glEnable(GL_TEXTURE_2D);

    //bind shader functions
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)glGetProcAddress("glCreateProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC)glGetProcAddress("glUseProgram");
    glCreateShader = (PFNGLCREATESHADERPROC)glGetProcAddress("glCreateShader");
    glDeleteShader = (PFNGLDELETESHADERPROC)glGetProcAddress("glDeleteShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)glGetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)glGetProcAddress("glCompileShader");
    glAttachShader = (PFNGLATTACHSHADERPROC)glGetProcAddress("glAttachShader");
    glDetachShader = (PFNGLDETACHSHADERPROC)glGetProcAddress("glDetachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)glGetProcAddress("glLinkProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)glGetProcAddress("glGetUniformLocation");
    glUniform1i = (PFNGLUNIFORM1IPROC)glGetProcAddress("glUniform1i");
    glUniform2fv = (PFNGLUNIFORM2FVPROC)glGetProcAddress("glUniform2fv");
    glUniform4fv = (PFNGLUNIFORM4FVPROC)glGetProcAddress("glUniform4fv");

    shader_support = glCreateProgram && glUseProgram && glCreateShader
    && glDeleteShader && glShaderSource && glCompileShader && glAttachShader
    && glDetachShader && glLinkProgram && glGetUniformLocation
    && glUniform1i && glUniform2fv && glUniform4fv;

    if(shader_support) glprogram = glCreateProgram();

    //create surface texture
    resize(256, 256);
  }

  void term() {
    if(gltexture) {
      glDeleteTextures(1, &gltexture);
      gltexture = 0;
    }

    if(buffer) {
      delete[] buffer;
      buffer = 0;
      iwidth = 0;
      iheight = 0;
    }
  }

  OpenGL() {
    gltexture = 0;
    glprogram = 0;
    fragmentshader = 0;
    vertexshader = 0;

    buffer = 0;
    iwidth = 0;
    iheight = 0;
  }
};
