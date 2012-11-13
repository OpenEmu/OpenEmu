#ifdef NALL_DSP_INTERNAL_HPP

struct ResampleLinear : Resampler {
  inline void setFrequency();
  inline void clear();
  inline void sample();
  ResampleLinear(DSP &dsp) : Resampler(dsp) {}

  real fraction;
  real step;
};

void ResampleLinear::setFrequency() {
  fraction = 0.0;
  step = dsp.settings.frequency / frequency;
}

void ResampleLinear::clear() {
  fraction = 0.0;
}

void ResampleLinear::sample() {
  while(fraction <= 1.0) {
    real channel[dsp.settings.channels];

    for(unsigned n = 0; n < dsp.settings.channels; n++) {
      real a = dsp.buffer.read(n, -1);
      real b = dsp.buffer.read(n, -0);

      real mu = fraction;

      channel[n] = a * (1.0 - mu) + b * mu;
    }

    dsp.write(channel);
    fraction += step;
  }

  dsp.buffer.rdoffset++;
  fraction -= 1.0;
}

#endif
