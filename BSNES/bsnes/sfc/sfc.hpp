#ifndef SFC_HPP
#define SFC_HPP

#include <emulator/emulator.hpp>
#include <processor/arm/arm.hpp>
#include <processor/gsu/gsu.hpp>
#include <processor/hg51b/hg51b.hpp>
#include <processor/r65816/r65816.hpp>
#include <processor/spc700/spc700.hpp>
#include <processor/upd96050/upd96050.hpp>

namespace SuperFamicom {
  namespace Info {
    static const char Name[] = "bsnes";
    static const unsigned SerializerVersion = 25;
  }
}

/*
  bsnes - Super Famicom emulator
  author: byuu
  license: GPLv3
  project started: 2004-10-14
*/

#include <libco/libco.h>
#include <gb/gb.hpp>

namespace SuperFamicom {
  struct Thread {
    cothread_t thread;
    unsigned frequency;
    int64 clock;

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

  #include <sfc/memory/memory.hpp>
  #include <sfc/ppu/counter/counter.hpp>

  #if defined(PROFILE_ACCURACY)
  #include "profile-accuracy.hpp"
  #elif defined(PROFILE_COMPATIBILITY)
  #include "profile-compatibility.hpp"
  #elif defined(PROFILE_PERFORMANCE)
  #include "profile-performance.hpp"
  #endif

  #include <sfc/controller/controller.hpp>
  #include <sfc/system/system.hpp>
  #include <sfc/chip/chip.hpp>
  #include <sfc/cartridge/cartridge.hpp>
  #include <sfc/cheat/cheat.hpp>
  #include <sfc/interface/interface.hpp>

  #include <sfc/memory/memory-inline.hpp>
  #include <sfc/ppu/counter/counter-inline.hpp>
}

#endif
