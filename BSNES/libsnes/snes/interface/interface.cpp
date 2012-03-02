#include <snes/snes.hpp>

namespace SNES {

Interface *interface = 0;

void Interface::videoRefresh(const uint32_t *data, bool hires, bool interlace, bool overscan) {
}

void Interface::audioSample(int16_t l_sample, int16_t r_sample) {
}

int16_t Interface::inputPoll(bool port, Input::Device::e device, unsigned index, unsigned id) {
  return 0;
}

void Interface::message(const string &text) {
  print(text, "\n");
}

time_t Interface::currentTime() {
  return time(0);
}

time_t Interface::randomSeed() {
  return time(0);
}

}
