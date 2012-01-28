// SNES SPC-700 APU emulator

// snes_spc 0.9.0
#ifndef SNES_SPC_H
#define SNES_SPC_H

#include "SPC_DSP.h"
#include "blargg_endian.h"

#ifdef DEBUGGER
#include "snes9x.h"
#include "display.h"
#include "debug.h"
#endif

struct SNES_SPC {
public:
	typedef BOOST::uint8_t uint8_t;
	
	// Must be called once before using
	blargg_err_t init();
	
	// Sample pairs generated per second
	enum { sample_rate = 32000 };
	
// Emulator use
	
	// Sets IPL ROM data. Library does not include ROM data. Most SPC music files
	// don't need ROM, but a full emulator must provide this.
	enum { rom_size = 0x40 };
	void init_rom( uint8_t const rom [rom_size] );

	// Sets destination for output samples
	typedef short sample_t;
	void set_output( sample_t* out, int out_size );

	// Number of samples written to output since last set
	int sample_count() const;

	// Resets SPC to power-on state. This resets your output buffer, so you must
	// call set_output() after this.
	void reset();

	// Emulates pressing reset switch on SNES. This resets your output buffer, so
	// you must call set_output() after this.
	void soft_reset();

	// 1024000 SPC clocks per second, sample pair every 32 clocks
	typedef int time_t;
	enum { clock_rate = 1024000 };
	enum { clocks_per_sample = 32 };
	
	// Emulated port read/write at specified time
	enum { port_count = 4 };
	int  read_port ( time_t, int port );
	void write_port( time_t, int port, int data );

	// Runs SPC to end_time and starts a new time frame at 0
	void end_frame( time_t end_time );
	
// Sound control
	
	// Mutes voices corresponding to non-zero bits in mask (issues repeated KOFF events).
	// Reduces emulation accuracy.
	enum { voice_count = 8 };
	void mute_voices( int mask );
	
	// If true, prevents channels and global volumes from being phase-negated.
	// Only supported by fast DSP.
	void disable_surround( bool disable = true );
	
	// Sets tempo, where tempo_unit = normal, tempo_unit / 2 = half speed, etc.
	enum { tempo_unit = 0x100 };
	void set_tempo( int );

// SPC music files

	// Loads SPC data into emulator
	enum { spc_min_file_size = 0x10180 };
	enum { spc_file_size     = 0x10200 };
	blargg_err_t load_spc( void const* in, long size );
	
	// Clears echo region. Useful after loading an SPC as many have garbage in echo.
	void clear_echo();

	// Plays for count samples and write samples to out. Discards samples if out
	// is NULL. Count must be a multiple of 2 since output is stereo.
	blargg_err_t play( int count, sample_t* out );
	
	// Skips count samples. Several times faster than play() when using fast DSP.
	blargg_err_t skip( int count );
	
// State save/load (only available with accurate DSP)

#if !SPC_NO_COPY_STATE_FUNCS
	// Saves/loads state
	enum { state_size = 68 * 1024L }; // maximum space needed when saving
	typedef SPC_DSP::copy_func_t copy_func_t;
	void copy_state( unsigned char** io, copy_func_t );
	
	// Writes minimal header to spc_out
	static void init_header( void* spc_out );

	// Saves emulator state as SPC file data. Writes spc_file_size bytes to spc_out.
	// Does not set up SPC header; use init_header() for that.
	void save_spc( void* spc_out );

	// Returns true if new key-on events occurred since last check. Useful for
	// trimming silence while saving an SPC.
	bool check_kon();
#endif

//// Snes9x Accessor

	void	spc_allow_time_overflow( bool );

	void    dsp_set_spc_snapshot_callback( void (*callback) (void) );
	void    dsp_dump_spc_snapshot( void );
	void    dsp_set_stereo_switch( int );
	uint8_t dsp_reg_value( int, int );
	int     dsp_envx_value( int );

//// Snes9x Debugger

#ifdef DEBUGGER
	void	debug_toggle_trace( void );
	bool    debug_is_enabled( void );
	void	debug_do_trace( int, int, int, uint8_t const *, uint8_t *, int, int, int, int );
	void	debug_op_print( char *, int, int, int, uint8_t const *, uint8_t *, int, int, int, int );
	void	debug_io_print( char * );
#endif

public:
	BLARGG_DISABLE_NOTHROW
	
	typedef BOOST::uint16_t uint16_t;
	
	// Time relative to m_spc_time. Speeds up code a bit by eliminating need to
	// constantly add m_spc_time to time from CPU. CPU uses time that ends at
	// 0 to eliminate reloading end time every instruction. It pays off.
	typedef int rel_time_t;
	
	struct Timer
	{
		rel_time_t next_time; // time of next event
		int prescaler;
		int period;
		int divider;
		int enabled;
		int counter;
	};
	enum { reg_count = 0x10 };
	enum { timer_count = 3 };
	enum { extra_size = SPC_DSP::extra_size };
	
