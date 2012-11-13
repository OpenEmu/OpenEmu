#ifndef NALL_STREAM_MEMORY_HPP
#define NALL_STREAM_MEMORY_HPP

#include <nall/stream/stream.hpp>

namespace nall {

struct memorystream : stream {
  using stream::read;
  using stream::write;

  bool seekable() const { return true; }
  bool readable() const { return true; }
  bool writable() const { return pwritable; }
  bool randomaccess() const { return true; }

  uint8_t *data() const { return pdata; }
  unsigned size() const { return psize; }
  unsigned offset() const { return poffset; }
  void seek(unsigned offset) const { poffset = offset; }

  uint8_t read() const { return pdata[poffset++]; }
  void write(uint8_t data) const { pdata[poffset++] = data; }

  uint8_t read(unsigned offset) const { return pdata[offset]; }
  void write(unsigned offset, uint8_t data) const { pdata[offset] = data; }

  memorystream() : pdata(nullptr), psize(0), poffset(0), pwritable(true) {}

  memorystream(uint8_t *data, unsigned size) {
    pdata = data, psize = size, poffset = 0;
    pwritable = true;
  }

  memorystream(const uint8_t *data, unsigned size) {
    pdata = (uint8_t*)data, psize = size, poffset = 0;
    pwritable = false;
  }

protected:
  mutable uint8_t *pdata;
  mutable unsigned psize, poffset, pwritable;
};

}

#endif
