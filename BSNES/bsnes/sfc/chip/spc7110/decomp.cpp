#ifdef SPC7110_CPP

uint8 SPC7110::Decomp::read() {
  if(decomp_buffer_length == 0) {
    //decompress at least (decomp_buffer_size / 2) bytes to the buffer
    switch(decomp_mode) {
      case 0: mode0(false); break;
      case 1: mode1(false); break;
      case 2: mode2(false); break;
      default: return 0x00;
    }
  }

  uint8 data = decomp_buffer[decomp_buffer_rdoffset++];
  decomp_buffer_rdoffset &= decomp_buffer_size - 1;
  decomp_buffer_length--;
  return data;
}

void SPC7110::Decomp::write(uint8 data) {
  decomp_buffer[decomp_buffer_wroffset++] = data;
  decomp_buffer_wroffset &= decomp_buffer_size - 1;
  decomp_buffer_length++;
}

uint8 SPC7110::Decomp::dataread() {
  unsigned size = cartridge.rom.size() - spc7110.data_rom_offset;
  while(decomp_offset >= size) decomp_offset -= size;
  return cartridge.rom.read(spc7110.data_rom_offset + decomp_offset++);
}

void SPC7110::Decomp::init(unsigned mode, unsigned offset, unsigned index) {
  decomp_mode = mode;
  decomp_offset = offset;

  decomp_buffer_rdoffset = 0;
  decomp_buffer_wroffset = 0;
  decomp_buffer_length   = 0;

  //reset context states
  for(unsigned i = 0; i < 32; i++) {
    context[i].index  = 0;
    context[i].invert = 0;
  }

  switch(decomp_mode) {
    case 0: mode0(true); break;
    case 1: mode1(true); break;
    case 2: mode2(true); break;
  }

  //decompress up to requested output data index
  while(index--) read();
}

//

void SPC7110::Decomp::mode0(bool init) {
  static uint8 val, in, span;
  static int out, inverts, lps, in_count;

  if(init == true) {
    out = inverts = lps = 0;
    span = 0xff;
    val = dataread();
    in = dataread();
    in_count = 8;
    return;
  }

  while(decomp_buffer_length < (decomp_buffer_size >> 1)) {
    for(unsigned bit = 0; bit < 8; bit++) {
      //get context
      uint8 mask = (1 << (bit & 3)) - 1;
      uint8 con = mask + ((inverts & mask) ^ (lps & mask));
      if(bit > 3) con += 15;

      //get prob and mps
      unsigned prob = probability(con);
      unsigned mps = (((out >> 15) & 1) ^ context[con].invert);

      //get bit
      unsigned flag_lps;
      if(val <= span - prob) { //mps
        span = span - prob;
        out = (out << 1) + mps;
        flag_lps = 0;
      } else { //lps
        val = val - (span - (prob - 1));
        span = prob - 1;
        out = (out << 1) + 1 - mps;
        flag_lps = 1;
      }

      //renormalize
      unsigned shift = 0;
      while(span < 0x7f) {
        shift++;

        span = (span << 1) + 1;
        val = (val << 1) + (in >> 7);

        in <<= 1;
        if(--in_count == 0) {
          in = dataread();
          in_count = 8;
        }
      }

      //update processing info
      lps = (lps << 1) + flag_lps;
      inverts = (inverts << 1) + context[con].invert;

      //update context state
      if(flag_lps & toggle_invert(con)) context[con].invert ^= 1;
      if(flag_lps) context[con].index = next_lps(con);
      else if(shift) context[con].index = next_mps(con);
    }

    //save byte
    write(out);
  }
}

