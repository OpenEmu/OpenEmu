void APU::serialize(serializer &s) {
  Thread::serialize(s);

  s.integer(regs.bias.level);
  s.integer(regs.bias.amplitude);
  s.integer(regs.clock);

  s.integer(square1.sweep.shift);
  s.integer(square1.sweep.direction);
  s.integer(square1.sweep.frequency);
  s.integer(square1.sweep.enable);
  s.integer(square1.sweep.negate);
  s.integer(square1.sweep.period);

  s.integer(square1.envelope.frequency);
  s.integer(square1.envelope.direction);
  s.integer(square1.envelope.volume);
  s.integer(square1.envelope.period);

  s.integer(square1.enable);
  s.integer(square1.length);
  s.integer(square1.duty);
  s.integer(square1.frequency);
  s.integer(square1.counter);
  s.integer(square1.initialize);
  s.integer(square1.shadowfrequency);
  s.integer(square1.signal);
  s.integer(square1.output);
  s.integer(square1.period);
  s.integer(square1.phase);
  s.integer(square1.volume);

  s.integer(square2.envelope.frequency);
  s.integer(square2.envelope.direction);
  s.integer(square2.envelope.volume);
  s.integer(square2.envelope.period);

  s.integer(square2.enable);
  s.integer(square2.length);
  s.integer(square2.duty);
  s.integer(square2.frequency);
  s.integer(square2.counter);
  s.integer(square2.initialize);
  s.integer(square2.shadowfrequency);
  s.integer(square2.signal);
  s.integer(square2.output);
  s.integer(square2.period);
  s.integer(square2.phase);
  s.integer(square2.volume);

  s.integer(wave.mode);
  s.integer(wave.bank);
  s.integer(wave.dacenable);
  s.integer(wave.length);
  s.integer(wave.volume);
  s.integer(wave.frequency);
  s.integer(wave.counter);
  s.integer(wave.initialize);
  for(auto &value : wave.pattern) s.integer(value);
  s.integer(wave.enable);
  s.integer(wave.output);
  s.integer(wave.patternaddr);
  s.integer(wave.patternbank);
  s.integer(wave.patternsample);
  s.integer(wave.period);

  s.integer(noise.envelope.frequency);
  s.integer(noise.envelope.direction);
  s.integer(noise.envelope.volume);
  s.integer(noise.envelope.period);

  s.integer(noise.length);
  s.integer(noise.divisor);
  s.integer(noise.narrowlfsr);
  s.integer(noise.frequency);
  s.integer(noise.counter);
  s.integer(noise.initialize);
  s.integer(noise.enable);
  s.integer(noise.lfsr);
  s.integer(noise.output);
  s.integer(noise.period);
  s.integer(noise.volume);

  s.integer(sequencer.volume);
  s.integer(sequencer.lvolume);
  s.integer(sequencer.rvolume);
  for(auto &flag : sequencer.lenable) s.integer(flag);
  for(auto &flag : sequencer.renable) s.integer(flag);
  for(auto &flag : sequencer.enable) s.integer(flag);
  s.integer(sequencer.masterenable);
  s.integer(sequencer.base);
  s.integer(sequencer.step);
  s.integer(sequencer.lsample);
  s.integer(sequencer.rsample);

  for(auto &f : fifo) {
    for(auto &value : f.sample) s.integer(value);
    s.integer(f.output);
    s.integer(f.rdoffset);
    s.integer(f.wroffset);
    s.integer(f.size);
    s.integer(f.volume);
    s.integer(f.lenable);
    s.integer(f.renable);
    s.integer(f.timer);
  }
}
