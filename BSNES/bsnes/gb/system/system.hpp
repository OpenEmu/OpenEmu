class Interface;

enum class Input : unsigned {
  Up, Down, Left, Right, B, A, Select, Start,
};

struct System : property<System> {
  enum class Revision : unsigned {
    GameBoy,
    SuperGameBoy,
    GameBoyColor,
  };
  readonly<Revision> revision;
  inline bool dmg() const { return revision == Revision::GameBoy; }
  inline bool sgb() const { return revision == Revision::SuperGameBoy; }
  inline bool cgb() const { return revision == Revision::GameBoyColor; }

  struct BootROM {
    uint8 dmg[ 256];
    uint8 sgb[ 256];
    uint8 cgb[2048];
  } bootROM;

  void run();
  void runtosave();
  void runthreadtosave();

  void init();
  void load(Revision);
  void power();

  unsigned clocks_executed;

  //serialization.cpp
  unsigned serialize_size;

  serializer serialize();
  bool unserialize(serializer&);

  void serialize(serializer&);
  void serialize_all(serializer&);
  void serialize_init();

  System();
};

#include <gb/interface/interface.hpp>

extern System system;