void SPC7110::Decomp::mode1(bool init) {
  static int pixelorder[4], realorder[4];
  static uint8 in, val, span;
  static int out, inverts, lps, in_count;

  if(init == true) {
    for(unsigned i = 0; i < 4; i++) pixelorder[i] = i;
    out = inverts = lps = 0;
    span = 0xff;
    val = dataread();
    in = dataread();
    in_count = 8;
    return;
  }

  while(decomp_buffer_length < (decomp_buffer_size >> 1)) {
    for(unsigned pixel = 0; pixel < 8; pixel++) {
      //get first symbol context
      unsigned a = ((out >> (1 * 2)) & 3);
      unsigned b = ((out >> (7 * 2)) & 3);
      unsigned c = ((out >> (8 * 2)) & 3);
      unsigned con = (a == b) ? (b != c) : (b == c) ? 2 : 4 - (a == c);

      //update pixel order
      unsigned m, n;
      for(m = 0; m < 4; m++) if(pixelorder[m] == a) break;
      for(n = m; n > 0; n--) pixelorder[n] = pixelorder[n - 1];
      pixelorder[0] = a;

      //calculate the real pixel order
      for(m = 0; m < 4; m++) realorder[m] = pixelorder[m];

      //rotate reference pixel c value to top
      for(m = 0; m < 4; m++) if(realorder[m] == c) break;
      for(n = m; n > 0; n--) realorder[n] = realorder[n - 1];
      realorder[0] = c;

      //rotate reference pixel b value to top
      for(m = 0; m < 4; m++) if(realorder[m] == b) break;
      for(n = m; n > 0; n--) realorder[n] = realorder[n - 1];
      realorder[0] = b;

      //rotate reference pixel a value to top
      for(m = 0; m < 4; m++) if(realorder[m] == a) break;
      for(n = m; n > 0; n--) realorder[n] = realorder[n - 1];
      realorder[0] = a;

      //get 2 symbols
      for(unsigned bit = 0; bit < 2; bit++) {
        //get prob
        unsigned prob = probability(con);

        //get symbol
        unsigned flag_lps;
        if(val <= span - prob) { //mps
          span = span - prob;
          flag_lps = 0;
        } else { //lps
          val = val - (span - (prob - 1));
          span = prob - 1;
          flag_lps = 1;
        }

        //renormalize
        unsigned shift = 0;
        while(span < 0x7f) {
          shift++;

          span = (span << 1) + 1;
          val = (val << 1) + (in >> 7);

          in <<= 1;
          if(--in_count == 0) {
            in = dataread();
            in_count = 8;
          }
        }

        //update processing info
        lps = (lps << 1) + flag_lps;
        inverts = (inverts << 1) + context[con].invert;

        //update context state
        if(flag_lps & toggle_invert(con)) context[con].invert ^= 1;
        if(flag_lps) context[con].index = next_lps(con);
        else if(shift) context[con].index = next_mps(con);

        //get next context
        con = 5 + (con << 1) + ((lps ^ inverts) & 1);
      }

      //get pixel
      b = realorder[(lps ^ inverts) & 3];
      out = (out << 2) + b;
    }

    //turn pixel data into bitplanes
    unsigned data = deinterleave_2x8(out);
    write(data >> 8);
    write(data >> 0);
  }
}

