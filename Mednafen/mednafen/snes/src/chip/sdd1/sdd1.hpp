#include "sdd1emu.hpp"

class SDD1 : public MMIO, public Memory {
public:
  void init();
  void enable();
  void power();
  void reset();

  uint8 mmio_read(unsigned addr);
  void mmio_write(unsigned addr, uint8 data);

  uint8 read(unsigned addr);
  void write(unsigned addr, uint8 data);

  void serialize(serializer&);
  SDD1();
  ~SDD1();

private:
  MMIO *cpu_mmio[0x80];  //bus spying hooks to glean information for struct dma[]

  uint8 sdd1_enable;     //channel bit-mask
  uint8 xfer_enable;     //channel bit-mask
  unsigned mmc[4];       //memory map controller ROM indices

  struct {
    unsigned addr;       //$43x2-$43x4 -- DMA transfer address
    uint16 size;         //$43x5-$43x6 -- DMA transfer size
  } dma[8];

  SDD1emu sdd1emu;
  struct {
    uint8 data[65536];   //pointer to decompressed S-DD1 data
    uint16 offset;       //read index into S-DD1 decompression buffer
    unsigned size;       //length of data buffer; reads decrement counter, set ready to false at 0
    bool ready;          //true when data[] is valid; false to invoke sdd1emu.decompress()
  } buffer;
};

extern SDD1 sdd1;
