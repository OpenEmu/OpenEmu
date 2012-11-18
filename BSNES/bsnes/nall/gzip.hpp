#ifndef NALL_GZIP_HPP
#define NALL_GZIP_HPP

#include <nall/file.hpp>
#include <nall/inflate.hpp>

namespace nall {

struct gzip {
  string filename;
  uint8_t *data;
  unsigned size;

  inline bool decompress(const string &filename);
  inline bool decompress(const uint8_t *data, unsigned size);

  inline gzip();
  inline ~gzip();
};

bool gzip::decompress(const string &filename) {
  if(auto memory = file::read(filename)) {
    return decompress(memory.data(), memory.size());
  }
  return false;
}

bool gzip::decompress(const uint8_t *data, unsigned size) {
  if(size < 18) return false;
  if(data[0] != 0x1f) return false;
  if(data[1] != 0x8b) return false;
  unsigned cm = data[2];
  unsigned flg = data[3];
  unsigned mtime = data[4];
  mtime |= data[5] << 8;
  mtime |= data[6] << 16;
  mtime |= data[7] << 24;
  unsigned xfl = data[8];
  unsigned os = data[9];
  unsigned p = 10;
  unsigned isize = data[size - 4];
  isize |= data[size - 3] << 8;
  isize |= data[size - 2] << 16;
  isize |= data[size - 1] << 24;
  filename = "";

  if(flg & 0x04) {  //FEXTRA
    unsigned xlen = data[p + 0];
    xlen |= data[p + 1] << 8;
    p += 2 + xlen;
  }

  if(flg & 0x08) {  //FNAME
    char buffer[PATH_MAX];
    for(unsigned n = 0; n < PATH_MAX; n++, p++) {
      buffer[n] = data[p];
      if(data[p] == 0) break;
    }
    if(data[p++]) return false;
    filename = buffer;
  }

  if(flg & 0x10) {  //FCOMMENT
    while(data[p++]);
  }

  if(flg & 0x02) {  //FHCRC
    p += 2;
  }

  this->size = isize;
  this->data = new uint8_t[this->size];
  return inflate(this->data, this->size, data + p, size - p - 8);
}

gzip::gzip() : data(nullptr) {
}

gzip::~gzip() {
  if(data) delete[] data;
}

}

#endif