void SPC7110::Decomp::mode2(bool init) {
  static int pixelorder[16], realorder[16];
  static uint8 bitplanebuffer[16], buffer_index;
  static uint8 in, val, span;
  static int out0, out1, inverts, lps, in_count;

  if(init == true) {
    for(unsigned i = 0; i < 16; i++) pixelorder[i] = i;
    buffer_index = 0;
    out0 = out1 = inverts = lps = 0;
    span = 0xff;
    val = dataread();
    in = dataread();
    in_count = 8;
    return;
  }

  while(decomp_buffer_length < (decomp_buffer_size >> 1)) {
    for(unsigned pixel = 0; pixel < 8; pixel++) {
      //get first symbol context
      unsigned a = ((out0 >> (0 * 4)) & 15);
      unsigned b = ((out0 >> (7 * 4)) & 15);
      unsigned c = ((out1 >> (0 * 4)) & 15);
      unsigned con = 0;
      unsigned refcon = (a == b) ? (b != c) : (b == c) ? 2 : 4 - (a == c);

      //update pixel order
      unsigned m, n;
      for(m = 0; m < 16; m++) if(pixelorder[m] == a) break;
      for(n = m; n >  0; n--) pixelorder[n] = pixelorder[n - 1];
      pixelorder[0] = a;

      //calculate the real pixel order
      for(m = 0; m < 16; m++) realorder[m] = pixelorder[m];

      //rotate reference pixel c value to top
      for(m = 0; m < 16; m++) if(realorder[m] == c) break;
      for(n = m; n >  0; n--) realorder[n] = realorder[n - 1];
      realorder[0] = c;

      //rotate reference pixel b value to top
      for(m = 0; m < 16; m++) if(realorder[m] == b) break;
      for(n = m; n >  0; n--) realorder[n] = realorder[n - 1];
      realorder[0] = b;

      //rotate reference pixel a value to top
      for(m = 0; m < 16; m++) if(realorder[m] == a) break;
      for(n = m; n >  0; n--) realorder[n] = realorder[n - 1];
      realorder[0] = a;

      //get 4 symbols
      for(unsigned bit = 0; bit < 4; bit++) {
        //get prob
        unsigned prob = probability(con);

        //get symbol
        unsigned flag_lps;
        if(val <= span - prob) { //mps
          span = span - prob;
          flag_lps = 0;
        } else { //lps
          val = val - (span - (prob - 1));
          span = prob - 1;
          flag_lps = 1;
        }

        //renormalize
        unsigned shift = 0;
        while(span < 0x7f) {
          shift++;

          span = (span << 1) + 1;
          val = (val << 1) + (in >> 7);

          in <<= 1;
          if(--in_count == 0) {
            in = dataread();
            in_count = 8;
          }
        }

        //update processing info
        lps = (lps << 1) + flag_lps;
        unsigned invertbit = context[con].invert;
        inverts = (inverts << 1) + invertbit;

        //update context state
        if(flag_lps & toggle_invert(con)) context[con].invert ^= 1;
        if(flag_lps) context[con].index = next_lps(con);
        else if(shift) context[con].index = next_mps(con);

        //get next context
        con = mode2_context_table[con][flag_lps ^ invertbit] + (con == 1 ? refcon : 0);
      }

      //get pixel
      b = realorder[(lps ^ inverts) & 0x0f];
      out1 = (out1 << 4) + ((out0 >> 28) & 0x0f);
      out0 = (out0 << 4) + b;
    }

    //convert pixel data into bitplanes
    unsigned data = deinterleave_4x8(out0);
    write(data >> 24);
    write(data >> 16);
    bitplanebuffer[buffer_index++] = data >> 8;
    bitplanebuffer[buffer_index++] = data >> 0;

    if(buffer_index == 16) {
      for(unsigned i = 0; i < 16; i++) write(bitplanebuffer[i]);
      buffer_index = 0;
    }
  }
}

//

const uint8 SPC7110::Decomp::evolution_table[53][4] = {
//{ prob, nextlps, nextmps, toggle invert },

  { 0x5a,  1,  1, 1 },
  { 0x25,  6,  2, 0 },
  { 0x11,  8,  3, 0 },
  { 0x08, 10,  4, 0 },
  { 0x03, 12,  5, 0 },
  { 0x01, 15,  5, 0 },

  { 0x5a,  7,  7, 1 },
  { 0x3f, 19,  8, 0 },
  { 0x2c, 21,  9, 0 },
  { 0x20, 22, 10, 0 },
  { 0x17, 23, 11, 0 },
  { 0x11, 25, 12, 0 },
  { 0x0c, 26, 13, 0 },
  { 0x09, 28, 14, 0 },
  { 0x07, 29, 15, 0 },
  { 0x05, 31, 16, 0 },
  { 0x04, 32, 17, 0 },
  { 0x03, 34, 18, 0 },
  { 0x02, 35,  5, 0 },

  { 0x5a, 20, 20, 1 },
  { 0x48, 39, 21, 0 },
  { 0x3a, 40, 22, 0 },
  { 0x2e, 42, 23, 0 },
  { 0x26, 44, 24, 0 },
  { 0x1f, 45, 25, 0 },
  { 0x19, 46, 26, 0 },
  { 0x15, 25, 27, 0 },
  { 0x11, 26, 28, 0 },
  { 0x0e, 26, 29, 0 },
  { 0x0b, 27, 30, 0 },
  { 0x09, 28, 31, 0 },
  { 0x08, 29, 32, 0 },
  { 0x07, 30, 33, 0 },
  { 0x05, 31, 34, 0 },
  { 0x04, 33, 35, 0 },
  { 0x04, 33, 36, 0 },
  { 0x03, 34, 37, 0 },
  { 0x02, 35, 38, 0 },
  { 0x02, 36,  5, 0 },

  { 0x58, 39, 40, 1 },
  { 0x4d, 47, 41, 0 },
  { 0x43, 48, 42, 0 },
  { 0x3b, 49, 43, 0 },
  { 0x34, 50, 44, 0 },
  { 0x2e, 51, 45, 0 },
  { 0x29, 44, 46, 0 },
  { 0x25, 45, 24, 0 },

  { 0x56, 47, 48, 1 },
  { 0x4f, 47, 49, 0 },
  { 0x47, 48, 50, 0 },
  { 0x41, 49, 51, 0 },
  { 0x3c, 50, 52, 0 },
  { 0x37, 51, 43, 0 },
};

