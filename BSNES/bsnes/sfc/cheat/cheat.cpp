#include <sfc/sfc.hpp>

#define CHEAT_CPP
namespace SuperFamicom {

Cheat cheat;

bool Cheat::enabled() const {
  return system_enabled;
}

void Cheat::enable(bool state) {
  system_enabled = state;
  cheat_enabled = system_enabled && code_enabled;
}

void Cheat::synchronize() {
  memset(override, 0x00, 16 * 1024 * 1024);
  code_enabled = size() > 0;

  for(unsigned i = 0; i < size(); i++) {
    const CheatCode &code = operator[](i);

    unsigned addr = mirror(code.addr);
    override[addr] = true;
    if((addr & 0xffe000) == 0x7e0000) {
      //mirror $7e:0000-1fff to $00-3f|80-bf:0000-1fff
      unsigned mirroraddr;
      for(unsigned x = 0; x <= 0x3f; x++) {
        mirroraddr = ((0x00 + x) << 16) + (addr & 0x1fff);
        override[mirroraddr] = true;

        mirroraddr = ((0x80 + x) << 16) + (addr & 0x1fff);
        override[mirroraddr] = true;
      }
    }
  }

  cheat_enabled = system_enabled && code_enabled;
}

uint8 Cheat::read(unsigned addr) const {
  addr = mirror(addr);

  for(unsigned i = 0; i < size(); i++) {
    const CheatCode &code = operator[](i);
    if(addr == mirror(code.addr)) {
      return code.data;
    }
  }

  return 0x00;
}

void Cheat::init() {
  memset(override, 0x00, 16 * 1024 * 1024);
}

Cheat::Cheat() {
  override = new uint8[16 * 1024 * 1024];
  system_enabled = true;
}

Cheat::~Cheat() {
  delete[] override;
}

bool Cheat::decode(const string &code, unsigned &addr, unsigned &data) {
  string t = code;
  t.lower();

  #define ischr(n) ((n >= '0' && n <= '9') || (n >= 'a' && n <= 'f'))

  if(t.wildcard("??????:??")) {
    //Direct
    t = { substr(t, 0, 6), substr(t, 7, 2) };
    for(unsigned n = 0; n < 8; n++) if(!ischr(t[n])) return false;  //validate input
    unsigned r = hex(t);

    addr = r >> 8;
    data = r & 0xff;
    return true;
  }

  if(t.wildcard("????" "-" "????")) {
    //Game Genie
    t = { substr(t, 0, 4), substr(t, 5, 4) };
    for(unsigned n = 0; n < 8; n++) if(!ischr(t[n])) return false;  //validate input
    t.transform("df4709156bc8a23e", "0123456789abcdef");
    unsigned r = hex(t);
    static unsigned bits[] = { 13, 12, 11, 10, 5, 4, 3, 2, 23, 22, 21, 20, 1, 0, 15, 14, 19, 18, 17, 16, 9, 8, 7, 6 };

    addr = 0;
    for(unsigned n = 0; n < 24; n++) addr |= r & (1 << bits[n]) ? 0x800000 >> n : 0;
    data = r >> 24;
    return true;
  } else {
    return false;
  }

  #undef ischr
}

unsigned Cheat::mirror(unsigned addr) const {
  //$00-3f|80-bf:0000-1fff -> $7e:0000-1fff
  if((addr & 0x40e000) == 0x000000) return (0x7e0000 + (addr & 0x1fff));  
  return addr;
}

}
