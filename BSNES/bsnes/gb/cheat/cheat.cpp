#include <gb/gb.hpp>

namespace GameBoy {

Cheat cheat;

bool Cheat::decode(const string &code_, unsigned &addr, unsigned &data, unsigned &comp) {
  static bool initialize = false;
  static uint8 mapProActionReplay[256], mapGameGenie[256];

  if(initialize == false) {
    initialize = true;

    for(auto &n : mapProActionReplay) n = ~0;
    mapProActionReplay['0'] =  0; mapProActionReplay['1'] =  1; mapProActionReplay['2'] =  2; mapProActionReplay['3'] =  3;
    mapProActionReplay['4'] =  4; mapProActionReplay['5'] =  5; mapProActionReplay['6'] =  6; mapProActionReplay['7'] =  7;
    mapProActionReplay['8'] =  8; mapProActionReplay['9'] =  9; mapProActionReplay['A'] = 10; mapProActionReplay['B'] = 11;
    mapProActionReplay['C'] = 12; mapProActionReplay['D'] = 13; mapProActionReplay['E'] = 14; mapProActionReplay['F'] = 15;

    for(auto &n : mapGameGenie) n = ~0;
    mapGameGenie['0'] =  0; mapGameGenie['1'] =  1; mapGameGenie['2'] =  2; mapGameGenie['3'] =  3;
    mapGameGenie['4'] =  4; mapGameGenie['5'] =  5; mapGameGenie['6'] =  6; mapGameGenie['7'] =  7;
    mapGameGenie['8'] =  8; mapGameGenie['9'] =  9; mapGameGenie['A'] = 10; mapGameGenie['B'] = 11;
    mapGameGenie['C'] = 12; mapGameGenie['D'] = 13; mapGameGenie['E'] = 14; mapGameGenie['F'] = 15;
  }

  string code = code_;
  code.upper();
  unsigned length = code.length(), bits = 0;

  if(code.wildcard("????:??")) {
    code = { substr(code, 0, 4), substr(code, 5, 2) };
    for(unsigned n = 0; n < 6; n++) if(mapProActionReplay[code[n]] > 15) return false;
    bits = hex(code);
    addr = (bits >> 8) & 0xffff;
    data = (bits >> 0) & 0xff;
    comp = ~0;
    return true;
  }

  if(code.wildcard("????:??:??")) {
    code = { substr(code, 0, 4), substr(code, 5, 2), substr(code, 8, 2) };
    for(unsigned n = 0; n < 8; n++) if(mapProActionReplay[code[n]] > 15) return false;
    bits = hex(code);
    addr = (bits >> 16) & 0xffff;
    data = (bits >>  8) & 0xff;
    comp = (bits >>  0) & 0xff;
    return true;
  }

  if(code.wildcard("???" "-" "???")) {
    code = { substr(code, 0, 3), substr(code, 4, 3) };
    for(unsigned n = 0; n < 6; n++) if(mapGameGenie[code[n]] > 15) return false;
    for(unsigned n = 0; n < 6; n++) bits |= mapGameGenie[code[n]] << (20 - n * 4);

    addr = (bits >>  0) & 0xffff;
    data = (bits >> 16) & 0xff;
    comp = ~0;

    addr = (((addr >> 4) | (addr << 12)) & 0xffff) ^ 0xf000;

    return true;
  }

  if(code.wildcard("???" "-" "???" "-" "???")) {
    code = { substr(code, 0, 3), substr(code, 4, 3), substr(code, 8, 1), substr(code, 10, 1) };
    for(unsigned n = 0; n < 8; n++) if(mapGameGenie[code[n]] > 15) return false;
    for(unsigned n = 0; n < 8; n++) bits |= mapGameGenie[code[n]] << (28 - n * 4);

    addr = (bits >>  8) & 0xffff;
    data = (bits >> 24) & 0xff;
    comp = (bits >>  0) & 0xff;

    addr = (((addr >> 4) | (addr << 12)) & 0xffff) ^ 0xf000;
    comp = (((comp >> 2) | (comp << 6)) & 0xff) ^ 0xba;

    return true;
  }

  return false;
}

void Cheat::synchronize() {
  for(auto &n : override) n = false;

  for(unsigned n = 0; n < size(); n++) {
    override[operator[](n).addr] = true;
  }
}

}
