#include <fc/fc.hpp>

namespace Famicom {

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
    mapGameGenie['A'] =  0; mapGameGenie['P'] =  1; mapGameGenie['Z'] =  2; mapGameGenie['L'] =  3;
    mapGameGenie['G'] =  4; mapGameGenie['I'] =  5; mapGameGenie['T'] =  6; mapGameGenie['Y'] =  7;
    mapGameGenie['E'] =  8; mapGameGenie['O'] =  9; mapGameGenie['X'] = 10; mapGameGenie['U'] = 11;
    mapGameGenie['K'] = 12; mapGameGenie['S'] = 13; mapGameGenie['V'] = 14; mapGameGenie['N'] = 15;
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

  if(length == 6) {
    for(unsigned n = 0; n < 6; n++) if(mapGameGenie[code[n]] > 15) return false;
    for(unsigned n = 0; n < 6; n++) bits |= mapGameGenie[code[n]] << (20 - n * 4);
    unsigned addrTable[] = { 10, 9, 8, 7, 2, 1, 0, 19, 14, 13, 12, 11, 6, 5, 4 };
    unsigned dataTable[] = { 23, 18, 17, 16, 3, 22, 21, 20 };

    addr = 0x8000, data = 0x00, comp = ~0;
    for(unsigned n = 0; n < 15; n++) addr |= bits & (1 << addrTable[n]) ? 0x4000 >> n : 0;
    for(unsigned n = 0; n <  8; n++) data |= bits & (1 << dataTable[n]) ?   0x80 >> n : 0;
    return true;
  }

  if(length == 8) {
    for(unsigned n = 0; n < 8; n++) if(mapGameGenie[code[n]] > 15) return false;
    for(unsigned n = 0; n < 8; n++) bits |= mapGameGenie[code[n]] << (28 - n * 4);
    unsigned addrTable[] = { 18, 17, 16, 15, 10, 9, 8, 27, 22, 21, 20, 19, 14, 13, 12 };
    unsigned dataTable[] = { 31, 26, 25, 24, 3, 30, 29, 28 };
    unsigned compTable[] = { 7, 2, 1, 0, 11, 6, 5,4 };

    addr = 0x8000, data = 0x00, comp = 0x00;
    for(unsigned n = 0; n < 15; n++) addr |= bits & (1 << addrTable[n]) ? 0x4000 >> n : 0;
    for(unsigned n = 0; n <  8; n++) data |= bits & (1 << dataTable[n]) ?   0x80 >> n : 0;
    for(unsigned n = 0; n <  8; n++) comp |= bits & (1 << compTable[n]) ?   0x80 >> n : 0;
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
