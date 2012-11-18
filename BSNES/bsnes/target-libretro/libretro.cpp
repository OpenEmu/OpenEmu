#include "libretro.h"
#include <sfc/sfc.hpp>

#include <nall/snes/cartridge.hpp>
#include <nall/gb/cartridge.hpp>
#include <nall/stream/mmap.hpp>
using namespace nall;

const uint8 iplrom[64] = {
/*ffc0*/  0xcd, 0xef,        //mov   x,#$ef
/*ffc2*/  0xbd,              //mov   sp,x
/*ffc3*/  0xe8, 0x00,        //mov   a,#$00
/*ffc5*/  0xc6,              //mov   (x),a
/*ffc6*/  0x1d,              //dec   x
/*ffc7*/  0xd0, 0xfc,        //bne   $ffc5
/*ffc9*/  0x8f, 0xaa, 0xf4,  //mov   $f4,#$aa
/*ffcc*/  0x8f, 0xbb, 0xf5,  //mov   $f5,#$bb
/*ffcf*/  0x78, 0xcc, 0xf4,  //cmp   $f4,#$cc
/*ffd2*/  0xd0, 0xfb,        //bne   $ffcf
/*ffd4*/  0x2f, 0x19,        //bra   $ffef
/*ffd6*/  0xeb, 0xf4,        //mov   y,$f4
/*ffd8*/  0xd0, 0xfc,        //bne   $ffd6
/*ffda*/  0x7e, 0xf4,        //cmp   y,$f4
/*ffdc*/  0xd0, 0x0b,        //bne   $ffe9
/*ffde*/  0xe4, 0xf5,        //mov   a,$f5
/*ffe0*/  0xcb, 0xf4,        //mov   $f4,y
/*ffe2*/  0xd7, 0x00,        //mov   ($00)+y,a
/*ffe4*/  0xfc,              //inc   y
/*ffe5*/  0xd0, 0xf3,        //bne   $ffda
/*ffe7*/  0xab, 0x01,        //inc   $01
/*ffe9*/  0x10, 0xef,        //bpl   $ffda
/*ffeb*/  0x7e, 0xf4,        //cmp   y,$f4
/*ffed*/  0x10, 0xeb,        //bpl   $ffda
/*ffef*/  0xba, 0xf6,        //movw  ya,$f6
/*fff1*/  0xda, 0x00,        //movw  $00,ya
/*fff3*/  0xba, 0xf4,        //movw  ya,$f4
/*fff5*/  0xc4, 0xf4,        //mov   $f4,a
/*fff7*/  0xdd,              //mov   a,y
/*fff8*/  0x5d,              //mov   x,a
/*fff9*/  0xd0, 0xdb,        //bne   $ffd6
/*fffb*/  0x1f, 0x00, 0x00,  //jmp   ($0000+x)
/*fffe*/  0xc0, 0xff         //reset vector location ($ffc0)
};

struct Callbacks : Emulator::Interface::Bind {
  retro_video_refresh_t pvideo_refresh;
  retro_audio_sample_t paudio_sample;
  retro_input_poll_t pinput_poll;
  retro_input_state_t pinput_state;
  retro_environment_t penviron;
  bool overscan;

  Emulator::Interface *iface;
  string basename;

  static unsigned snes_to_retro(unsigned device) {
    switch ((SuperFamicom::Input::Device)device) {
       default:
       case SuperFamicom::Input::Device::None:       return RETRO_DEVICE_NONE;
       case SuperFamicom::Input::Device::Joypad:     return RETRO_DEVICE_JOYPAD;
       case SuperFamicom::Input::Device::Multitap:   return RETRO_DEVICE_JOYPAD_MULTITAP;
       case SuperFamicom::Input::Device::Mouse:      return RETRO_DEVICE_MOUSE;
       case SuperFamicom::Input::Device::SuperScope: return RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE;
       case SuperFamicom::Input::Device::Justifier:  return RETRO_DEVICE_LIGHTGUN_JUSTIFIER;
       case SuperFamicom::Input::Device::Justifiers: return RETRO_DEVICE_LIGHTGUN_JUSTIFIERS;
    }
  }

  // TODO: Properly map Mouse/Lightguns.
  static unsigned snes_to_retro(unsigned, unsigned id) {
    return id;
  }

