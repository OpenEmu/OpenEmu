StaticRAM iram;

struct CPUIRAM : Memory {
  unsigned size() const;
  alwaysinline uint8 read(unsigned);
  alwaysinline void write(unsigned, uint8);
} cpuiram;

struct CPUBWRAM : Memory {
  unsigned size() const;
  alwaysinline uint8 read(unsigned);
  alwaysinline void write(unsigned, uint8);
  bool dma;
} cpubwram;
