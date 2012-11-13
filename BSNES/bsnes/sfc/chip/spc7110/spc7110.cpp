#include <sfc/sfc.hpp>

#define SPC7110_CPP
namespace SuperFamicom {

SPC7110 spc7110;

#include "serialization.cpp"
#include "decomp.cpp"

const unsigned SPC7110::months[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void SPC7110::init() {
}

void SPC7110::load() {
  for(unsigned n = 0; n < 20; n++) rtc[n] = 0xff;
  if(cartridge.has_spc7110rtc()) interface->memory.append({ID::SPC7110RTC, "rtc.ram"});
}

void SPC7110::unload() {
}

void SPC7110::power() {
}

void SPC7110::reset() {
  r4801 = 0x00;
  r4802 = 0x00;
  r4803 = 0x00;
  r4804 = 0x00;
  r4805 = 0x00;
  r4806 = 0x00;
  r4807 = 0x00;
  r4808 = 0x00;
  r4809 = 0x00;
  r480a = 0x00;
  r480b = 0x00;
  r480c = 0x00;

  decomp.reset();

  r4811 = 0x00;
  r4812 = 0x00;
  r4813 = 0x00;
  r4814 = 0x00;
  r4815 = 0x00;
  r4816 = 0x00;
  r4817 = 0x00;
  r4818 = 0x00;

  r481x = 0x00;
  r4814_latch = false;
  r4815_latch = false;

  r4820 = 0x00;
  r4821 = 0x00;
  r4822 = 0x00;
  r4823 = 0x00;
  r4824 = 0x00;
  r4825 = 0x00;
  r4826 = 0x00;
  r4827 = 0x00;
  r4828 = 0x00;
  r4829 = 0x00;
  r482a = 0x00;
  r482b = 0x00;
  r482c = 0x00;
  r482d = 0x00;
  r482e = 0x00;
  r482f = 0x00;

  r4830 = 0x00;
  mmio_write(0x4831, 0);
  mmio_write(0x4832, 1);
  mmio_write(0x4833, 2);
  r4834 = 0x00;

  r4840 = 0x00;
  r4841 = 0x00;
  r4842 = 0x00;

  if(cartridge.has_spc7110rtc()) {
    rtc_state = RTCS_Inactive;
    rtc_mode  = RTCM_Linear;
    rtc_index = 0;
  }
}

unsigned SPC7110::datarom_addr(unsigned addr) {
  unsigned size = cartridge.rom.size() - data_rom_offset;
  while(addr >= size) addr -= size;
  return data_rom_offset + addr;
}

unsigned SPC7110::data_pointer()   { return r4811 + (r4812 << 8) + (r4813 << 16); }
unsigned SPC7110::data_adjust()    { return r4814 + (r4815 << 8); }
unsigned SPC7110::data_increment() { return r4816 + (r4817 << 8); }
void SPC7110::set_data_pointer(unsigned addr) { r4811 = addr; r4812 = addr >> 8; r4813 = addr >> 16; }
void SPC7110::set_data_adjust(unsigned addr)  { r4814 = addr; r4815 = addr >> 8; }

void SPC7110::update_time(int offset) {
  time_t rtc_time = (rtc[16] << 0) | (rtc[17] << 8) | (rtc[18] << 16) | (rtc[19] << 24);
  time_t current_time = time(0) - offset;

  //sizeof(time_t) is platform-dependent; though rtc[] needs to be platform-agnostic.
  //yet platforms with 32-bit signed time_t will overflow every ~68 years. handle this by
  //accounting for overflow at the cost of 1-bit precision (to catch underflow). this will allow
  //rtc[] timestamp to remain valid for up to ~34 years from the last update, even if
  //time_t overflows. calculation should be valid regardless of number representation, time_t size,
  //or whether time_t is signed or unsigned.
  time_t diff
  = (current_time >= rtc_time)
  ? (current_time - rtc_time)
  : (std::numeric_limits<time_t>::max() - rtc_time + current_time + 1);  //compensate for overflow
  if(diff > std::numeric_limits<time_t>::max() / 2) diff = 0;            //compensate for underflow

  bool update = true;
  if(rtc[13] & 1) update = false;  //do not update if CR0 timer disable flag is set
  if(rtc[15] & 3) update = false;  //do not update if CR2 timer disable flags are set

  if(diff > 0 && update == true) {
    unsigned second  = rtc[ 0] + rtc[ 1] * 10;
    unsigned minute  = rtc[ 2] + rtc[ 3] * 10;
    unsigned hour    = rtc[ 4] + rtc[ 5] * 10;
    unsigned day     = rtc[ 6] + rtc[ 7] * 10;
    unsigned month   = rtc[ 8] + rtc[ 9] * 10;
    unsigned year    = rtc[10] + rtc[11] * 10;
    unsigned weekday = rtc[12];

    day--;
    month--;
    year += (year >= 90) ? 1900 : 2000;  //range = 1990-2089

    second += diff;
    while(second >= 60) {
      second -= 60;

      minute++;
      if(minute < 60) continue;
      minute = 0;

      hour++;
      if(hour < 24) continue;
      hour = 0;

      day++;
      weekday = (weekday + 1) % 7;
      unsigned days = months[month % 12];
      if(days == 28) {
        bool leapyear = false;
        if((year % 4) == 0) {
          leapyear = true;
          if((year % 100) == 0 && (year % 400) != 0) leapyear = false;
        }
        if(leapyear) days++;
      }
      if(day < days) continue;
      day = 0;

      month++;
      if(month < 12) continue;
      month = 0;

      year++;
    }

    day++;
    month++;
    year %= 100;

    rtc[ 0] = second % 10;
    rtc[ 1] = second / 10;
    rtc[ 2] = minute % 10;
    rtc[ 3] = minute / 10;
    rtc[ 4] = hour % 10;
    rtc[ 5] = hour / 10;
    rtc[ 6] = day % 10;
    rtc[ 7] = day / 10;
    rtc[ 8] = month % 10;
    rtc[ 9] = month / 10;
    rtc[10] = year % 10;
    rtc[11] = (year / 10) % 10;
    rtc[12] = weekday % 7;
  }

  rtc[16] = current_time >>  0;
  rtc[17] = current_time >>  8;
  rtc[18] = current_time >> 16;
  rtc[19] = current_time >> 24;
}

uint8 SPC7110::mmio_read(unsigned addr) {
  addr &= 0xffff;

  switch(addr) {
    //==================
    //decompression unit
    //==================

    case 0x4800: {
      uint16 counter = (r4809 + (r480a << 8));
      counter--;
      r4809 = counter;
      r480a = counter >> 8;
      return decomp.read();
    }
    case 0x4801: return r4801;
    case 0x4802: return r4802;
    case 0x4803: return r4803;
    case 0x4804: return r4804;
    case 0x4805: return r4805;
    case 0x4806: return r4806;
    case 0x4807: return r4807;
    case 0x4808: return r4808;
    case 0x4809: return r4809;
    case 0x480a: return r480a;
    case 0x480b: return r480b;
    case 0x480c: {
      uint8 status = r480c;
      r480c &= 0x7f;
      return status;
    }

    //==============
    //data port unit
    //==============

    case 0x4810: {
      if(r481x != 0x07) return 0x00;

      unsigned addr = data_pointer();
      unsigned adjust = data_adjust();
      if(r4818 & 8) adjust = (int16)adjust;  //16-bit sign extend

      unsigned adjustaddr = addr;
      if(r4818 & 2) {
        adjustaddr += adjust;
        set_data_adjust(adjust + 1);
      }

      uint8 data = cartridge.rom.read(datarom_addr(adjustaddr));
      if(!(r4818 & 2)) {
        unsigned increment = (r4818 & 1) ? data_increment() : 1;
        if(r4818 & 4) increment = (int16)increment;  //16-bit sign extend

        if((r4818 & 16) == 0) {
          set_data_pointer(addr + increment);
        } else {
          set_data_adjust(adjust + increment);
        }
      }

      return data;
    }
    case 0x4811: return r4811;
    case 0x4812: return r4812;
    case 0x4813: return r4813;
    case 0x4814: return r4814;
    case 0x4815: return r4815;
    case 0x4816: return r4816;
    case 0x4817: return r4817;
    case 0x4818: return r4818;
    case 0x481a: {
      if(r481x != 0x07) return 0x00;

      unsigned addr = data_pointer();
      unsigned adjust = data_adjust();
      if(r4818 & 8) adjust = (int16)adjust;  //16-bit sign extend

      uint8 data = cartridge.rom.read(datarom_addr(addr + adjust));
      if((r4818 & 0x60) == 0x60) {
        if((r4818 & 16) == 0) {
          set_data_pointer(addr + adjust);
        } else {
          set_data_adjust(adjust + adjust);
        }
      }

      return data;
    }

    //=========
    //math unit
    //=========

    case 0x4820: return r4820;
    case 0x4821: return r4821;
    case 0x4822: return r4822;
    case 0x4823: return r4823;
    case 0x4824: return r4824;
    case 0x4825: return r4825;
    case 0x4826: return r4826;
    case 0x4827: return r4827;
    case 0x4828: return r4828;
    case 0x4829: return r4829;
    case 0x482a: return r482a;
    case 0x482b: return r482b;
    case 0x482c: return r482c;
    case 0x482d: return r482d;
    case 0x482e: return r482e;
    case 0x482f: {
      uint8 status = r482f;
      r482f &= 0x7f;
      return status;
    }

    //===================
    //memory mapping unit
    //===================

    case 0x4830: return r4830;
    case 0x4831: return r4831;
    case 0x4832: return r4832;
    case 0x4833: return r4833;
    case 0x4834: return r4834;

    //====================
    //real-time clock unit
    //====================

    case 0x4840: return r4840;
    case 0x4841: {
      if(rtc_state == RTCS_Inactive || rtc_state == RTCS_ModeSelect) return 0x00;

      r4842 = 0x80;
      uint8 data = rtc[rtc_index];
      rtc_index = (rtc_index + 1) & 15;
      return data;
    }
    case 0x4842: {
      uint8 status = r4842;
      r4842 &= 0x7f;
      return status;
    }
  }

  return cpu.regs.mdr;
}

void SPC7110::mmio_write(unsigned addr, uint8 data) {
  addr &= 0xffff;

  switch(addr) {
    //==================
    //decompression unit
    //==================

    case 0x4801: r4801 = data; break;
    case 0x4802: r4802 = data; break;
    case 0x4803: r4803 = data; break;
    case 0x4804: r4804 = data; break;
    case 0x4805: r4805 = data; break;
    case 0x4806: {
      r4806 = data;

      unsigned table   = (r4801 + (r4802 << 8) + (r4803 << 16));
      unsigned index   = (r4804 << 2);
      unsigned length  = (r4809 + (r480a << 8));
      unsigned addr    = datarom_addr(table + index);
      unsigned mode    = (cartridge.rom.read(addr + 0));
      unsigned offset  = (cartridge.rom.read(addr + 1) << 16)
                       + (cartridge.rom.read(addr + 2) <<  8)
                       + (cartridge.rom.read(addr + 3) <<  0);

      decomp.init(mode, offset, (r4805 + (r4806 << 8)) << mode);
      r480c = 0x80;
    } break;

    case 0x4807: r4807 = data; break;
    case 0x4808: r4808 = data; break;
    case 0x4809: r4809 = data; break;
    case 0x480a: r480a = data; break;
    case 0x480b: r480b = data; break;

    //==============
    //data port unit
    //==============

    case 0x4811: r4811 = data; r481x |= 0x01; break;
    case 0x4812: r4812 = data; r481x |= 0x02; break;
    case 0x4813: r4813 = data; r481x |= 0x04; break;
    case 0x4814: {
      r4814 = data;
      r4814_latch = true;
      if(!r4815_latch) break;
      if(!(r4818 & 2)) break;
      if(r4818 & 0x10) break;

      if((r4818 & 0x60) == 0x20) {
        unsigned increment = data_adjust() & 0xff;
        if(r4818 & 8) increment = (int8)increment;  //8-bit sign extend
        set_data_pointer(data_pointer() + increment);
      } else if((r4818 & 0x60) == 0x40) {
        unsigned increment = data_adjust();
        if(r4818 & 8) increment = (int16)increment;  //16-bit sign extend
        set_data_pointer(data_pointer() + increment);
      }
    } break;
    case 0x4815: {
      r4815 = data;
      r4815_latch = true;
      if(!r4814_latch) break;
      if(!(r4818 & 2)) break;
      if(r4818 & 0x10) break;

      if((r4818 & 0x60) == 0x20) {
        unsigned increment = data_adjust() & 0xff;
        if(r4818 & 8) increment = (int8)increment;  //8-bit sign extend
        set_data_pointer(data_pointer() + increment);
      } else if((r4818 & 0x60) == 0x40) {
        unsigned increment = data_adjust();
        if(r4818 & 8) increment = (int16)increment;  //16-bit sign extend
        set_data_pointer(data_pointer() + increment);
      }
    } break;
    case 0x4816: r4816 = data; break;
    case 0x4817: r4817 = data; break;
    case 0x4818: {
      if(r481x != 0x07) break;

      r4818 = data;
      r4814_latch = r4815_latch = false;
    } break;

    //=========
    //math unit
    //=========

    case 0x4820: r4820 = data; break;
    case 0x4821: r4821 = data; break;
    case 0x4822: r4822 = data; break;
    case 0x4823: r4823 = data; break;
    case 0x4824: r4824 = data; break;
    case 0x4825: {
      r4825 = data;

      if(r482e & 1) {
        //signed 16-bit x 16-bit multiplication
        int16 r0 = (int16)(r4824 + (r4825 << 8));
        int16 r1 = (int16)(r4820 + (r4821 << 8));

        signed result = r0 * r1;
        r4828 = result;
        r4829 = result >> 8;
        r482a = result >> 16;
        r482b = result >> 24;
      } else {
        //unsigned 16-bit x 16-bit multiplication
        uint16 r0 = (uint16)(r4824 + (r4825 << 8));
        uint16 r1 = (uint16)(r4820 + (r4821 << 8));

        unsigned result = r0 * r1;
        r4828 = result;
        r4829 = result >> 8;
        r482a = result >> 16;
        r482b = result >> 24;
      }

      r482f = 0x80;
    } break;
    case 0x4826: r4826 = data; break;
    case 0x4827: {
      r4827 = data;

      if(r482e & 1) {
        //signed 32-bit x 16-bit division
        int32 dividend = (int32)(r4820 + (r4821 << 8) + (r4822 << 16) + (r4823 << 24));
        int16 divisor  = (int16)(r4826 + (r4827 << 8));

        int32 quotient;
        int16 remainder;

        if(divisor) {
          quotient  = (int32)(dividend / divisor);
          remainder = (int32)(dividend % divisor);
        } else {
          //illegal division by zero
          quotient  = 0;
          remainder = dividend & 0xffff;
        }

        r4828 = quotient;
        r4829 = quotient >> 8;
        r482a = quotient >> 16;
        r482b = quotient >> 24;

        r482c = remainder;
        r482d = remainder >> 8;
      } else {
        //unsigned 32-bit x 16-bit division
        uint32 dividend = (uint32)(r4820 + (r4821 << 8) + (r4822 << 16) + (r4823 << 24));
        uint16 divisor  = (uint16)(r4826 + (r4827 << 8));

        uint32 quotient;
        uint16 remainder;

        if(divisor) {
          quotient  = (uint32)(dividend / divisor);
          remainder = (uint16)(dividend % divisor);
        } else {
          //illegal division by zero
          quotient  = 0;
          remainder = dividend & 0xffff;
        }

        r4828 = quotient;
        r4829 = quotient >> 8;
        r482a = quotient >> 16;
        r482b = quotient >> 24;

        r482c = remainder;
        r482d = remainder >> 8;
      }

      r482f = 0x80;
    } break;

    case 0x482e: {
      //reset math unit
      r4820 = r4821 = r4822 = r4823 = 0;
      r4824 = r4825 = r4826 = r4827 = 0;
      r4828 = r4829 = r482a = r482b = 0;
      r482c = r482d = 0;

      r482e = data;
    } break;

    //===================
    //memory mapping unit
    //===================

    case 0x4830: r4830 = data; break;

    case 0x4831: {
      r4831 = data;
      dx_offset = datarom_addr(data * 0x100000);
    } break;

    case 0x4832: {
      r4832 = data;
      ex_offset = datarom_addr(data * 0x100000);
    } break;

    case 0x4833: {
      r4833 = data;
      fx_offset = datarom_addr(data * 0x100000);
    } break;

    case 0x4834: r4834 = data; break;

    //====================
    //real-time clock unit
    //====================

    case 0x4840: {
      r4840 = data;
      if(!(r4840 & 1)) {
        //disable RTC
        rtc_state = RTCS_Inactive;
        update_time();
      } else {
        //enable RTC
        r4842 = 0x80;
        rtc_state = RTCS_ModeSelect;
      }
    } break;

    case 0x4841: {
      r4841 = data;

      switch(rtc_state) {
        case RTCS_ModeSelect: {
          if(data == RTCM_Linear || data == RTCM_Indexed) {
            r4842 = 0x80;
            rtc_state = RTCS_IndexSelect;
            rtc_mode  = (RTC_Mode)data;
            rtc_index = 0;
          }
        } break;

        case RTCS_IndexSelect: {
          r4842 = 0x80;
          rtc_index = data & 15;
          if(rtc_mode == RTCM_Linear) rtc_state = RTCS_Write;
        } break;

        case RTCS_Write: {
          r4842 = 0x80;

          //control register 0
          if(rtc_index == 13) {
            //increment second counter
            if(data & 2) update_time(+1);

            //round minute counter
            if(data & 8) {
              update_time();

              unsigned second = rtc[ 0] + rtc[ 1] * 10;
              //clear seconds
              rtc[0] = 0;
              rtc[1] = 0;

              if(second >= 30) update_time(+60);
            }
          }

          //control register 2
          if(rtc_index == 15) {
            //disable timer and clear second counter
            if((data & 1) && !(rtc[15] & 1)) {
              update_time();

              //clear seconds
              rtc[0] = 0;
              rtc[1] = 0;
            }

            //disable timer
            if((data & 2) && !(rtc[15] & 2)) {
              update_time();
            }
          }

          rtc[rtc_index] = data & 15;
          rtc_index = (rtc_index + 1) & 15;
        } break;
      } //switch(rtc_state)
    } break;
  }
}

SPC7110::SPC7110() {
}

//============
//SPC7110::MCU
//============

uint8 SPC7110::mcu_read(unsigned addr) {
  if(addr <= 0xdfffff) return cartridge.rom.read(dx_offset + (addr & 0x0fffff));
  if(addr <= 0xefffff) return cartridge.rom.read(ex_offset + (addr & 0x0fffff));
  if(addr <= 0xffffff) return cartridge.rom.read(fx_offset + (addr & 0x0fffff));
  return cpu.regs.mdr;
}

void SPC7110::mcu_write(unsigned addr, uint8 data) {
}

//============
//SPC7110::DCU
//============

uint8 SPC7110::dcu_read(unsigned) {
  return mmio_read(0x4800);
}

void SPC7110::dcu_write(unsigned, uint8) {
}

//============
//SPC7110::RAM
//============

uint8 SPC7110::ram_read(unsigned addr) {
  return cartridge.ram.read(addr & 0x1fff);
}

void SPC7110::ram_write(unsigned addr, uint8 data) {
  if(r4830 & 0x80) cartridge.ram.write(addr & 0x1fff, data);
}

}
