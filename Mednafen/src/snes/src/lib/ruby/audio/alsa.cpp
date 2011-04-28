//audio.alsa (2009-11-30)
//authors: BearOso, byuu, Nach, RedDwarf

#include <alsa/asoundlib.h>

namespace ruby {

class pAudioALSA {
public:
  struct {
    snd_pcm_t *handle;
    snd_pcm_format_t format;
    snd_pcm_uframes_t buffer_size;
    snd_pcm_uframes_t period_size;
    int channels;
    const char *name;
  } device;

  struct {
    uint32_t *data;
    unsigned length;
  } buffer;

  struct {
    bool synchronize;
    unsigned frequency;
    unsigned latency;
  } settings;

  bool cap(const string& name) {
    if(name == Audio::Synchronize) return true;
    if(name == Audio::Frequency) return true;
    if(name == Audio::Latency) return true;
    return false;
  }

  any get(const string& name) {
    if(name == Audio::Synchronize) return settings.synchronize;
    if(name == Audio::Frequency) return settings.frequency;
    if(name == Audio::Latency) return settings.latency;
    return false;
  }

  bool set(const string& name, const any& value) {
    if(name == Audio::Synchronize) {
      if(settings.synchronize != any_cast<bool>(value)) {
        settings.synchronize = any_cast<bool>(value);
        if(device.handle) init();
      }
      return true;
    }

    if(name == Audio::Frequency) {
      if(settings.frequency != any_cast<unsigned>(value)) {
        settings.frequency = any_cast<unsigned>(value);
        if(device.handle) init();
      }
      return true;
    }

    if(name == Audio::Latency) {
      if(settings.latency != any_cast<unsigned>(value)) {
        settings.latency = any_cast<unsigned>(value);
        if(device.handle) init();
      }
      return true;
    }

    return false;
  }

  void sample(uint16_t left, uint16_t right) {
    if(!device.handle) return;

    buffer.data[buffer.length++] = left + (right << 16);
    if(buffer.length < device.period_size) return;

    snd_pcm_sframes_t avail;
    do {
      avail = snd_pcm_avail_update(device.handle);
      if(avail < 0) snd_pcm_recover(device.handle, avail, 1);
      if(avail < buffer.length) {
        if(settings.synchronize == false) {
          buffer.length = 0;
          return;
        }
        int error = snd_pcm_wait(device.handle, -1);
        if(error < 0) snd_pcm_recover(device.handle, error, 1);
      }
    } while(avail < buffer.length);

    //below code has issues with PulseAudio sound server
    #if 0
    if(settings.synchronize == false) {
      snd_pcm_sframes_t avail = snd_pcm_avail_update(device.handle);
      if(avail < device.period_size) {
        buffer.length = 0;
        return;
      }
    }
    #endif

    uint32_t *buffer_ptr = buffer.data;
    int i = 4;

    while((buffer.length > 0) && i--) {
      snd_pcm_sframes_t written = snd_pcm_writei(device.handle, buffer_ptr, buffer.length);
      if(written < 0) {
        //no samples written
        snd_pcm_recover(device.handle, written, 1);
      } else if(written <= buffer.length) {
        buffer.length -= written;
        buffer_ptr += written;
      }
    }

    if(i < 0) {
      if(buffer.data == buffer_ptr) {
        buffer.length--;
        buffer_ptr++;
      }
      memmove(buffer.data, buffer_ptr, buffer.length * sizeof(uint32_t));
    }
  }

  void clear() {
  }

  bool init() {
    term();

    if(snd_pcm_open(&device.handle, device.name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) < 0) {
      term();
      return false;
    }

    //below code will not work with 24khz frequency rate (ALSA library bug)
    #if 0
    if(snd_pcm_set_params(device.handle, device.format, SND_PCM_ACCESS_RW_INTERLEAVED,
      device.channels, settings.frequency, 1, settings.latency * 1000) < 0) {
      //failed to set device parameters
      term();
      return false;
    }

    if(snd_pcm_get_params(device.handle, &device.buffer_size, &device.period_size) < 0) {
      device.period_size = settings.latency * 1000 * 1e-6 * settings.frequency / 4;
    }
    #endif

    snd_pcm_hw_params_t *hwparams;
    snd_pcm_sw_params_t *swparams;
    unsigned rate = settings.frequency;
    unsigned buffer_time = settings.latency * 1000;
    unsigned period_time = settings.latency * 1000 / 4;

    snd_pcm_hw_params_alloca(&hwparams);
    if(snd_pcm_hw_params_any(device.handle, hwparams) < 0) {
      term();
      return false;
    }

    if(snd_pcm_hw_params_set_access(device.handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0
    || snd_pcm_hw_params_set_format(device.handle, hwparams, device.format) < 0
    || snd_pcm_hw_params_set_channels(device.handle, hwparams, device.channels) < 0
    || snd_pcm_hw_params_set_rate_near(device.handle, hwparams, &rate, 0) < 0
    || snd_pcm_hw_params_set_period_time_near(device.handle, hwparams, &period_time, 0) < 0
    || snd_pcm_hw_params_set_buffer_time_near(device.handle, hwparams, &buffer_time, 0) < 0
    ) {
      term();
      return false;
    }

    if(snd_pcm_hw_params(device.handle, hwparams) < 0) {
      term();
      return false;
    }

    if(snd_pcm_get_params(device.handle, &device.buffer_size, &device.period_size) < 0) {
      term();
      return false;
    }

    snd_pcm_sw_params_alloca(&swparams);
    if(snd_pcm_sw_params_current(device.handle, swparams) < 0) {
      term();
      return false;
    }

    if(snd_pcm_sw_params_set_start_threshold(device.handle, swparams,
      (device.buffer_size / device.period_size) * device.period_size) < 0
    ) {
      term();
      return false;
    }

    if(snd_pcm_sw_params(device.handle, swparams) < 0) {
      term();
      return false;
    }

    buffer.data = new uint32_t[device.period_size];
    return true;
  }

  void term() {
    if(device.handle) {
      snd_pcm_drain(device.handle);
      snd_pcm_close(device.handle);
      device.handle = 0;
    }

    if(buffer.data) {
      delete[] buffer.data;
      buffer.data = 0;
	}
  }

  pAudioALSA() {
    device.handle = 0;
    device.format = SND_PCM_FORMAT_S16_LE;
    device.channels = 2;
    device.name = "default";

    buffer.data = 0;
    buffer.length = 0;

    settings.synchronize = false;
    settings.frequency = 22050;
    settings.latency = 60;
  }

  ~pAudioALSA() {
    term();
  }
};

DeclareAudio(ALSA)

};
