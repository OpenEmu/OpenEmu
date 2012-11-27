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

#ifndef GOB_INTER_H
#define GOB_INTER_H

#include "common/func.h"
#include "common/hashmap.h"

#include "gob/goblin.h"
#include "gob/variables.h"
#include "gob/iniconfig.h"
#include "gob/databases.h"

namespace Common {
	class PEResources;
}

namespace Gob {

class Cheater_Geisha;

namespace Geisha {
	class Diving;
	class Penetration;
}

// This is to help devices with small memory (PDA, smartphones, ...)
// to save a bit of memory used by opcode names in the Gob engine.
#ifndef REDUCE_MEMORY_USAGE
	#define _OPCODEDRAW(ver, x)  setProc(new Common::Functor0Mem<void, ver>(this, &ver::x), #x)
	#define _OPCODEFUNC(ver, x)  setProc(new Common::Functor1Mem<OpFuncParams &, void, ver>(this, &ver::x), #x)
	#define _OPCODEGOB(ver, x)   setProc(new Common::Functor1Mem<OpGobParams &, void, ver>(this, &ver::x), #x)
#else
	#define _OPCODEDRAW(ver, x)  setProc(new Common::Functor0Mem<void, ver>(this, &ver::x), "")
	#define _OPCODEFUNC(ver, x)  setProc(new Common::Functor1Mem<OpFuncParams &, void, ver>(this, &ver::x), "")
	#define _OPCODEGOB(ver, x)   setProc(new Common::Functor1Mem<OpGobParams &, void, ver>(this, &ver::x), "")
#endif

#define CLEAROPCODEDRAW(i) _opcodesDraw[i].setProc(0, 0)
#define CLEAROPCODEFUNC(i) _opcodesFunc[i].setProc(0, 0)
#define CLEAROPCODEGOB(i)  _opcodesGob.erase(i)

typedef Common::Functor0<void> OpcodeDraw;
typedef Common::Functor1<struct OpFuncParams &, void> OpcodeFunc;
typedef Common::Functor1<struct OpGobParams &, void> OpcodeGob;

struct OpFuncParams {
	byte cmdCount;
	byte counter;
	int16 retFlag;
	bool doReturn;
};
struct OpGobParams {
	int16 extraData;
	int16 paramCount;
	VariableReference retVarPtr;
	Goblin::Gob_Object *objDesc;
};

template<typename T>
struct OpcodeEntry : Common::NonCopyable {
	T *proc;
	const char *desc;

	OpcodeEntry() : proc(0), desc(0) {}
	~OpcodeEntry() {
		setProc(0, 0);
	}

	void setProc(T *p, const char *d) {
		if (proc != p) {
			delete proc;
			proc = p;
		}
		desc = d;
	}
};

class Inter {
public:
	uint8 _terminate;

	int16 *_breakFromLevel;
	int16 *_nestLevel;

	uint32 _soundEndTimeKey;
	int16 _soundStopVal;

	Variables *_variables;

	void setupOpcodes();

	void initControlVars(char full);
	void renewTimeInVars();
	void storeMouse();
	void storeKey(int16 key);

	void writeVar(uint32 offset, uint16 type, uint32 value);

	void funcBlock(int16 retFlag);
	void callSub(int16 retFlag);

	void allocateVars(uint32 count);
	void delocateVars();

	virtual int16 loadSound(int16 slot) = 0;
	virtual void animPalette() = 0;

	Inter(GobEngine *vm);
	virtual ~Inter();

protected:
	OpcodeEntry<OpcodeDraw> _opcodesDraw[256];
	OpcodeEntry<OpcodeFunc> _opcodesFunc[256];
	Common::HashMap<int, OpcodeEntry<OpcodeGob> > _opcodesGob;

	bool _break;

	int16 _animPalLowIndex[8];
	int16 _animPalHighIndex[8];
	int16 _animPalDir[8];

	VariableStack _varStack;

	// Busy-wait detection
	bool   _noBusyWait;
	uint32 _lastBusyWait;

	GobEngine *_vm;

	void executeOpcodeDraw(byte i);
	void executeOpcodeFunc(byte i, byte j, OpFuncParams &params);
	void executeOpcodeGob(int i, OpGobParams &params);

