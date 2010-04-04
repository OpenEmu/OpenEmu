const char *Audio::Volume = "Volume";
const char *Audio::Resample = "Resample";
const char *Audio::ResampleRatio = "ResampleRatio";

const char *Audio::Handle = "Handle";
const char *Audio::Synchronize = "Synchronize";
const char *Audio::Frequency = "Frequency";
const char *Audio::Latency = "Latency";

bool AudioInterface::init() {
  if(!p) driver();
  return p->init();
}

void AudioInterface::term() {
  if(p) {
    delete p;
    p = 0;
  }
}

bool AudioInterface::cap(const string& name) {
  if(name == Audio::Volume) return true;
  if(name == Audio::Resample) return true;
  if(name == Audio::ResampleRatio) return true;

  return p ? p->cap(name) : false;
}

any AudioInterface::get(const string& name) {
  if(name == Audio::Volume) return volume;
  if(name == Audio::Resample) return resample_enabled;
  if(name == Audio::ResampleRatio);

  return p ? p->get(name) : false;
}

bool AudioInterface::set(const string& name, const any& value) {
  if(name == Audio::Volume) {
    volume = any_cast<unsigned>(value);
    return true;
  }

  if(name == Audio::Resample) {
    resample_enabled = any_cast<bool>(value);
    return true;
  }

  if(name == Audio::ResampleRatio) {
    r_step = any_cast<double>(value);
    r_frac = 0;
    return true;
  }

  return p ? p->set(name, value) : false;
}

//4-tap hermite interpolation
double AudioInterface::hermite(double mu1, double a, double b, double c, double d) {
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

void AudioInterface::sample(uint16_t left, uint16_t right) {
  int s_left  = (int16_t)left;
  int s_right = (int16_t)right;

  if(volume != 100) {
    s_left  = sclamp<16>((double)s_left  * (double)volume / 100.0);
    s_right = sclamp<16>((double)s_right * (double)volume / 100.0);
  }

  r_left [0] = r_left [1];
  r_left [1] = r_left [2];
  r_left [2] = r_left [3];
  r_left [3] = s_left;

  r_right[0] = r_right[1];
  r_right[1] = r_right[2];
  r_right[2] = r_right[3];
  r_right[3] = s_right;

  if(resample_enabled == false) {
    if(p) p->sample(left, right);
    return;
  }

  while(r_frac <= 1.0) {
    int output_left  = sclamp<16>(hermite(r_frac, r_left [0], r_left [1], r_left [2], r_left [3]));
    int output_right = sclamp<16>(hermite(r_frac, r_right[0], r_right[1], r_right[2], r_right[3]));
    r_frac += r_step;
    if(p) p->sample(output_left, output_right);
  }

  r_frac -= 1.0;
}

void AudioInterface::clear() {
  r_frac = 0;
  r_left [0] = r_left [1] = r_left [2] = r_left [3] = 0;
  r_right[0] = r_right[1] = r_right[2] = r_right[3] = 0;
  if(p) p->clear();
}

AudioInterface::AudioInterface() {
  p = 0;
  volume = 100;
  resample_enabled = false;
  r_step = r_frac = 0;
  r_left [0] = r_left [1] = r_left [2] = r_left [3] = 0;
  r_right[0] = r_right[1] = r_right[2] = r_right[3] = 0;
}

AudioInterface::~AudioInterface() {
  term();
}
