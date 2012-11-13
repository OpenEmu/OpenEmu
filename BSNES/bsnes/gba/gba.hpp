#ifndef GBA_HPP
#define GBA_HPP

#include <emulator/emulator.hpp>
#include <processor/arm/arm.hpp>

namespace GameBoyAdvance {
  namespace Info {
    static const char Name[] = "bgba";
    static const unsigned SerializerVersion = 2;
  }
}

/*
  bgba - Game Boy Advance emulator
  authors: byuu, Cydrak
  license: GPLv3
  project started: 2012-03-19
*/

#include <libco/libco.h>

namespace GameBoyAdvance {
  enum : unsigned { Byte = 8, Half = 16, Word = 32 };

  struct Thread {
    cothread_t thread;
    unsigned frequency;
    signed clock;

    inline void create(void (*entrypoint)(), unsigned frequency) {
      if(thread) co_delete(thread);
      thread = co_create(65536 * sizeof(void*), entrypoint);
      this->frequency = frequency;
      clock = 0;
    }

    inline void serialize(serializer &s) {
      s.integer(frequency);
      s.integer(clock);
    }

    inline Thread() : thread(nullptr) {
    }

    inline ~Thread() {
      if(thread) co_delete(thread);
    }
  };

  #include <gba/memory/memory.hpp>
  #include <gba/interface/interface.hpp>
  #include <gba/scheduler/scheduler.hpp>
  #include <gba/system/system.hpp>
  #include <gba/cartridge/cartridge.hpp>
  #include <gba/cpu/cpu.hpp>
  #include <gba/ppu/ppu.hpp>
  #include <gba/apu/apu.hpp>
  #include <gba/video/video.hpp>
}

#endif
