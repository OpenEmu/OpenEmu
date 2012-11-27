// Allpass filter implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#include "freeverb.h"

allpass::allpass()
{
	bufidx = 0;
}

void allpass::setbuffer(float *buf, int size)
{
	buffer = buf;
	bufsize = size;
}

void allpass::mute()
{
	for (int i=0; i<bufsize; i++)
		buffer[i]=0;
}

void allpass::setfeedback(float val)
{
	feedback = val;
}

float allpass::getfeedback()
{
	return feedback;
}

void allpass::deletebuffer()
{
	delete[] buffer;
	buffer = 0;
}
// Comb filter implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

comb::comb()
{
	filterstore = 0;
	bufidx = 0;
}

void comb::setbuffer(float *buf, int size)
{
	buffer = buf;
	bufsize = size;
}

void comb::mute()
{
	for (int i=0; i<bufsize; i++)
		buffer[i]=0;
}

void comb::setdamp(float val)
{
	damp1 = val;
	damp2 = 1-val;
}

float comb::getdamp()
{
	return damp1;
}

void comb::setfeedback(float val)
{
	feedback = val;
}

float comb::getfeedback()
{
	return feedback;
}

void comb::deletebuffer()
{
	delete[] buffer;
	buffer = 0;
}
// Reverb model implementation
//
// Written by Jezar at Dreampoint, June 2000
// Modifications by Jerome Fisher, 2009, 2011
// http://www.dreampoint.co.uk
// This code is public domain

revmodel::revmodel(float scaletuning)
{
	int i;
	int bufsize;

	// Allocate buffers for the components
	for (i = 0; i < numcombs; i++) {
		bufsize = int(scaletuning * combtuning[i]);
		combL[i].setbuffer(new float[bufsize], bufsize);
		bufsize += int(scaletuning * stereospread);
		combR[i].setbuffer(new float[bufsize], bufsize);
	}
	for (i = 0; i < numallpasses; i++) {
		bufsize = int(scaletuning * allpasstuning[i]);
		allpassL[i].setbuffer(new float[bufsize], bufsize);
		allpassL[i].setfeedback(0.5f);
		bufsize += int(scaletuning * stereospread);
		allpassR[i].setbuffer(new float[bufsize], bufsize);
		allpassR[i].setfeedback(0.5f);
	}

	// Set default values
	dry = initialdry;
	wet = initialwet*scalewet;
	damp = initialdamp*scaledamp;
	roomsize = (initialroom*scaleroom) + offsetroom;
	width = initialwidth;
	mode = initialmode;
	update();

	// Buffer will be full of rubbish - so we MUST mute them
	mute();
}

revmodel::~revmodel()
{
	int i;

	for (i = 0; i < numcombs; i++) {
		combL[i].deletebuffer();
		combR[i].deletebuffer();
	}
	for (i = 0; i < numallpasses; i++) {
		allpassL[i].deletebuffer();
		allpassR[i].deletebuffer();
	}
}

void revmodel::mute()
{
	int i;

	if (getmode() >= freezemode)
		return;

	for (i=0;i<numcombs;i++)
	{
		combL[i].mute();
		combR[i].mute();
	}
	for (i=0;i<numallpasses;i++)
	{
		allpassL[i].mute();
		allpassR[i].mute();
	}

	// Init LPF history
	filtprev1 = 0;
	filtprev2 = 0;
}

void revmodel::process(const float *inputL, const float *inputR, float *outputL, float *outputR, long numsamples)
{
	float outL,outR,input;

	while (numsamples-- > 0)
	{
		int i;

		outL = outR = 0;
		input = (*inputL + *inputR) * gain;

		// Implementation of 2-stage IIR single-pole low-pass filter
		// found at the entrance of reverb processing on real devices
		filtprev1 += (input - filtprev1) * filtval;
		filtprev2 += (filtprev1 - filtprev2) * filtval;
		input = filtprev2;

		int s = -1;
		// Accumulate comb filters in parallel
		for (i=0; i<numcombs; i++)
		{
			outL += s * combL[i].process(input);
			outR += s * combR[i].process(input);
			s = -s;
		}

		// Feed through allpasses in series
		for (i=0; i<numallpasses; i++)
		{
			outL = allpassL[i].process(outL);
			outR = allpassR[i].process(outR);
		}

		// Calculate output REPLACING anything already there
		*outputL = outL*wet1 + outR*wet2;
		*outputR = outR*wet1 + outL*wet2;

		inputL++;
		inputR++;
		outputL++;
		outputR++;
	}
}

void revmodel::update()
{
// Recalculate internal values after parameter change

	int i;

	wet1 = wet*(width/2 + 0.5f);
	wet2 = wet*((1-width)/2);

	if (mode >= freezemode)
	{
		roomsize1 = 1;
		damp1 = 0;
		gain = muted;
	}
	else
	{
		roomsize1 = roomsize;
		damp1 = damp;
		gain = fixedgain;
	}

	for (i=0; i<numcombs; i++)
	{
		combL[i].setfeedback(roomsize1);
		combR[i].setfeedback(roomsize1);
	}

	for (i=0; i<numcombs; i++)
	{
		combL[i].setdamp(damp1);
		combR[i].setdamp(damp1);
	}
}

// The following get/set functions are not inlined, because
// speed is never an issue when calling them, and also
// because as you develop the reverb model, you may
// wish to take dynamic action when they are called.

void revmodel::setroomsize(float value)
{
	roomsize = (value*scaleroom) + offsetroom;
	update();
}

float revmodel::getroomsize()
{
	return (roomsize-offsetroom)/scaleroom;
}

void revmodel::setdamp(float value)
{
	damp = value*scaledamp;
	update();
}

float revmodel::getdamp()
{
	return damp/scaledamp;
}

void revmodel::setwet(float value)
{
	wet = value*scalewet;
	update();
}

float revmodel::getwet()
{
	return wet/scalewet;
}

void revmodel::setdry(float value)
{
	dry = value*scaledry;
}

float revmodel::getdry()
{
	return dry/scaledry;
}

void revmodel::setwidth(float value)
{
	width = value;
	update();
}

float revmodel::getwidth()
{
	return width;
}

void revmodel::setmode(float value)
{
	mode = value;
	update();
}

float revmodel::getmode()
{
	if (mode >= freezemode)
		return 1;
	else
		return 0;
}

void revmodel::setfiltval(float value)
{
	filtval = value;
}
