struct DMC {
  unsigned length_counter;
  bool irq_pending;

  uint4 period;
  unsigned period_counter;

  bool irq_enable;
  bool loop_mode;

  uint8 dac_latch;
  uint8 addr_latch;
  uint8 length_latch;

  uint15 read_addr;
  unsigned dma_delay_counter;

  uint3 bit_counter;
  bool have_dma_buffer;
  uint8 dma_buffer;

  bool have_sample;
  uint8 sample;

  void start();
  void stop();
  uint8 clock();

  void power();
  void reset();
  void serialize(serializer&);
} dmc;
