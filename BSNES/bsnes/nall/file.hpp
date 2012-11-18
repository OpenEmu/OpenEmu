#ifndef NALL_FILE_HPP
#define NALL_FILE_HPP

#include <nall/platform.hpp>
#include <nall/stdint.hpp>
#include <nall/string.hpp>
#include <nall/utility.hpp>
#include <nall/windows/utf8.hpp>
#include <nall/stream/memory.hpp>

namespace nall {
  inline FILE* fopen_utf8(const string &utf8_filename, const char *mode) {
    #if !defined(_WIN32)
    return fopen(utf8_filename, mode);
    #else
    return _wfopen(utf16_t(utf8_filename), utf16_t(mode));
    #endif
  }

  struct file {
    enum class mode : unsigned { read, write, readwrite, writeread };
    enum class index : unsigned { absolute, relative };
    enum class time : unsigned { create, modify, access };

    static bool remove(const string &filename) {
      return unlink(filename) == 0;
    }

    static bool truncate(const string &filename, unsigned size) {
      #if !defined(_WIN32)
      return truncate(filename, size) == 0;
      #else
      bool result = false;
      FILE *fp = fopen(filename, "rb+");
      if(fp) {
        result = _chsize(fileno(fp), size) == 0;
        fclose(fp);
      }
      return result;
      #endif
    }

    static vector<uint8_t> read(const string &filename) {
      vector<uint8_t> memory;
      file fp;
      if(fp.open(filename, mode::read)) {
        memory.resize(fp.size());
        fp.read(memory.data(), memory.size());
      }
      return memory;
    }

    static bool write(const string &filename, const uint8_t *data, unsigned size) {
      file fp;
      if(fp.open(filename, mode::write) == false) return false;
      fp.write(data, size);
      fp.close();
      return true;
    }

    uint8_t read() {
      if(!fp) return 0xff;                       //file not open
      if(file_mode == mode::write) return 0xff;  //reads not permitted
      if(file_offset >= file_size) return 0xff;  //cannot read past end of file
      buffer_sync();
      return buffer[(file_offset++) & buffer_mask];
    }

    uintmax_t readl(unsigned length = 1) {
      uintmax_t data = 0;
      for(int i = 0; i < length; i++) {
        data |= (uintmax_t)read() << (i << 3);
      }
      return data;
    }

    uintmax_t readm(unsigned length = 1) {
      uintmax_t data = 0;
      while(length--) {
        data <<= 8;
        data |= read();
      }
      return data;
    }

    void read(uint8_t *buffer, unsigned length) {
      while(length--) *buffer++ = read();
    }

    void write(uint8_t data) {
      if(!fp) return;                      //file not open
      if(file_mode == mode::read) return;  //writes not permitted
      buffer_sync();
      buffer[(file_offset++) & buffer_mask] = data;
      buffer_dirty = true;
      if(file_offset > file_size) file_size = file_offset;
    }

    void writel(uintmax_t data, unsigned length = 1) {
      while(length--) {
        write(data);
        data >>= 8;
      }
    }

    void writem(uintmax_t data, unsigned length = 1) {
      for(int i = length - 1; i >= 0; i--) {
        write(data >> (i << 3));
      }
    }

    void write(const uint8_t *buffer, unsigned length) {
      while(length--) write(*buffer++);
    }

    template<typename... Args> void print(Args... args) {
      string data(args...);
      const char *p = data;
      while(*p) write(*p++);
    }

    void flush() {
      buffer_flush();
      fflush(fp);
    }

    void seek(int offset, index index_ = index::absolute) {
      if(!fp) return;  //file not open
      buffer_flush();

      uintmax_t req_offset = file_offset;
      switch(index_) {
        case index::absolute: req_offset  = offset; break;
        case index::relative: req_offset += offset; break;
      }

      if(req_offset < 0) req_offset = 0;  //cannot seek before start of file
      if(req_offset > file_size) {
        if(file_mode == mode::read) {     //cannot seek past end of file
          req_offset = file_size;
        } else {                          //pad file to requested location
          file_offset = file_size;
          while(file_size < req_offset) write(0x00);
        }
      }

      file_offset = req_offset;
    }

    unsigned offset() const {
      if(!fp) return 0;  //file not open
      return file_offset;
    }

    unsigned size() const {
      if(!fp) return 0;  //file not open
      return file_size;
    }

