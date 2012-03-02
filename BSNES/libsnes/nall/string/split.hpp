#ifndef NALL_STRING_SPLIT_HPP
#define NALL_STRING_SPLIT_HPP

namespace nall {

void lstring::split(const char *key, const char *src, unsigned limit) {
  reset();

  int ssl = strlen(src), ksl = strlen(key);
  int lp = 0, split_count = 0;

  for(int i = 0; i <= ssl - ksl;) {
    if(!memcmp(src + i, key, ksl)) {
      strlcpy(operator[](split_count++), src + lp, i - lp + 1);
      i += ksl;
      lp = i;
      if(!--limit) break;
    } else i++;
  }

  operator[](split_count++) = src + lp;
}

void lstring::qsplit(const char *key, const char *src, unsigned limit) {
  reset();

  int ssl = strlen(src), ksl = strlen(key);
  int lp = 0, split_count = 0;

  for(int i = 0; i <= ssl - ksl;) {
    uint8_t x = src[i];

    if(x == '\"' || x == '\'') {
      int z = i++;                        //skip opening quote
      while(i < ssl && src[i] != x) i++;
      if(i >= ssl) i = z;                 //failed match, rewind i
      else {
        i++;                              //skip closing quote
        continue;                         //restart in case next char is also a quote
      }
    }

    if(!memcmp(src + i, key, ksl)) {
      strlcpy(operator[](split_count++), src + lp, i - lp + 1);
      i += ksl;
      lp = i;
      if(!--limit) break;
    } else i++;
  }

  operator[](split_count++) = src + lp;
}

};

#endif
