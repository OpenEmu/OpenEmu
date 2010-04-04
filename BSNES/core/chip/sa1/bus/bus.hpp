struct VBRBus : Bus {
  void init();
};

struct SA1Bus : Bus {
  void init();
};

struct VectorSelectionPage : Memory {
  alwaysinline uint8 read(unsigned);
  alwaysinline void write(unsigned, uint8);
  void sync();
  Memory *access;
};

struct CPUIRAM : Memory {
  unsigned size() const;
  alwaysinline uint8 read(unsigned);
  alwaysinline void write(unsigned, uint8);
};

struct SA1IRAM : Memory {
  unsigned size() const;
  alwaysinline uint8 read(unsigned);
  alwaysinline void write(unsigned, uint8);
};

struct SA1BWRAM : Memory {
  unsigned size() const;
  alwaysinline uint8 read(unsigned);
  alwaysinline void write(unsigned, uint8);
};

struct CC1BWRAM : Memory {
  unsigned size() const;
  alwaysinline uint8 read(unsigned);
  alwaysinline void write(unsigned, uint8);
  bool dma;
};

struct BitmapRAM : Memory {
  unsigned size() const;
  alwaysinline uint8 read(unsigned);
  alwaysinline void write(unsigned, uint8);
};
