struct MBC3 : MMIO {
  bool  ram_enable;  //$0000-1fff
  uint8 rom_select;  //$2000-3fff
  uint8 ram_select;  //$4000-5fff
  bool rtc_latch;    //$6000-7fff

  bool rtc_halt;
  unsigned rtc_second;
  unsigned rtc_minute;
  unsigned rtc_hour;
  unsigned rtc_day;
  bool rtc_day_carry;

  unsigned rtc_latch_second;
  unsigned rtc_latch_minute;
  unsigned rtc_latch_hour;
  unsigned rtc_latch_day;
  unsigned rtc_latch_day_carry;

  void second();
  uint8 mmio_read(uint16 addr);
  void mmio_write(uint16 addr, uint8 data);
  void power();
} mbc3;
