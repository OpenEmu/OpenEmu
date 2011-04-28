/*
  video.wgl
  authors: byuu, krom
*/

#include "opengl.hpp"

namespace ruby {

class pVideoWGL : public OpenGL {
public:
  BOOL (APIENTRY *glSwapInterval)(int);

  HDC display;
  HGLRC wglcontext;
  HWND window;
  HINSTANCE glwindow;

  struct {
    HWND handle;
    bool synchronize;
    unsigned filter;

    unsigned width;
    unsigned height;
  } settings;

  bool cap(const string& name) {
    if(name == Video::Handle) return true;
    if(name == Video::Synchronize) return true;
    if(name == Video::Filter) return true;
    if(name == Video::FragmentShader) return true;
    if(name == Video::VertexShader) return true;
    return false;
  }

  any get(const string& name) {
    if(name == Video::Handle) return (uintptr_t)settings.handle;
    if(name == Video::Synchronize) return settings.synchronize;
    if(name == Video::Filter) return settings.filter;
    return false;
  }

  bool set(const string& name, const any& value) {
    if(name == Video::Handle) {
      settings.handle = (HWND)any_cast<uintptr_t>(value);
      return true;
    }

    if(name == Video::Synchronize) {
      if(settings.synchronize != any_cast<bool>(value)) {
        settings.synchronize = any_cast<bool>(value);
        if(wglcontext) init();
      }
    }

    if(name == Video::Filter) {
      settings.filter = any_cast<unsigned>(value);
      return true;
    }

    if(name == Video::FragmentShader) {
      OpenGL::set_fragment_shader(any_cast<const char*>(value));
      return true;
    }

    if(name == Video::VertexShader) {
      OpenGL::set_vertex_shader(any_cast<const char*>(value));
      return true;
    }

    return false;
  }

  bool lock(uint32_t *&data, unsigned &pitch, unsigned width, unsigned height) {
    resize(width, height);
    settings.width  = width;
    settings.height = height;
    return OpenGL::lock(data, pitch);
  }

  void unlock() {
  }

  void clear() {
    OpenGL::clear();
    SwapBuffers(display);
  }

  void refresh() {
    RECT rc;
    GetClientRect(settings.handle, &rc);

    OpenGL::refresh(settings.filter == Video::FilterLinear,
      settings.width, settings.height,
      rc.right - rc.left, rc.bottom - rc.top);

    SwapBuffers(display);
  }

  bool init() {
    term();

    GLuint pixel_format;
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;

    display = GetDC(settings.handle);
    pixel_format = ChoosePixelFormat(display, &pfd);
    SetPixelFormat(display, pixel_format, &pfd);

    wglcontext = wglCreateContext(display);
    wglMakeCurrent(display, wglcontext);

    OpenGL::init();
    settings.width  = 256;
    settings.height = 256;

    //vertical synchronization
    if(!glSwapInterval) glSwapInterval = (BOOL (APIENTRY*)(int))glGetProcAddress("wglSwapIntervalEXT");
    if( glSwapInterval) glSwapInterval(settings.synchronize);

    return true;
  }

  void term() {
    OpenGL::term();

    if(wglcontext) {
      wglDeleteContext(wglcontext);
      wglcontext = 0;
    }
  }

  pVideoWGL() : glSwapInterval(0) {
    settings.handle = 0;
    settings.synchronize = false;
    settings.filter = 0;

    window = 0;
    wglcontext = 0;
    glwindow = 0;
  }

  ~pVideoWGL() { term(); }
};

DeclareVideo(WGL)

};
