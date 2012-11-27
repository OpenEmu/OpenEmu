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

#include "audio/mods/infogrames.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/textconsole.h"

namespace Audio {

Infogrames::Instruments::Instruments() {
	init();
}

Infogrames::Instruments::~Instruments() {
	delete[] _sampleData;
}

void Infogrames::Instruments::init() {
	int i;

	for (i = 0; i < 32; i++) {
		_samples[i].data = 0;
		_samples[i].dataRepeat = 0;
		_samples[i].length = 0;
		_samples[i].lengthRepeat = 0;
	}
	_count = 0;
	_sampleData = 0;
}

bool Infogrames::Instruments::load(const char *ins) {
	Common::File f;

	if (f.open(ins))
		return load(f);
	return false;
}

bool Infogrames::Instruments::load(Common::SeekableReadStream &ins) {
	int i;
	int32 fsize;
	int32 offset[32];
	int32 offsetRepeat[32];
	int32 dataOffset;

	unload();

	fsize = ins.readUint32BE();
	dataOffset = fsize;
	for (i = 0; (i < 32) && !ins.eos(); i++) {
		offset[i] = ins.readUint32BE();
		offsetRepeat[i] = ins.readUint32BE();
		if ((offset[i] > fsize) || (offsetRepeat[i] > fsize) ||
				(offset[i] < (ins.pos() + 4)) ||
				(offsetRepeat[i] < (ins.pos() + 4))) {
			// Definitely no real entry anymore
			ins.seek(-8, SEEK_CUR);
			break;
		}

		dataOffset = MIN(dataOffset, MIN(offset[i], offsetRepeat[i]));
		ins.skip(4); // Unknown
		_samples[i].length = ins.readUint16BE() * 2;
		_samples[i].lengthRepeat = ins.readUint16BE() * 2;
	}

	if (dataOffset >= fsize)
		return false;

	_count = i;
	_sampleData = new int8[fsize - dataOffset];
	ins.seek(dataOffset + 4);
	ins.read(_sampleData, fsize - dataOffset);

	for (i--; i >= 0; i--) {
		_samples[i].data = _sampleData + (offset[i] - dataOffset);
		_samples[i].dataRepeat = _sampleData + (offsetRepeat[i] - dataOffset);
	}

	return true;
}

void Infogrames::Instruments::unload() {
	delete[] _sampleData;
	init();
}

const uint8 Infogrames::tickCount[] =
	{2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96};
const uint16 Infogrames::periods[] =
	{0x6ACC, 0x64CC, 0x5F25, 0x59CE, 0x54C3, 0x5003, 0x4B86, 0x4747, 0x4346,
	 0x3F8B, 0x3BF3, 0x3892, 0x3568, 0x3269, 0x2F93, 0x2CEA, 0x2A66, 0x2801,
	 0x2566, 0x23A5, 0x21AF, 0x1FC4, 0x1DFE, 0x1C4E, 0x1ABC, 0x1936, 0x17CC,
	 0x1676, 0x1533, 0x1401, 0x12E4, 0x11D5, 0x10D4, 0x0FE3, 0x0EFE, 0x0E26,
	 0x0D5B, 0x0C9B, 0x0BE5, 0x0B3B, 0x0A9B, 0x0A02, 0x0972, 0x08E9, 0x0869,
	 0x07F1, 0x077F, 0x0713, 0x06AD, 0x064D, 0x05F2, 0x059D, 0x054D, 0x0500,
	 0x04B8, 0x0475, 0x0435, 0x03F8, 0x03BF, 0x038A, 0x0356, 0x0326, 0x02F9,
	 0x02CF, 0x02A6, 0x0280, 0x025C, 0x023A, 0x021A, 0x01FC, 0x01E0, 0x01C5,
	 0x01AB, 0x0193, 0x017D, 0x0167, 0x0153, 0x0140, 0x012E, 0x011D, 0x010D,
	 0x00FE, 0x00F0, 0x00E2, 0x00D6, 0x00CA, 0x00BE, 0x00B4, 0x00AA, 0x00A0,
	 0x0097, 0x008F, 0x0087, 0x007F, 0x0078, 0x0070, 0x0060, 0x0050, 0x0040,
	 0x0030, 0x0020, 0x0010, 0x0000, 0x0000, 0x0020, 0x2020, 0x2020, 0x2020,
	 0x2020, 0x3030, 0x3030, 0x3020, 0x2020, 0x2020, 0x2020, 0x2020, 0x2020,
	 0x2020, 0x2020, 0x2020, 0x2090, 0x4040, 0x4040, 0x4040, 0x4040, 0x4040,
	 0x4040, 0x4040, 0x400C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C40, 0x4040,
	 0x4040, 0x4040, 0x0909, 0x0909, 0x0909, 0x0101, 0x0101, 0x0101, 0x0101,
	 0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x4040, 0x4040, 0x4040,
	 0x0A0A, 0x0A0A, 0x0A0A, 0x0202, 0x0202, 0x0202, 0x0202, 0x0202, 0x0202,
	 0x0202, 0x0202, 0x0202, 0x0202, 0x4040, 0x4040, 0x2000};

Infogrames::Infogrames(Instruments &ins, bool stereo, int rate,
		int interruptFreq) : Paula(stereo, rate, interruptFreq) {
	_instruments = &ins;
	_data = 0;
	_repCount = -1;

	reset();
}

Infogrames::~Infogrames() {
	delete[] _data;
}

void Infogrames::init() {
	int i;

	_volume = 0;
	_period = 0;
	_sample = 0;
	_speedCounter = _speed;

	for (i = 0; i < 4; i++) {
		_chn[i].cmds = 0;
		_chn[i].cmdBlocks = 0;
		_chn[i].volSlide.finetuneNeg = 0;
		_chn[i].volSlide.finetunePos = 0;
		_chn[i].volSlide.data = 0;
		_chn[i].volSlide.amount = 0;
		_chn[i].volSlide.dataOffset = 0;
		_chn[i].volSlide.flags = 0;
		_chn[i].volSlide.curDelay1 = 0;
		_chn[i].volSlide.curDelay2 = 0;
		_chn[i].periodSlide.finetuneNeg = 0;
		_chn[i].periodSlide.finetunePos = 0;
		_chn[i].periodSlide.data = 0;
		_chn[i].periodSlide.amount = 0;
		_chn[i].periodSlide.dataOffset = 0;
		_chn[i].periodSlide.flags = 0;
		_chn[i].periodSlide.curDelay1 = 0;
		_chn[i].periodSlide.curDelay2 = 0;
		_chn[i].period = 0;
		_chn[i].flags = 0x81;
		_chn[i].ticks = 0;
		_chn[i].tickCount = 0;
		_chn[i].periodMod = 0;
	}

	_end = (_data == 0);
}

void Infogrames::reset() {
	int i;

	stopPlay();
	init();

	_volSlideBlocks = 0;
	_periodSlideBlocks = 0;
	_subSong = 0;
	_cmdBlocks = 0;
	_speedCounter = 0;
	_speed = 0;

	for (i = 0; i < 4; i++)
		_chn[i].cmdBlockIndices = 0;
}

bool Infogrames::load(const char *dum) {
	Common::File f;

	if (f.open(dum))
		return load(f);
	return false;
}

bool Infogrames::load(Common::SeekableReadStream &dum) {
	int subSong = 0;
	int i;
	uint32 size;

	size = dum.size();
	if (size < 20)
		return false;

	_data = new uint8[size];
	dum.seek(0);
	dum.read(_data, size);

	Common::MemoryReadStream dataStr(_data, size);

	dataStr.seek(subSong * 2);
	dataStr.seek(dataStr.readUint16BE());
	_subSong = _data + dataStr.pos();
	if (_subSong > (_data + size))
		return false;

	_speedCounter = dataStr.readUint16BE();
	_speed = _speedCounter;
	_volSlideBlocks = _subSong + dataStr.readUint16BE();
	_periodSlideBlocks = _subSong + dataStr.readUint16BE();
	for (i = 0; i < 4; i++) {
		_chn[i].cmdBlockIndices = _subSong + dataStr.readUint16BE();
		_chn[i].flags = 0x81;
	}
	_cmdBlocks = _data + dataStr.pos() + 2;

	if ((_volSlideBlocks > (_data + size)) ||
			(_periodSlideBlocks > (_data + size)) ||
			(_chn[0].cmdBlockIndices > (_data + size)) ||
			(_chn[1].cmdBlockIndices > (_data + size)) ||
			(_chn[2].cmdBlockIndices > (_data + size)) ||
			(_chn[3].cmdBlockIndices > (_data + size)) ||
			(_cmdBlocks > (_data + size)))
		return false;

	startPaula();
	return true;
}

void Infogrames::unload() {
	stopPlay();

	delete[] _data;
	_data = 0;

	clearVoices();
	reset();
}

void Infogrames::getNextSample(Channel &chn) {
	byte *data;
	byte cmdBlock = 0;
	uint16 cmd;
	bool cont = false;

	if (chn.flags & 64)
		return;

	if (chn.flags & 1) {
		chn.flags &= ~1;
		chn.cmdBlocks = chn.cmdBlockIndices;
	} else {
		chn.flags &= ~1;
		if (_speedCounter == 0)
			chn.ticks--;
		if (chn.ticks != 0) {
			_volume = MAX((int16) 0, tune(chn.volSlide, 0));
			_period = tune(chn.periodSlide, chn.period);
			return;
		} else {
			chn.ticks = chn.tickCount;
			cont = true;
		}
	}

	while (1) {
		while (cont || ((cmdBlock = *chn.cmdBlocks) != 0xFF)) {
			if (!cont) {
				chn.cmdBlocks++;
				chn.cmds = _subSong +
					READ_BE_UINT16(_cmdBlocks + (cmdBlock * 2));
			} else
				cont = false;
			while ((cmd = *chn.cmds) != 0xFF) {
				chn.cmds++;
				if (cmd & 128)
				{
					switch (cmd & 0xE0) {
					case 0x80: // 100xxxxx - Set ticks
						chn.ticks = tickCount[cmd & 0xF];
						chn.tickCount = tickCount[cmd & 0xF];
						break;
					case 0xA0: // 101xxxxx - Set sample
						_sample = cmd & 0x1F;
						break;
					case 0xC0: // 110xxxxx - Set volume slide/finetune
						data = _volSlideBlocks + (cmd & 0x1F) * 13;
						chn.volSlide.flags = (*data & 0x80) | 1;
						chn.volSlide.amount = *data++ & 0x7F;
						chn.volSlide.data = data;
						chn.volSlide.dataOffset = 0;
						chn.volSlide.finetunePos = 0;
						chn.volSlide.finetuneNeg = 0;
						chn.volSlide.curDelay1 = 0;
						chn.volSlide.curDelay2 = 0;
						break;
					case 0xE0: // 111xxxxx - Extended
						switch (cmd & 0x1F) {
						case 0: // Set period modifier
							chn.periodMod = (int8) *chn.cmds++;
							break;
						case 1: // Set continuous period slide
							chn.periodSlide.data =
								_periodSlideBlocks + *chn.cmds++ * 13 + 1;
							chn.periodSlide.amount = 0;
							chn.periodSlide.dataOffset = 0;
							chn.periodSlide.finetunePos = 0;
							chn.periodSlide.finetuneNeg = 0;
							chn.periodSlide.curDelay1 = 0;
							chn.periodSlide.curDelay2 = 0;
							chn.periodSlide.flags = 0x81;
							break;
						case 2: // Set non-continuous period slide
							chn.periodSlide.data =
								_periodSlideBlocks + *chn.cmds++ * 13 + 1;
							chn.periodSlide.amount = 0;
							chn.periodSlide.dataOffset = 0;
							chn.periodSlide.finetunePos = 0;
							chn.periodSlide.finetuneNeg = 0;
							chn.periodSlide.curDelay1 = 0;
							chn.periodSlide.curDelay2 = 0;
							chn.periodSlide.flags = 1;
							break;
						case 3: // NOP
							break;
						default:
							warning("Unknown Infogrames command: %X", cmd);
						}
						break;
					}
				} else { // 0xxxxxxx - Set period
					if (cmd != 0)
						cmd += chn.periodMod;
					chn.period = periods[cmd];
					chn.volSlide.dataOffset = 0;
					chn.volSlide.finetunePos = 0;
					chn.volSlide.finetuneNeg = 0;
					chn.volSlide.curDelay1 = 0;
					chn.volSlide.curDelay2 = 0;
					chn.volSlide.flags |= 1;
					chn.volSlide.flags &= ~4;
					chn.periodSlide.dataOffset = 0;
					chn.periodSlide.finetunePos = 0;
					chn.periodSlide.finetuneNeg = 0;
					chn.periodSlide.curDelay1 = 0;
					chn.periodSlide.curDelay2 = 0;
					chn.periodSlide.flags |= 1;
					chn.periodSlide.flags &= ~4;
					_volume = MAX((int16) 0, tune(chn.volSlide, 0));
					_period = tune(chn.periodSlide, chn.period);
					return;
				}
			}
		}
		if (!(chn.flags & 32)) {
			chn.flags |= 0x40;
			_volume = 0;
			return;
		} else
			chn.cmdBlocks = chn.cmdBlockIndices;
	}
}

int16 Infogrames::tune(Slide &slide, int16 start) const {
	byte *data;
	uint8 off;

	data = slide.data + slide.dataOffset;

	if (slide.flags & 1)
		slide.finetunePos += (int8) data[1];
	slide.flags &= ~1;

	start += slide.finetunePos - slide.finetuneNeg;
	if (start < 0)
		start = 0;

	if (slide.flags & 4)
		return start;

	slide.curDelay1++;
	if (slide.curDelay1 != data[2])
		return start;
	slide.curDelay2++;
	slide.curDelay1 = 0;
	if (slide.curDelay2 == data[0]) {
		slide.curDelay2 = 0;
		off = slide.dataOffset + 3;
		if (off == 12) {
			if (slide.flags == 0) {
				slide.flags |= 4;
				return start;
			} else {
				slide.curDelay2 = 0;
				slide.finetuneNeg += slide.amount;
				off = 3;
			}
		}
		slide.dataOffset = off;
	}
	slide.flags |= 1;
	return start;
}

void Infogrames::interrupt() {
	int chn;

	if (!_data) {
		clearVoices();
		return;
	}

	_speedCounter--;
	_sample = 0xFF;
	for (chn = 0; chn < 4; chn++) {
		_volume = 0;
		_period = 0;
		getNextSample(_chn[chn]);
		setChannelVolume(chn, _volume);
		setChannelPeriod(chn, _period);
		if ((_sample != 0xFF) && (_sample < _instruments->_count)) {
			setChannelData(chn,
			               _instruments->_samples[_sample].data,
			               _instruments->_samples[_sample].dataRepeat,
			               _instruments->_samples[_sample].length,
			               _instruments->_samples[_sample].lengthRepeat);
			_sample = 0xFF;
		}
	}
	if (_speedCounter == 0)
		_speedCounter = _speed;

	// End reached?
	if ((_chn[0].flags & 64) && (_chn[1].flags & 64) &&
			(_chn[2].flags & 64) && (_chn[3].flags & 64)) {
		if (_repCount > 0) {
			_repCount--;
			init();
		} else if (_repCount != -1) {
			stopPaula();
		} else {
			init();
		}
	}
}

} // End of namespace Audio
