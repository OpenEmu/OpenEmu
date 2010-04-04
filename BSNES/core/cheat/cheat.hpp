class Cheat {
public:
  enum type_t {
    ProActionReplay,
    GameGenie,
  };

  struct cheat_t {
    bool enabled;
    string code;
    string desc;

    unsigned count;
    array<unsigned> addr;
    array<uint8_t> data;

    cheat_t& operator=(const cheat_t&);
    bool operator<(const cheat_t&);
  };

  bool decode(const char *s, cheat_t &item) const;
  bool read(unsigned addr, uint8_t &data) const;

  bool enabled() const;
  void enable();
  void disable();

  inline unsigned count() const;
  inline bool active() const;
  inline bool exists(unsigned addr) const;

  void add(bool enable, const char *code, const char *desc);
  void edit(unsigned i, bool enable, const char *code, const char *desc);
  bool remove(unsigned i);
  bool get(unsigned i, cheat_t &item) const;

  bool enabled(unsigned i) const;
  void enable(unsigned i);
  void disable(unsigned i);

  void load(string data);
  string save() const;
  void clear();

  Cheat();

private:
  bool cheat_enabled;  //cheat_enabled == (cheat_enabled_code_exists && cheat_system_enabled);
  bool cheat_enabled_code_exists;
  bool cheat_system_enabled;

  uint8_t mask[0x200000];
  vector<cheat_t> code;

  bool decode(const char *str, unsigned &addr, uint8_t &data, type_t &type) const;
  bool encode(string &str, unsigned addr, uint8_t data, type_t type) const;

  void update_cheat_status();
  unsigned mirror_address(unsigned addr) const;

  void update(const cheat_t& item);
  void set(unsigned addr);
  void clear(unsigned addr);

  string& encode_description(string &desc) const;
  string& decode_description(string &desc) const;
};

extern Cheat cheat;
