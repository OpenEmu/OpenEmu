struct CPU : Processor::ARM, Thread, MMIO {
  uint8 *iwram;
  uint8 *ewram;
  #include "registers.hpp"
  #include "state.hpp"

  static void Enter();
  void main();
  void step(unsigned clocks);
  void sync_step(unsigned clocks);

  void bus_idle(uint32 addr);
  uint32 bus_read(uint32 addr, uint32 size);
  void bus_write(uint32 addr, uint32 size, uint32 word);

  void keypad_run();
  void power();

  uint8 read(uint32 addr);
  void write(uint32 addr, uint8 byte);

  uint32 iwram_read(uint32 addr, uint32 size);
  void iwram_write(uint32 addr, uint32 size, uint32 word);

  uint32 ewram_read(uint32 addr, uint32 size);
  void ewram_write(uint32 addr, uint32 size, uint32 word);

  void dma_run();
  void dma_transfer(Registers::DMA &dma);
  void dma_vblank();
  void dma_hblank();
  void dma_hdma();

  void timer_step(unsigned clocks);
  void timer_increment(unsigned n);
  void timer_fifo_run(unsigned n);

  void serialize(serializer&);
  CPU();
  ~CPU();
};

extern CPU cpu;
