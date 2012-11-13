struct Configuration {
  Input::Device controller_port1;
  Input::Device controller_port2;
  System::ExpansionPortDevice expansion_port;
  System::Region region;
  bool random;

  struct CPU {
    unsigned version;
    unsigned ntsc_frequency;
    unsigned pal_frequency;
    unsigned wram_init_value;
  } cpu;

  struct SMP {
    unsigned ntsc_frequency;
    unsigned pal_frequency;
  } smp;

  struct PPU1 {
    unsigned version;
  } ppu1;

  struct PPU2 {
    unsigned version;
  } ppu2;

  struct SuperFX {
    unsigned speed;
  } superfx;

  Configuration();
};

extern Configuration config;
