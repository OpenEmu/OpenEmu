#ifndef SFC_HPP
namespace SuperFamicom {
#endif

struct ID {
  enum : unsigned {
    IPLROM,
    Nec7725DSP,
    Nec96050DSP,
    HitachiDSP,
    ArmDSP,
    ROM,
    SuperGameBoyROM,
    BsxFlashROM,
    SufamiTurboSlotAROM,
    SufamiTurboSlotBROM,
    RAM,
    NecDSPRAM,
    RTC,
    SPC7110RTC,
    BsxRAM,
    BsxPSRAM,
    SuperGameBoyRAM,
    SuperGameBoyRTC,
    SufamiTurboSlotARAM,
    SufamiTurboSlotBRAM,
  };

  enum : unsigned {
    Port1 = 1,
    Port2 = 2,
  };
};

struct Interface : Emulator::Interface {
  double videoFrequency();
  double audioFrequency();

  bool loaded();
  string sha256();
  unsigned group(unsigned id);
  void load(unsigned id, const stream &stream, const string &markup = "");
  void save(unsigned id, const stream &stream);
  void unload();

  void connect(unsigned port, unsigned device);
  void power();
  void reset();
  void run();

  serializer serialize();
  bool unserialize(serializer&);

  void cheatSet(const lstring&);

  void updatePalette();

  Interface();

private:
  vector<Device> device;
};

extern Interface *interface;

#ifndef SFC_HPP
}
#endif
