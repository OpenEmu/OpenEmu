#ifdef SYSTEM_CPP

Audio audio;

void Audio::coprocessor_enable(bool state) {
  coprocessor = state;

  dsp_rdoffset = cop_rdoffset = 0;
  dsp_wroffset = cop_wroffset = 0;
  dsp_length = cop_length = 0;

  for(unsigned i = 0; i < 4; i++) r_left[i] = r_right[i] = 0;
}

void Audio::coprocessor_frequency(double input_frequency) {
  double output_frequency;
  if(system.region() == System::NTSC) {
    output_frequency = config.smp.ntsc_clock_rate / 768.0;
  } else /* (system.region() == System::PAL) */ {
    output_frequency = config.smp.pal_clock_rate  / 768.0;
  }

  r_step = input_frequency / output_frequency;
  r_frac = 0;
}

void Audio::sample(uint16 left, uint16 right) {
  if(coprocessor == false) {
    system.interface->audio_sample(left, right);
  } else {
    dsp_buffer[dsp_wroffset] = (left << 0) + (right << 16);
    dsp_wroffset = (dsp_wroffset + 1) & 32767;
    dsp_length = (dsp_length + 1) & 32767;
    flush();
  }
}

void Audio::coprocessor_sample(uint16 left, uint16 right) {
  r_left [0] = r_left [1];
  r_left [1] = r_left [2];
  r_left [2] = r_left [3];
  r_left [3] = (int16)left;

  r_right[0] = r_right[1];
  r_right[1] = r_right[2];
  r_right[2] = r_right[3];
  r_right[3] = (int16)right;

  while(r_frac <= 1.0) {
    left  = sclamp<16>(hermite(r_frac, r_left [0], r_left [1], r_left [2], r_left [3]));
    right = sclamp<16>(hermite(r_frac, r_right[0], r_right[1], r_right[2], r_right[3]));
    r_frac += r_step;

    cop_buffer[cop_wroffset] = (left << 0) + (right << 16);
    cop_wroffset = (cop_wroffset + 1) & 32767;
    cop_length = (cop_length + 1) & 32767;
    flush();
  }

  r_frac -= 1.0;
}

void Audio::init() {
}

//========
//private:
//========

//4-tap hermite interpolation
double Audio::hermite(double mu1, double a, double b, double c, double d) {
  const double tension = 0.0; //-1 = low, 0 = normal, 1 = high
  const double bias    = 0.0; //-1 = left, 0 = even, 1 = right

  double mu2, mu3, m0, m1, a0, a1, a2, a3;

  mu2 = mu1 * mu1;
  mu3 = mu2 * mu1;

  m0  = (b - a) * (1 + bias) * (1 - tension) / 2;
  m0 += (c - b) * (1 - bias) * (1 - tension) / 2;
  m1  = (c - b) * (1 + bias) * (1 - tension) / 2;
  m1 += (d - c) * (1 - bias) * (1 - tension) / 2;

  a0 = +2 * mu3 - 3 * mu2 + 1;
  a1 =      mu3 - 2 * mu2 + mu1;
  a2 =      mu3 -     mu2;
  a3 = -2 * mu3 + 3 * mu2;

  return (a0 * b) + (a1 * m0) + (a2 * m1) + (a3 * c);
}

void Audio::flush() {
  while(dsp_length > 0 && cop_length > 0) {
    uint32 dsp_sample = dsp_buffer[dsp_rdoffset];
    uint32 cop_sample = cop_buffer[cop_rdoffset];

    dsp_rdoffset = (dsp_rdoffset + 1) & 32767;
    cop_rdoffset = (cop_rdoffset + 1) & 32767;

    dsp_length--;
    cop_length--;

    int dsp_left  = (int16)(dsp_sample >>  0);
    int dsp_right = (int16)(dsp_sample >> 16);

    int cop_left  = (int16)(cop_sample >>  0);
    int cop_right = (int16)(cop_sample >> 16);

    system.interface->audio_sample(
      sclamp<16>((dsp_left  + cop_left ) / 2),
      sclamp<16>((dsp_right + cop_right) / 2)
    );
  }
}

#endif
