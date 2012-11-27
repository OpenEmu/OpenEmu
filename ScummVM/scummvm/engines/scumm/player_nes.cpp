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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef DISABLE_NES_APU

#include "engines/engine.h"
#include "scumm/player_nes.h"
#include "scumm/scumm.h"
#include "audio/mixer.h"

namespace Scumm {

static const byte channelMask[4] = {1, 2, 4, 8};

static const uint16 freqTable[64] = {
	0x07F0, 0x077E, 0x0712, 0x06AE, 0x064E, 0x05F3, 0x059E, 0x054D,
	0x0501, 0x04B9, 0x0475, 0x0435, 0x03F8, 0x03BF, 0x0389, 0x0357,
	0x0327, 0x02F9, 0x02CF, 0x02A6, 0x0280, 0x025C, 0x023A, 0x021A,
	0x01FC, 0x01DF, 0x01C4, 0x01AB, 0x0193, 0x017C, 0x0167, 0x0152,
	0x013F, 0x012D, 0x011C, 0x010C, 0x00FD, 0x00EE, 0x00E1, 0x00D4,
	0x00C8, 0x00BD, 0x00B2, 0x00A8, 0x009F, 0x0096, 0x008D, 0x0085,
	0x007E, 0x0076, 0x0070, 0x0069, 0x0063, 0x005E, 0x0058, 0x0053,
	0x004F, 0x004A, 0x0046, 0x0042, 0x003E, 0x003A, 0x0037, 0x0034
};

static const byte instChannel[16] = {
	0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 1, 3, 3, 3
};
static const byte startCmd[16] = {
	0x05, 0x03, 0x06, 0x08, 0x0B, 0x01, 0x01, 0x1A,
	0x16, 0x06, 0x04, 0x17, 0x02, 0x10, 0x0E, 0x0D
};
static const byte releaseCmd[16] = {
	0x0F, 0x00, 0x00, 0x09, 0x00, 0x14, 0x15, 0x00,
	0x00, 0x00, 0x1B, 0x1B, 0x0F, 0x0F, 0x0F, 0x0F
};
static const byte nextCmd[28] =	{
	0xFF, 0xFF, 0xFF, 0xFF, 0x17, 0xFF, 0x07, 0xFF,
	0xFF, 0x0A, 0x09, 0x0C, 0x00, 0x00, 0x00, 0x00,
	0x11, 0x12, 0x11, 0x03, 0xFF, 0xFF, 0x18, 0x00,
	0x19, 0x00, 0x00, 0x00
};
static const byte nextDelay[28] = {
	0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00,
	0x00, 0x05, 0x08, 0x03, 0x00, 0x00, 0x00, 0x00,
	0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00,
	0x03, 0x00, 0x00, 0x00
};

namespace APUe {

static const byte LengthCounts[32] = {
	0x0A,0xFE,
	0x14,0x02,
	0x28,0x04,
	0x50,0x06,
	0xA0,0x08,
	0x3C,0x0A,
	0x0E,0x0C,
	0x1A,0x0E,

	0x0C,0x10,
	0x18,0x12,
	0x30,0x14,
	0x60,0x16,
	0xC0,0x18,
	0x48,0x1A,
	0x10,0x1C,
	0x20,0x1E
};

class SoundGen {
protected:
	byte wavehold;
	uint32 freq;	// short
	uint32 CurD;

public:
	byte Timer;
	int32 Pos;
	uint32 Cycles;	// short

	inline byte GetTimer() const { return Timer; }
};

class Square : public SoundGen {
protected:
	byte volume, envelope, duty, swpspeed, swpdir, swpstep, swpenab;
	byte Vol;
	byte EnvCtr, Envelope, BendCtr;
	bool Enabled, ValidFreq, Active;
	bool EnvClk, SwpClk;

