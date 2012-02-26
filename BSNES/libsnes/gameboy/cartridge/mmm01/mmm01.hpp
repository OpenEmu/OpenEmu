struct MMM01 : MMIO {
  bool rom_mode;
  uint8 rom_base;

  bool ram_enable;
  uint8 rom_select;
  uint8 ram_select;

  uint8 mmio_read(uint16 addr);
  void mmio_write(uint16 addr, uint8 data);
  void power();
} mmm01;
