#include "config/config.hpp"
#include "debugger/debugger.hpp"
#include "interface/interface.hpp"
#include "scheduler/scheduler.hpp"
#include "statemanager/statemanager.hpp"

#include "video/video.hpp"
#include "audio/audio.hpp"
#include "input/input.hpp"

class System {
public:
  void coprocessor_enter();

  enum Region { NTSC = 0, PAL = 1 };
  enum RegionAutodetect { Autodetect = 2 };
  enum ExpansionPortDevice { ExpansionNone = 0, ExpansionBSX = 1 };

  //system functions
  virtual void run();
  virtual void runtosave();

  virtual void init(Interface*);
  virtual void term();
  virtual void power();
  virtual void reset();
  virtual void unload();

  virtual void frame();
  virtual void scanline();

  //return *active* region / expansion port device information
  //settings cached upon power-on
  Region region() const;
  ExpansionPortDevice expansion() const;

  serializer serialize();
  bool unserialize(serializer&);

  System();
  virtual ~System() {}

private:
  unsigned serialize_size;
  void serialize(serializer&);
  void serialize_all(serializer&);
  void serialize_init();

  Interface *interface;
  unsigned snes_region;
  unsigned snes_expansion;

  friend class Cartridge;
  friend class Video;
  friend class Audio;
  friend class Input;
  friend class StateManager;
};

extern System system;
