#ifndef NALL_DL_HPP
#define NALL_DL_HPP

//dynamic linking support

#include <nall/intrinsics.hpp>
#include <nall/stdint.hpp>
#include <nall/string.hpp>
#include <nall/utility.hpp>

#if defined(PLATFORM_X) || defined(PLATFORM_OSX)
  #include <dlfcn.h>
#elif defined(PLATFORM_WINDOWS)
  #include <windows.h>
  #include <nall/windows/utf8.hpp>
#endif

namespace nall {
  struct library {
    bool opened() const { return handle; }
    bool open(const char*, const char* = "");
    bool open_absolute(const char*);
    void* sym(const char*);
    void close();

    library() : handle(0) {}
    ~library() { close(); }

    library& operator=(const library&) = delete;
    library(const library&) = delete;

  private:
    uintptr_t handle;
  };

  #if defined(PLATFORM_X)
  inline bool library::open(const char *name, const char *path) {
    if(handle) close();
    handle = (uintptr_t)dlopen(string(path, *path && !strend(path, "/") ? "/" : "", "lib", name, ".so"), RTLD_LAZY);
    if(!handle) handle = (uintptr_t)dlopen(string("/usr/local/lib/lib", name, ".so"), RTLD_LAZY);
    return handle;
  }

  inline bool library::open_absolute(const char *name) {
    if(handle) close();
    handle = (uintptr_t)dlopen(name, RTLD_LAZY);
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
  #elif defined(PLATFORM_OSX)
  inline bool library::open(const char *name, const char *path) {
    if(handle) close();
    handle = (uintptr_t)dlopen(string(path, *path && !strend(path, "/") ? "/" : "", "lib", name, ".dylib"), RTLD_LAZY);
    if(!handle) handle = (uintptr_t)dlopen(string("/usr/local/lib/lib", name, ".dylib"), RTLD_LAZY);
    return handle;
  }

  inline bool library::open_absolute(const char *name) {
    if(handle) close();
    handle = (uintptr_t)dlopen(name, RTLD_LAZY);
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
  #elif defined(PLATFORM_WINDOWS)
  inline bool library::open(const char *name, const char *path) {
    if(handle) close();
    string filepath(path, *path && !strend(path, "/") && !strend(path, "\\") ? "\\" : "", name, ".dll");
    handle = (uintptr_t)LoadLibraryW(utf16_t(filepath));
    return handle;
  }

  inline bool library::open_absolute(const char *name) {
    if(handle) close();
    handle = (uintptr_t)LoadLibraryW(utf16_t(name));
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
  inline bool library::open(const char*, const char*) { return false; }
  inline void* library::sym(const char*) { return 0; }
  inline void library::close() {}
  #endif
};

#endif
