class Audio {
public:
  void coprocessor_enable(bool state);
  void coprocessor_frequency(double frequency);
  void sample(uint16 left, uint16 right);
  void coprocessor_sample(uint16 left, uint16 right);
  void init();

private:
  bool coprocessor;
  uint32 dsp_buffer[32768], cop_buffer[32768];
  unsigned dsp_rdoffset, cop_rdoffset;
  unsigned dsp_wroffset, cop_wroffset;
  unsigned dsp_length, cop_length;

  double hermite(double mu, double a, double b, double c, double d);
  double r_step, r_frac;
  int r_left[4], r_right[4];

  void flush();
};

extern Audio audio;
