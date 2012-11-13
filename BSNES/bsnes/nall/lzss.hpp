#ifndef NALL_LZSS_HPP
#define NALL_LZSS_HPP

#include <nall/file.hpp>
#include <nall/filemap.hpp>
#include <nall/stdint.hpp>
#include <nall/string.hpp>

namespace nall {

//19:5 pulldown
//8:1 marker: d7-d0
//length: { 4 - 35 }, offset: { 1 - 0x80000 }
//4-byte file size header
//little-endian encoding
struct lzss {
  inline void source(const uint8_t *data, unsigned size);
  inline bool source(const string &filename);
  inline unsigned size() const;
  inline bool compress(const string &filename);
  inline bool decompress(uint8_t *targetData, unsigned targetSize);
  inline bool decompress(const string &filename);

protected:
  struct Node {
    unsigned offset;
    Node *next;
    inline Node() : offset(0), next(nullptr) {}
    inline ~Node() { if(next) delete next; }
  } *tree[65536];

  filemap sourceFile;
  const uint8_t *sourceData;
  unsigned sourceSize;

public:
  inline lzss() : sourceData(nullptr), sourceSize(0) {}
};

void lzss::source(const uint8_t *data, unsigned size) {
  sourceData = data;
  sourceSize = size;
}

bool lzss::source(const string &filename) {
  if(sourceFile.open(filename, filemap::mode::read) == false) return false;
  sourceData = sourceFile.data();
  sourceSize = sourceFile.size();
  return true;
}

unsigned lzss::size() const {
  unsigned size = 0;
  if(sourceSize < 4) return size;
  for(unsigned n = 0; n < 32; n += 8) size |= sourceData[n >> 3] << n;
  return size;
}

bool lzss::compress(const string &filename) {
  file targetFile;
  if(targetFile.open(filename, file::mode::write) == false) return false;

  for(unsigned n = 0; n < 32; n += 8) targetFile.write(sourceSize >> n);
  for(unsigned n = 0; n < 65536; n++) tree[n] = 0;

  uint8_t buffer[25];
  unsigned sourceOffset = 0;

  while(sourceOffset < sourceSize) {
    uint8_t mask = 0x00;
    unsigned bufferOffset = 1;

    for(unsigned iteration = 0; iteration < 8; iteration++) {
      if(sourceOffset >= sourceSize) break;

      uint16_t symbol = sourceData[sourceOffset + 0];
      if(sourceOffset < sourceSize - 1) symbol |= sourceData[sourceOffset + 1] << 8;
      Node *node = tree[symbol];
      unsigned maxLength = 0, maxOffset = 0;

      while(node) {
        if(node->offset < sourceOffset - 0x80000) {
          //out-of-range: all subsequent nodes will also be, so free up their memory
          if(node->next) { delete node->next; node->next = 0; }
          break;
        }

        unsigned length = 0, x = sourceOffset, y = node->offset;
        while(length < 35 && x < sourceSize && sourceData[x++] == sourceData[y++]) length++;
        if(length > maxLength) maxLength = length, maxOffset = node->offset;
        if(length == 35) break;

        node = node->next;
      }

      //attach current symbol to top of tree for subsequent searches
      node = new Node;
      node->offset = sourceOffset;
      node->next = tree[symbol];
      tree[symbol] = node;

      if(maxLength < 4) {
        buffer[bufferOffset++] = sourceData[sourceOffset++];
      } else {
        unsigned output = ((maxLength - 4) << 19) | (sourceOffset - 1 - maxOffset);
        for(unsigned n = 0; n < 24; n += 8) buffer[bufferOffset++] = output >> n;
        mask |= 0x80 >> iteration;
        sourceOffset += maxLength;
      }
    }

    buffer[0] = mask;
    targetFile.write(buffer, bufferOffset);
  }

  sourceFile.close();
  targetFile.close();
  return true;
}

bool lzss::decompress(uint8_t *targetData, unsigned targetSize) {
  if(targetSize < size()) return false;

  unsigned sourceOffset = 4, targetOffset = 0;
  while(sourceOffset < sourceSize) {
    uint8_t mask = sourceData[sourceOffset++];

    for(unsigned iteration = 0; iteration < 8; iteration++) {
      if(sourceOffset >= sourceSize) break;

      if((mask & (0x80 >> iteration)) == 0) {
        targetData[targetOffset++] = sourceData[sourceOffset++];
      } else {
        unsigned code = 0;
        for(unsigned n = 0; n < 24; n += 8) code |= sourceData[sourceOffset++] << n;
        unsigned length = (code >> 19) + 4;
        unsigned offset = targetOffset - 1 - (code & 0x7ffff);
        while(length--) targetData[targetOffset++] = targetData[offset++];
      }
    }
  }
}

bool lzss::decompress(const string &filename) {
  if(sourceSize < 4) return false;
  unsigned targetSize = size();

  file fp;
  if(fp.open(filename, file::mode::write) == false) return false;
  fp.truncate(targetSize);
  fp.close();

  filemap targetFile;
  if(targetFile.open(filename, filemap::mode::readwrite) == false) return false;
  uint8_t *targetData = targetFile.data();

  bool result = decompress(targetData, targetSize);
  sourceFile.close();
  targetFile.close();
  return result;
}

}

#endif
