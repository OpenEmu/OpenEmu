#ifdef NALL_DSP_INTERNAL_HPP

struct ResampleAverage : Resampler {
  inline void setFrequency();
  inline void clear();
  inline void sample();
  inline void sampleLinear();
  ResampleAverage(DSP &dsp) : Resampler(dsp) {}

  real fraction;
  real step;
};

void ResampleAverage::setFrequency() {
  fraction = 0.0;
  step = dsp.settings.frequency / frequency;
}

void ResampleAverage::clear() {
  fraction = 0.0;
}

void ResampleAverage::sample() {
  //can only average if input frequency >= output frequency
  if(step < 1.0) return sampleLinear();

  fraction += 1.0;

  real scalar = 1.0;
  if(fraction > step) scalar = 1.0 - (fraction - step);

  for(unsigned c = 0; c < dsp.settings.channels; c++) {
    dsp.output.write(c) += dsp.buffer.read(c) * scalar;
  }

  if(fraction >= step) {
    for(unsigned c = 0; c < dsp.settings.channels; c++) {
      dsp.output.write(c) /= step;
    }
    dsp.output.wroffset++;

    fraction -= step;
    for(unsigned c = 0; c < dsp.settings.channels; c++) {
      dsp.output.write(c) = dsp.buffer.read(c) * fraction;
    }
  }

  dsp.buffer.rdoffset++;
}

void ResampleAverage::sampleLinear() {
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
