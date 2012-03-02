struct Interface {
  virtual void videoRefresh(const uint32_t *data, bool hires, bool interlace, bool overscan);
  virtual void audioSample(int16_t lsample, int16_t rsample);
  virtual int16_t inputPoll(bool port, Input::Device::e device, unsigned index, unsigned id);

  virtual string path(Cartridge::Slot::e slot, const string &hint) = 0;
  virtual void message(const string &text);
  virtual time_t currentTime();
  virtual time_t randomSeed();
};

extern Interface *interface;
