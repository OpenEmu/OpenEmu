struct BSXFlash : Memory {
  MappedRAM memory;

  void init();
  void load();
  void unload();
  void power();
  void reset();

  unsigned size() const;
  uint8 read(unsigned addr);
  void write(unsigned addr, uint8 data);

private:
  struct {
    unsigned command;
    uint8 write_old;
    uint8 write_new;

    bool flash_enable;
    bool read_enable;
    bool write_enable;
  } regs;
};

extern BSXFlash bsxflash;
