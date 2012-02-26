#define LIBSNES_CORE
#include "libsnes.hpp"
#include <snes/snes.hpp>

#include <nall/snes/cartridge.hpp>
#include <nall/gameboy/cartridge.hpp>
using namespace nall;

struct Interface : public SNES::Interface {
  snes_video_refresh_t pvideo_refresh;
  snes_audio_sample_t paudio_sample;
  snes_input_poll_t pinput_poll;
  snes_input_state_t pinput_state;
  string basename;
  uint16_t *buffer;
  SNES::Video video;

  void videoRefresh(const uint32_t *data, bool hires, bool interlace, bool overscan) {
    unsigned width = hires ? 512 : 256;
    unsigned height = overscan ? 239 : 224;
    unsigned pitch = 1024 >> interlace;
    if(interlace) height <<= 1;
    data += 9 * 1024;  //skip front porch

    for(unsigned y = 0; y < height; y++) {
      const uint32_t *sp = data + y * pitch;
      uint16_t *dp = buffer + y * pitch;
      for(unsigned x = 0; x < width; x++) {
        *dp++ = video.palette[*sp++];
      }
    }

    if(pvideo_refresh) pvideo_refresh(buffer, width, height);
    if(pinput_poll) pinput_poll();
  }

  void audioSample(int16_t left, int16_t right) {
    if(paudio_sample) return paudio_sample(left, right);
  }

  int16_t inputPoll(bool port, SNES::Input::Device::e device, unsigned index, unsigned id) {
    if(id > 11) return 0;
    if(pinput_state) return pinput_state(port, (unsigned)device, index, id);
    return 0;
  }

  void message(const string &text) {
    print(text, "\n");
  }

  string path(SNES::Cartridge::Slot::e slot, const string &hint) {
    return string( basename, hint );
  }

  Interface() : pvideo_refresh(0), paudio_sample(0), pinput_poll(0), pinput_state(0) {
    buffer = new uint16_t[512 * 480];
    video.generate(SNES::Video::Format::RGB15);
  }

  void setCheats(const lstring &list = lstring()) {
    if(SNES::cartridge.mode.i == SNES::Cartridge::Mode::SuperGameBoy) {
      GameBoy::cheat.reset();
      foreach(code, list) {
        lstring codelist;
        codelist.split("+", code);
        foreach(part, codelist) {
          unsigned addr, data, comp;
          if(GameBoy::Cheat::decode(part, addr, data, comp)) {
            GameBoy::CheatCode code_ = { addr, data, comp };
            GameBoy::cheat.append(code_);
          }
        }
      }
      GameBoy::cheat.synchronize();
      return;
    }

    SNES::cheat.reset();
    foreach(code, list) {
      lstring codelist;
      codelist.split("+", code);
      foreach(part, codelist) {
        unsigned addr, data;
        if(SNES::Cheat::decode(part, addr, data)) {
          SNES::CheatCode code_ = { addr, data };
          SNES::cheat.append(code_);
        }
      }
    }
    SNES::cheat.synchronize();
  }

  ~Interface() {
    delete[] buffer;
  }
};

static Interface interface;
static snes_environment_t environment_cb;

EXPORT const char* snes_library_id(void) {
  static string version("bsnes v085 (", SNES::Info::Profile, ")");
  return version;
}

EXPORT unsigned snes_library_revision_major(void) {
  return 1;
}

EXPORT unsigned snes_library_revision_minor(void) {
  return 3;
}

EXPORT void snes_set_environment(snes_environment_t environ_cb) {
  environment_cb = environ_cb;
}

EXPORT void snes_set_video_refresh(snes_video_refresh_t video_refresh) {
  interface.pvideo_refresh = video_refresh;
}

EXPORT void snes_set_audio_sample(snes_audio_sample_t audio_sample) {
  interface.paudio_sample = audio_sample;
}