  static SuperFamicom::Input::Device retro_to_snes(unsigned device) {
    switch (device) {
       default:
       case RETRO_DEVICE_NONE:                 return SuperFamicom::Input::Device::None;
       case RETRO_DEVICE_JOYPAD:               return SuperFamicom::Input::Device::Joypad;
       case RETRO_DEVICE_ANALOG:               return SuperFamicom::Input::Device::Joypad;
       case RETRO_DEVICE_JOYPAD_MULTITAP:      return SuperFamicom::Input::Device::Multitap;
       case RETRO_DEVICE_MOUSE:                return SuperFamicom::Input::Device::Mouse;
       case RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE: return SuperFamicom::Input::Device::SuperScope;
       case RETRO_DEVICE_LIGHTGUN_JUSTIFIER:   return SuperFamicom::Input::Device::Justifier;
       case RETRO_DEVICE_LIGHTGUN_JUSTIFIERS:  return SuperFamicom::Input::Device::Justifiers;
    }
  }

  void videoRefresh(const uint32_t *data, unsigned pitch, unsigned width, unsigned height) {
    if (!overscan) {
      data += 8 * 1024;

      if (height == 240)
        height = 224;
      else if (height == 480)
        height = 448;
    }

    pvideo_refresh(data, width, height, pitch);
    pinput_poll();
  }

  void audioSample(int16_t left, int16_t right) {
    paudio_sample(left, right);
  }

  int16_t inputPoll(unsigned port, unsigned device, unsigned id) {
    if(id > 11) return 0;
    return pinput_state(port, snes_to_retro(device), 0, snes_to_retro(device, id));
  }

  void loadRequest(unsigned id, const string &p) {
    // Look for BIOS in system directory as well.
    const char *dir = 0;
    penviron(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir);

    string load_path = {path(0), p};
    if(file::exists(load_path)) {
      mmapstream stream(load_path);
      iface->load(id, stream);
    } else if(dir) {
      load_path = {dir, "/", p};
      mmapstream stream(load_path);
      iface->load(id, stream);
    } else {
      fprintf(stderr, "bsnes: Cannot find requested file: \"%s\".\n", (const char*)p);
    }
  }

  string path(unsigned) {
    return string(basename);
  }

  uint32_t videoColor(unsigned, uint16_t r, uint16_t g, uint16_t b) {
    r >>= 8;
    g >>= 8;
    b >>= 8;
    return (r << 16) | (g << 8) | (b << 0);
  }
};

static Callbacks core_bind;

struct Interface : public SuperFamicom::Interface {
  SuperFamicomCartridge::Mode mode;

  void setCheats(const lstring &list = lstring());

  Interface(); 

  void init() {
    updatePalette();
  }
};

struct GBInterface : public GameBoy::Interface {
  GBInterface() { bind = &core_bind; }
  void init() {
    updatePalette();
  }
};

static Interface core_interface;
static GBInterface core_gb_interface;

Interface::Interface() {
  bind = &core_bind;
  core_bind.iface = &core_interface;
}

void Interface::setCheats(const lstring &list) {
  if(core_interface.mode == SuperFamicomCartridge::ModeSuperGameBoy) {
    GameBoy::cheat.reset();
    for(auto &code : list) {
      lstring codelist;
      codelist.split("+", code);
      for(auto &part : codelist) {
        unsigned addr, data, comp;
        if(GameBoy::Cheat::decode(part, addr, data, comp)) {
          GameBoy::cheat.append({addr, data, comp});
        }
      }
    }
    GameBoy::cheat.synchronize();
    return;
  }

  SuperFamicom::cheat.reset();
  for(auto &code : list) {
    lstring codelist;
    codelist.split("+", code);
    for(auto &part : codelist) {
      unsigned addr, data;
      if(SuperFamicom::Cheat::decode(part, addr, data)) {
        SuperFamicom::cheat.append({addr, data});
      }
    }
  }

  SuperFamicom::cheat.synchronize();
}

unsigned retro_api_version(void) {
  return RETRO_API_VERSION;
}

void retro_set_environment(retro_environment_t environ_cb)        { core_bind.penviron       = environ_cb; }
void retro_set_video_refresh(retro_video_refresh_t video_refresh) { core_bind.pvideo_refresh = video_refresh; }
void retro_set_audio_sample(retro_audio_sample_t audio_sample)    { core_bind.paudio_sample  = audio_sample; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t)     {}
void retro_set_input_poll(retro_input_poll_t input_poll)          { core_bind.pinput_poll    = input_poll; }
void retro_set_input_state(retro_input_state_t input_state)       { core_bind.pinput_state   = input_state; }

