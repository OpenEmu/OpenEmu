#ifndef NALL_STREAM_VECTOR_HPP
#define NALL_STREAM_VECTOR_HPP

#include <nall/stream/stream.hpp>
#include <nall/vector.hpp>

namespace nall {

struct vectorstream : stream {
  using stream::read;
  using stream::write;

  bool seekable() const { return true; }
  bool readable() const { return true; }
  bool writable() const { return pwritable; }
  bool randomaccess() const { return true; }

  uint8_t* data() const { return memory.data(); }
  unsigned size() const { return memory.size(); }
  unsigned offset() const { return poffset; }
  void seek(unsigned offset) const { poffset = offset; }

  uint8_t read() const { return memory[poffset++]; }
  void write(uint8_t data) const { memory[poffset++] = data; }

  uint8_t read(unsigned offset) const { return memory[offset]; }
  void write(unsigned offset, uint8_t data) const { memory[offset] = data; }

  vectorstream(vector<uint8_t> &memory) : memory(memory), poffset(0), pwritable(true) {}
  vectorstream(const vector<uint8_t> &memory) : memory((vector<uint8_t>&)memory), poffset(0), pwritable(false) {}

protected:
  vector<uint8_t> &memory;
  mutable unsigned poffset, pwritable;
};

}

#endif
