/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 *  This file is based on reSID, a MOS6581 SID emulator engine.
 *  Copyright (C) 2004  Dag Lem <resid@nimrod.no>
 */

#ifndef DISABLE_SID

#include "audio/softsynth/sid.h"
#include "audio/null.h"

namespace Resid {

// Fixpoint constants (16.16 bits).
const int SID::FIXP_SHIFT = 16;
const int SID::FIXP_MASK = 0xffff;

/*
 * WaveformGenerator
 */

WaveformGenerator::WaveformGenerator() {
	sync_source = this;

	reset();
}

void WaveformGenerator::set_sync_source(WaveformGenerator* source) {
	sync_source = source;
	source->sync_dest = this;
}

void WaveformGenerator::writeFREQ_LO(reg8 freq_lo) {
	freq = (freq & 0xff00) | (freq_lo & 0x00ff);
}

void WaveformGenerator::writeFREQ_HI(reg8 freq_hi) {
	freq = ((freq_hi << 8) & 0xff00) | (freq & 0x00ff);
}

void WaveformGenerator::writePW_LO(reg8 pw_lo) {
	pw = (pw & 0xf00) | (pw_lo & 0x0ff);
}

void WaveformGenerator::writePW_HI(reg8 pw_hi) {
	pw = ((pw_hi << 8) & 0xf00) | (pw & 0x0ff);
}

void WaveformGenerator::writeCONTROL_REG(reg8 control) {
	waveform = (control >> 4) & 0x0f;
	ring_mod = control & 0x04;
	sync = control & 0x02;

	reg8 test_next = control & 0x08;

	// Test bit set.
	if (test_next) {
		accumulator = 0;
		shift_register = 0;
	}
	// Test bit cleared.
	else if (test) {
		shift_register = 0x7ffff8;
	}

	test = test_next;

	// The gate bit is handled by the EnvelopeGenerator.
}

reg8 WaveformGenerator::readOSC() {
	return output() >> 4;
}

void WaveformGenerator::reset() {
	accumulator = 0;
	shift_register = 0x7ffff8;
	freq = 0;
	pw = 0;

	test = 0;
	ring_mod = 0;
	sync = 0;

	msb_rising = false;
}

RESID_INLINE void WaveformGenerator::updateClock(cycle_count delta_t) {
	// No operation if test bit is set.
	if (test) {
		return;
	}

	reg24 accumulator_prev = accumulator;

	// Calculate new accumulator value;
	reg24 delta_accumulator = delta_t*freq;
	accumulator += delta_accumulator;
	accumulator &= 0xffffff;

	// Check whether the MSB is set high. This is used for synchronization.
	msb_rising = !(accumulator_prev & 0x800000) && (accumulator & 0x800000);

	// Shift noise register once for each time accumulator bit 19 is set high.
	// Bit 19 is set high each time 2^20 (0x100000) is added to the accumulator.
	reg24 shift_period = 0x100000;

	while (delta_accumulator) {
		if (delta_accumulator < shift_period) {
			shift_period = delta_accumulator;
			// Determine whether bit 19 is set on the last period.
			// NB! Requires two's complement integer.
			if (shift_period <= 0x080000) {
				// Check for flip from 0 to 1.
				if (((accumulator - shift_period) & 0x080000) || !(accumulator & 0x080000))
				{
					break;
				}
			}
			else {
				// Check for flip from 0 (to 1 or via 1 to 0) or from 1 via 0 to 1.
				if (((accumulator - shift_period) & 0x080000) && !(accumulator & 0x080000))
				{
					break;
				}
			}
		}

		// Shift the noise/random register.
		// NB! The shift is actually delayed 2 cycles, this is not modeled.
		reg24 bit0 = ((shift_register >> 22) ^ (shift_register >> 17)) & 0x1;
		shift_register <<= 1;
		shift_register &= 0x7fffff;
		shift_register |= bit0;

		delta_accumulator -= shift_period;
	}
}


/**
 * Synchronize oscillators.
 * This must be done after all the oscillators have been updateClock()'ed since the
 * oscillators operate in parallel.
 * Note that the oscillators must be clocked exactly on the cycle when the
 * MSB is set high for hard sync to operate correctly. See SID::updateClock().
 */
RESID_INLINE void WaveformGenerator::synchronize() {
	// A special case occurs when a sync source is synced itself on the same
	// cycle as when its MSB is set high. In this case the destination will
	// not be synced. This has been verified by sampling OSC3.
	if (msb_rising && sync_dest->sync && !(sync && sync_source->msb_rising)) {
		sync_dest->accumulator = 0;
	}
}


/*
 * Output functions
 */

// No waveform: Zero output.
RESID_INLINE reg12 WaveformGenerator::output____() {
	return 0x000;
}

// Triangle:
RESID_INLINE reg12 WaveformGenerator::output___T() {
	reg24 msb = (ring_mod ? accumulator ^ sync_source->accumulator : accumulator)
		& 0x800000;
	return ((msb ? ~accumulator : accumulator) >> 11) & 0xfff;
}

// Sawtooth:
RESID_INLINE reg12 WaveformGenerator::output__S_() {
	return accumulator >> 12;
}

// Pulse:
RESID_INLINE reg12 WaveformGenerator::output_P__() {
	return (test || (accumulator >> 12) >= pw) ? 0xfff : 0x000;
}

// Noise:
RESID_INLINE reg12 WaveformGenerator::outputN___() {
	return
		((shift_register & 0x400000) >> 11) |
		((shift_register & 0x100000) >> 10) |
		((shift_register & 0x010000) >> 7) |
		((shift_register & 0x002000) >> 5) |
		((shift_register & 0x000800) >> 4) |
		((shift_register & 0x000080) >> 1) |
		((shift_register & 0x000010) << 1) |
		((shift_register & 0x000004) << 2);
}

// Combined waveforms:

RESID_INLINE reg12 WaveformGenerator::output__ST() {
	return wave6581__ST[output__S_()] << 4;
}

RESID_INLINE reg12 WaveformGenerator::output_P_T() {
	return (wave6581_P_T[output___T() >> 1] << 4) & output_P__();
}

RESID_INLINE reg12 WaveformGenerator::output_PS_() {
	return (wave6581_PS_[output__S_()] << 4) & output_P__();
}

RESID_INLINE reg12 WaveformGenerator::output_PST() {
	return (wave6581_PST[output__S_()] << 4) & output_P__();
}

// Combined waveforms including noise:

RESID_INLINE reg12 WaveformGenerator::outputN__T() {
	return 0;
}

RESID_INLINE reg12 WaveformGenerator::outputN_S_() {
	return 0;
}

RESID_INLINE reg12 WaveformGenerator::outputN_ST() {
	return 0;
}

RESID_INLINE reg12 WaveformGenerator::outputNP__() {
	return 0;
}

RESID_INLINE reg12 WaveformGenerator::outputNP_T() {
	return 0;
}

RESID_INLINE reg12 WaveformGenerator::outputNPS_() {
	return 0;
}

RESID_INLINE reg12 WaveformGenerator::outputNPST() {
	return 0;
}

/**
 * Select one of 16 possible combinations of waveforms.
 */
RESID_INLINE reg12 WaveformGenerator::output() {
	// It may seem cleaner to use an array of member functions to return
	// waveform output; however a switch with inline functions is faster.

	switch (waveform) {
	default:
	case 0x0:
		return output____();
	case 0x1:
		return output___T();
	case 0x2:
		return output__S_();
	case 0x3:
		return output__ST();
	case 0x4:
		return output_P__();
	case 0x5:
		return output_P_T();
	case 0x6:
		return output_PS_();
	case 0x7:
		return output_PST();
	case 0x8:
		return outputN___();
	case 0x9:
		return outputN__T();
	case 0xa:
		return outputN_S_();
	case 0xb:
		return outputN_ST();
	case 0xc:
		return outputNP__();
	case 0xd:
		return outputNP_T();
	case 0xe:
		return outputNPS_();
	case 0xf:
		return outputNPST();
	}
}

/*
 * Our objective is to construct a smooth interpolating single-valued function
 * y = f(x).
 * Our approach is to approximate the properties of Catmull-Rom splines for
 * piecewice cubic polynomials.
 */

/**
 * Calculation of coefficients.
 */
inline void cubic_coefficients(double x1, double y1, double x2, double y2,
						double k1, double k2,
						double& a, double& b, double& c, double& d)
{
	double dx = x2 - x1, dy = y2 - y1;

	a = ((k1 + k2) - 2*dy/dx)/(dx*dx);
	b = ((k2 - k1)/dx - 3*(x1 + x2)*a)/2;
	c = k1 - (3*x1*a + 2*b)*x1;
	d = y1 - ((x1*a + b)*x1 + c)*x1;
}

/**
 * Evaluation of cubic polynomial by forward differencing.
 */
template<class PointPlotter>
inline void interpolate_segment(double x1, double y1, double x2, double y2,
								double k1, double k2,
								PointPlotter plot, double res)
{
	double a, b, c, d;
	cubic_coefficients(x1, y1, x2, y2, k1, k2, a, b, c, d);

	double y = ((a*x1 + b)*x1 + c)*x1 + d;
	double dy = (3*a*(x1 + res) + 2*b)*x1*res + ((a*res + b)*res + c)*res;
	double d2y = (6*a*(x1 + res) + 2*b)*res*res;
	double d3y = 6*a*res*res*res;

	// Calculate each point.
	for (double x = x1; x <= x2; x += res) {
		plot(x, y);
		y += dy; dy += d2y; d2y += d3y;
	}
}

template<class PointIter>
inline double x(PointIter p) {
	return (*p)[0];
}

template<class PointIter>
inline double y(PointIter p) {
	return (*p)[1];
}

/**
 * Evaluation of complete interpolating function.
 * Note that since each curve segment is controlled by four points, the
 * end points will not be interpolated. If extra control points are not
 * desirable, the end points can simply be repeated to ensure interpolation.
 * Note also that points of non-differentiability and discontinuity can be
 * introduced by repeating points.
 */
template<class PointIter, class PointPlotter>
inline void interpolate(PointIter p0, PointIter pn, PointPlotter plot, double res) {
	double k1, k2;

	// Set up points for first curve segment.
	PointIter p1 = p0; ++p1;
	PointIter p2 = p1; ++p2;
	PointIter p3 = p2; ++p3;

	// Draw each curve segment.
	for (; p2 != pn; ++p0, ++p1, ++p2, ++p3) {
		// p1 and p2 equal; single point.
		if (x(p1) == x(p2)) {
			continue;
		}
		// Both end points repeated; straight line.
		if (x(p0) == x(p1) && x(p2) == x(p3)) {
			k1 = k2 = (y(p2) - y(p1))/(x(p2) - x(p1));
		}
		// p0 and p1 equal; use f''(x1) = 0.
		else if (x(p0) == x(p1)) {
			k2 = (y(p3) - y(p1))/(x(p3) - x(p1));
			k1 = (3*(y(p2) - y(p1))/(x(p2) - x(p1)) - k2)/2;
		}
		// p2 and p3 equal; use f''(x2) = 0.
		else if (x(p2) == x(p3)) {
			k1 = (y(p2) - y(p0))/(x(p2) - x(p0));
			k2 = (3*(y(p2) - y(p1))/(x(p2) - x(p1)) - k1)/2;
		}
		// Normal curve.
		else {
			k1 = (y(p2) - y(p0))/(x(p2) - x(p0));
			k2 = (y(p3) - y(p1))/(x(p3) - x(p1));
		}

		interpolate_segment(x(p1), y(p1), x(p2), y(p2), k1, k2, plot, res);
	}
}

/**
 * Class for plotting integers into an array.
 */
template<class F>
class PointPlotter {
protected:
	F* f;

public:
	PointPlotter(F* arr) : f(arr) {
	}

