#ifdef NALL_STRING_INTERNAL_HPP

namespace nall {

string currentpath() {
  char path[PATH_MAX];
  if(::getcwd(path)) {
    string result(path);
    result.transform("\\", "/");
    if(result.endswith("/") == false) result.append("/");
    return result;
  }
  return "./";
}

string userpath() {
  char path[PATH_MAX];
  if(::userpath(path)) {
    string result(path);
    result.transform("\\", "/");
    if(result.endswith("/") == false) result.append("/");
    return result;
  }
  return currentpath();
}

string realpath(const char *name) {
  char path[PATH_MAX];
  if(::realpath(name, path)) {
    string result(path);
    result.transform("\\", "/");
    return result;
  }
  return userpath();
}

}

#endif
