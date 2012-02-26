struct Serial : Controller, public library {
  void enter();
  uint8 read();
  void write(uint8 data);
  uint2 data();
  void latch(bool data);
  Serial(bool port);
  ~Serial();

private:
  bool enable;
  function<unsigned ()> baudrate;
  function<bool ()> flowcontrol;
  function<void (void (*)(unsigned), uint8_t (*)(), void (*)(uint8_t))> main;

  bool latched;
  bool data1;
  bool data2;
};
