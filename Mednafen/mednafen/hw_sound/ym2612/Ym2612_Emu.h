// YM2612 FM sound chip emulator interface

// Game_Music_Emu 0.5.2
#ifndef YM2612_EMU_H
#define YM2612_EMU_H

struct Ym2612_Impl;

class Ym2612_Emu  {
	Ym2612_Impl* impl;
public:
	Ym2612_Emu() { impl = 0; }
	~Ym2612_Emu();
	
	// Set output sample rate and chip clock rates, in Hz. Returns non-zero
	// if error.
	const char* set_rate(void);
	
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

	// Run and add pair_count samples into current output buffer contents
	typedef short sample_t;
	enum { out_chan_count = 2 }; // stereo
	void run( int pair_count, sample_t* out );

	unsigned int get_state_max_len(void);
	void save_state(unsigned char *buffer);
	void load_state(const unsigned char *buffer);
};

#endif
