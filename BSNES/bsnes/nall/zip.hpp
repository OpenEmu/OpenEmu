#ifndef NALL_UNZIP_HPP
#define NALL_UNZIP_HPP

#include <nall/filemap.hpp>
#include <nall/inflate.hpp>
#include <nall/string.hpp>
#include <nall/vector.hpp>

namespace nall {

struct zip {
  struct File {
    string name;
    const uint8_t *data;
    unsigned size;
    unsigned csize;
    unsigned cmode;  //0 = uncompressed, 8 = deflate
    unsigned crc32;
  };

  inline bool open(const string &filename) {
    close();
    if(fm.open(filename, filemap::mode::read) == false) return false;
    if(open(fm.data(), fm.size()) == false) {
      fm.close();
      return false;
    }
    return true;
  }

  inline bool open(const uint8_t *data, unsigned size) {
    if(size < 22) return false;

    filedata = data;
    filesize = size;

    file.reset();

    const uint8_t *footer = data + size - 22;
    while(true) {
      if(footer <= data + 22) return false;
      if(read(footer, 4) == 0x06054b50) {
        unsigned commentlength = read(footer + 20, 2);
        if(footer + 22 + commentlength == data + size) break;
      }
      footer--;
    }
    const uint8_t *directory = data + read(footer + 16, 4);

    while(true) {
      unsigned signature = read(directory + 0, 4);
      if(signature != 0x02014b50) break;

      File file;
      file.cmode = read(directory + 10, 2);
      file.crc32 = read(directory + 16, 4);
      file.csize = read(directory + 20, 4);
      file.size  = read(directory + 24, 4);

      unsigned namelength = read(directory + 28, 2);
      unsigned extralength = read(directory + 30, 2);
      unsigned commentlength = read(directory + 32, 2);

      char *filename = new char[namelength + 1];
      memcpy(filename, directory + 46, namelength);
      filename[namelength] = 0;
      file.name = filename;
      delete[] filename;

      unsigned offset = read(directory + 42, 4);
      unsigned offsetNL = read(data + offset + 26, 2);
      unsigned offsetEL = read(data + offset + 28, 2);
      file.data = data + offset + 30 + offsetNL + offsetEL;

      directory += 46 + namelength + extralength + commentlength;

      this->file.append(file);
    }

    return true;
  }

  inline bool extract(File &file, uint8_t *&data, unsigned &size) {
    data = 0, size = 0;

    if(file.cmode == 0) {
      size = file.size;
      data = new uint8_t[size];
      memcpy(data, file.data, size);
      return true;
    }

    if(file.cmode == 8) {
      size = file.size;
      data = new uint8_t[size];
      if(inflate(data, size, file.data, file.csize) == false) {
        delete[] data;
        size = 0;
        return false;
      }
      return true;
    }

    return false;
  }

  inline void close() {
    if(fm.open()) fm.close();
  }

  ~zip() {
    close();
  }

protected:
  filemap fm;
  const uint8_t *filedata;
  unsigned filesize;

  unsigned read(const uint8_t *data, unsigned size) {
    unsigned result = 0, shift = 0;
    while(size--) { result |= *data++ << shift; shift += 8; }
    return result;
  }

public:
  vector<File> file;
};

}

#endif
