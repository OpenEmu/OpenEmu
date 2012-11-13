#ifdef NALL_STRING_INTERNAL_HPP

namespace nall {

char* strlower(char *str) {
  if(!str) return 0;
  int i = 0;
  while(str[i]) {
    str[i] = chrlower(str[i]);
    i++;
  }
  return str;
}

char* strupper(char *str) {
  if(!str) return 0;
  int i = 0;
  while(str[i]) {
    str[i] = chrupper(str[i]);
    i++;
  }
  return str;
}

char* qstrlower(char *s) {
  if(!s) return 0;
  bool quoted = false;
  while(*s) {
    if(*s == '\"' || *s == '\'') quoted ^= 1;
    if(quoted == false && *s >= 'A' && *s <= 'Z') *s += 0x20;
    s++;
  }
}

char* qstrupper(char *s) {
  if(!s) return 0;
  bool quoted = false;
  while(*s) {
    if(*s == '\"' || *s == '\'') quoted ^= 1;
    if(quoted == false && *s >= 'a' && *s <= 'z') *s -= 0x20;
    s++;
  }
}

char* strtr(char *dest, const char *before, const char *after) {
  if(!dest || !before || !after) return dest;
  int sl = strlen(dest), bsl = strlen(before), asl = strlen(after);

  if(bsl != asl || bsl == 0) return dest;  //patterns must be the same length for 1:1 replace
  for(unsigned i = 0; i < sl; i++) {
    for(unsigned l = 0; l < bsl; l++) {
      if(dest[i] == before[l]) {
        dest[i] = after[l];
        break;
      }
    }
  }

  return dest;
}

}

#endif
