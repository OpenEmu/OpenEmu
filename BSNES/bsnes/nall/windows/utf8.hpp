#ifndef NALL_UTF8_HPP
#define NALL_UTF8_HPP

//UTF-8 <> UTF-16 conversion
//used only for Win32; Linux, etc use UTF-8 internally

#if defined(_WIN32)

#undef UNICODE
#undef _WIN32_WINNT
#undef  NOMINMAX
#define UNICODE
#define _WIN32_WINNT 0x0501
#define NOMINMAX
#include <windows.h>
#undef interface

namespace nall {
  //UTF-8 to UTF-16
  class utf16_t {
  public:
    operator wchar_t*() {
      return buffer;
    }

    operator const wchar_t*() const {
      return buffer;
    }

    utf16_t(const char *s = "") {
      if(!s) s = "";
      unsigned length = MultiByteToWideChar(CP_UTF8, 0, s, -1, 0, 0);
      buffer = new wchar_t[length + 1]();
      MultiByteToWideChar(CP_UTF8, 0, s, -1, buffer, length);
    }

    ~utf16_t() {
      delete[] buffer;
    }

  private:
    wchar_t *buffer;
  };

  //UTF-16 to UTF-8
  class utf8_t {
  public:
    operator char*() {
      return buffer;
    }

    operator const char*() const {
      return buffer;
    }

    utf8_t(const wchar_t *s = L"") {
      if(!s) s = L"";
      unsigned length = WideCharToMultiByte(CP_UTF8, 0, s, -1, 0, 0, (const char*)0, (BOOL*)0);
      buffer = new char[length + 1]();
      WideCharToMultiByte(CP_UTF8, 0, s, -1, buffer, length, (const char*)0, (BOOL*)0);
    }

    ~utf8_t() {
      delete[] buffer;
    }

    utf8_t(const utf8_t&) = delete;
    utf8_t& operator=(const utf8_t&) = delete;

  private:
    char *buffer;
  };

  inline void utf8_args(int &argc, char **&argv) {
    wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
    argv = new char*[argc];
    for(unsigned i = 0; i < argc; i++) {
      argv[i] = new char[_MAX_PATH];
      strcpy(argv[i], nall::utf8_t(wargv[i]));
    }
  }
}

#endif  //if defined(_WIN32)

#endif
