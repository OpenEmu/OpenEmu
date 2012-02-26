struct Memory {
  uint8_t *data;
  unsigned size;

  uint8_t& operator[](unsigned addr);
  void allocate(unsigned size);
  void copy(const uint8_t *data, unsigned size);
  void free();
  Memory();
  ~Memory();
};

struct MMIO {
  virtual uint8 mmio_read(uint16 addr) = 0;
  virtual void mmio_write(uint16 addr, uint8 data) = 0;
};

struct Unmapped : MMIO {
  uint8 mmio_read(uint16) { return 0x00; }
  void mmio_write(uint16, uint8) {}
};

struct Bus {
  MMIO *mmio[65536];
  uint8 read(uint16 addr);
  void write(uint16 addr, uint8 data);

  void power();
};

extern Unmapped unmapped;
extern Bus bus;
