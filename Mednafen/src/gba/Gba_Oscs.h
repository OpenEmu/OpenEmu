
// Private oscillators used by Gba_Apu

// Gba_Snd_Emu 0.1.1. Copyright (C) 2003-2004 Shay Green. GNU GPL license.

#ifndef GB_OSCS_H
#define GB_OSCS_H

#include <blip/Blip_Buffer.h>
#include "../types.h"

struct Gba_Osc
{
	Blip_Buffer* outputs [4]; // NULL, right, left, center
	Blip_Buffer* output;
	int output_select;
	
	int delay;
	int last_amp;
	int period;
	int volume;
	int frequency;
	int length;
	int new_length;
	int ME;
	bool8 enabled;
	bool8 length_enabled;
	int peradjust;
	
	int gbamode;

	Gba_Osc();
	virtual ~Gba_Osc() { };
	void clock_length();
	void reset();
	void setME(int s);

	virtual void run( gb_time_t begin, gb_time_t end ) = 0;
	virtual void write_register( int reg, int value );
};

struct Gba_Env : Gba_Osc {
	int env_period;
	int env_dir;
	int env_delay;
	int new_env_period;
	int new_env_dir;
	int new_volume;
	
	Gba_Env();
	void reset();
	void clock_envelope();
	void write_register( int, int );
};

struct Gba_Square : Gba_Env
{
	int phase;
	int duty;
	
	int sweep_period;
	int sweep_delay;
	int sweep_shift;
	int sweep_dir;
	int sweep_freq;
	bool has_sweep;
	
	typedef Blip_Synth<blip_good_quality,15 * 2> Synth;
	const Synth* synth;
	
	Gba_Square();
	void reset();
	void run( gb_time_t, gb_time_t );
	void write_register( int, int );
	void clock_sweep();
};

struct Gba_Wave : Gba_Osc
{
	unsigned char beepie;
	int volume_shift;
	unsigned wave_pos;
	int ws_offset, ws_mask;
	enum { wave_size = 32 };
	uint8_t wave [wave_size * 2]; // wave_size for GB, wave_size * 2 for GBA;
	
	typedef Blip_Synth<blip_med_quality,15 * 2> Synth;
	Synth synth;
	
	Gba_Wave();
	void reset();
	void run( gb_time_t, gb_time_t );
	void write_register( int, int );
};

struct Gba_Noise : Gba_Env
{
	unsigned bits;
	int tap;
	
	typedef Blip_Synth<blip_low_quality,15 * 2> Synth;
	Synth synth;
	
	Gba_Noise();
	void reset();
	void run( gb_time_t, gb_time_t );
	void write_register( int, int );
};

#endif

