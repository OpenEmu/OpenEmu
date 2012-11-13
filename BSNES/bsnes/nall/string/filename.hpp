#ifdef NALL_STRING_INTERNAL_HPP

namespace nall {

// "foo/bar.c" -> "foo/"
// "foo/" -> "foo/"
// "bar.c" -> "./"
inline string dir(char const *name) {
  string result = name;
  for(signed i = strlen(result); i >= 0; i--) {
    if(result[i] == '/' || result[i] == '\\') {
      result[i + 1] = 0;
      break;
    }
    if(i == 0) result = "./";
  }
  return result;
}

// "foo/bar.c" -> "bar.c"
inline string notdir(char const *name) {
  for(signed i = strlen(name); i >= 0; i--) {
    if(name[i] == '/' || name[i] == '\\') {
      name += i + 1;
      break;
    }
  }
  string result = name;
  return result;
}

// "foo/bar.c" -> "foo/bar"
inline string basename(char const *name) {
  string result = name;
  for(signed i = strlen(result); i >= 0; i--) {
    if(result[i] == '/' || result[i] == '\\') {
      //file has no extension
      break;
    }
    if(result[i] == '.') {
      result[i] = 0;
      break;
    }
  }
  return result;
}

// "foo/bar.c" -> "c"
inline string extension(char const *name) {
  for(signed i = strlen(name); i >= 0; i--) {
    if(name[i] == '.') {
      name += i + 1;
      break;
    }
  }
  string result = name;
  return result;
}

}

#endif
