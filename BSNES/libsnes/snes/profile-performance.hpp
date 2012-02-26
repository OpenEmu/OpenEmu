namespace Info {
  static const char Profile[] = "Performance";
}

#if defined(DEBUGGER)
  #error "bsnes: debugger not supported with performance profile."
#endif

#include <snes/alt/cpu/cpu.hpp>
#include <snes/alt/smp/smp.hpp>
#include <snes/alt/dsp/dsp.hpp>
#include <snes/alt/ppu-performance/ppu.hpp>
