#ifdef NALL_DSP_INTERNAL_HPP

struct ResampleNearest : Resampler {
  inline void setFrequency();
  inline void clear();
  inline void sample();
  ResampleNearest(DSP &dsp) : Resampler(dsp) {}

  real fraction;
  real step;
};

void ResampleNearest::setFrequency() {
  fraction = 0.0;
  step = dsp.settings.frequency / frequency;
}

void ResampleNearest::clear() {
  fraction = 0.0;
}

void ResampleNearest::sample() {
  while(fraction <= 1.0) {
    real channel[dsp.settings.channels];

    for(unsigned n = 0; n < dsp.settings.channels; n++) {
      real a = dsp.buffer.read(n, -1);
      real b = dsp.buffer.read(n, -0);

      real mu = fraction;

      channel[n] = mu < 0.5 ? a : b;
    }

    dsp.write(channel);
    fraction += step;
  }

  dsp.buffer.rdoffset++;
  fraction -= 1.0;
}

#endif
