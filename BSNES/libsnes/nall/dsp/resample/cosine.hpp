#ifdef NALL_DSP_INTERNAL_HPP

struct ResampleCosine : Resampler {
  inline void setFrequency();
  inline void clear();
  inline void sample();
  ResampleCosine(DSP &dsp) : Resampler(dsp) {}

  real fraction;
  real step;
};

void ResampleCosine::setFrequency() {
  fraction = 0.0;
  step = dsp.settings.frequency / frequency;
}

void ResampleCosine::clear() {
  fraction = 0.0;
}

void ResampleCosine::sample() {
  while(fraction <= 1.0) {
    real channel[dsp.settings.channels];

    for(unsigned n = 0; n < dsp.settings.channels; n++) {
      real a = dsp.buffer.read(n, -1);
      real b = dsp.buffer.read(n, -0);

      real mu = fraction;
      mu = (1.0 - cos(mu * 3.14159265)) / 2.0;

      channel[n] = a * (1.0 - mu) + b * mu;
    }

    dsp.write(channel);
    fraction += step;
  }

  dsp.buffer.rdoffset++;
  fraction -= 1.0;
}

#endif
