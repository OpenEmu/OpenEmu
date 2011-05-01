class SuperGameBoy : public MMIO, public Memory, public library {
public:
  void enter();

  MMIO *mmio[3];
  uint8 mmio_read(unsigned addr);
  void mmio_write(unsigned addr, uint8 data);

  uint8 read(unsigned addr);
  void write(unsigned addr, uint8 data);

  void init();
  void enable();
  void power();
  void reset();
  void unload();

  void serialize(serializer&);

private:
  uint32_t samplebuffer[4096];
  unsigned row;
  bool version;

  enum { SuperGameBoy1 = 0, SuperGameBoy2 = 1 };
  function<void (uint8_t*, unsigned)> sgb_rom;
  function<void (uint8_t*, unsigned)> sgb_ram;
  function<void (uint8_t*, unsigned)> sgb_rtc;
  function<bool (bool)> sgb_init;
  function<void ()> sgb_term;
  function<void ()> sgb_power;
  function<void ()> sgb_reset;
  function<void (unsigned)> sgb_row;
  function<uint8 (uint16)> sgb_read;
  function<void (uint16, uint8)> sgb_write;
  function<unsigned (uint32_t*, unsigned)> sgb_run;
  function<void ()> sgb_save;
  function<void (serializer&)> sgb_serialize;
};

extern SuperGameBoy supergameboy;
