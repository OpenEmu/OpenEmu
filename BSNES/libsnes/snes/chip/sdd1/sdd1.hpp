class SDD1 {
public:
  void init();
  void load();
  void unload();
  void power();
  void reset();

  uint8 mmio_read(unsigned addr);
  void mmio_write(unsigned addr, uint8 data);

  uint8 rom_read(unsigned addr);
  uint8 mcu_read(unsigned addr);
  void mcu_write(unsigned addr, uint8 data);

  void serialize(serializer&);
  SDD1();
  ~SDD1();

private:
  uint8 sdd1_enable;  //channel bit-mask
  uint8 xfer_enable;  //channel bit-mask
  bool dma_ready;     //used to initialize decompression module
  unsigned mmc[4];    //memory map controller ROM indices

  struct {
    unsigned addr;    //$43x2-$43x4 -- DMA transfer address
    uint16 size;      //$43x5-$43x6 -- DMA transfer size
  } dma[8];

public:
  #include "decomp.hpp"
  Decomp decomp;
};

extern SDD1 sdd1;