	void CheckActive();

public:
	void Reset();
	void Write(int Reg, byte Val);
	void Run();
	void QuarterFrame();
	void HalfFrame();
};

static const int8 Duties[4][8] = {
	{-4,+4,-4,-4,-4,-4,-4,-4},
	{-4,+4,+4,-4,-4,-4,-4,-4},
	{-4,+4,+4,+4,+4,-4,-4,-4},
	{+4,-4,-4,+4,+4,+4,+4,+4}
};

void Square::Reset() {
	memset(this, 0, sizeof(*this));
	Cycles = 1;
	EnvCtr = 1;
	BendCtr = 1;
}

void Square::CheckActive() {
	ValidFreq = (freq >= 0x8) && ((swpdir) || !((freq + (freq >> swpstep)) & 0x800));
	Active = Timer && ValidFreq;
	Pos = Active ? (Duties[duty][CurD] * Vol) : 0;
}

void Square::Write(int Reg, byte Val) {
	switch (Reg) {
	case 0:
		volume = Val & 0xF;
		envelope = Val & 0x10;
		wavehold = Val & 0x20;
		duty = (Val >> 6) & 0x3;
		Vol = envelope ? volume : Envelope;
		break;

	case 1:
		swpstep = Val & 0x07;
		swpdir = Val & 0x08;
		swpspeed = (Val >> 4) & 0x7;
		swpenab = Val & 0x80;
		SwpClk = true;
		break;

	case 2:
		freq &= 0x700;
		freq |= Val;
		break;

	case 3:
		freq &= 0xFF;
		freq |= (Val & 0x7) << 8;

		if (Enabled)
			Timer = LengthCounts[(Val >> 3) & 0x1F];

		CurD = 0;
		EnvClk = true;
		break;

	case 4:
		Enabled = (Val != 0);
		if (!Enabled)
			Timer = 0;
		break;
	}
	CheckActive();
}

void Square::Run() {
	Cycles = (freq + 1) << 1;
	CurD = (CurD + 1) & 0x7;

	if (Active)
		Pos = Duties[duty][CurD] * Vol;
}

void Square::QuarterFrame() {
	if (EnvClk) {
		EnvClk = false;
		Envelope = 0xF;
		EnvCtr = volume + 1;
	} else if (!--EnvCtr) {
		EnvCtr = volume + 1;

		if (Envelope)
			Envelope--;
		else
			Envelope = wavehold ? 0xF : 0x0;
	}

	Vol = envelope ? volume : Envelope;
	CheckActive();
}

void Square::HalfFrame() {
	if (!--BendCtr) {
		BendCtr = swpspeed + 1;

		if (swpenab && swpstep && ValidFreq) {
			int sweep = freq >> swpstep;
			// FIXME: Is -sweep or ~sweep correct???
			freq += swpdir ? -sweep : sweep;
		}
	}

	if (SwpClk) {
		SwpClk = false;
		BendCtr = swpspeed + 1;
	}

	if (Timer && !wavehold)
		Timer--;

	CheckActive();
}


class Triangle : public SoundGen {
protected:
	byte linear;
	byte LinCtr;
	bool Enabled, Active;
	bool LinClk;

	void CheckActive();

public:
	void Reset();
	void Write(int Reg, byte Val);
	void Run();
	void QuarterFrame();
	void HalfFrame();
};

static const int8 TriDuty[32] = {
	-8,-7,-6,-5,-4,-3,-2,-1,
	+0,+1,+2,+3,+4,+5,+6,+7,
	+7,+6,+5,+4,+3,+2,+1,+0,
	-1,-2,-3,-4,-5,-6,-7,-8
};

void Triangle::Reset() {
	memset(this, 0, sizeof(*this));
	Cycles = 1;
}

void Triangle::CheckActive() {
	Active = Timer && LinCtr;

	if (freq < 4)
		Pos = 0;	// beyond hearing range
	else
		Pos = TriDuty[CurD] * 8;
}

void Triangle::Write(int Reg, byte Val) {
	switch (Reg) {
	case 0:
		linear = Val & 0x7F;
		wavehold = (Val >> 7) & 0x1;
		break;

	case 2:
		freq &= 0x700;
		freq |= Val;
		break;

	case 3:
		freq &= 0xFF;
		freq |= (Val & 0x7) << 8;

		if (Enabled)
			Timer = LengthCounts[(Val >> 3) & 0x1F];

		LinClk = true;
		break;

	case 4:
		Enabled = (Val != 0);
		if (!Enabled)
			Timer = 0;
		break;
	}
	CheckActive();
}

void Triangle::Run() {
	Cycles = freq + 1;

	if (Active) {
		CurD++;
		CurD &= 0x1F;

		if (freq < 4)
			Pos = 0;	// beyond hearing range
		else
			Pos = TriDuty[CurD] * 8;
	}
}

void Triangle::QuarterFrame() {
	if (LinClk)
		LinCtr = linear;
	else if (LinCtr)
		LinCtr--;

	if (!wavehold)
		LinClk = false;

	CheckActive();
}

void Triangle::HalfFrame() {
	if (Timer && !wavehold)
		Timer--;

	CheckActive();
}

class Noise : public SoundGen {
protected:
	byte volume, envelope, datatype;
	byte Vol;
	byte EnvCtr, Envelope;
	bool Enabled;
	bool EnvClk;

