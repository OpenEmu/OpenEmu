struct ICD2 : Emulator::Interface::Bind, GameBoy::Interface::Hook, Coprocessor {
  unsigned revision;

  static void Enter();
  void enter();

  void init();
  void load();
  void unload();
  void power();
  void reset();

  uint8 read(unsigned addr);
  void write(unsigned addr, uint8 data);

  void serialize(serializer&);

private:
  Emulator::Interface::Bind *bind;
  GameBoy::Interface::Hook *hook;
  #include "interface/interface.hpp"
  #include "mmio/mmio.hpp"
};

extern ICD2 icd2;
