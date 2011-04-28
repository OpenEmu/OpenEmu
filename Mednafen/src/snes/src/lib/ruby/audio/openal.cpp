/*
  audio.openal (2007-12-26)
  author: Nach
  contributors: byuu, wertigon, _willow_
*/

#if defined(PLATFORM_OSX)
  #include <OpenAL/al.h>
  #include <OpenAL/alc.h>
#else
  #include <AL/al.h>
  #include <AL/alc.h>
#endif

namespace ruby {

class pAudioOpenAL {
public:
  struct {
    ALCdevice *handle;
    ALCcontext *context;
    ALuint source;
    ALenum format;
    unsigned latency;
    unsigned queue_length;
  } device;

  struct {
    uint32_t *data;
    unsigned length;
    unsigned size;
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
      settings.synchronize = any_cast<bool>(value);
      return true;
    }

    if(name == Audio::Frequency) {
      settings.frequency = any_cast<unsigned>(value);
      return true;
    }

    if(name == Audio::Latency) {
      if(settings.latency != any_cast<unsigned>(value)) {
        settings.latency = any_cast<unsigned>(value);
        update_latency();
      }
      return true;
    }

    return false;
  }

  void sample(uint16_t sl, uint16_t sr) {
    buffer.data[buffer.length++] = sl + (sr << 16);
    if(buffer.length < buffer.size) return;

    ALuint albuffer = 0;
    int processed = 0;
    while(true) {
      alGetSourcei(device.source, AL_BUFFERS_PROCESSED, &processed);
      while(processed--) {
        alSourceUnqueueBuffers(device.source, 1, &albuffer);
        alDeleteBuffers(1, &albuffer);
        device.queue_length--;
      }
      //wait for buffer playback to catch up to sample generation if not synchronizing
      if(settings.synchronize == false || device.queue_length < 3) break;
    }

    if(device.queue_length < 3) {
      alGenBuffers(1, &albuffer);
      alBufferData(albuffer, device.format, buffer.data, buffer.size * 4, settings.frequency);
      alSourceQueueBuffers(device.source, 1, &albuffer);
      device.queue_length++;
    }

    ALint playing;
    alGetSourcei(device.source, AL_SOURCE_STATE, &playing);
    if(playing != AL_PLAYING) alSourcePlay(device.source);
    buffer.length = 0;
  }

  void clear() {
  }

  void update_latency() {
    if(buffer.data) delete[] buffer.data;
    buffer.size = settings.frequency * settings.latency / 1000.0 + 0.5;
    buffer.data = new uint32_t[buffer.size];
  }

  bool init() {
    update_latency();
    device.queue_length = 0;

    bool success = false;
    if(device.handle = alcOpenDevice(NULL)) {
      if(device.context = alcCreateContext(device.handle, NULL)) {
        alcMakeContextCurrent(device.context);
        alGenSources(1, &device.source);

        //alSourcef (device.source, AL_PITCH, 1.0);
        //alSourcef (device.source, AL_GAIN, 1.0);
        //alSource3f(device.source, AL_POSITION, 0.0, 0.0, 0.0);
        //alSource3f(device.source, AL_VELOCITY, 0.0, 0.0, 0.0);
        //alSource3f(device.source, AL_DIRECTION, 0.0, 0.0, 0.0);
        //alSourcef (device.source, AL_ROLLOFF_FACTOR, 0.0);
        //alSourcei (device.source, AL_SOURCE_RELATIVE, AL_TRUE);

        alListener3f(AL_POSITION, 0.0, 0.0, 0.0);
        alListener3f(AL_VELOCITY, 0.0, 0.0, 0.0);
        ALfloat listener_orientation[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
        alListenerfv(AL_ORIENTATION, listener_orientation);

        success = true;
      }
    }

    if(success == false) {
      term();
      return false;
    }

    return true;
  }

  void term() {
    if(alIsSource(device.source) == AL_TRUE) {
      int playing = 0;
      alGetSourcei(device.source, AL_SOURCE_STATE, &playing);
      if(playing == AL_PLAYING) {
        alSourceStop(device.source);
        int queued = 0;
        alGetSourcei(device.source, AL_BUFFERS_QUEUED, &queued);
        while(queued--) {
          ALuint albuffer = 0;
          alSourceUnqueueBuffers(device.source, 1, &albuffer);
          alDeleteBuffers(1, &albuffer);
          device.queue_length--;
        }
      }

      alDeleteSources(1, &device.source);
      device.source = 0;
    }

    if(device.context) {
      alcMakeContextCurrent(NULL);
      alcDestroyContext(device.context);
      device.context = 0;
    }

    if(device.handle) {
      alcCloseDevice(device.handle);
      device.handle = 0;
    }

    if(buffer.data) {
      delete[] buffer.data;
      buffer.data = 0;
    }
  }

  pAudioOpenAL() {
    device.source = 0;
    device.handle = 0;
    device.context = 0;
    device.format = AL_FORMAT_STEREO16;
    device.queue_length = 0;

    buffer.data = 0;
    buffer.length = 0;
    buffer.size = 0;

    settings.synchronize = true;
    settings.frequency = 22050;
    settings.latency = 40;
  }

  ~pAudioOpenAL() {
    term();
  }
};

DeclareAudio(OpenAL)

};
