struct Memory {
  virtual uint32 read(uint32 addr, uint32 size) = 0;
  virtual void write(uint32 addr, uint32 size, uint32 word) = 0;
};

struct MMIO : Memory {
  virtual uint8 read(uint32 addr) = 0;
  virtual void write(uint32 addr, uint8 data) = 0;
  uint32 read(uint32 addr, uint32 size);
  void write(uint32 addr, uint32 size, uint32 word);
};

struct Bus : Memory {
  Memory *mmio[0x400];
  bool idleflag;
  static uint32 mirror(uint32 addr, uint32 size);

  uint32 speed(uint32 addr, uint32 size);
  void idle(uint32 addr);
  uint32 read(uint32 addr, uint32 size);
  void write(uint32 addr, uint32 size, uint32 word);
  void power();

  void serialize(serializer&);
};

extern Bus bus;
