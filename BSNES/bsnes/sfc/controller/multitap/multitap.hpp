struct Multitap : Controller {
  uint2 data();
  void latch(bool data);
  Multitap(bool port);

private:
  bool latched;
  unsigned counter1;
  unsigned counter2;
};
