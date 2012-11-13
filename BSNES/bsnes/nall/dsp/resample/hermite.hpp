#ifdef NALL_DSP_INTERNAL_HPP

struct ResampleHermite : Resampler {
  inline void setFrequency();
  inline void clear();
  inline void sample();
  ResampleHermite(DSP &dsp) : Resampler(dsp) {}

  real fraction;
  real step;
};

void ResampleHermite::setFrequency() {
  fraction = 0.0;
  step = dsp.settings.frequency / frequency;
}

void ResampleHermite::clear() {
  fraction = 0.0;
}

void ResampleHermite::sample() {
  while(fraction <= 1.0) {
    real channel[dsp.settings.channels];

    for(unsigned n = 0; n < dsp.settings.channels; n++) {
      real a = dsp.buffer.read(n, -3);
      real b = dsp.buffer.read(n, -2);
      real c = dsp.buffer.read(n, -1);
      real d = dsp.buffer.read(n, -0);

      const real tension = 0.0;  //-1 = low, 0 = normal, +1 = high
      const real bias = 0.0;  //-1 = left, 0 = even, +1 = right

      real mu1, mu2, mu3, m0, m1, a0, a1, a2, a3;

      mu1 = fraction;
      mu2 = mu1 * mu1;
      mu3 = mu2 * mu1;

      m0  = (b - a) * (1.0 + bias) * (1.0 - tension) / 2.0;
      m0 += (c - b) * (1.0 - bias) * (1.0 - tension) / 2.0;
      m1  = (c - b) * (1.0 + bias) * (1.0 - tension) / 2.0;
      m1 += (d - c) * (1.0 - bias) * (1.0 - tension) / 2.0;

      a0 = +2 * mu3 - 3 * mu2 + 1;
      a1 =      mu3 - 2 * mu2 + mu1;
      a2 =      mu3 -     mu2;
      a3 = -2 * mu3 + 3 * mu2;

      channel[n] = (a0 * b) + (a1 * m0) + (a2 * m1) + (a3 * c);
    }

    dsp.write(channel);
    fraction += step;
  }

  dsp.buffer.rdoffset++;
  fraction -= 1.0;
}

#endif
