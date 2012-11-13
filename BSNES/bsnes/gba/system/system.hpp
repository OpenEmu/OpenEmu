enum class Input : unsigned {
  A, B, Select, Start, Right, Left, Up, Down, R, L,
};

struct BIOS : Memory {
  uint8 *data;
  unsigned size;
  uint32 mdr;

  uint32 read(uint32 addr, uint32 size);
  void write(uint32 addr, uint32 size, uint32 word);

  BIOS();
  ~BIOS();
};

struct System {
  void init();
  void term();
  void load();
  void power();
  void run();
  void runtosave();
  void runthreadtosave();

  unsigned serialize_size;

  serializer serialize();
  bool unserialize(serializer&);

  void serialize(serializer&);
  void serialize_all(serializer&);
  void serialize_init();
};

extern BIOS bios;
extern System system;
