#ifdef CARTRIDGE_CPP

void Cartridge::serialize(serializer &s) {
  if(info.battery) s.array(ramdata, ramsize);
  s.integer(bootrom_enable);

  s.integer(mbc1.ram_enable);
  s.integer(mbc1.rom_select);
  s.integer(mbc1.ram_select);
  s.integer(mbc1.mode_select);

  s.integer(mbc2.ram_enable);
  s.integer(mbc2.rom_select);

  s.integer(mbc3.ram_enable);
  s.integer(mbc3.rom_select);
  s.integer(mbc3.ram_select);
  s.integer(mbc3.rtc_latch);

  s.integer(mbc3.rtc_halt);
  s.integer(mbc3.rtc_second);
  s.integer(mbc3.rtc_minute);
  s.integer(mbc3.rtc_hour);
  s.integer(mbc3.rtc_day);
  s.integer(mbc3.rtc_day_carry);

  s.integer(mbc3.rtc_latch_second);
  s.integer(mbc3.rtc_latch_minute);
  s.integer(mbc3.rtc_latch_hour);
  s.integer(mbc3.rtc_latch_day);
  s.integer(mbc3.rtc_latch_day_carry);

  s.integer(mbc5.ram_enable);
  s.integer(mbc5.rom_select);
  s.integer(mbc5.ram_select);

  s.integer(mmm01.rom_mode);
  s.integer(mmm01.rom_base);

  s.integer(mmm01.ram_enable);
  s.integer(mmm01.rom_select);
  s.integer(mmm01.ram_select);

  s.integer(huc1.ram_writable);
  s.integer(huc1.rom_select);
  s.integer(huc1.ram_select);
  s.integer(huc1.model);

  s.integer(huc3.ram_enable);
  s.integer(huc3.rom_select);
  s.integer(huc3.ram_select);
}

#endif