	const char *getDescOpcodeDraw(byte i);
	const char *getDescOpcodeFunc(byte i, byte j);
	const char *getDescOpcodeGob(int i);

	virtual void setupOpcodesDraw() = 0;
	virtual void setupOpcodesFunc() = 0;
	virtual void setupOpcodesGob()  = 0;

	virtual void checkSwitchTable(uint32 &offset) = 0;

	void o_drawNOP() {}
	void o_funcNOP(OpFuncParams &params) {}
	void o_gobNOP(OpGobParams &params) {}

	void storeValue(uint16 index, uint16 type, uint32 value);
	void storeValue(uint32 value);

	void storeString(uint16 index, uint16 type, const char *value);
	void storeString(const char *value);

	uint32 readValue(uint16 index, uint16 type);

	void handleBusyWait();
};

class Inter_v1 : public Inter {
public:
	Inter_v1(GobEngine *vm);
	virtual ~Inter_v1() {}

	virtual int16 loadSound(int16 slot);
	virtual void animPalette();

protected:
	virtual void setupOpcodesDraw();
	virtual void setupOpcodesFunc();
	virtual void setupOpcodesGob();

	virtual void checkSwitchTable(uint32 &offset);

	void o1_loadMult();
	void o1_playMult();
	void o1_freeMultKeys();
	void o1_initCursor();
	void o1_initCursorAnim();
	void o1_clearCursorAnim();
	void o1_setRenderFlags();
	void o1_loadAnim();
	void o1_freeAnim();
	void o1_updateAnim();
	void o1_initMult();
	void o1_freeMult();
	void o1_animate();
	void o1_loadMultObject();
	void o1_getAnimLayerInfo();
	void o1_getObjAnimSize();
	void o1_loadStatic();
	void o1_freeStatic();
	void o1_renderStatic();
	void o1_loadCurLayer();
	void o1_playCDTrack();
	void o1_getCDTrackPos();
	void o1_stopCD();
	void o1_loadFontToSprite();
	void o1_freeFontToSprite();
	void o1_callSub(OpFuncParams &params);
	void o1_printTotText(OpFuncParams &params);
	void o1_loadCursor(OpFuncParams &params);
	void o1_switch (OpFuncParams &params);
	void o1_repeatUntil(OpFuncParams &params);
	void o1_whileDo(OpFuncParams &params);
	void o1_if(OpFuncParams &params);
	void o1_assign(OpFuncParams &params);
	void o1_loadSpriteToPos(OpFuncParams &params);
	void o1_printText(OpFuncParams &params);
	void o1_loadTot(OpFuncParams &params);
	void o1_palLoad(OpFuncParams &params);
	void o1_keyFunc(OpFuncParams &params);
	void o1_capturePush(OpFuncParams &params);
	void o1_capturePop(OpFuncParams &params);
	void o1_animPalInit(OpFuncParams &params);
	void o1_drawOperations(OpFuncParams &params);
	void o1_setcmdCount(OpFuncParams &params);
	void o1_return(OpFuncParams &params);
	void o1_renewTimeInVars(OpFuncParams &params);
	void o1_speakerOn(OpFuncParams &params);
	void o1_speakerOff(OpFuncParams &params);
	void o1_putPixel(OpFuncParams &params);
	void o1_goblinFunc(OpFuncParams &params);
	void o1_createSprite(OpFuncParams &params);
	void o1_freeSprite(OpFuncParams &params);
	void o1_returnTo(OpFuncParams &params);
	void o1_loadSpriteContent(OpFuncParams &params);
	void o1_copySprite(OpFuncParams &params);
	void o1_fillRect(OpFuncParams &params);
	void o1_drawLine(OpFuncParams &params);
	void o1_strToLong(OpFuncParams &params);
	void o1_invalidate(OpFuncParams &params);
	void o1_setBackDelta(OpFuncParams &params);
	void o1_playSound(OpFuncParams &params);
	void o1_stopSound(OpFuncParams &params);
	void o1_loadSound(OpFuncParams &params);
	void o1_freeSoundSlot(OpFuncParams &params);
	void o1_waitEndPlay(OpFuncParams &params);
	void o1_playComposition(OpFuncParams &params);
	void o1_getFreeMem(OpFuncParams &params);
	void o1_checkData(OpFuncParams &params);
	void o1_cleanupStr(OpFuncParams &params);
	void o1_insertStr(OpFuncParams &params);
	void o1_cutStr(OpFuncParams &params);
	void o1_strstr(OpFuncParams &params);
	void o1_istrlen(OpFuncParams &params);
	void o1_setMousePos(OpFuncParams &params);
	void o1_setFrameRate(OpFuncParams &params);
	void o1_animatePalette(OpFuncParams &params);
	void o1_animateCursor(OpFuncParams &params);
	void o1_blitCursor(OpFuncParams &params);
	void o1_loadFont(OpFuncParams &params);
	void o1_freeFont(OpFuncParams &params);
	void o1_readData(OpFuncParams &params);
	void o1_writeData(OpFuncParams &params);
	void o1_manageDataFile(OpFuncParams &params);
	void o1_setState(OpGobParams &params);
	void o1_setCurFrame(OpGobParams &params);
	void o1_setNextState(OpGobParams &params);
	void o1_setMultState(OpGobParams &params);
	void o1_setOrder(OpGobParams &params);
	void o1_setActionStartState(OpGobParams &params);
	void o1_setCurLookDir(OpGobParams &params);
	void o1_setType(OpGobParams &params);
	void o1_setNoTick(OpGobParams &params);
	void o1_setPickable(OpGobParams &params);
	void o1_setXPos(OpGobParams &params);
	void o1_setYPos(OpGobParams &params);
	void o1_setDoAnim(OpGobParams &params);
	void o1_setRelaxTime(OpGobParams &params);
	void o1_setMaxTick(OpGobParams &params);
	void o1_getState(OpGobParams &params);
	void o1_getCurFrame(OpGobParams &params);
	void o1_getNextState(OpGobParams &params);
	void o1_getMultState(OpGobParams &params);
	void o1_getOrder(OpGobParams &params);
	void o1_getActionStartState(OpGobParams &params);
	void o1_getCurLookDir(OpGobParams &params);
	void o1_getType(OpGobParams &params);
	void o1_getNoTick(OpGobParams &params);
	void o1_getPickable(OpGobParams &params);
	void o1_getObjMaxFrame(OpGobParams &params);
	void o1_getXPos(OpGobParams &params);
	void o1_getYPos(OpGobParams &params);
	void o1_getDoAnim(OpGobParams &params);
	void o1_getRelaxTime(OpGobParams &params);
	void o1_getMaxTick(OpGobParams &params);
	void o1_manipulateMap(OpGobParams &params);
	void o1_getItem(OpGobParams &params);
	void o1_manipulateMapIndirect(OpGobParams &params);
	void o1_getItemIndirect(OpGobParams &params);
	void o1_setPassMap(OpGobParams &params);
	void o1_setGoblinPosH(OpGobParams &params);
	void o1_getGoblinPosXH(OpGobParams &params);
	void o1_getGoblinPosYH(OpGobParams &params);
	void o1_setGoblinMultState(OpGobParams &params);
	void o1_setGoblinUnk14(OpGobParams &params);
	void o1_setItemIdInPocket(OpGobParams &params);
	void o1_setItemIndInPocket(OpGobParams &params);
	void o1_getItemIdInPocket(OpGobParams &params);
	void o1_getItemIndInPocket(OpGobParams &params);
	void o1_setGoblinPos(OpGobParams &params);
	void o1_setGoblinState(OpGobParams &params);
	void o1_setGoblinStateRedraw(OpGobParams &params);
	void o1_decRelaxTime(OpGobParams &params);
	void o1_getGoblinPosX(OpGobParams &params);
	void o1_getGoblinPosY(OpGobParams &params);
	void o1_clearPathExistence(OpGobParams &params);
	void o1_setGoblinVisible(OpGobParams &params);
	void o1_setGoblinInvisible(OpGobParams &params);
	void o1_getObjectIntersect(OpGobParams &params);
	void o1_getGoblinIntersect(OpGobParams &params);
	void o1_setItemPos(OpGobParams &params);
	void o1_loadObjects(OpGobParams &params);
	void o1_freeObjects(OpGobParams &params);
	void o1_animateObjects(OpGobParams &params);
	void o1_drawObjects(OpGobParams &params);
	void o1_loadMap(OpGobParams &params);
	void o1_moveGoblin(OpGobParams &params);
	void o1_switchGoblin(OpGobParams &params);
	void o1_loadGoblin(OpGobParams &params);
	void o1_writeTreatItem(OpGobParams &params);
	void o1_moveGoblin0(OpGobParams &params);
	void o1_setGoblinTarget(OpGobParams &params);
	void o1_setGoblinObjectsPos(OpGobParams &params);
	void o1_initGoblin(OpGobParams &params);

