struct CheatCode {
  unsigned addr;
  unsigned data;
};

struct Cheat : public linear_vector<CheatCode> {
  uint8 *override;

  bool enabled() const;
  void enable(bool);
  void synchronize();
  uint8 read(unsigned) const;
  void init();

  Cheat();
  ~Cheat();

  static bool decode(const string&, unsigned&, unsigned&);

private:
  bool system_enabled;
  bool code_enabled;
  bool cheat_enabled;
  unsigned mirror(unsigned) const;
};

extern Cheat cheat;
