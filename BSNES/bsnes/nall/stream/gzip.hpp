#ifndef NALL_STREAM_GZIP_HPP
#define NALL_STREAM_GZIP_HPP

#include <nall/gzip.hpp>

namespace nall {

struct gzipstream : memorystream {
  using stream::read;
  using stream::write;

  gzipstream(const stream &stream) {
    unsigned size = stream.size();
    uint8_t *data = new uint8_t[size];
    stream.read(data, size);

    gzip archive;
    bool result = archive.decompress(data, size);
    delete[] data;
    if(result == false) return;

    psize = archive.size;
    pdata = new uint8_t[psize];
    memcpy(pdata, archive.data, psize);
  }

  ~gzipstream() {
    if(pdata) delete[] pdata;
  }
};

}

#endif
