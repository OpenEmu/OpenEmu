struct SuperFXBus : Bus {
  void init();
};

struct SuperFXGSUROM : Memory {
  unsigned size() const;
  uint8 read(unsigned);
  void write(unsigned, uint8);
};

struct SuperFXGSURAM : Memory {
  unsigned size() const;
  uint8 read(unsigned);
  void write(unsigned, uint8);
};

struct SuperFXCPUROM : Memory {
  unsigned size() const;
  uint8 read(unsigned);
  void write(unsigned, uint8);
};

struct SuperFXCPURAM : Memory {
  unsigned size() const;
  uint8 read(unsigned);
  void write(unsigned, uint8);
};

namespace memory {
  extern SuperFXGSUROM gsurom;
  extern SuperFXGSURAM gsuram;
  extern SuperFXCPUROM fxrom;
  extern SuperFXCPURAM fxram;
}
