// Sega Master System SN76489 PSG sound chip emulator

// Sms_Snd_Emu 0.1.4
#ifndef SMS_APU_H
#define SMS_APU_H

#include "Sms_Oscs.h"

typedef struct
{
        int32 sq_period[3];
        int32 sq_phase[3];
        int32 noise_period;
        uint32 noise_shifter;
        uint32 noise_feedback;

        int32 volume[4];
	uint8 ggstereo;
        uint8 latch;
} Sms_ApuState;

class Sms_Apu {
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
	void reset( unsigned noise_feedback = 0, int noise_width = 0 );
	
	// Write GameGear left/right assignment byte
	void write_ggstereo( blip_time_t, int );
	
	// Write to data port
	void write_data( blip_time_t, int );
	
	// Run all oscillators up to specified time, end current frame, then
	// start a new frame at time 0.
	void end_frame( blip_time_t );

        void save_state(Sms_ApuState *);
        void load_state(const Sms_ApuState *);

public:
	Sms_Apu();
	~Sms_Apu();
private:
	// noncopyable
	Sms_Apu( const Sms_Apu& );
	Sms_Apu& operator = ( const Sms_Apu& );
	
	Sms_Osc*    oscs [osc_count];
	Sms_Square  squares [3];
	Sms_Square::Synth square_synth; // used by squares
	blip_time_t last_time;
	int         latch;
	Sms_Noise   noise;
	unsigned    noise_feedback;
	unsigned    looped_feedback;
	unsigned int ggstereo_save;
	
	void run_until( blip_time_t );
};

struct sms_apu_state_t
{
	unsigned char regs [8] [2];
	unsigned char latch;
};

inline void Sms_Apu::output( Blip_Buffer* b ) { output( b, b, b ); }

inline void Sms_Apu::osc_output( int i, Blip_Buffer* b ) { osc_output( i, b, b, b ); }

#endif