void retro_set_controller_port_device(unsigned port, unsigned device) {
  if (port < 2)
    SuperFamicom::input.connect(port, Callbacks::retro_to_snes(device));
}

void retro_init(void) {
  SuperFamicom::interface = &core_interface;
  GameBoy::interface = &core_gb_interface;

  core_interface.init();

  memcpy(SuperFamicom::smp.iplrom, iplrom, 64);
  SuperFamicom::system.init();
  SuperFamicom::input.connect(SuperFamicom::Controller::Port1, SuperFamicom::Input::Device::Joypad);
  SuperFamicom::input.connect(SuperFamicom::Controller::Port2, SuperFamicom::Input::Device::Joypad);
}

void retro_deinit(void) {
  SuperFamicom::system.term();
}

void retro_reset(void) {
  SuperFamicom::system.reset();
}

void retro_run(void) {
  SuperFamicom::system.run();
}

size_t retro_serialize_size(void) {
  return SuperFamicom::system.serialize_size();
}

bool retro_serialize(void *data, size_t size) {
  SuperFamicom::system.runtosave();
  serializer s = SuperFamicom::system.serialize();
  if(s.size() > size) return false;
  memcpy(data, s.data(), s.size());
  return true;
}

bool retro_unserialize(const void *data, size_t size) {
  serializer s((const uint8_t*)data, size);
  return SuperFamicom::system.unserialize(s);
}

struct CheatList {
  bool enable;
  string code;
  CheatList() : enable(false) {}
};

static linear_vector<CheatList> cheatList;

void retro_cheat_reset(void) {
  cheatList.reset();
  core_interface.setCheats();
}

void retro_cheat_set(unsigned index, bool enable, const char *code) {
  cheatList[index].enable = enable;
  cheatList[index].code = code;
  lstring list;

  for(unsigned n = 0; n < cheatList.size(); n++) {
    if(cheatList[n].enable) list.append(cheatList[n].code);
  }

  core_interface.setCheats(list);
}

void retro_get_system_info(struct retro_system_info *info) {
  static string version("v", Emulator::Version, " (", SuperFamicom::Info::Profile, ")");
  info->library_name     = "bSNES";
  info->library_version  = version;
  info->valid_extensions = "sfc|smc";
  info->need_fullpath    = false;
}

void retro_get_system_av_info(struct retro_system_av_info *info) {
  struct retro_system_timing timing = { 0.0, 32040.5 };
  timing.fps = retro_get_region() == RETRO_REGION_NTSC ? 21477272.0 / 357366.0 : 21281370.0 / 425568.0;

  if (!core_bind.penviron(RETRO_ENVIRONMENT_GET_OVERSCAN, &core_bind.overscan))
     core_bind.overscan = false;

  unsigned base_width = 256;
  unsigned base_height = core_bind.overscan ? 240 : 224;
  struct retro_game_geometry geom = { base_width, base_height, base_width << 1, base_height << 1, 4.0 / 3.0 };

  info->timing   = timing;
  info->geometry = geom;

  enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
  core_bind.penviron(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt);
}

static bool snes_load_cartridge_normal(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size
) {
  string xmlrom = (rom_xml && *rom_xml) ? string(rom_xml) : SuperFamicomCartridge(rom_data, rom_size).markup;
  SuperFamicom::cartridge.load(xmlrom, memorystream(rom_data, rom_size));
  SuperFamicom::system.power();
  return true;
}

static bool snes_load_cartridge_bsx_slotted(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *bsx_xml, const uint8_t *bsx_data, unsigned bsx_size
) {
  string xmlrom = (rom_xml && *rom_xml) ? string(rom_xml) : SuperFamicomCartridge(rom_data, rom_size).markup;
  string xmlbsx = (bsx_xml && *bsx_xml) ? string(bsx_xml) : SuperFamicomCartridge(bsx_data, bsx_size).markup;

  SuperFamicom::bsxflash.memory.copy(memorystream(bsx_data, bsx_size));
  SuperFamicom::cartridge.load(xmlrom, memorystream(rom_data, rom_size));

  SuperFamicom::system.power();
  return true;
}

static bool snes_load_cartridge_bsx(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *bsx_xml, const uint8_t *bsx_data, unsigned bsx_size
) {
  string xmlrom = (rom_xml && *rom_xml) ? string(rom_xml) : SuperFamicomCartridge(rom_data, rom_size).markup;
  string xmlbsx = (bsx_xml && *bsx_xml) ? string(bsx_xml) : SuperFamicomCartridge(bsx_data, bsx_size).markup;

  SuperFamicom::bsxflash.memory.copy(memorystream(bsx_data, bsx_size));
  SuperFamicom::cartridge.load(xmlrom, memorystream(rom_data, rom_size));

  SuperFamicom::system.power();
  return true;
}