	void CheckActive();

public:
	void Reset();
	void Write(int Reg, byte Val);
	void Run();
	void QuarterFrame();
	void HalfFrame();
};

static const uint32 NoiseFreq[16] = {
	0x004,0x008,0x010,0x020,0x040,0x060,0x080,0x0A0,
	0x0CA,0x0FE,0x17C,0x1FC,0x2FA,0x3F8,0x7F2,0xFE4
};

void Noise::Reset() {
	memset(this, 0, sizeof(*this));
	CurD = 1;
	Cycles = 1;
	EnvCtr = 1;

}

void Noise::Write(int Reg, byte Val) {
	switch (Reg) {
	case 0:
		volume = Val & 0x0F;
		envelope = Val & 0x10;
		wavehold = Val & 0x20;
		Vol = envelope ? volume : Envelope;

		if (Timer)
			Pos = ((CurD & 0x4000) ? -2 : 2) * Vol;
		break;

	case 2:
		freq = Val & 0xF;
		datatype = Val & 0x80;
		break;

	case 3:
		if (Enabled)
			Timer = LengthCounts[(Val >> 3) & 0x1F];

		EnvClk = true;
		break;

	case 4:
		Enabled = (Val != 0);
		if (!Enabled)
			Timer = 0;
		break;
	}
}

void Noise::Run() {
	Cycles = NoiseFreq[freq];	/* no + 1 here */

	if (datatype)
		CurD = (CurD << 1) | (((CurD >> 14) ^ (CurD >> 8)) & 0x1);
	else
		CurD = (CurD << 1) | (((CurD >> 14) ^ (CurD >> 13)) & 0x1);

	if (Timer)
		Pos = ((CurD & 0x4000) ? -2 : 2) * Vol;
}

void Noise::QuarterFrame() {
	if (EnvClk) {
		EnvClk = false;
		Envelope = 0xF;
		EnvCtr = volume + 1;
	} else if (!--EnvCtr) {
		EnvCtr = volume + 1;

		if (Envelope)
			Envelope--;
		else
			Envelope = wavehold ? 0xF : 0x0;
	}

	Vol = envelope ? volume : Envelope;

	if (Timer)
		Pos = ((CurD & 0x4000) ? -2 : 2) * Vol;
}

void Noise::HalfFrame() {
	if (Timer && !wavehold)
		Timer--;
}

class APU {
protected:
	int	BufPos;
	int	SampleRate;

	Square _square0;
	Square _square1;
	Triangle _triangle;
	Noise _noise;

	struct {
		uint32 Cycles;
		int Num;
	} Frame;

public:
	APU(int rate) : SampleRate(rate) {
		Reset();
	}

