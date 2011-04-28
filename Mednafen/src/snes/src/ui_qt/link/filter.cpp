//==============
//ScanlineFilter
//==============

ScanlineFilter scanlineFilter;

void ScanlineFilter::size(unsigned &width, unsigned &height) {
  if(enabled && height <= 240) height *= 2;
}

void ScanlineFilter::render(
  const uint16_t *&input, unsigned &pitch,
  const unsigned *&line, unsigned width, unsigned &height
) {
  if(enabled && height <= 240) {
    pitch >>= 1;

    const uint16_t *sp = input;
    uint16_t *dp = buffer;
    unsigned *lp = linewidth;
    for(unsigned y = 0; y < height; y++) {
      for(unsigned x = 0; x < line[y]; x++) {
        uint16_t color = *sp++;
        *(dp +   0) = color;
        *(dp + 512) = adjust[color];
        dp++;
      }

      sp += pitch - line[y];
      dp += 1024 - line[y];

      *lp++ = line[y];
      *lp++ = line[y];
    }

    input = buffer;
    pitch = 1024;
    line = linewidth;
    height *= 2;
  }
}

void ScanlineFilter::setIntensity(unsigned intensity) {
  if(intensity >= 100) {
    enabled = false;
  } else {
    enabled = true;

    for(unsigned i = 0; i < 32768; i++) {
      unsigned r = (i >>  0) & 31;
      unsigned g = (i >>  5) & 31;
      unsigned b = (i >> 10) & 31;

      r = (double)r * (double)intensity / 100.0;
      g = (double)g * (double)intensity / 100.0;
      b = (double)b * (double)intensity / 100.0;

      adjust[i] = (r << 0) + (g << 5) + (b << 10);
    }
  }
}

ScanlineFilter::ScanlineFilter() {
  enabled = false;
  adjust = new uint16_t[32768];
  buffer = new uint16_t[512 * 480];
  setIntensity(50);
}

ScanlineFilter::~ScanlineFilter() {
  delete[] adjust;
  delete[] buffer;
}

//======
//Filter
//======

Filter filter;

const uint8_t Filter::gamma_ramp_table[32] = {
  0x00, 0x01, 0x03, 0x06, 0x0a, 0x0f, 0x15, 0x1c,
  0x24, 0x2d, 0x37, 0x42, 0x4e, 0x5b, 0x69, 0x78,
  0x88, 0x90, 0x98, 0xa0, 0xa8, 0xb0, 0xb8, 0xc0,
  0xc8, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0xff,
};

uint8_t Filter::contrast_adjust(uint8_t input) {
  signed result = input - contrast + (2 * contrast * input + 127) / 255;
  return max(0, min(255, result));
}

uint8_t Filter::brightness_adjust(uint8_t input) {
  signed result = input + brightness;
  return max(0, min(255, result));
}

uint8_t Filter::gamma_adjust(uint8_t input) {
  signed result = (signed)(pow(((double)input / 255.0), (double)gamma / 100.0) * 255.0 + 0.5);
  return max(0, min(255, result));
}

void Filter::colortable_update() {
  double kr = 0.2126, kb = 0.0722, kg = (1.0 - kr - kb);  //luminance weights

  for(unsigned i = 0; i < 32768; i++) {
    unsigned color  //bgr555->rgb888 conversion
    = ((i & 0x001f) << 19) | ((i & 0x001c) << 14)
    | ((i & 0x03e0) <<  6) | ((i & 0x0380) <<  1)
    | ((i & 0x7c00) >>  7) | ((i & 0x7000) >> 12);

    signed l;
    signed r = (color >> 16) & 0xff;
    signed g = (color >>  8) & 0xff;
    signed b = (color      ) & 0xff;

    if(gamma_ramp == true) {
      r = gamma_ramp_table[r >> 3];
      g = gamma_ramp_table[g >> 3];
      b = gamma_ramp_table[b >> 3];
    }

    if(contrast != 0) {
      r = contrast_adjust(r);
      g = contrast_adjust(g);
      b = contrast_adjust(b);
    }

    if(brightness != 0) {
      r = brightness_adjust(r);
      g = brightness_adjust(g);
      b = brightness_adjust(b);
    }

    if(gamma != 100) {
      r = gamma_adjust(r);
      g = gamma_adjust(g);
      b = gamma_adjust(b);
    }

    if(sepia == true) {
      l = (signed)((double)r * kr + (double)g * kg + (double)b * kb);
      l = max(0, min(255, l));

      r = (signed)((double)l * (1.0 + 0.300));
      g = (signed)((double)l * (1.0 - 0.055));
      b = (signed)((double)l * (1.0 - 0.225));

      r = max(0, min(255, r));
      g = max(0, min(255, g));
      b = max(0, min(255, b));
    }

    if(grayscale == true) {
      l = (signed)((double)r * kr + (double)g * kg + (double)b * kb);
      l = max(0, min(255, l));
      r = g = b = l;
    }

    if(invert == true) {
      r ^= 0xff;
      g ^= 0xff;
      b ^= 0xff;
    }

    colortable[i] = (r << 16) | (g << 8) | (b);
  }
}

void Filter::size(unsigned &outwidth, unsigned &outheight, unsigned width, unsigned height) {
  scanlineFilter.size(width, height);

  if(opened() && renderer > 0) {
    return dl_size(renderer, outwidth, outheight, width, height);
  }

  outwidth  = width;
  outheight = height;
}

void Filter::render(
  uint32_t *output, unsigned outpitch,
  const uint16_t *input, unsigned pitch,
  const unsigned *line, unsigned width, unsigned height
) {
  scanlineFilter.render(input, pitch, line, width, height);

  if(opened() && renderer > 0) {
    return dl_render(renderer, output, outpitch, input, pitch, line, width, height);
  }

  pitch >>= 1;
  outpitch >>= 2;

  for(unsigned y = 0; y < height; y++) {
    if(width == 512 && line[y] == 256) {
      for(unsigned x = 0; x < 256; x++) {
        uint16_t p = *input++;
        *output++ = colortable[p];
        *output++ = colortable[p];
      }
      input += 256;
    } else {
      for(unsigned x = 0; x < width; x++) {
        uint16_t p = *input++;
        *output++ = colortable[p];
      }
    }
    input  += pitch - width;
    output += outpitch - width;
  }
}

QWidget* Filter::settings() {
  if(opened() && renderer > 0) {
    return dl_settings(renderer);
  } else {
    return 0;
  }
}

Filter::Filter() {
  renderer = 0;

  contrast = 0;
  brightness = 0;
  gamma = 100;

  gamma_ramp = false;
  sepia = false;
  grayscale = false;
  invert = false;

  colortable = new uint32_t[32768];
  colortable_update();

  if(open("snesfilter")) {
    dl_supported = sym("snesfilter_supported");
    dl_colortable = sym("snesfilter_colortable");
    dl_configuration = sym("snesfilter_configuration");
    dl_size = sym("snesfilter_size");
    dl_render = sym("snesfilter_render");
    dl_settings = sym("snesfilter_settings");

    dl_colortable(colortable);
    dl_configuration(config());
  } else {
    config().video.windowed.swFilter   = 0;
    config().video.fullscreen.swFilter = 0;
  }
}

Filter::~Filter() {
  delete[] colortable;
}
