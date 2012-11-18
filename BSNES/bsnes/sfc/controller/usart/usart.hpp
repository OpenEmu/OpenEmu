struct USART : Controller, public library {
  void enter();

  bool quit();
  void usleep(unsigned milliseconds);
  bool readable();
  uint8 read();
  bool writable();
  void write(uint8 data);

  uint2 data();
  void latch(bool data);

  USART(bool port);
  ~USART();

private:
  bool latched;
  bool data1;
  bool data2;
  unsigned counter;

  uint8 rxlength;
  uint8 rxdata;
  vector<uint8> rxbuffer;

  uint8 txlength;
  uint8 txdata;
  vector<uint8> txbuffer;

  function<void (
    function<bool ()>,          //quit
    function<void (unsigned)>,  //usleep
    function<bool ()>,          //readable
    function<uint8 ()>,         //read
    function<bool ()>,          //writable
    function<void (uint8)>      //write
  )> init;
  function<void ()> main;
};
