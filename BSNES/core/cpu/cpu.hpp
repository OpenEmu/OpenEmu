class CPU : public PPUcounter, public MMIO {
public:
  virtual void enter() = 0;

  //CPU version number
  //* 1 and 2 are known
  //* reported by $4210
  //* affects timing (DRAM refresh, HDMA init, etc)
  uint8 cpu_version;

  virtual uint8 pio() = 0;
  virtual bool joylatch() = 0;
  virtual uint8 port_read(uint8 port) = 0;
  virtual void port_write(uint8 port, uint8 value) = 0;

  virtual void scanline() = 0;
  virtual void power();
  virtual void reset();

  virtual void serialize(serializer&);
  CPU();
  virtual ~CPU();
};
