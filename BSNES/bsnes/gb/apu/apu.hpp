struct APU : Thread, MMIO {
  #include "square1/square1.hpp"
  #include "square2/square2.hpp"
  #include "wave/wave.hpp"
  #include "noise/noise.hpp"
  #include "master/master.hpp"

  uint8 mmio_data[48];
  uint13 sequencer_base;
  uint3 sequencer_step;

  Square1 square1;
  Square2 square2;
  Wave wave;
  Noise noise;
  Master master;

  static void Main();
  void main();
  void power();

  uint8 mmio_read(uint16 addr);
  void mmio_write(uint16 addr, uint8 data);

  void serialize(serializer&);
};

extern APU apu;
