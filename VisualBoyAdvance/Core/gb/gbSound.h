#ifndef GBSOUND_H
#define GBSOUND_H

// GB sound emulation

// See Sound.h for sound setup/options

//// GB sound options

void gbSoundSetSampleRate( long sampleRate );

// Manages declicking mode. When enabled, clicks are reduced. Note that clicks
// are normal for GB and GBC sound hardware.
void gbSoundSetDeclicking( bool enable );
bool gbSoundGetDeclicking();

// Effects configuration
struct gb_effects_config_t
{
	bool enabled;   // false = disable all effects

	float echo;     // 0.0 = none, 1.0 = lots
	float stereo;   // 0.0 = channels in center, 1.0 = channels on left/right
	bool surround;  // true = put some channels in back
};

// Changes effects configuration
void gbSoundConfigEffects( gb_effects_config_t const& );
extern gb_effects_config_t gb_effects_config; // current configuration


//// GB sound emulation

// GB sound registers
#define NR10 0xff10
#define NR11 0xff11
#define NR12 0xff12
#define NR13 0xff13
#define NR14 0xff14
#define NR21 0xff16
#define NR22 0xff17
#define NR23 0xff18
#define NR24 0xff19
#define NR30 0xff1a
#define NR31 0xff1b
#define NR32 0xff1c
#define NR33 0xff1d
#define NR34 0xff1e
#define NR41 0xff20
#define NR42 0xff21
#define NR43 0xff22
#define NR44 0xff23
#define NR50 0xff24
#define NR51 0xff25
#define NR52 0xff26

// Resets emulated sound hardware
void gbSoundReset();

// Emulates write to sound hardware
void gbSoundEvent( u16 address, int data );
#define SOUND_EVENT gbSoundEvent

// Emulates read from sound hardware
u8 gbSoundRead( u16 address );

// Notifies emulator that SOUND_CLOCK_TICKS clocks have passed
void gbSoundTick();
extern int SOUND_CLOCK_TICKS;   // Number of 16.8 MHz clocks between calls to gbSoundTick()
extern int soundTicks;          // Number of 16.8 MHz clocks until gbSoundTick() will be called

// Saves/loads emulator state
void gbSoundSaveGame( gzFile out );
void gbSoundReadGame( int version, gzFile in );

#endif // GBSOUND_H
