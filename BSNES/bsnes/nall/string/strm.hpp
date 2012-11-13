#ifdef NALL_STRING_INTERNAL_HPP

namespace nall {

//
//strmcpy, strmcat created by byuu
//

//return = strlen(target)
unsigned strmcpy(char *target, const char *source, unsigned length) {
  const char *origin = target;
  if(length) {
    while(*source && --length) *target++ = *source++;
    *target = 0;
  }
  return target - origin;
}

//return = strlen(target)
unsigned strmcat(char *target, const char *source, unsigned length) {
  const char *origin = target;
  while(*target && length) target++, length--;
  return (target - origin) + strmcpy(target, source, length);
}

//return = true when all of source was copied
bool strccpy(char *target, const char *source, unsigned length) {
  return !source[strmcpy(target, source, length)];
}

//return = true when all of source was copied
bool strccat(char *target, const char *source, unsigned length) {
  while(*target && length) target++, length--;
  return !source[strmcpy(target, source, length)];
}

//return = reserved for future use
void strpcpy(char *&target, const char *source, unsigned &length) {
  unsigned offset = strmcpy(target, source, length);
  target += offset, length -= offset;
}

}

#endif
