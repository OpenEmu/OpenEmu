struct Random {
  void seed(unsigned seed);
  unsigned operator()(unsigned result = 0);
  void serialize(serializer&);
  Random();

private:
  unsigned iter;
};

extern Random random;
