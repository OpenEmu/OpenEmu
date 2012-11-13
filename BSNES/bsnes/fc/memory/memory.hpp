struct Bus {
  uint8 read(uint16 addr);
  void write(uint16 addr, uint8 data);
};

extern Bus bus;
