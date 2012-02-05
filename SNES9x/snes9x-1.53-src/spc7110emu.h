/*****
 * SPC7110 emulator - version 0.03 (2008-08-10)
 * Copyright (c) 2008, byuu and neviksti
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * The software is provided "as is" and the author disclaims all warranties
 * with regard to this software including all implied warranties of
 * merchantibility and fitness, in no event shall the author be liable for
 * any special, direct, indirect, or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether in an
 * action of contract, negligence or other tortious action, arising out of
 * or in connection with the use or performance of this software.
 *****/


#ifndef _SPC7110EMU_H_
#define _SPC7110EMU_H_

#include "spc7110dec.h"

class SPC7110 {
public:
  void init();
  void enable();
  void power();
  void reset();

  unsigned datarom_addr(unsigned addr);

  unsigned data_pointer();
  unsigned data_adjust();
  unsigned data_increment();
  void set_data_pointer(unsigned addr);
  void set_data_adjust(unsigned addr);

  void update_time(int offset = 0);
  time_t create_time();

  uint8 mmio_read (unsigned addr);
  void  mmio_write(unsigned addr, uint8 data);

  uint8 read (unsigned addr);
  void  write(unsigned addr, uint8 data);

  //spc7110decomp
  void decomp_init();
  uint8 decomp_read();

  SPC7110();

  //==================
  //decompression unit
  //==================
  uint8 r4801; //compression table low
  uint8 r4802; //compression table high
  uint8 r4803; //compression table bank
  uint8 r4804; //compression table index
  uint8 r4805; //decompression buffer index low
  uint8 r4806; //decompression buffer index high
  uint8 r4807; //???
  uint8 r4808; //???
  uint8 r4809; //compression length low
  uint8 r480a; //compression length high
  uint8 r480b; //decompression control register
  uint8 r480c; //decompression status

  SPC7110Decomp decomp;

  //==============
  //data port unit
  //==============
  uint8 r4811; //data pointer low
  uint8 r4812; //data pointer high
  uint8 r4813; //data pointer bank
  uint8 r4814; //data adjust low
  uint8 r4815; //data adjust high
  uint8 r4816; //data increment low
  uint8 r4817; //data increment high
  uint8 r4818; //data port control register

  uint8 r481x;

  bool r4814_latch;
  bool r4815_latch;

  //=========
  //math unit
  //=========
  uint8 r4820; //16-bit multiplicand B0, 32-bit dividend B0
  uint8 r4821; //16-bit multiplicand B1, 32-bit dividend B1
  uint8 r4822; //32-bit dividend B2
  uint8 r4823; //32-bit dividend B3
  uint8 r4824; //16-bit multiplier B0
  uint8 r4825; //16-bit multiplier B1
  uint8 r4826; //16-bit divisor B0
  uint8 r4827; //16-bit divisor B1
  uint8 r4828; //32-bit product B0, 32-bit quotient B0
  uint8 r4829; //32-bit product B1, 32-bit quotient B1
  uint8 r482a; //32-bit product B2, 32-bit quotient B2
  uint8 r482b; //32-bit product B3, 32-bit quotient B3
  uint8 r482c; //16-bit remainder B0
  uint8 r482d; //16-bit remainder B1
  uint8 r482e; //math control register
  uint8 r482f; //math status

  //===================
  //memory mapping unit
  //===================
  uint8 r4830; //SRAM write enable
  uint8 r4831; //$[d0-df]:[0000-ffff] mapping
  uint8 r4832; //$[e0-ef]:[0000-ffff] mapping
  uint8 r4833; //$[f0-ff]:[0000-ffff] mapping
  uint8 r4834; //???

  unsigned dx_offset;
  unsigned ex_offset;
  unsigned fx_offset;

  //====================
  //real-time clock unit
  //====================
  uint8 r4840; //RTC latch
  uint8 r4841; //RTC index/data port
  uint8 r4842; //RTC status

  enum RTC_State { RTCS_Inactive, RTCS_ModeSelect, RTCS_IndexSelect, RTCS_Write } rtc_state;
  enum RTC_Mode  { RTCM_Linear = 0x03, RTCM_Indexed = 0x0c } rtc_mode;
  unsigned rtc_index;

  static const unsigned months[12];
};

#endif