	void manipulateMap(int16 xPos, int16 yPos, int16 item);
};

class Inter_Geisha : public Inter_v1 {
public:
	Inter_Geisha(GobEngine *vm);
	virtual ~Inter_Geisha();

protected:
	virtual void setupOpcodesDraw();
	virtual void setupOpcodesFunc();
	virtual void setupOpcodesGob();

	void oGeisha_loadCursor(OpFuncParams &params);
	void oGeisha_loadTot(OpFuncParams &params);
	void oGeisha_goblinFunc(OpFuncParams &params);
	void oGeisha_loadSound(OpFuncParams &params);
	void oGeisha_checkData(OpFuncParams &params);
	void oGeisha_readData(OpFuncParams &params);
	void oGeisha_writeData(OpFuncParams &params);

	void oGeisha_gamePenetration(OpGobParams &params);
	void oGeisha_gameDiving(OpGobParams &params);
	void oGeisha_loadTitleMusic(OpGobParams &params);
	void oGeisha_playMusic(OpGobParams &params);
	void oGeisha_stopMusic(OpGobParams &params);

	void oGeisha_caress1(OpGobParams &params);
	void oGeisha_caress2(OpGobParams &params);

	int16 loadSound(int16 slot);

private:
	Geisha::Diving      *_diving;
	Geisha::Penetration *_penetration;

