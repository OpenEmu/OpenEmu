#ifndef _freeverb_
#define _freeverb_

// Reverb model tuning values
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

const int   numcombs        = 8;
const int   numallpasses    = 4;
const float muted           = 0;
const float fixedgain       = 0.015f;
const float scalewet        = 3;
const float scaledry        = 2;
const float scaledamp       = 0.4f;
const float scaleroom       = 0.28f;
const float offsetroom      = 0.7f;
const float initialroom     = 0.5f;
const float initialdamp     = 0.5f;
const float initialwet      = 1/scalewet;
const float initialdry      = 0;
const float initialwidth    = 1;
const float initialmode     = 0;
const float freezemode      = 0.5f;
const int   stereospread    = 23;

const int combtuning[]      = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
const int allpasstuning[]   = {556, 441, 341, 225};

// Macro for killing denormalled numbers
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// Based on IS_DENORMAL macro by Jon Watte
// This code is public domain

static inline float undenormalise(float x) {
	union {
		float f;
		unsigned int i;
	} u;
	u.f = x;
	if ((u.i & 0x7f800000) == 0) {
		return 0.0f;
	}
	return x;
}

// Allpass filter declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

class allpass
{
public:
	                allpass();
	        void    setbuffer(float *buf, int size);
	        void    deletebuffer();
	inline  float   process(float inp);
	        void    mute();
	        void    setfeedback(float val);
	        float   getfeedback();
// private:
	float   feedback;
	float   *buffer;
	int     bufsize;
	int     bufidx;
};


// Big to inline - but crucial for speed

inline float allpass::process(float input)
{
	float output;
	float bufout;

	bufout = undenormalise(buffer[bufidx]);

	output = -input + bufout;
	buffer[bufidx] = input + (bufout*feedback);

	if (++bufidx>=bufsize) bufidx = 0;

	return output;
}

// Comb filter class declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

class comb
{
public:
	                comb();
	        void    setbuffer(float *buf, int size);
	        void    deletebuffer();
	inline  float   process(float inp);
	        void    mute();
	        void    setdamp(float val);
	        float   getdamp();
	        void    setfeedback(float val);
	        float   getfeedback();
private:
	float   feedback;
	float   filterstore;
	float   damp1;
	float   damp2;
	float   *buffer;
	int     bufsize;
	int     bufidx;
};


// Big to inline - but crucial for speed

inline float comb::process(float input)
{
	float output;

	output = undenormalise(buffer[bufidx]);

	filterstore = undenormalise((output*damp2) + (filterstore*damp1));

	buffer[bufidx] = input + (filterstore*feedback);

	if (++bufidx>=bufsize) bufidx = 0;

	return output;
}

// Reverb model declaration
//
// Written by Jezar at Dreampoint, June 2000
// Modifications by Jerome Fisher, 2009
// http://www.dreampoint.co.uk
// This code is public domain

class revmodel
{
public:
			       revmodel(float scaletuning);
						 ~revmodel();
			void   mute();
			void   process(const float *inputL, const float *inputR, float *outputL, float *outputR, long numsamples);
			void   setroomsize(float value);
			float  getroomsize();
			void   setdamp(float value);
			float  getdamp();
			void   setwet(float value);
			float  getwet();
			void   setdry(float value);
			float  getdry();
			void   setwidth(float value);
			float  getwidth();
			void   setmode(float value);
			float  getmode();
			void   setfiltval(float value);
private:
			void   update();
private:
	float  gain;
	float  roomsize,roomsize1;
	float  damp,damp1;
	float  wet,wet1,wet2;
	float  dry;
	float  width;
	float  mode;

	// LPF stuff
	float filtval;
	float filtprev1;
	float filtprev2;

	// Comb filters
	comb   combL[numcombs];
	comb   combR[numcombs];

	// Allpass filters
	allpass	allpassL[numallpasses];
	allpass	allpassR[numallpasses];
};

#endif//_freeverb_
