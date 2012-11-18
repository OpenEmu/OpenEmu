#ifndef NALL_ATOI_HPP
#define NALL_ATOI_HPP

namespace nall {

//note: this header is intended to form the base for user-defined literals;
//once they are supported by GCC. eg:
//unsigned operator "" b(const char *s) { return binary(s); }
//-> signed data = 1001b;
//(0b1001 is nicer, but is not part of the C++ standard)

constexpr inline uintmax_t binary_(const char *s, uintmax_t sum = 0) {
  return (
    *s == '0' || *s == '1' ? binary_(s + 1, (sum << 1) | *s - '0') :
    sum
  );
}

constexpr inline uintmax_t octal_(const char *s, uintmax_t sum = 0) {
  return (
    *s >= '0' && *s <= '7' ? octal_(s + 1, (sum << 3) | *s - '0') :
    sum
  );
}

constexpr inline uintmax_t decimal_(const char *s, uintmax_t sum = 0) {
  return (
    *s >= '0' && *s <= '9' ? decimal_(s + 1, (sum * 10) + *s - '0') :
    sum
  );
}

constexpr inline uintmax_t hex_(const char *s, uintmax_t sum = 0) {
  return (
    *s >= 'A' && *s <= 'F' ? hex_(s + 1, (sum << 4) | *s - 'A' + 10) :
    *s >= 'a' && *s <= 'f' ? hex_(s + 1, (sum << 4) | *s - 'a' + 10) :
    *s >= '0' && *s <= '9' ? hex_(s + 1, (sum << 4) | *s - '0') :
    sum
  );
}

//

constexpr inline uintmax_t binary(const char *s) {
  return (
    *s == '0' && *(s + 1) == 'B' ? binary_(s + 2) :
    *s == '0' && *(s + 1) == 'b' ? binary_(s + 2) :
    *s == '%' ? binary_(s + 1) :
    binary_(s)
  );
}

constexpr inline uintmax_t octal(const char *s) {
  return (
    octal_(s)
  );
}

constexpr inline intmax_t integer(const char *s) {
  return (
    *s == '+' ? +decimal_(s + 1) :
    *s == '-' ? -decimal_(s + 1) :
    decimal_(s)
  );
}

constexpr inline uintmax_t decimal(const char *s) {
  return (
    decimal_(s)
  );
}

constexpr inline uintmax_t hex(const char *s) {
  return (
    *s == '0' && *(s + 1) == 'X' ? hex_(s + 2) :
    *s == '0' && *(s + 1) == 'x' ? hex_(s + 2) :
    *s == '$' ? hex_(s + 1) :
    hex_(s)
  );
}

constexpr inline intmax_t numeral(const char *s) {
  return (
    *s == '0' && *(s + 1) == 'X' ? hex_(s + 2) :
    *s == '0' && *(s + 1) == 'x' ? hex_(s + 2) :
    *s == '0' && *(s + 1) == 'B' ? binary_(s + 2) :
    *s == '0' && *(s + 1) == 'b' ? binary_(s + 2) :
    *s == '0' ? octal_(s + 1) :
    *s == '+' ? +decimal_(s + 1) :
    *s == '-' ? -decimal_(s + 1) :
    decimal_(s)
  );
}

inline double fp(const char *s) {
  return atof(s);
}

}

#endif
