#ifndef LIBSNES_HPP
#define LIBSNES_HPP

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER) && defined(LIBSNES_CORE)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

///////////////////////////////////////////////////////////////////////////////
// LIBSNES Super Nintendo emulation API
//
// Things you should know:
//  - Linking against libsnes requires a C++ compiler. It can be compiled with
//    a C99 compiler if you #include <stdbool.h> and if your C99 compiler's
//    bool type is compatible with the bool type used by the C++ compiler used
//    to compile libsnes.
//  - libsnes supports exactly one emulated SNES; if you want to run two SNESes
//    in a single process, you'll need to link against or dlopen() two
//    different copies of the library.
//
// Typical usage of the libsnes API looks like this:
//
// 1. Call snes_init() to initialize the library.
// 2. Tell libsnes which callback should be called for each event (see the
//    documentation on the individual callback types below.
// 3. Call one of the snes_load_cartridge_* functions to load cartridge data
//    into the emulated SNES.
// 4. If the physical cart had any non-volatile storage, there may be data from
//    a previous emulation run that needs to be loaded. Find the storage buffer
//    by calling the snes_get_memory_* functions and load any saved data into
//    it.
// 5. Call snes_set_controller_port_device() to connect appropriate controllers
//    to the emulated SNES.
// 6. Call snes_get_region() to determine the intended screen refresh rate for
//    this cartridge..
// 7. Call snes_run() to emulate a single frame. Before snes_run() returns, the
//    installed callbacks will be called - possibly multiple times.
// 8. When you're done, call snes_term() to free all memory allocated
//    associated with the emulated SNES.
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Constants                                                                {{{

// These constants represent the two controller ports on the front of the SNES,
// for use with the snes_set_controller_port_device() function and the
// snes_input_state_t callback.
#define SNES_PORT_1  0
#define SNES_PORT_2  1

// These constants represent the different kinds of controllers that can be
// connected to a controller port, for use with the
// snes_set_controller_port_device() function and the snes_input_state_t
// callback.
#define SNES_DEVICE_NONE         0
#define SNES_DEVICE_JOYPAD       1
#define SNES_DEVICE_MULTITAP     2
#define SNES_DEVICE_MOUSE        3
#define SNES_DEVICE_SUPER_SCOPE  4
#define SNES_DEVICE_JUSTIFIER    5
#define SNES_DEVICE_JUSTIFIERS   6
#define SNES_DEVICE_SERIAL_CABLE 7

// These constants represent the button and axis inputs on various controllers,
// for use with the snes_input_state_t callback.
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

// These constants will be returned by snes_get_region(), representing the
// region of the last loaded cartridge.
#define SNES_REGION_NTSC  0
#define SNES_REGION_PAL   1

// These constants represent the kinds of non-volatile memory a SNES cartridge
// might have, for use with the snes_get_memory_* functions.
#define SNES_MEMORY_CARTRIDGE_RAM       0
#define SNES_MEMORY_CARTRIDGE_RTC       1
#define SNES_MEMORY_BSX_RAM             2
#define SNES_MEMORY_BSX_PRAM            3
#define SNES_MEMORY_SUFAMI_TURBO_A_RAM  4
#define SNES_MEMORY_SUFAMI_TURBO_B_RAM  5
#define SNES_MEMORY_GAME_BOY_RAM        6
#define SNES_MEMORY_GAME_BOY_RTC        7

// These constants represent the various kinds of volatile storage the SNES
// offers, to allow libsnes clients to implement things like cheat-searching
// and certain kinds of debugging. They are for use with the snes_get_memory_*
// functions.
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
                                                //
#define SNES_ENVIRONMENT_GET_CAN_REWIND 7       // bool * --
                                                // Boolean value telling if SSNES is able to rewind.
                                                // Some implementations might need to take extra precautions
                                                // to allow this as smoothly as possible.

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


////////////////////////////////////////////////////////////////////////////}}}

///////////////////////////////////////////////////////////////////////////////
// Callback types                                                           {{{
//
// In order to deliver controller input to the emulated SNES, and retrieve
// video frames and audio samples, you will need to register callbacks.

// snes_audio_sample_t:
//
//    This callback delivers a stereo audio sample pair generated by the
//    emulated SNES.
//
//    This function is called once for every audio frame (one sample from left
//    and right channels). The SNES generates audio samples at a rate of about
//    32040Hz (varies from unit to unit).
//
//    Because the SNES generates video at exactly 59.94fps and most computer
//    monitors only support a 60fps refresh rate, real-time emulation needs to
//    run slightly fast so that each computer frame displays one emulated SNES
//    frame. Because the emulation runs slightly fast, and because most
//    consumer audio hardware does not play audio at precisely the requested
//    sample rate, you'll likely need to let the end-user tweak the effective
//    sample rate by 100Hz or so in either direction.
//
//    Although the parameters are declared as unsigned for historical reasons,
//    the data they contain is actually signed. To work with the audio (e.g.
//    resample), you will need to reinterpret the sample value:
//
//          int16_t real_left = *(int16_t*)(&left);
//
//    Parameters:
//
//      left:
//          A signed 16-bit integer containing the next audio sample from the
//          left audio channel. Yes, it's declared as unsigned for historical
//          reasons.
//
//      right:
//          A signed 16-bit integer containing the next audio sample from the
//          right audio channel. Yes, it's declared as unsigned for historical
//          reasons.
//

typedef void (*snes_audio_sample_t)(uint16_t left, uint16_t right);


// snes_video_refresh_t:
//
//    This callback delivers a single SNES frame, generated by the emulated
//    SNES. The same memory buffer may be re-used later, so take a copy of the
//    data if you want to refer to it after your callback returns.
//
//    The framebuffer is an array of unsigned 16-bit pixels, in a somewhat
//    complicated format. A quick refresher on SNES video modes:
//      - The basic SNES video-mode renders 256 pixels per scanline for a total
//        of 224 scanlines.
//      - When "overscan" mode is enabled, the SNES renders a few extra
//        scanlines at the end of the frame, for a total of 239 scanlines.
//      - When "hi-res" mode is enabled, the SNES speeds up its pixel rendering
//        to fit 512 pixels per scanline.
//      - Normally the SNES renders its pixels to one field of the interlaced
//        NTSC signal, but if "interlaced" mode is enabled the SNES renders
//        a second set of scanlines inbetween the regular set, for a total of
//        448 (normal) or 478 (overscan) scanlines.
//
//     Thus, the framebuffer memory layout for a standard 256x240 frame looks
//     something like this (note that 'height' has been reduced to 4 or 8 for
//     these examples):
//
//          0                                   1024b
//          ,---------------------------------------.
//          |====== width ======|...................| -.
//          |.......................................|  |
//          |===================|...................|  |
//          |.......................................|  +- height = 4
//          |===================|...................|  |
//          |.......................................|  |
//          |===================|...................|  |
//          |.......................................| -'
//          `---------------------------------------'
//
//      A hi-res frame would look like this:
//
//          0                                   1024b
//          ,---------------------------------------.
//          |================ width ================| -.
//          |.......................................|  |
//          |=======================================|  |
//          |.......................................|  +- height = 4
//          |=======================================|  |
//          |.......................................|  |
//          |=======================================|  |
//          |.......................................| -'
//          `---------------------------------------'
//
//      An interlaced frame would look like this:
//
//          0                                   1024b
//          ,---------------------------------------.
//          |====== width ======|...................| -.
//          |===================|...................|  |
//          |===================|...................|  |
//          |===================|...................|  +- height = 8
//          |===================|...................|  |
//          |===================|...................|  |
//          |===================|...................|  |
//          |===================|...................| -'
//          `---------------------------------------'
//
//      And of course a hi-res, interlaced frame would look like this:
//
//          0                                   1024b
//          ,---------------------------------------.
//          |================ width ================| -.
//          |=======================================|  |
//          |=======================================|  |
//          |=======================================|  |+- height = 8
//          |=======================================|  |
//          |=======================================|  |
//          |=======================================|  |
//          |=======================================| -'
//          `---------------------------------------'
//
//    More succinctly:
//      - the buffer begins at the top-left of the frame
//      - the first "width" bytes contain the first scanline.
//      - if the emulated SNES is in an interlaced video-mode (that is, if the
//        "height" parameter" is 448 or 478) then the second scanline begins at
//        an offset of 1024 bytes (512 pixels) after the first.
//      - otherwise the second scanline begins at an offset of 2048 bytes (1024
//        pixels) after the first.
//      - there are "height" scanlines in total.
//
//    Each pixel contains a 15-bit RGB tuple: 0RRRRRGGGGGBBBBB (XRGB1555)
//
//    Example code:
//
//      void pack_frame (uint16_t * restrict out, const uint16_t * restrict in,
//              unsigned width, unsigned height)
//      {
//         // Normally our pitch is 2048 bytes.
//         int pitch_pixels = 1024;
//         // If we have an interlaced mode, pitch is 1024 bytes.
//         if ( height == 448 || height == 478 )
//            pitch_pixels = 512;
//
//         for ( int y = 0; y < height; y++ )
//         {
//            const uint16_t *src = in + y * pitch_pixels;
//            uint16_t *dst = out + y * width;
//
//            memcpy(dst, src, width * sizeof(uint16_t));
//         }
//      }
//
//    Parameters:
//
//      data:
//          a pointer to the beginning of the framebuffer described above.
//
//      width:
//          the width of the frame, in pixels.
//
//      height:
//          the number of scanlines in the frame.

typedef void (*snes_video_refresh_t)(const uint16_t *data, unsigned width,
        unsigned height);

// snes_input_poll_t:
//
//    This callback requests that you poll your input devices for events, if
//    required.
//
//    Generally called once per frame before the snes_input_state_t callback is
//    called.
//

typedef void (*snes_input_poll_t)(void);

// snes_input_state_t:
//
//    This callback asks for information about the state of a particular input.
//
//    The callback may be called multiple times per frame with the same
//    parameters.
//
//    The callback might not be called at all, if the software running in the
//    emulated SNES does not try to probe the controllers.
//
//    The callback will not be called for a particular port if DEVICE_NONE is
//    connected to it.
//
//    If you wish to emulate any kind of turbo-fire, etc. then you will need to
//    put that logic into this callback.
//
//    Parameters:
//
//      port:
//          One of the constants SNES_PORT_1 or SNES_PORT_2, describing which
//          controller port you should report.
//
//      device:
//          One of the SNES_DEVICE_* constants describing which type of device
//          is currently connected to the given port.
//
//      index:
//          A number describing which of the devices connected to the port is
//          being reported. It's only useful for SNES_DEVICE_MULTITAP and
//          SNES_DEVICE_JUSTIFIERS - for other device types, this parameter is
//          always 0.
//
//      id:
//          One of the SNES_DEVICE_ID_* constants for the given device,
//          describing which button or axis is being reported (for
//          SNES_DEVICE_MULTITAP, use the SNES_DEVICE_ID_JOYPAD_* IDs; for
//          SNES_DEVICE_JUSTIFIERS use the SNES_DEVICE_ID_JUSTIFIER_* IDs.).
//
//    Returns:
//
//      An integer representing the state of the described button or axis.
//
//      - If it represents a digital input such as SNES_DEVICE_ID_JOYPAD_B or
//        SNES_DEVICE_ID_MOUSE_LEFT), return 1 if the button is pressed, and
//        0 otherwise.
//      - If "id" is SNES_DEVICE_ID_MOUSE_X or SNES_DEVICE_ID_MOUSE_Y then
//        return the relative movement of the mouse during the current frame;
//        values outside the range -127 to +127 will be clamped.
//      - If "id" is one of the light-gun axes (such as
//        SNES_DEVICE_ID_JUSTIFIER_Y or SNES_DEVICE_ID_SUPER_SCOPE_X), you
//        should return the relative movement of the pointing device during the
//        current frame.

typedef int16_t (*snes_input_state_t)(bool port, unsigned device,
        unsigned index, unsigned id);

////////////////////////////////////////////////////////////////////////////}}}

///////////////////////////////////////////////////////////////////////////////
// libsnes setup                                                            {{{
//
// These functions are used to get information about and manipulate the libsnes
// library itself, not the emulated SNES it implements.

// snes_library_id:
//
//    Returns a human readable string describing this libsnes implementation.
//    It is not supposed to be parsed or used in any other way than being
//    printed to screen on request by user or otherwise.
//
//    Returns:
//
//       A human-readable string describing this implementation.

EXPORT const char* snes_library_id(void);


// snes_library_revision_major:
//
//    Returns the major API version of this libsnes implementation.
//
//    This number is increased every time there is a compatibility-breaking
//    change to the libsnes API. At startup, your program should call this
//    function and compare the return value to the major API version the
//    program was designed to work with. If they are different, your program
//    will (very likely) not work with this libsnes implementation.
//
//    For example, if your program was designed to work with the libsnes API
//    whose major.minor revision was 1.5, and this function returns a major
//    version of 2, you have a problem.
//
//    Returns:
//
//      An integer, the major API version of this libsnes implementation.

EXPORT unsigned snes_library_revision_major(void);

// snes_library_revision_minor:
//
//    Returns the minor API version of this libsnes implementation.
//
//    This number is increased every time there is a backwards-compatible
//    change to the libsnes API. At startup, your program should call this
//    function and compare the return value to the minor API version the
//    program was designed to work with. If the return value is less than the
//    expected minor version, your program will (very likely) not work with
//    this libsnes implementation.
//
//    For example, if your program was designed to work with the libsnes API
//    whose major.minor revision was 1.5, and this libsnes implementation's
//    major.minor version is 1.3, it's probably missing features you require.
//    On the other hand, if this libsnes implementation's major.minor version
//    is 1.9, it probably has extra fancy features you don't need to worry
//    about.
//
//    Returns:
//
//      An integer, the minor API version of this libsnes implementation.

EXPORT unsigned snes_library_revision_minor(void);

// snes_init:
//
//    Initializes the libsnes implementation.
//
//    This function must be called exactly once before any other library
//    functions are called.

EXPORT void snes_init(void);

// snes_term:
//
//    Shuts down the libsnes implementation.
//
//    This function must be called exactly once. Once called, you should not
//    call any other libsnes functions besides (perhaps) snes_init().

EXPORT void snes_term(void);

////////////////////////////////////////////////////////////////////////////}}}

///////////////////////////////////////////////////////////////////////////////
// Callback registration                                                    {{{
//
// Note that all callbacks should be set up before snes_run() is called for the
// first time.

// snes_set_video_refresh:
//
//    Sets the callback that will receive new video frames.
//
//    See the documentation for snes_video_refresh_t for details.
//
//    Parameters:
//
//      A pointer to a function matching the snes_video_refresh_t call
//      signature.

EXPORT void snes_set_video_refresh(snes_video_refresh_t);

// snes_set_audio_sample
//
//    Sets the callback that will receive new audio sample pairs.
//
//    See the documentation for snes_audio_sample_t for details.
//
//    Parameters:
//
//      A pointer to a function matching the snes_audio_sample_t call
//      signature.

EXPORT void snes_set_audio_sample(snes_audio_sample_t);

// snes_set_input_poll:
//
//    Sets the callback that will be notified to poll input devices.
//
//    See the documentation for snes_input_poll_t for details.
//
//    Parameters:
//
//      A pointer to a function matching the snes_input_poll_t call signature.

EXPORT void snes_set_input_poll(snes_input_poll_t);

// snes_set_input_state:
//
//    Sets the callback that will be used to read input device state.
//
//    See the documentation for snes_input_state_t for details.
//
//    Parameters:
//
//      A pointer to a function matching the snes_input_state_t call signature.

EXPORT void snes_set_input_state(snes_input_state_t);

////////////////////////////////////////////////////////////////////////////}}}

///////////////////////////////////////////////////////////////////////////////
// SNES operation                                                           {{{
//
// Functions for manipulating the emulated SNES.

// snes_set_controller_port_device:
//
//    Sets the input device connected to a given controller port.
//
//    Connecting a device to a port implicitly removes any device previously
//    connected to that port. To remove a device without connecting a new one,
//    pass DEVICE_NONE as the device parameter. From this point onward, the
//    callback passed to set_input_state_cb() will be called with the
//    appropriate device, index and id parameters.
//
//    If this function is never called, the default is to have a DEVICE_JOYPAD
//    connected to both ports.
//
//    Calling this callback from inside the set_input_state_cb() has undefined
//    results, so don't do that.
//
//    Parameters:
//
//      port:
//          One of the constants SNES_PORT_1 or SNES_PORT_2, describing which
//          controller port is being configured.
//
//      device:
//          One of the SNES_DEVICE_* constants describing which type of device
//          should be connected to the given port. Note that some devices can
//          only be connected to SNES_PORT_2. Attempting to connect
//          a port-2-only device to SNES_PORT_1 has undefined results.
//
//          These devices work in either port:
//              - SNES_DEVICE_NONE: No device is connected to this port.
//              - SNES_DEVICE_JOYPAD: A standard SNES gamepad.
//              - SNES_DEVICE_MULTITAP: A multitap controller, which acts like
//                4 SNES_DEVICE_JOYPADs. Your input state callback will be
//                passed "id" parameters between 0 and 3, inclusive.
//              - SNES_DEVICE_MOUSE: A SNES mouse controller, as shipped with
//                Mario Paint.
//
//          These devices only work properly when connected to port 2:
//              - SNES_DEVICE_SUPER_SCOPE: A Nintendo Super Scope light-gun
//                device.
//              - SNES_DEVICE_JUSTIFIER: A Konami Justifier light-gun device.
//              - SNES_DEVICE_JUSTIFIERS: Two Konami Justifier light-gun
//                devices, daisy-chained together. Your input state callback
//                will be passed "id" parameters 0 and 1.

EXPORT void snes_set_controller_port_device(bool port, unsigned device);

// snes_power:
//
//    Turns the emulated console off and back on.
//
//    This functionality is sometimes called "hard reset" and guarantees that
//    all hardware state is reset to a reasonable default.
//
//    Before bsnes v070r07, this resets the controller ports to both contain
//    SNES_DEVICE_JOYPADs.
//
//    This requires that a cartridge is loaded.

EXPORT void snes_power(void);

// snes_reset:
//
//    Presses the "reset" button on the emulated SNES.
//
//    This functionality is sometimes called "soft reset". Most hardware state
//    is reset to a reasonable befault, but not all.
//
//    As of bsnes v073r01, this function (as a side-effect) resets the
//    controller ports to both contain SNES_DEVICE_JOYPADs.
//
//    This requires that a cartridge is loaded.

EXPORT void snes_reset(void);

// snes_run():
//
//    Runs the emulated SNES until the end of the next video frame.
//
//    Usually causes each registered callback to be called before returning.
//
//    This function will run as fast as possible. It is up to the caller to
//    make sure that the game runs at the intended speed.
//
//    For optimal A/V sync, make sure that the audio callback never blocks for
//    longer than a frame (approx 16ms for NTSC, 20ms for PAL)
//
//    Optimally, it should never block for more than a few ms at a time.

EXPORT void snes_run(void);

// snes_get_region():
//
//    Determines the intended frame-rate of the loaded cartridge.
//
//    The two main SNES hardware variants are the US/Japan variant, designed
//    for NTSC output, and the European variant, designed for PAL output.
//    However, the world is not quite so tidy as that, and there are countries
//    like Brazil that use PAL output at NTSC frame-rates.
//
//    For historical reasons this function is named snes_get_region(), but
//    effectively the only information you can reliably infer is the
//    frame-rate.
//
//    Returns:
//
//      One of the SNES_REGION_* constants. SNES_REGION_PAL means 50fps,
//      SNES_REGION_NTSC means 60fps.

EXPORT bool snes_get_region(void);

////////////////////////////////////////////////////////////////////////////}}}

///////////////////////////////////////////////////////////////////////////////
// Save state support                                                       {{{
//
// libsnes has the ability to save the current emulation state and restore it
// at a later time.
//
// Note 1: It is impossible to reliably restore the *exact* state, although the
// difference is only a few cycles. If you demand the ability to reliably
// restore state, call snes_serialize() after each frame to ensure the emulated
// SNES is in a state that can be reliably restored.
//
// Note 2: The save state information is specific to a particular cartridge
// loaded into a particular version of a particular libsnes implementation.
// Unfortunately, there is not yet a way to determine whether a given save
// state is compatible with a given libsnes implementation, other than by
// loading it. However, if snes_serialize_size() does not match the size of an
// old save state, that's a strong hint that something has incompatibly
// changed.

// snes_serialize_size:
//
//    Determines the minimum size of a save state.
//
//    This value can change depending on the features used by the loaded
//    cartridge, and the version of the libsnes implementation used.
//
//    Returns:
//
//      An integer representing the number of bytes required to store the
//      current emulation state.

EXPORT unsigned snes_serialize_size(void);

// snes_serialize:
//
//    Serialize the current emulation state to a buffer.
//
//    If the allocated buffer is smaller than the size returned by
//    snes_serialize_size(), serialization will fail. If the allocated buffer
//    is larger, only the first snes_serialize_size() bytes will be written to.
//
//    The resulting buffer may be stored, and later passed to
//    snes_unserialize() to restore the saved emulation state.
//
//    Parameters:
//
//      data:
//          A pointer to an allocated buffer of memory.
//
//      size:
//          The size of the buffer pointed to by "data". Should be greater than
//          or equal to the value returned by snes_serialize_size().
//
//    Returns:
//
//      A boolean; True means the emulation state was serialized successfully,
//      False means a problem was encountered.

EXPORT bool snes_serialize(uint8_t *data, unsigned size);

// snes_unserialize:
//
//    Unserialize the emulation state from a buffer.
//
//    If the serialization data in the buffer does not appear to be compatible
//    with the current libsnes implementation, the function returns False and
//    the current emulation state is not modified.
//
//    Parameters:
//
//      data:
//          A pointer to an allocated buffer of memory.
//
//      size:
//          The size of the buffer pointed to by "data". Should be greater than
//          or equal to the value returned by snes_serialize_size().
//
//    Returns:
//
//      A boolean; True means the emulation state was loaded successfully,
//      False means a problem was encountered.

EXPORT bool snes_unserialize(const uint8_t *data, unsigned size);

////////////////////////////////////////////////////////////////////////////}}}

///////////////////////////////////////////////////////////////////////////////
// Cheat support                                                            {{{
//
// libsnes does not include any kind of cheat management API; the intention is
// that any change to the set of applied cheats will cause the containing
// application to call snes_cheat_reset() then apply the new set of cheats with
// snes_cheat_set().
//
// Any currently-applied cheats are discarded when a new cartridge is loaded.

// snes_cheat_reset:
//
//    Discards all cheat codes applied to the emulated SNES.

EXPORT void snes_cheat_reset(void);

// snes_cheat_set:
//
//    Apply a sequence of cheat codes to the emulated SNES.
//
//    Since a "cheat code" is basically an instruction to override the value of
//    a particular byte in the SNES' memory, more complex cheats may require
//    several individual codes applied at once. There's no effective difference
//    between applying these codes in a group with one call to
//    snes_cheat_set(), or applying them one at a time with individual calls.
//    However, most cheat databases will have a collection of available cheats
//    for each game, where each item in the collection has a description and
//    a sequence of codes to be applied as a unit. This API makes it easy to
//    present the list of descriptions to the user, and apply each cheat the
//    user selects.
//
//    Parameters:
//
//      index:
//          The given cheat code will be stored at this index in the array of
//          applied cheats. If a cheat already exists at this location, it will
//          be replaced by the new cheat. If the index is larger than any
//          previously specififed index, the array will be resized to
//          accommodate.
//
//      enabled:
//          True means that the cheat will actually be applied, False means
//          that the cheat will have no effect. There is no way to enable or
//          disable a cheat after it has been added, other than to call
//          snes_cheat_set() a second time with the same values for "index" and
//          "code".
//
//      code:
//          A string containing a sequence of cheat codes separated by '+'
//          characters. Any spaces in the string will be removed before
//          parsing.
//
//          Each code in the sequence must be in either GameGenie format
//          ("1234-ABCD") or ProActionReplay format ("1234AB:CD" or
//          "1234ABCD").

EXPORT void snes_cheat_set(unsigned index, bool enabled, const char *code);

////////////////////////////////////////////////////////////////////////////}}}

///////////////////////////////////////////////////////////////////////////////
// Cartridge loading and unloading                                          {{{
//
// Before calling snes_run(), a cartridge must be loaded into the emulated SNES
// so that it has code to run.
//
// Loading a cartridge of any kind calls snes_cheat_reset() as a side-effect.

// snes_load_cartridge_normal:
//
//    Load a normal ROM image into the emulated SNES.
//
//    Parameters:
//
//      rom_xml:
//          A pointer to a null-terminated string containing an XML memory map
//          that describes where the ROM image is mapped into the SNES address
//          space, what special chips it uses (and where they're mapped), etc.
//
//          If NULL, libsnes will guess a memory map. The guessed memory map
//          should be correct for all licenced games in all regions.
//
//      rom_data:
//          A pointer to a byte array containing the uncompressed,
//          de-interleaved, headerless ROM image.
//
//      rom_size:
//          The length of the rom_data array, in bytes.
//
//    Returns:
//
//      A boolean; True means the cartridge was loaded correctly, False means
//      an error occurred.

EXPORT bool snes_load_cartridge_normal(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size
);

// snes_load_cartridge_bsx:
//
//    Load a BS-X base cart image, optionally with a memory pack.
//
//    The Satellaview system, abbreviated "BS-X" for unclear reasons, was an
//    addon for the Super Famicom that connected it to the St. GIGA satellite
//    network. The network would broadcast games at a particular time, and
//    users could download them to replaceable memory packs.
//
//    For more information, see http://en.wikipedia.org/wiki/Satellaview
//
//    Parameters:
//
//      rom_xml:
//          A pointer to a null-terminated string containing an XML memory map
//          that describes where the BS-X base cartridge ROM image is mapped
//          into the SNES address space.
//
//          If NULL, libsnes will guess a memory map. The guessed memory map
//          should be correct for all known BS-X base cartridge images.
//
//      rom_data:
//          A pointer to a byte array containing the uncompressed,
//          de-interleaved, headerless ROM image of the BS-X base cartridge.
//
//          The BS-X base cartridge is named "BS-X - Sore wa Namae o Nusumareta
//          Machi no Monogatari" in some SNES game databases.
//
//      rom_size:
//          The length of the rom_data array, in bytes.
//
//      bsx_xml:
//          A pointer to a null-terminated string containing an XML memory map
//          that describes the BS-X memory pack.
//
//          This parameter is currently ignored and should be passed as NULL.
//
//      bsx_data:
//          A pointer to a byte array containing the uncompressed,
//          de-interleaved, headerless image of the BS-X memory-pack.
//
//          If NULL, libsnes will behave as though no memory-pack were inserted
//          into the base cartridge.
//
//      bsx_size:
//          The length of the bsx_data array, in bytes.
//
//    Returns:
//
//      A boolean; True means the cartridge was loaded correctly, False means
//      an error occurred.

EXPORT bool snes_load_cartridge_bsx(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *bsx_xml, const uint8_t *bsx_data, unsigned bsx_size
);

// snes_load_cartridge_bsx_slotted:
//
//    Load a BS-X slotted cartridge, optionally with a memory pack.
//
//    A BS-X slotted cartridge is an ordinary SNES cartridge, with a slot in
//    the top that accepts the same memory packs used by the BS-X base
//    cartridge.
//
//    Parameters:
//
//      rom_xml:
//          A pointer to a null-terminated string containing an XML memory map
//          that describes where the ROM image is mapped into the SNES address
//          space, what special chips it uses (and where they're mapped), etc.
//
//          If NULL, libsnes will guess a memory map. The guessed memory map
//          should be correct for all licenced games in all regions.
//
//      rom_data:
//          A pointer to a byte array containing the uncompressed,
//          de-interleaved, headerless ROM image.
//
//      rom_size:
//          The length of the rom_data array, in bytes.
//
//      bsx_xml:
//          A pointer to a null-terminated string containing an XML memory map
//          that describes the BS-X memory pack.
//
//          This parameter is currently ignored and should be passed as NULL.
//
//      bsx_data:
//          A pointer to a byte array containing the uncompressed,
//          de-interleaved, headerless image of the BS-X memory-pack.
//
//          If NULL, libsnes will behave as though no memory-pack were inserted
//          into the base cartridge.
//
//      bsx_size:
//          The length of the bsx_data array, in bytes.
//
//    Returns:
//
//      A boolean; True means the cartridge was loaded correctly, False means
//      an error occurred.

EXPORT bool snes_load_cartridge_bsx_slotted(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *bsx_xml, const uint8_t *bsx_data, unsigned bsx_size
);

// snes_load_cartridge_sufami_turbo:
//
//    Load a SuFami Turbo base cart image, optionally with game packs.
//
//    The SuFami Turbo was a cartridge available for the Super Famicom, created
//    by Bandai, with two slots in the top designed to accept special
//    mini-cartridges. The cartridge in Slot A was the cartridge that actually
//    ran, while the cartridge in Slot B was made available to the Slot
//    A cartridge, enabling sharing of save-game data or using characters from
//    one game in another.
//
//    For more information, see: http://en.wikipedia.org/wiki/Sufami_Turbo
//
//    Parameters:
//
//      rom_xml:
//          A pointer to a null-terminated string containing an XML memory map
//          that describes where the SuFami Turbo base cartridge ROM image is
//          mapped into the SNES address space.
//
//          If NULL, libsnes will guess a memory map. The guessed memory map
//          should be correct for all known SuFami Turbo base cartridge images.
//
//      rom_data:
//          A pointer to a byte array containing the uncompressed,
//          de-interleaved, headerless ROM image of the SuFami Turbo base
//          cartridge.
//
//          The SuFami Turbo base cartridge is named "Sufami Turbo" in some
//          SNES game databases.
//
//      rom_size:
//          The length of the rom_data array, in bytes.
//
//      sta_xml:
//          A pointer to a null-terminated string containing an XML memory map
//          that describes the Sufami Turbo cartridge in Slot A.
//
//          This parameter is currently ignored and should be passed as NULL.
//
//      sta_data:
//          A pointer to a byte array containing the uncompressed,
//          de-interleaved, headerless ROM image of the SuFami Turbo cartridge
//          in Slot A.
//
//          This is the cartridge that will be executed by the SNES.
//
//          If NULL, libsnes will behave as though no cartridge were inserted
//          into the Slot A.
//
//      sta_size:
//          The length of the sta_data array, in bytes.
//
//      stb_xml:
//          A pointer to a null-terminated string containing an XML memory map
//          that describes the Sufami Turbo cartridge in Slot B.
//
//          This parameter is currently ignored and should be passed as NULL.
//
//      stb_data:
//          A pointer to a byte array containing the uncompressed,
//          de-interleaved, headerless ROM image of the SuFami Turbo cartridge
//          in Slot B.
//
//          The data in this cartridge will be made available to the cartridge
//          in Slot A.
//
//          If NULL, libsnes will behave as though no cartridge were inserted
//          into Slot B.
//
//      stb_size:
//          The length of the stb_data array, in bytes.
//
//    Returns:
//
//      A boolean; True means the cartridge was loaded correctly, False means
//      an error occurred.

EXPORT bool snes_load_cartridge_sufami_turbo(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *sta_xml, const uint8_t *sta_data, unsigned sta_size,
  const char *stb_xml, const uint8_t *stb_data, unsigned stb_size
);

// snes_load_cartridge_super_game_boy:
//
//    Load a Super Game Boy base cart, optionally with a Gameboy cartridge.
//
//    The Super Game Boy was a cartridge available for the Super Famicom and
//    Super Nintendo that accepted ordinary (original) Gameboy cartridges and
//    allowed the user to play them with a Super Nintendo controller, on a TV.
//    It extended the orginal Gameboy hardware in a few ways, including the
//    ability to display games in various palettes (rather than strictly
//    monochrome), to display a full-colour border image around the Gameboy
//    video output, or even run native SNES code to enhance the game.
//
//    For more information, see: http://en.wikipedia.org/wiki/Super_Game_Boy
//
//    Up until bsnes v073, loading Super Game Boy cartridges only works if the
//    libsupergameboy library from the bsnes release is installed. bsnes v074
//    includes a custom Gameboy emulation core, and external code is no longer
//    required.
//
//    Parameters:
//
//      rom_xml:
//          A pointer to a null-terminated string containing an XML memory map
//          that describes where the Super Game Boy base cartridge ROM image is
//          mapped into the SNES address space.
//
//          If NULL, libsnes will guess a memory map. The guessed memory map
//          should be correct for all known Super Game Boy base cartridge
//          images.
//
//      rom_data:
//          A pointer to a byte array containing the uncompressed,
//          de-interleaved, headerless ROM image of the Super Game Boy base
//          cartridge.
//
//          Appropriate base cartridge images are named "Super Game Boy" or
//          "Super Game Boy 2" in some SNES game databases.
//
//      rom_size:
//          The length of the rom_data array, in bytes.
//
//      dmg_xml:
//          A pointer to a null-terminated string containing an XML memory map
//          that describes the inserted Gameboy cartridge.
//
//          If NULL, libsnes will guess a memory map. The guesed memory map
//          should be correct for all licensed original Gameboy games in all
//          regions.
//
//      dmg_data:
//          A pointer to a byte array containing the uncompressed, headerless
//          ROM image of the inserted Gameboy cartridge.
//
//          If NULL, libsnes will behave as though no cartridge were inserted.
//
//      dmg_size:
//          The length of the dmg_size array, in bytes.
//
//    Returns:
//
//      A boolean; True means the cartridge was loaded correctly, False means
//      an error occurred.

EXPORT bool snes_load_cartridge_super_game_boy(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *dmg_xml, const uint8_t *dmg_data, unsigned dmg_size
);

// snes_set_cartridge_basename:
//
//    Set the location and name of the loaded cartridge.
//
//    libsnes uses this information to locate additional resources the
//    cartridge might require. Currently, these resources include:
//
//    - The MSU-1 data pack and associated audio tracks, if the cartridge makes
//      use of bsnes' MSU-1 special-chip.
//    - The serial-port data receiving library, if the cartridge makes uses of
//      bsnes' serial-data-over-controller-port feature.
//
//    Parameters:
//
//      basename:
//          The path and basename of the loaded cartridge. For example, if the
//          full path to the loaded cartridge is "/path/to/filename.sfc", this
//          parameter should be set to "/path/to/filename".

EXPORT void snes_set_cartridge_basename(const char *basename);

// snes_unload_cartridge:
//
//    Unloads the currently loaded cartridge from the emulated SNES.
//
//    You will be unable to call snes_run() until another cartridge is loaded.

EXPORT void snes_unload_cartridge(void);

////////////////////////////////////////////////////////////////////////////}}}

///////////////////////////////////////////////////////////////////////////////
// Volatile and non-volatile storage                                        {{{
//
// Certain SNES cartridges include non-volatile storage or other kinds of data
// that would persist after the SNES is turned off. libsnes exposes this
// information via the snes_get_memory_data() and snes_get_memory_size()
// functions. Since version 1.2 of the libsnes API, libsnes also exposes the
// contents of volatile storage such as WRAM and VRAM.
//
// After a cartridge is loaded, call snes_get_memory_size() and
// snes_get_memory_data() with the various SNES_MEMORY_* constants to determine
// which kinds of non-volatile storage the cartridge supports - unsupported
// storage types will have a size of 0 and the data pointer NULL.
//
// If you have non-volatile storage data from a previous run, you can memcpy()
// the data from your storage into the buffer described by the data and size
// values before calling snes_run(). Do not load non-volatile storage data if
// the size of the data you have is different from the size returned by
// snes_get_memory_size().
//
// Before calling snes_unload_cartridge(), you should copy the contents of the
// relevant storage buffers into a file (or some non-volatile storage of your
// own) so that you can load it the next time you load the same cartridge into
// the emulated SNES. Do not call free() on the storage buffers; they will be
// handled by libsnes. Note: It is not necessary to store the contents of
// volatile storage; the emulated SNES expects information in volatile storage
// to be lost (hence the name 'volatile').
//
// Because non-volatile storage is read and written by the software running on
// the emulated SNES, it should be compatible between different versions of
// different emulators running on different platforms, unlike save states.
//
// The various kinds of non-volatile storage and their uses are:
//
//    SNES_MEMORY_CARTRIDGE_RAM:
//      Standard battery-backed static RAM (SRAM). Traditionally, the SRAM for
//      a ROM image named "foo.sfc" is stored in a file named "foo.srm" beside
//      it.
//
//    SNES_MEMORY_CARTRIDGE_RTC:
//      Real-time clock data. Traditionally, the RTC data for a ROM image named
//      "foo.sfc" is stored in a file named "foo.rtc" beside it.
//
//    SNES_MEMORY_BSX_RAM:
//      RAM data used with the BS-X base cartridge.
//
//    SNES_MEMORY_BSX_PRAM:
//      PRAM data used with the BS-X base cartridge.
//
//    SNES_MEMORY_SUFAMI_TURBO_A_RAM:
//      RAM data stored in the mini-cartridge inserted into Slot A of the
//      SuFami Turbo base cartridge.
//
//    SNES_MEMORY_SUFAMI_TURBO_B_RAM:
//      RAM data stored in the mini-cartridge inserted into Slot B of the
//      SuFami Turbo base cartridge.
//
//    SNES_MEMORY_GAME_BOY_RAM:
//      Standard battery-backed static RAM (SRAM) in the Gameboy cartridge
//      inserted into the Super Game Boy base cartridge. Not all Gameboy games
//      have SRAM.
//
//    SNES_MEMORY_GAME_BOY_RTC:
//      Real-time clock data in the Gameboy cartridge inserted into the Super
//      Game Boy base cartridge. Not all Gameboy games have an RTC.
//
// The various kinds of volatile storage are:
//
//    SNES_MEMORY_WRAM:
//      Working RAM, accessible by the CPU. SNES software tends to keep runtime
//      information in here; games' life-bars and inventory contents and so
//      forth are in here somewhere.
//
//    SNES_MEMORY_APURAM:
//      RAM accessible by the Audio Processing Unit. Contains audio samples,
//      music data and the code responsible for feeding the right notes to the
//      DSP at the right times.
//
//    SNES_MEMORY_VRAM:
//      Video RAM. Stores almost everything related to video output, including
//      the patterns used for each tile and sprite, tilemaps for each
//      background. The exact format used depends on the current video mode of
//      the emulated SNES.
//
//    SNES_MEMORY_OAM:
//      Object Attribute Memory. Stores the location, orientation and priority
//      of all the sprites the SNES displays.
//
//    SNES_MEMORY_CGRAM:
//      Color Generator RAM. Contains the colour palettes used by tiles and
//      sprites. Each palette entry is stored in a 16-bit int, in the standard
//      XBGR1555 format.

// snes_get_memory_data:
//
//    Returns a pointer to the given non-volatile storage buffer.
//
//    This requires that a cartridge is loaded.
//
//    Parameters:
//
//      id:
//          One of the SNES_MEMORY_* constants.
//
//    Returns:
//
//      A pointer to the memory buffer used for storing the given type of data.
//      The size of the buffer can be obtained from snes_get_memory_size().
//
//      If NULL, the loaded cartridge does not store the given type of data.

EXPORT uint8_t* snes_get_memory_data(unsigned id);

// snes_get_memory_size:
//
//    Returns the size of the given non-volatile storage buffer.
//
//    This requires that a cartridge is loaded.
//
//    Parameters:
//
//      id:
//          One of the SNES_MEMORY_* constants.
//
//    Returns:
//
//      The size of the memory buffer used for storing the given type of data.
//      A pointer to the buffer can be obtained from snes_get_memory_data().
//
//      If 0, the loaded cartridge does not store the given type of data.

EXPORT unsigned snes_get_memory_size(unsigned id);

////////////////////////////////////////////////////////////////////////////}}}

#ifdef __cplusplus
}
#endif

#endif
