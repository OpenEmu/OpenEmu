/* Global Headers */

#if defined(VIDEO_QTOPENGL) || defined(VIDEO_QTRASTER)
  #include <QApplication>
  #include <QtGui>
#endif

#if defined(VIDEO_QTOPENGL)
  #include <QGLWidget>
  #if defined(PLATFORM_WIN)
    #include <GL/glext.h>
  #endif
#endif

#if defined(PLATFORM_X)
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>
  #include <X11/Xatom.h>
#elif defined(PLATFORM_OSX)
  #include <Carbon/Carbon.h>
#elif defined(PLATFORM_WIN)
  #define _WIN32_WINNT 0x0501
  #include <windows.h>
#endif

/* Video */

#define DeclareVideo(Name) \
  class Video##Name : public Video { \
  public: \
    bool cap(const string& name) { return p.cap(name); } \
    any get(const string& name) { return p.get(name); } \
    bool set(const string& name, const any& value) { return p.set(name, value); } \
    \
    bool lock(uint32_t *&data, unsigned &pitch, unsigned width, unsigned height) { return p.lock(data, pitch, width, height); } \
    void unlock() { p.unlock(); } \
    \
    void clear() { p.clear(); } \
    void refresh() { p.refresh(); } \
    bool init() { return p.init(); } \
    void term() { p.term(); } \
    \
    Video##Name() : p(*new pVideo##Name) {} \
    ~Video##Name() { delete &p; } \
  \
  private: \
    pVideo##Name &p; \
  };

#ifdef VIDEO_DIRECT3D
  #include <ruby/video/direct3d.cpp>
#endif

#ifdef VIDEO_DIRECTDRAW
  #include <ruby/video/directdraw.cpp>
#endif

#ifdef VIDEO_GDI
  #include <ruby/video/gdi.cpp>
#endif

#ifdef VIDEO_GLX
  #include <ruby/video/glx.cpp>
#endif

#ifdef VIDEO_QTOPENGL
  #include <ruby/video/qtopengl.cpp>
#endif

#ifdef VIDEO_QTRASTER
  #include <ruby/video/qtraster.cpp>
#endif

#ifdef VIDEO_SDL
  #include <ruby/video/sdl.cpp>
#endif

#ifdef VIDEO_WGL
  #include <ruby/video/wgl.cpp>
#endif

#ifdef VIDEO_XV
  #include <ruby/video/xv.cpp>
#endif

/* Audio */

#define DeclareAudio(Name) \
  class Audio##Name : public Audio { \
  public: \
    bool cap(const string& name) { return p.cap(name); } \
    any get(const string& name) { return p.get(name); } \
    bool set(const string& name, const any& value) { return p.set(name, value); } \
    \
    void sample(uint16_t left, uint16_t right) { p.sample(left, right); } \
    void clear() { p.clear(); } \
    bool init() { return p.init(); } \
    void term() { p.term(); } \
    \
    Audio##Name() : p(*new pAudio##Name) {} \
    ~Audio##Name() { delete &p; } \
  \
  private: \
    pAudio##Name &p; \
  };

#ifdef AUDIO_ALSA
  #include <ruby/audio/alsa.cpp>
#endif

#ifdef AUDIO_AO
  #include <ruby/audio/ao.cpp>
#endif

#ifdef AUDIO_DIRECTSOUND
  #include <ruby/audio/directsound.cpp>
#endif

#ifdef AUDIO_OPENAL
  #include <ruby/audio/openal.cpp>
#endif

#ifdef AUDIO_OSS
  #include <ruby/audio/oss.cpp>
#endif

#ifdef AUDIO_PULSEAUDIO
  #include <ruby/audio/pulseaudio.cpp>
#endif

/* Input */

#define DeclareInput(Name) \
  class Input##Name : public Input { \
  public: \
    bool cap(const string& name) { return p.cap(name); } \
    any get(const string& name) { return p.get(name); } \
    bool set(const string& name, const any& value) { return p.set(name, value); } \
    \
    bool acquire() { return p.acquire(); } \
    bool unacquire() { return p.unacquire(); } \
    bool acquired() { return p.acquired(); } \
    \
    bool poll(int16_t *table) { return p.poll(table); } \
    bool init() { return p.init(); } \
    void term() { p.term(); } \
    \
    Input##Name() : p(*new pInput##Name) {} \
    ~Input##Name() { delete &p; } \
  \
  private: \
    pInput##Name &p; \
  };

#ifdef INPUT_DIRECTINPUT
  #include <ruby/input/directinput.cpp>
#endif

#ifdef INPUT_RAWINPUT
  #include <ruby/input/rawinput.cpp>
#endif

#ifdef INPUT_SDL
  #include <ruby/input/sdl.cpp>
#endif

#ifdef INPUT_X
  #include <ruby/input/x.cpp>
#endif

#ifdef INPUT_CARBON
  #include <ruby/input/carbon.cpp>
#endif
