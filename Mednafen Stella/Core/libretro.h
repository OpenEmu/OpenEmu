#ifndef LIBRETRO_H__
#define LIBRETRO_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#else
#if defined(_MSC_VER) && !defined(__cplusplus)
#define bool unsigned char
#define true 1
#define false 0
#else
#include <stdbool.h>
#endif
#endif
    
#define RETRO_API_VERSION         1
    
#define RETRO_DEVICE_MASK         0xff
#define RETRO_DEVICE_NONE         0
#define RETRO_DEVICE_JOYPAD       1
#define RETRO_DEVICE_MOUSE        2
#define RETRO_DEVICE_KEYBOARD     3
#define RETRO_DEVICE_LIGHTGUN     4
#define RETRO_DEVICE_ANALOG       5
    
#define RETRO_DEVICE_JOYPAD_MULTITAP        ((1 << 8) | RETRO_DEVICE_JOYPAD)
#define RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE   ((1 << 8) | RETRO_DEVICE_LIGHTGUN)
#define RETRO_DEVICE_LIGHTGUN_JUSTIFIER     ((2 << 8) | RETRO_DEVICE_LIGHTGUN)
#define RETRO_DEVICE_LIGHTGUN_JUSTIFIERS    ((3 << 8) | RETRO_DEVICE_LIGHTGUN)
    
#define RETRO_DEVICE_ID_JOYPAD_B        0
#define RETRO_DEVICE_ID_JOYPAD_Y        1
#define RETRO_DEVICE_ID_JOYPAD_SELECT   2
#define RETRO_DEVICE_ID_JOYPAD_START    3
#define RETRO_DEVICE_ID_JOYPAD_UP       4
#define RETRO_DEVICE_ID_JOYPAD_DOWN     5
#define RETRO_DEVICE_ID_JOYPAD_LEFT     6
#define RETRO_DEVICE_ID_JOYPAD_RIGHT    7
#define RETRO_DEVICE_ID_JOYPAD_A        8
#define RETRO_DEVICE_ID_JOYPAD_X        9
#define RETRO_DEVICE_ID_JOYPAD_L       10
#define RETRO_DEVICE_ID_JOYPAD_R       11
#define RETRO_DEVICE_ID_JOYPAD_L2      12
#define RETRO_DEVICE_ID_JOYPAD_R2      13
#define RETRO_DEVICE_ID_JOYPAD_L3      14
#define RETRO_DEVICE_ID_JOYPAD_R3      15
    
#define RETRO_DEVICE_INDEX_ANALOG_LEFT   0
#define RETRO_DEVICE_INDEX_ANALOG_RIGHT  1
#define RETRO_DEVICE_ID_ANALOG_X         0
#define RETRO_DEVICE_ID_ANALOG_Y         1
    
#define RETRO_DEVICE_ID_MOUSE_X      0
#define RETRO_DEVICE_ID_MOUSE_Y      1
#define RETRO_DEVICE_ID_MOUSE_LEFT   2
#define RETRO_DEVICE_ID_MOUSE_RIGHT  3
    
#define RETRO_DEVICE_ID_LIGHTGUN_X        0
#define RETRO_DEVICE_ID_LIGHTGUN_Y        1
#define RETRO_DEVICE_ID_LIGHTGUN_TRIGGER  2
#define RETRO_DEVICE_ID_LIGHTGUN_CURSOR   3
#define RETRO_DEVICE_ID_LIGHTGUN_TURBO    4
#define RETRO_DEVICE_ID_LIGHTGUN_PAUSE    5
#define RETRO_DEVICE_ID_LIGHTGUN_START    6
    
#define RETRO_REGION_NTSC  0
#define RETRO_REGION_PAL   1
    
#define RETRO_MEMORY_MASK        0xff
#define RETRO_MEMORY_SAVE_RAM    0
#define RETRO_MEMORY_RTC         1
#define RETRO_MEMORY_SYSTEM_RAM  2
#define RETRO_MEMORY_VIDEO_RAM   3
    
