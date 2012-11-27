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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SCUMM_IMUSE_PCSPK_H
#define SCUMM_IMUSE_PCSPK_H

#include "audio/softsynth/emumidi.h"
#include "audio/softsynth/pcspk.h"

namespace Scumm {

class PcSpkDriver : public MidiDriver_Emulated {
public:
	PcSpkDriver(Audio::Mixer *mixer);
	~PcSpkDriver();

	virtual int open();
	virtual void close();

	virtual void send(uint32 d);
	virtual void sysEx_customInstrument(byte channel, uint32 type, const byte *instr);

	virtual MidiChannel *allocateChannel();
	virtual MidiChannel *getPercussionChannel() { return 0; }

	bool isStereo() const { return _pcSpk.isStereo(); }
	int getRate() const { return _pcSpk.getRate(); }
protected:
	void generateSamples(int16 *buf, int len);
	void onTimer();

private:
	Audio::PCSpeaker _pcSpk;
	int _effectTimer;
	uint8 _randBase;

	void updateNote();
	void output(uint16 out);

	static uint8 getEffectModifier(uint16 level);
	int16 getEffectModLevel(int16 level, int8 mod);
	int16 getRandScale(int16 input);

	struct EffectEnvelope {
		uint8 state;
		int16 currentLevel;
		int16 duration;
		int16 maxLevel;
		int16 startLevel;
		uint8 loop;
		uint8 stateTargetLevels[4];
		uint8 stateModWheelLevels[4];
		uint8 modWheelSensitivity;
		uint8 modWheelState;
		uint8 modWheelLast;
		int16 stateNumSteps;
		int16 stateStepCounter;
		int16 changePerStep;
		int8 dir;
		int16 changePerStepRem;
		int16 changeCountRem;
	};

	struct EffectDefinition {
		int16 phase;
		uint8 type;
		uint8 useModWheel;
		EffectEnvelope *envelope;
	};

	struct OutputChannel {
		uint8 active;
		uint8 note;
		uint8 sustainNoteOff;
		uint8 length;
		const uint8 *instrument;
		uint8 unkA;
		uint8 unkB;
		uint8 unkC;
		int16 unkE;
		EffectEnvelope effectEnvelopeA;
		EffectDefinition effectDefA;
		EffectEnvelope effectEnvelopeB;
		EffectDefinition effectDefB;
		int16 unk60;
	};

	struct MidiChannel_PcSpk : public MidiChannel {
		virtual MidiDriver *device();
		virtual byte getNumber();
		virtual void release();

		virtual void send(uint32 b);
		virtual void noteOff(byte note);
		virtual void noteOn(byte note, byte velocity);
		virtual void programChange(byte program);
		virtual void pitchBend(int16 bend);
		virtual void controlChange(byte control, byte value);
		virtual void pitchBendFactor(byte value);
		virtual void priority(byte value);
		virtual void sysEx_customInstrument(uint32 type, const byte *instr);

		void init(PcSpkDriver *owner, byte channel);
		bool allocate();

		PcSpkDriver *_owner;
		bool _allocated;
		byte _channel;

		OutputChannel _out;
		uint8 _instrument[23];
		uint8 _programNr;
		uint8 _priority;
		uint8 _tl;
		uint8 _modWheel;
		uint8 _sustain;
		uint8 _pitchBendFactor;
		int16 _pitchBend;
	};

	void setupEffects(MidiChannel_PcSpk &chan, EffectEnvelope &env, EffectDefinition &def, byte flags, const byte *data);
	void startEffect(EffectEnvelope &env, const byte *data);
	void initNextEnvelopeState(EffectEnvelope &env);
	void updateEffectGenerator(MidiChannel_PcSpk &chan, EffectEnvelope &env, EffectDefinition &def);
	uint8 advanceEffectEnvelope(EffectEnvelope &env, EffectDefinition &def);

	MidiChannel_PcSpk _channels[6];
	MidiChannel_PcSpk *_activeChannel;

	MidiChannel_PcSpk *_lastActiveChannel;
	uint16 _lastActiveOut;

	static const byte _outInstrumentData[1024];
	static const byte _outputTable1[];
	static const byte _outputTable2[];
	static const uint16 _effectEnvStepTable[];
	static const uint16 _frequencyTable[];
};

} // End of namespace Scumm

#endif
