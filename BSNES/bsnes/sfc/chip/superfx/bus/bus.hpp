struct ROM : Memory {
  unsigned size() const;
  uint8 read(unsigned);
  void write(unsigned, uint8);
} rom;

struct RAM : Memory {
  unsigned size() const;
  uint8 read(unsigned);
  void write(unsigned, uint8);
} ram;