	Cheater_Geisha *_cheater;
};

class Inter_v2 : public Inter_v1 {
public:
	Inter_v2(GobEngine *vm);
	virtual ~Inter_v2() {}

	virtual int16 loadSound(int16 search);
	virtual void animPalette();

protected:
	virtual void setupOpcodesDraw();
	virtual void setupOpcodesFunc();
	virtual void setupOpcodesGob();

	virtual void checkSwitchTable(uint32 &offset);

	void o2_playMult();
	void o2_freeMultKeys();
	void o2_setRenderFlags();
	void o2_multSub();
	void o2_initMult();
	void o2_loadMultObject();
	void o2_renderStatic();
	void o2_loadCurLayer();
	void o2_playCDTrack();
	void o2_waitCDTrackEnd();
	void o2_stopCD();
	void o2_readLIC();
	void o2_freeLIC();
	void o2_getCDTrackPos();
	void o2_loadFontToSprite();
	void o2_totSub();
	void o2_switchTotSub();
	void o2_pushVars();
	void o2_popVars();
	void o2_loadMapObjects();
	void o2_freeGoblins();
	void o2_moveGoblin();
	void o2_writeGoblinPos();
	void o2_stopGoblin();
	void o2_setGoblinState();
	void o2_placeGoblin();
	void o2_initScreen();
	void o2_scroll();
	void o2_setScrollOffset();
	void o2_playImd();
	void o2_getImdInfo();
	void o2_openItk();
	void o2_closeItk();
	void o2_setImdFrontSurf();
	void o2_resetImdFrontSurf();
	void o2_assign(OpFuncParams &params);
	void o2_printText(OpFuncParams &params);
	void o2_animPalInit(OpFuncParams &params);
	void o2_addHotspot(OpFuncParams &params);
	void o2_removeHotspot(OpFuncParams &params);
	void o2_goblinFunc(OpFuncParams &params);
	void o2_stopSound(OpFuncParams &params);
	void o2_loadSound(OpFuncParams &params);
	void o2_getFreeMem(OpFuncParams &params);
	void o2_checkData(OpFuncParams &params);
	void o2_readData(OpFuncParams &params);
	void o2_writeData(OpFuncParams &params);
	void o2_loadInfogramesIns(OpGobParams &params);
	void o2_playInfogrames(OpGobParams &params);
	void o2_startInfogrames(OpGobParams &params);
	void o2_stopInfogrames(OpGobParams &params);
	void o2_handleGoblins(OpGobParams &params);
	void o2_playProtracker(OpGobParams &params);
	void o2_stopProtracker(OpGobParams &params);
};

class Inter_Bargon : public Inter_v2 {
public:
	Inter_Bargon(GobEngine *vm);
	virtual ~Inter_Bargon() {}

protected:
	virtual void setupOpcodesDraw();
	virtual void setupOpcodesFunc();
	virtual void setupOpcodesGob();

