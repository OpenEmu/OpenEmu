struct System {
  void run();
  void runtosave();
  void runthreadtosave();

  void load();
  void power();
  void reset();

  void init();
  void term();

  serializer serialize();
  bool unserialize(serializer&);

  void serialize(serializer&);
  void serialize_all(serializer&);
  void serialize_init();
  unsigned serialize_size;
};

extern System system;
