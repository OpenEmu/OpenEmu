/*
  libco
  auto-selection module
  license: public domain
*/

#if defined(__GNUC__) && defined(__i386__)
  #include "x86.c"
#elif defined(__GNUC__) && defined(__amd64__)
  #include "amd64.c"
#elif defined(_MSC_VER) && defined(_WIN64)
  #include "amd64_msvc.c"
#elif defined(_MSC_VER) && defined(_WIN32)
  #include "x86_msvc.c"
#elif defined(__GNUC__) && (defined(__powerpc__) || defined(__POWERPC__)) // Seems to run fine on PPC G5 OSX even when no __ELF__ is defined.
  #include "ppc.c"
#elif defined(__GNUC__)
  #include "sjlj.c"
#elif defined(_MSC_VER) && defined(_M_IX86)
  #include "x86.c"
#elif defined(_MSC_VER) && defined(_M_AMD64)
  #include "amd64.c"
#elif defined(_MSC_VER)
  #include "fiber.c"
#else
  #error "libco: unsupported processor, compiler or operating system"
#endif

