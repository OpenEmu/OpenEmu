#ifndef NALL_STRING_COMPARE_HPP
#define NALL_STRING_COMPARE_HPP

namespace nall {

inline char chrlower(char c) {
  return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

inline char chrupper(char c) {
  return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

inline int stricmp(const char *dest, const char *src) {
  while(*dest) {
    if(chrlower(*dest) != chrlower(*src)) break;
    dest++;
    src++;
  }

  return (int)chrlower(*dest) - (int)chrlower(*src);
}

inline bool strbegin(const char *str, const char *key) {
  int i, ssl = strlen(str), ksl = strlen(key);

  if(ksl > ssl) return false;
  return (!memcmp(str, key, ksl));
}

inline bool stribegin(const char *str, const char *key) {
  int ssl = strlen(str), ksl = strlen(key);

  if(ksl > ssl) return false;
  for(int i = 0; i < ksl; i++) {
    if(str[i] >= 'A' && str[i] <= 'Z') {
      if(str[i] != key[i] && str[i]+0x20 != key[i])return false;
    } else if(str[i] >= 'a' && str[i] <= 'z') {
      if(str[i] != key[i] && str[i]-0x20 != key[i])return false;
    } else {
      if(str[i] != key[i])return false;
    }
  }
  return true;
}

inline bool strend(const char *str, const char *key) {
  int ssl = strlen(str), ksl = strlen(key);

  if(ksl > ssl) return false;
  return (!memcmp(str + ssl - ksl, key, ksl));
}

inline bool striend(const char *str, const char *key) {
  int ssl = strlen(str), ksl = strlen(key);

  if(ksl > ssl) return false;
  for(int i = ssl - ksl, z = 0; i < ssl; i++, z++) {
    if(str[i] >= 'A' && str[i] <= 'Z') {
      if(str[i] != key[z] && str[i]+0x20 != key[z])return false;
    } else if(str[i] >= 'a' && str[i] <= 'z') {
      if(str[i] != key[z] && str[i]-0x20 != key[z])return false;
    } else {
      if(str[i] != key[z])return false;
    }
  }
  return true;
}

bool wildcard(const char *s, const char *p) {
  const char *cp = 0, *mp = 0;
  while(*s && *p != '*') {
    if(*p != '?' && *s != *p) return false;
    p++, s++;
  }
  while(*s) {
    if(*p == '*') {
      if(!*++p) return true;
      mp = p, cp = s + 1;
    } else if(*p == '?' || *p == *s) {
      p++, s++;
    } else {
      p = mp, s = cp++;
    }
  }
  while(*p == '*') p++;
  return !*p;
}

bool iwildcard(const char *s, const char *p) {
  const char *cp = 0, *mp = 0;
  while(*s && *p != '*') {
    if(*p != '?' && chrlower(*s) != chrlower(*p)) return false;
    p++, s++;
  }
  while(*s) {
    if(*p == '*') {
      if(!*++p) return true;
      mp = p, cp = s + 1;
    } else if(*p == '?' || chrlower(*p) == chrlower(*s)) {
      p++, s++;
    } else {
      p = mp, s = cp++;
    }
  }
  while(*p == '*') p++;
  return !*p;
}

}

#endif
