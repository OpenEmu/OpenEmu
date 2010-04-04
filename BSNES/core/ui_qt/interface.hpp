class Interface : public SNES::Interface {
public:
  void video_refresh(uint16_t *data, unsigned pitch, unsigned *line, unsigned width, unsigned height);
  void audio_sample(uint16_t left, uint16_t right);
  void input_poll();
  int16_t input_poll(unsigned deviceid, unsigned id);

  Interface();
  void captureScreenshot(uint32_t*, unsigned, unsigned, unsigned);
  bool saveScreenshot;
} interface;
