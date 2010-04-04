/*
  video.glx
  author: byuu
  license: public domain
  last updated: 2008-08-20

  Design notes:
  SGI's GLX is the X11/Xlib interface to OpenGL.
  At the time of this writing, there are three relevant versions of the API: versions 1.2, 1.3 and 1.4.

  Version 1.2 was released on March 4th, 1997.
  Version 1.3 was released on October 19th, 1998.
  Version 1.4 was released on December 16th, 2005.

  Despite version 1.3 being roughly ten years old at this time, there are still many modern X11 GLX drivers
  that lack full support for the specification. Most notable would be the official video drivers from ATI.
  Given this, 1.4 support is pretty much hopeless to target.

  Luckily, each version has been designed to be backwards compatible with the previous version. As well,
  version 1.2 is wholly sufficient, albeit less convenient, to implement this video module.

  Therefore, for the purpose of compatibility, this driver only uses GLX 1.2 or earlier API commands.
  As well, it only uses raw Xlib API commands, so that it is compatible with any toolkit.
*/

#include "opengl.hpp"

namespace ruby {

//returns true once window is mapped (created and displayed onscreen)
static Bool glx_wait_for_map_notify(Display *d, XEvent *e, char *arg) {
  return (e->type == MapNotify) && (e->xmap.window == (Window)arg);
}

class pVideoGLX : public OpenGL {
public:
  Display *display;
  int screen;
  Window xwindow;
  Colormap colormap;
  GLXContext glxcontext;
  GLXWindow glxwindow;

  struct {
    int version_major, version_minor;
    bool double_buffer;
    bool is_direct;
  } glx;

  struct {
    Window handle;
    bool synchronize;
    unsigned filter;

    unsigned width;
    unsigned height;
  } settings;

  bool cap(const string& name) {
    if(name == Video::Handle) return true;
    if(name == Video::Synchronize) return true;
    if(name == Video::Filter) return true;
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
      settings.handle = any_cast<uintptr_t>(value);
      return true;
    }

    if(name == Video::Synchronize) {
      if(settings.synchronize != any_cast<bool>(value)) {
        settings.synchronize = any_cast<bool>(value);
        if(glxcontext) init();
        return true;
      }
    }

    if(name == Video::Filter) {
      settings.filter = any_cast<unsigned>(value);
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
    if(glx.double_buffer) glXSwapBuffers(display, glxwindow);
  }

  void refresh() {
    //we must ensure that the child window is the same size as the parent window.
    //unfortunately, we cannot hook the parent window resize event notification,
    //as we did not create the parent window, nor have any knowledge of the toolkit used.
    //therefore, inelegant as it may be, we query each window size and resize as needed.
    XWindowAttributes parent, child;
    XGetWindowAttributes(display, settings.handle, &parent);
    XGetWindowAttributes(display, xwindow, &child);
    if(child.width != parent.width || child.height != parent.height) {
      XResizeWindow(display, xwindow, parent.width, parent.height);
    }

    OpenGL::refresh(settings.filter == Video::FilterLinear,
      settings.width, settings.height, parent.width, parent.height);
    if(glx.double_buffer) glXSwapBuffers(display, glxwindow);
  }

  bool init() {
    term();

    display = XOpenDisplay(0);
    screen = DefaultScreen(display);
    glXQueryVersion(display, &glx.version_major, &glx.version_minor);
    //require GLX 1.2+ API
    if(glx.version_major < 1 || (glx.version_major == 1 && glx.version_minor < 2)) return false;

    XWindowAttributes window_attributes;
    XGetWindowAttributes(display, settings.handle, &window_attributes);

    //let GLX determine the best Visual to use for GL output; provide a few hints
    //note: some video drivers will override double buffering attribute
    int elements = 0;
    int attributelist[] = { GLX_RGBA, None };
    int attributelist_sync[] = { GLX_RGBA, GLX_DOUBLEBUFFER, None };
    XVisualInfo *vi = glXChooseVisual(display, screen,
      settings.synchronize ? attributelist_sync : attributelist);

    //Window settings.handle has already been realized, most likely with DefaultVisual.
    //GLX requires that the GL output window has the same Visual as the GLX context.
    //it is not possible to change the Visual of an already realized (created) window.
    //therefore a new child window, using the same GLX Visual, must be created and binded to settings.handle.
    colormap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
    XSetWindowAttributes attributes;
    attributes.colormap = colormap;
    attributes.border_pixel = 0;
    attributes.event_mask = StructureNotifyMask;
    xwindow = XCreateWindow(display, /* parent = */ settings.handle,
      /* x = */ 0, /* y = */ 0, window_attributes.width, window_attributes.height,
      /* border_width = */ 0, vi->depth, InputOutput, vi->visual,
      CWColormap | CWBorderPixel | CWEventMask, &attributes);
    XSetWindowBackground(display, xwindow, /* color = */ 0);
    XMapWindow(display, xwindow);
    XEvent event;
    //window must be realized (appear onscreen) before we make the context current
    XIfEvent(display, &event, glx_wait_for_map_notify, (char*)xwindow);

    glxcontext = glXCreateContext(display, vi, /* sharelist = */ 0, /* direct = */ GL_TRUE);
    glXMakeCurrent(display, glxwindow = xwindow, glxcontext);

    //read attributes of frame buffer for later use, as requested attributes from above are not always granted
    int value = 0;
    glXGetConfig(display, vi, GLX_DOUBLEBUFFER, &value);
    glx.double_buffer = value;
    glx.is_direct = glXIsDirect(display, glxcontext);

    OpenGL::init();
    settings.width  = 256;
    settings.height = 256;
    return true;
  }

  void term() {
    OpenGL::term();

    if(glxcontext) {
      glXDestroyContext(display, glxcontext);
      glxcontext = 0;
    }

    if(xwindow) {
      XUnmapWindow(display, xwindow);
      xwindow = 0;
    }

    if(colormap) {
      XFreeColormap(display, colormap);
      colormap = 0;
    }
  }

  pVideoGLX() {
    settings.handle = 0;
    settings.synchronize = false;
    xwindow = 0;
    colormap = 0;
    glxcontext = 0;
    glxwindow = 0;
  }

  ~pVideoGLX() { term(); }
};

DeclareVideo(GLX)

};