EXPORT void snes_set_input_poll(snes_input_poll_t input_poll) {
  interface.pinput_poll = input_poll;
}

EXPORT void snes_set_input_state(snes_input_state_t input_state) {
  interface.pinput_state = input_state;
}

EXPORT void snes_set_controller_port_device(bool port, unsigned device) {
  SNES::input.connect(port, (SNES::Input::Device::e)device);
}

EXPORT void snes_set_cartridge_basename(const char *basename) {
  interface.basename = basename;
}

EXPORT void snes_init(void) {
  SNES::interface = &interface;
  SNES::system.init();
  SNES::input.connect(SNES::Controller::Port1, SNES::Input::Device::Joypad);
  SNES::input.connect(SNES::Controller::Port2, SNES::Input::Device::Joypad);
}

EXPORT void snes_term(void) {
  SNES::system.term();
}

EXPORT void snes_power(void) {
  SNES::system.power();
}

EXPORT void snes_reset(void) {
  SNES::system.reset();
}

EXPORT void snes_run(void) {
  SNES::system.run();
}

EXPORT unsigned snes_serialize_size(void) {
  return SNES::system.serialize_size;
}

EXPORT bool snes_serialize(uint8_t *data, unsigned size) {
  SNES::system.runtosave();
  serializer s = SNES::system.serialize();
  if(s.size() > size) return false;
  memcpy(data, s.data(), s.size());
  return true;
}

EXPORT bool snes_unserialize(const uint8_t *data, unsigned size) {
  serializer s(data, size);
  return SNES::system.unserialize(s);
}

struct CheatList {
  bool enable;
  string code;
  CheatList() : enable(false) {}
};

static linear_vector<CheatList> cheatList;

EXPORT void snes_cheat_reset(void) {
  cheatList.reset();
  interface.setCheats();
}

EXPORT void snes_cheat_set(unsigned index, bool enable, const char *code) {
  cheatList[index].enable = enable;
  cheatList[index].code = code;
  lstring list;
  for(unsigned n = 0; n < cheatList.size(); n++) {
    if(cheatList[n].enable) list.append(cheatList[n].code);
  }
  interface.setCheats(list);
}

static void set_timing() {
  if (environment_cb) {
    snes_system_timing timing = { 0.0, 32040.5 };
    timing.fps = snes_get_region() == SNES_REGION_NTSC ? 21477272.0 / 357366.0 : 21281370.0 / 425568.0;
    environment_cb(SNES_ENVIRONMENT_SET_TIMING, &timing);
  }
}

EXPORT bool snes_load_cartridge_normal(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size
) {
  snes_cheat_reset();
  if(rom_data) SNES::cartridge.rom.copy(rom_data, rom_size);
  string xmlrom = (rom_xml && *rom_xml) ? string(rom_xml) : SnesCartridge(rom_data, rom_size).markup;
  SNES::cartridge.load(SNES::Cartridge::Mode::Normal, xmlrom);
  SNES::system.power();
  set_timing();
  return true;
}

EXPORT bool snes_load_cartridge_bsx_slotted(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *bsx_xml, const uint8_t *bsx_data, unsigned bsx_size
) {
  snes_cheat_reset();
  if(rom_data) SNES::cartridge.rom.copy(rom_data, rom_size);
  string xmlrom = (rom_xml && *rom_xml) ? string(rom_xml) : SnesCartridge(rom_data, rom_size).markup;
  if(bsx_data) SNES::bsxflash.memory.copy(bsx_data, bsx_size);
  string xmlbsx = (bsx_xml && *bsx_xml) ? string(bsx_xml) : SnesCartridge(bsx_data, bsx_size).markup;
  SNES::cartridge.load(SNES::Cartridge::Mode::BsxSlotted, xmlrom);
  SNES::system.power();
  set_timing();
  return true;
}