static bool snes_load_cartridge_sufami_turbo(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *sta_xml, const uint8_t *sta_data, unsigned sta_size,
  const char *stb_xml, const uint8_t *stb_data, unsigned stb_size
) {
  string xmlrom = (rom_xml && *rom_xml) ? string(rom_xml) : SuperFamicomCartridge(rom_data, rom_size).markup;
  string xmlsta = (sta_xml && *sta_xml) ? string(sta_xml) : SuperFamicomCartridge(sta_data, sta_size).markup;
  string xmlstb = (stb_xml && *stb_xml) ? string(stb_xml) : SuperFamicomCartridge(stb_data, stb_size).markup;

  if(sta_data) SuperFamicom::sufamiturbo.slotA.rom.copy(memorystream(sta_data, sta_size));
  if(stb_data) SuperFamicom::sufamiturbo.slotB.rom.copy(memorystream(stb_data, stb_size));
  SuperFamicom::cartridge.load(xmlrom, memorystream(rom_data, rom_size));

  SuperFamicom::system.power();
  return true;
}

static bool snes_load_cartridge_super_game_boy(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *dmg_xml, const uint8_t *dmg_data, unsigned dmg_size
) {
  string xmlrom = (rom_xml && *rom_xml) ? string(rom_xml) : SuperFamicomCartridge(rom_data, rom_size).markup;

  if(dmg_data) {
    //GameBoyCartridge needs to modify dmg_data (for MMM01 emulation); so copy data
    uint8_t *data = new uint8_t[dmg_size];
    memcpy(data, dmg_data, dmg_size);
    string xmldmg = (dmg_xml && *dmg_xml) ? string(dmg_xml) : GameBoyCartridge(data, dmg_size).markup;
    GameBoy::cartridge.load(GameBoy::System::Revision::SuperGameBoy, xmldmg, memorystream(data, dmg_size));
    delete[] data;
  }

  SuperFamicom::cartridge.load(xmlrom, memorystream(rom_data, rom_size));
  SuperFamicom::system.power();
  return true;
}

bool retro_load_game(const struct retro_game_info *info) {
  retro_cheat_reset();
  if (info->path) {
    core_bind.basename = info->path;
    char *dot = strrchr(core_bind.basename(), '/');
    if (!dot)
       dot = strrchr(core_bind.basename(), '\\');

    if (dot)
      dot[1] = '\0';
    else
      core_bind.basename = "";
  }

  core_interface.mode = SuperFamicomCartridge::ModeNormal;
  return snes_load_cartridge_normal(info->meta, (const uint8_t*)info->data, info->size);
}

bool retro_load_game_special(unsigned game_type,
      const struct retro_game_info *info, size_t num_info) {

  retro_cheat_reset();
  if (info[0].path) {
    core_bind.basename = info[0].path;
    char *dot = strrchr(core_bind.basename(), '/');
    if (!dot)
       dot = strrchr(core_bind.basename(), '\\');

    if (dot)
      dot[1] = '\0';
    else
      core_bind.basename = "";
  }

  switch (game_type) {
     case RETRO_GAME_TYPE_BSX:
       core_interface.mode = SuperFamicomCartridge::ModeBsx;
       return num_info == 2 && snes_load_cartridge_bsx(info[0].meta, (const uint8_t*)info[0].data, info[0].size,
             info[1].meta, (const uint8_t*)info[1].data, info[1].size);
       
     case RETRO_GAME_TYPE_BSX_SLOTTED:
       core_interface.mode = SuperFamicomCartridge::ModeBsxSlotted;
       return num_info == 2 && snes_load_cartridge_bsx_slotted(info[0].meta, (const uint8_t*)info[0].data, info[0].size,
             info[1].meta, (const uint8_t*)info[1].data, info[1].size);

     case RETRO_GAME_TYPE_SUPER_GAME_BOY:
       core_interface.mode = SuperFamicomCartridge::ModeSuperGameBoy;
       return num_info == 2 && snes_load_cartridge_super_game_boy(info[0].meta, (const uint8_t*)info[0].data, info[0].size,
             info[1].meta, (const uint8_t*)info[1].data, info[1].size);

     case RETRO_GAME_TYPE_SUFAMI_TURBO:
       core_interface.mode = SuperFamicomCartridge::ModeSufamiTurbo;
       return num_info == 3 && snes_load_cartridge_sufami_turbo(info[0].meta, (const uint8_t*)info[0].data, info[0].size,
             info[1].meta, (const uint8_t*)info[1].data, info[1].size,
             info[2].meta, (const uint8_t*)info[2].data, info[2].size);

     default:
       return false;
  }
}

