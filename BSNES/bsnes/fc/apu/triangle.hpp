struct Triangle {
  unsigned length_counter;

  uint8 linear_length;
  bool halt_length_counter;

  uint11 period;
  unsigned period_counter;

  uint5 step_counter;
  uint8 linear_length_counter;
  bool reload_linear;

  void clock_length();
  void clock_linear_length();
  uint8 clock();

  void power();
  void reset();
  void serialize(serializer&);
} triangle;