EXPORT bool snes_load_cartridge_bsx(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *bsx_xml, const uint8_t *bsx_data, unsigned bsx_size
) {
  snes_cheat_reset();
  if(rom_data) SNES::cartridge.rom.copy(rom_data, rom_size);
  string xmlrom = (rom_xml && *rom_xml) ? string(rom_xml) : SnesCartridge(rom_data, rom_size).markup;
  if(bsx_data) SNES::bsxflash.memory.copy(bsx_data, bsx_size);
  string xmlbsx = (bsx_xml && *bsx_xml) ? string(bsx_xml) : SnesCartridge(bsx_data, bsx_size).markup;
  SNES::cartridge.load(SNES::Cartridge::Mode::Bsx, xmlrom);
  SNES::system.power();
  set_timing();
  return true;
}

EXPORT bool snes_load_cartridge_sufami_turbo(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *sta_xml, const uint8_t *sta_data, unsigned sta_size,
  const char *stb_xml, const uint8_t *stb_data, unsigned stb_size
) {
  snes_cheat_reset();
  if(rom_data) SNES::cartridge.rom.copy(rom_data, rom_size);
  string xmlrom = (rom_xml && *rom_xml) ? string(rom_xml) : SnesCartridge(rom_data, rom_size).markup;
  if(sta_data) SNES::sufamiturbo.slotA.rom.copy(sta_data, sta_size);
  string xmlsta = (sta_xml && *sta_xml) ? string(sta_xml) : SnesCartridge(sta_data, sta_size).markup;
  if(stb_data) SNES::sufamiturbo.slotB.rom.copy(stb_data, stb_size);
  string xmlstb = (stb_xml && *stb_xml) ? string(stb_xml) : SnesCartridge(stb_data, stb_size).markup;
  SNES::cartridge.load(SNES::Cartridge::Mode::SufamiTurbo, xmlrom);
  SNES::system.power();
  set_timing();
  return true;
}

EXPORT bool snes_load_cartridge_super_game_boy(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *dmg_xml, const uint8_t *dmg_data, unsigned dmg_size
) {
  snes_cheat_reset();
  if(rom_data) SNES::cartridge.rom.copy(rom_data, rom_size);
  string xmlrom = (rom_xml && *rom_xml) ? string(rom_xml) : SnesCartridge(rom_data, rom_size).markup;
  if(dmg_data) {
    //GameBoyCartridge needs to modify dmg_data (for MMM01 emulation); so copy data
    uint8_t *data = new uint8_t[dmg_size];
    memcpy(data, dmg_data, dmg_size);
    string xmldmg = (dmg_xml && *dmg_xml) ? string(dmg_xml) : GameBoyCartridge(data, dmg_size).markup;
    GameBoy::cartridge.load(GameBoy::System::Revision::SuperGameBoy, xmldmg, data, dmg_size);
    delete[] data;
  }
  SNES::cartridge.load(SNES::Cartridge::Mode::SuperGameBoy, xmlrom);
  SNES::system.power();
  set_timing();
  return true;
}

EXPORT void snes_unload_cartridge(void) {
  SNES::cartridge.unload();
}

EXPORT bool snes_get_region(void) {
  return SNES::system.region.i == SNES::System::Region::NTSC ? 0 : 1;
}