#define RETRO_MEMORY_SNES_BSX_RAM             ((1 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_BSX_PRAM            ((2 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_SUFAMI_TURBO_A_RAM  ((3 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_SUFAMI_TURBO_B_RAM  ((4 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_GAME_BOY_RAM        ((5 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_GAME_BOY_RTC        ((6 << 8) | RETRO_MEMORY_RTC)
    
#define RETRO_GAME_TYPE_BSX             0x101
#define RETRO_GAME_TYPE_BSX_SLOTTED     0x102
#define RETRO_GAME_TYPE_SUFAMI_TURBO    0x103
#define RETRO_GAME_TYPE_SUPER_GAME_BOY  0x104
    
    
    // Environment commands.
#define RETRO_ENVIRONMENT_SET_ROTATION  1  // const unsigned * --
    // Sets screen rotation of graphics.
    // Is only implemented if rotation can be accelerated by hardware.
    // Valid values are 0, 1, 2, 3, which rotates screen by 0, 90, 180, 270 degrees
    // counter-clockwise respectively.
    //
#define RETRO_ENVIRONMENT_GET_OVERSCAN  2  // bool * --
    // Boolean value whether or not the implementation should use overscan, or crop away overscan.
    //
#define RETRO_ENVIRONMENT_GET_CAN_DUPE  3  // bool * --
    // Boolean value whether or not RetroArch supports frame duping,
    // passing NULL to video frame callback.
    //
#define RETRO_ENVIRONMENT_GET_VARIABLE  4  // struct retro_variable * --
    // Interface to aquire user-defined information from environment
    // that cannot feasibly be supported in a multi-system way.
    // Mostly used for obscure,
    // specific features that the user can tap into when neseccary.
    //
#define RETRO_ENVIRONMENT_SET_VARIABLES 5  // const struct retro_variable * --
    // Allows an implementation to signal the environment
    // which variables it might want to check for later using GET_VARIABLE.
    // 'data' points to an array of retro_variable structs terminated by a { NULL, NULL } element.
    // retro_variable::value should contain a human readable description of the key.
    //
#define RETRO_ENVIRONMENT_SET_MESSAGE   6  // const struct retro_message * --
    // Sets a message to be displayed in implementation-specific manner for a certain amount of 'frames'.
    // Should not be used for trivial messages, which should simply be logged to stderr.
#define RETRO_ENVIRONMENT_SHUTDOWN      7  // N/A (NULL) --
    // Requests the frontend to shutdown.
    // Should only be used if game has a specific
    // way to shutdown the game from a menu item or similar.
    //
#define RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL 8
    // const unsigned * --
    // Gives a hint to the frontend how demanding this implementation
    // is on a system. E.g. reporting a level of 2 means
    // this implementation should run decently on all frontends
    // of level 2 and up.
    //
    // It can be used by the frontend to potentially warn
    // about too demanding implementations.
    //
    // The levels are "floating", but roughly defined as:
    // 1: Low-powered devices such as Raspberry Pi, smart phones, tablets, etc.
    // 2: Medium-spec consoles, such as PS3/360, with sub-par CPUs.
    // 3: Modern desktop/laptops with reasonably powerful CPUs.
    // 4: High-end desktops with very powerful CPUs.
    //
    // This function can be called on a per-game basis,
    // as certain games an implementation can play might be
    // particularily demanding.
    // If called, it should be called in retro_load_game().
    //
#define RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY 9
    // const char ** --
    // Returns the "system" directory of the frontend.
    // This directory can be used to store system specific ROMs such as BIOSes, configuration data, etc.
    // The returned value can be NULL.
    // If so, no such directory is defined,
    // and it's up to the implementation to find a suitable directory.
    //