	void WriteReg(int Addr, byte Val);
	byte Read4015();
	void Reset ();
	int16 GetSample();
};

void APU::WriteReg(int Addr, byte Val) {
	switch (Addr) {
	case 0x000:	_square0.Write(0,Val);	break;
	case 0x001:	_square0.Write(1,Val);	break;
	case 0x002:	_square0.Write(2,Val);	break;
	case 0x003:	_square0.Write(3,Val);	break;
	case 0x004:	_square1.Write(0,Val);	break;
	case 0x005:	_square1.Write(1,Val);	break;
	case 0x006:	_square1.Write(2,Val);	break;
	case 0x007:	_square1.Write(3,Val);	break;
	case 0x008:	_triangle.Write(0,Val);	break;
	case 0x009:	_triangle.Write(1,Val);	break;
	case 0x00A:	_triangle.Write(2,Val);	break;
	case 0x00B:	_triangle.Write(3,Val);	break;
	case 0x00C:	_noise.Write(0,Val);	break;
	case 0x00D:	_noise.Write(1,Val);	break;
	case 0x00E:	_noise.Write(2,Val);	break;
	case 0x00F:	_noise.Write(3,Val);	break;
	case 0x015:	_square0.Write(4,Val & 0x1);
				_square1.Write(4,Val & 0x2);
				_triangle.Write(4,Val & 0x4);
				_noise.Write(4,Val & 0x8);
		break;
	}
}

byte APU::Read4015() {
	byte result =
		(( _square0.GetTimer()) ? 0x01 : 0) |
		(( _square1.GetTimer()) ? 0x02 : 0) |
		((_triangle.GetTimer()) ? 0x04 : 0) |
		((   _noise.GetTimer()) ? 0x08 : 0);
	return result;
}

void APU::Reset () {
	BufPos = 0;

	_square0.Reset();
	_square1.Reset();
	_triangle.Reset();
	_noise.Reset();

	Frame.Num = 0;
	Frame.Cycles = 1;
}

template<class T>
int step(T &obj, int sampcycles, uint frame_Cycles, int frame_Num) {
	int samppos = 0;
	while (sampcycles) {
		// Compute the maximal amount we can step ahead before triggering
		// an action (i.e. compute the minimum of sampcycles, frame_Cycles
		// and obj.Cycles).
		uint max_step = sampcycles;
		if (max_step > frame_Cycles)
			max_step = frame_Cycles;
		if (max_step > obj.Cycles)
			max_step = obj.Cycles;

		// During all but the last of these steps, we just add the value of obj.Pos
		// to samppos -- so we can to that all at once with a simple multiplication:
		samppos += obj.Pos * (max_step - 1);

		// Now step ahead...
		sampcycles -= max_step;
		frame_Cycles -= max_step;
		obj.Cycles -= max_step;

		if (!frame_Cycles) {
			frame_Cycles = 7457;

			if (frame_Num < 4) {
				obj.QuarterFrame();

				if (frame_Num & 1)
					frame_Cycles++;
				else
					obj.HalfFrame();

				frame_Num++;
			} else
				frame_Num = 0;
		}

		if (!obj.Cycles)
			obj.Run();

		samppos += obj.Pos;
	}

	return samppos;
}

int16 APU::GetSample() {
	int samppos = 0;

	const int sampcycles = 1+(1789773-BufPos-1)/SampleRate;
	BufPos = BufPos + sampcycles * SampleRate - 1789773;

	samppos += step( _square0, sampcycles, Frame.Cycles, Frame.Num);
	samppos += step( _square1, sampcycles, Frame.Cycles, Frame.Num);
	samppos += step(_triangle, sampcycles, Frame.Cycles, Frame.Num);
	samppos += step(   _noise, sampcycles, Frame.Cycles, Frame.Num);

	uint tmp = sampcycles;
	while (tmp >= Frame.Cycles) {
		tmp -= Frame.Cycles;
		Frame.Cycles = 7457;

		if (Frame.Num < 4) {
			if (Frame.Num & 1)
				Frame.Cycles++;
			Frame.Num++;
		} else
			Frame.Num = 0;
	}

	Frame.Cycles -= tmp;

	return (samppos << 6) / sampcycles;
}

} // End of namespace APUe

Player_NES::Player_NES(ScummEngine *scumm, Audio::Mixer *mixer) {
	int i;
	_vm = scumm;
	_mixer = mixer;
	_sampleRate = _mixer->getOutputRate();
	_apu = new APUe::APU(_sampleRate);

	_samples_per_frame = _sampleRate / 60;
	_current_sample = 0;

	for (i = 0; i < NUMSLOTS; i++) {
		_slot[i].id = -1;
		_slot[i].framesleft = 0;
		_slot[i].type = 0;
		_slot[i].offset = 0;
		_slot[i].data = NULL;
	}

	for (i = 0; i < NUMCHANS; i++) {
		_mchan[i].command = 0;
		_mchan[i].framedelay = 0;
		_mchan[i].pitch = 0;
		_mchan[i].volume = 0;
		_mchan[i].voldelta = 0;
		_mchan[i].envflags = 0;
		_mchan[i].cmdlock = 0;
	}
	isSFXplaying = wasSFXplaying = false;

	auxData1 = auxData2 = NULL;
	numNotes = 0;

	APU_writeControl(0);

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

Player_NES::~Player_NES() {
	_mixer->stopHandle(_soundHandle);
	delete _apu;
}

void Player_NES::setMusicVolume (int vol) {
	_maxvol = vol;
}

int Player_NES::readBuffer(int16 *buffer, const int numSamples) {
	for (int n = 0; n < numSamples; n++) {
		buffer[n] = _apu->GetSample() * _maxvol / 255;
		_current_sample++;

		if (_current_sample == _samples_per_frame) {
			_current_sample = 0;
			sound_play();
		}
	}
	return numSamples;
}
void Player_NES::stopAllSounds() {
	for (int i = 0; i < NUMSLOTS; i++) {
		_slot[i].framesleft = 0;
		_slot[i].type = 0;
		_slot[i].id = -1;
	}

	isSFXplaying = 0;
	checkSilenceChannels(0);
}

void Player_NES::stopSound(int nr) {
	if (nr == -1)
		return;

	for (int i = 0; i < NUMSLOTS; i++) {
		if (_slot[i].id != nr)
			continue;

		isSFXplaying = 0;
		_slot[i].framesleft = 0;
		_slot[i].type = 0;
		_slot[i].id = -1;
		checkSilenceChannels(i);
	}
}

void Player_NES::startSound(int nr) {
	byte *data = _vm->getResourceAddress(rtSound, nr) + 2;
	assert(data);

	int soundType = data[1];
	int chan = data[0];

	if (chan == 4) {
		if (_slot[2].framesleft)
			return;
		chan = 0;
	}

	if (soundType < _slot[chan].type)
		return;

	_slot[chan].type = soundType;
	_slot[chan].id = nr;
	_slot[chan].data = data;
	_slot[chan].offset = 2;
	_slot[chan].framesleft = 1;
	checkSilenceChannels(chan);
	if (chan == 2) {
		numNotes = _slot[chan].data[2];
		auxData1 = _slot[chan].data + 3;
		auxData2 = auxData1 + numNotes;
		_slot[chan].data = auxData2 + numNotes;
		_slot[chan].offset = 0;

		for (int i = 0; i < NUMCHANS; i++)
			_mchan[i].cmdlock = 0;
	}
}

void Player_NES::checkSilenceChannels(int chan) {
	for (chan--; chan >= 0; chan--) {
		if (_slot[chan].framesleft)
			return;
	}
	APU_writeControl(0);
}

void Player_NES::sound_play() {
	if (_slot[0].framesleft)
		playSFX(0);
	else if (_slot[1].framesleft)
		playSFX(1);

	playMusic();
}

void Player_NES::playSFX (int nr) {
	if (--_slot[nr].framesleft)
		return;

	while (1) {
		int a = _slot[nr].data[_slot[nr].offset++];
		if (a < 16) {
			a >>= 2;
			APU_writeControl(APU_readStatus() | channelMask[a]);
			isSFXplaying = true;
			APU_writeChannel(a, 0, _slot[nr].data[_slot[nr].offset++]);
			APU_writeChannel(a, 1, _slot[nr].data[_slot[nr].offset++]);
			APU_writeChannel(a, 2, _slot[nr].data[_slot[nr].offset++]);
			APU_writeChannel(a, 3, _slot[nr].data[_slot[nr].offset++]);
		} else if (a == 0xFE) {
			_slot[nr].offset = 2;
		} else if (a == 0xFF) {
			_slot[nr].id = -1;
			_slot[nr].type = 0;
			isSFXplaying = false;
			APU_writeControl(0);

			if (!nr && _slot[1].framesleft) {
				_slot[1].framesleft = 1;
				isSFXplaying = true;
			}
			return;
		} else {
			_slot[nr].framesleft = _slot[nr].data[_slot[nr].offset++];
			return;
		}
	}
}

void Player_NES::playMusic() {
	if (!_slot[2].framesleft)
		return;

	if (wasSFXplaying && !isSFXplaying)
		for (int x = 1; x >= 0; x--)
			if (_mchan[x].cmdlock) {
				_mchan[x].command = _mchan[x].cmdlock;
				_mchan[x].framedelay = 1;
			}

	wasSFXplaying = isSFXplaying;
	if (!--_slot[2].framesleft) {
top:
		int b = _slot[2].data[_slot[2].offset++];
		if (b == 0xFF) {
			_slot[2].id = -1;
			_slot[2].type = 0;
			b = 0;
		} else if (b == 0xFE) {
			_slot[2].offset = 0;
			goto top;
		} else {
			if (b < numNotes) {
				int inst = auxData1[b];
				int ch = instChannel[inst];
				_mchan[ch].pitch = auxData2[b];
				_mchan[ch].cmdlock = startCmd[inst];
				_mchan[ch].command = startCmd[inst];
				_mchan[ch].framedelay = 1;
				goto top;
			}
			b -= numNotes;
			if (b < 16) {
				int inst = b;
				int ch = instChannel[inst];
				_mchan[ch].cmdlock = 0;
				_mchan[ch].command = releaseCmd[inst];
				_mchan[ch].framedelay = 1;
				goto top;
			}
			b -= 16;
		}
		_slot[2].framesleft = b;
	}

	for (int x = NUMCHANS - 1; x >= 0; x--) {
		if (_slot[0].framesleft || _slot[1].framesleft) {
			_mchan[x].volume = 0;
			_mchan[x].framedelay = 0;
			continue;
		}

		if (_mchan[x].framedelay && !--_mchan[x].framedelay) {
			switch (_mchan[x].command) {
			case 0x00:
			case 0x13:
				_mchan[x].voldelta = -10;
				break;

			case 0x01:
			case 0x03:
			case 0x08:
			case 0x16:
				_mchan[x].envflags = 0x30;
				_mchan[x].volume = 0x6F;
				_mchan[x].voldelta = 0;

				APU_writeChannel(x, 0, 0x00);
				APU_writeChannel(x, 1, 0x7F);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x02:
				_mchan[x].envflags = 0xB0;
				_mchan[x].volume = 0x6F;
				_mchan[x].voldelta = 0;

				APU_writeChannel(x, 0, 0x00);
				APU_writeChannel(x, 1, 0x84);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x04:
				_mchan[x].envflags = 0x80;
				_mchan[x].volume = 0x6F;
				_mchan[x].voldelta = 0;

				APU_writeChannel(x, 0, 0x00);
				APU_writeChannel(x, 1, 0x7F);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x05:
				_mchan[x].envflags = 0xF0;
				_mchan[x].volume = 0x6F;
				_mchan[x].voldelta = -15;

				APU_writeChannel(x, 1, 0x7F);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x06:
				_mchan[x].pitch += 0x18;
				_mchan[x].envflags = 0x80;
				_mchan[x].volume = 0x6F;
				_mchan[x].voldelta = 0;

				APU_writeChannel(x, 0, 0x00);
				APU_writeChannel(x, 1, 0x7F);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x07:
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch - 0x0C] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch - 0x0C] >> 8);

				chainCommand(x);
				break;

			case 0x09:
				_mchan[x].voldelta = -2;

				APU_writeChannel(x, 1, 0x7F);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x0A:
				APU_writeChannel(x, 1, 0x86);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x0B:	case 0x1A:
				_mchan[x].envflags = 0x70;
				_mchan[x].volume = 0x6F;
				_mchan[x].voldelta = 0;

				APU_writeChannel(x, 0, 0x00);
				APU_writeChannel(x, 1, 0x7F);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x0C:
				_mchan[x].envflags = 0xB0;

				chainCommand(x);
				break;

			case 0x0D:
				_mchan[x].envflags = 0x30;
				_mchan[x].volume = 0x5F;
				_mchan[x].voldelta = -22;

				APU_writeChannel(x, 0, 0x00);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, _mchan[x].pitch & 0xF);
				APU_writeChannel(x, 3, 0xFF);

				chainCommand(x);
				break;

			case 0x0E:
			case 0x10:
				_mchan[x].envflags = 0x30;
				_mchan[x].volume = 0x5F;
				_mchan[x].voldelta = -6;

				APU_writeChannel(x, 0, 0x00);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, _mchan[x].pitch & 0xF);
				APU_writeChannel(x, 3, 0xFF);

