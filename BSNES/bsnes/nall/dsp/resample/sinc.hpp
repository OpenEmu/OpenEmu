#ifdef NALL_DSP_INTERNAL_HPP

#include "lib/sinc.hpp"

struct ResampleSinc : Resampler {
  inline void setFrequency();
  inline void clear();
  inline void sample();
  inline ResampleSinc(DSP &dsp);

private:
  inline void remakeSinc();
  SincResample *sinc_resampler[8];
};

void ResampleSinc::setFrequency() {
  remakeSinc();
}

void ResampleSinc::clear() {
  remakeSinc();
}

void ResampleSinc::sample() {
  for(unsigned c = 0; c < dsp.settings.channels; c++) {
    sinc_resampler[c]->write(dsp.buffer.read(c));
  }

  if(sinc_resampler[0]->output_avail()) {
    do {
      for(unsigned c = 0; c < dsp.settings.channels; c++) {
        dsp.output.write(c) = sinc_resampler[c]->read();
      }
      dsp.output.wroffset++;
    } while(sinc_resampler[0]->output_avail());
  }

  dsp.buffer.rdoffset++;
}

ResampleSinc::ResampleSinc(DSP &dsp) : Resampler(dsp) {
  for(unsigned n = 0; n < 8; n++) sinc_resampler[n] = 0;
}

void ResampleSinc::remakeSinc() {
  assert(dsp.settings.channels < 8);

  for(unsigned c = 0; c < dsp.settings.channels; c++) {
    if(sinc_resampler[c]) delete sinc_resampler[c];
    sinc_resampler[c] = new SincResample(dsp.settings.frequency, frequency, 0.85, SincResample::QUALITY_HIGH);
  }
}

#endif
