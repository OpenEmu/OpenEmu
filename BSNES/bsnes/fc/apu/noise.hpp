struct Noise {
  unsigned length_counter;

  Envelope envelope;

  uint4 period;
  unsigned period_counter;

  bool short_mode;
  uint15 lfsr;

  void clock_length();
  uint8 clock();

  void power();
  void reset();
  void serialize(serializer&);
} noise;
