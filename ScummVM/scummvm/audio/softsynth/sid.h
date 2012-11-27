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

#ifndef AUDIO_SOFTSYNTH_SID_H
#define AUDIO_SOFTSYNTH_SID_H

// Inlining on/off.
#define RESID_INLINE inline

namespace Resid {

// We could have used the smallest possible data type for each SID register,
// however this would give a slower engine because of data type conversions.
// An int is assumed to be at least 32 bits (necessary in the types reg24,
// cycle_count, and sound_sample). GNU does not support 16-bit machines
// (GNU Coding Standards: Portability between CPUs), so this should be
// a valid assumption.

typedef unsigned int reg4;
typedef unsigned int reg8;
typedef unsigned int reg12;
typedef unsigned int reg16;
typedef unsigned int reg24;

typedef int cycle_count;
typedef int sound_sample;
typedef sound_sample fc_point[2];


class WaveformGenerator {
public:
	WaveformGenerator();

	void set_sync_source(WaveformGenerator *);

	void updateClock(cycle_count delta_t);
	void synchronize();
	void reset();

	void writeFREQ_LO(reg8);
	void writeFREQ_HI(reg8);
	void writePW_LO(reg8);
	void writePW_HI(reg8);
	void writeCONTROL_REG(reg8);
	reg8 readOSC();

	// 12-bit waveform output.
	reg12 output();

protected:
	const WaveformGenerator* sync_source;
	WaveformGenerator* sync_dest;

	// Tell whether the accumulator MSB was set high on this cycle.
	bool msb_rising;

	reg24 accumulator;
	reg24 shift_register;

	// Fout  = (Fn*Fclk/16777216)Hz
	reg16 freq;
	// PWout = (PWn/40.95)%
	reg12 pw;

	// The control register right-shifted 4 bits; used for output function
	// table lookup.
	reg8 waveform;

	// The remaining control register bits.
	reg8 test;
	reg8 ring_mod;
	reg8 sync;
	// The gate bit is handled by the EnvelopeGenerator.

	// 16 possible combinations of waveforms.
	reg12 output____();
	reg12 output___T();
	reg12 output__S_();
	reg12 output__ST();
	reg12 output_P__();
	reg12 output_P_T();
	reg12 output_PS_();
	reg12 output_PST();
	reg12 outputN___();
	reg12 outputN__T();
	reg12 outputN_S_();
	reg12 outputN_ST();
	reg12 outputNP__();
	reg12 outputNP_T();
	reg12 outputNPS_();
	reg12 outputNPST();

	// Sample data for combinations of waveforms.
	static const reg8 wave6581__ST[];
	static const reg8 wave6581_P_T[];
	static const reg8 wave6581_PS_[];
	static const reg8 wave6581_PST[];

	friend class Voice;
	friend class SID;
};

class Filter {
public:
	Filter();

	void enable_filter(bool enable);

	void updateClock(cycle_count delta_t,
		sound_sample voice1, sound_sample voice2, sound_sample voice3);
	void reset();

	// Write registers.
	void writeFC_LO(reg8);
	void writeFC_HI(reg8);
	void writeRES_FILT(reg8);
	void writeMODE_VOL(reg8);

	// SID audio output (16 bits).
	sound_sample output();

protected:
	void set_w0();
	void set_Q();

	// Filter enabled.
	bool enabled;

	// Filter cutoff frequency.
	reg12 fc;

	// Filter resonance.
	reg8 res;

	// Selects which inputs to route through filter.
	reg8 filt;

	// Switch voice 3 off.
	reg8 voice3off;

	// Highpass, bandpass, and lowpass filter modes.
	reg8 hp_bp_lp;

	// Output master volume.
	reg4 vol;

	// Mixer DC offset.
	sound_sample mixer_DC;

	// State of filter.
	sound_sample Vhp; // highpass
	sound_sample Vbp; // bandpass
	sound_sample Vlp; // lowpass
	sound_sample Vnf; // not filtered

	// Cutoff frequency, resonance.
	sound_sample w0, w0_ceil_1, w0_ceil_dt;
	sound_sample _1024_div_Q;

	// Cutoff frequency tables.
	// FC is an 11 bit register.
	sound_sample f0_6581[2048];
	sound_sample* f0;
	static fc_point f0_points_6581[];
	fc_point* f0_points;
	int f0_count;

	friend class SID;
};

class EnvelopeGenerator {
public:
	EnvelopeGenerator();

	enum State { ATTACK, DECAY_SUSTAIN, RELEASE };

	void updateClock(cycle_count delta_t);
	void reset();

	void writeCONTROL_REG(reg8);
	void writeATTACK_DECAY(reg8);
	void writeSUSTAIN_RELEASE(reg8);
	reg8 readENV();

	// 8-bit envelope output.
	reg8 output();

protected:
	reg16 rate_counter;
	reg16 rate_period;
	reg8 exponential_counter;
	reg8 exponential_counter_period;
	reg8 envelope_counter;
	bool hold_zero;

	reg4 attack;
	reg4 decay;
	reg4 sustain;
	reg4 release;

	reg8 gate;

	State state;

	// Lookup table to convert from attack, decay, or release value to rate
	// counter period.
	static reg16 rate_counter_period[];

	// The 16 selectable sustain levels.
	static reg8 sustain_level[];

	friend class SID;
};

class ExternalFilter {
public:
	ExternalFilter();

	void enable_filter(bool enable);
	void set_sampling_parameter(double pass_freq);

	void updateClock(cycle_count delta_t, sound_sample Vi);
	void reset();

	// Audio output (20 bits).
	sound_sample output();

protected:
	// Filter enabled.
	bool enabled;

	// Maximum mixer DC offset.
	sound_sample mixer_DC;

	// State of filters.
	sound_sample Vlp; // lowpass
	sound_sample Vhp; // highpass
	sound_sample Vo;

	// Cutoff frequencies.
	sound_sample w0lp;
	sound_sample w0hp;

	friend class SID;
};

class Voice {
public:
	Voice();

	void set_sync_source(Voice *);
	void reset();

	void writeCONTROL_REG(reg8);

	// Amplitude modulated waveform output.
	// Range [-2048*255, 2047*255].
	sound_sample output() {
		// Multiply oscillator output with envelope output.
		return (wave.output() - wave_zero)*envelope.output() + voice_DC;
	}

protected:
	WaveformGenerator wave;
	EnvelopeGenerator envelope;

	// Waveform D/A zero level.
	sound_sample wave_zero;

	// Multiplying D/A DC offset.
	sound_sample voice_DC;

	friend class SID;
};


class SID {
public:
	SID();
	~SID();

	void enable_filter(bool enable);
	void enable_external_filter(bool enable);
	bool set_sampling_parameters(double clock_freq,
		double sample_freq, double pass_freq = -1,
		double filter_scale = 0.97);

	void updateClock(cycle_count delta_t);
	int updateClock(cycle_count& delta_t, short* buf, int n, int interleave = 1);
	void reset();

	// Read/write registers.
	reg8 read(reg8 offset);
	void write(reg8 offset, reg8 value);

	// 16-bit output (AUDIO OUT).
	int output();

protected:
	Voice voice[3];
	Filter filter;
	ExternalFilter extfilt;

	reg8 bus_value;
	cycle_count bus_value_ttl;

	double clock_frequency;

	// Fixpoint constants.
	static const int FIXP_SHIFT;
	static const int FIXP_MASK;

	// Sampling variables.
	cycle_count cycles_per_sample;
	cycle_count sample_offset;
	short sample_prev;
};

}

#endif // not AUDIO_SOFTSYNTH_SID_H
