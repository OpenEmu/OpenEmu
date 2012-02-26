unsigned wram_addr(uint16 addr) const;
void mmio_joyp_poll();
uint8 mmio_read(uint16 addr);
void mmio_write(uint16 addr, uint8 data);
