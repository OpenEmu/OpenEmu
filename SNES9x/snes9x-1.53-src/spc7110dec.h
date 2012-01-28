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

#ifndef _SPC7110DEC_H_
#define _SPC7110DEC_H_

class SPC7110Decomp {
public:
  uint8 read();
  void init(unsigned mode, unsigned offset, unsigned index);
  void reset();

  SPC7110Decomp();
  ~SPC7110Decomp();

  unsigned decomp_mode;
  unsigned decomp_offset;

  //read() will spool chunks half the size of decomp_buffer_size
  enum { decomp_buffer_size = SPC7110_DECOMP_BUFFER_SIZE }; //must be >= 64, and must be a power of two
  uint8 *decomp_buffer;
  unsigned decomp_buffer_rdoffset;
  unsigned decomp_buffer_wroffset;
  unsigned decomp_buffer_length;

  void write(uint8 data);
  uint8 dataread();

  void mode0(bool init);
  void mode1(bool init);
  void mode2(bool init);

  static const uint8 evolution_table[53][4];
  static const uint8 mode2_context_table[32][2];

  struct ContextState {
    uint8 index;
    uint8 invert;
  } context[32];

  uint8 probability(unsigned n);
  uint8 next_lps(unsigned n);
  uint8 next_mps(unsigned n);
  bool toggle_invert(unsigned n);

  unsigned morton16[2][256];
  unsigned morton32[4][256];
  unsigned morton_2x8(unsigned data);
  unsigned morton_4x8(unsigned data);
};

#endif
