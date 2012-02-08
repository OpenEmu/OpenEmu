// Nintendo Game Boy sound hardware emulator with save state support

// Gb_Snd_Emu 0.2.0
#ifndef GB_APU_H
#define GB_APU_H

#include "Gb_Oscs.h"

struct gb_apu_state_t;

/* 0: Square 1, 1: Square 2, 2: Wave, 3: Noise */
#define OSC_COUNT 4

/* Resets hardware to initial power on state BEFORE boot ROM runs. Mode selects*/
/* sound hardware. Additional AGB wave features are enabled separately.*/
#define MODE_DMG	0
#define MODE_CGB	1
#define MODE_AGB	2

#define START_ADDR	0xFF10
#define END_ADDR	0xFF3F

/* Reads and writes must be within the START_ADDR to END_ADDR range, inclusive.*/
/* Addresses outside this range are not mapped to the sound hardware.*/
#define REGISTER_COUNT	48
#define REGS_SIZE 64

/* Clock rate that sound hardware runs at.*/
/* 4194304 * 4 */
#define CLOCK_RATE 16777216

class Gb_Apu
{
	public:

	// Sets buffer(s) to generate sound into. If left and right are NULL, output is mono.
	// If all are NULL, no output is generated but other emulation still runs.
	// If chan is specified, only that channel's output is changed, otherwise all are.
	void set_output( Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right,
			int chan);

	void reset( uint32_t mode, bool agb_wave);


	// Times are specified as the number of clocks since the beginning of the
	// current time frame.

	// Emulates CPU write of data to addr at specified time.
	void write_register( int32_t time, unsigned addr, int data );

	// Emulates CPU read from addr at specified time.
	int read_register( int32_t time, unsigned addr );

	// Sound adjustments

	// Sets overall volume, where 1.0 is normal.
	void volume( double );

	// If true, reduces clicking by disabling DAC biasing. Note that this reduces
	// emulation accuracy, since the clicks are authentic.
	void reduce_clicks( bool reduce);

	// Save states

	// Saves full emulation state to state_out. Data format is portable and
	// includes some extra space to avoid expansion in case more state needs
	// to be stored in the future.
	void save_state( gb_apu_state_t* state_out );

	// Loads state. You should call reset() BEFORE this.
	const char * load_state( gb_apu_state_t const& in );
	int32_t last_time;          // time sound emulator has been run to
	int32_t     frame_time;     // time of next frame sequencer action
	Gb_Apu();
	void run_until_( int32_t );
	private:
	// noncopyable
	Gb_Apu( const Gb_Apu& );
	Gb_Apu& operator = ( const Gb_Apu& );

	Gb_Osc*     oscs [OSC_COUNT];
	int32_t frame_period;       // clocks between each frame sequencer step
	double      volume_;
	bool        reduce_clicks_;

	Gb_Sweep_Square square1;
	Gb_Square       square2;
	Gb_Wave         wave;
	Gb_Noise        noise;
	int             frame_phase;    // phase of next frame sequencer step
	uint8_t  regs [REGS_SIZE];// last values written to registers

	// large objects after everything else
	Blip_Synth  good_synth;
	Blip_Synth   med_synth;

	int calc_output( int osc ) const;
	void apply_stereo();
	void apply_volume();
	void synth_volume( int );
	void silence_osc( Gb_Osc& );
	void write_osc( int index, int reg, int old_data, int data );
	const char* save_load( gb_apu_state_t*, bool save );
	void save_load2( gb_apu_state_t*, bool save );
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
