class Video {
public:
  enum Mode {
    ModeNTSC,
    ModePAL,
  };
  void set_mode(Mode);

private:
  Mode mode;
  bool frame_hires;
  bool frame_interlace;

  unsigned pline_width[240];  //progressive
  unsigned iline_width[480];  //interlace

  void update();
  void scanline();
  void init();

  static const uint8_t cursor[15 * 15];
  void draw_cursor(uint16_t color, int x, int y);

  friend class System;
};

extern Video video;
