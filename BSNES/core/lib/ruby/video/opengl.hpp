#include <GL/gl.h>

#if defined(PLATFORM_X)
  #include <GL/glx.h>
#elif defined(PLATFORM_WIN)
  #include <GL/glext.h>
#else
  #error "ruby::OpenGL: unsupported platform"
#endif

class OpenGL {
public:
  GLuint gltexture;
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
    buffer = 0;
    iwidth = 0;
    iheight = 0;
  }
};
