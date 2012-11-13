#ifndef NALL_BPS_LINEAR_HPP
#define NALL_BPS_LINEAR_HPP

#include <nall/crc32.hpp>
#include <nall/file.hpp>
#include <nall/filemap.hpp>
#include <nall/stdint.hpp>
#include <nall/string.hpp>

namespace nall {

struct bpslinear {
  inline void source(const uint8_t *data, unsigned size);
  inline void target(const uint8_t *data, unsigned size);

  inline bool source(const string &filename);
  inline bool target(const string &filename);
  inline bool create(const string &filename, const string &metadata = "");

protected:
  enum : unsigned { SourceRead, TargetRead, SourceCopy, TargetCopy };
  enum : unsigned { Granularity = 1 };

  filemap sourceFile;
  const uint8_t *sourceData;
  unsigned sourceSize;

  filemap targetFile;
  const uint8_t *targetData;
  unsigned targetSize;
};

void bpslinear::source(const uint8_t *data, unsigned size) {
  sourceData = data;
  sourceSize = size;
}

void bpslinear::target(const uint8_t *data, unsigned size) {
  targetData = data;
  targetSize = size;
}

bool bpslinear::source(const string &filename) {
  if(sourceFile.open(filename, filemap::mode::read) == false) return false;
  source(sourceFile.data(), sourceFile.size());
  return true;
}

bool bpslinear::target(const string &filename) {
  if(targetFile.open(filename, filemap::mode::read) == false) return false;
  target(targetFile.data(), targetFile.size());
  return true;
}

bool bpslinear::create(const string &filename, const string &metadata) {
  file modifyFile;
  if(modifyFile.open(filename, file::mode::write) == false) return false;

  uint32_t modifyChecksum = ~0;
  unsigned targetRelativeOffset = 0, outputOffset = 0;

  auto write = [&](uint8_t data) {
    modifyFile.write(data);
    modifyChecksum = crc32_adjust(modifyChecksum, data);
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

  unsigned targetReadLength = 0;

  auto targetReadFlush = [&]() {
    if(targetReadLength) {
      encode(TargetRead | ((targetReadLength - 1) << 2));
      unsigned offset = outputOffset - targetReadLength;
      while(targetReadLength) write(targetData[offset++]), targetReadLength--;
    }
  };

  write('B');
  write('P');
  write('S');
  write('1');

  encode(sourceSize);
  encode(targetSize);

  unsigned markupSize = metadata.length();
  encode(markupSize);
  for(unsigned n = 0; n < markupSize; n++) write(metadata[n]);

  while(outputOffset < targetSize) {
    unsigned sourceLength = 0;
    for(unsigned n = 0; outputOffset + n < min(sourceSize, targetSize); n++) {
      if(sourceData[outputOffset + n] != targetData[outputOffset + n]) break;
      sourceLength++;
    }

    unsigned rleLength = 0;
    for(unsigned n = 1; outputOffset + n < targetSize; n++) {
      if(targetData[outputOffset] != targetData[outputOffset + n]) break;
      rleLength++;
    }

    if(rleLength >= 4) {
      //write byte to repeat
      targetReadLength++;
      outputOffset++;
      targetReadFlush();

      //copy starting from repetition byte
      encode(TargetCopy | ((rleLength - 1) << 2));
      unsigned relativeOffset = (outputOffset - 1) - targetRelativeOffset;
      encode(relativeOffset << 1);
      outputOffset += rleLength;
      targetRelativeOffset = outputOffset - 1;
    } else if(sourceLength >= 4) {
      targetReadFlush();
      encode(SourceRead | ((sourceLength - 1) << 2));
      outputOffset += sourceLength;
    } else {
      targetReadLength += Granularity;
      outputOffset += Granularity;
    }
  }

  targetReadFlush();

  uint32_t sourceChecksum = crc32_calculate(sourceData, sourceSize);
  for(unsigned n = 0; n < 32; n += 8) write(sourceChecksum >> n);
  uint32_t targetChecksum = crc32_calculate(targetData, targetSize);
  for(unsigned n = 0; n < 32; n += 8) write(targetChecksum >> n);
  uint32_t outputChecksum = ~modifyChecksum;
  for(unsigned n = 0; n < 32; n += 8) write(outputChecksum >> n);

  modifyFile.close();
  return true;
}

}

#endif
