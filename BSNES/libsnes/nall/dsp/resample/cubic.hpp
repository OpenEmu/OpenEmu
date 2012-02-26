#ifdef NALL_DSP_INTERNAL_HPP

struct ResampleCubic : Resampler {
  inline void setFrequency();
  inline void clear();
  inline void sample();
  ResampleCubic(DSP &dsp) : Resampler(dsp) {}

  real fraction;
  real step;
};

void ResampleCubic::setFrequency() {
  fraction = 0.0;
  step = dsp.settings.frequency / frequency;
}

void ResampleCubic::clear() {
  fraction = 0.0;
}

void ResampleCubic::sample() {
  while(fraction <= 1.0) {
    real channel[dsp.settings.channels];

    for(unsigned n = 0; n < dsp.settings.channels; n++) {
      real a = dsp.buffer.read(n, -3);
      real b = dsp.buffer.read(n, -2);
      real c = dsp.buffer.read(n, -1);
      real d = dsp.buffer.read(n, -0);

      real mu = fraction;

      real A = d - c - a + b;
      real B = a - b - A;
      real C = c - a;
      real D = b;

      channel[n] = A * (mu * 3) + B * (mu * 2) + C * mu + D;
    }

    dsp.write(channel);
    fraction += step;
  }

  dsp.buffer.rdoffset++;
  fraction -= 1.0;
}

#endif
