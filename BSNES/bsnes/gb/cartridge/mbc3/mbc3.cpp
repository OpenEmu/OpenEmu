#ifdef CARTRIDGE_CPP

void Cartridge::MBC3::second() {
  if(rtc_halt == false) {
    if(++rtc_second >= 60) {
      rtc_second = 0;
      if(++rtc_minute >= 60) {
        rtc_minute = 0;
        if(++rtc_hour >= 24) {
          rtc_hour = 0;
          if(++rtc_day >= 512) {
            rtc_day = 0;
            rtc_day_carry = true;
          }
        }
      }
    }
  }
}

uint8 Cartridge::MBC3::mmio_read(uint16 addr) {
  if((addr & 0xc000) == 0x0000) {  //$0000-3fff
    return cartridge.rom_read(addr);
  }

  if((addr & 0xc000) == 0x4000) {  //$4000-7fff
    return cartridge.rom_read((rom_select << 14) | (addr & 0x3fff));
  }

  if((addr & 0xe000) == 0xa000) {  //$a000-bfff
    if(ram_enable) {
      if(ram_select >= 0x00 && ram_select <= 0x03) {
        return cartridge.ram_read((ram_select << 13) | (addr & 0x1fff));
      }
      if(ram_select == 0x08) return rtc_latch_second;
      if(ram_select == 0x09) return rtc_latch_minute;
      if(ram_select == 0x0a) return rtc_latch_hour;
      if(ram_select == 0x0b) return rtc_latch_day;
      if(ram_select == 0x0c) return (rtc_latch_day_carry << 7) | (rtc_latch_day >> 8);
    }
    return 0x00;
  }

  return 0x00;
}

void Cartridge::MBC3::mmio_write(uint16 addr, uint8 data) {
  if((addr & 0xe000) == 0x0000) {  //$0000-1fff
    ram_enable = (data & 0x0f) == 0x0a;
    return;
  }

  if((addr & 0xe000) == 0x2000) {  //$2000-3fff
    rom_select = (data & 0x7f) + ((data & 0x7f) == 0);
    return;
  }

  if((addr & 0xe000) == 0x4000) {  //$4000-5fff
    ram_select = data;
    return;
  }

  if((addr & 0xe000) == 0x6000) {  //$6000-7fff
    if(rtc_latch == 0 && data == 1) {
      rtc_latch_second = rtc_second;
      rtc_latch_minute = rtc_minute;
      rtc_latch_hour = rtc_hour;
      rtc_latch_day = rtc_day;
      rtc_latch_day_carry = rtc_day_carry;
    }
    rtc_latch = data;
    return;
  }

  if((addr & 0xe000) == 0xa000) {  //$a000-bfff
    if(ram_enable) {
      if(ram_select >= 0x00 && ram_select <= 0x03) {
        cartridge.ram_write((ram_select << 13) | (addr & 0x1fff), data);
      } else if(ram_select == 0x08) {
        if(data >= 60) data = 0;
        rtc_second = data;
      } else if(ram_select == 0x09) {
        if(data >= 60) data = 0;
        rtc_minute = data;
      } else if(ram_select == 0x0a) {
        if(data >= 24) data = 0;
        rtc_hour = data;
      } else if(ram_select == 0x0b) {
        rtc_day = (rtc_day & 0x0100) | data;
      } else if(ram_select == 0x0c) {
        rtc_day = ((data & 1) << 8) | (rtc_day & 0xff);
        rtc_halt = data & 0x40;
        rtc_day_carry = data & 0x80;
      }
    }
    return;
  }
}

void Cartridge::MBC3::power() {
  ram_enable = false;
  rom_select = 0x01;
  ram_select = 0x00;
  rtc_latch = 0;

  rtc_halt = true;
  rtc_second = 0;
  rtc_minute = 0;
  rtc_hour = 0;
  rtc_day = 0;
  rtc_day_carry = false;

  rtc_latch_second = 0;
  rtc_latch_minute = 0;
  rtc_latch_hour = 0;
  rtc_latch_day = 0;
  rtc_latch_day_carry = false;
}

#endif
