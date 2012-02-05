/*****
 * S-RTC emulation code
 * Copyright (c) byuu
 *****/


#ifndef _SRTCEMU_H_
#define _SRTCEMU_H_

class SRTC {
public:
  void update_time();
  unsigned weekday(unsigned year, unsigned month, unsigned day);

  void init();
  void enable();
  void power();
  void reset();

  uint8 mmio_read (unsigned addr);
  void  mmio_write(unsigned addr, uint8 data);

  SRTC();

  static const unsigned months[12];
  enum RTC_Mode { RTCM_Ready, RTCM_Command, RTCM_Read, RTCM_Write } rtc_mode;
  signed rtc_index;
};

#endif
