struct Board {
  struct Memory {
    uint8_t *data;
    unsigned size;
    bool writable;

    inline uint8 read(unsigned addr) const;
    inline void write(unsigned addr, uint8 data);

    inline Memory(uint8_t *data, unsigned size) : data(data), size(size) {}
    inline Memory() : data(nullptr), size(0u), writable(false) {}
    inline ~Memory() { if(data) delete[] data; }
  };

  static unsigned mirror(unsigned addr, unsigned size);

  virtual void main();
  virtual void tick();

  virtual uint8 prg_read(unsigned addr) = 0;
  virtual void prg_write(unsigned addr, uint8 data) = 0;

  virtual uint8 chr_read(unsigned addr);
  virtual void chr_write(unsigned addr, uint8 data);

  virtual inline void scanline(unsigned y) {}

  virtual Memory& memory();

  virtual void power();
  virtual void reset();

  virtual void serialize(serializer&);
  Board(XML::Document &document, const stream &memory);
  virtual ~Board();

  static Board* load(const string &markup, const stream &memory);

  struct Information {
    string type;
    bool battery;
  } information;

  Memory prgrom;
  Memory prgram;
  Memory chrrom;
  Memory chrram;
};
