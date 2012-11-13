#ifndef NALL_STREAM_STREAM_HPP
#define NALL_STREAM_STREAM_HPP

namespace nall {

struct stream {
  virtual bool seekable() const = 0;
  virtual bool readable() const = 0;
  virtual bool writable() const = 0;
  virtual bool randomaccess() const = 0;

  virtual uint8_t* data() const { return nullptr; }
  virtual unsigned size() const = 0;
  virtual unsigned offset() const = 0;
  virtual void seek(unsigned offset) const = 0;

  virtual uint8_t read() const = 0;
  virtual void write(uint8_t data) const = 0;

  virtual uint8_t read(unsigned) const { return 0; }
  virtual void write(unsigned, uint8_t) const {}

  operator bool() const {
    return size();
  }

  bool empty() const {
    return size() == 0;
  }

  bool end() const {
    return offset() >= size();
  }

  uintmax_t readl(unsigned length = 1) const {
    uintmax_t data = 0, shift = 0;
    while(length--) { data |= read() << shift; shift += 8; }
    return data;
  }

  uintmax_t readm(unsigned length = 1) const {
    uintmax_t data = 0;
    while(length--) data = (data << 8) | read();
    return data;
  }

  void read(uint8_t *data, unsigned length) const {
    while(length--) *data++ = read();
  }

  void writel(uintmax_t data, unsigned length = 1) const {
    while(length--) {
      write(data);
      data >>= 8;
    }
  }

  void writem(uintmax_t data, unsigned length = 1) const {
    uintmax_t shift = 8 * length;
    while(length--) {
      shift -= 8;
      write(data >> shift);
    }
  }

  void write(const uint8_t *data, unsigned length) const {
    while(length--) write(*data++);
  }

  struct byte {
    operator uint8_t() const { return s.read(offset); }
    byte& operator=(uint8_t data) { s.write(offset, data); }
    byte(const stream &s, unsigned offset) : s(s), offset(offset) {}

  private:
    const stream &s;
    const unsigned offset;
  };

  byte operator[](unsigned offset) const {
    return byte(*this, offset);
  }

  stream() {}
  virtual ~stream() {}
  stream(const stream&) = delete;
  stream& operator=(const stream&) = delete;
};

}

#endif
