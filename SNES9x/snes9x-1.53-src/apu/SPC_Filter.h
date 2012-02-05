// Simple low-pass and high-pass filter to better match sound output of a SNES

// snes_spc 0.9.0
#ifndef SPC_FILTER_H
#define SPC_FILTER_H

#include "blargg_common.h"

struct SPC_Filter {
public:
	
	// Filters count samples of stereo sound in place. Count must be a multiple of 2.
	typedef short sample_t;
	void run( sample_t* io, int count );
	
// Optional features

	// Clears filter to silence
	void clear();

	// Sets gain (volume), where gain_unit is normal. Gains greater than gain_unit
	// are fine, since output is clamped to 16-bit sample range.
	enum { gain_unit = 0x100 };
	void set_gain( int gain );

	// Sets amount of bass (logarithmic scale)
	enum { bass_none =  0 };
	enum { bass_norm =  8 }; // normal amount
	enum { bass_max  = 31 };
	void set_bass( int bass );
	
public:
	SPC_Filter();
	BLARGG_DISABLE_NOTHROW
private:
	enum { gain_bits = 8 };
	int gain;
	int bass;
	struct chan_t { int p1, pp1, sum; };
	chan_t ch [2];
};

inline void SPC_Filter::set_gain( int g ) { gain = g; }

inline void SPC_Filter::set_bass( int b ) { bass = b; }

#endif
