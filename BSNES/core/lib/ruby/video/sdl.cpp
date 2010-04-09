#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>
#include <X11/extensions/XShm.h>
#include <SDL/SDL.h>

namespace ruby {

class pVideoSDL {
public:
  Display *display;
  SDL_Surface *screen, *buffer;
  unsigned iwidth, iheight;

  struct {
    uintptr_t handle;

    unsigned width;
    unsigned height;
  } settings;

  bool cap(const string& name) {
    if(name == Video::Handle) return true;
    return false;
  }

  any get(const string& name) {
    if(name == Video::Handle) return settings.handle;
    return false;
  }

  bool set(const string& name, const any& value) {
    if(name == Video::Handle) {
      settings.handle = any_cast<uintptr_t>(value);
      return true;
    }

    return false;
  }

  void resize(unsigned width, unsigned height) {
    if(iwidth >= width && iheight >= height) return;

    iwidth  = max(width,  iwidth);
    iheight = max(height, iheight);

    if(buffer) SDL_FreeSurface(buffer);
    buffer = SDL_CreateRGBSurface(
      SDL_SWSURFACE, iwidth, iheight, 32,
      0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000
    );
  }

  bool lock(uint32_t *&data, unsigned &pitch, unsigned width, unsigned height) {
    if(width != settings.width || height != settings.height) {
      resize(settings.width = width, settings.height = height);
    }

    if(SDL_MUSTLOCK(buffer)) SDL_LockSurface(buffer);
    pitch = buffer->pitch;
    return data = (uint32_t*)buffer->pixels;
  }

  void unlock() {
    if(SDL_MUSTLOCK(buffer)) SDL_UnlockSurface(buffer);
  }

  void clear() {
    if(SDL_MUSTLOCK(buffer)) SDL_LockSurface(buffer);
    for(unsigned y = 0; y < iheight; y++) {
      uint32_t *data = (uint32_t*)buffer->pixels + y * (buffer->pitch >> 2);
      for(unsigned x = 0; x < iwidth; x++) *data++ = 0xff000000;
    }
    if(SDL_MUSTLOCK(buffer)) SDL_UnlockSurface(buffer);
    refresh();
  }

  void refresh() {
    //ruby input is X8R8G8B8, top 8-bits are ignored.
    //as SDL forces us to use a 32-bit buffer, we must set alpha to 255 (full opacity)
    //to prevent blending against the window beneath when X window visual is 32-bits.
    if(SDL_MUSTLOCK(buffer)) SDL_LockSurface(buffer);
    for(unsigned y = 0; y < settings.height; y++) {
      uint32_t *data = (uint32_t*)buffer->pixels + y * (buffer->pitch >> 2);
      for(unsigned x = 0; x < settings.width; x++) *data++ |= 0xff000000;
    }
    if(SDL_MUSTLOCK(buffer)) SDL_UnlockSurface(buffer);

    XWindowAttributes attributes;
    XGetWindowAttributes(display, settings.handle, &attributes);

    SDL_Rect src, dest;

    src.x = 0;
    src.y = 0;
    src.w = settings.width;
    src.h = settings.height;

    dest.x = 0;
    dest.y = 0;
    dest.w = attributes.width;
    dest.h = attributes.height;

    SDL_SoftStretch(buffer, &src, screen, &dest);
    SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);
  }

  bool init() {
    display = XOpenDisplay(0);

    char env[512];
    sprintf(env, "SDL_WINDOWID=%ld", settings.handle);
    putenv(env);

    SDL_InitSubSystem(SDL_INIT_VIDEO);
    screen = SDL_SetVideoMode(2560, 1600, 32, SDL_HWSURFACE);

    buffer  = 0;
    iwidth  = 0;
    iheight = 0;
    resize(settings.width = 256, settings.height = 256);

    return true;
  }

  void term() {
    XCloseDisplay(display);
    SDL_FreeSurface(buffer);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
  }

  pVideoSDL() {
    settings.handle = 0;
  }
};

DeclareVideo(SDL)

};
