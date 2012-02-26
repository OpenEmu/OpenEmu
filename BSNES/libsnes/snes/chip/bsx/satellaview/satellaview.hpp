class BSXSatellaview {
public:
  void init();
  void load();
  void unload();
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

extern BSXSatellaview bsxsatellaview;
