class NSS {
public:
  void init();
  void load();
  void unload();
  void power();
  void reset();

  void set_dip(uint16 dip);
  uint8 read(unsigned addr);
  void write(unsigned addr, uint8 data);

private:
  uint16 dip;
};

extern NSS nss;
