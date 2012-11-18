#ifndef NALL_STREAM_FILE_HPP
#define NALL_STREAM_FILE_HPP

#include <nall/file.hpp>

namespace nall {

struct filestream : stream {
  using stream::read;
  using stream::write;

  bool seekable() const { return true; }
  bool readable() const { return true; }
  bool writable() const { return pwritable; }
  bool randomaccess() const { return false; }

  unsigned size() const { return pfile.size(); }
  unsigned offset() const { return pfile.offset(); }
  void seek(unsigned offset) const { pfile.seek(offset); }

  uint8_t read() const { return pfile.read(); }
  void write(uint8_t data) const { pfile.write(data); }

  filestream(const string &filename) {
    pfile.open(filename, file::mode::readwrite);
    pwritable = pfile.open();
    if(!pwritable) pfile.open(filename, file::mode::read);
  }

  filestream(const string &filename, file::mode mode) {
    pfile.open(filename, mode);
    pwritable = mode == file::mode::write || mode == file::mode::readwrite;
  }

private:
  mutable file pfile;
  bool pwritable;
};

}

#endif
