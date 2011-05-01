#ifdef __APPLE__
  #include <OpenGL/OpenGL.h>
#endif

namespace ruby {

class pVideoQtOpenGL {
public:
  QWidget *parent;
  QVBoxLayout *layout;

  class RubyGLWidget : public QGLWidget {
  public:
    GLuint texture;
    unsigned textureWidth, textureHeight;

    uint32_t *buffer;
    unsigned rasterWidth, rasterHeight;

    bool synchronize;
    unsigned filter;

    void resize(unsigned width, unsigned height) {
      if(width > textureWidth || height > textureHeight) {
        textureWidth  = max(width,  textureWidth);
        textureHeight = max(height, textureHeight);

        if(buffer) {
          delete[] buffer;
          glDeleteTextures(1, &texture);
        }

        buffer = new uint32_t[textureWidth * textureHeight];
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, textureWidth);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer);
      }
    }

    void updateSynchronization() {
      #ifdef __APPLE__
      makeCurrent();
      CGLContextObj context = CGLGetCurrentContext();
      GLint value = synchronize;  //0 = draw immediately (no vsync), 1 = draw once per frame (vsync)
      CGLSetParameter(context, kCGLCPSwapInterval, &value);
      #endif
    }

    void paintGL() {
      unsigned outputWidth  = width();
      unsigned outputHeight = height();

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, outputWidth, 0, outputHeight, -1.0, 1.0);
      glViewport(0, 0, outputWidth, outputHeight);

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (filter == Video::FilterPoint) ? GL_NEAREST : GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (filter == Video::FilterPoint) ? GL_NEAREST : GL_LINEAR);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rasterWidth, rasterHeight, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer);

      double w = (double)rasterWidth  / (double)textureWidth;
      double h = (double)rasterHeight / (double)textureHeight;
      unsigned u = outputWidth;
      unsigned v = outputHeight;

      glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0, 0); glVertex3i(0, v, 0);
        glTexCoord2f(w, 0); glVertex3i(u, v, 0);
        glTexCoord2f(0, h); glVertex3i(0, 0, 0);
        glTexCoord2f(w, h); glVertex3i(u, 0, 0);
      glEnd();
    }

    void initializeGL() {
      format().setDoubleBuffer(true);

      texture = 0;
      textureWidth = 0;
      textureHeight = 0;
      buffer = 0;
      resize(rasterWidth = 256, rasterHeight = 256);

      glDisable(GL_ALPHA_TEST);
      glDisable(GL_BLEND);
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_POLYGON_SMOOTH);
      glDisable(GL_STENCIL_TEST);
      glEnable(GL_DITHER);
      glEnable(GL_TEXTURE_2D);
      glClearColor(0.0, 0.0, 0.0, 0.0);
    }
  } *widget;

  bool cap(const string& name) {
    if(name == Video::Synchronize) return true;
    if(name == Video::Filter) return true;
    if(name == "QWidget") return true;
    return false;
  }

  any get(const string& name) {
    if(name == Video::Synchronize) return widget->synchronize;
    if(name == Video::Filter) return widget->filter;
    if(name == "QWidget") return parent;
    return false;
  }

  bool set(const string& name, const any& value) {
    if(name == Video::Synchronize) {
      widget->synchronize = any_cast<bool>(value);
      widget->updateSynchronization();
      return true;
    }

    if(name == Video::Filter) {
      widget->filter = any_cast<unsigned>(value);
      return true;
    }

    if(name == "QWidget") {
      parent = any_cast<QWidget*>(value);
      return true;
    }

    return false;
  }

  bool lock(uint32_t *&data, unsigned &pitch, unsigned width, unsigned height) {
    widget->resize(width, height);
    widget->rasterWidth  = width;
    widget->rasterHeight = height;

    pitch = widget->textureWidth * sizeof(uint32_t);
    return data = widget->buffer;
  }

  void unlock() {
  }

  void clear() {
    memset(widget->buffer, 0, widget->textureWidth * widget->textureHeight * sizeof(uint32_t));
    widget->updateGL();
  }

  void refresh() {
    widget->updateGL();
  }

  bool init() {
    layout = new QVBoxLayout;
    layout->setMargin(0);

    widget = new RubyGLWidget;
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(widget);
    parent->setLayout(layout);

    return true;
  }

  void term() {
  }
};

DeclareVideo(QtOpenGL)

};
