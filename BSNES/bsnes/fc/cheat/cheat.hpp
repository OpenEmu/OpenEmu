struct CheatCode {
  unsigned addr;
  unsigned data;
  unsigned comp;
};

struct Cheat : public linear_vector<CheatCode> {
  static bool decode(const string &code, unsigned &addr, unsigned &data, unsigned &comp);

  void synchronize();
  bool override[65536];
};

extern Cheat cheat;
