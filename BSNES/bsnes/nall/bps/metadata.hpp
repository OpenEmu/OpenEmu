#ifndef NALL_BPS_METADATA_HPP
#define NALL_BPS_METADATA_HPP

#include <nall/crc32.hpp>
#include <nall/file.hpp>
#include <nall/filemap.hpp>
#include <nall/stdint.hpp>
#include <nall/string.hpp>

namespace nall {

struct bpsmetadata {
  inline bool load(const string &filename);
  inline bool save(const string &filename, const string &metadata);
  inline string metadata() const;

protected:
  file sourceFile;
  string metadataString;
};

bool bpsmetadata::load(const string &filename) {
  if(sourceFile.open(filename, file::mode::read) == false) return false;

  auto read = [&]() -> uint8_t {
    return sourceFile.read();
  };

  auto decode = [&]() -> uint64_t {
    uint64_t data = 0, shift = 1;
    while(true) {
      uint8_t x = read();
      data += (x & 0x7f) * shift;
      if(x & 0x80) break;
      shift <<= 7;
      data += shift;
    }
    return data;
  };

  if(read() != 'B') return false;
  if(read() != 'P') return false;
  if(read() != 'S') return false;
  if(read() != '1') return false;
  decode();
  decode();
  unsigned metadataSize = decode();
  char data[metadataSize + 1];
  for(unsigned n = 0; n < metadataSize; n++) data[n] = read();
  data[metadataSize] = 0;
  metadataString = (const char*)data;

  return true;
}

bool bpsmetadata::save(const string &filename, const string &metadata) {
  file targetFile;
  if(targetFile.open(filename, file::mode::write) == false) return false;
  if(sourceFile.open() == false) return false;
  sourceFile.seek(0);

  auto read = [&]() -> uint8_t {
    return sourceFile.read();
  };

  auto decode = [&]() -> uint64_t {
    uint64_t data = 0, shift = 1;
    while(true) {
      uint8_t x = read();
      data += (x & 0x7f) * shift;
      if(x & 0x80) break;
      shift <<= 7;
      data += shift;
    }
    return data;
  };

  uint32_t checksum = ~0;

  auto write = [&](uint8_t data) {
    targetFile.write(data);
    checksum = crc32_adjust(checksum, data);
  };

  auto encode = [&](uint64_t data) {
    while(true) {
      uint64_t x = data & 0x7f;
      data >>= 7;
      if(data == 0) {
        write(0x80 | x);
        break;
      }
      write(x);
      data--;
    }
  };

  for(unsigned n = 0; n < 4; n++) write(read());
  encode(decode());
  encode(decode());
  unsigned sourceLength = decode();
  unsigned targetLength = metadata.length();
  encode(targetLength);
  sourceFile.seek(sourceLength, file::index::relative);
  for(unsigned n = 0; n < targetLength; n++) write(metadata[n]);
  unsigned length = sourceFile.size() - sourceFile.offset() - 4;
  for(unsigned n = 0; n < length; n++) write(read());
  uint32_t outputChecksum = ~checksum;
  for(unsigned n = 0; n < 32; n += 8) write(outputChecksum >> n);

  targetFile.close();
  return true;
}

string bpsmetadata::metadata() const {
  return metadataString;
}

}

#endif
