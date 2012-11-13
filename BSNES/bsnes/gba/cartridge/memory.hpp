struct Memory {
  uint8 *data;
  unsigned size;
  unsigned mask;
} rom, ram;

struct EEPROM {
  uint8 *data;
  unsigned size;
  unsigned mask;
  unsigned test;
  unsigned bits;

  enum class Mode : unsigned { Wait, Command, ReadAddress, ReadValidate, ReadData, WriteAddress, WriteData, WriteValidate } mode;
  unsigned offset;
  unsigned address;
  unsigned addressbits;

  bool read(unsigned addr);
  void write(unsigned addr, bool bit);

  bool read();
  void write(bool bit);
  void power();
  void serialize(serializer&);
} eeprom;

struct FlashROM {
  uint8 *data;
  unsigned size;
  uint16 id;

  bool unlockhi;
  bool unlocklo;
  bool idmode;
  bool erasemode;
  bool bankselect;
  bool writeselect;
  bool bank;

  uint8 read(uint16 addr);
  void write(uint16 addr, uint8 byte);
  void power();
  void serialize(serializer&);
} flashrom;
