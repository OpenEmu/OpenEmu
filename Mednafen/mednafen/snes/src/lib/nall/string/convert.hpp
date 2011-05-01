#ifndef NALL_STRING_CONVERT_HPP
#define NALL_STRING_CONVERT_HPP

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

uintmax_t strhex(const char *str) {
  if(!str) return 0;
  uintmax_t result = 0;

  //skip hex identifiers 0x and $, if present
  if(*str == '0' && (*(str + 1) == 'X' || *(str + 1) == 'x')) str += 2;
  else if(*str == '$') str++;

  while(*str) {
    uint8_t x = *str++;
    if(x >= '0' && x <= '9') x -= '0';
    else if(x >= 'A' && x <= 'F') x -= 'A' - 10;
    else if(x >= 'a' && x <= 'f') x -= 'a' - 10;
    else break;  //stop at first invalid character
    result = result * 16 + x;
  }

  return result;
}

intmax_t strsigned(const char *str) {
  if(!str) return 0;
  intmax_t result = 0;
  bool negate = false;

  //check for negation
  if(*str == '-') {
    negate = true;
    str++;
  }

  while(*str) {
    uint8_t x = *str++;
    if(x >= '0' && x <= '9') x -= '0';
    else break;  //stop at first invalid character
    result = result * 10 + x;
  }

  return !negate ? result : -result;
}

uintmax_t strunsigned(const char *str) {
  if(!str) return 0;
  uintmax_t result = 0;

  while(*str) {
    uint8_t x = *str++;
    if(x >= '0' && x <= '9') x -= '0';
    else break;  //stop at first invalid character
    result = result * 10 + x;
  }

  return result;
}

uintmax_t strbin(const char *str) {
  if(!str) return 0;
  uintmax_t result = 0;

  //skip bin identifiers 0b and %, if present
  if(*str == '0' && (*(str + 1) == 'B' || *(str + 1) == 'b')) str += 2;
  else if(*str == '%') str++;

  while(*str) {
    uint8_t x = *str++;
    if(x == '0' || x == '1') x -= '0';
    else break;  //stop at first invalid character
    result = result * 2 + x;
  }

  return result;
}

double strdouble(const char *str) {
  if(!str) return 0.0;
  bool negate = false;

  //check for negation
  if(*str == '-') {
    negate = true;
    str++;
  }

  intmax_t result_integral = 0;
  while(*str) {
    uint8_t x = *str++;
    if(x >= '0' && x <= '9') x -= '0';
    else if(x == '.') break;  //break loop and read fractional part
    else return (double)result_integral;  //invalid value, assume no fractional part
    result_integral = result_integral * 10 + x;
  }

  intmax_t result_fractional = 0;
  while(*str) {
    uint8_t x = *str++;
    if(x >= '0' && x <= '9') x -= '0';
    else break;  //stop at first invalid character
    result_fractional = result_fractional * 10 + x;
  }

  //calculate fractional portion
  double result = (double)result_fractional;
  while((uintmax_t)result > 0) result /= 10.0;
  result += (double)result_integral;

  return !negate ? result : -result;
}

//

size_t strhex(char *str, uintmax_t value, size_t length /* = 0 */) {
  if(length == 0) length -= 1U;  //"infinite" length
  size_t initial_length = length;

  //count number of digits in value
  int digits_integral = 1;
  uintmax_t digits_integral_ = value;
  while(digits_integral_ /= 16) digits_integral++;

  int digits = digits_integral;
  if(!str) return digits + 1;  //only computing required length?

  length = nall::min(digits, length - 1);
  str += length;  //seek to end of target string
  *str = 0;  //set null terminator

  while(length--) {
    uint8_t x = value % 16;
    value /= 16;
    *--str = x < 10 ? (x + '0') : (x + 'a' - 10);  //iterate backwards to write string
  }

  return nall::min(initial_length, digits + 1);
}

size_t strsigned(char *str, intmax_t value_, size_t length /* = 0 */) {
  if(length == 0) length = -1U;  //"infinite" length
  size_t initial_length = length;

  bool negate = value_ < 0;
  uintmax_t value = value_ >= 0 ? value_ : -value_;

  //count number of digits in value
  int digits_integral = 1;
  uintmax_t digits_integral_ = value;
  while(digits_integral_ /= 10) digits_integral++;

  int digits = (negate ? 1 : 0) + digits_integral;
  if(!str) return digits + 1;  //only computing required length?

  length = nall::min(digits, length - 1);
  str += length;  //seek to end of target string
  *str = 0;  //set null terminator
  while(length && digits_integral--) {
    uint8_t x = '0' + (value % 10);
    value /= 10;
    *--str = x;  //iterate backwards to write string
    length--;
  }

  if(length && negate) {
    *--str = '-';
  }

  return nall::min(initial_length, digits + 1);
}

size_t strunsigned(char *str, uintmax_t value, size_t length /* = 0 */) {
  if(length == 0) length = -1U;  //"infinite" length
  size_t initial_length = length;

  //count number of digits in value
  int digits_integral = 1;
  uintmax_t digits_integral_ = value;
  while(digits_integral_ /= 10) digits_integral++;

  int digits = digits_integral;
  if(!str) return digits_integral + 1;  //only computing required length?

  length = nall::min(digits, length - 1);
  str += length;  //seek to end of target string
  *str = 0;  //set null terminator

  while(length--) {
    uint8_t x = '0' + (value % 10);
    value /= 10;
    *--str = x;  //iterate backwards to write string
  }

  return nall::min(initial_length, digits + 1);
}

size_t strbin(char *str, uintmax_t value, size_t length /* = 0 */) {
  if(length == 0) length = -1U;  //"infinite" length
  size_t initial_length = length;

  //count number of digits in value
  int digits_integral = 1;
  uintmax_t digits_integral_ = value;
  while(digits_integral_ /= 2) digits_integral++;

  int digits = digits_integral;
  if(!str) return digits + 1;  //only computing required length?

  length = nall::min(digits, length - 1);
  str += length;  //seek to end of target string
  *str = 0;  //set null terminator

  while(length--) {
    uint8_t x = '0' + (value % 2);
    value /= 2;
    *--str = x;  //iterate backwards to write string
  }

  return nall::min(initial_length, digits + 1);
}

//using sprintf is certainly not the most ideal method to convert
//a double to a string ... but attempting to parse a double by
//hand, digit-by-digit, results in subtle rounding errors.
//
//note: length parameter is currently ignored.
//it remains for consistency and possible future support.
size_t strdouble(char *str, double value, size_t length /* = 0 */) {
  char buffer[256];
  sprintf(buffer, "%f", value);

  //remove excess 0's in fraction (2.500000 -> 2.5)
  for(char *p = buffer; *p; p++) {
    if(*p == '.') {
      char *p = buffer + strlen(buffer) - 1;
      while(*p == '0') {
        if(*(p - 1) != '.') *p = 0;  //... but not for eg 1.0 -> 1.
        p--;
      }
      break;
    }
  }

  length = strlen(buffer);
  if(str) strcpy(str, buffer);
  return length + 1;
}

#endif
