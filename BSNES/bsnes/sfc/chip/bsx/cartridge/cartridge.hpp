struct BSXCartridge {
  MappedRAM sram;
  MappedRAM psram;

  void init();
  void load();
  void unload();
  void power();
  void reset();

  uint8 memory_access(bool write, Memory &memory, unsigned addr, uint8 data);
  uint8 memory_read(Memory &memory, unsigned addr);
  void memory_write(Memory &memory, unsigned addr, uint8 data);

  uint8 mcu_access(bool write, unsigned addr, uint8 data = 0x00);
  uint8 mcu_read(unsigned addr);
  void mcu_write(unsigned addr, uint8 data);

  uint8 mmio_read(unsigned addr);
  void mmio_write(unsigned addr, uint8 data);
  void mmio_commit();

  void serialize(serializer&);

private:
  uint8 r[16];
  bool r00, r01, r02, r03;
  bool r04, r05, r06, r07;
  bool r08, r09, r0a, r0b;
  bool r0c, r0d, r0e, r0f;
};

extern BSXCartridge bsxcartridge;
