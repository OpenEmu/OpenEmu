#include <ruby/ruby.hpp>
using namespace nall;

#include <ruby/ruby_impl.cpp>

namespace ruby {

VideoInterface video;
AudioInterface audio;
InputInterface input;

/* VideoInterface */

const char *Video::Handle = "Handle";
const char *Video::Synchronize = "Synchronize";
const char *Video::Filter = "Filter";

void VideoInterface::driver(const char *driver) {
  if(p) term();

  if(!driver || !*driver) driver = default_driver();

  if(0);

  #ifdef VIDEO_DIRECT3D
  else if(!strcmp(driver, "Direct3D")) p = new VideoD3D();
  #endif

  #ifdef VIDEO_DIRECTDRAW
  else if(!strcmp(driver, "DirectDraw")) p = new VideoDD();
  #endif

  #ifdef VIDEO_GDI
  else if(!strcmp(driver, "GDI")) p = new VideoGDI();
  #endif

  #ifdef VIDEO_GLX
  else if(!strcmp(driver, "OpenGL")) p = new VideoGLX();
  #endif

  #ifdef VIDEO_QTOPENGL
  else if(!strcmp(driver, "Qt-OpenGL")) p = new VideoQtOpenGL();
  #endif

  #ifdef VIDEO_QTRASTER
  else if(!strcmp(driver, "Qt-Raster")) p = new VideoQtRaster();
  #endif

  #ifdef VIDEO_SDL
  else if(!strcmp(driver, "SDL")) p = new VideoSDL();
  #endif

  #ifdef VIDEO_WGL
  else if(!strcmp(driver, "OpenGL")) p = new VideoWGL();
  #endif

  #ifdef VIDEO_XV
  else if(!strcmp(driver, "X-Video")) p = new VideoXv();
  #endif

  else p = new Video();
}

//select the *safest* available driver, not the fastest
const char* VideoInterface::default_driver() {
  #if defined(VIDEO_DIRECT3D)
  return "Direct3D";
  #elif defined(VIDEO_WGL)
  return "OpenGL";
  #elif defined(VIDEO_DIRECTDRAW)
  return "DirectDraw";
  #elif defined(VIDEO_GDI)
  return "GDI";
  #elif defined(VIDEO_QTOPENGL)
  return "Qt-OpenGL";
  #elif defined(VIDEO_QTRASTER)
  return "Qt-Raster";
  #elif defined(VIDEO_SDL)
  return "SDL";
  #elif defined(VIDEO_XV)
  return "X-Video";
  #elif defined(VIDEO_GLX)
  return "OpenGL";
  #else
  return "None";
  #endif
}

//returns list of available drivers, sorted from most to least optimal
const char* VideoInterface::driver_list() {
  return

  //Windows

  #if defined(VIDEO_DIRECT3D)
  "Direct3D;"
  #endif

  #if defined(VIDEO_WGL)
  "OpenGL;"
  #endif

  #if defined(VIDEO_DIRECTDRAW)
  "DirectDraw;"
  #endif

  #if defined(VIDEO_GDI)
  "GDI;"
  #endif

  //Linux

  #if defined(VIDEO_GLX)
  "OpenGL;"
  #endif

  #if defined(VIDEO_QTOPENGL)
  "Qt-OpenGL;"
  #endif

  #if defined(VIDEO_XV)
  "X-Video;"
  #endif

  #if defined(VIDEO_QTRASTER)
  "Qt-Raster;"
  #endif

  #if defined(VIDEO_SDL)
  "SDL;"
  #endif

  "None";
}

bool VideoInterface::init() {
  if(!p) driver();
  return p->init();
}

void VideoInterface::term() {
  if(p) {
    delete p;
    p = 0;
  }
}

bool VideoInterface::cap(const string& name) { return p ? p->cap(name) : false; }
any VideoInterface::get(const string& name) { return p ? p->get(name) : false; }
bool VideoInterface::set(const string& name, const any& value) { return p ? p->set(name, value) : false; }
bool VideoInterface::lock(uint32_t *&data, unsigned &pitch, unsigned width, unsigned height) { return p ? p->lock(data, pitch, width, height) : false; }
void VideoInterface::unlock() { if(p) p->unlock(); }
void VideoInterface::clear() { if(p) p->clear(); }
void VideoInterface::refresh() { if(p) p->refresh(); }
VideoInterface::VideoInterface() : p(0) {}
VideoInterface::~VideoInterface() { term(); }

/* AudioInterface */

void AudioInterface::driver(const char *driver) {
  if(p) term();

  if(!driver || !*driver) driver = default_driver();

  if(0);

  #ifdef AUDIO_ALSA
  else if(!strcmp(driver, "ALSA")) p = new AudioALSA();
  #endif

  #ifdef AUDIO_AO
  else if(!strcmp(driver, "libao")) p = new AudioAO();
  #endif

  #ifdef AUDIO_DIRECTSOUND
  else if(!strcmp(driver, "DirectSound")) p = new AudioDS();
  #endif

  #ifdef AUDIO_OPENAL
  else if(!strcmp(driver, "OpenAL")) p = new AudioOpenAL();
  #endif

  #ifdef AUDIO_OSS
  else if(!strcmp(driver, "OSS")) p = new AudioOSS();
  #endif

  #ifdef AUDIO_PULSEAUDIO
  else if(!strcmp(driver, "PulseAudio")) p = new AudioPulseAudio();
  #endif

  else p = new Audio();
}

//select the *safest* available driver, not the fastest
const char* AudioInterface::default_driver() {
  #if defined(AUDIO_DIRECTSOUND)
  return "DirectSound";
  #elif defined(AUDIO_ALSA)
  return "ALSA";
  #elif defined(AUDIO_OPENAL)
  return "OpenAL";
  #elif defined(AUDIO_PULSEAUDIO)
  return "PulseAudio";
  #elif defined(AUDIO_AO)
  return "libao";
  #elif defined(AUDIO_OSS)
  return "OSS";
  #else
  return "None";
  #endif
}

//returns list of available drivers, sorted from most to least optimal
const char* AudioInterface::driver_list() {
  return

  //Windows

  #if defined(AUDIO_DIRECTSOUND)
  "DirectSound;"
  #endif

  //Linux

  #if defined(AUDIO_ALSA)
  "ALSA;"
  #endif

  #if defined(AUDIO_OPENAL)
  "OpenAL;"
  #endif

  #if defined(AUDIO_OSS)
  "OSS;"
  #endif

  #if defined(AUDIO_PULSEAUDIO)
  "PulseAudio;"
  #endif

  #if defined(AUDIO_AO)
  "libao;"
  #endif

  "None";
}

#include "ruby_audio.cpp"

/* InputInterface */

const char *Input::Handle = "Handle";
const char *Input::KeyboardSupport = "KeyboardSupport";
const char *Input::MouseSupport = "MouseSupport";
const char *Input::JoypadSupport = "JoypadSupport";

void InputInterface::driver(const char *driver) {
  if(p) term();

  if(!driver || !*driver) driver = default_driver();

  if(0);

  #ifdef INPUT_DIRECTINPUT
  else if(!strcmp(driver, "DirectInput")) p = new InputDI();
  #endif

  #ifdef INPUT_RAWINPUT
  else if(!strcmp(driver, "RawInput")) p = new InputRaw();
  #endif

  #ifdef INPUT_SDL
  else if(!strcmp(driver, "SDL")) p = new InputSDL();
  #endif

  #ifdef INPUT_X
  else if(!strcmp(driver, "X-Windows")) p = new InputX();
  #endif

  #ifdef INPUT_CARBON
  else if(!strcmp(driver, "Carbon")) p = new InputCarbon();
  #endif

  else p = new Input();
}

//select the *safest* available driver, not the fastest
const char* InputInterface::default_driver() {
  #if defined(INPUT_RAWINPUT)
  return "RawInput";
  #elif defined(INPUT_DIRECTINPUT)
  return "DirectInput";
  #elif defined(INPUT_SDL)
  return "SDL";
  #elif defined(INPUT_X)
  return "X-Windows";
  #elif defined(INPUT_CARBON)
  return "Carbon";
  #else
  return "none";
  #endif
}

const char* InputInterface::driver_list() {
  return

  //Windows

  #if defined(INPUT_RAWINPUT)
  "RawInput;"
  #endif

  #if defined(INPUT_DIRECTINPUT)
  "DirectInput;"
  #endif

  //Linux

  #if defined(INPUT_SDL)
  "SDL;"
  #endif

  #if defined(INPUT_X)
  "X-Windows;"
  #endif

  //OS X

  #if defined(INPUT_CARBON)
  "Carbon;"
  #endif

  "None";
}

bool InputInterface::init() {
  if(!p) driver();
  return p->init();
}

void InputInterface::term() {
  if(p) {
    delete p;
    p = 0;
  }
}

bool InputInterface::cap(const string& name) { return p ? p->cap(name) : false; }
any InputInterface::get(const string& name) { return p ? p->get(name) : false; }
bool InputInterface::set(const string& name, const any& value) { return p ? p->set(name, value) : false; }
bool InputInterface::acquire() { return p ? p->acquire() : false; }
bool InputInterface::unacquire() { return p ? p->unacquire() : false; }
bool InputInterface::acquired() { return p ? p->acquired() : false; }
bool InputInterface::poll(int16_t *table) { return p ? p->poll(table) : false; }
InputInterface::InputInterface() : p(0) {}
InputInterface::~InputInterface() { term(); }

};
