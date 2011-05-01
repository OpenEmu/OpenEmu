#ifndef NALL_STRING_COMPARE_HPP
#define NALL_STRING_COMPARE_HPP

char chrlower(char c) {
  return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

char chrupper(char c) {
  return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

int stricmp(const char *dest, const char *src) {
  while(*dest) {
    if(chrlower(*dest) != chrlower(*src)) break;
    dest++;
    src++;
  }

  return (int)chrlower(*dest) - (int)chrlower(*src);
}

int strpos(const char *str, const char *key) {
  int ssl = strlen(str), ksl = strlen(key);

  if(ksl > ssl) return -1;
  for(int i = 0; i <= ssl - ksl; i++) {
    if(!memcmp(str + i, key, ksl)) {
      return i;
    }
  }
  return -1;
}

int qstrpos(const char *str, const char *key) {
  int ssl = strlen(str), ksl = strlen(key);

  if(ksl > ssl) return -1;
  for(int i = 0; i <= ssl - ksl;) {
  uint8_t x = str[i];
    if(x == '\"' || x == '\'') {
    uint8_t z = i++;
      while(str[i] != x && i < ssl) i++;
      if(i >= ssl) i = z;
    }
    if(!memcmp(str + i, key, ksl)) {
      return i;
    } else {
      i++;
    }
  }
  return -1;
}

bool strbegin(const char *str, const char *key) {
  int i, ssl = strlen(str), ksl = strlen(key);

  if(ksl > ssl) return false;
  return (!memcmp(str, key, ksl));
}

bool stribegin(const char *str, const char *key) {
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

bool strend(const char *str, const char *key) {
  int ssl = strlen(str), ksl = strlen(key);

  if(ksl > ssl) return false;
  return (!memcmp(str + ssl - ksl, key, ksl));
}

bool striend(const char *str, const char *key) {
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

#endif