				chainCommand(x);
				break;

			case 0x0F:
				chainCommand(x);
				break;

			case 0x11:
				APU_writeChannel(x, 2, _mchan[x].pitch & 0xF);
				APU_writeChannel(x, 3, 0xFF);

				chainCommand(x);
				break;

			case 0x12:
				APU_writeChannel(x, 2, (_mchan[x].pitch + 3) & 0xF);
				APU_writeChannel(x, 3, 0xFF);

				chainCommand(x);
				break;

			case 0x14:
				_mchan[x].voldelta = -12;

				APU_writeChannel(x, 1, 0x8C);

				chainCommand(x);
				break;

			case 0x15:
				_mchan[x].voldelta = -12;

				APU_writeChannel(x, 1, 0x84);

				chainCommand(x);
				break;

			case 0x17:
				_mchan[x].pitch += 0x0C;
				_mchan[x].envflags = 0x80;
				_mchan[x].volume = 0x6F;
				_mchan[x].voldelta = 0;

				APU_writeChannel(x, 0, 0x00);
				APU_writeChannel(x, 1, 0x7F);
				APU_writeControl(APU_readStatus() | channelMask[x]);
				APU_writeChannel(x, 2, freqTable[_mchan[x].pitch] & 0xFF);
				APU_writeChannel(x, 3, freqTable[_mchan[x].pitch] >> 8);