EXPORT uint8_t* snes_get_memory_data(unsigned id) {
  if(SNES::cartridge.loaded() == false) return 0;

  switch(id) {
    case SNES_MEMORY_CARTRIDGE_RAM:
      return SNES::cartridge.ram.data();
    case SNES_MEMORY_CARTRIDGE_RTC:
      if(SNES::cartridge.has_srtc()) return SNES::srtc.rtc;
      if(SNES::cartridge.has_spc7110rtc()) return SNES::spc7110.rtc;
      return 0;
    case SNES_MEMORY_BSX_RAM:
      if(SNES::cartridge.mode.i != SNES::Cartridge::Mode::Bsx) break;
      return SNES::bsxcartridge.sram.data();
    case SNES_MEMORY_BSX_PRAM:
      if(SNES::cartridge.mode.i != SNES::Cartridge::Mode::Bsx) break;
      return SNES::bsxcartridge.psram.data();
    case SNES_MEMORY_SUFAMI_TURBO_A_RAM:
      if(SNES::cartridge.mode.i != SNES::Cartridge::Mode::SufamiTurbo) break;
      return SNES::sufamiturbo.slotA.ram.data();
    case SNES_MEMORY_SUFAMI_TURBO_B_RAM:
      if(SNES::cartridge.mode.i != SNES::Cartridge::Mode::SufamiTurbo) break;
      return SNES::sufamiturbo.slotB.ram.data();
    case SNES_MEMORY_GAME_BOY_RAM:
      if(SNES::cartridge.mode.i != SNES::Cartridge::Mode::SuperGameBoy) break;
      return GameBoy::cartridge.ramdata;
  //case SNES_MEMORY_GAME_BOY_RTC:
  //  if(SNES::cartridge.mode.i != SNES::Cartridge::Mode::SuperGameBoy) break;
  //  return GameBoy::cartridge.rtcdata;

    case SNES_MEMORY_WRAM:
      return SNES::cpu.wram;
    case SNES_MEMORY_APURAM:
      return SNES::smp.apuram;
    case SNES_MEMORY_VRAM:
      return SNES::ppu.vram;
    case SNES_MEMORY_OAM:
      return SNES::ppu.oam;
    case SNES_MEMORY_CGRAM:
      return SNES::ppu.cgram;
  }

  return 0;
}

EXPORT unsigned snes_get_memory_size(unsigned id) {
  if(SNES::cartridge.loaded() == false) return 0;
  unsigned size = 0;

  switch(id) {
    case SNES_MEMORY_CARTRIDGE_RAM:
      size = SNES::cartridge.ram.size();
      break;
    case SNES_MEMORY_CARTRIDGE_RTC:
      if(SNES::cartridge.has_srtc() || SNES::cartridge.has_spc7110rtc()) size = 20;
      break;
    case SNES_MEMORY_BSX_RAM:
      if(SNES::cartridge.mode.i != SNES::Cartridge::Mode::Bsx) break;
      size = SNES::bsxcartridge.sram.size();
      break;
    case SNES_MEMORY_BSX_PRAM:
      if(SNES::cartridge.mode.i != SNES::Cartridge::Mode::Bsx) break;
      size = SNES::bsxcartridge.psram.size();
      break;
    case SNES_MEMORY_SUFAMI_TURBO_A_RAM:
      if(SNES::cartridge.mode.i != SNES::Cartridge::Mode::SufamiTurbo) break;
      size = SNES::sufamiturbo.slotA.ram.size();
      break;
    case SNES_MEMORY_SUFAMI_TURBO_B_RAM:
      if(SNES::cartridge.mode.i != SNES::Cartridge::Mode::SufamiTurbo) break;
      size = SNES::sufamiturbo.slotB.ram.size();
      break;
    case SNES_MEMORY_GAME_BOY_RAM:
      if(SNES::cartridge.mode.i != SNES::Cartridge::Mode::SuperGameBoy) break;
      size = GameBoy::cartridge.ramsize;
      break;
  //case SNES_MEMORY_GAME_BOY_RTC:
  //  if(SNES::cartridge.mode.i != SNES::Cartridge::Mode::SuperGameBoy) break;
  //  size = GameBoy::cartridge.rtcsize;
  //  break;

    case SNES_MEMORY_WRAM:
      size = 128 * 1024;
      break;
    case SNES_MEMORY_APURAM:
      size = 64 * 1024;
      break;
    case SNES_MEMORY_VRAM:
      size = 64 * 1024;
      break;
    case SNES_MEMORY_OAM:
      size = 544;
      break;
    case SNES_MEMORY_CGRAM:
      size = 512;
      break;
  }

  if(size == -1U) size = 0;
  return size;
}
