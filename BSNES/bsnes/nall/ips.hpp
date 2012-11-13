#ifndef NALL_IPS_HPP
#define NALL_IPS_HPP

#include <nall/file.hpp>
#include <nall/stdint.hpp>
#include <nall/string.hpp>

namespace nall {

struct ips {
  inline bool apply();
  inline void source(const uint8_t *data, unsigned size);
  inline void modify(const uint8_t *data, unsigned size);
  inline ips();
  inline ~ips();

  uint8_t *data;
  unsigned size;
  const uint8_t *sourceData;
  unsigned sourceSize;
  const uint8_t *modifyData;
  unsigned modifySize;
};

bool ips::apply() {
  if(modifySize < 8) return false;
  if(modifyData[0] != 'P') return false;
  if(modifyData[1] != 'A') return false;
  if(modifyData[2] != 'T') return false;
  if(modifyData[3] != 'C') return false;
  if(modifyData[4] != 'H') return false;

  if(data) delete[] data;
  data = new uint8_t[16 * 1024 * 1024 + 65536]();  //maximum size of IPS patch + single-tag padding
  size = sourceSize;
  memcpy(data, sourceData, sourceSize);
  unsigned offset = 5;

  while(true) {
    unsigned address, length;

    if(offset > modifySize - 3) break;
    address  = modifyData[offset++] << 16;
    address |= modifyData[offset++] <<  8;
    address |= modifyData[offset++] <<  0;

    if(address == 0x454f46) {  //EOF
      if(offset == modifySize) return true;
      if(offset == modifySize - 3) {
        size  = modifyData[offset++] << 16;
        size |= modifyData[offset++] <<  8;
        size |= modifyData[offset++] <<  0;
        return true;
      }
    }

    if(offset > modifySize - 2) break;
    length  = modifyData[offset++] << 8;
    length |= modifyData[offset++] << 0;

    if(length) {  //Copy
      if(offset > modifySize - length) break;
      while(length--) data[address++] = modifyData[offset++];
    } else {  //RLE
      if(offset > modifySize - 3) break;
      length  = modifyData[offset++] << 8;
      length |= modifyData[offset++] << 0;
      if(length == 0) break;  //illegal
      while(length--) data[address++] = modifyData[offset];
      offset++;
    }

    size = max(size, address);
  }

  delete[] data;
  data = nullptr;
  return false;
}

void ips::source(const uint8_t *data, unsigned size) {
  sourceData = data, sourceSize = size;
}

void ips::modify(const uint8_t *data, unsigned size) {
  modifyData = data, modifySize = size;
}

ips::ips() : data(nullptr), sourceData(nullptr), modifyData(nullptr) {
}

ips::~ips() {
  if(data) delete[] data;
  if(sourceData) delete[] sourceData;
  if(modifyData) delete[] modifyData;
}

}

#endif