				chainCommand(x);
				break;

			case 0x18:
				_mchan[x].envflags = 0x70;

				chainCommand(x);
				break;

			case 0x19:
				_mchan[x].envflags = 0xB0;

				chainCommand(x);
				break;

			case 0x1B:
				_mchan[x].envflags = 0x00;
				_mchan[x].voldelta = -10;
				break;
			}
		}

		_mchan[x].volume += _mchan[x].voldelta;

		if (_mchan[x].volume < 0)
			_mchan[x].volume = 0;
		if (_mchan[x].volume > MAXVOLUME)
			_mchan[x].volume = MAXVOLUME;

		APU_writeChannel(x, 0, (_mchan[x].volume >> 3) | _mchan[x].envflags);
	}
}

void Player_NES::chainCommand(int c) {
	int i = _mchan[c].command;
	_mchan[c].command = nextCmd[i];
	_mchan[c].framedelay = nextDelay[i];
}

int Player_NES::getSoundStatus(int nr) const {
	for (int i = 0; i < NUMSLOTS; i++)
		if (_slot[i].id == nr)
			return 1;
	return 0;
}

void Player_NES::APU_writeChannel(int chan, int offset, byte value) {
	_apu->WriteReg(0x000 + 4 * chan + offset, value);
}
void Player_NES::APU_writeControl(byte value) {
	_apu->WriteReg(0x015, value);
}
byte Player_NES::APU_readStatus() {
	return _apu->Read4015();
}

} // End of namespace Scumm

#endif // DISABLE_NES_APU
