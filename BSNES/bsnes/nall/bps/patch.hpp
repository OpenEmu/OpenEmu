#ifndef NALL_BPS_PATCH_HPP
#define NALL_BPS_PATCH_HPP

#include <nall/crc32.hpp>
#include <nall/file.hpp>
#include <nall/filemap.hpp>
#include <nall/stdint.hpp>
#include <nall/string.hpp>

namespace nall {

struct bpspatch {
  inline bool modify(const uint8_t *data, unsigned size);
  inline void source(const uint8_t *data, unsigned size);
  inline void target(uint8_t *data, unsigned size);

  inline bool modify(const string &filename);
  inline bool source(const string &filename);
  inline bool target(const string &filename);

  inline string metadata() const;
  inline unsigned size() const;

  enum result : unsigned {
    unknown,
    success,
    patch_too_small,
    patch_invalid_header,
    source_too_small,
    target_too_small,
    source_checksum_invalid,
    target_checksum_invalid,
    patch_checksum_invalid,
  };

  inline result apply();

protected:
  enum : unsigned { SourceRead, TargetRead, SourceCopy, TargetCopy };

  filemap modifyFile;
  const uint8_t *modifyData;
  unsigned modifySize;

  filemap sourceFile;
  const uint8_t *sourceData;
  unsigned sourceSize;

  filemap targetFile;
  uint8_t *targetData;
  unsigned targetSize;

  unsigned modifySourceSize;
  unsigned modifyTargetSize;
  unsigned modifyMarkupSize;
  string metadataString;
};

bool bpspatch::modify(const uint8_t *data, unsigned size) {
  if(size < 19) return false;
  modifyData = data;
  modifySize = size;

  unsigned offset = 4;
  auto decode = [&]() -> uint64_t {
    uint64_t data = 0, shift = 1;
    while(true) {
      uint8_t x = modifyData[offset++];
      data += (x & 0x7f) * shift;
      if(x & 0x80) break;
      shift <<= 7;
      data += shift;
    }
    return data;
  };

  modifySourceSize = decode();
  modifyTargetSize = decode();
  modifyMarkupSize = decode();

  char buffer[modifyMarkupSize + 1];
  for(unsigned n = 0; n < modifyMarkupSize; n++) buffer[n] = modifyData[offset++];
  buffer[modifyMarkupSize] = 0;
  metadataString = (const char*)buffer;

  return true;
}

void bpspatch::source(const uint8_t *data, unsigned size) {
  sourceData = data;
  sourceSize = size;
}

void bpspatch::target(uint8_t *data, unsigned size) {
  targetData = data;
  targetSize = size;
}

bool bpspatch::modify(const string &filename) {
  if(modifyFile.open(filename, filemap::mode::read) == false) return false;
  return modify(modifyFile.data(), modifyFile.size());
}

bool bpspatch::source(const string &filename) {
  if(sourceFile.open(filename, filemap::mode::read) == false) return false;
  source(sourceFile.data(), sourceFile.size());
  return true;
}

bool bpspatch::target(const string &filename) {
  file fp;
  if(fp.open(filename, file::mode::write) == false) return false;
  fp.truncate(modifyTargetSize);
  fp.close();

  if(targetFile.open(filename, filemap::mode::readwrite) == false) return false;
  target(targetFile.data(), targetFile.size());
  return true;
}

string bpspatch::metadata() const {
  return metadataString;
}

unsigned bpspatch::size() const {
  return modifyTargetSize;
}

bpspatch::result bpspatch::apply() {
  if(modifySize < 19) return result::patch_too_small;

  uint32_t modifyChecksum = ~0, targetChecksum = ~0;
  unsigned modifyOffset = 0, sourceRelativeOffset = 0, targetRelativeOffset = 0, outputOffset = 0;

  auto read = [&]() -> uint8_t {
    uint8_t data = modifyData[modifyOffset++];
    modifyChecksum = crc32_adjust(modifyChecksum, data);
    return data;
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

  auto write = [&](uint8_t data) {
    targetData[outputOffset++] = data;
    targetChecksum = crc32_adjust(targetChecksum, data);
  };

  if(read() != 'B') return result::patch_invalid_header;
  if(read() != 'P') return result::patch_invalid_header;
  if(read() != 'S') return result::patch_invalid_header;
  if(read() != '1') return result::patch_invalid_header;

  modifySourceSize = decode();
  modifyTargetSize = decode();
  modifyMarkupSize = decode();
  for(unsigned n = 0; n < modifyMarkupSize; n++) read();

  if(modifySourceSize > sourceSize) return result::source_too_small;
  if(modifyTargetSize > targetSize) return result::target_too_small;

  while(modifyOffset < modifySize - 12) {
    unsigned length = decode();
    unsigned mode = length & 3;
    length = (length >> 2) + 1;

    switch(mode) {
    case SourceRead:
      while(length--) write(sourceData[outputOffset]);
      break;
    case TargetRead:
      while(length--) write(read());
      break;
    case SourceCopy:
    case TargetCopy:
      signed offset = decode();
      bool negative = offset & 1;
      offset >>= 1;
      if(negative) offset = -offset;

      if(mode == SourceCopy) {
        sourceRelativeOffset += offset;
        while(length--) write(sourceData[sourceRelativeOffset++]);
      } else {
        targetRelativeOffset += offset;
        while(length--) write(targetData[targetRelativeOffset++]);
      }
      break;
    }
  }

  uint32_t modifySourceChecksum = 0, modifyTargetChecksum = 0, modifyModifyChecksum = 0;
  for(unsigned n = 0; n < 32; n += 8) modifySourceChecksum |= read() << n;
  for(unsigned n = 0; n < 32; n += 8) modifyTargetChecksum |= read() << n;
  uint32_t checksum = ~modifyChecksum;
  for(unsigned n = 0; n < 32; n += 8) modifyModifyChecksum |= read() << n;

  uint32_t sourceChecksum = crc32_calculate(sourceData, modifySourceSize);
  targetChecksum = ~targetChecksum;

  if(sourceChecksum != modifySourceChecksum) return result::source_checksum_invalid;
  if(targetChecksum != modifyTargetChecksum) return result::target_checksum_invalid;
  if(checksum != modifyModifyChecksum) return result::patch_checksum_invalid;

  return result::success;
}

}

#endif
