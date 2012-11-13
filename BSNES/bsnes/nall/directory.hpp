#ifndef NALL_DIRECTORY_HPP
#define NALL_DIRECTORY_HPP

#include <nall/intrinsics.hpp>
#include <nall/sort.hpp>
#include <nall/string.hpp>
#include <nall/vector.hpp>

#if defined(PLATFORM_WINDOWS)
  #include <nall/windows/utf8.hpp>
#else
  #include <dirent.h>
  #include <stdio.h>
  #include <sys/types.h>
#endif

namespace nall {

struct directory {
  static bool create(const string &pathname, unsigned permissions = 0755);
  static bool remove(const string &pathname);
  static bool exists(const string &pathname);
  static lstring folders(const string &pathname, const string &pattern = "*");
  static lstring files(const string &pathname, const string &pattern = "*");
  static lstring contents(const string &pathname, const string &pattern = "*");
};

#if defined(PLATFORM_WINDOWS)
  inline bool directory::create(const string &pathname, unsigned permissions) {
    return _wmkdir(utf16_t(pathname)) == 0;
  }

  inline bool directory::remove(const string &pathname) {
    return _wrmdir(utf16_t(pathname)) == 0;
  }

  inline bool directory::exists(const string &pathname) {
    DWORD result = GetFileAttributes(utf16_t(pathname));
    if(result == INVALID_FILE_ATTRIBUTES) return false;
    return (result & FILE_ATTRIBUTE_DIRECTORY);
  }

  inline lstring directory::folders(const string &pathname, const string &pattern) {
    lstring list;
    string path = pathname;
    path.transform("/", "\\");
    if(!strend(path, "\\")) path.append("\\");
    path.append("*");
    HANDLE handle;
    WIN32_FIND_DATA data;
    handle = FindFirstFile(utf16_t(path), &data);
    if(handle != INVALID_HANDLE_VALUE) {
      if(wcscmp(data.cFileName, L".") && wcscmp(data.cFileName, L"..")) {
        if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
          string name = (const char*)utf8_t(data.cFileName);
          if(wildcard(name, pattern)) list.append(name);
        }
      }
      while(FindNextFile(handle, &data) != false) {
        if(wcscmp(data.cFileName, L".") && wcscmp(data.cFileName, L"..")) {
          if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            string name = (const char*)utf8_t(data.cFileName);
            if(wildcard(name, pattern)) list.append(name);
          }
        }
      }
      FindClose(handle);
    }
    if(list.size() > 0) list.sort();
    for(auto &name : list) name.append("/");  //must append after sorting
    return list;
  }

  inline lstring directory::files(const string &pathname, const string &pattern) {
    lstring list;
    string path = pathname;
    path.transform("/", "\\");
    if(!strend(path, "\\")) path.append("\\");
    path.append("*");
    HANDLE handle;
    WIN32_FIND_DATA data;
    handle = FindFirstFile(utf16_t(path), &data);
    if(handle != INVALID_HANDLE_VALUE) {
      if((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        string name = (const char*)utf8_t(data.cFileName);
        if(wildcard(name, pattern)) list.append(name);
      }
      while(FindNextFile(handle, &data) != false) {
        if((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
          string name = (const char*)utf8_t(data.cFileName);
          if(wildcard(name, pattern)) list.append(name);
        }
      }
      FindClose(handle);
    }
    if(list.size() > 0) list.sort();
    return list;
  }

  inline lstring directory::contents(const string &pathname, const string &pattern) {
    lstring folders = directory::folders(pathname);  //pattern search of contents() should only filter files
    lstring files = directory::files(pathname, pattern);
    for(auto &file : files) folders.append(file);
    return folders;
  }
#else
  inline bool directory::create(const string &pathname, unsigned permissions) {
    return mkdir(pathname, permissions) == 0;
  }

  inline bool directory::remove(const string &pathname) {
    return rmdir(pathname) == 0;
  }

  inline bool directory::exists(const string &pathname) {
    DIR *dp = opendir(pathname);
    if(!dp) return false;
    closedir(dp);
    return true;
  }

  inline lstring directory::folders(const string &pathname, const string &pattern) {
    lstring list;
    DIR *dp;
    struct dirent *ep;
    dp = opendir(pathname);
    if(dp) {
      while(ep = readdir(dp)) {
        if(!strcmp(ep->d_name, ".")) continue;
        if(!strcmp(ep->d_name, "..")) continue;
        if(ep->d_type & DT_DIR) {
          if(wildcard(ep->d_name, pattern)) list.append(ep->d_name);
        }
      }
      closedir(dp);
    }
    if(list.size() > 0) list.sort();
    for(auto &name : list) name.append("/");  //must append after sorting
    return list;
  }

  inline lstring directory::files(const string &pathname, const string &pattern) {
    lstring list;
    DIR *dp;
    struct dirent *ep;
    dp = opendir(pathname);
    if(dp) {
      while(ep = readdir(dp)) {
        if(!strcmp(ep->d_name, ".")) continue;
        if(!strcmp(ep->d_name, "..")) continue;
        if((ep->d_type & DT_DIR) == 0) {
          if(wildcard(ep->d_name, pattern)) list.append(ep->d_name);
        }
      }
      closedir(dp);
    }
    if(list.size() > 0) list.sort();
    return list;
  }

  inline lstring directory::contents(const string &pathname, const string &pattern) {
    lstring folders = directory::folders(pathname);  //pattern search of contents() should only filter files
    lstring files = directory::files(pathname, pattern);
    for(auto &file : files) folders.append(file);
    return folders;
  }
#endif

}

#endif
