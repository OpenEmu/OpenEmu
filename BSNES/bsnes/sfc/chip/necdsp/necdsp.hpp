struct NECDSP : Processor::uPD96050, Coprocessor {
  static void Enter();
  void enter();

  uint8 sr_read(unsigned);
  void sr_write(unsigned, uint8 data);

  uint8 dr_read(unsigned);
  void dr_write(unsigned, uint8 data);

  uint8 dp_read(unsigned addr);
  void dp_write(unsigned addr, uint8 data);

  void init();
  void load();
  void unload();
  void power();
  void reset();

  void serialize(serializer&);
};

extern NECDSP necdsp;
