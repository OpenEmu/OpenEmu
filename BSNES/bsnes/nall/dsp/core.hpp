#ifdef NALL_DSP_INTERNAL_HPP

#include <math.h>
#include <nall/stdint.hpp>

namespace nall {

//precision: can be float, double or long double
#define real float

struct DSP;

struct Resampler {
  DSP &dsp;
  real frequency;

  virtual void setFrequency() = 0;
  virtual void clear() = 0;
  virtual void sample() = 0;
  Resampler(DSP &dsp) : dsp(dsp) {}
};

struct DSP {
  enum class ResampleEngine : unsigned {
    Nearest,
    Linear,
    Cosine,
    Cubic,
    Hermite,
    Average,
    Sinc,
  };

  inline void setChannels(unsigned channels);
  inline void setPrecision(unsigned precision);
  inline void setFrequency(real frequency);  //inputFrequency
  inline void setVolume(real volume);
  inline void setBalance(real balance);

  inline void setResampler(ResampleEngine resamplingEngine);
  inline void setResamplerFrequency(real frequency);  //outputFrequency

  inline void sample(signed channel[]);
  inline bool pending();
  inline void read(signed channel[]);

  inline void clear();
  inline DSP();
  inline ~DSP();

protected:
  friend class ResampleNearest;
  friend class ResampleLinear;
  friend class ResampleCosine;
  friend class ResampleCubic;
  friend class ResampleAverage;
  friend class ResampleHermite;
  friend class ResampleSinc;

  struct Settings {
    unsigned channels;
    unsigned precision;
    real frequency;
    real volume;
    real balance;

    //internal
    real intensity;
    real intensityInverse;
  } settings;

  Resampler *resampler;
  inline void write(real channel[]);

  #include "buffer.hpp"
  Buffer buffer;
  Buffer output;

  inline void adjustVolume();
  inline void adjustBalance();
  inline signed clamp(const unsigned bits, const signed x);
};

#include "resample/nearest.hpp"
#include "resample/linear.hpp"
#include "resample/cosine.hpp"
#include "resample/cubic.hpp"
#include "resample/hermite.hpp"
#include "resample/average.hpp"
#include "resample/sinc.hpp"
#include "settings.hpp"

void DSP::sample(signed channel[]) {
  for(unsigned c = 0; c < settings.channels; c++) {
    buffer.write(c) = (real)channel[c] * settings.intensityInverse;
  }
  buffer.wroffset++;
  resampler->sample();
}

bool DSP::pending() {
  return output.rdoffset != output.wroffset;
}

void DSP::read(signed channel[]) {
  adjustVolume();
  adjustBalance();

  for(unsigned c = 0; c < settings.channels; c++) {
    channel[c] = clamp(settings.precision, output.read(c) * settings.intensity);
  }
  output.rdoffset++;
}

void DSP::write(real channel[]) {
  for(unsigned c = 0; c < settings.channels; c++) {
    output.write(c) = channel[c];
  }
  output.wroffset++;
}

void DSP::adjustVolume() {
  for(unsigned c = 0; c < settings.channels; c++) {
    output.read(c) *= settings.volume;
  }
}

void DSP::adjustBalance() {
  if(settings.channels != 2) return;  //TODO: support > 2 channels
  if(settings.balance < 0.0) output.read(1) *= 1.0 + settings.balance;
  if(settings.balance > 0.0) output.read(0) *= 1.0 - settings.balance;
}

signed DSP::clamp(const unsigned bits, const signed x) {
  const signed b = 1U << (bits - 1);
  const signed m = (1U << (bits - 1)) - 1;
  return (x > m) ? m : (x < -b) ? -b : x;
}

void DSP::clear() {
  buffer.clear();
  output.clear();
  resampler->clear();
}

DSP::DSP() {
  setResampler(ResampleEngine::Hermite);
  setResamplerFrequency(44100.0);

  setChannels(2);
  setPrecision(16);
  setFrequency(44100.0);
  setVolume(1.0);
  setBalance(0.0);

  clear();
}

DSP::~DSP() {
  if(resampler) delete resampler;
}

#undef real

}

#endif
