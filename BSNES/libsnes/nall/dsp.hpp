#ifndef NALL_DSP_HPP
#define NALL_DSP_HPP

#include <algorithm>
#ifdef __SSE__
  #include <xmmintrin.h>
#endif

#define NALL_DSP_INTERNAL_HPP
#include <nall/dsp/core.hpp>
#undef NALL_DSP_INTERNAL_HPP

#endif
