struct LayerWindow {
  bool one_enable;
  bool one_invert;
  bool two_enable;
  bool two_invert;

  unsigned mask;

  bool main_enable;
  bool sub_enable;

  uint8 main[256];
  uint8 sub[256];

  void render(bool screen);
  void serialize(serializer&);
};

struct ColorWindow {
  bool one_enable;
  bool one_invert;
  bool two_enable;
  bool two_invert;

  unsigned mask;

  unsigned main_mask;
  unsigned sub_mask;

  uint8 main[256];
  uint8 sub[256];

  void render(bool screen);
  void serialize(serializer&);
};