    bool truncate(unsigned size) {
      if(!fp) return false;  //file not open
      #if !defined(_WIN32)
      return ftruncate(fileno(fp), size) == 0;
      #else
      return _chsize(fileno(fp), size) == 0;
      #endif
    }

    bool end() {
      if(!fp) return true;  //file not open
      return file_offset >= file_size;
    }

    static bool exists(const string &filename) {
      #if !defined(_WIN32)
      struct stat64 data;
      return stat64(filename, &data) == 0;
      #else
      struct __stat64 data;
      return _wstat64(utf16_t(filename), &data) == 0;
      #endif
    }

    static uintmax_t size(const string &filename) {
      #if !defined(_WIN32)
      struct stat64 data;
      stat64(filename, &data);
      #else
      struct __stat64 data;
      _wstat64(utf16_t(filename), &data);
      #endif
      return S_ISREG(data.st_mode) ? data.st_size : 0u;
    }

    static time_t timestamp(const string &filename, file::time mode = file::time::create) {
      #if !defined(_WIN32)
      struct stat64 data;
      stat64(filename, &data);
      #else
      struct __stat64 data;
      _wstat64(utf16_t(filename), &data);
      #endif
      switch(mode) { default:
        case file::time::create: return data.st_ctime;
        case file::time::modify: return data.st_mtime;
        case file::time::access: return data.st_atime;
      }
    }

    bool open() const {
      return fp;
    }

    bool open(const string &filename, mode mode_) {
      if(fp) return false;

      switch(file_mode = mode_) {
        #if !defined(_WIN32)
        case mode::read:      fp = fopen(filename, "rb" ); break;
        case mode::write:     fp = fopen(filename, "wb+"); break;  //need read permission for buffering
        case mode::readwrite: fp = fopen(filename, "rb+"); break;
        case mode::writeread: fp = fopen(filename, "wb+"); break;
        #else
        case mode::read:      fp = _wfopen(utf16_t(filename), L"rb" ); break;
        case mode::write:     fp = _wfopen(utf16_t(filename), L"wb+"); break;
        case mode::readwrite: fp = _wfopen(utf16_t(filename), L"rb+"); break;
        case mode::writeread: fp = _wfopen(utf16_t(filename), L"wb+"); break;
        #endif
      }
      if(!fp) return false;
      buffer_offset = -1;  //invalidate buffer
      file_offset = 0;
      fseek(fp, 0, SEEK_END);
      file_size = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      return true;
    }

    void close() {
      if(!fp) return;
      buffer_flush();
      fclose(fp);
      fp = 0;
    }

    file() {
      memset(buffer, 0, sizeof buffer);
      buffer_offset = -1;  //invalidate buffer
      buffer_dirty = false;
      fp = 0;
      file_offset = 0;
      file_size = 0;
      file_mode = mode::read;
    }

    ~file() {
      close();
    }

    file& operator=(const file&) = delete;
    file(const file&) = delete;

  private:
    enum { buffer_size = 1 << 12, buffer_mask = buffer_size - 1 };
    char buffer[buffer_size];
    int buffer_offset;
    bool buffer_dirty;
    FILE *fp;
    unsigned file_offset;
    unsigned file_size;
    mode file_mode;

    void buffer_sync() {
      if(!fp) return;  //file not open
      if(buffer_offset != (file_offset & ~buffer_mask)) {
        buffer_flush();
        buffer_offset = file_offset & ~buffer_mask;
        fseek(fp, buffer_offset, SEEK_SET);
        unsigned length = (buffer_offset + buffer_size) <= file_size ? buffer_size : (file_size & buffer_mask);
        if(length) unsigned unused = fread(buffer, 1, length, fp);
      }
    }

    void buffer_flush() {
      if(!fp) return;                      //file not open
      if(file_mode == mode::read) return;  //buffer cannot be written to
      if(buffer_offset < 0) return;        //buffer unused
      if(buffer_dirty == false) return;    //buffer unmodified since read
      fseek(fp, buffer_offset, SEEK_SET);
      unsigned length = (buffer_offset + buffer_size) <= file_size ? buffer_size : (file_size & buffer_mask);
      if(length) unsigned unused = fwrite(buffer, 1, length, fp);
      buffer_offset = -1;                  //invalidate buffer
      buffer_dirty = false;
    }
  };
}

#endif
