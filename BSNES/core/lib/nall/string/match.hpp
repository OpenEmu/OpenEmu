#ifndef NALL_STRING_MATCH_HPP
#define NALL_STRING_MATCH_HPP

bool match(const char *p, const char *s) {
  const char *p_ = 0, *s_ = 0;

  for(;;) {
    if(!*s) {
      while(*p == '*') p++;
      return !*p;
    }

    //wildcard match
    if(*p == '*') {
      p_ = p++, s_ = s;
      continue;
    }

    //any match
    if(*p == '?') {
      p++, s++;
      continue;
    }

    //ranged match
    if(*p == '{') {
      #define pattern(name_, rule_) \
        if(strbegin(p, name_)) { \
          if(rule_) { \
            p += sizeof(name_) - 1, s++; \
            continue; \
          } \
          goto failure; \
        }

      pattern("{alpha}",        (*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z'))
      pattern("{alphanumeric}", (*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z') || (*s >= '0' && *s <= '9'))
      pattern("{binary}",       (*s == '0' || *s == '1'))
      pattern("{hex}",          (*s >= '0' && *s <= '9') || (*s >= 'A' && *s <= 'F') || (*s >= 'a' && *s <= 'f'))
      pattern("{lowercase}",    (*s >= 'a' && *s <= 'z'))
      pattern("{numeric}",      (*s >= '0' && *s <= '9'))
      pattern("{uppercase}",    (*s >= 'A' && *s <= 'Z'))
      pattern("{whitespace}",   (*s == ' ' || *s == '\t'))

      #undef pattern
      goto failure;
    }

    //reserved character match
    if(*p == '\\') {
      p++;
    //fallthrough
    }

    //literal match
    if(*p == *s) {
      p++, *s++;
      continue;
    }

    //attempt wildcard rematch
    failure:
    if(p_) {
      p = p_, s = s_ + 1;
      continue;
    }

    return false;
  }
}

#endif
