struct Wave {
  bool enable;

  bool dac_enable;
  unsigned volume_shift;
  uint11 frequency;
  bool counter;
  uint8 pattern[32];

  int16 output;
  uint8 length;
  unsigned period;
  uint5 pattern_offset;
  uint4 pattern_sample;

  void run();
  void clock_length();
  void write(unsigned r, uint8 data);
  void write_pattern(unsigned p, uint8 data);
  void power();
  void serialize(serializer&);
};
