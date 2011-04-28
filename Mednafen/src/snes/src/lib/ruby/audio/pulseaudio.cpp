//audio.pulseaudio (2010-01-05)
//author: RedDwarf

#include <pulse/pulseaudio.h>

namespace ruby {

class pAudioPulseAudio {
public:
  struct {
    pa_mainloop *mainloop;
    pa_context *context;
    pa_stream *stream;
    pa_sample_spec spec;
    pa_buffer_attr buffer_attr;
    bool first;
  } device;

  struct {
    uint32_t *data;
    size_t size;
    unsigned offset;
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
  }

  any get(const string& name) {
    if(name == Audio::Synchronize) return settings.synchronize;
    if(name == Audio::Frequency) return settings.frequency;
    if(name == Audio::Latency) return settings.latency;
  }

  bool set(const string& name, const any& value) {
    if(name == Audio::Synchronize) {
      settings.synchronize = any_cast<bool>(value);
      return true;
    }

    if(name == Audio::Frequency) {
      settings.frequency = any_cast<unsigned>(value);
      if(device.stream) {
        pa_operation_unref(pa_stream_update_sample_rate(device.stream, settings.frequency, NULL, NULL));
      }
      return true;
    }

    if(name == Audio::Latency) {
      settings.latency = any_cast<unsigned>(value);
      if(device.stream) {
        device.buffer_attr.tlength = pa_usec_to_bytes(settings.latency * PA_USEC_PER_MSEC, &device.spec);
        pa_stream_set_buffer_attr(device.stream, &device.buffer_attr, NULL, NULL);
      }
      return true;
    }
  }

  void sample(uint16_t left, uint16_t right) {
    pa_stream_begin_write(device.stream, (void**)&buffer.data, &buffer.size);
    buffer.data[buffer.offset++] = left + (right << 16);
    if((buffer.offset + 1) * pa_frame_size(&device.spec) <= buffer.size) return;

    while(true) {
      if(device.first) {
        device.first = false;
        pa_mainloop_iterate(device.mainloop, 0, NULL);
      } else {
        pa_mainloop_iterate(device.mainloop, 1, NULL);
      }
      unsigned length = pa_stream_writable_size(device.stream);
      if(length >= buffer.offset * pa_frame_size(&device.spec)) break;
      if(settings.synchronize == false) {
        buffer.offset = 0;
        return;
      }
    }

    pa_stream_write(device.stream, (const void*)buffer.data, buffer.offset * pa_frame_size(&device.spec), NULL, 0LL, PA_SEEK_RELATIVE);
    buffer.data = 0;
    buffer.offset = 0;
  }

  void clear() {
  }

  bool init() {
    device.mainloop = pa_mainloop_new();

    device.context = pa_context_new(pa_mainloop_get_api(device.mainloop), "ruby::pulseaudio");
    pa_context_connect(device.context, NULL, PA_CONTEXT_NOFLAGS, NULL);

    pa_context_state_t cstate;
    do {
      pa_mainloop_iterate(device.mainloop, 1, NULL);
      cstate = pa_context_get_state(device.context);
      if(!PA_CONTEXT_IS_GOOD(cstate)) return false;
    } while(cstate != PA_CONTEXT_READY);

    device.spec.format = PA_SAMPLE_S16LE;
    device.spec.channels = 2;
    device.spec.rate = settings.frequency;
    device.stream = pa_stream_new(device.context, "audio", &device.spec, NULL);

    device.buffer_attr.maxlength = -1;
    device.buffer_attr.tlength = pa_usec_to_bytes(settings.latency * PA_USEC_PER_MSEC, &device.spec);
    device.buffer_attr.prebuf = -1;
    device.buffer_attr.minreq = -1;
    device.buffer_attr.fragsize = -1;

    pa_stream_flags_t flags = (pa_stream_flags_t)(PA_STREAM_ADJUST_LATENCY | PA_STREAM_VARIABLE_RATE);
    pa_stream_connect_playback(device.stream, NULL, &device.buffer_attr, flags, NULL, NULL);

    pa_stream_state_t sstate;
    do {
      pa_mainloop_iterate(device.mainloop, 1, NULL);
      sstate = pa_stream_get_state(device.stream);
      if(!PA_STREAM_IS_GOOD(sstate)) return false;
    } while(sstate != PA_STREAM_READY);

    buffer.size = 960;
    buffer.offset = 0;
    device.first = true;

    return true;
  }

  void term() {
    if(buffer.data) {
      pa_stream_cancel_write(device.stream);
      buffer.data = 0;
    }

    if(device.stream) {
      pa_stream_disconnect(device.stream);
      pa_stream_unref(device.stream);
      device.stream = 0;
    }

    if(device.context) {
      pa_context_disconnect(device.context);
      pa_context_unref(device.context);
      device.context = 0;
    }

    if(device.mainloop) {
      pa_mainloop_free(device.mainloop);
      device.mainloop = 0;
    }
  }

  pAudioPulseAudio() {
    device.mainloop = 0;
    device.context = 0;
    device.stream = 0;
    buffer.data = 0;
    settings.synchronize = false;
    settings.frequency = 22050;
    settings.latency = 60;
  }

  ~pAudioPulseAudio() {
    term();
  }
};

DeclareAudio(PulseAudio)

}
