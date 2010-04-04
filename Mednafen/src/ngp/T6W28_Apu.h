// T6W28_Snd_Emu

#ifndef SMS_APU_H
#define SMS_APU_H

typedef long sms_time_t; // clock cycle count

#include "T6W28_Oscs.h"

typedef struct
{
	int32 sq_period[3];
	int32 sq_phase[3];
	int32 noise_period;
	uint32 noise_shifter;
	uint32 noise_tap;

	int32 volume_left[4];
	int32 volume_right[4];
	uint8 latch_left, latch_right;
} T6W28_ApuState;

class T6W28_Apu {
public:
	// Set overall volume of all oscillators, where 1.0 is full volume
	void volume( double );
	
	// Set treble equalization
	void treble_eq( const blip_eq_t& );
	
	// Outputs can be assigned to a single buffer for mono output, or to three
	// buffers for stereo output (using Stereo_Buffer to do the mixing).
	
	// Assign all oscillator outputs to specified buffer(s). If buffer
	// is NULL, silences all oscillators.
	void output( Blip_Buffer* mono );
	void output( Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right );
	
	// Assign single oscillator output to buffer(s). Valid indicies are 0 to 3,
	// which refer to Square 1, Square 2, Square 3, and Noise. If buffer is NULL,
	// silences oscillator.
	enum { osc_count = 4 };
	void osc_output( int index, Blip_Buffer* mono );
	void osc_output( int index, Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right );
	
	// Reset oscillators and internal state
	void reset();
	
	// Write to data port
	void write_data_left( sms_time_t, int );
	void write_data_right( sms_time_t, int );

	// Run all oscillators up to specified time, end current frame, then
	// start a new frame at time 0. Returns true if any oscillators added
	// sound to one of the left/right buffers, false if they only added
	// to the center buffer.
	bool end_frame( sms_time_t );

	T6W28_ApuState *save_state(void);
	void load_state(T6W28_ApuState*);
public:
	T6W28_Apu();
	~T6W28_Apu();
private:
	// noncopyable
	T6W28_Apu( const T6W28_Apu& );
	T6W28_Apu& operator = ( const T6W28_Apu& );
	
	T6W28_Osc*    oscs [osc_count];
	T6W28_Square  squares [3];
	T6W28_Square::Synth square_synth; // used by squares
	sms_time_t  last_time;
	int         latch_left, latch_right;
	T6W28_Noise   noise;
	
	void run_until( sms_time_t );
};

inline void T6W28_Apu::output( Blip_Buffer* b ) { output( b, b, b ); }

inline void T6W28_Apu::osc_output( int i, Blip_Buffer* b ) { osc_output( i, b, b, b ); }

#endif

