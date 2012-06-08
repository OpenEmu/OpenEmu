// YM2612 FM sound chip emulator interface
#ifndef YM2612_EMU_H
#define YM2612_EMU_H

#include "../../mednafen.h"
#include "../../lepacker.h"

struct Ym2612_Impl;

class Ym2612_Emu  {
	Ym2612_Impl* impl;
public:
	Ym2612_Emu();
	~Ym2612_Emu();

	// Reset to power-up state
	void reset();
	
	// Mute voice n if bit n (1 << n) of mask is set
	enum { channel_count = 6 };
	void mute_voices( int mask );
	
	// Write addr to register 0 then data to register 1
	void write0( int addr, int data );
	
	// Write addr to register 2 then data to register 3
	void write1( int addr, int data );
	
	int read(void);

	// Run and add nt samples into current output buffer contents
	typedef short sample_t;
	enum { out_chan_count = 2 }; // stereo
	void run( sample_t* out );

	void serialize(MDFN::LEPacker &slizer, bool load);
};

#endif
