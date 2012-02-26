struct Interface {
  virtual void lcdScanline();
  virtual void joypWrite(bool p15, bool p14);

  virtual void videoRefresh(const uint16_t *data);
  virtual void audioSample(int16_t center, int16_t left, int16_t right);
  virtual bool inputPoll(unsigned id);

  virtual void message(const string &text);
};

extern Interface *interface;
