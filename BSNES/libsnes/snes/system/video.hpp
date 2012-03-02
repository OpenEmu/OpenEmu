struct Video {
  struct Format { enum e { RGB30, RGB24, RGB16, RGB15 } i; };
  unsigned *palette;

  unsigned palette30(unsigned color);
  void generate(Format::e format);
  Video();
  ~Video();

private:
  bool hires;
  unsigned line_width[240];

  void update();
  void scanline();
  void init();

  static const uint8_t cursor[15 * 15];
  void draw_cursor(uint16_t color, int x, int y);

  friend class System;
};

extern Video video;
