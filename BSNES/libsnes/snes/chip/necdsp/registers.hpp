struct Flag {
  bool s1, s0, c, z, ov1, ov0;

  inline operator unsigned() const {
    return (s1 << 5) + (s0 << 4) + (c << 3) + (z << 2) + (ov1 << 1) + (ov0 << 0);
  }

  inline unsigned operator=(unsigned d) {
    s1 = d & 0x20; s0 = d & 0x10; c = d & 0x08; z = d & 0x04; ov1 = d & 0x02; ov0 = d & 0x01;
    return d;
  }
};

struct Status {
  bool rqm, usf1, usf0, drs, dma, drc, soc, sic, ei, p1, p0;

  inline operator unsigned() const {
    return (rqm << 15) + (usf1 << 14) + (usf0 << 13) + (drs << 12)
         + (dma << 11) + (drc  << 10) + (soc  <<  9) + (sic <<  8)
         + (ei  <<  7) + (p1   <<  1) + (p0   <<  0);
  }

  inline unsigned operator=(unsigned d) {
    rqm = d & 0x8000; usf1 = d & 0x4000; usf0 = d & 0x2000; drs = d & 0x1000;
    dma = d & 0x0800; drc  = d & 0x0400; soc  = d & 0x0200; sic = d & 0x0100;
    ei  = d & 0x0080; p1   = d & 0x0002; p0   = d & 0x0001;
    return d;
  }
};

struct Regs {
  uint16 stack[16];  //LIFO
  varuint pc;        //program counter
  varuint rp;        //ROM pointer
  varuint dp;        //data pointer
  uint4 sp;          //stack pointer
  int16 k;
  int16 l;
  int16 m;
  int16 n;
  int16 a;           //accumulator
  int16 b;           //accumulator
  Flag flaga;
  Flag flagb;
  uint16 tr;         //temporary register
  uint16 trb;        //temporary register
  Status sr;         //status register
  uint16 dr;         //data register
  uint16 si;
  uint16 so;
} regs;
