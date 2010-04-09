/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aam�s                                    *
 *   aamas@stud.ntnu.no                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License version 2 for more details.                *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   version 2 along with this program; if not, write to the               *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "gambatte.h"
#include "cpu.h"
#include "savestate.h"
#include "statesaver.h"
#include "initstate.h"
#include "state_osd_elements.h"
#include <string>
#include <sstream>

static const std::string itos(int i) {
	std::stringstream ss;
	
	ss << i;
	
	std::string out;
	
	ss >> out;
	
	return out;
}

static const std::string statePath(const std::string &basePath, int stateNo) {
	return basePath + "_" + itos(stateNo) + ".gqs";
}

namespace Gambatte {
GB::GB() : z80(new CPU), stateNo(1) {}

GB::~GB() {
	delete z80;
}

unsigned GB::runFor(Gambatte::uint_least32_t *const soundBuf, const unsigned samples) {
	z80->setSoundBuffer(soundBuf);
	z80->runFor(samples * 2);
	
	return z80->fillSoundBuffer();
}

void GB::reset() {
	z80->saveSavedata();
	
	SaveState state;
	z80->setStatePtrs(state);
	setInitState(state, z80->isCgb());
	z80->loadState(state);
	z80->loadSavedata();
	
// 	z80->reset();
}

void GB::setVideoBlitter(VideoBlitter *vb) {
	z80->setVideoBlitter(vb);
}

void GB::videoBufferChange() {
	z80->videoBufferChange();
}

unsigned GB::videoWidth() const {
	return z80->videoWidth();
}

unsigned GB::videoHeight() const {
	return z80->videoHeight();
}

void GB::setVideoFilter(const unsigned n) {
	z80->setVideoFilter(n);
}

std::vector<const FilterInfo*> GB::filterInfo() const {
	return z80->filterInfo();
}

void GB::setInputStateGetter(InputStateGetter *getInput) {
	z80->setInputStateGetter(getInput);
}

void GB::set_savedir(const char *sdir) {
	z80->set_savedir(sdir);
}

bool GB::load(const char* romfile, const bool forceDmg) {
	z80->saveSavedata();
	
	const bool failed = z80->load(romfile, forceDmg);
	
	if (!failed) {
		SaveState state;
		z80->setStatePtrs(state);
		setInitState(state, z80->isCgb());
		z80->loadState(state);
		z80->loadSavedata();
		
		stateNo = 1;
		z80->setOsdElement(std::auto_ptr<OsdElement>());
	}
	
	return failed;
}

bool GB::isCgb() const {
	return z80->isCgb();
}

void GB::setDmgPaletteColor(unsigned palNum, unsigned colorNum, unsigned rgb32) {
	z80->setDmgPaletteColor(palNum, colorNum, rgb32);
}

void GB::loadState(const char *const filepath, const bool osdMessage) {
	z80->saveSavedata();
	
	SaveState state;
	z80->setStatePtrs(state);
	
	if (StateSaver::loadState(state, filepath)) {
		z80->loadState(state);
		
		if (osdMessage)
			z80->setOsdElement(newStateLoadedOsdElement(stateNo));
	}
}

void GB::saveState() {
	saveState(statePath(z80->saveBasePath(), stateNo).c_str());
	z80->setOsdElement(newStateSavedOsdElement(stateNo));
}

void GB::loadState() {
	loadState(statePath(z80->saveBasePath(), stateNo).c_str(), true);
}

void GB::saveState(const char *filepath) {
	SaveState state;
	z80->setStatePtrs(state);
	z80->saveState(state);
	StateSaver::saveState(state, filepath);
}

void GB::loadState(const char *const filepath) {
	loadState(filepath, false);
}

void GB::selectState(int n) {
	n -= (n / 10) * 10;
	stateNo = n < 0 ? n + 10 : n;
	z80->setOsdElement(newSaveStateOsdElement(statePath(z80->saveBasePath(), stateNo).c_str(), stateNo));
}
}
