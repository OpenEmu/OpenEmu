class Audio {
public:
  static const char *Volume;
  static const char *Resample;
  static const char *ResampleRatio;

  static const char *Handle;
  static const char *Synchronize;
  static const char *Frequency;
  static const char *Latency;

  virtual bool cap(const nall::string& name) { return false; }
  virtual nall::any get(const nall::string& name) { return false; }
  virtual bool set(const nall::string& name, const nall::any& value) { return false; }

  virtual void sample(uint16_t left, uint16_t right) {}
  virtual void clear() {}
  virtual bool init() { return true; }
  virtual void term() {}

  Audio() {}
  virtual ~Audio() {}
};
