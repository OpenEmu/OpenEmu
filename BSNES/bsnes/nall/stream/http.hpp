#ifndef NALL_STREAM_HTTP_HPP
#define NALL_STREAM_HTTP_HPP

#include <nall/http.hpp>

namespace nall {

struct httpstream : stream {
  using stream::read;
  using stream::write;

  bool seekable() const { return true; }
  bool readable() const { return true; }
  bool writable() const { return true; }
  bool randomaccess() const { return true; }

  unsigned size() const { return psize; }
  unsigned offset() const { return poffset; }
  void seek(unsigned offset) const { poffset = offset; }

  uint8_t read() const { return pdata[poffset++]; }
  void write(uint8_t data) const { pdata[poffset++] = data; }

  uint8_t read(unsigned offset) const { return pdata[offset]; }
  void write(unsigned offset, uint8_t data) const { pdata[offset] = data; }

  httpstream(const string &url, unsigned port) : pdata(nullptr), psize(0), poffset(0) {
    string uri = url;
    uri.ltrim<1>("http://");
    lstring part = uri.split<1>("/");
    part[1] = { "/", part[1] };

    http connection;
    if(connection.connect(part[0], port) == false) return;
    connection.download(part[1], pdata, psize);
  }

  ~httpstream() {
    if(pdata) delete[] pdata;
  }

private:
  mutable uint8_t *pdata;
  mutable unsigned psize, poffset;
};

}

#endif
