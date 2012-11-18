struct Video {
  unsigned *palette;
  void generate_palette();

  Video();
  ~Video();
};

extern Video video;
