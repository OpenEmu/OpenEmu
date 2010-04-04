/*
  audio.directsound (2007-12-26)
  author: byuu
*/

#include <dsound.h>

namespace ruby {

class pAudioDS {
public:
  LPDIRECTSOUND ds;
  LPDIRECTSOUNDBUFFER dsb_p, dsb_b;
  DSBUFFERDESC dsbd;
  WAVEFORMATEX wfx;

  struct {
    unsigned rings;
    unsigned latency;

    uint32_t *buffer;
    unsigned bufferoffset;

    unsigned readring;
    unsigned writering;
    int distance;
  } device;

  struct {
    HWND handle;
    bool synchronize;
    unsigned frequency;
    unsigned latency;
  } settings;

  bool cap(const string& name) {
    if(name == Audio::Handle) return true;
    if(name == Audio::Synchronize) return true;
    if(name == Audio::Frequency) return true;
    if(name == Audio::Latency) return true;
    return false;
  }

  any get(const string& name) {
    if(name == Audio::Handle) return (uintptr_t)settings.handle;
    if(name == Audio::Synchronize) return settings.synchronize;
    if(name == Audio::Frequency) return settings.frequency;
    if(name == Audio::Latency) return settings.latency;
    return false;
  }

  bool set(const string& name, const any& value) {
    if(name == Audio::Handle) {
      settings.handle = (HWND)any_cast<uintptr_t>(value);
      return true;
    }

    if(name == Audio::Synchronize) {
      settings.synchronize = any_cast<bool>(value);
      if(ds) clear();
      return true;
    }

    if(name == Audio::Frequency) {
      settings.frequency = any_cast<unsigned>(value);
      if(ds) init();
      return true;
    }

    if(name == Audio::Latency) {
      settings.latency = any_cast<unsigned>(value);
      if(ds) init();
      return true;
    }

    return false;
  }

  void sample(uint16_t left, uint16_t right) {
    device.buffer[device.bufferoffset++] = left + (right << 16);
    if(device.bufferoffset < device.latency) return;
    device.bufferoffset = 0;

    DWORD pos, size;
    void *output;

    if(settings.synchronize == true) {
      //wait until playback buffer has an empty ring to write new audio data to
      while(device.distance >= device.rings - 1) {
        dsb_b->GetCurrentPosition(&pos, 0);
        unsigned activering = pos / (device.latency * 4);
        if(activering == device.readring) {
          if(settings.synchronize == false) Sleep(1);
          continue;
        }

        //subtract number of played rings from ring distance counter
        device.distance -= (device.rings + activering - device.readring) % device.rings;
        device.readring = activering;

        if(device.distance < 2) {
          //buffer underflow; set max distance to recover quickly
          device.distance  = device.rings - 1;
          device.writering = (device.rings + device.readring - 1) % device.rings;
          break;
        }
      }
    }

    device.writering = (device.writering + 1) % device.rings;
    device.distance  = (device.distance  + 1) % device.rings;

    if(dsb_b->Lock(device.writering * device.latency * 4, device.latency * 4, &output, &size, 0, 0, 0) == DS_OK) {
      memcpy(output, device.buffer, device.latency * 4);
      dsb_b->Unlock(output, size, 0, 0);
    }
  }

  void clear() {
    device.readring  = 0;
    device.writering = device.rings - 1;
    device.distance  = device.rings - 1;

    device.bufferoffset = 0;
    if(device.buffer) memset(device.buffer, 0, device.latency * device.rings * 4);

    if(!dsb_b) return;
    dsb_b->Stop();
    dsb_b->SetCurrentPosition(0);

    DWORD size;
    void *output;
    dsb_b->Lock(0, device.latency * device.rings * 4, &output, &size, 0, 0, 0);
    memset(output, 0, size);
    dsb_b->Unlock(output, size, 0, 0);

    dsb_b->Play(0, 0, DSBPLAY_LOOPING);
  }

  bool init() {
    term();

    device.rings   = 8;
    device.latency = settings.frequency * settings.latency / device.rings / 1000.0 + 0.5;
    device.buffer  = new uint32_t[device.latency * device.rings];
    device.bufferoffset = 0;

    DirectSoundCreate(0, &ds, 0);
    ds->SetCooperativeLevel((HWND)settings.handle, DSSCL_PRIORITY);

    memset(&dsbd, 0, sizeof(dsbd));
    dsbd.dwSize        = sizeof(dsbd);
    dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
    dsbd.dwBufferBytes = 0;
    dsbd.lpwfxFormat   = 0;
    ds->CreateSoundBuffer(&dsbd, &dsb_p, 0);

    memset(&wfx, 0, sizeof(wfx));
    wfx.wFormatTag      = WAVE_FORMAT_PCM;
    wfx.nChannels       = 2;
    wfx.nSamplesPerSec  = settings.frequency;
    wfx.wBitsPerSample  = 16;
    wfx.nBlockAlign     = wfx.wBitsPerSample / 8 * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    dsb_p->SetFormat(&wfx);

    memset(&dsbd, 0, sizeof(dsbd));
    dsbd.dwSize  = sizeof(dsbd);
    dsbd.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE;
    dsbd.dwBufferBytes   = device.latency * device.rings * sizeof(uint32_t);
    dsbd.guid3DAlgorithm = GUID_NULL;
    dsbd.lpwfxFormat     = &wfx;
    ds->CreateSoundBuffer(&dsbd, &dsb_b, 0);
    dsb_b->SetFrequency(settings.frequency);
    dsb_b->SetCurrentPosition(0);

    clear();
    return true;
  }

  void term() {
    if(device.buffer) {
      delete[] device.buffer;
      device.buffer = 0;
    }

    if(dsb_b) { dsb_b->Stop(); dsb_b->Release(); dsb_b = 0; }
    if(dsb_p) { dsb_p->Stop(); dsb_p->Release(); dsb_p = 0; }
    if(ds) { ds->Release(); ds = 0; }
  }

  pAudioDS() {
    ds = 0;
    dsb_p = 0;
    dsb_b = 0;

    device.buffer = 0;
    device.bufferoffset = 0;
    device.readring = 0;
    device.writering = 0;
    device.distance = 0;

    settings.handle = GetDesktopWindow();
    settings.synchronize = false;
    settings.frequency = 22050;
    settings.latency = 120;
  }
};

DeclareAudio(DS)

};