	void oBargon_intro0(OpGobParams &params);
	void oBargon_intro1(OpGobParams &params);
	void oBargon_intro2(OpGobParams &params);
	void oBargon_intro3(OpGobParams &params);
	void oBargon_intro4(OpGobParams &params);
	void oBargon_intro5(OpGobParams &params);
	void oBargon_intro6(OpGobParams &params);
	void oBargon_intro7(OpGobParams &params);
	void oBargon_intro8(OpGobParams &params);
	void oBargon_intro9(OpGobParams &params);
};

class Inter_Fascination : public Inter_v2 {
public:
	Inter_Fascination(GobEngine *vm);
	virtual ~Inter_Fascination() {}

protected:
	virtual void setupOpcodesDraw();
	virtual void setupOpcodesFunc();
	virtual void setupOpcodesGob();

	void oFascin_playProtracker(OpGobParams &params);

	void oFascin_repeatUntil(OpFuncParams &params);
	void oFascin_assign(OpFuncParams &params);
	void oFascin_copySprite(OpFuncParams &params);
	void oFascin_keyFunc(OpFuncParams &params);

	void oFascin_playTirb(OpGobParams &params);
	void oFascin_playTira(OpGobParams &params);
	void oFascin_loadExtasy(OpGobParams &params);
	void oFascin_adlibPlay(OpGobParams &params);
	void oFascin_adlibStop(OpGobParams &params);
	void oFascin_adlibUnload(OpGobParams &params);
	void oFascin_loadMus1(OpGobParams &params);
	void oFascin_loadMus2(OpGobParams &params);
	void oFascin_loadMus3(OpGobParams &params);
	void oFascin_loadBatt1(OpGobParams &params);
	void oFascin_loadBatt2(OpGobParams &params);
	void oFascin_loadBatt3(OpGobParams &params);
	void oFascin_loadMod(OpGobParams &params);
	void oFascin_setWinSize();
	void oFascin_closeWin();
	void oFascin_activeWin();
	void oFascin_openWin();
	void oFascin_initCursorAnim();
	void oFascin_setRenderFlags();
	void oFascin_setWinFlags();
};

class Inter_LittleRed : public Inter_v2 {
public:
	Inter_LittleRed(GobEngine *vm);
	virtual ~Inter_LittleRed() {}

protected:
	virtual void setupOpcodesDraw();
	virtual void setupOpcodesFunc();
	virtual void setupOpcodesGob();

	void oLittleRed_keyFunc(OpFuncParams &params);
	void oLittleRed_playComposition(OpFuncParams &params);
};

class Inter_v3 : public Inter_v2 {
public:
	Inter_v3(GobEngine *vm);
	virtual ~Inter_v3() {}

protected:
	virtual void setupOpcodesDraw();
	virtual void setupOpcodesFunc();
	virtual void setupOpcodesGob();

	void o3_getTotTextItemPart(OpFuncParams &params);
	void o3_speakerOn(OpFuncParams &params);
	void o3_speakerOff(OpFuncParams &params);
	void o3_copySprite(OpFuncParams &params);

	void o3_wobble(OpGobParams &params);

private:
	bool _ignoreSpeakerOff;
};

class Inter_Inca2 : public Inter_v3 {
public:
	Inter_Inca2(GobEngine *vm);
	virtual ~Inter_Inca2() {}

protected:
	virtual void setupOpcodesDraw();
	virtual void setupOpcodesFunc();
	virtual void setupOpcodesGob();

	void oInca2_spaceShooter(OpFuncParams &params);
};

class Inter_v4 : public Inter_v3 {
public:
	Inter_v4(GobEngine *vm);
	virtual ~Inter_v4() {}

protected:
	virtual void setupOpcodesDraw();
	virtual void setupOpcodesFunc();
	virtual void setupOpcodesGob();

