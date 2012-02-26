unsigned rom_mask;  //rom_size - 1
unsigned ram_mask;  //ram_size - 1
uint8 bus_read(unsigned addr);
void bus_write(unsigned addr, uint8 data);

uint8 op_read(uint16 addr);
alwaysinline uint8 peekpipe();
alwaysinline uint8 pipe();

void cache_flush();
uint8 cache_mmio_read(uint16 addr);
void cache_mmio_write(uint16 addr, uint8 data);

void memory_reset();
