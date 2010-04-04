class Interface {
public:
  virtual void video_refresh(uint16_t *data, unsigned pitch, unsigned *line, unsigned width, unsigned height) {}
  virtual void audio_sample(uint16_t l_sample, uint16_t r_sample) {}
  virtual void input_poll() {}
  virtual int16_t input_poll(unsigned deviceid, unsigned id) { return 0; }
};
