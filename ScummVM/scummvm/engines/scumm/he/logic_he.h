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

#if !defined(SCUMM_HE_LOGIC_HE_H) && defined(ENABLE_HE)
#define SCUMM_HE_LOGIC_HE_H

namespace Scumm {

class ScummEngine_v90he;

class LogicHE {
public:
	static LogicHE *makeLogicHE(ScummEngine_v90he *vm);

	virtual ~LogicHE();

	virtual void beforeBootScript() {}
	virtual void initOnce() {}
	virtual int startOfFrame() { return 1; }
	void endOfFrame() {}
	void processKeyStroke(int keyPressed) {}

	virtual int versionID();
	virtual int32 dispatch(int op, int numArgs, int32 *args);

protected:
	// Only to be used from makeLogicHE()
	LogicHE(ScummEngine_v90he *vm);

	ScummEngine_v90he *_vm;

	void writeScummVar(int var, int32 value);
	int getFromArray(int arg0, int idx2, int idx1);
	void putInArray(int arg0, int idx2, int idx1, int val);
	int32 scummRound(double arg) { return (int32)(arg + 0.5); }

	#define RAD2DEG (180 / M_PI)
	#define DEG2RAD (M_PI / 180)
};

// Logic declarations
LogicHE *makeLogicHErace(ScummEngine_v90he *vm);
LogicHE *makeLogicHEfunshop(ScummEngine_v90he *vm);
LogicHE *makeLogicHEfootball(ScummEngine_v90he *vm);
LogicHE *makeLogicHEfootball2002(ScummEngine_v90he *vm);
LogicHE *makeLogicHEsoccer(ScummEngine_v90he *vm);
LogicHE *makeLogicHEbaseball2001(ScummEngine_v90he *vm);
LogicHE *makeLogicHEbasketball(ScummEngine_v90he *vm);
LogicHE *makeLogicHEmoonbase(ScummEngine_v90he *vm);

} // End of namespace Scumm

#endif
