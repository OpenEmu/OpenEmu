class PPUDebugger : public PPU, public ChipDebugger {
public:
  bool property(unsigned id, string &name, string &value);

  uint8 vram_mmio_read(uint16 addr);
  void vram_mmio_write(uint16 addr, uint8 data);

  uint8 oam_mmio_read(uint16 addr);
  void oam_mmio_write(uint16 addr, uint8 data);

  uint8 cgram_mmio_read(uint16 addr);
  void cgram_mmio_write(uint16 addr, uint8 data);

  PPUDebugger();
};
