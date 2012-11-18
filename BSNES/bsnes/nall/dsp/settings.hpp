#ifdef NALL_DSP_INTERNAL_HPP

void DSP::setChannels(unsigned channels) {
  assert(channels > 0);
  buffer.setChannels(channels);
  output.setChannels(channels);
  settings.channels = channels;
}

void DSP::setPrecision(unsigned precision) {
  settings.precision = precision;
  settings.intensity = 1 << (settings.precision - 1);
  settings.intensityInverse = 1.0 / settings.intensity;
}

void DSP::setFrequency(real frequency) {
  settings.frequency = frequency;
  resampler->setFrequency();
}

void DSP::setVolume(real volume) {
  settings.volume = volume;
}

void DSP::setBalance(real balance) {
  settings.balance = balance;
}

void DSP::setResampler(ResampleEngine engine) {
  if(resampler) delete resampler;

  switch(engine) {
  case ResampleEngine::Nearest: resampler = new ResampleNearest(*this); return;
  case ResampleEngine::Linear:  resampler = new ResampleLinear (*this); return;
  case ResampleEngine::Cosine:  resampler = new ResampleCosine (*this); return;
  case ResampleEngine::Cubic:   resampler = new ResampleCubic  (*this); return;
  case ResampleEngine::Hermite: resampler = new ResampleHermite(*this); return;
  case ResampleEngine::Average: resampler = new ResampleAverage(*this); return;
  case ResampleEngine::Sinc:    resampler = new ResampleSinc   (*this); return;
  }

  throw;
}

void DSP::setResamplerFrequency(real frequency) {
  resampler->frequency = frequency;
  resampler->setFrequency();
}

#endif
