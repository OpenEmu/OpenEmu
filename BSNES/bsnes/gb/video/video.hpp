struct Video {
  uint32_t *palette;
  void generate_palette();

  Video();
  ~Video();

private:
  static const double monochrome[4][3];
  uint32_t palette_dmg(unsigned color) const;
  uint32_t palette_sgb(unsigned color) const;
  uint32_t palette_cgb(unsigned color) const;
};

extern Video video;
