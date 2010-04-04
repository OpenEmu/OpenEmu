// Private oscillators used by Gb_Apu

// Gb_Snd_Emu 0.2.0
#ifndef GB_OSCS_H
#define GB_OSCS_H

#include "blargg_common.h"
#include "Blip_Buffer.h"

#ifndef GB_APU_OVERCLOCK
	#define GB_APU_OVERCLOCK 1
#endif

#if GB_APU_OVERCLOCK & (GB_APU_OVERCLOCK - 1)
	#error "GB_APU_OVERCLOCK must be a power of 2"
#endif

class Gb_Osc {
protected:

	// 11-bit frequency in NRx3 and NRx4
	int frequency() const { return (regs [4] & 7) * 0x100 + regs [3]; }

	void update_amp( blip_time_t, int new_amp );
	int write_trig( int frame_phase, int max_len, int old_data );
public:

	enum { clk_mul  = GB_APU_OVERCLOCK };
	enum { dac_bias = 7 };

	Blip_Buffer*    outputs [4];// NULL, right, left, center
	Blip_Buffer*    output;     // where to output sound
	BOOST::uint8_t* regs;       // osc's 5 registers
	int             mode;       // mode_dmg, mode_cgb, mode_agb
	int             dac_off_amp;// amplitude when DAC is off
	int             last_amp;   // current amplitude in Blip_Buffer
	typedef Blip_Synth<blip_good_quality,1> Good_Synth;
	typedef Blip_Synth<blip_med_quality ,1> Med_Synth;
	Good_Synth const* good_synth;
	Med_Synth  const* med_synth;

	int         delay;      // clocks until frequency timer expires
	int         length_ctr; // length counter
	unsigned    phase;      // waveform phase (or equivalent)
	bool        enabled;    // internal enabled flag

	void clock_length();
	void reset();
};

class Gb_Env : public Gb_Osc {
public:
	int  env_delay;
	int  volume;
	bool env_enabled;

	void clock_envelope();
	bool write_register( int frame_phase, int reg, int old_data, int data );

	void reset()
	{
		env_delay = 0;
		volume    = 0;
		Gb_Osc::reset();
	}
protected:
	// Non-zero if DAC is enabled
	int dac_enabled() const { return regs [2] & 0xF8; }
private:
	void zombie_volume( int old, int data );
	int reload_env_timer();
};

class Gb_Square : public Gb_Env {
public:
	bool write_register( int frame_phase, int reg, int old_data, int data );
	void run( blip_time_t, blip_time_t );

	void reset()
	{
		Gb_Env::reset();
		delay = 0x40000000; // TODO: something less hacky (never clocked until first trigger)
	}
private:
	// Frequency timer period
	int period() const { return (2048 - frequency()) * (4 * clk_mul); }
};

class Gb_Sweep_Square : public Gb_Square {
public:
	int  sweep_freq;
	int  sweep_delay;
	bool sweep_enabled;
	bool sweep_neg;

	void clock_sweep();
	void write_register( int frame_phase, int reg, int old_data, int data );

	void reset()
	{
		sweep_freq    = 0;
		sweep_delay   = 0;
		sweep_enabled = false;
		sweep_neg     = false;
		Gb_Square::reset();
	}
private:
	enum { period_mask = 0x70 };
	enum { shift_mask  = 0x07 };

	void calc_sweep( bool update );
	void reload_sweep_timer();
};

class Gb_Noise : public Gb_Env {
public:

	int divider; // noise has more complex frequency divider setup

	void run( blip_time_t, blip_time_t );
	void write_register( int frame_phase, int reg, int old_data, int data );

	void reset()
	{
		divider = 0;
		Gb_Env::reset();
		delay = 4 * clk_mul; // TODO: remove?
	}
private:
	enum { period2_mask = 0x1FFFF };

	int period2_index() const { return regs [3] >> 4; }
	int period2( int base = 8 ) const { return base << period2_index(); }
	unsigned lfsr_mask() const { return (regs [3] & 0x08) ? ~0x4040 : ~0x4000; }
};

class Gb_Wave : public Gb_Osc {
public:
	int sample_buf; // last wave RAM byte read (hardware has this as well)

	void write_register( int frame_phase, int reg, int old_data, int data );
	void run( blip_time_t, blip_time_t );

	// Reads/writes wave RAM
	int read( unsigned addr ) const;
	void write( unsigned addr, int data );

	void reset()
	{
		sample_buf = 0;
		Gb_Osc::reset();
	}

private:
	enum { bank40_mask = 0x40 };
	enum { bank_size   = 32 };

	int agb_mask;               // 0xFF if AGB features enabled, 0 otherwise
	BOOST::uint8_t* wave_ram;   // 32 bytes (64 nybbles), stored in APU

	friend class Gb_Apu;

	// Frequency timer period
	int period() const { return (2048 - frequency()) * (2 * clk_mul); }

	// Non-zero if DAC is enabled
	int dac_enabled() const { return regs [0] & 0x80; }

	void corrupt_wave();

	BOOST::uint8_t* wave_bank() const { return &wave_ram [(~regs [0] & bank40_mask) >> 2 & agb_mask]; }

	// Wave index that would be accessed, or -1 if no access would occur
	int access( unsigned addr ) const;
};

inline int Gb_Wave::read( unsigned addr ) const
{
	int index = access( addr );
	return (index < 0 ? 0xFF : wave_bank() [index]);
}

inline void Gb_Wave::write( unsigned addr, int data )
{
	int index = access( addr );
	if ( index >= 0 )
		wave_bank() [index] = data;;
}

#endif
