struct Master {
  bool left_in_enable;
  uint3 left_volume;
  bool right_in_enable;
  uint3 right_volume;
  bool channel4_left_enable;
  bool channel3_left_enable;
  bool channel2_left_enable;
  bool channel1_left_enable;
  bool channel4_right_enable;
  bool channel3_right_enable;
  bool channel2_right_enable;
  bool channel1_right_enable;
  bool enable;

  int16 center;
  int16 left;
  int16 right;

  void run();
  void write(unsigned r, uint8 data);
  void power();
  void serialize(serializer&);
};
