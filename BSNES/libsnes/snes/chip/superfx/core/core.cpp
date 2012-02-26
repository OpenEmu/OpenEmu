#ifdef SUPERFX_CPP

#include "opcodes.cpp"
#include "opcode_table.cpp"

uint8 SuperFX::color(uint8 source) {
  if(regs.por.highnibble) return (regs.colr & 0xf0) | (source >> 4);
  if(regs.por.freezehigh) return (regs.colr & 0xf0) | (source & 0x0f);
  return source;
}

void SuperFX::plot(uint8 x, uint8 y) {
  uint8 color = regs.colr;

  if(regs.por.dither && regs.scmr.md != 3) {
    if((x ^ y) & 1) color >>= 4;
    color &= 0x0f;
  }

  if(!regs.por.transparent) {
    if(regs.scmr.md == 3) {
      if(regs.por.freezehigh) {
        if((color & 0x0f) == 0) return;
      } else {
        if(color == 0) return;
      }
    } else {
      if((color & 0x0f) == 0) return;
    }
  }

  uint16 offset = (y << 5) + (x >> 3);
  if(offset != pixelcache[0].offset) {
    pixelcache_flush(pixelcache[1]);
    pixelcache[1] = pixelcache[0];
    pixelcache[0].bitpend = 0x00;
    pixelcache[0].offset = offset;
  }

  x = (x & 7) ^ 7;
  pixelcache[0].data[x] = color;
  pixelcache[0].bitpend |= 1 << x;
  if(pixelcache[0].bitpend == 0xff) {
    pixelcache_flush(pixelcache[1]);
    pixelcache[1] = pixelcache[0];
    pixelcache[0].bitpend = 0x00;
  }
}

uint8 SuperFX::rpix(uint8 x, uint8 y) {
  pixelcache_flush(pixelcache[1]);
  pixelcache_flush(pixelcache[0]);

  unsigned cn;  //character number
  switch(regs.por.obj ? 3 : regs.scmr.ht) {
    case 0: cn = ((x & 0xf8) << 1) + ((y & 0xf8) >> 3); break;
    case 1: cn = ((x & 0xf8) << 1) + ((x & 0xf8) >> 1) + ((y & 0xf8) >> 3); break;
    case 2: cn = ((x & 0xf8) << 1) + ((x & 0xf8) << 0) + ((y & 0xf8) >> 3); break;
    case 3: cn = ((y & 0x80) << 2) + ((x & 0x80) << 1) + ((y & 0x78) << 1) + ((x & 0x78) >> 3); break;
  }
  unsigned bpp = 2 << (regs.scmr.md - (regs.scmr.md >> 1));  // = [regs.scmr.md]{ 2, 4, 4, 8 };
  unsigned addr = 0x700000 + (cn * (bpp << 3)) + (regs.scbr << 10) + ((y & 0x07) * 2);
  uint8 data = 0x00;
  x = (x & 7) ^ 7;

  for(unsigned n = 0; n < bpp; n++) {
    unsigned byte = ((n >> 1) << 4) + (n & 1);  // = [n]{ 0, 1, 16, 17, 32, 33, 48, 49 };
    add_clocks(memory_access_speed);
    data |= ((bus_read(addr + byte) >> x) & 1) << n;
  }

  return data;
}

void SuperFX::pixelcache_flush(pixelcache_t &cache) {
  if(cache.bitpend == 0x00) return;

  uint8 x = cache.offset << 3;
  uint8 y = cache.offset >> 5;

  unsigned cn;  //character number
  switch(regs.por.obj ? 3 : regs.scmr.ht) {
    case 0: cn = ((x & 0xf8) << 1) + ((y & 0xf8) >> 3); break;
    case 1: cn = ((x & 0xf8) << 1) + ((x & 0xf8) >> 1) + ((y & 0xf8) >> 3); break;
    case 2: cn = ((x & 0xf8) << 1) + ((x & 0xf8) << 0) + ((y & 0xf8) >> 3); break;
    case 3: cn = ((y & 0x80) << 2) + ((x & 0x80) << 1) + ((y & 0x78) << 1) + ((x & 0x78) >> 3); break;
  }
  unsigned bpp = 2 << (regs.scmr.md - (regs.scmr.md >> 1));  // = [regs.scmr.md]{ 2, 4, 4, 8 };
  unsigned addr = 0x700000 + (cn * (bpp << 3)) + (regs.scbr << 10) + ((y & 0x07) * 2);

  for(unsigned n = 0; n < bpp; n++) {
    unsigned byte = ((n >> 1) << 4) + (n & 1);  // = [n]{ 0, 1, 16, 17, 32, 33, 48, 49 };
    uint8 data = 0x00;
    for(unsigned x = 0; x < 8; x++) data |= ((cache.data[x] >> n) & 1) << x;
    if(cache.bitpend != 0xff) {
      add_clocks(memory_access_speed);
      data &= cache.bitpend;
      data |= bus_read(addr + byte) & ~cache.bitpend;
    }
    add_clocks(memory_access_speed);
    bus_write(addr + byte, data);
  }

  cache.bitpend = 0x00;
}

#endif
