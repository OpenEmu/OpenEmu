//audio.pulseaudiosimple (2010-01-05)
//author: byuu

#include <pulse/simple.h>
#include <pulse/error.h>

namespace ruby {

class pAudioPulseAudioSimple {
public:
  struct {
    pa_simple *handle;
    pa_sample_spec spec;
  } device;

  struct {
    uint32_t *data;
    unsigned offset;
  } buffer;

  struct {
    unsigned frequency;
  } settings;

  bool cap(const string& name) {
    if(name == Audio::Frequency) return true;
    return false;
  }

  any get(const string& name) {
    if(name == Audio::Frequency) return settings.frequency;
    return false;
  }

  bool set(const string& name, const any& value) {
    if(name == Audio::Frequency) {
      settings.frequency = any_cast<unsigned>(value);
      if(device.handle) init();
      return true;
    }

    return false;
  }

  void sample(uint16_t left, uint16_t right) {
    if(!device.handle) return;

    buffer.data[buffer.offset++] = left + (right << 16);
    if(buffer.offset >= 64) {
      int error;
      pa_simple_write(device.handle, (const void*)buffer.data, buffer.offset * sizeof(uint32_t), &error);
      buffer.offset = 0;
    }
  }

  void clear() {
  }

  bool init() {
    term();

    device.spec.format   = PA_SAMPLE_S16LE;
    device.spec.channels = 2;
    device.spec.rate     = settings.frequency;

    int error = 0;
    device.handle = pa_simple_new(
      0,                         //default server
      "ruby::pulseaudiosimple",  //application name
      PA_STREAM_PLAYBACK,        //direction
      0,                         //default device
      "audio",                   //stream description
      &device.spec,              //sample format
      0,                         //default channel map
      0,                         //default buffering attributes
      &error                     //error code
    );
    if(!device.handle) {
      fprintf(stderr, "ruby::pulseaudiosimple failed to initialize - %s\n", pa_strerror(error));
      return false;
    }

    buffer.data = new uint32_t[64];
    buffer.offset = 0;
    return true;
  }

  void term() {
    if(device.handle) {
      int error;
      pa_simple_flush(device.handle, &error);
      pa_simple_free(device.handle);
      device.handle = 0;
    }

    if(buffer.data) {
      delete[] buffer.data;
      buffer.data = 0;
    }
  }

  pAudioPulseAudioSimple() {
    device.handle = 0;
    buffer.data = 0;
    settings.frequency = 22050;
  }

  ~pAudioPulseAudioSimple() {
    term();
  }
};

DeclareAudio(PulseAudioSimple)

};
