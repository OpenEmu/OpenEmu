// Private oscillators used by Sms_Apu

// Sms_Snd_Emu 0.1.4
#ifndef SMS_OSCS_H
#define SMS_OSCS_H

#include <blip/Blip_Buffer.h>

struct Sms_Osc
{
	Blip_Buffer* outputs [4]; // NULL, right, left, center
	Blip_Buffer* output;
	int output_select;
	
	int delay;
	int last_amp;
	int volume;
	
	Sms_Osc();
	void reset();
};

struct Sms_Square : Sms_Osc
{
	int period;
	int phase;
	
	typedef Blip_Synth<blip_good_quality,1> Synth;
	const Synth* synth;
	
	void reset();
	void run( blip_time_t, blip_time_t );
};

struct Sms_Noise : Sms_Osc
{
	const int* period;
	unsigned shifter;
	unsigned feedback;
	
	typedef Blip_Synth<blip_med_quality,1> Synth;
	Synth synth;
	
	void reset();
	void run( blip_time_t, blip_time_t );
};

#endif
