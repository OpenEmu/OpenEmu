#ifdef NALL_STRING_INTERNAL_HPP

namespace nall {

template<unsigned Limit, bool Insensitive, bool Quoted>
string& string::ureplace(const char *key, const char *token) {
  if(!key || !*key) return *this;
  enum : unsigned { limit = Limit ? Limit : ~0u };

  const char *p = data;
  unsigned counter = 0, keyLength = 0;

  while(*p) {
    if(quoteskip<Quoted>(p)) continue;
    for(unsigned n = 0;; n++) {
      if(key[n] == 0) { counter++; p += n; keyLength = n; break; }
      if(!chrequal<Insensitive>(key[n], p[n])) { p++; break; }
    }
  }
  if(counter == 0) return *this;
  if(Limit) counter = min(counter, Limit);

  char *t = data, *base;
  unsigned tokenLength = strlen(token);
  if(tokenLength > keyLength) {
    t = base = strdup(data);
    reserve((unsigned)(p - data) + ((tokenLength - keyLength) * counter));
  }
  char *o = data;

  while(*t && counter) {
    if(quotecopy<Quoted>(o, t)) continue;
    for(unsigned n = 0;; n++) {
      if(key[n] == 0) { counter--; memcpy(o, token, tokenLength); t += keyLength; o += tokenLength; break; }
      if(!chrequal<Insensitive>(key[n], t[n])) { *o++ = *t++; break; }
    }
  }
  do *o++ = *t; while(*t++);
  if(tokenLength > keyLength) free(base);

  return *this;
}

template<unsigned Limit> string &string::replace(const char *key, const char *token) { return ureplace<Limit, false, false>(key, token); }
template<unsigned Limit> string &string::ireplace(const char *key, const char *token) { return ureplace<Limit, true, false>(key, token); }
template<unsigned Limit> string &string::qreplace(const char *key, const char *token) { return ureplace<Limit, false, true>(key, token); }
template<unsigned Limit> string &string::iqreplace(const char *key, const char *token) { return ureplace<Limit, true, true>(key, token); }

};

#endif
