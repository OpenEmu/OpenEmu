#include "config/config.hpp"
#include "debugger/debugger.hpp"
#include "interface/interface.hpp"
#include "scheduler/scheduler.hpp"

#include "video/video.hpp"
#include "audio/audio.hpp"
#include "input/input.hpp"

class System : property<System> {
public:
  enum Region { NTSC = 0, PAL = 1 };
  enum RegionAutodetect { Autodetect = 2 };
  enum ExpansionPortDevice { ExpansionNone = 0, ExpansionBSX = 1 };

  void run_mednafen_custom();

  void run();
  void runtosave();

  void init(Interface*);
  void term();
  void power();
  void reset();
  void unload();

  void frame();
  void scanline();

  //return *active* system information (settings are cached upon power-on)
  readonly<unsigned> region;
  readonly<unsigned> expansion;
  readonly<unsigned> serialize_size;

  serializer serialize();
  bool unserialize(serializer&);

  System();
  virtual ~System() {}

private:
  Interface *interface;
  void coprocessor_enter();
  void runthreadtosave();

  void serialize(serializer&);
  void serialize_all(serializer&);
  void serialize_init();

  friend class Cartridge;
  friend class Video;
  friend class Audio;
  friend class Input;
  friend class StateManager;
  friend void threadentry_cop();
};

extern System system;