	void operator ()(double x, double y) {
		// Clamp negative values to zero.
		if (y < 0) {
			y = 0;
		}

		f[F(x)] = F(y);
	}
};

fc_point Filter::f0_points_6581[] = {
	//  FC      f         FCHI FCLO
	// ----------------------------
	{    0,   220 },   // 0x00      - repeated end point
	{    0,   220 },   // 0x00
	{  128,   230 },   // 0x10
	{  256,   250 },   // 0x20
	{  384,   300 },   // 0x30
	{  512,   420 },   // 0x40
	{  640,   780 },   // 0x50
	{  768,  1600 },   // 0x60
	{  832,  2300 },   // 0x68
	{  896,  3200 },   // 0x70
	{  960,  4300 },   // 0x78
	{  992,  5000 },   // 0x7c
	{ 1008,  5400 },   // 0x7e
	{ 1016,  5700 },   // 0x7f
	{ 1023,  6000 },   // 0x7f 0x07
	{ 1023,  6000 },   // 0x7f 0x07 - discontinuity
	{ 1024,  4600 },   // 0x80      -
	{ 1024,  4600 },   // 0x80
	{ 1032,  4800 },   // 0x81
	{ 1056,  5300 },   // 0x84
	{ 1088,  6000 },   // 0x88
	{ 1120,  6600 },   // 0x8c
	{ 1152,  7200 },   // 0x90
	{ 1280,  9500 },   // 0xa0
	{ 1408, 12000 },   // 0xb0
	{ 1536, 14500 },   // 0xc0
	{ 1664, 16000 },   // 0xd0
	{ 1792, 17100 },   // 0xe0
	{ 1920, 17700 },   // 0xf0
	{ 2047, 18000 },   // 0xff 0x07
	{ 2047, 18000 }    // 0xff 0x07 - repeated end point
};


/*
 * Filter
 */

Filter::Filter() {
	fc = 0;

	res = 0;

	filt = 0;

	voice3off = 0;

	hp_bp_lp = 0;

	vol = 0;

	// State of filter.
	Vhp = 0;
	Vbp = 0;
	Vlp = 0;
	Vnf = 0;

	enable_filter(true);

	// Create mappings from FC to cutoff frequency.
	interpolate(f0_points_6581, f0_points_6581
		+ sizeof(f0_points_6581)/sizeof(*f0_points_6581) - 1,
		PointPlotter<sound_sample>(f0_6581), 1.0);

	mixer_DC = (-0xfff*0xff/18) >> 7;

	f0 = f0_6581;
	f0_points = f0_points_6581;
	f0_count = sizeof(f0_points_6581)/sizeof(*f0_points_6581);

	set_w0();
	set_Q();
}

void Filter::enable_filter(bool enable) {
	enabled = enable;
}

void Filter::reset(){
	fc = 0;

	res = 0;

	filt = 0;

	voice3off = 0;

	hp_bp_lp = 0;

	vol = 0;

	// State of filter.
	Vhp = 0;
	Vbp = 0;
	Vlp = 0;
	Vnf = 0;

	set_w0();
	set_Q();
}

void Filter::writeFC_LO(reg8 fc_lo) {
	fc = (fc & 0x7f8) | (fc_lo & 0x007);
	set_w0();
}

void Filter::writeFC_HI(reg8 fc_hi) {
	fc = ((fc_hi << 3) & 0x7f8) | (fc & 0x007);
	set_w0();
}

void Filter::writeRES_FILT(reg8 res_filt) {
	res = (res_filt >> 4) & 0x0f;
	set_Q();

	filt = res_filt & 0x0f;
}

void Filter::writeMODE_VOL(reg8 mode_vol) {
	voice3off = mode_vol & 0x80;

	hp_bp_lp = (mode_vol >> 4) & 0x07;

	vol = mode_vol & 0x0f;
}

// Set filter cutoff frequency.
void Filter::set_w0() {
	const double pi = 3.1415926535897932385;

	// Multiply with 1.048576 to facilitate division by 1 000 000 by right-
	// shifting 20 times (2 ^ 20 = 1048576).
	w0 = static_cast<sound_sample>(2*pi*f0[fc]*1.048576);

	// Limit f0 to 16kHz to keep 1 cycle filter stable.
	const sound_sample w0_max_1 = static_cast<sound_sample>(2*pi*16000*1.048576);
	w0_ceil_1 = w0 <= w0_max_1 ? w0 : w0_max_1;

	// Limit f0 to 4kHz to keep delta_t cycle filter stable.
	const sound_sample w0_max_dt = static_cast<sound_sample>(2*pi*4000*1.048576);
	w0_ceil_dt = w0 <= w0_max_dt ? w0 : w0_max_dt;
}

// Set filter resonance.
void Filter::set_Q() {
	// Q is controlled linearly by res. Q has approximate range [0.707, 1.7].
	// As resonance is increased, the filter must be clocked more often to keep
	// stable.

	// The coefficient 1024 is dispensed of later by right-shifting 10 times
	// (2 ^ 10 = 1024).
	_1024_div_Q = static_cast<sound_sample>(1024.0/(0.707 + 1.0*res/0x0f));
}

RESID_INLINE void Filter::updateClock(cycle_count delta_t,
				   sound_sample voice1,
				   sound_sample voice2,
				   sound_sample voice3)
{
	// Scale each voice down from 20 to 13 bits.
	voice1 >>= 7;
	voice2 >>= 7;

	// NB! Voice 3 is not silenced by voice3off if it is routed through
	// the filter.
	if (voice3off && !(filt & 0x04)) {
		voice3 = 0;
	}
	else {
		voice3 >>= 7;
	}

	// Enable filter on/off.
	// This is not really part of SID, but is useful for testing.
	// On slow CPUs it may be necessary to bypass the filter to lower the CPU
	// load.
	if (!enabled) {
		Vnf = voice1 + voice2 + voice3;
		Vhp = Vbp = Vlp = 0;
		return;
	}

	// Route voices into or around filter.
	// The code below is expanded to a switch for faster execution.
	// (filt1 ? Vi : Vnf) += voice1;
	// (filt2 ? Vi : Vnf) += voice2;
	// (filt3 ? Vi : Vnf) += voice3;

	sound_sample Vi;

	switch (filt) {
	default:
	case 0x0:
		Vi = 0;
		Vnf = voice1 + voice2 + voice3;
		break;
	case 0x1:
		Vi = voice1;
		Vnf = voice2 + voice3;
		break;
	case 0x2:
		Vi = voice2;
		Vnf = voice1 + voice3;
		break;
	case 0x3:
		Vi = voice1 + voice2;
		Vnf = voice3;
		break;
	case 0x4:
		Vi = voice3;
		Vnf = voice1 + voice2;
		break;
	case 0x5:
		Vi = voice1 + voice3;
		Vnf = voice2;
		break;
	case 0x6:
		Vi = voice2 + voice3;
		Vnf = voice1;
		break;
	case 0x7:
		Vi = voice1 + voice2 + voice3;
		Vnf = 0;
		break;
	case 0x8:
		Vi = 0;
		Vnf = voice1 + voice2 + voice3;
		break;
	case 0x9:
		Vi = voice1;
		Vnf = voice2 + voice3;
		break;
	case 0xa:
		Vi = voice2;
		Vnf = voice1 + voice3;
		break;
	case 0xb:
		Vi = voice1 + voice2;
		Vnf = voice3;
		break;
	case 0xc:
		Vi = voice3;
		Vnf = voice1 + voice2;
		break;
	case 0xd:
		Vi = voice1 + voice3;
		Vnf = voice2;
		break;
	case 0xe:
		Vi = voice2 + voice3;
		Vnf = voice1;
		break;
	case 0xf:
		Vi = voice1 + voice2 + voice3;
		Vnf = 0;
		break;
	}

	// Maximum delta cycles for the filter to work satisfactorily under current
	// cutoff frequency and resonance constraints is approximately 8.
	cycle_count delta_t_flt = 8;

	while (delta_t) {
		if (delta_t < delta_t_flt) {
			delta_t_flt = delta_t;
		}

		// delta_t is converted to seconds given a 1MHz clock by dividing
		// with 1 000 000. This is done in two operations to avoid integer
		// multiplication overflow.

		// Calculate filter outputs.
		// Vhp = Vbp/Q - Vlp - Vi;
		// dVbp = -w0*Vhp*dt;
		// dVlp = -w0*Vbp*dt;
		sound_sample w0_delta_t = w0_ceil_dt*delta_t_flt >> 6;

		sound_sample dVbp = (w0_delta_t*Vhp >> 14);
		sound_sample dVlp = (w0_delta_t*Vbp >> 14);
		Vbp -= dVbp;
		Vlp -= dVlp;
		Vhp = (Vbp*_1024_div_Q >> 10) - Vlp - Vi;

		delta_t -= delta_t_flt;
	}
}

RESID_INLINE sound_sample Filter::output() {
	// This is handy for testing.
	if (!enabled) {
		return (Vnf + mixer_DC)*static_cast<sound_sample>(vol);
	}

	// Mix highpass, bandpass, and lowpass outputs. The sum is not
	// weighted, this can be confirmed by sampling sound output for
	// e.g. bandpass, lowpass, and bandpass+lowpass from a SID chip.

	// The code below is expanded to a switch for faster execution.
	// if (hp) Vf += Vhp;
	// if (bp) Vf += Vbp;
	// if (lp) Vf += Vlp;

	sound_sample Vf;

	switch (hp_bp_lp) {
	default:
	case 0x0:
		Vf = 0;
		break;
	case 0x1:
		Vf = Vlp;
		break;
	case 0x2:
		Vf = Vbp;
		break;
	case 0x3:
		Vf = Vlp + Vbp;
		break;
	case 0x4:
		Vf = Vhp;
		break;
	case 0x5:
		Vf = Vlp + Vhp;
		break;
	case 0x6:
		Vf = Vbp + Vhp;
		break;
	case 0x7:
		Vf = Vlp + Vbp + Vhp;
		break;
	}

	// Sum non-filtered and filtered output.
	// Multiply the sum with volume.
	return (Vnf + Vf + mixer_DC)*static_cast<sound_sample>(vol);
}


/*
 * EnvelopeGenerator
 */

EnvelopeGenerator::EnvelopeGenerator() {
	reset();
}

void EnvelopeGenerator::reset() {
	envelope_counter = 0;

	attack = 0;
	decay = 0;
	sustain = 0;
	release = 0;

	gate = 0;

	rate_counter = 0;
	exponential_counter = 0;
	exponential_counter_period = 1;

	state = RELEASE;
	rate_period = rate_counter_period[release];
	hold_zero = true;
}

reg16 EnvelopeGenerator::rate_counter_period[] = {
	9,  //   2ms*1.0MHz/256 =     7.81
	32,  //   8ms*1.0MHz/256 =    31.25
	63,  //  16ms*1.0MHz/256 =    62.50
	95,  //  24ms*1.0MHz/256 =    93.75
	149,  //  38ms*1.0MHz/256 =   148.44
	220,  //  56ms*1.0MHz/256 =   218.75
	267,  //  68ms*1.0MHz/256 =   265.63
	313,  //  80ms*1.0MHz/256 =   312.50
	392,  // 100ms*1.0MHz/256 =   390.63
	977,  // 250ms*1.0MHz/256 =   976.56
	1954,  // 500ms*1.0MHz/256 =  1953.13
	3126,  // 800ms*1.0MHz/256 =  3125.00
	3907,  //   1 s*1.0MHz/256 =  3906.25
	11720,  //   3 s*1.0MHz/256 = 11718.75
	19532,  //   5 s*1.0MHz/256 = 19531.25
	31251   //   8 s*1.0MHz/256 = 31250.00
};


reg8 EnvelopeGenerator::sustain_level[] = {
	0x00,
	0x11,
	0x22,
	0x33,
	0x44,
	0x55,
	0x66,
	0x77,
	0x88,
	0x99,
	0xaa,
	0xbb,
	0xcc,
	0xdd,
	0xee,
	0xff,
};

void EnvelopeGenerator::writeCONTROL_REG(reg8 control) {
	reg8 gate_next = control & 0x01;

	// The rate counter is never reset, thus there will be a delay before the
	// envelope counter starts counting up (attack) or down (release).

	// Gate bit on: Start attack, decay, sustain.
	if (!gate && gate_next) {
		state = ATTACK;
		rate_period = rate_counter_period[attack];

		// Switching to attack state unlocks the zero freeze.
		hold_zero = false;
	}
	// Gate bit off: Start release.
	else if (gate && !gate_next) {
		state = RELEASE;
		rate_period = rate_counter_period[release];
	}

	gate = gate_next;
}

void EnvelopeGenerator::writeATTACK_DECAY(reg8 attack_decay) {
	attack = (attack_decay >> 4) & 0x0f;
	decay = attack_decay & 0x0f;
	if (state == ATTACK) {
		rate_period = rate_counter_period[attack];
	}
	else if (state == DECAY_SUSTAIN) {
		rate_period = rate_counter_period[decay];
	}
}

void EnvelopeGenerator::writeSUSTAIN_RELEASE(reg8 sustain_release) {
	sustain = (sustain_release >> 4) & 0x0f;
	release = sustain_release & 0x0f;
	if (state == RELEASE) {
		rate_period = rate_counter_period[release];
	}
}

reg8 EnvelopeGenerator::readENV() {
	return output();
}

RESID_INLINE void EnvelopeGenerator::updateClock(cycle_count delta_t) {
	// Check for ADSR delay bug.
	// If the rate counter comparison value is set below the current value of the
	// rate counter, the counter will continue counting up until it wraps around
	// to zero at 2^15 = 0x8000, and then count rate_period - 1 before the
	// envelope can finally be stepped.
	// This has been verified by sampling ENV3.
	//

	// NB! This requires two's complement integer.
	int rate_step = rate_period - rate_counter;
	if (rate_step <= 0) {
		rate_step += 0x7fff;
	}

	while (delta_t) {
		if (delta_t < rate_step) {
			rate_counter += delta_t;
			if (rate_counter & 0x8000) {
				++rate_counter &= 0x7fff;
			}
			return;
		}

		rate_counter = 0;
		delta_t -= rate_step;

		// The first envelope step in the attack state also resets the exponential
		// counter. This has been verified by sampling ENV3.
		//
		if (state == ATTACK	|| ++exponential_counter == exponential_counter_period)
		{
			exponential_counter = 0;

			// Check whether the envelope counter is frozen at zero.
			if (hold_zero) {
				rate_step = rate_period;
				continue;
			}

			switch (state) {
			case ATTACK:
				// The envelope counter can flip from 0xff to 0x00 by changing state to
				// release, then to attack. The envelope counter is then frozen at
				// zero; to unlock this situation the state must be changed to release,
				// then to attack. This has been verified by sampling ENV3.
				//
				++envelope_counter &= 0xff;
				if (envelope_counter == 0xff) {
					state = DECAY_SUSTAIN;
					rate_period = rate_counter_period[decay];
				}
				break;
			case DECAY_SUSTAIN:
				if (envelope_counter != sustain_level[sustain]) {
					--envelope_counter;
				}
				break;
			case RELEASE:
				// The envelope counter can flip from 0x00 to 0xff by changing state to
				// attack, then to release. The envelope counter will then continue
				// counting down in the release state.
				// This has been verified by sampling ENV3.
				// NB! The operation below requires two's complement integer.
				//
				--envelope_counter &= 0xff;
				break;
			}

			// Check for change of exponential counter period.
			switch (envelope_counter) {
			case 0xff:
				exponential_counter_period = 1;
				break;
			case 0x5d:
				exponential_counter_period = 2;
				break;
			case 0x36:
				exponential_counter_period = 4;
				break;
			case 0x1a:
				exponential_counter_period = 8;
				break;
			case 0x0e:
				exponential_counter_period = 16;
				break;
			case 0x06:
				exponential_counter_period = 30;
				break;
			case 0x00:
				exponential_counter_period = 1;

				// When the envelope counter is changed to zero, it is frozen at zero.
				// This has been verified by sampling ENV3.
				hold_zero = true;
				break;
			}
		}

		rate_step = rate_period;
	}
}

RESID_INLINE reg8 EnvelopeGenerator::output() {
	return envelope_counter;
}


/*
 * ExternalFilter
 */

ExternalFilter::ExternalFilter() {
	reset();
	enable_filter(true);
	set_sampling_parameter(15915.6);
	mixer_DC = ((((0x800 - 0x380) + 0x800)*0xff*3 - 0xfff*0xff/18) >> 7)*0x0f;
}

void ExternalFilter::enable_filter(bool enable) {
	enabled = enable;
}

void ExternalFilter::set_sampling_parameter(double pass_freq) {
	static const double pi = 3.1415926535897932385;

	w0hp = 105;
	w0lp = (sound_sample) (pass_freq * (2.0 * pi * 1.048576));
	if (w0lp > 104858)
		w0lp = 104858;
}

void ExternalFilter::reset() {
	// State of filter.
	Vlp = 0;
	Vhp = 0;
	Vo = 0;
}

RESID_INLINE void ExternalFilter::updateClock(cycle_count delta_t, sound_sample Vi) {
	// This is handy for testing.
	if (!enabled) {
		// Remove maximum DC level since there is no filter to do it.
		Vlp = Vhp = 0;
		Vo = Vi - mixer_DC;
		return;
	}

	// Maximum delta cycles for the external filter to work satisfactorily
	// is approximately 8.
	cycle_count delta_t_flt = 8;

	while (delta_t) {
		if (delta_t < delta_t_flt) {
			delta_t_flt = delta_t;
		}

		// delta_t is converted to seconds given a 1MHz clock by dividing
		// with 1 000 000.

		// Calculate filter outputs.
		// Vo  = Vlp - Vhp;
		// Vlp = Vlp + w0lp*(Vi - Vlp)*delta_t;
		// Vhp = Vhp + w0hp*(Vlp - Vhp)*delta_t;

		sound_sample dVlp = (w0lp*delta_t_flt >> 8)*(Vi - Vlp) >> 12;
		sound_sample dVhp = w0hp*delta_t_flt*(Vlp - Vhp) >> 20;
		Vo = Vlp - Vhp;
		Vlp += dVlp;
		Vhp += dVhp;

		delta_t -= delta_t_flt;
	}
}

RESID_INLINE sound_sample ExternalFilter::output() {
	return Vo;
}


/*
 * Voice
 */

Voice::Voice() {
	wave_zero = 0x380;
	voice_DC = 0x800*0xff;
}

void Voice::set_sync_source(Voice* source) {
	wave.set_sync_source(&source->wave);
}

void Voice::writeCONTROL_REG(reg8 control) {
	wave.writeCONTROL_REG(control);
	envelope.writeCONTROL_REG(control);
}

void Voice::reset() {
	wave.reset();
	envelope.reset();
}


/*
 * SID
 */

SID::SID() {
	voice[0].set_sync_source(&voice[2]);
	voice[1].set_sync_source(&voice[0]);
	voice[2].set_sync_source(&voice[1]);

	set_sampling_parameters(985248, 44100);

	bus_value = 0;
	bus_value_ttl = 0;
}

SID::~SID() {}

void SID::reset() {
	for (int i = 0; i < 3; i++) {
		voice[i].reset();
	}
	filter.reset();
	extfilt.reset();

	bus_value = 0;
	bus_value_ttl = 0;
}

int SID::output() {
	const int range = 1 << 16;
	const int half = range >> 1;
	int sample = extfilt.output()/((4095*255 >> 7)*3*15*2/range);
	if (sample >= half) {
		return half - 1;
	}
	if (sample < -half) {
		return -half;
	}
	return sample;
}


/**
 * Read registers.
 *
 * Reading a write only register returns the last byte written to any SID
 * register. The individual bits in this value start to fade down towards
 * zero after a few cycles. All bits reach zero within approximately
 * $2000 - $4000 cycles.
 * It has been claimed that this fading happens in an orderly fashion, however
 * sampling of write only registers reveals that this is not the case.
 * NB! This is not correctly modeled.
 * The actual use of write only registers has largely been made in the belief
 * that all SID registers are readable. To support this belief the read
 * would have to be done immediately after a write to the same register
 * (remember that an intermediate write to another register would yield that
 * value instead). With this in mind we return the last value written to
 * any SID register for $2000 cycles without modeling the bit fading.
 */
reg8 SID::read(reg8 offset) {
	switch (offset) {
		case 0x19:
		case 0x1a:
			return 0; //readPOT();
		case 0x1b:
			return voice[2].wave.readOSC();
		case 0x1c:
			return voice[2].envelope.readENV();
		default:
			return bus_value;
	}
}

void SID::write(reg8 offset, reg8 value) {
	bus_value = value;
	bus_value_ttl = 0x2000;

	switch (offset) {
	  case 0x00:
		  voice[0].wave.writeFREQ_LO(value);
		  break;
	  case 0x01:
		  voice[0].wave.writeFREQ_HI(value);
		  break;
	  case 0x02:
		  voice[0].wave.writePW_LO(value);
		  break;
	  case 0x03:
		  voice[0].wave.writePW_HI(value);
		  break;
	  case 0x04:
		  voice[0].writeCONTROL_REG(value);
		  break;
	  case 0x05:
		  voice[0].envelope.writeATTACK_DECAY(value);
		  break;
	  case 0x06:
		  voice[0].envelope.writeSUSTAIN_RELEASE(value);
		  break;
	  case 0x07:
		  voice[1].wave.writeFREQ_LO(value);
		  break;
	  case 0x08:
		  voice[1].wave.writeFREQ_HI(value);
		  break;
	  case 0x09:
		  voice[1].wave.writePW_LO(value);
		  break;
	  case 0x0a:
		  voice[1].wave.writePW_HI(value);
		  break;
	  case 0x0b:
		  voice[1].writeCONTROL_REG(value);
		  break;
	  case 0x0c:
		  voice[1].envelope.writeATTACK_DECAY(value);
		  break;
	  case 0x0d:
		  voice[1].envelope.writeSUSTAIN_RELEASE(value);
		  break;
	  case 0x0e:
		  voice[2].wave.writeFREQ_LO(value);
		  break;
	  case 0x0f:
		  voice[2].wave.writeFREQ_HI(value);
		  break;
	  case 0x10:
		  voice[2].wave.writePW_LO(value);
		  break;
	  case 0x11:
		  voice[2].wave.writePW_HI(value);
		  break;
	  case 0x12:
		  voice[2].writeCONTROL_REG(value);
		  break;
	  case 0x13:
		  voice[2].envelope.writeATTACK_DECAY(value);
		  break;
	  case 0x14:
		  voice[2].envelope.writeSUSTAIN_RELEASE(value);
		  break;
	  case 0x15:
		  filter.writeFC_LO(value);
		  break;
	  case 0x16:
		  filter.writeFC_HI(value);
		  break;
	  case 0x17:
		  filter.writeRES_FILT(value);
		  break;
	  case 0x18:
		  filter.writeMODE_VOL(value);
		  break;
	  default:
		  break;
	}
}

void SID::enable_filter(bool enable) {
	filter.enable_filter(enable);
}

void SID::enable_external_filter(bool enable) {
	extfilt.enable_filter(enable);
}


/**
 * Setting of SID sampling parameters.
 *
 * Use a clock freqency of 985248Hz for PAL C64, 1022730Hz for NTSC C64.
 * The default end of passband frequency is pass_freq = 0.9*sample_freq/2
 * for sample frequencies up to ~ 44.1kHz, and 20kHz for higher sample
 * frequencies.
 *
 * For resampling, the ratio between the clock frequency and the sample
 * frequency is limited as follows:
 *   125*clock_freq/sample_freq < 16384
 * E.g. provided a clock frequency of ~ 1MHz, the sample frequency can not
 * be set lower than ~ 8kHz. A lower sample frequency would make the
 * resampling code overfill its 16k sample ring buffer.
 *
 * The end of passband frequency is also limited:
 *   pass_freq <= 0.9*sample_freq/2
 *
 * E.g. for a 44.1kHz sampling rate the end of passband frequency is limited
 * to slightly below 20kHz. This constraint ensures that the FIR table is
 * not overfilled.
 */
bool SID::set_sampling_parameters(double clock_freq,
								  double sample_freq, double pass_freq,
								  double filter_scale)
{
	// The default passband limit is 0.9*sample_freq/2 for sample
	// frequencies below ~ 44.1kHz, and 20kHz for higher sample frequencies.
	if (pass_freq < 0) {
		pass_freq = 20000;
		if (2*pass_freq/sample_freq >= 0.9) {
			pass_freq = 0.9*sample_freq/2;
		}
	}
	// Check whether the FIR table would overfill.
	else if (pass_freq > 0.9*sample_freq/2) {
		return false;
	}

	// The filter scaling is only included to avoid clipping, so keep
	// it sane.
	if (filter_scale < 0.9 || filter_scale > 1.0) {
		return false;
	}

	// Set the external filter to the pass freq
	extfilt.set_sampling_parameter (pass_freq);
	clock_frequency = clock_freq;

	cycles_per_sample =
		cycle_count(clock_freq/sample_freq*(1 << FIXP_SHIFT) + 0.5);

	sample_offset = 0;
	sample_prev = 0;

	return true;
}

void SID::updateClock(cycle_count delta_t) {
	int i;

	if (delta_t <= 0) {
		return;
	}

	// Age bus value.
	bus_value_ttl -= delta_t;
	if (bus_value_ttl <= 0) {
		bus_value = 0;
		bus_value_ttl = 0;
	}

	// Clock amplitude modulators.
	for (i = 0; i < 3; i++) {
		voice[i].envelope.updateClock(delta_t);
	}

	// Clock and synchronize oscillators.
	// Loop until we reach the current cycle.
	cycle_count delta_t_osc = delta_t;
	while (delta_t_osc) {
		cycle_count delta_t_min = delta_t_osc;

		// Find minimum number of cycles to an oscillator accumulator MSB toggle.
		// We have to clock on each MSB on / MSB off for hard sync to operate
		// correctly.
		for (i = 0; i < 3; i++) {
			WaveformGenerator& wave = voice[i].wave;

			// It is only necessary to clock on the MSB of an oscillator that is
			// a sync source and has freq != 0.
			if (!(wave.sync_dest->sync && wave.freq)) {
				continue;
			}

			reg16 freq = wave.freq;
			reg24 accumulator = wave.accumulator;

			// Clock on MSB off if MSB is on, clock on MSB on if MSB is off.
			reg24 delta_accumulator =
				(accumulator & 0x800000 ? 0x1000000 : 0x800000) - accumulator;

			cycle_count delta_t_next = delta_accumulator/freq;
			if (delta_accumulator%freq) {
				++delta_t_next;
			}

			if (delta_t_next < delta_t_min) {
				delta_t_min = delta_t_next;
			}
		}

		// Clock oscillators.
		for (i = 0; i < 3; i++) {
			voice[i].wave.updateClock(delta_t_min);
		}

		// Synchronize oscillators.
		for (i = 0; i < 3; i++) {
			voice[i].wave.synchronize();
		}

		delta_t_osc -= delta_t_min;
	}

	// Clock filter.
	filter.updateClock(delta_t,
		voice[0].output(), voice[1].output(), voice[2].output());

	// Clock external filter.
	extfilt.updateClock(delta_t, filter.output());
}


/**
 * SID clocking with audio sampling.
 * Fixpoint arithmetics is used.
 */
int SID::updateClock(cycle_count& delta_t, short* buf, int n, int interleave) {
	int s = 0;

	for (;;) {
		cycle_count next_sample_offset = sample_offset + cycles_per_sample + (1 << (FIXP_SHIFT - 1));
		cycle_count delta_t_sample = next_sample_offset >> FIXP_SHIFT;
		if (delta_t_sample > delta_t) {
			break;
		}
		if (s >= n) {
			return s;
		}
		updateClock(delta_t_sample);
		delta_t -= delta_t_sample;
		sample_offset = (next_sample_offset & FIXP_MASK) - (1 << (FIXP_SHIFT - 1));
		buf[s++*interleave] = output();
	}

	updateClock(delta_t);
	sample_offset -= delta_t << FIXP_SHIFT;
	delta_t = 0;
	return s;
}

}

//	Plugin interface
//	(This can only create a null driver since C64 audio support is not part of the
//	midi driver architecture. But we need the plugin for the options menu in the launcher
//	and for MidiDriver::detectDevice() which is more or less used by all engines.)

class C64MusicPlugin : public NullMusicPlugin {
public:
	const char *getName() const {
		return _s("C64 Audio Emulator");
	}

	const char *getId() const {
		return "C64";
	}

	MusicDevices getDevices() const;
};

MusicDevices C64MusicPlugin::getDevices() const {
	MusicDevices devices;
	devices.push_back(MusicDevice(this, "", MT_C64));
	return devices;
}

//#if PLUGIN_ENABLED_DYNAMIC(C64)
	//REGISTER_PLUGIN_DYNAMIC(C64, PLUGIN_TYPE_MUSIC, C64MusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(C64, PLUGIN_TYPE_MUSIC, C64MusicPlugin);
//#endif

#endif
