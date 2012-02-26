#include <gameboy/gameboy.hpp>

namespace GameBoy {

Interface *interface = 0;

void Interface::lcdScanline() {
}

void Interface::joypWrite(bool p15, bool p14) {
}

void Interface::videoRefresh(const uint16_t *data) {
}

void Interface::audioSample(int16_t center, int16_t left, int16_t right) {
}

bool Interface::inputPoll(unsigned id) {
  return false;
}

void Interface::message(const string &text) {
  print(text, "\n");
}

}