const uint8 SPC7110::Decomp::mode2_context_table[32][2] = {
//{ next 0, next 1 },

  {  1,  2 },

  {  3,  8 },
  { 13, 14 },

  { 15, 16 },
  { 17, 18 },
  { 19, 20 },
  { 21, 22 },
  { 23, 24 },
  { 25, 26 },
  { 25, 26 },
  { 25, 26 },
  { 25, 26 },
  { 25, 26 },
  { 27, 28 },
  { 29, 30 },

  { 31, 31 },
  { 31, 31 },
  { 31, 31 },
  { 31, 31 },
  { 31, 31 },
  { 31, 31 },
  { 31, 31 },
  { 31, 31 },
  { 31, 31 },
  { 31, 31 },
  { 31, 31 },
  { 31, 31 },
  { 31, 31 },
  { 31, 31 },
  { 31, 31 },
  { 31, 31 },

  { 31, 31 },
};

uint8 SPC7110::Decomp::probability  (unsigned n) { return evolution_table[context[n].index][0]; }
uint8 SPC7110::Decomp::next_lps     (unsigned n) { return evolution_table[context[n].index][1]; }
uint8 SPC7110::Decomp::next_mps     (unsigned n) { return evolution_table[context[n].index][2]; }
bool  SPC7110::Decomp::toggle_invert(unsigned n) { return evolution_table[context[n].index][3]; }

unsigned SPC7110::Decomp::deinterleave_2x8(unsigned data) {
  //reverse morton lookup: de-interleave two 8-bit values
  //15, 13, 11,  9,  7,  5,  3,  1 -> 15- 8
  //14, 12, 10,  8,  6,  4,  2,  0 ->  7- 0
  unsigned result = 0;
  for(unsigned mask = 1u << 15; mask; mask >>= 2) result = (result << 1) | (bool)(data & mask);
  for(unsigned mask = 1u << 14; mask; mask >>= 2) result = (result << 1) | (bool)(data & mask);
  return result;
}

unsigned SPC7110::Decomp::deinterleave_4x8(unsigned data) {
  //reverse morton lookup: de-interleave four 8-bit values
  //31, 27, 23, 19, 15, 11,  7,  3 -> 31-24
  //30, 26, 22, 18, 14, 10,  6,  2 -> 23-16
  //29, 25, 21, 17, 13,  9,  5,  1 -> 15- 8
  //28, 24, 20, 16, 12,  8,  4,  0 ->  7- 0
  unsigned result = 0;
  for(unsigned mask = 1u << 31; mask; mask >>= 4) result = (result << 1) | (bool)(data & mask);
  for(unsigned mask = 1u << 30; mask; mask >>= 4) result = (result << 1) | (bool)(data & mask);
  for(unsigned mask = 1u << 29; mask; mask >>= 4) result = (result << 1) | (bool)(data & mask);
  for(unsigned mask = 1u << 28; mask; mask >>= 4) result = (result << 1) | (bool)(data & mask);
  return result;
}

//

void SPC7110::Decomp::reset() {
  //mode 3 is invalid; this is treated as a special case to always return 0x00
  //set to mode 3 so that reading decomp port before starting first decomp will return 0x00
  decomp_mode = 3;

  decomp_buffer_rdoffset = 0;
  decomp_buffer_wroffset = 0;
  decomp_buffer_length   = 0;
}

SPC7110::Decomp::Decomp() {
  decomp_buffer = new uint8_t[decomp_buffer_size];
  reset();
}

SPC7110::Decomp::~Decomp() {
  delete[] decomp_buffer;
}

#endif
