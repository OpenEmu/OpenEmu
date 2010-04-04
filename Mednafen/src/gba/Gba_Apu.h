
// Nintendo GameBoy PAPU sound chip emulator

// Gba_Snd_Emu 0.1.1. Copyright (C) 2003-2004 Shay Green. GNU GPL license.

#ifndef GB_APU_H
#define GB_APU_H

typedef long gb_time_t;     // clock cycle count
typedef unsigned gb_addr_t; // 16-bit address

#include <inttypes.h>
#include <string.h>

#include "Gba_Oscs.h"

class Gba_Apu {
public:
	Gba_Apu();
	~Gba_Apu();
	
	int gbamode;
	void setgba(int s);

	// Overall volume of all oscillators, where 1.0 is full volume.
	void volume( double );
	
	// Treble equalization (see notes.txt).
	void treble_eq( const blip_eq_t& );
	
	// Reset oscillators and internal state.
	void reset();
	
	// Assign all oscillator outputs to specified buffer(s). If buffer
	// is NULL, silence all oscillators.
	void output( Blip_Buffer* mono );
	void output( Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right );
	
	// Assign oscillator output to buffer(s). Valid indicies are 0 to
	// osc_count - 1, which refer to Square 1, Square 2, Wave, and
	// Noise, respectively. If buffer is NULL, silence oscillator.
	enum { osc_count = 4 };
	void osc_output( int index, Blip_Buffer* mono );
	void osc_output( int index, Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right );
	
	// Reads and writes at addr must satisfy start_addr <= addr <= end_addr
	enum { start_addr = 0xff10 };
	enum { end_addr   = 0xff3f };
	
	// Write 'data' to address at specified time. Previous writes and reads
	// within the current frame must not have specified a time later
	// than 't'.
	void write_register( gb_time_t, gb_addr_t, int data );
	
	// Write from address at specified time. Previous writes and reads within
	// the current frame must not have specified a time later than 't'.
	int read_register( gb_time_t, gb_addr_t );
	
	// Run all oscillators up to specified time, end current frame, then
	// start a new frame at time 0. Return true if any oscillators added
	// sound to one of the left/right buffers, false if they only added
	// to the center buffer.
	bool end_frame( gb_time_t );
	
	void dirty(void);

	static void begin_debug_log();
//private:
	// noncopyable
	Gba_Apu( const Gba_Apu& );
	Gba_Apu& operator = ( const Gba_Apu& );
	
	Gba_Osc*     oscs [osc_count];
	gb_time_t   next_frame_time;
	gb_time_t   last_time;
	int         frame_count;
	bool        stereo_found;
	
	Gba_Square   square1;
	Gba_Square   square2;
	Gba_Wave     wave;
	Gba_Noise    noise;
	Gba_Square::Synth square_synth; // shared between squares
	uint8_t regs [end_addr - start_addr + 1];
	
	void run_until( gb_time_t );
};

	inline void Gba_Apu::output( Blip_Buffer* mono ) {
		output( mono, NULL, NULL );
	}
	
	inline void Gba_Apu::osc_output( int index, Blip_Buffer* mono ) {
		osc_output( index, mono, NULL, NULL );
	}

#endif