#define RETRO_ENVIRONMENT_SET_PIXEL_FORMAT 10
    // const enum retro_pixel_format * --
    // Sets the internal pixel format used by the implementation.
    // The default pixel format is RETRO_PIXEL_FORMAT_XRGB1555.
    // If the call returns false, the frontend does not support this pixel format.
    // This function should be called inside retro_load_game() or retro_get_system_av_info().
    
    enum retro_pixel_format
    {
        RETRO_PIXEL_FORMAT_0RGB1555 = 0, // 0RGB1555, native endian. 0 bit must be set to 0.
        RETRO_PIXEL_FORMAT_XRGB8888      // XRGB8888, native endian. X bits are ignored.
    };
    
    struct retro_message
    {
        const char *msg;        // Message to be displayed.
        unsigned    frames;     // Duration in frames of message.
    };
    
    struct retro_system_info
    {
        const char *library_name;      // Descriptive name of library. Should not contain any version numbers, etc.
        const char *library_version;   // Descriptive version of core.
        
        const char *valid_extensions;  // A string listing probably rom extensions the core will be able to load, separated with pipe.
        // I.e. "bin|rom|iso".
        // Typically used for a GUI to filter out extensions.
        
        bool        need_fullpath;     // If true, retro_load_game() is guaranteed to provide a valid pathname in retro_game_info::path.
        // ::data and ::size are both invalid.
        // If false, ::data and ::size are guaranteed to be valid, but ::path might not be valid.
        // This is typically set to true for libretro implementations that must load from file.
        // Implementations should strive for setting this to false, as it allows the frontend to perform patching, etc.
        
        bool        block_extract;     // If true, the frontend is not allowed to extract any archives before loading the real ROM.
        // Necessary for certain libretro implementations that load games from zipped archives.
    };
    
    struct retro_game_geometry
    {
        unsigned base_width;    // Nominal video width of game.
        unsigned base_height;   // Nominal video height of game.
        unsigned max_width;     // Maximum possible width of game.
        unsigned max_height;    // Maximum possible height of game.
        
        float    aspect_ratio;  // Nominal aspect ratio of game. If aspect_ratio is <= 0.0,
        // an aspect ratio of base_width / base_height is assumed.
        // A frontend could override this setting if desired.
    };
    
    struct retro_system_timing
    {
        double fps;             // FPS of video content.
        double sample_rate;     // Sampling rate of audio.
    };
    
    struct retro_system_av_info
    {
        struct retro_game_geometry geometry;
        struct retro_system_timing timing;
    };
    
    struct retro_variable
    {
        const char *key;        // Variable to query in RETRO_ENVIRONMENT_GET_VARIABLE.
        // If NULL, obtains the complete environment string if more complex parsing is necessary.
        // The environment string is formatted as key-value pairs delimited by semicolons as so:
        // "key1=value1;key2=value2;..."
        const char *value;      // Value to be obtained. If key does not exist, it is set to NULL.
    };
    
    struct retro_game_info
    {
        const char *path;       // Path to game, UTF-8 encoded. Usually used as a reference.
        // May be NULL if rom was loaded from stdin or similar.
        // retro_system_info::need_fullpath guaranteed that this path is valid.
        const void *data;       // Memory buffer of loaded game. Will be NULL if need_fullpath was set.
        size_t      size;       // Size of memory buffer.
        const char *meta;       // String of implementation specific meta-data.
    };
    
    // Callbacks
    //
    // Environment callback. Gives implementations a way of performing uncommon tasks. Extensible.
    typedef bool (*retro_environment_t)(unsigned cmd, void *data);
    
    // Render a frame. Pixel format is 15-bit 0RGB1555 native endian unless changed (see RETRO_ENVIRONMENT_SET_PIXEL_FORMAT).
    // Width and height specify dimensions of buffer.
    // Pitch specifices length in bytes between two lines in buffer.
    typedef void (*retro_video_refresh_t)(const void *data, unsigned width, unsigned height, size_t pitch);
    
    // Renders a single audio frame. Should only be used if implementation generates a single sample at a time.
    // Format is signed 16-bit native endian.
    typedef void (*retro_audio_sample_t)(int16_t left, int16_t right);
    // Renders multiple audio frames in one go. One frame is defined as a sample of left and right channels, interleaved.
    // I.e. int16_t buf[4] = { l, r, l, r }; would be 2 frames.
    // Only one of the audio callbacks must ever be used.
    typedef size_t (*retro_audio_sample_batch_t)(const int16_t *data, size_t frames);
    
    // Polls input.
    typedef void (*retro_input_poll_t)(void);
    // Queries for input for player 'port'. device will be masked with RETRO_DEVICE_MASK.
    // Specialization of devices such as RETRO_DEVICE_JOYPAD_MULTITAP that have been set with retro_set_controller_port_device()
    // will still use the higher level RETRO_DEVICE_JOYPAD to request input.
    typedef int16_t (*retro_input_state_t)(unsigned port, unsigned device, unsigned index, unsigned id);
    
    // Sets callbacks. retro_set_environment() is guaranteed to be called before retro_init().
    // The rest of the set_* functions are guaranteed to have been called before the first call to retro_run() is made.
    void retro_set_environment(retro_environment_t);
    void retro_set_video_refresh(retro_video_refresh_t);
    void retro_set_audio_sample(retro_audio_sample_t);
    void retro_set_audio_sample_batch(retro_audio_sample_batch_t);
    void retro_set_input_poll(retro_input_poll_t);
    void retro_set_input_state(retro_input_state_t);
    
    // Library global initialization/deinitialization.
    void retro_init(void);
    void retro_deinit(void);
    
    // Must return RETRO_API_VERSION. Used to validate ABI compatibility when the API is revised.
    unsigned retro_api_version(void);
    
    // Gets statically known system info. Pointers provided in *info must be statically allocated.
    // Can be called at any time, even before retro_init().
    void retro_get_system_info(struct retro_system_info *info);
    
    // Gets information about system audio/video timings and geometry.
    // Can be called only after retro_load_game() has successfully completed.
    void retro_get_system_av_info(struct retro_system_av_info *info);
    
    // Sets device to be used for player 'port'.
    void retro_set_controller_port_device(unsigned port, unsigned device);
    
    // Resets the current game.
    void retro_reset(void);
    
    // Runs the game for one video frame.
    // During retro_run(), input_poll callback must be called at least once.
    //
    // If a frame is not rendered for reasons where a game "dropped" a frame,
    // this still counts as a frame, and retro_run() should explicitly dupe a frame if GET_CAN_DUPE returns true.
    // In this case, the video callback can take a NULL argument for data.
    void retro_run(void);
    
    // Returns the amount of data the implementation requires to serialize internal state (save states).
    // Beetween calls to retro_load_game() and retro_unload_game(), the returned size is never allowed to be larger than a previous returned value, to
    // ensure that the frontend can allocate a save state buffer once.
    size_t retro_serialize_size(void);
    
    // Serializes internal state. If failed, or size is lower than retro_serialize_size(), it should return false, true otherwise.
    bool retro_serialize(void *data, size_t size);
    bool retro_unserialize(const void *data, size_t size);
    
    void retro_cheat_reset(void);
    void retro_cheat_set(unsigned index, bool enabled, const char *code);
    
    // Loads a game.
    bool retro_load_game(const struct retro_game_info *game);
    
    // Loads a "special" kind of game. Should not be used except in extreme cases.
    bool retro_load_game_special(
                                 unsigned game_type,
                                 const struct retro_game_info *info, size_t num_info
                                 );
    
    // Unloads a currently loaded game.
    void retro_unload_game(void);
    
    // Gets region of game.
    unsigned retro_get_region(void);
    
    // Gets region of memory.
    void *retro_get_memory_data(unsigned id);
    size_t retro_get_memory_size(unsigned id);
    
#ifdef __cplusplus
}
#endif

#endif
