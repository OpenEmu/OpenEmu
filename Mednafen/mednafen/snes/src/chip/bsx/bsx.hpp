class BSXBase : public MMIO {
public:
  void init();
  void enable();
  void power();
  void reset();

  uint8 mmio_read(unsigned addr);
  void mmio_write(unsigned addr, uint8 data);

private:
  struct {
    uint8 r2188, r2189, r218a, r218b;
    uint8 r218c, r218d, r218e, r218f;
    uint8 r2190, r2191, r2192, r2193;
    uint8 r2194, r2195, r2196, r2197;
    uint8 r2198, r2199, r219a, r219b;
    uint8 r219c, r219d, r219e, r219f;

    uint8 r2192_counter;
    uint8 r2192_hour, r2192_minute, r2192_second;
  } regs;
};

class BSXCart : public MMIO {
public:
  void init();
  void enable();
  void power();
  void reset();

  uint8 mmio_read(unsigned addr);
  void mmio_write(unsigned addr, uint8 data);

  BSXCart();
  ~BSXCart();

private:
  struct {
    uint8 r[16];
  } regs;

  void update_memory_map();
};

class BSXFlash : public Memory {
public:
  void init();
  void enable();
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

extern BSXBase  bsxbase;
extern BSXCart  bsxcart;
extern BSXFlash bsxflash;
