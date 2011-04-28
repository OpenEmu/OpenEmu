struct CPUDebugger : ChipDebugger {
  bool property(unsigned id, string &name, string &value);

  //internal
  virtual unsigned mdr() { return 0; }

  //$2181-2183
  virtual unsigned wram_address() { return 0; }

  //$4016
  virtual bool joypad_strobe_latch() { return 0; }

  //$4200
  virtual bool nmi_enable() { return 0; }
  virtual bool hirq_enable() { return 0; }
  virtual bool virq_enable() { return 0; }
  virtual bool auto_joypad_poll() { return 0; }

  //$4201
  virtual unsigned pio_bits() { return 0; }

  //$4202
  virtual unsigned multiplicand() { return 0; }

  //$4203
  virtual unsigned multiplier() { return 0; }

  //$4204-$4205
  virtual unsigned dividend() { return 0; }

  //$4206
  virtual unsigned divisor() { return 0; }

  //$4207-$4208
  virtual unsigned htime() { return 0; }

  //$4209-$420a
  virtual unsigned vtime() { return 0; }

  //$420d
  virtual bool fastrom_enable() { return 0; }
};
