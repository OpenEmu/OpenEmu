class State {
public:
  bool save(unsigned);
  bool load(unsigned);

  void frame();
  void resetHistory();
  bool rewind();

  State();
  ~State();

private:
  serializer *history;
  unsigned historySize;
  unsigned historyIndex;
  unsigned historyCount;
  unsigned frameCounter;

  bool allowed() const;
  string name(unsigned slot) const;
};

extern State state;
