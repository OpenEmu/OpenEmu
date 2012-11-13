#ifdef NALL_STRING_INTERNAL_HPP

namespace nall {

template<unsigned Limit, bool Insensitive, bool Quoted> lstring& lstring::usplit(const char *key, const char *base) {
  reset();
  if(!key || !*key) return *this;

  const char *p = base;

  while(*p) {
    if(Limit) if(size() >= Limit) break;
    if(quoteskip<Quoted>(p)) continue;
    for(unsigned n = 0;; n++) {
      if(key[n] == 0) {
        append(substr(base, 0, p - base));
        p += n;
        base = p;
        break;
      }
      if(!chrequal<Insensitive>(key[n], p[n])) { p++; break; }
    }
  }

  append(base);
  return *this;
}

template<unsigned Limit> lstring& lstring::split(const char *key, const char *src) { return usplit<Limit, false, false>(key, src); }
template<unsigned Limit> lstring& lstring::isplit(const char *key, const char *src) { return usplit<Limit, true, false>(key, src); }
template<unsigned Limit> lstring& lstring::qsplit(const char *key, const char *src) { return usplit<Limit, false, true>(key, src); }
template<unsigned Limit> lstring& lstring::iqsplit(const char *key, const char *src) { return usplit<Limit, true, true>(key, src); }

};

#endif
