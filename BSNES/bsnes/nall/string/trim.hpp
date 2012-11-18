#ifdef NALL_STRING_INTERNAL_HPP

namespace nall {

//limit defaults to zero, which will underflow on first compare; equivalent to no limit
template<unsigned Limit> char* ltrim(char *str, const char *key) {
  unsigned limit = Limit;
  if(!key || !*key) return str;
  while(strbegin(str, key)) {
    char *dest = str, *src = str + strlen(key);
    while(true) {
      *dest = *src++;
      if(!*dest) break;
      dest++;
    }
    if(--limit == 0) break;
  }
  return str;
}

template<unsigned Limit> char* rtrim(char *str, const char *key) {
  unsigned limit = Limit;
  if(!key || !*key) return str;
  while(strend(str, key)) {
    str[strlen(str) - strlen(key)] = 0;
    if(--limit == 0) break;
  }
  return str;
}

template<unsigned limit> char* trim(char *str, const char *key, const char *rkey) {
  if(rkey) return ltrim<limit>(rtrim<limit>(str, rkey), key);
  return ltrim<limit>(rtrim<limit>(str, key), key);
}

}

#endif
