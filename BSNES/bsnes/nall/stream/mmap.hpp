#ifndef NALL_STREAM_MMAP_HPP
#define NALL_STREAM_MMAP_HPP

#include <nall/filemap.hpp>

namespace nall {

struct mmapstream : stream {
  using stream::read;
  using stream::write;

  bool seekable() const { return true; }
  bool readable() const { return true; }
  bool writable() const { return pwritable; }
  bool randomaccess() const { return true; }

  unsigned size() const { return pmmap.size(); }
  unsigned offset() const { return poffset; }
  void seek(unsigned offset) const { poffset = offset; }

  uint8_t read() const { return pdata[poffset++]; }
  void write(uint8_t data) const { pdata[poffset++] = data; }

  uint8_t read(unsigned offset) const { return pdata[offset]; }
  void write(unsigned offset, uint8_t data) const { pdata[offset] = data; }

  mmapstream(const string &filename) {
    pmmap.open(filename, filemap::mode::readwrite);
    pwritable = pmmap.open();
    if(!pwritable) pmmap.open(filename, filemap::mode::read);
    pdata = pmmap.data(), poffset = 0;
  }

private:
  mutable filemap pmmap;
  mutable uint8_t *pdata;
  mutable unsigned pwritable, poffset;
};

}

#endif
