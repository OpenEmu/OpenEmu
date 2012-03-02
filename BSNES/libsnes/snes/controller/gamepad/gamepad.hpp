struct Gamepad : Controller {
  uint2 data();
  void latch(bool data);
  Gamepad(bool port);

  void serialize(serializer &s);
private:
  bool latched;
  unsigned counter;
};
