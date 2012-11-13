struct Gamepad : Controller {
  uint2 data();
  void latch(bool data);
  Gamepad(bool port);

private:
  bool latched;
  unsigned counter;
};
