class Audio {
public:
  void coprocessor_enable(bool state);
  void coprocessor_frequency(double frequency);
  void sample(int16 left, int16 right);
  void coprocessor_sample(int16 left, int16 right);
  void init();

private:
  bool coprocessor;
  uint32 dsp_buffer[32768], cop_buffer[32768];
  unsigned dsp_rdoffset, cop_rdoffset;
  unsigned dsp_wroffset, cop_wroffset;
  unsigned dsp_length, cop_length;

  double r_step, r_frac;
  int r_sum_l, r_sum_r;

  void flush();
};

extern Audio audio;
