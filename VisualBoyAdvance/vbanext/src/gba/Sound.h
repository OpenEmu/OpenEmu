#ifndef SOUND_H
#define SOUND_H

// Sound emulation setup/options and GBA sound emulation

#include "../System.h"

//// Setup/options (these affect GBA and GB sound)

// Initializes sound and returns true if successful. Sets sound quality to
// current value in soundQuality global.
bool soundInit();

// sets the Sound throttle
void soundSetThrottle(unsigned short throttle);

// Manages sound volume, where 1.0 is normal
void soundSetVolume( float );
float soundGetVolume();

// Manages muting bitmask. The bits control the following channels:
// 0x001 Pulse 1
// 0x002 Pulse 2
// 0x004 Wave
// 0x008 Noise
// 0x100 PCM 1
// 0x200 PCM 2
void soundSetEnable( int mask );
int  soundGetEnable();

// Pauses/resumes system sound output
void soundPause();
void soundResume();
extern bool soundPaused; // current paused state

// Cleans up sound. Afterwards, soundInit() can be called again.
void soundShutdown();

//// GBA sound options

long soundGetSampleRate();
void soundSetSampleRate(long sampleRate);

// Sound settings

//// GBA sound emulation

// GBA sound registers
#define SGCNT0_H 0x82
#define FIFOA_L 0xa0
#define FIFOA_H 0xa2
#define FIFOB_L 0xa4
#define FIFOB_H 0xa6

// Resets emulated sound hardware
void soundReset();

// Emulates write to sound hardware
void soundEvent_u8( int gb_addr, uint32_t addr, uint8_t  data );
void soundEvent_u8_parallel(int gb_addr[], uint32_t address[], uint8_t data[]);
void soundEvent_u16( uint32_t addr, uint16_t data );

// Notifies emulator that a timer has overflowed
void soundTimerOverflow( int which );

// Notifies emulator that SOUND_CLOCK_TICKS clocks have passed
void psoundTickfn();
extern int SOUND_CLOCK_TICKS;   // Number of 16.8 MHz clocks between calls to soundTick()
extern int soundTicks;          // Number of 16.8 MHz clocks until soundTick() will be called

// Saves/loads emulator state
void soundSaveGame( gzFile );
void soundReadGame( gzFile, int version );

#ifdef __LIBSNES__
void soundSaveGameMem(uint8_t *& data);
void soundReadGameMem(const uint8_t *& data, int version);
#endif

class Simple_Effects_Buffer;

void flush_samples(Simple_Effects_Buffer * buffer);

#endif // SOUND_H
