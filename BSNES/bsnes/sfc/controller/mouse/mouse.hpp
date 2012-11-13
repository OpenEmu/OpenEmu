struct Mouse : Controller {
  uint2 data();
  void latch(bool data);
  Mouse(bool port);

private:
  bool latched;
  unsigned counter;
};
