#ifndef NALL_STRING_TRIM_HPP
#define NALL_STRING_TRIM_HPP

inline char* ltrim(char *str, const char *key) {
  if(!key || !*key) return str;
  while(strbegin(str, key)) {
    char *dest = str, *src = str + strlen(key);
    while(true) {
      *dest = *src++;
      if(!*dest) break;
      dest++;
    }
  }
  return str;
}

inline char* rtrim(char *str, const char *key) {
  if(!key || !*key) return str;
  while(strend(str, key)) str[strlen(str) - strlen(key)] = 0;
  return str;
}

inline char* trim(char *str, const char *key) {
  return ltrim(rtrim(str, key), key);
}

inline char* ltrim_once(char *str, const char *key) {
  if(!key || !*key) return str;
  if(strbegin(str, key)) {
    char *dest = str, *src = str + strlen(key);
    while(true) {
      *dest = *src++;
      if(!*dest) break;
      dest++;
    }
  }
  return str;
}

inline char* rtrim_once(char *str, const char *key) {
  if(!key || !*key) return str;
  if(strend(str, key)) str[strlen(str) - strlen(key)] = 0;
  return str;
}

inline char* trim_once(char *str, const char *key) {
  return ltrim_once(rtrim_once(str, key), key);
}

#endif
