#ifndef NALL_DL_HPP
#define NALL_DL_HPP

//dynamic linking support

#include <string.h>
#include <nall/detect.hpp>
#include <nall/stdint.hpp>
#include <nall/utility.hpp>

#if defined(PLATFORM_X)
  #include <dlfcn.h>
#elif defined(PLATFORM_WIN)
  #include <windows.h>
  #include <nall/utf8.hpp>
#endif

namespace nall {
  struct library : noncopyable {
    bool opened() const { return handle; }
    bool open(const char*);
    void* sym(const char*);
    void close();

    library() : handle(0) {}
    ~library() { close(); }

  private:
    uintptr_t handle;
  };

  #if defined(PLATFORM_X)
  inline bool library::open(const char *name) {
    if(handle) close();
    char *t = new char[strlen(name) + 256];
    strcpy(t, "lib");
    strcat(t, name);
    strcat(t, ".so");
    handle = (uintptr_t)dlopen(t, RTLD_LAZY);
    if(!handle) {
      strcpy(t, "/usr/local/lib/lib");
      strcat(t, name);
      strcat(t, ".so");
      handle = (uintptr_t)dlopen(t, RTLD_LAZY);
    }
    delete[] t;
    return handle;
  }

  inline void* library::sym(const char *name) {
    if(!handle) return 0;
    return dlsym((void*)handle, name);
  }

  inline void library::close() {
    if(!handle) return;
    dlclose((void*)handle);
    handle = 0;
  }
  #elif defined(PLATFORM_WIN)
  inline bool library::open(const char *name) {
    if(handle) close();
    char *t = new char[strlen(name) + 8];
    strcpy(t, name);
    strcat(t, ".dll");
    handle = (uintptr_t)LoadLibraryW(utf16_t(t));
    delete[] t;
    return handle;
  }

  inline void* library::sym(const char *name) {
    if(!handle) return 0;
    return (void*)GetProcAddress((HMODULE)handle, name);
  }

  inline void library::close() {
    if(!handle) return;
    FreeLibrary((HMODULE)handle);
    handle = 0;
  }
  #else
  inline bool library::open(const char*) { return false; }
  inline void* library::sym(const char*) { return 0; }
  inline void library::close() {}
  #endif
};

#endif