	enum { signature_size = 35 };
	
private:
	SPC_DSP dsp;
	
	#if SPC_LESS_ACCURATE
		static signed char const reg_times_ [256];
		signed char reg_times [256];
	#endif
	
	struct state_t
	{
		Timer timers [timer_count];
		
		uint8_t smp_regs [2] [reg_count];
		
		struct
		{
			int pc;
			int a;
			int x;
			int y;
			int psw;
			int sp;
		} cpu_regs;
		
		rel_time_t  dsp_time;
		time_t      spc_time;
		bool        echo_accessed;
		
		int         tempo;
		int         skipped_kon;
		int         skipped_koff;
		const char* cpu_error;
		
		int         extra_clocks;
		sample_t*   buf_begin;
		sample_t const* buf_end;
		sample_t*   extra_pos;
		sample_t    extra_buf [extra_size];
		
		int         rom_enabled;
		uint8_t     rom    [rom_size];
		uint8_t     hi_ram [rom_size];
		
		unsigned char cycle_table [256];
		
		struct
		{
			// padding to neutralize address overflow
			union {
				uint8_t padding1 [0x100];
				uint16_t align; // makes compiler align data for 16-bit access
			} padding1 [1];
			uint8_t ram      [0x10000];
			uint8_t padding2 [0x100];
		} ram;
	};
	state_t m;
	
	enum { rom_addr = 0xFFC0 };
	
	enum { skipping_time = 127 };
	
	// Value that padding should be filled with
	enum { cpu_pad_fill = 0xFF };
	
	enum {
        r_test     = 0x0, r_control  = 0x1,
        r_dspaddr  = 0x2, r_dspdata  = 0x3,
        r_cpuio0   = 0x4, r_cpuio1   = 0x5,
        r_cpuio2   = 0x6, r_cpuio3   = 0x7,
        r_f8       = 0x8, r_f9       = 0x9,
        r_t0target = 0xA, r_t1target = 0xB, r_t2target = 0xC,
        r_t0out    = 0xD, r_t1out    = 0xE, r_t2out    = 0xF
	};
	
	void timers_loaded();
	void enable_rom( int enable );
	void reset_buf();
	void save_extra();
	void load_regs( uint8_t const in [reg_count] );
	void ram_loaded();
	void regs_loaded();
	void reset_time_regs();
	void reset_common( int timer_counter_init );
	
	Timer* run_timer_      ( Timer* t, rel_time_t );
	Timer* run_timer       ( Timer* t, rel_time_t );
	int dsp_read           ( rel_time_t );
	void dsp_write         ( int data, rel_time_t );
	void cpu_write_smp_reg_( int data, rel_time_t, int addr );
	void cpu_write_smp_reg ( int data, rel_time_t, int addr );
	void cpu_write_high    ( int data, int i, rel_time_t );
	void cpu_write         ( int data, int addr, rel_time_t );
	int cpu_read_smp_reg   ( int i, rel_time_t );
	int cpu_read           ( int addr, rel_time_t );
	unsigned CPU_mem_bit   ( uint8_t const* pc, rel_time_t );
	
	bool check_echo_access ( int addr );
	uint8_t* run_until_( time_t end_time );
	
	struct spc_file_t
	{
		char    signature [signature_size];
		uint8_t has_id666;
		uint8_t version;
		uint8_t pcl, pch;
		uint8_t a;
		uint8_t x;
		uint8_t y;
		uint8_t psw;
		uint8_t sp;
		char    text [212];
		uint8_t ram [0x10000];
		uint8_t dsp [128];
		uint8_t unused [0x40];
		uint8_t ipl_rom [0x40];
	};

	static char const signature [signature_size + 1];
	
	void save_regs( uint8_t out [reg_count] );

// Snes9x timing hack
	bool allow_time_overflow;
// Snes9x debugger
#ifdef DEBUGGER
	FILE *apu_trace;
	bool debug_trace;
#endif
};

#include <assert.h>

inline int SNES_SPC::sample_count() const { return (m.extra_clocks >> 5) * 2; }

inline int SNES_SPC::read_port( time_t t, int port )
{
	assert( (unsigned) port < port_count );
	return run_until_( t ) [port];
}

inline void SNES_SPC::write_port( time_t t, int port, int data )
{
	assert( (unsigned) port < port_count );
	run_until_( t ) [0x10 + port] = data;
	m.ram.ram [0xF4 + port] = data;
}

inline void SNES_SPC::mute_voices( int mask ) { dsp.mute_voices( mask ); }
	
inline void SNES_SPC::disable_surround( bool disable ) { dsp.disable_surround( disable ); }

#if !SPC_NO_COPY_STATE_FUNCS
inline bool SNES_SPC::check_kon() { return dsp.check_kon(); }
#endif

inline void SNES_SPC::spc_allow_time_overflow( bool allow ) { allow_time_overflow = allow; }

#endif