	void o4_initScreen();
	void o4_playVmdOrMusic();
};

class Inter_v5 : public Inter_v4 {
public:
	Inter_v5(GobEngine *vm);
	virtual ~Inter_v5() {}

protected:
	byte _gob_97_98_val;

	virtual void setupOpcodesDraw();
	virtual void setupOpcodesFunc();
	virtual void setupOpcodesGob();

	void o5_deleteFile();
	void o5_initScreen();

	void o5_istrlen(OpFuncParams &params);

	void o5_spaceShooter(OpGobParams &params);
	void o5_getSystemCDSpeed(OpGobParams &params);
	void o5_getSystemRAM(OpGobParams &params);
	void o5_getSystemCPUSpeed(OpGobParams &params);
	void o5_getSystemDrawSpeed(OpGobParams &params);
	void o5_totalSystemSpecs(OpGobParams &params);
	void o5_saveSystemSpecs(OpGobParams &params);
	void o5_loadSystemSpecs(OpGobParams &params);

	void o5_gob92(OpGobParams &params);
	void o5_gob95(OpGobParams &params);
	void o5_gob96(OpGobParams &params);
	void o5_gob97(OpGobParams &params);
	void o5_gob98(OpGobParams &params);
	void o5_gob100(OpGobParams &params);
	void o5_gob200(OpGobParams &params);
};

class Inter_v6 : public Inter_v5 {
public:
	Inter_v6(GobEngine *vm);
	virtual ~Inter_v6() {}

protected:
	bool _gotFirstPalette;

	virtual void setupOpcodesDraw();
	virtual void setupOpcodesFunc();
	virtual void setupOpcodesGob();

	void o6_totSub();
	void o6_playVmdOrMusic();

	void o6_loadCursor(OpFuncParams &params);
	void o6_assign(OpFuncParams &params);
	void o6_removeHotspot(OpFuncParams &params);
	void o6_fillRect(OpFuncParams &params);

	void probe16bitMusic(Common::String &fileName);
};

class Inter_Playtoons : public Inter_v6 {
public:
	Inter_Playtoons(GobEngine *vm);
	virtual ~Inter_Playtoons() {}

protected:
	virtual void setupOpcodesDraw();
	virtual void setupOpcodesFunc();
	virtual void setupOpcodesGob();

	void oPlaytoons_printText(OpFuncParams &params);
	void oPlaytoons_F_1B(OpFuncParams &params);
	void oPlaytoons_putPixel(OpFuncParams &params);
	void oPlaytoons_freeSprite(OpFuncParams &params);
	void oPlaytoons_checkData(OpFuncParams &params);
	void oPlaytoons_readData(OpFuncParams &params);

	void oPlaytoons_loadMultObject();
	void oPlaytoons_getObjAnimSize();
	void oPlaytoons_CD_20_23();
	void oPlaytoons_CD_25();
	void oPlaytoons_copyFile();
	void oPlaytoons_openItk();

	Common::String getFile(const char *path);

private:
	bool readSprite(Common::String file, int32 dataVar, int32 size, int32 offset);
};

class Inter_v7 : public Inter_Playtoons {
public:
	Inter_v7(GobEngine *vm);
	virtual ~Inter_v7();

protected:
	virtual void setupOpcodesDraw();
	virtual void setupOpcodesFunc();
	virtual void setupOpcodesGob();

	void o7_draw0x0C();
	void o7_loadCursor();
	void o7_displayWarning();
	void o7_logString();
	void o7_intToString();
	void o7_callFunction();
	void o7_loadFunctions();
	void o7_playVmdOrMusic();
	void o7_draw0x89();
	void o7_findFile();
	void o7_findCDFile();
	void o7_getSystemProperty();
	void o7_loadImage();
	void o7_setVolume();
	void o7_zeroVar();
	void o7_getINIValue();
	void o7_setINIValue();
	void o7_loadLBMPalette();
	void o7_opendBase();
	void o7_closedBase();
	void o7_getDBString();

	void o7_oemToANSI(OpGobParams &params);
	void o7_gob0x201(OpGobParams &params);

private:
	INIConfig _inis;
	Databases _databases;

	Common::PEResources *_cursors;

	Common::String findFile(const Common::String &mask);

	bool loadCursorFile();
	void resizeCursors(int16 width, int16 height, int16 count, bool transparency);
};

} // End of namespace Gob

#endif // GOB_INTER_H
