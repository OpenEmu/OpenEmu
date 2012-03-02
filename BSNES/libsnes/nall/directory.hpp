#ifndef NALL_DIRECTORY_HPP
#define NALL_DIRECTORY_HPP

#include <nall/foreach.hpp>
#include <nall/sort.hpp>
#include <nall/string.hpp>

#if defined(_WIN32)
  #include <nall/utf8.hpp>
#else
  #include <dirent.h>
  #include <stdio.h>
  #include <sys/types.h>
#endif

namespace nall {

struct directory {
  static lstring folders(const char *pathname);
  static lstring files(const char *pathname);
  static lstring contents(const char *pathname);
};

#if defined(_WIN32)
  inline lstring directory::folders(const char *pathname) {
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
          list.append(string(utf8_t(data.cFileName), "/"));
        }
      }
      while(FindNextFile(handle, &data) != false) {
        if(wcscmp(data.cFileName, L".") && wcscmp(data.cFileName, L"..")) {
          if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            list.append(string(utf8_t(data.cFileName), "/"));
          }
        }
      }
      FindClose(handle);
    }
    sort(&list[0], list.size());
    return list;
  }

  inline lstring directory::files(const char *pathname) {
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
        list.append(utf8_t(data.cFileName));
      }
      while(FindNextFile(handle, &data) != false) {
        if((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
          list.append(utf8_t(data.cFileName));
        }
      }
      FindClose(handle);
    }
    sort(&list[0], list.size());
    return list;
  }

  inline lstring directory::contents(const char *pathname) {
    lstring folders = directory::folders(pathname);
    lstring files = directory::files(pathname);
    foreach(file, files) folders.append(file);
    return folders;
  }
#else
  inline lstring directory::folders(const char *pathname) {
    lstring list;
    DIR *dp;
    struct dirent *ep;
    dp = opendir(pathname);
    if(dp) {
      while(ep = readdir(dp)) {
        if(!strcmp(ep->d_name, ".")) continue;
        if(!strcmp(ep->d_name, "..")) continue;
        if(ep->d_type & DT_DIR) list.append(string(ep->d_name, "/"));
      }
      closedir(dp);
    }
    sort(&list[0], list.size());
    return list;

  }

  inline lstring directory::files(const char *pathname) {
    lstring list;
    DIR *dp;
    struct dirent *ep;
    dp = opendir(pathname);
    if(dp) {
      while(ep = readdir(dp)) {
        if(!strcmp(ep->d_name, ".")) continue;
        if(!strcmp(ep->d_name, "..")) continue;
        if((ep->d_type & DT_DIR) == 0) list.append(ep->d_name);
      }
      closedir(dp);
    }
    sort(&list[0], list.size());
    return list;
  }

  inline lstring directory::contents(const char *pathname) {
    lstring folders = directory::folders(pathname);
    lstring files = directory::files(pathname);
    foreach(file, files) folders.append(file);
    return folders;
  }
#endif

}

#endif
