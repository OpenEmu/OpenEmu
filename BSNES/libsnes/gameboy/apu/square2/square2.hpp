struct Square2 {
  bool enable;

  uint2 duty;
  unsigned length;
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
  uint4 volume;

  bool dac_enable();

  void run();
  void clock_length();
  void clock_envelope();
  void write(unsigned r, uint8 data);
  void power();
  void serialize(serializer&);
};
