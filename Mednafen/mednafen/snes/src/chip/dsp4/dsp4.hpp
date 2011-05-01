class DSP4 : public Memory {
public:
  void init();
  void enable();
  void power();
  void reset();

  uint8 read (unsigned addr);
  void  write(unsigned addr, uint8 data);
};

extern DSP4 dsp4;
