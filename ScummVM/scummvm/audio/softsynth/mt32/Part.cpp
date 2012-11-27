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

//#include <cstdio>
//#include <cstring>

#include "mt32emu.h"
#include "PartialManager.h"

namespace MT32Emu {

static const Bit8u PartialStruct[13] = {
	0, 0, 2, 2, 1, 3,
	3, 0, 3, 0, 2, 1, 3
};

static const Bit8u PartialMixStruct[13] = {
	0, 1, 0, 1, 1, 0,
	1, 3, 3, 2, 2, 2, 2
};

static const float floatKeyfollow[17] = {
	-1.0f, -1.0f / 2.0f, -1.0f / 4.0f, 0.0f,
	1.0f / 8.0f, 1.0f / 4.0f, 3.0f / 8.0f, 1.0f / 2.0f, 5.0f / 8.0f, 3.0f / 4.0f, 7.0f / 8.0f, 1.0f,
	5.0f / 4.0f, 3.0f / 2.0f, 2.0f,
	1.0009765625f, 1.0048828125f
};


RhythmPart::RhythmPart(Synth *useSynth, unsigned int usePartNum): Part(useSynth, usePartNum) {
	strcpy(name, "Rhythm");
	rhythmTemp = &synth->mt32ram.rhythmTemp[0];
	refresh();
}

Part::Part(Synth *useSynth, unsigned int usePartNum) {
	synth = useSynth;
	partNum = usePartNum;
	patchCache[0].dirty = true;
	holdpedal = false;
	patchTemp = &synth->mt32ram.patchTemp[partNum];
	if (usePartNum == 8) {
		// Nasty hack for rhythm
		timbreTemp = NULL;
	} else {
		sprintf(name, "Part %d", partNum + 1);
		timbreTemp = &synth->mt32ram.timbreTemp[partNum];
	}
	currentInstr[0] = 0;
	currentInstr[10] = 0;
	modulation = 0;
	expression = 100;
	pitchBend = 0;
	activePartialCount = 0;
	memset(patchCache, 0, sizeof(patchCache));
	for (int i = 0; i < MT32EMU_MAX_POLY; i++) {
		freePolys.push_front(new Poly(this));
	}
}

Part::~Part() {
	while (!activePolys.empty()) {
		delete activePolys.front();
		activePolys.pop_front();
	}
	while (!freePolys.empty()) {
		delete freePolys.front();
		freePolys.pop_front();
	}
}

void Part::setDataEntryMSB(unsigned char midiDataEntryMSB) {
	if (nrpn) {
		// The last RPN-related control change was for an NRPN,
		// which the real synths don't support.
		return;
	}
	if (rpn != 0) {
		// The RPN has been set to something other than 0,
		// which is the only RPN that these synths support
		return;
	}
	patchTemp->patch.benderRange = midiDataEntryMSB > 24 ? 24 : midiDataEntryMSB;
	updatePitchBenderRange();
}

void Part::setNRPN() {
	nrpn = true;
}

void Part::setRPNLSB(unsigned char midiRPNLSB) {
	nrpn = false;
	rpn = (rpn & 0xFF00) | midiRPNLSB;
}

void Part::setRPNMSB(unsigned char midiRPNMSB) {
	nrpn = false;
	rpn = (rpn & 0x00FF) | (midiRPNMSB << 8);
}

void Part::setHoldPedal(bool pressed) {
	if (holdpedal && !pressed) {
		holdpedal = false;
		stopPedalHold();
	} else {
		holdpedal = pressed;
	}
}

Bit32s Part::getPitchBend() const {
	return pitchBend;
}

void Part::setBend(unsigned int midiBend) {
	// CONFIRMED:
	pitchBend = (((signed)midiBend - 8192) * pitchBenderRange) >> 14; // PORTABILITY NOTE: Assumes arithmetic shift
}

Bit8u Part::getModulation() const {
	return modulation;
}

void Part::setModulation(unsigned int midiModulation) {
	modulation = (Bit8u)midiModulation;
}

void Part::resetAllControllers() {
	modulation = 0;
	expression = 100;
	pitchBend = 0;
	setHoldPedal(false);
}

void Part::reset() {
	resetAllControllers();
	allSoundOff();
	rpn = 0xFFFF;
}

void RhythmPart::refresh() {
	// (Re-)cache all the mapped timbres ahead of time
	for (unsigned int drumNum = 0; drumNum < synth->controlROMMap->rhythmSettingsCount; drumNum++) {
		int drumTimbreNum = rhythmTemp[drumNum].timbre;
		if (drumTimbreNum >= 127) { // 94 on MT-32
			continue;
		}
		PatchCache *cache = drumCache[drumNum];
		backupCacheToPartials(cache);
		for (int t = 0; t < 4; t++) {
			// Common parameters, stored redundantly
			cache[t].dirty = true;
			cache[t].reverb = rhythmTemp[drumNum].reverbSwitch > 0;
		}
	}
	updatePitchBenderRange();
}

void Part::refresh() {
	backupCacheToPartials(patchCache);
	for (int t = 0; t < 4; t++) {
		// Common parameters, stored redundantly
		patchCache[t].dirty = true;
		patchCache[t].reverb = patchTemp->patch.reverbSwitch > 0;
	}
	memcpy(currentInstr, timbreTemp->common.name, 10);
	updatePitchBenderRange();
}

const char *Part::getCurrentInstr() const {
	return &currentInstr[0];
}

void RhythmPart::refreshTimbre(unsigned int absTimbreNum) {
	for (int m = 0; m < 85; m++) {
		if (rhythmTemp[m].timbre == absTimbreNum - 128) {
			drumCache[m][0].dirty = true;
		}
	}
}

void Part::refreshTimbre(unsigned int absTimbreNum) {
	if (getAbsTimbreNum() == absTimbreNum) {
		memcpy(currentInstr, timbreTemp->common.name, 10);
		patchCache[0].dirty = true;
	}
}

void Part::setPatch(const PatchParam *patch) {
	patchTemp->patch = *patch;
}

void RhythmPart::setTimbre(TimbreParam * /*timbre*/) {
	synth->printDebug("%s: Attempted to call setTimbre() - doesn't make sense for rhythm", name);
}

void Part::setTimbre(TimbreParam *timbre) {
	*timbreTemp = *timbre;
}

unsigned int RhythmPart::getAbsTimbreNum() const {
	synth->printDebug("%s: Attempted to call getAbsTimbreNum() - doesn't make sense for rhythm", name);
	return 0;
}

unsigned int Part::getAbsTimbreNum() const {
	return (patchTemp->patch.timbreGroup * 64) + patchTemp->patch.timbreNum;
}

#if MT32EMU_MONITOR_MIDI > 0
void RhythmPart::setProgram(unsigned int patchNum) {
	synth->printDebug("%s: Attempt to set program (%d) on rhythm is invalid", name, patchNum);
}
#else
void RhythmPart::setProgram(unsigned int) { }
#endif

void Part::setProgram(unsigned int patchNum) {
	setPatch(&synth->mt32ram.patches[patchNum]);
	holdpedal = false;
	allSoundOff();
	setTimbre(&synth->mt32ram.timbres[getAbsTimbreNum()].timbre);
	refresh();
}

void Part::updatePitchBenderRange() {
	pitchBenderRange = patchTemp->patch.benderRange * 683;
}

void Part::backupCacheToPartials(PatchCache cache[4]) {
	// check if any partials are still playing with the old patch cache
	// if so then duplicate the cached data from the part to the partial so that
	// we can change the part's cache without affecting the partial.
	// We delay this until now to avoid a copy operation with every note played
	for (Common::List<Poly *>::iterator polyIt = activePolys.begin(); polyIt != activePolys.end(); polyIt++) {
		(*polyIt)->backupCacheToPartials(cache);
	}
}

void Part::cacheTimbre(PatchCache cache[4], const TimbreParam *timbre) {
	backupCacheToPartials(cache);
	int partialCount = 0;
	for (int t = 0; t < 4; t++) {
		if (((timbre->common.partialMute >> t) & 0x1) == 1) {
			cache[t].playPartial = true;
			partialCount++;
		} else {
			cache[t].playPartial = false;
			continue;
		}

		// Calculate and cache common parameters
		cache[t].srcPartial = timbre->partial[t];

		cache[t].pcm = timbre->partial[t].wg.pcmWave;

		switch (t) {
		case 0:
			cache[t].PCMPartial = (PartialStruct[(int)timbre->common.partialStructure12] & 0x2) ? true : false;
			cache[t].structureMix = PartialMixStruct[(int)timbre->common.partialStructure12];
			cache[t].structurePosition = 0;
			cache[t].structurePair = 1;
			break;
		case 1:
			cache[t].PCMPartial = (PartialStruct[(int)timbre->common.partialStructure12] & 0x1) ? true : false;
			cache[t].structureMix = PartialMixStruct[(int)timbre->common.partialStructure12];
			cache[t].structurePosition = 1;
			cache[t].structurePair = 0;
			break;
		case 2:
			cache[t].PCMPartial = (PartialStruct[(int)timbre->common.partialStructure34] & 0x2) ? true : false;
			cache[t].structureMix = PartialMixStruct[(int)timbre->common.partialStructure34];
			cache[t].structurePosition = 0;
			cache[t].structurePair = 3;
			break;
		case 3:
			cache[t].PCMPartial = (PartialStruct[(int)timbre->common.partialStructure34] & 0x1) ? true : false;
			cache[t].structureMix = PartialMixStruct[(int)timbre->common.partialStructure34];
			cache[t].structurePosition = 1;
			cache[t].structurePair = 2;
			break;
		default:
			break;
		}

		cache[t].partialParam = &timbre->partial[t];

		cache[t].waveform = timbre->partial[t].wg.waveform;
	}
	for (int t = 0; t < 4; t++) {
		// Common parameters, stored redundantly
		cache[t].dirty = false;
		cache[t].partialCount = partialCount;
		cache[t].sustain = (timbre->common.noSustain == 0);
	}
	//synth->printDebug("Res 1: %d 2: %d 3: %d 4: %d", cache[0].waveform, cache[1].waveform, cache[2].waveform, cache[3].waveform);

#if MT32EMU_MONITOR_INSTRUMENTS > 0
	synth->printDebug("%s (%s): Recached timbre", name, currentInstr);
	for (int i = 0; i < 4; i++) {
		synth->printDebug(" %d: play=%s, pcm=%s (%d), wave=%d", i, cache[i].playPartial ? "YES" : "NO", cache[i].PCMPartial ? "YES" : "NO", timbre->partial[i].wg.pcmWave, timbre->partial[i].wg.waveform);
	}
#endif
}

const char *Part::getName() const {
	return name;
}

void Part::setVolume(unsigned int midiVolume) {
	// CONFIRMED: This calculation matches the table used in the control ROM
	patchTemp->outputLevel = (Bit8u)(midiVolume * 100 / 127);
	//synth->printDebug("%s (%s): Set volume to %d", name, currentInstr, midiVolume);
}

Bit8u Part::getVolume() const {
	return patchTemp->outputLevel;
}

Bit8u Part::getExpression() const {
	return expression;
}

void Part::setExpression(unsigned int midiExpression) {
	// CONFIRMED: This calculation matches the table used in the control ROM
	expression = (Bit8u)(midiExpression * 100 / 127);
}

void RhythmPart::setPan(unsigned int midiPan) {
	// CONFIRMED: This does change patchTemp, but has no actual effect on playback.
#if MT32EMU_MONITOR_MIDI > 0
	synth->printDebug("%s: Pointlessly setting pan (%d) on rhythm part", name, midiPan);
#endif
	Part::setPan(midiPan);
}

void Part::setPan(unsigned int midiPan) {
	// NOTE: Panning is inverted compared to GM.

	// CM-32L: Divide by 8.5
	patchTemp->panpot = (Bit8u)((midiPan << 3) / 68);
	// FIXME: MT-32: Divide by 9
	//patchTemp->panpot = (Bit8u)(midiPan / 9);

	//synth->printDebug("%s (%s): Set pan to %d", name, currentInstr, panpot);
}

/**
 * Applies key shift to a MIDI key and converts it into an internal key value in the range 12-108.
 */
unsigned int Part::midiKeyToKey(unsigned int midiKey) {
	int key = midiKey + patchTemp->patch.keyShift;
	if (key < 36) {
		// After keyShift is applied, key < 36, so move up by octaves
		while (key < 36) {
			key += 12;
		}
	} else if (key > 132) {
		// After keyShift is applied, key > 132, so move down by octaves
		while (key > 132) {
			key -= 12;
		}
	}
	key -= 24;
	return key;
}

void RhythmPart::noteOn(unsigned int midiKey, unsigned int velocity) {
	if (midiKey < 24 || midiKey > 108) { /*> 87 on MT-32)*/
		synth->printDebug("%s: Attempted to play invalid key %d (velocity %d)", name, midiKey, velocity);
		return;
	}
	unsigned int key = midiKey;
	unsigned int drumNum = key - 24;
	int drumTimbreNum = rhythmTemp[drumNum].timbre;
	if (drumTimbreNum >= 127) { // 94 on MT-32
		synth->printDebug("%s: Attempted to play unmapped key %d (velocity %d)", name, midiKey, velocity);
		return;
	}
	// CONFIRMED: Two special cases described by Mok
	if (drumTimbreNum == 64 + 6) {
		noteOff(0);
		key = 1;
	} else if (drumTimbreNum == 64 + 7) {
		// This noteOff(0) is not performed on MT-32, only LAPC-I
		noteOff(0);
		key = 0;
	}
	int absTimbreNum = drumTimbreNum + 128;
	TimbreParam *timbre = &synth->mt32ram.timbres[absTimbreNum].timbre;
	memcpy(currentInstr, timbre->common.name, 10);
	if (drumCache[drumNum][0].dirty) {
		cacheTimbre(drumCache[drumNum], timbre);
	}
#if MT32EMU_MONITOR_INSTRUMENTS > 0
	synth->printDebug("%s (%s): Start poly (drum %d, timbre %d): midiKey %u, key %u, velo %u, mod %u, exp %u, bend %u", name, currentInstr, drumNum, absTimbreNum, midiKey, key, velocity, modulation, expression, pitchBend);
#if MT32EMU_MONITOR_INSTRUMENTS > 1
	// According to info from Mok, keyShift does not appear to affect anything on rhythm part on LAPC-I, but may do on MT-32 - needs investigation
	synth->printDebug(" Patch: (timbreGroup %u), (timbreNum %u), (keyShift %u), fineTune %u, benderRange %u, assignMode %u, (reverbSwitch %u)", patchTemp->patch.timbreGroup, patchTemp->patch.timbreNum, patchTemp->patch.keyShift, patchTemp->patch.fineTune, patchTemp->patch.benderRange, patchTemp->patch.assignMode, patchTemp->patch.reverbSwitch);
	synth->printDebug(" PatchTemp: outputLevel %u, (panpot %u)", patchTemp->outputLevel, patchTemp->panpot);
	synth->printDebug(" RhythmTemp: timbre %u, outputLevel %u, panpot %u, reverbSwitch %u", rhythmTemp[drumNum].timbre, rhythmTemp[drumNum].outputLevel, rhythmTemp[drumNum].panpot, rhythmTemp[drumNum].reverbSwitch);
#endif
#endif
	playPoly(drumCache[drumNum], &rhythmTemp[drumNum], midiKey, key, velocity);
}

void Part::noteOn(unsigned int midiKey, unsigned int velocity) {
	unsigned int key = midiKeyToKey(midiKey);
	if (patchCache[0].dirty) {
		cacheTimbre(patchCache, timbreTemp);
	}
#if MT32EMU_MONITOR_INSTRUMENTS > 0
	synth->printDebug("%s (%s): Start poly: midiKey %u, key %u, velo %u, mod %u, exp %u, bend %u", name, currentInstr, midiKey, key, velocity, modulation, expression, pitchBend);
#if MT32EMU_MONITOR_INSTRUMENTS > 1
	synth->printDebug(" Patch: timbreGroup %u, timbreNum %u, keyShift %u, fineTune %u, benderRange %u, assignMode %u, reverbSwitch %u", patchTemp->patch.timbreGroup, patchTemp->patch.timbreNum, patchTemp->patch.keyShift, patchTemp->patch.fineTune, patchTemp->patch.benderRange, patchTemp->patch.assignMode, patchTemp->patch.reverbSwitch);
	synth->printDebug(" PatchTemp: outputLevel %u, panpot %u", patchTemp->outputLevel, patchTemp->panpot);
#endif
#endif
	playPoly(patchCache, NULL, midiKey, key, velocity);
}

void Part::abortPoly(Poly *poly) {
	if (poly->startAbort()) {
		while (poly->isActive()) {
			if (!synth->prerender()) {
				synth->printDebug("%s (%s): Ran out of prerender space to abort poly gracefully", name, currentInstr);
				poly->terminate();
				break;
			}
		}
	}
}

bool Part::abortFirstPoly(unsigned int key) {
	for (Common::List<Poly *>::iterator polyIt = activePolys.begin(); polyIt != activePolys.end(); polyIt++) {
		Poly *poly = *polyIt;
		if (poly->getKey() == key) {
			abortPoly(poly);
			return true;
		}
	}
	return false;
}

bool Part::abortFirstPoly(PolyState polyState) {
	for (Common::List<Poly *>::iterator polyIt = activePolys.begin(); polyIt != activePolys.end(); polyIt++) {
		Poly *poly = *polyIt;
		if (poly->getState() == polyState) {
			abortPoly(poly);
			return true;
		}
	}
	return false;
}

bool Part::abortFirstPolyPreferHeld() {
	if (abortFirstPoly(POLY_Held)) {
		return true;
	}
	return abortFirstPoly();
}

bool Part::abortFirstPoly() {
	if (activePolys.empty()) {
		return false;
	}
	abortPoly(activePolys.front());
	return true;
}

void Part::playPoly(const PatchCache cache[4], const MemParams::RhythmTemp *rhythmTemp, unsigned int midiKey, unsigned int key, unsigned int velocity) {
	// CONFIRMED: Even in single-assign mode, we don't abort playing polys if the timbre to play is completely muted.
	unsigned int needPartials = cache[0].partialCount;
	if (needPartials == 0) {
		synth->printDebug("%s (%s): Completely muted instrument", name, currentInstr);
		return;
	}

	if ((patchTemp->patch.assignMode & 2) == 0) {
		// Single-assign mode
		abortFirstPoly(key);
	}

	if (!synth->partialManager->freePartials(needPartials, partNum)) {
#if MT32EMU_MONITOR_PARTIALS > 0
		synth->printDebug("%s (%s): Insufficient free partials to play key %d (velocity %d); needed=%d, free=%d, assignMode=%d", name, currentInstr, midiKey, velocity, needPartials, synth->partialManager->getFreePartialCount(), patchTemp->patch.assignMode);
		synth->printPartialUsage();
#endif
		return;
	}

	if (freePolys.empty()) {
		synth->printDebug("%s (%s): No free poly to play key %d (velocity %d)", name, currentInstr, midiKey, velocity);
		return;
	}
	Poly *poly = freePolys.front();
	freePolys.pop_front();
	if (patchTemp->patch.assignMode & 1) {
		// Priority to data first received
		activePolys.push_front(poly);
	} else {
		activePolys.push_back(poly);
	}

	Partial *partials[4];
	for (int x = 0; x < 4; x++) {
		if (cache[x].playPartial) {
			partials[x] = synth->partialManager->allocPartial(partNum);
			activePartialCount++;
		} else {
			partials[x] = NULL;
		}
	}
	poly->reset(key, velocity, cache[0].sustain, partials);

	for (int x = 0; x < 4; x++) {
		if (partials[x] != NULL) {
#if MT32EMU_MONITOR_PARTIALS > 2
			synth->printDebug("%s (%s): Allocated partial %d", name, currentInstr, partials[x]->debugGetPartialNum());
#endif
			partials[x]->startPartial(this, poly, &cache[x], rhythmTemp, partials[cache[x].structurePair]);
		}
	}
#if MT32EMU_MONITOR_PARTIALS > 1
	synth->printPartialUsage();
#endif
}

void Part::allNotesOff() {
	// The MIDI specification states - and Mok confirms - that all notes off (0x7B)
	// should treat the hold pedal as usual.
	for (Common::List<Poly *>::iterator polyIt = activePolys.begin(); polyIt != activePolys.end(); polyIt++) {
		Poly *poly = *polyIt;
		// FIXME: This has special handling of key 0 in NoteOff that Mok has not yet confirmed applies to AllNotesOff.
		// if (poly->canSustain() || poly->getKey() == 0) {
		// FIXME: The real devices are found to be ignoring non-sustaining polys while processing AllNotesOff. Need to be confirmed.
		if (poly->canSustain()) {
			poly->noteOff(holdpedal);
		}
	}
}

void Part::allSoundOff() {
	// MIDI "All sound off" (0x78) should release notes immediately regardless of the hold pedal.
	// This controller is not actually implemented by the synths, though (according to the docs and Mok) -
	// we're only using this method internally.
	for (Common::List<Poly *>::iterator polyIt = activePolys.begin(); polyIt != activePolys.end(); polyIt++) {
		Poly *poly = *polyIt;
		poly->startDecay();
	}
}

void Part::stopPedalHold() {
	for (Common::List<Poly *>::iterator polyIt = activePolys.begin(); polyIt != activePolys.end(); polyIt++) {
		Poly *poly = *polyIt;
		poly->stopPedalHold();
	}
}

void RhythmPart::noteOff(unsigned int midiKey) {
	stopNote(midiKey);
}

void Part::noteOff(unsigned int midiKey) {
	stopNote(midiKeyToKey(midiKey));
}

void Part::stopNote(unsigned int key) {
#if MT32EMU_MONITOR_INSTRUMENTS > 0
	synth->printDebug("%s (%s): stopping key %d", name, currentInstr, key);
#endif

	for (Common::List<Poly *>::iterator polyIt = activePolys.begin(); polyIt != activePolys.end(); polyIt++) {
		Poly *poly = *polyIt;
		// Generally, non-sustaining instruments ignore note off. They die away eventually anyway.
		// Key 0 (only used by special cases on rhythm part) reacts to note off even if non-sustaining or pedal held.
		if (poly->getKey() == key && (poly->canSustain() || key == 0)) {
			if (poly->noteOff(holdpedal && key != 0)) {
				break;
			}
		}
	}
}

const MemParams::PatchTemp *Part::getPatchTemp() const {
	return patchTemp;
}

unsigned int Part::getActivePartialCount() const {
	return activePartialCount;
}

unsigned int Part::getActiveNonReleasingPartialCount() const {
	unsigned int activeNonReleasingPartialCount = 0;
	for (Common::List<Poly *>::const_iterator polyIt = activePolys.begin(); polyIt != activePolys.end(); polyIt++) {
		Poly *poly = *polyIt;
		if (poly->getState() != POLY_Releasing) {
			activeNonReleasingPartialCount += poly->getActivePartialCount();
		}
	}
	return activeNonReleasingPartialCount;
}

void Part::partialDeactivated(Poly *poly) {
	activePartialCount--;
	if (!poly->isActive()) {
		activePolys.remove(poly);
		freePolys.push_front(poly);
	}
}

}
