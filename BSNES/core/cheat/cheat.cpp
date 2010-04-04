#include <../base.hpp>

#define CHEAT_CPP
namespace SNES {

Cheat cheat;

Cheat::cheat_t& Cheat::cheat_t::operator=(const Cheat::cheat_t& source) {
  enabled = source.enabled;
  code = source.code;
  desc = source.desc;
  count = source.count;

  addr.reset();
  data.reset();
  for(unsigned n = 0; n < count; n++) {
    addr[n] = source.addr[n];
    data[n] = source.data[n];
  }

  return *this;
}

//used to sort cheat code list by description
bool Cheat::cheat_t::operator<(const Cheat::cheat_t& source) {
  return strcmp(desc, source.desc) < 0;
}

//parse item ("0123-4567+89AB-CDEF"), return cheat_t item
//return true if code is valid, false otherwise
bool Cheat::decode(const char *s, Cheat::cheat_t &item) const {
  item.enabled = false;
  item.count = 0;

  string code = s;
  code.replace(" ", "");

  lstring list;
  list.split("+", code);

  for(unsigned n = 0; n < list.size(); n++) {
    unsigned addr;
    uint8_t data;
    type_t type;
    if(decode(list[n], addr, data, type) == false) {
      item.count = 0;
      return false;
    }

    item.addr[item.count] = addr;
    item.data[item.count] = data;
    item.count++;
  }

  return true;
}

//read() is used by MemBus::read() if Cheat::enabled(addr) returns true to look up cheat code.
//returns true if cheat code was found, false if it was not.
//when true, cheat code substitution value is stored in data.
bool Cheat::read(unsigned addr, uint8_t &data) const {
  addr = mirror_address(addr);
  for(unsigned i = 0; i < code.size(); i++) {
    if(enabled(i) == false) continue;

    for(unsigned n = 0; n < code[i].count; n++) {
      if(addr == mirror_address(code[i].addr[n])) {
        data = code[i].data[n];
        return true;
      }
    }
  }

  //code not found, or code is disabled
  return false;
}

//==============
//master control
//==============

//global cheat system enable/disable:
//if disabled, *all* cheat codes are disabled;
//otherwise only individually disabled codes are.

bool Cheat::enabled() const {
  return cheat_system_enabled;
}

void Cheat::enable() {
  cheat_system_enabled = true;
  cheat_enabled = (cheat_system_enabled && cheat_enabled_code_exists);
}

void Cheat::disable() {
  cheat_system_enabled = false;
  cheat_enabled = false;
}

//================================
//cheat list manipulation routines
//================================

void Cheat::add(bool enable, const char *code_, const char *desc_) {
  cheat_t item;
  decode(code_, item);

  unsigned i = code.size();
  code[i] = item;
  code[i].enabled = enable;
  code[i].desc = desc_;
  code[i].code = code_;
  encode_description(code[i].desc);
  update(code[i]);

  update_cheat_status();
}

void Cheat::edit(unsigned i, bool enable, const char *code_, const char *desc_) {
  cheat_t item;
  decode(code_, item);

  //disable current code and clear from code lookup table
  code[i].enabled = false;
  update(code[i]);

  code[i] = item;
  code[i].enabled = enable;
  code[i].desc = desc_;
  code[i].code = code_;
  encode_description(code[i].desc);
  update(code[i]);

  update_cheat_status();
}

bool Cheat::remove(unsigned i) {
  unsigned size = code.size();
  if(i >= size) return false;  //also verifies size cannot be < 1

  for(unsigned n = i; n < size - 1; n++) code[n] = code[n + 1];
  code.resize(size - 1);

  update_cheat_status();
  return true;
}

bool Cheat::get(unsigned i, cheat_t &item) const {
  if(i >= code.size()) return false;

  item = code[i];
  decode_description(item.desc);
  return true;
}

//==============================
//cheat status modifier routines
//==============================

bool Cheat::enabled(unsigned i) const {
  return (i < code.size() ? code[i].enabled : false);
}

void Cheat::enable(unsigned i) {
  if(i >= code.size()) return;

  code[i].enabled = true;
  update(code[i]);
  update_cheat_status();
}

void Cheat::disable(unsigned i) {
  if(i >= code.size()) return;

  code[i].enabled = false;
  update(code[i]);
  update_cheat_status();
}

//===============================
//cheat file load / save routines
//
//file format:
//"description", status, nnnn-nnnn[+nnnn-nnnn...]\r\n
//...
//===============================

void Cheat::load(string data) {
  data.replace("\r", "");
  data.qreplace(" ", "");

  lstring line;
  line.split("\n", data);
  for(unsigned i = 0; i < line.size(); i++) {
    lstring part;
    part.qsplit(",", line[i]);
    if(part.size() != 3) continue;
    trim(part[2], "\"");
    add(part[0] == "enabled", /* code = */ part[1], /* desc = */ part[2]);
  }
}

string Cheat::save() const {
  string data;
  for(unsigned i = 0; i < code.size(); i++) {
    data << (code[i].enabled ? "enabled," : "disabled,")
         << code[i].code << ","
         << "\"" << code[i].desc << "\"\r\n";
  }
  return data;
}

void Cheat::clear() {
  cheat_enabled_code_exists = false;
  memset(mask, 0, 0x200000);
  code.reset();
}

Cheat::Cheat() : cheat_system_enabled(true) {
  clear();
}

//==================
//internal functions
//==================

//string <> binary code translation routines
//decode() "7e123456" -> 0x7e123456
//encode() 0x7e123456 -> "7e123456"

bool Cheat::decode(const char *s, unsigned &addr, uint8_t &data, type_t &type) const {
  string t = s;
  strlower(t);

  #define ischr(n) ((n >= '0' && n <= '9') || (n >= 'a' && n <= 'f'))

  if(strlen(t) == 8 || (strlen(t) == 9 && t[6] == ':')) {
    //strip ':'
    if(strlen(t) == 9 && t[6] == ':') t = string() << substr(t, 0, 6) << substr(t, 7);
    //validate input
    for(unsigned i = 0; i < 8; i++) if(!ischr(t[i])) return false;

    type = ProActionReplay;
    unsigned r = strhex((const char*)t);
    addr = r >> 8;
    data = r & 0xff;
    return true;
  } else if(strlen(t) == 9 && t[4] == '-') {
    //strip '-'
    t = string() << substr(t, 0, 4) << substr(t, 5);
    //validate input
    for(unsigned i = 0; i < 8; i++) if(!ischr(t[i])) return false;

    type = GameGenie;
    strtr(t, "df4709156bc8a23e", "0123456789abcdef");
    unsigned r = strhex((const char*)t);
    //8421 8421 8421 8421 8421 8421
    //abcd efgh ijkl mnop qrst uvwx
    //ijkl qrst opab cduv wxef ghmn
    addr = (!!(r & 0x002000) << 23) | (!!(r & 0x001000) << 22)
         | (!!(r & 0x000800) << 21) | (!!(r & 0x000400) << 20)
         | (!!(r & 0x000020) << 19) | (!!(r & 0x000010) << 18)
         | (!!(r & 0x000008) << 17) | (!!(r & 0x000004) << 16)
         | (!!(r & 0x800000) << 15) | (!!(r & 0x400000) << 14)
         | (!!(r & 0x200000) << 13) | (!!(r & 0x100000) << 12)
         | (!!(r & 0x000002) << 11) | (!!(r & 0x000001) << 10)
         | (!!(r & 0x008000) <<  9) | (!!(r & 0x004000) <<  8)
         | (!!(r & 0x080000) <<  7) | (!!(r & 0x040000) <<  6)
         | (!!(r & 0x020000) <<  5) | (!!(r & 0x010000) <<  4)
         | (!!(r & 0x000200) <<  3) | (!!(r & 0x000100) <<  2)
         | (!!(r & 0x000080) <<  1) | (!!(r & 0x000040) <<  0);
    data = r >> 24;
    return true;
  } else {
    return false;
  }
}

bool Cheat::encode(string &s, unsigned addr, uint8_t data, type_t type) const {
  char t[16];

  if(type == ProActionReplay) {
    sprintf(t, "%.6x%.2x", addr, data);
    s = t;
    return true;
  } else if(type == GameGenie) {
    unsigned r = addr;
    addr = (!!(r & 0x008000) << 23) | (!!(r & 0x004000) << 22)
         | (!!(r & 0x002000) << 21) | (!!(r & 0x001000) << 20)
         | (!!(r & 0x000080) << 19) | (!!(r & 0x000040) << 18)
         | (!!(r & 0x000020) << 17) | (!!(r & 0x000010) << 16)
         | (!!(r & 0x000200) << 15) | (!!(r & 0x000100) << 14)
         | (!!(r & 0x800000) << 13) | (!!(r & 0x400000) << 12)
         | (!!(r & 0x200000) << 11) | (!!(r & 0x100000) << 10)
         | (!!(r & 0x000008) <<  9) | (!!(r & 0x000004) <<  8)
         | (!!(r & 0x000002) <<  7) | (!!(r & 0x000001) <<  6)
         | (!!(r & 0x080000) <<  5) | (!!(r & 0x040000) <<  4)
         | (!!(r & 0x020000) <<  3) | (!!(r & 0x010000) <<  2)
         | (!!(r & 0x000800) <<  1) | (!!(r & 0x000400) <<  0);
    sprintf(t, "%.2x%.2x-%.4x", data, addr >> 16, addr & 0xffff);
    strtr(t, "0123456789abcdef", "df4709156bc8a23e");
    s = t;
    return true;
  } else {
    return false;
  }
}

//speed up S-CPU memory reads by disabling cheat code lookup when either:
//a) cheat system is disabled by user, or b) no enabled cheat codes exist
void Cheat::update_cheat_status() {
  for(unsigned i = 0; i < code.size(); i++) {
    if(code[i].enabled) {
      cheat_enabled_code_exists = true;
      cheat_enabled = (cheat_system_enabled && cheat_enabled_code_exists);
      return;
    }
  }
  cheat_enabled_code_exists = false;
  cheat_enabled = false;
}

//address lookup table manipulation and mirroring
//mirror_address() 0x000000 -> 0x7e0000
//set() enable specified address, mirror accordingly
//clear() disable specified address, mirror accordingly
unsigned Cheat::mirror_address(unsigned addr) const {
  if((addr & 0x40e000) != 0x0000) return addr;
  //8k WRAM mirror
  //$[00-3f|80-bf]:[0000-1fff] -> $7e:[0000-1fff]
  return (0x7e0000 + (addr & 0x1fff));
}

//updates mask[] table enabled bits;
//must be called after modifying item.enabled state.
void Cheat::update(const cheat_t &item) {
  for(unsigned n = 0; n < item.count; n++) {
    (item.enabled) ? set(item.addr[n]) : clear(item.addr[n]);
  }
}

void Cheat::set(unsigned addr) {
  addr = mirror_address(addr);

  mask[addr >> 3] |= 1 << (addr & 7);
  if((addr & 0xffe000) == 0x7e0000) {
    //mirror $7e:[0000-1fff] to $[00-3f|80-bf]:[0000-1fff]
    unsigned mirror;
    for(unsigned x = 0; x <= 0x3f; x++) {
      mirror = ((0x00 + x) << 16) + (addr & 0x1fff);
      mask[mirror >> 3] |= 1 << (mirror & 7);
      mirror = ((0x80 + x) << 16) + (addr & 0x1fff);
      mask[mirror >> 3] |= 1 << (mirror & 7);
    }
  }
}

void Cheat::clear(unsigned addr) {
  addr = mirror_address(addr);

  //if there is more than one cheat code using the same address,
  //(eg with a different override value) then do not clear code
  //lookup table entry.
  uint8_t r;
  if(read(addr, r) == true) return;

  mask[addr >> 3] &= ~(1 << (addr & 7));
  if((addr & 0xffe000) == 0x7e0000) {
    //mirror $7e:[0000-1fff] to $[00-3f|80-bf]:[0000-1fff]
    unsigned mirror;
    for(unsigned x = 0; x <= 0x3f; x++) {
      mirror = ((0x00 + x) << 16) + (addr & 0x1fff);
      mask[mirror >> 3] &= ~(1 << (mirror & 7));
      mirror = ((0x80 + x) << 16) + (addr & 0x1fff);
      mask[mirror >> 3] &= ~(1 << (mirror & 7));
    }
  }
}

//these two functions are used to safely store description text inside .cfg file format.

string& Cheat::encode_description(string &desc) const {
  desc.replace("\"", "\\q");
  desc.replace("\n", "\\n");
  return desc;
}

string& Cheat::decode_description(string &desc) const {
  desc.replace("\\q", "\"");
  desc.replace("\\n", "\n");
  return desc;
}

};

