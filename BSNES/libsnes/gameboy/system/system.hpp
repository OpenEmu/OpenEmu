class Interface;

struct Input {
  enum e {
    Up, Down, Left, Right, B, A, Select, Start,
  } i;
};

struct System : property<System> {
  struct Revision {
    enum e {
      GameBoy,
      SuperGameBoy,
      GameBoyColor,
    } i;
  };
  Revision revision;
  inline bool dmg() const { return revision.i == Revision::GameBoy; }
  inline bool sgb() const { return revision.i == Revision::SuperGameBoy; }
  inline bool cgb() const { return revision.i == Revision::GameBoyColor; }

  struct BootROM {
    static const uint8 dmg[ 256];
    static const uint8 sgb[ 256];
    static const uint8 cgb[2048];
  } bootROM;

  void run();
  void runtosave();
  void runthreadtosave();

  void init();
  void load(Revision::e);
  void power();

  unsigned clocks_executed;

  //serialization.cpp
  unsigned serialize_size;

  serializer serialize();
  bool unserialize(serializer&);

  void serialize(serializer&);
  void serialize_all(serializer&);
  void serialize_init();
};

#include <gameboy/interface/interface.hpp>

extern System system;
