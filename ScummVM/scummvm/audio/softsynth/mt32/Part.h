/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MT32EMU_PART_H
#define MT32EMU_PART_H

#include <common/list.h>

namespace MT32Emu {

class PartialManager;
class Synth;

class Part {
private:
	// Direct pointer to sysex-addressable memory dedicated to this part (valid for parts 1-8, NULL for rhythm)
	TimbreParam *timbreTemp;

	// 0=Part 1, .. 7=Part 8, 8=Rhythm
	unsigned int partNum;

	bool holdpedal;

	unsigned int activePartialCount;
	PatchCache patchCache[4];
	Common::List<Poly *> freePolys;
	Common::List<Poly *> activePolys;

	void setPatch(const PatchParam *patch);
	unsigned int midiKeyToKey(unsigned int midiKey);

	void abortPoly(Poly *poly);
	bool abortFirstPoly(unsigned int key);

protected:
	Synth *synth;
	// Direct pointer into sysex-addressable memory
	MemParams::PatchTemp *patchTemp;
	char name[8]; // "Part 1".."Part 8", "Rhythm"
	char currentInstr[11];
	Bit8u modulation;
	Bit8u expression;
	Bit32s pitchBend;
	bool nrpn;
	Bit16u rpn;
	Bit16u pitchBenderRange; // (patchTemp->patch.benderRange * 683) at the time of the last MIDI program change or MIDI data entry.

	void backupCacheToPartials(PatchCache cache[4]);
	void cacheTimbre(PatchCache cache[4], const TimbreParam *timbre);
	void playPoly(const PatchCache cache[4], const MemParams::RhythmTemp *rhythmTemp, unsigned int midiKey, unsigned int key, unsigned int velocity);
	void stopNote(unsigned int key);
	const char *getName() const;

public:
	Part(Synth *synth, unsigned int usePartNum);
	virtual ~Part();
	void reset();
	void setDataEntryMSB(unsigned char midiDataEntryMSB);
	void setNRPN();
	void setRPNLSB(unsigned char midiRPNLSB);
	void setRPNMSB(unsigned char midiRPNMSB);
	void resetAllControllers();
	virtual void noteOn(unsigned int midiKey, unsigned int velocity);
	virtual void noteOff(unsigned int midiKey);
	void allNotesOff();
	void allSoundOff();
	Bit8u getVolume() const; // Internal volume, 0-100, exposed for use by ExternalInterface
	void setVolume(unsigned int midiVolume);
	Bit8u getModulation() const;
	void setModulation(unsigned int midiModulation);
	Bit8u getExpression() const;
	void setExpression(unsigned int midiExpression);
	virtual void setPan(unsigned int midiPan);
	Bit32s getPitchBend() const;
	void setBend(unsigned int midiBend);
	virtual void setProgram(unsigned int midiProgram);
	void setHoldPedal(bool pedalval);
	void stopPedalHold();
	void updatePitchBenderRange();
	virtual void refresh();
	virtual void refreshTimbre(unsigned int absTimbreNum);
	virtual void setTimbre(TimbreParam *timbre);
	virtual unsigned int getAbsTimbreNum() const;
	const char *getCurrentInstr() const;
	unsigned int getActivePartialCount() const;
	unsigned int getActiveNonReleasingPartialCount() const;

	const MemParams::PatchTemp *getPatchTemp() const;

	// This should only be called by Poly
	void partialDeactivated(Poly *poly);

	// These are rather specialised, and should probably only be used by PartialManager
	bool abortFirstPoly(PolyState polyState);
	// Abort the first poly in PolyState_HELD, or if none exists, the first active poly in any state.
	bool abortFirstPolyPreferHeld();
	bool abortFirstPoly();
};

class RhythmPart: public Part {
	// Pointer to the area of the MT-32's memory dedicated to rhythm
	const MemParams::RhythmTemp *rhythmTemp;

	// This caches the timbres/settings in use by the rhythm part
	PatchCache drumCache[85][4];
public:
	RhythmPart(Synth *synth, unsigned int usePartNum);
	void refresh();
	void refreshTimbre(unsigned int timbreNum);
	void setTimbre(TimbreParam *timbre);
	void noteOn(unsigned int key, unsigned int velocity);
	void noteOff(unsigned int midiKey);
	unsigned int getAbsTimbreNum() const;
	void setPan(unsigned int midiPan);
	void setProgram(unsigned int patchNum);
};

}
#endif
