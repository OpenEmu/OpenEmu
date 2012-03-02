#ifndef NALL_STRING_VARIADIC_HPP
#define NALL_STRING_VARIADIC_HPP

namespace nall {

template <typename T1>
inline void print(T1 const & v1)
{ printf("%s", (const char*)string(v1)); }

template <typename T1,
          typename T2>
inline void print(T1 const & v1,
                  T2 const & v2)
{ printf("%s", (const char*)string(v1, v2)); }

template <typename T1,
          typename T2,
          typename T3>
inline void print(T1 const & v1,
                  T2 const & v2,
                  T3 const & v3)
{ printf("%s", (const char*)string(v1, v2, v3)); }

template <typename T1,
          typename T2,
          typename T3,
          typename T4>
inline void print(T1 const & v1,
                  T2 const & v2,
                  T3 const & v3,
                  T4 const & v4)
{ printf("%s", (const char*)string(v1, v2, v3, v4)); }

template <typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5>
inline void print(T1 const & v1,
                  T2 const & v2,
                  T3 const & v3,
                  T4 const & v4,
                  T5 const & v5)
{ printf("%s", (const char*)string(v1, v2, v3, v4, v5)); }

template <typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename T6>
inline void print(T1 const & v1,
                  T2 const & v2,
                  T3 const & v3,
                  T4 const & v4,
                  T5 const & v5,
                  T6 const & v6)
{ printf("%s", (const char*)string(v1, v2, v3, v4, v5, v6)); }

template <typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename T6,
          typename T7>
inline void print(T1 const & v1,
                  T2 const & v2,
                  T3 const & v3,
                  T4 const & v4,
                  T5 const & v5,
                  T6 const & v6,
                  T7 const & v7)
{ printf("%s", (const char*)string(v1, v2, v3, v4, v5, v6, v7)); }

template <typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename T6,
          typename T7,
          typename T8>
inline void print(T1 const & v1,
                  T2 const & v2,
                  T3 const & v3,
                  T4 const & v4,
                  T5 const & v5,
                  T6 const & v6,
                  T7 const & v7,
                  T8 const & v8)
{ printf("%s", (const char*)string(v1, v2, v3, v4, v5, v6, v7, v8)); }

}

#endif
