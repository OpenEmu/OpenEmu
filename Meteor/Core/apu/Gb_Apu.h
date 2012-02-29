// Nintendo Game Boy sound hardware emulator with save state support

// Gb_Snd_Emu 0.2.0
#ifndef GB_APU_H
#define GB_APU_H

#include "Gb_Oscs.h"

struct gb_apu_state_t;

class Gb_Apu {
public:
// Basics

	// Clock rate that sound hardware runs at.
	enum { clock_rate = 4194304 * GB_APU_OVERCLOCK };

	// Sets buffer(s) to generate sound into. If left and right are NULL, output is mono.
	// If all are NULL, no output is generated but other emulation still runs.
	// If chan is specified, only that channel's output is changed, otherwise all are.
	enum { osc_count = 4 }; // 0: Square 1, 1: Square 2, 2: Wave, 3: Noise
	void set_output( Blip_Buffer* center, Blip_Buffer* left = NULL, Blip_Buffer* right = NULL,
			int chan = osc_count );

	// Resets hardware to initial power on state BEFORE boot ROM runs. Mode selects
	// sound hardware. Additional AGB wave features are enabled separately.
	enum mode_t {
		mode_dmg,   // Game Boy monochrome
		mode_cgb,   // Game Boy Color
		mode_agb    // Game Boy Advance
	};
	void reset( mode_t mode = mode_cgb, bool agb_wave = false );

	// Reads and writes must be within the start_addr to end_addr range, inclusive.
	// Addresses outside this range are not mapped to the sound hardware.
	enum { start_addr = 0xFF10 };
	enum { end_addr   = 0xFF3F };
	enum { register_count = end_addr - start_addr + 1 };

	// Times are specified as the number of clocks since the beginning of the
	// current time frame.

	// Emulates CPU write of data to addr at specified time.
	void write_register( blip_time_t time, unsigned addr, int data );

	// Emulates CPU read from addr at specified time.
	int read_register( blip_time_t time, unsigned addr );

	// Emulates sound hardware up to specified time, ends current time frame, then
	// starts a new frame at time 0.
	void end_frame( blip_time_t frame_length );

// Sound adjustments

	// Sets overall volume, where 1.0 is normal.
	void volume( double );

	// If true, reduces clicking by disabling DAC biasing. Note that this reduces
	// emulation accuracy, since the clicks are authentic.
	void reduce_clicks( bool reduce = true );

	// Sets treble equalization.
	void treble_eq( blip_eq_t const& );

	// Treble and bass values for various hardware.
	enum {
		speaker_treble =  -47, // speaker on system
		speaker_bass   = 2000,
		dmg_treble     =    0, // headphones on each system
		dmg_bass       =   30,
		cgb_treble     =    0,
		cgb_bass       =  300, // CGB has much less bass
		agb_treble     =    0,
		agb_bass       =   30
	};

	// Sets frame sequencer rate, where 1.0 is normal. Meant for adjusting the
	// tempo in a game music player.
	void set_tempo( double );

// Save states

	// Saves full emulation state to state_out. Data format is portable and
	// includes some extra space to avoid expansion in case more state needs
	// to be stored in the future.
	void save_state( gb_apu_state_t* state_out );

	// Loads state. You should call reset() BEFORE this.
	blargg_err_t load_state( gb_apu_state_t const& in );

public:
	Gb_Apu();

	// Use set_output() in place of these
	BLARGG_DEPRECATED void output    (        Blip_Buffer* c                                 ) { set_output( c, c, c    ); }
	BLARGG_DEPRECATED void output    (        Blip_Buffer* c, Blip_Buffer* l, Blip_Buffer* r ) { set_output( c, l, r    ); }
	BLARGG_DEPRECATED void osc_output( int i, Blip_Buffer* c                                 ) { set_output( c, c, c, i ); }
	BLARGG_DEPRECATED void osc_output( int i, Blip_Buffer* c, Blip_Buffer* l, Blip_Buffer* r ) { set_output( c, l, r, i ); }

private:
	// noncopyable
	Gb_Apu( const Gb_Apu& );
	Gb_Apu& operator = ( const Gb_Apu& );

	Gb_Osc*     oscs [osc_count];
	blip_time_t last_time;          // time sound emulator has been run to
	blip_time_t frame_period;       // clocks between each frame sequencer step
	double      volume_;
	bool        reduce_clicks_;

	Gb_Sweep_Square square1;
	Gb_Square       square2;
	Gb_Wave         wave;
	Gb_Noise        noise;
	blip_time_t     frame_time;     // time of next frame sequencer action
	int             frame_phase;    // phase of next frame sequencer step
	enum { regs_size = register_count + 0x10 };
	BOOST::uint8_t  regs [regs_size];// last values written to registers

	// large objects after everything else
	Gb_Osc::Good_Synth  good_synth;
	Gb_Osc::Med_Synth   med_synth;

	void reset_lengths();
	void reset_regs();
	int calc_output( int osc ) const;
	void apply_stereo();
	void apply_volume();
	void synth_volume( int );
	void run_until_( blip_time_t );
	void run_until( blip_time_t );
	void silence_osc( Gb_Osc& );
	void write_osc( int index, int reg, int old_data, int data );
	const char* save_load( gb_apu_state_t*, bool save );
	void save_load2( gb_apu_state_t*, bool save );
	friend class Gb_Apu_Tester;
};

// Format of save state. Should be stable across versions of the library,
// with earlier versions properly opening later save states. Includes some
// room for expansion so the state size shouldn't increase.
struct gb_apu_state_t
{
#if GB_APU_CUSTOM_STATE
	// Values stored as plain int so your code can read/write them easily.
	// Structure can NOT be written to disk, since format is not portable.
	typedef int val_t;
#else
	// Values written in portable little-endian format, allowing structure
	// to be written directly to disk.
	typedef unsigned char val_t [4];
#endif

	enum { format0 = 0x50414247 };

	val_t format;   // format of all following data
	val_t version;  // later versions just add fields to end

	unsigned char regs [0x40];
	val_t frame_time;
	val_t frame_phase;

	val_t sweep_freq;
	val_t sweep_delay;
	val_t sweep_enabled;
	val_t sweep_neg;
	val_t noise_divider;
	val_t wave_buf;

	val_t delay      [4];
	val_t length_ctr [4];
	val_t phase      [4];
	val_t enabled    [4];

	val_t env_delay   [3];
	val_t env_volume  [3];
	val_t env_enabled [3];

	val_t unused  [13]; // for future expansion
};

#endif
