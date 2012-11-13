struct SuperScope : Controller {
  void enter();
  uint2 data();
  void latch(bool data);
  SuperScope(bool port);

//private:
  bool latched;
  unsigned counter;

  signed x, y;

  bool trigger;
  bool cursor;
  bool turbo;
  bool pause;
  bool offscreen;

  bool turbolock;
  bool triggerlock;
  bool pauselock;
};
