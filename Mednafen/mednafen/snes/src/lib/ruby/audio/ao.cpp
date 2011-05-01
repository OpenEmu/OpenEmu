/*
  audio.ao (2008-06-01)
  authors: Nach, RedDwarf
*/

#include <ao/ao.h>

namespace ruby {

class pAudioAO {
public:
  int driver_id;
  ao_sample_format driver_format;
  ao_device *audio_device;

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
      if(audio_device) init();
      return true;
    }

    return false;
  }

  void sample(uint16_t l_sample, uint16_t r_sample) {
    uint32_t samp = (l_sample << 0) + (r_sample << 16);
    ao_play(audio_device, (char*)&samp, 4); //This may need to be byte swapped for Big Endian
  }

  void clear() {
  }

  bool init() {
    term();

    driver_id = ao_default_driver_id(); //ao_driver_id((const char*)driver)
    if(driver_id < 0) return false;

    driver_format.bits = 16;
    driver_format.channels = 2;
    driver_format.rate = settings.frequency;
    driver_format.byte_format = AO_FMT_LITTLE;

    ao_option *options = 0;
    ao_info *di = ao_driver_info(driver_id);
    if(!di) return false;
    if(!strcmp(di->short_name, "alsa")) {
      ao_append_option(&options, "buffer_time", "100000"); //100ms latency (default was 500ms)
    }

    audio_device = ao_open_live(driver_id, &driver_format, options);
    if(!audio_device) return false;

    return true;
  }

  void term() {
    if(audio_device) {
      ao_close(audio_device);
      audio_device = 0;
    }
  }

  pAudioAO() {
    audio_device = 0;
    ao_initialize();

    settings.frequency = 22050;
  }

  ~pAudioAO() {
    term();
  //ao_shutdown(); //FIXME: this is causing a segfault for some reason when called ...
  }
};

DeclareAudio(AO)

};
