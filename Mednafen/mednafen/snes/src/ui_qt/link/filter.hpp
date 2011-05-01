class ScanlineFilter {
public:
  bool enabled;

  void size(unsigned&, unsigned&);
  void render(const uint16_t*&, unsigned&, const unsigned*&, unsigned, unsigned&);
  void setIntensity(unsigned);

  ScanlineFilter();
  ~ScanlineFilter();

private:
  uint16_t *adjust;
  uint16_t *buffer;
  unsigned linewidth[480];
};

class Filter : public library {
public:
  function<const char* ()> dl_supported;
  function<void (const uint32_t*)> dl_colortable;
  function<void (configuration&)> dl_configuration;
  function<void (unsigned, unsigned&, unsigned&, unsigned, unsigned)> dl_size;
  function<void (unsigned, uint32_t*, unsigned, const uint16_t*, unsigned, const unsigned*, unsigned, unsigned)> dl_render;
  function<QWidget* (unsigned)> dl_settings;

  unsigned renderer;
  uint32_t *colortable;

  signed contrast;
  signed brightness;
  signed gamma;

  bool gamma_ramp;
  bool sepia;
  bool grayscale;
  bool invert;

  void colortable_update();
  void size(unsigned&, unsigned&, unsigned, unsigned);
  void render(uint32_t*, unsigned, const uint16_t*, unsigned, const unsigned*, unsigned, unsigned);
  QWidget* settings();

  Filter();
  ~Filter();

private:
  static const uint8_t gamma_ramp_table[32];
  uint8_t contrast_adjust(uint8_t input);
  uint8_t brightness_adjust(uint8_t input);
  uint8_t gamma_adjust(uint8_t input);
};

extern ScanlineFilter scanlineFilter;
extern Filter filter;
