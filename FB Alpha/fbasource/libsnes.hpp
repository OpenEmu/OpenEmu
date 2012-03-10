#ifndef LIBSNES_HPP
#define LIBSNES_HPP

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SNES_PORT_1  0
#define SNES_PORT_2  1

#define SNES_DEVICE_NONE          0
#define SNES_DEVICE_JOYPAD        1
#define SNES_DEVICE_MULTITAP      2
#define SNES_DEVICE_MOUSE         3
#define SNES_DEVICE_SUPER_SCOPE   4
#define SNES_DEVICE_JUSTIFIER     5
#define SNES_DEVICE_JUSTIFIERS    6
#define SNES_DEVICE_SERIAL_CABLE  7

#define SNES_DEVICE_ID_JOYPAD_B        0
#define SNES_DEVICE_ID_JOYPAD_Y        1
#define SNES_DEVICE_ID_JOYPAD_SELECT   2
#define SNES_DEVICE_ID_JOYPAD_START    3
#define SNES_DEVICE_ID_JOYPAD_UP       4
#define SNES_DEVICE_ID_JOYPAD_DOWN     5
#define SNES_DEVICE_ID_JOYPAD_LEFT     6
#define SNES_DEVICE_ID_JOYPAD_RIGHT    7
#define SNES_DEVICE_ID_JOYPAD_A        8
#define SNES_DEVICE_ID_JOYPAD_X        9
#define SNES_DEVICE_ID_JOYPAD_L       10
#define SNES_DEVICE_ID_JOYPAD_R       11

#define SNES_DEVICE_ID_MOUSE_X      0
#define SNES_DEVICE_ID_MOUSE_Y      1
#define SNES_DEVICE_ID_MOUSE_LEFT   2
#define SNES_DEVICE_ID_MOUSE_RIGHT  3

#define SNES_DEVICE_ID_SUPER_SCOPE_X        0
#define SNES_DEVICE_ID_SUPER_SCOPE_Y        1
#define SNES_DEVICE_ID_SUPER_SCOPE_TRIGGER  2
#define SNES_DEVICE_ID_SUPER_SCOPE_CURSOR   3
#define SNES_DEVICE_ID_SUPER_SCOPE_TURBO    4
#define SNES_DEVICE_ID_SUPER_SCOPE_PAUSE    5

#define SNES_DEVICE_ID_JUSTIFIER_X        0
#define SNES_DEVICE_ID_JUSTIFIER_Y        1
#define SNES_DEVICE_ID_JUSTIFIER_TRIGGER  2
#define SNES_DEVICE_ID_JUSTIFIER_START    3

#define SNES_REGION_NTSC  0
#define SNES_REGION_PAL   1

#define SNES_MEMORY_CARTRIDGE_RAM       0
#define SNES_MEMORY_CARTRIDGE_RTC       1
#define SNES_MEMORY_BSX_RAM             2
#define SNES_MEMORY_BSX_PRAM            3
#define SNES_MEMORY_SUFAMI_TURBO_A_RAM  4
#define SNES_MEMORY_SUFAMI_TURBO_B_RAM  5
#define SNES_MEMORY_GAME_BOY_RAM        6
#define SNES_MEMORY_GAME_BOY_RTC        7

#define SNES_MEMORY_WRAM    100
#define SNES_MEMORY_APURAM  101
#define SNES_MEMORY_VRAM    102
#define SNES_MEMORY_OAM     103
#define SNES_MEMORY_CGRAM   104

// SSNES extension. Not required to be implemented for a working implementation.

#define SNES_ENVIRONMENT_GET_FULLPATH 0         // const char ** --
                                                // Full path of game loaded.
                                                //
#define SNES_ENVIRONMENT_SET_GEOMETRY 1         // const struct snes_geometry * --
                                                // Window geometry information for the system/game.
                                                //
#define SNES_ENVIRONMENT_SET_PITCH 2            // const unsigned * --
                                                // Pitch of game image.
                                                //
#define SNES_ENVIRONMENT_GET_OVERSCAN 3         // bool * --
                                                // Boolean value whether or not the implementation should use overscan.
                                                //
#define SNES_ENVIRONMENT_SET_TIMING 4           // const struct snes_system_timing * --
                                                // Set exact timings of the system. Used primarily for video recording.
                                                //
#define SNES_ENVIRONMENT_GET_CAN_DUPE 5         // bool * --
                                                // Boolean value whether or not SSNES supports frame duping,
                                                // passing NULL to video frame callback.
                                                //
                                                //
#define SNES_ENVIRONMENT_SET_NEED_FULLPATH 6    // const bool * --
                                                // Boolean value telling if implementation needs a valid fullpath to be able to run.
                                                // If this is the case, SSNES will not open the rom directly,
                                                // and pass NULL to rom data.
                                                // Implementation must then use SNES_ENVIRONMENT_GET_FULLPATH.
                                                // This is useful for implementations with very large roms,
                                                // which are impractical to load fully into RAM.

struct snes_geometry
{
   unsigned base_width;    // Nominal video width of system.
   unsigned base_height;   // Nominal video height of system.
   unsigned max_width;     // Maximum possible width of system.
   unsigned max_height;    // Maximum possible height of system.
};

struct snes_system_timing
{
   double fps;
   double sample_rate;
};

typedef bool (*snes_environment_t)(unsigned cmd, void *data);

// Must be called before calling snes_init().
void snes_set_environment(snes_environment_t);
////


typedef void (*snes_video_refresh_t)(const uint16_t *data, unsigned width, unsigned height);
typedef void (*snes_audio_sample_t)(uint16_t left, uint16_t right);
typedef void (*snes_input_poll_t)(void);
typedef int16_t (*snes_input_state_t)(bool port, unsigned device, unsigned index, unsigned id);

const char* snes_library_id(void);
unsigned snes_library_revision_major(void);
unsigned snes_library_revision_minor(void);

void snes_set_video_refresh(snes_video_refresh_t);
void snes_set_audio_sample(snes_audio_sample_t);
void snes_set_input_poll(snes_input_poll_t);
void snes_set_input_state(snes_input_state_t);

void snes_set_controller_port_device(bool port, unsigned device);
void snes_set_cartridge_basename(const char *basename);

void snes_init(void);
void snes_term(void);
void snes_power(void);
void snes_reset(void);
void snes_run(void);

unsigned snes_serialize_size(void);
bool snes_serialize(uint8_t *data, unsigned size);
bool snes_unserialize(const uint8_t *data, unsigned size);

void snes_cheat_reset(void);
void snes_cheat_set(unsigned index, bool enabled, const char *code);

bool snes_load_cartridge_normal(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size
);

bool snes_load_cartridge_bsx_slotted(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *bsx_xml, const uint8_t *bsx_data, unsigned bsx_size
);

bool snes_load_cartridge_bsx(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *bsx_xml, const uint8_t *bsx_data, unsigned bsx_size
);

bool snes_load_cartridge_sufami_turbo(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *sta_xml, const uint8_t *sta_data, unsigned sta_size,
  const char *stb_xml, const uint8_t *stb_data, unsigned stb_size
);

bool snes_load_cartridge_super_game_boy(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *dmg_xml, const uint8_t *dmg_data, unsigned dmg_size
);

void snes_unload_cartridge(void);

bool snes_get_region(void);
uint8_t* snes_get_memory_data(unsigned id);
unsigned snes_get_memory_size(unsigned id);

#ifdef __cplusplus
}
#endif

#endif
