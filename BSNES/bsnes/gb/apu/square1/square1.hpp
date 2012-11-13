struct Square1 {
  bool enable;

  uint3 sweep_frequency;
  bool sweep_direction;
  uint3 sweep_shift;
  bool sweep_negate;
  uint2 duty;
  uint6 length;
  uint4 envelope_volume;
  bool envelope_direction;
  uint3 envelope_frequency;
  uint11 frequency;
  bool counter;

  int16 output;
  bool duty_output;
  uint3 phase;
  unsigned period;
  uint3 envelope_period;
  uint3 sweep_period;
  signed frequency_shadow;
  bool sweep_enable;
  uint4 volume;

  bool dac_enable();

  void run();
  void sweep(bool update);
  void clock_length();
  void clock_sweep();
  void clock_envelope();
  void write(unsigned r, uint8 data);
  void power();
  void serialize(serializer&);
};
