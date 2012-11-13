struct Pulse {
  unsigned length_counter;

  Envelope envelope;
  Sweep sweep;

  uint2 duty;
  uint3 duty_counter;

  uint11 period;
  unsigned period_counter;

  void clock_length();
  bool check_period();
  uint8 clock();

  void power();
  void reset();
  void serialize(serializer&);
} pulse[2];
