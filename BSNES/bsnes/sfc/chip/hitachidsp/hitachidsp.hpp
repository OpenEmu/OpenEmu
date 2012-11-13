struct HitachiDSP : Processor::HG51B, Coprocessor {
  unsigned frequency;
  #include "mmio.hpp"

  static void Enter();
  void enter();

  void init();
  void load();
  void unload();
  void power();
  void reset();

  //HG51B read/write
  uint8 bus_read(uint24 addr);
  void bus_write(uint24 addr, uint8 data);

  //CPU ROM read/write
  uint8 rom_read(unsigned addr);
  void rom_write(unsigned addr, uint8 data);

  //CPU MMIO read/write
  uint8 dsp_read(unsigned addr);
  void dsp_write(unsigned addr, uint8 data);

  void serialize(serializer&);
};

extern HitachiDSP hitachidsp;
