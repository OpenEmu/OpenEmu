struct Sweep {
  uint8 shift;
  bool decrement;
  uint3 period;
  uint8 counter;
  bool enable;
  bool reload;
  uint11 pulse_period;

  bool check_period();
  void clock(unsigned channel);

  void power();
  void reset();
  void serialize(serializer&);
};