void retro_unload_game(void) {
  SuperFamicom::cartridge.unload();
}

unsigned retro_get_region(void) {
  return SuperFamicom::system.region() == SuperFamicom::System::Region::NTSC ? 0 : 1;
}

void* retro_get_memory_data(unsigned id) {
  if(SuperFamicom::cartridge.loaded() == false) return 0;

  switch(id) {
    case RETRO_MEMORY_SAVE_RAM:
      return SuperFamicom::cartridge.ram.data();
    case RETRO_MEMORY_RTC:
      if(SuperFamicom::cartridge.has_srtc()) return SuperFamicom::srtc.rtc;
      if(SuperFamicom::cartridge.has_spc7110rtc()) return SuperFamicom::spc7110.rtc;
      return 0;
    case RETRO_MEMORY_SNES_BSX_RAM:
      if(core_interface.mode != SuperFamicomCartridge::ModeBsx) break;
      return SuperFamicom::bsxcartridge.sram.data();
    case RETRO_MEMORY_SNES_BSX_PRAM:
      if(core_interface.mode != SuperFamicomCartridge::ModeBsx) break;
      return SuperFamicom::bsxcartridge.psram.data();
    case RETRO_MEMORY_SNES_SUFAMI_TURBO_A_RAM:
      if(core_interface.mode != SuperFamicomCartridge::ModeSufamiTurbo) break;
      return SuperFamicom::sufamiturbo.slotA.ram.data();
    case RETRO_MEMORY_SNES_SUFAMI_TURBO_B_RAM:
      if(core_interface.mode != SuperFamicomCartridge::ModeSufamiTurbo) break;
      return SuperFamicom::sufamiturbo.slotB.ram.data();
    case RETRO_MEMORY_SNES_GAME_BOY_RAM:
      if(core_interface.mode != SuperFamicomCartridge::ModeSuperGameBoy) break;
      return GameBoy::cartridge.ramdata;

    case RETRO_MEMORY_SYSTEM_RAM:
      return SuperFamicom::cpu.wram;
    case RETRO_MEMORY_VIDEO_RAM:
      return SuperFamicom::ppu.vram;
  }

  return 0;
}

size_t retro_get_memory_size(unsigned id) {
  if(SuperFamicom::cartridge.loaded() == false) return 0;
  size_t size = 0;

  switch(id) {
    case RETRO_MEMORY_SAVE_RAM:
      size = SuperFamicom::cartridge.ram.size();
      break;
    case RETRO_MEMORY_RTC:
      if(SuperFamicom::cartridge.has_srtc() || SuperFamicom::cartridge.has_spc7110rtc()) size = 20;
      break;
    case RETRO_MEMORY_SNES_BSX_RAM:
      if(core_interface.mode != SuperFamicomCartridge::ModeBsx) break;
      size = SuperFamicom::bsxcartridge.sram.size();
      break;
    case RETRO_MEMORY_SNES_BSX_PRAM:
      if(core_interface.mode != SuperFamicomCartridge::ModeBsx) break;
      size = SuperFamicom::bsxcartridge.psram.size();
      break;
    case RETRO_MEMORY_SNES_SUFAMI_TURBO_A_RAM:
      if(core_interface.mode != SuperFamicomCartridge::ModeSufamiTurbo) break;
      size = SuperFamicom::sufamiturbo.slotA.ram.size();
      break;
    case RETRO_MEMORY_SNES_SUFAMI_TURBO_B_RAM:
      if(core_interface.mode != SuperFamicomCartridge::ModeSufamiTurbo) break;
      size = SuperFamicom::sufamiturbo.slotB.ram.size();
      break;
    case RETRO_MEMORY_SNES_GAME_BOY_RAM:
      if(core_interface.mode != SuperFamicomCartridge::ModeSuperGameBoy) break;
      size = GameBoy::cartridge.ramsize;
      break;

    case RETRO_MEMORY_SYSTEM_RAM:
      size = 128 * 1024;
      break;
    case RETRO_MEMORY_VIDEO_RAM:
      size = 64 * 1024;
      break;
  }

  if(size == -1U) size = 0;
  return size;
}

