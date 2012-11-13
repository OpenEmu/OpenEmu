#ifndef GBA_HPP
namespace GameBoyAdvance {
#endif

struct ID {
  enum : unsigned {
    BIOS,
    ROM,
    RAM,
    EEPROM,
    FlashROM,
  };

  enum : unsigned {
    Device = 1,
  };
};

struct Interface : Emulator::Interface {
  double videoFrequency();
  double audioFrequency();

  bool loaded();
  void load(unsigned id, const stream &stream, const string &markup = "");
  void save(unsigned id, const stream &stream);
  void unload();

  void power();
  void reset();
  void run();

  serializer serialize();
  bool unserialize(serializer&);

  void updatePalette();

  Interface();

private:
  vector<Device> device;
};

extern Interface *interface;

#ifndef GBA_HPP
}
#endif
