class DSP {
public:
  virtual void enter() = 0;

  virtual uint8 read(uint8 addr) = 0;
  virtual void write(uint8 addr, uint8 data) = 0;

  virtual void power() = 0;
  virtual void reset() = 0;

  virtual void serialize(serializer&) {}
};
