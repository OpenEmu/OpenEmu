#ifndef NALL_STRING_STRPOS_HPP
#define NALL_STRING_STRPOS_HPP

//usage example:
//if(auto pos = strpos(str, key)) print(pos(), "\n");
//prints position of key within str, only if it is found

namespace nall {

template<bool Insensitive, bool Quoted>
optional<unsigned> ustrpos(const char *str, const char *key) {
  const char *base = str;

  while(*str) {
    if(quoteskip<Quoted>(str)) continue;
    for(unsigned n = 0;; n++) {
      if(key[n] == 0) return optional<unsigned>(true, (unsigned)(str - base));
      if(str[n] == 0) return optional<unsigned>(false, 0);
      if(!chrequal<Insensitive>(str[n], key[n])) break;
    }
    str++;
  }

  return optional<unsigned>(false, 0);
}

optional<unsigned> strpos(const char *str, const char *key) { return ustrpos<false, false>(str, key); }
optional<unsigned> istrpos(const char *str, const char *key) { return ustrpos<true, false>(str, key); }
optional<unsigned> qstrpos(const char *str, const char *key) { return ustrpos<false, true>(str, key); }
optional<unsigned> iqstrpos(const char *str, const char *key) { return ustrpos<true, true>(str, key); }

}

#endif
