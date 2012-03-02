struct Interface;

struct System : property<System> {
  struct Region {
    enum e {
      NTSC = 0,
      PAL = 1,
      Autodetect = 2
    } i;
  };

  struct ExpansionPortDevice {
    enum e {
      None = 0,
      BSX = 1
    } i;
  };

  void run();
  void runtosave();

  void init();
  void term();
  void load();
  void unload();
  void power();
  void reset();

  void frame();
  void scanline();

  //return *active* system information (settings are cached upon power-on)
  Region region;
  ExpansionPortDevice expansion;
  unsigned cpu_frequency;
  unsigned apu_frequency;
  unsigned serialize_size;

  serializer serialize();
  bool unserialize(serializer&);

  System();

private:
  void runthreadtosave();

  void serialize(serializer&);
  void serialize_all(serializer&);
  void serialize_init();

  friend class Cartridge;
  friend class Video;
  friend class Audio;
  friend class Input;
};

#include "video.hpp"
#include "audio.hpp"
#include "input.hpp"

#include <snes/config/config.hpp>
#include <snes/debugger/debugger.hpp>
#include <snes/scheduler/scheduler.hpp>
#include <snes/random/random.hpp>

extern System system;
