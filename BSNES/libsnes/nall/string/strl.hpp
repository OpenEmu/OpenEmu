#ifndef NALL_STRING_STRL_HPP
#define NALL_STRING_STRL_HPP

namespace nall {

//strlcpy, strlcat based on OpenBSD implementation by Todd C. Miller

//return = strlen(src)
inline unsigned strlcpy(char *dest, const char *src, unsigned length) {
  char *d = dest;
  const char *s = src;
  unsigned n = length;

  if(n) {
    while(--n && (*d++ = *s++)); //copy as many bytes as possible, or until null terminator reached
  }

  if(!n) {
    if(length) *d = 0;
    while(*s++); //traverse rest of s, so that s - src == strlen(src)
  }

  return (s - src - 1); //return length of copied string, sans null terminator
}

//return = strlen(src) + min(length, strlen(dest))
inline unsigned strlcat(char *dest, const char *src, unsigned length) {
  char *d = dest;
  const char *s = src;
  unsigned n = length;

  while(n-- && *d) d++; //find end of dest
  unsigned dlength = d - dest;
  n = length - dlength; //subtract length of dest from maximum string length

  if(!n) return dlength + strlen(s);

  while(*s) {
    if(n != 1) {
      *d++ = *s;
      n--;
    }
    s++;
  }
  *d = 0;

  return dlength + (s - src); //return length of resulting string, sans null terminator
}

}

#endif
