struct Noise {
  bool enable;

  uint4 envelope_volume;
  bool envelope_direction;
  uint3 envelope_frequency;
  uint4 frequency;
  bool narrow_lfsr;
  unsigned divisor;
  bool counter;

  int16 output;
  unsigned length;
  uint3 envelope_period;
  uint4 volume;
  unsigned period;
  uint15 lfsr;

  bool dac_enable();

  void run();
  void clock_length();
  void clock_envelope();
  void write(unsigned r, uint8 data);
  void power();
  void serialize(serializer&);
};
