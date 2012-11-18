struct Link : Coprocessor, library {
  string program;

  static void Enter();
  void enter();
  void init();
  void load();
  void unload();
  void power();
  void reset();

  uint8 read(unsigned addr);
  void write(unsigned addr, uint8 data);

private:
  function<void ()> link_power;
  function<void ()> link_reset;
  function<unsigned ()> link_run;
  function<uint8 (unsigned)> link_read;
  function<void (unsigned, uint8)> link_write;
};

extern Link link;
