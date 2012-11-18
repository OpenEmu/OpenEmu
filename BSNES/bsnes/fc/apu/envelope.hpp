struct Envelope {
  uint4 speed;
  bool use_speed_as_volume;
  bool loop_mode;

  bool reload_decay;
  uint8 decay_counter;
  uint4 decay_volume;

  unsigned volume() const;
  void clock();

  void power();
  void reset();
  void serialize(serializer&);
};
