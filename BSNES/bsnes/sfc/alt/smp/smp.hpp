struct SMP : Thread {
  uint8 iplrom[64];
  uint8 *apuram;

  enum : bool { Threaded = false };
  alwaysinline void synchronize_cpu();
  alwaysinline void synchronize_dsp();

  unsigned port_read(unsigned port);
  void port_write(unsigned port, unsigned data);

  unsigned mmio_read(unsigned addr);
  void mmio_write(unsigned addr, unsigned data);

  void enter();
  void power();
  void reset();

  void serialize(serializer&);
  SMP();
  ~SMP();

  void disassemble_opcode(char *output, uint16 addr);

//private:
  struct Flags {
    bool n, v, p, b, h, i, z, c;

    alwaysinline operator unsigned() const {
      return (n << 7) | (v << 6) | (p << 5) | (b << 4)
           | (h << 3) | (i << 2) | (z << 1) | (c << 0);
    };

    alwaysinline unsigned operator=(unsigned data) {
      n = data & 0x80; v = data & 0x40; p = data & 0x20; b = data & 0x10;
      h = data & 0x08; i = data & 0x04; z = data & 0x02; c = data & 0x01;
      return data;
    }

    alwaysinline unsigned operator|=(unsigned data) { return operator=(operator unsigned() | data); }
    alwaysinline unsigned operator^=(unsigned data) { return operator=(operator unsigned() ^ data); }
    alwaysinline unsigned operator&=(unsigned data) { return operator=(operator unsigned() & data); }
  };

  unsigned opcode_number;
  unsigned opcode_cycle;

  struct Regs {
    uint16 pc;
    uint8 sp;
    union {
      uint16 ya;
      struct { uint8 order_lsb2(a, y); };
    };
    uint8 x;
    Flags p;
  } regs;

  uint16 rd, wr, dp, sp, ya, bit;

  struct Status {
    //$00f1
    bool iplrom_enable;

    //$00f2
    unsigned dsp_addr;

    //$00f8,$00f9
    unsigned ram00f8;
    unsigned ram00f9;
  } status;

  template<unsigned frequency>
  struct Timer {
    bool enable;
    uint8 target;
    uint8 stage1_ticks;
    uint8 stage2_ticks;
    uint8 stage3_ticks;

    void tick();
    void tick(unsigned clocks);
  };

  Timer<128> timer0;
  Timer<128> timer1;
  Timer< 16> timer2;

  void tick();
  alwaysinline void op_io();
  alwaysinline uint8 op_read(uint16 addr);
  alwaysinline void op_write(uint16 addr, uint8 data);
  alwaysinline void op_step();
  static const unsigned cycle_count_table[256];
  uint64 cycle_table_cpu[256];
  unsigned cycle_table_dsp[256];
  uint64 cycle_step_cpu;

  uint8  op_adc (uint8  x, uint8  y);
  uint16 op_addw(uint16 x, uint16 y);
  uint8  op_and (uint8  x, uint8  y);
  uint8  op_cmp (uint8  x, uint8  y);
  uint16 op_cmpw(uint16 x, uint16 y);
  uint8  op_eor (uint8  x, uint8  y);
  uint8  op_inc (uint8  x);
  uint8  op_dec (uint8  x);
  uint8  op_or  (uint8  x, uint8  y);
  uint8  op_sbc (uint8  x, uint8  y);
  uint16 op_subw(uint16 x, uint16 y);
  uint8  op_asl (uint8  x);
  uint8  op_lsr (uint8  x);
  uint8  op_rol (uint8  x);
  uint8  op_ror (uint8  x);
};

extern SMP smp;
