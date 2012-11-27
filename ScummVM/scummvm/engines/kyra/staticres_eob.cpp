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
 *
 */

#include "kyra/eob.h"
#include "kyra/resource.h"


namespace Kyra {

#ifdef ENABLE_EOB
const DarkMoonAnimCommand *StaticResource::loadEoB2SeqData(int id, int &entries) {
	return (const DarkMoonAnimCommand *)getData(id, kEoB2SequenceData, entries);
}

const DarkMoonShapeDef *StaticResource::loadEoB2ShapeData(int id, int &entries) {
	return (const DarkMoonShapeDef *)getData(id, kEoB2ShapeData, entries);
}

const EoBCharacter *StaticResource::loadEoBNpcData(int id, int &entries) {
	return (const EoBCharacter *)getData(id, kEoBNpcData, entries);
}

bool StaticResource::loadEoB2SeqData(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	size = stream.size() / 11;

	DarkMoonAnimCommand *s = new DarkMoonAnimCommand[size];

	for (int i = 0; i < size; i++) {
		s[i].command = stream.readByte();
		s[i].obj = stream.readByte();
		s[i].x1 = stream.readSint16BE();
		s[i].y1 = stream.readByte();
		s[i].delay = stream.readByte();
		s[i].pal = stream.readByte();
		s[i].x2 = stream.readByte();
		s[i].y2 = stream.readByte();
		s[i].w = stream.readByte();
		s[i].h = stream.readByte();
	}

	ptr = s;
	return true;
}

bool StaticResource::loadEoB2ShapeData(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	size = stream.size() / 6;

	DarkMoonShapeDef *s = new DarkMoonShapeDef[size];

	for (int i = 0; i < size; i++) {
		s[i].index = stream.readSint16BE();
		s[i].x = stream.readByte();
		s[i].y = stream.readByte();
		s[i].w = stream.readByte();
		s[i].h = stream.readByte();
	}

	ptr = s;
	return true;
}

bool StaticResource::loadEoBNpcData(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	size = stream.readUint16BE();

	EoBCharacter *e = new EoBCharacter[size];
	memset(e, 0, size * sizeof(EoBCharacter));
	EoBCharacter *s = e;

	for (int i = 0; i < size; i++, s++) {
		s->id = stream.readByte();
		s->flags = stream.readByte();
		stream.read(s->name, 11);
		s->strengthCur = stream.readSByte();
		s->strengthMax = stream.readSByte();
		s->strengthExtCur = stream.readSByte();
		s->strengthExtMax = stream.readSByte();
		s->intelligenceCur = stream.readSByte();
		s->intelligenceMax = stream.readSByte();
		s->wisdomCur = stream.readSByte();
		s->wisdomMax = stream.readSByte();
		s->dexterityCur = stream.readSByte();
		s->dexterityMax = stream.readSByte();
		s->constitutionCur = stream.readSByte();
		s->constitutionMax = stream.readSByte();
		s->charismaCur = stream.readSByte();
		s->charismaMax = stream.readSByte();
		s->hitPointsCur = stream.readSint16BE();
		s->hitPointsMax = stream.readSint16BE();
		s->armorClass = stream.readSByte();
		s->disabledSlots = stream.readByte();
		s->raceSex = stream.readByte();
		s->cClass = stream.readByte();
		s->alignment = stream.readByte();
		s->portrait = stream.readSByte();
		s->food = stream.readByte();
		stream.read(s->level, 3);
		s->experience[0] = stream.readUint32BE();
		s->experience[1] = stream.readUint32BE();
		s->experience[2] = stream.readUint32BE();
		s->mageSpellsAvailableFlags = stream.readUint32BE();
		for (int ii = 0; ii < 27; ii++)
			s->inventory[ii] = stream.readSint16BE();
	}

	ptr = e;
	return true;
}

void StaticResource::freeEoB2SeqData(void *&ptr, int &size) {
	DarkMoonAnimCommand *d = (DarkMoonAnimCommand *)ptr;
	delete[] d;
	ptr = 0;
	size = 0;
}

void StaticResource::freeEoB2ShapeData(void *&ptr, int &size) {
	DarkMoonShapeDef *d = (DarkMoonShapeDef *)ptr;
	delete[] d;
	ptr = 0;
	size = 0;
}

void StaticResource::freeEoBNpcData(void *&ptr, int &size) {
	EoBCharacter *d = (EoBCharacter *)ptr;
	delete[] d;
	ptr = 0;
	size = 0;
}

const ScreenDim Screen_EoB::_screenDimTable[] = {
	{ 0x00, 0x00, 0x28, 0xC8, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x08, 0x48, 0x18, 0x38, 0x0E, 0x0C, 0x00, 0x00 },
	{ 0x13, 0x40, 0x14, 0x80, 0x06, 0x0C, 0x00, 0x00 },
	{ 0x1D, 0x78, 0x08, 0x40, 0x0F, 0x0D, 0x00, 0x00 },
	{ 0x02, 0x18, 0x14, 0x78, 0x0F, 0x02, 0x03, 0x00 },
	{ 0x00, 0x00, 0x16, 0x78, 0x0F, 0x0D, 0x00, 0x00 },
	{ 0x0A, 0x6C, 0x15, 0x28, 0x0F, 0x00, 0x00, 0x00 },
	{ 0x01, 0xB4, 0x22, 0x12, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x02, 0x18, 0x14, 0x00, 0x0F, 0x02, 0x03, 0x00 },
	{ 0x01, 0x7D, 0x26, 0x40, 0x0F, 0x00, 0x03, 0x00 },
	{ 0x00, 0x00, 0x16, 0x90, 0x0F, 0x02, 0x00, 0x00 },
	{ 0x01, 0x14, 0x14, 0x38, 0x0F, 0x02, 0x00, 0x00 },
	{ 0x01, 0x04, 0x14, 0x9C, 0x0F, 0x02, 0x00, 0x00 },
	{ 0x01, 0x19, 0x26, 0x64, 0x0F, 0x02, 0x00, 0x00 },
	{ 0x01, 0x14, 0x14, 0x58, 0x0F, 0x02, 0x00, 0x00 },
	{ 0x02, 0x06, 0x23, 0x78, 0x0F, 0x02, 0x00, 0x00 },
	{ 0x09, 0x14, 0x16, 0x38, 0x0F, 0x02, 0x00, 0x00 },
	{ 0x01, 0x96, 0x26, 0x31, 0x0F, 0x00, 0x00, 0x00 },
	{ 0x01, 0x08, 0x26, 0x80, 0x0C, 0x0F, 0x00, 0x00 },
	{ 0x01, 0x10, 0x26, 0x14, 0x00, 0x0F, 0x06, 0x00 },
	{ 0x00, 0x10, 0x10, 0x0C, 0x00, 0x0F, 0x06, 0x00 },
	{ 0x00, 0x10, 0x17, 0x00, 0x00, 0x0F, 0x06, 0x00 },
	{ 0x00, 0x10, 0x10, 0x00, 0x00, 0x0F, 0x06, 0x00 },
	{ 0x00, 0x10, 0x07, 0x04, 0x00, 0x0F, 0x06, 0x00 },
	{ 0x00, 0x00, 0x11, 0x05, 0x00, 0x0F, 0x06, 0x00 },
	{ 0x00, 0x00, 0x15, 0x05, 0x00, 0x0F, 0x06, 0x00 },
	{ 0x00, 0x00, 0x11, 0x08, 0x00, 0x0F, 0x06, 0x00 },
	{ 0x00, 0x00, 0x15, 0x03, 0x00, 0x0F, 0x06, 0x00 },
	{ 0x0A, 0xA8, 0x15, 0x18, 0x0F, 0x0C, 0x00, 0x00 }
};

const int Screen_EoB::_screenDimTableCount = ARRAYSIZE(Screen_EoB::_screenDimTable);

const uint8 EoBCoreEngine::_hpIncrPerLevel[] = { 10, 4, 8, 6, 10, 10, 9, 10, 9, 10, 9, 9, 3, 1, 2, 2, 3, 3 };

const uint8 EoBCoreEngine::_numLevelsPerClass[] = { 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 2, 2, 3, 2, 2 };

const int8 EoBCoreEngine::_characterClassType[] = {
	0, -1, -1, 5, -1, -1, 4, -1, -1, 1, -1, -1, 2, -1, -1, 3, -1, -1,  0,
	2, -1, 0, 3, -1, 0, 1, -1, 0, 1, 3, 3, 1, -1, 2, 3, -1, 0, 2,  1,  5,
	2, -1, 2, 1, -1
};

const int16 EoBCoreEngine::_hpConstModifiers[] = { -1, -3, -2, -2, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 6, 6, 7, 7 };

const uint8 EoBCoreEngine::_charClassModifier[] = {
	0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x03, 0x02,
	0x00, 0x00, 0x02
};

const uint8 EoBCoreEngine::_itemsOverlayCGA[] = {
	0x00, 0x55, 0x55, 0xFF
};

const uint8 EoBCoreEngine::_teleporterShapeDefs[] = {
	0x0C, 0x58, 0x02, 0x0E,
	0x0C, 0x67, 0x01, 0x07,
	0x0C, 0x6F, 0x01, 0x07,
	0x0C, 0x77, 0x01, 0x05,
	0x0C, 0x7D, 0x01, 0x05,
	0x0C, 0x83, 0x01, 0x03
};

const uint8 EoBCoreEngine::_wallOfForceShapeDefs[] = {
	0x00, 0x00, 0x04, 0x08,
	0x00, 0x08, 0x04, 0x08,
	0x04, 0x00, 0x04, 0x08,
	0x04, 0x08, 0x04, 0x08,
	0x08, 0x00, 0x05, 0x10,
	0x0C, 0x00, 0x05, 0x10
};

const int16 EoBCoreEngine::_buttonList1[] = {
	58, 0, 1, 2, 3, 90, 91, 4, 5, 6, 7, 8, 9, 10, 11, 12, 78, 79, 13, 14,  15,  16,
	80, 81, 17, 18, 19, 20, 82, 83, 49, 50, 51, 52, 53, 54, 56, 57, -1
};

const int16 EoBCoreEngine::_buttonList2[] = {
	58, 61, 62, 63, 64, 65, 93, 94, 66, 67, 68, 69, 70, 71, 76, 77, 88, 0, 1, 2, 3,
	90, 91,  4,  5, 6, 7, 8, 9, 10, 11, 12, 78, 79, 13, 14, 15, 16, 80, 81, 17, 18,
	19, 20, 82, 83, 49, 50, 51, 52, 53, 54, 56, 57, -1
};

const int16 EoBCoreEngine::_buttonList3[] = {
	58, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 84, 85, 46, 47, 48, 60, 59, 92, 4, 5, 6, 7, 8, 49,  50,
	51, 52, 53, 54, 56, 57, -1
};

const int16 EoBCoreEngine::_buttonList4[] = {
	58, 47, 48, 60, 59, 92, 4, 5, 6, 7, 8, 49, 50, 51, 52, 53, 54, 56, 57, -1
};

const int16 EoBCoreEngine::_buttonList5[] = {
	58, 61, 62, 63, 64, 65, 93, 66, 67, 68, 69, 70, 71, 88, 21, 22, 23, 24, 25, 26,
	27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 84,
	85, 46, 47, 48, 60, 59, 92, 4, 5, 6, 7, 8, 49, 50, 51, 52, 53, 54, 56, 57, -1
};

const int16 EoBCoreEngine::_buttonList6[] = {
	58, 61, 62, 63, 64, 65, 93, 66, 67, 68, 69, 70, 71, 88, 46, 47, 48, 60, 59, 92,
	4, 5, 6, 7, 8, 49, 50, 51, 52, 53, 54, 56, 57, -1
};

const int16 EoBCoreEngine::_buttonList7[] = {
	17, 18, 19, 20, 82, 83, 55, -1
};

const int16 EoBCoreEngine::_buttonList8[] = {
	72, 73, 74, 75, 86, 87, 89, -1
};

const uint8 EoBCoreEngine::_clock2Timers[] = {
	0x00, 0x01, 0x20, 0x21, 0x22, 0x22,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
	0x04, 0x05, 0x06, 0x07
};

const uint8 EoBCoreEngine::_numClock2Timers = ARRAYSIZE(EoBCoreEngine::_clock2Timers);

void EoBCoreEngine::initStaticResource() {
	int temp;
	_chargenStatStrings = _staticres->loadStrings(kEoBBaseChargenStatStrings, temp);
	_chargenRaceSexStrings = _staticres->loadStrings(kEoBBaseChargenRaceSexStrings, temp);
	_chargenClassStrings = _staticres->loadStrings(kEoBBaseChargenClassStrings, temp);
	_chargenAlignmentStrings = _staticres->loadStrings(kEoBBaseChargenAlignmentStrings, temp);

	_pryDoorStrings = _staticres->loadStrings(kEoBBasePryDoorStrings, temp);
	_warningStrings = _staticres->loadStrings(kEoBBaseWarningStrings, temp);

	_suffixStringsRings = _staticres->loadStrings(kEoBBaseItemSuffixStringsRings, temp);
	_suffixStringsPotions = _staticres->loadStrings(kEoBBaseItemSuffixStringsPotions, temp);
	_suffixStringsWands = _staticres->loadStrings(kEoBBaseItemSuffixStringsWands, temp);

	_ripItemStrings = _staticres->loadStrings(kEoBBaseRipItemStrings, temp);
	_cursedString = _staticres->loadStrings(kEoBBaseCursedString, temp);
	_enchantedString = _staticres->loadStrings(kEoBBaseEnchantedString, temp);
	_magicObjectStrings = _staticres->loadStrings(kEoBBaseMagicObjectStrings, temp);
	_magicObjectString5 = _staticres->loadStrings(kEoBBaseMagicObjectString5, temp);
	_patternSuffix = _staticres->loadStrings(kEoBBasePatternSuffix, temp);
	_patternGrFix1 = _staticres->loadStrings(kEoBBasePatternGrFix1, temp);
	_patternGrFix2 = _staticres->loadStrings(kEoBBasePatternGrFix2, temp);
	_validateArmorString = _staticres->loadStrings(kEoBBaseValidateArmorString, temp);
	_validateCursedString = _staticres->loadStrings(kEoBBaseValidateCursedString, temp);
	_validateNoDropString = _staticres->loadStrings(kEoBBaseValidateNoDropString, temp);
	_potionStrings = _staticres->loadStrings(kEoBBasePotionStrings, temp);
	_wandStrings = _staticres->loadStrings(kEoBBaseWandStrings, temp);
	_itemMisuseStrings = _staticres->loadStrings(kEoBBaseItemMisuseStrings, temp);

	_takenStrings = _staticres->loadStrings(kEoBBaseTakenStrings, temp);
	_potionEffectStrings = _staticres->loadStrings(kEoBBasePotionEffectStrings, temp);

	_yesNoStrings = _staticres->loadStrings(kEoBBaseYesNoStrings, temp);
	_npcMaxStrings = _staticres->loadStrings(kEoBBaseNpcMaxStrings, temp);
	_okStrings = _staticres->loadStrings(_flags.gameID == GI_EOB2 ? kEoBBaseOkStrings : kRpgCommonMoreStrings, temp);
	_npcJoinStrings = _staticres->loadStrings(kEoBBaseNpcJoinStrings, temp);
	_cancelStrings = _staticres->loadStrings(kEoBBaseCancelStrings, temp);
	_abortStrings = _staticres->loadStrings(_flags.gameID == GI_EOB2 ? kEoBBaseAbortStrings : kEoBBaseCancelStrings, temp);

	_menuStringsMain = _staticres->loadStrings(kEoBBaseMenuStringsMain, temp);
	_menuStringsSaveLoad = _staticres->loadStrings(kEoBBaseMenuStringsSaveLoad, temp);
	_menuStringsOnOff = _staticres->loadStrings(kEoBBaseMenuStringsOnOff, temp);
	_menuStringsSpells = _staticres->loadStrings(kEoBBaseMenuStringsSpells, temp);
	_menuStringsRest = _staticres->loadStrings(kEoBBaseMenuStringsRest, temp);
	_menuStringsDrop = _staticres->loadStrings(kEoBBaseMenuStringsDrop, temp);
	_menuStringsExit = _staticres->loadStrings(kEoBBaseMenuStringsExit, temp);
	_menuStringsStarve = _staticres->loadStrings(kEoBBaseMenuStringsStarve, temp);
	_menuStringsScribe = _staticres->loadStrings(kEoBBaseMenuStringsScribe, temp);
	_menuStringsDrop2 = _staticres->loadStrings(kEoBBaseMenuStringsDrop2, temp);
	_menuStringsHead = _staticres->loadStrings(kEoBBaseMenuStringsHead, temp);
	_menuStringsPoison = _staticres->loadStrings(kEoBBaseMenuStringsPoison, temp);
	_menuStringsMgc = _staticres->loadStrings(kEoBBaseMenuStringsMgc, temp);
	_menuStringsPrefs = _staticres->loadStrings(kEoBBaseMenuStringsPrefs, temp);
	_menuStringsRest2 = _staticres->loadStrings(kEoBBaseMenuStringsRest2, temp);
	_menuStringsRest3 = _staticres->loadStrings(kEoBBaseMenuStringsRest3, temp);
	_menuStringsRest4 = _staticres->loadStrings(kEoBBaseMenuStringsRest4, temp);
	_menuStringsDefeat = _staticres->loadStrings(kEoBBaseMenuStringsDefeat, temp);
	_menuStringsTransfer = _staticres->loadStrings(kEoBBaseMenuStringsTransfer, temp);
	_menuStringsSpec = _staticres->loadStrings(kEoBBaseMenuStringsSpec, temp);
	_menuStringsSpellNo = _staticres->loadStrings(kEoBBaseMenuStringsSpellNo, temp);
	_menuYesNoStrings = _staticres->loadStrings(kEoBBaseMenuYesNoStrings, temp);

	_spellLevelsMage = _staticres->loadRawData(kEoBBaseSpellLevelsMage, _spellLevelsMageSize);
	_spellLevelsCleric = _staticres->loadRawData(kEoBBaseSpellLevelsCleric, _spellLevelsClericSize);
	_numSpellsCleric = _staticres->loadRawData(kEoBBaseNumSpellsCleric, temp);
	_numSpellsWisAdj = _staticres->loadRawData(kEoBBaseNumSpellsWisAdj, temp);
	_numSpellsPal = _staticres->loadRawData(kEoBBaseNumSpellsPal, temp);
	_numSpellsMage = _staticres->loadRawData(kEoBBaseNumSpellsMage, temp);

	_characterGuiStringsHp = _staticres->loadStrings(kEoBBaseCharGuiStringsHp, temp);
	_characterGuiStringsWp = _staticres->loadStrings(_flags.gameID == GI_EOB2 ? kEoBBaseCharGuiStringsWp2 : kEoBBaseCharGuiStringsWp1, temp);
	_characterGuiStringsWr = _staticres->loadStrings(kEoBBaseCharGuiStringsWr, temp);
	_characterGuiStringsSt = _staticres->loadStrings(_flags.gameID == GI_EOB2 ? kEoBBaseCharGuiStringsSt2 : kEoBBaseCharGuiStringsSt1, temp);
	_characterGuiStringsIn = _staticres->loadStrings(kEoBBaseCharGuiStringsIn, temp);

	_characterStatusStrings7 = _staticres->loadStrings(kEoBBaseCharStatusStrings7, temp);
	_characterStatusStrings8 = _staticres->loadStrings(_flags.gameID == GI_EOB2 ? kEoBBaseCharStatusStrings82 : kEoBBaseCharStatusStrings81, temp);
	_characterStatusStrings9 = _staticres->loadStrings(kEoBBaseCharStatusStrings9, temp);
	_characterStatusStrings12 = _staticres->loadStrings(kEoBBaseCharStatusStrings12, temp);
	_characterStatusStrings13 = _staticres->loadStrings(_flags.gameID == GI_EOB2 ? kEoBBaseCharStatusStrings132 : kEoBBaseCharStatusStrings131, temp);

	_levelGainStrings = _staticres->loadStrings(kEoBBaseLevelGainStrings, temp);
	for (int i = 0; i < 5; i++)
		_expRequirementTables[i] = _staticres->loadRawDataBe32(kEoBBaseExperienceTable0 + i, temp);
	_expRequirementTables[5] = _staticres->loadRawDataBe32(kEoBBaseExperienceTable4, temp);

	_classModifierFlags = _staticres->loadRawData(kEoBBaseClassModifierFlags, temp);

	_saveThrowTables[0] = _saveThrowTables[4] = _saveThrowTables[5] = _staticres->loadRawData(kEoBBaseSaveThrowTable1, temp);
	_saveThrowTables[1] = _staticres->loadRawData(kEoBBaseSaveThrowTable2, temp);
	_saveThrowTables[2] = _staticres->loadRawData(kEoBBaseSaveThrowTable3, temp);
	_saveThrowTables[3] = _staticres->loadRawData(kEoBBaseSaveThrowTable4, temp);
	_saveThrowLevelIndex = _staticres->loadRawData(kEoBBaseSaveThrwLvlIndex, temp);
	_saveThrowModDiv = _staticres->loadRawData(kEoBBaseSaveThrwModDiv, temp);
	_saveThrowModExt = _staticres->loadRawData(kEoBBaseSaveThrwModExt, temp);

	_encodeMonsterShpTable = _staticres->loadRawDataBe16(kEoBBaseEncodeMonsterDefs, temp);
	_npcPreset = _staticres->loadEoBNpcData(kEoBBaseNpcPresets, temp);

	_teleporterShapeCoords = _staticres->loadRawData(kEoBBaseDscTelptrShpCoords, temp);
	_portalSeq = (const int8 *)_staticres->loadRawData(kEoBBasePortalSeqData, temp);
	_mnDef = _staticres->loadRawData(kEoBBaseManDef, temp);
	_mnWord = _staticres->loadStrings(kEoBBaseManWord, _mnNumWord);
	_mnPrompt = _staticres->loadStrings(kEoBBaseManPrompt, temp);

	_monsterStepTable0 = (const int8 *)_staticres->loadRawData(_flags.gameID == GI_EOB2 ? kEoBBaseMonsterStepTable02 : kEoBBaseMonsterStepTable01, temp);
	_monsterStepTable1 = (const int8 *)_staticres->loadRawData(kEoBBaseMonsterStepTable1, temp);
	_monsterStepTable2 = (const int8 *)_staticres->loadRawData(kEoBBaseMonsterStepTable2, temp);
	_monsterStepTable3 = (const int8 *)_staticres->loadRawData(kEoBBaseMonsterStepTable3, temp);
	_monsterCloseAttPosTable1 = _staticres->loadRawData(kEoBBaseMonsterCloseAttPosTable1, temp);
	_monsterCloseAttPosTable2 = _staticres->loadRawData(_flags.gameID == GI_EOB2 ? kEoBBaseMonsterCloseAttPosTable22 : kEoBBaseMonsterCloseAttPosTable21, temp);
	_monsterCloseAttUnkTable = (const int8 *)_staticres->loadRawData(kEoBBaseMonsterCloseAttUnkTable, temp);
	_monsterCloseAttChkTable1 = _staticres->loadRawData(kEoBBaseMonsterCloseAttChkTable1, temp);
	_monsterCloseAttChkTable2 = _staticres->loadRawData(kEoBBaseMonsterCloseAttChkTable2, temp);
	_monsterCloseAttDstTable1 = _staticres->loadRawData(kEoBBaseMonsterCloseAttDstTable1, temp);
	_monsterCloseAttDstTable2 = _staticres->loadRawData(kEoBBaseMonsterCloseAttDstTable2, temp);

	_monsterProximityTable = _staticres->loadRawData(kEoBBaseMonsterProximityTable, temp);
	_findBlockMonstersTable = _staticres->loadRawData(kEoBBaseFindBlockMonstersTable, temp);
	_monsterDirChangeTable = (const int8 *)_staticres->loadRawData(kEoBBaseMonsterDirChangeTable, temp);
	_monsterSpecAttStrings = _staticres->loadStrings(kEoBBaseMonsterDistAttStrings, temp);

	_monsterFrmOffsTable1 = (const int8 *)_staticres->loadRawData(kEoBBaseDscMonsterFrmOffsTbl1, temp);
	_monsterFrmOffsTable2 = (const int8 *)_staticres->loadRawData(kEoBBaseDscMonsterFrmOffsTbl2, temp);

	_inventorySlotsX = _staticres->loadRawDataBe16(kEoBBaseInvSlotX, temp);
	_inventorySlotsY = _staticres->loadRawData(kEoBBaseInvSlotY, temp);
	_slotValidationFlags = _staticres->loadRawDataBe16(kEoBBaseSlotValidationFlags, temp);

	_projectileWeaponAmmoTypes = (const int8 *)_staticres->loadRawData(kEoBBaseProjectileWeaponTypes, temp);
	_wandTypes = _staticres->loadRawData(kEoBBaseWandTypes, temp);

	_drawObjPosIndex = _staticres->loadRawData(kEoBBaseDrawObjPosIndex, temp);
	_flightObjFlipIndex = _staticres->loadRawData(kEoBBaseFlightObjFlipIndex, temp);
	_flightObjShpMap = (const int8 *)_staticres->loadRawData(kEoBBaseFlightObjShpMap, temp);
	_flightObjSclIndex = (const int8 *)_staticres->loadRawData(kEoBBaseFlightObjSclIndex, temp);

	_wllFlagPreset = _staticres->loadRawData(kEoBBaseWllFlagPreset, _wllFlagPresetSize);
	_dscShapeCoords = (const int16 *)_staticres->loadRawDataBe16(kEoBBaseDscShapeCoords, temp);

	_dscDoorScaleOffs = _staticres->loadRawData(kEoBBaseDscDoorScaleOffs, temp);
	_dscDoorScaleMult1 = _staticres->loadRawData(kEoBBaseDscDoorScaleMult1, temp);
	_dscDoorScaleMult2 = _staticres->loadRawData(kEoBBaseDscDoorScaleMult2, temp);
	_dscDoorScaleMult3 = _staticres->loadRawData(kEoBBaseDscDoorScaleMult3, temp);
	_dscDoorY1 = _staticres->loadRawData(kEoBBaseDscDoorY1, temp);
	_dscDoorXE = _staticres->loadRawData(kEoBBaseDscDoorXE, temp);

	_dscItemPosIndex = _staticres->loadRawData(kEoBBaseDscItemPosIndex, temp);
	_dscItemShpX = (const int16 *)_staticres->loadRawDataBe16(kEoBBaseDscItemShpX, temp);
	_dscItemScaleIndex = _staticres->loadRawData(kEoBBaseDscItemScaleIndex, temp);
	_dscItemTileIndex = _staticres->loadRawData(kEoBBaseDscItemTileIndex, temp);
	_dscItemShapeMap = _staticres->loadRawData(kEoBBaseDscItemShapeMap, temp);

	_bookNumbers = _staticres->loadStrings(kEoBBaseBookNumbers, temp);
	_mageSpellList = _staticres->loadStrings(kEoBBaseMageSpellsList, _mageSpellListSize);
	_clericSpellList = _staticres->loadStrings(kEoBBaseClericSpellsList, temp);
	_spellNames = _staticres->loadStrings(kEoBBaseSpellNames, temp);

	_magicStrings1 = _staticres->loadStrings(kEoBBaseMagicStrings1, temp);
	_magicStrings2 = _staticres->loadStrings(kEoBBaseMagicStrings2, temp);
	_magicStrings3 = _staticres->loadStrings(kEoBBaseMagicStrings3, temp);
	_magicStrings4 = _staticres->loadStrings(kEoBBaseMagicStrings4, temp);
	_magicStrings6 = _staticres->loadStrings(kEoBBaseMagicStrings6, temp);
	_magicStrings7 = _staticres->loadStrings(kEoBBaseMagicStrings7, temp);
	_magicStrings8 = _staticres->loadStrings(kEoBBaseMagicStrings8, temp);

	_expObjectTlMode = _staticres->loadRawData(kEoBBaseExpObjectTlMode, temp);
	_expObjectTblIndex = _staticres->loadRawData(kEoBBaseExpObjectTblIndex, temp);
	_expObjectShpStart = _staticres->loadRawData(kEoBBaseExpObjectShpStart, temp);
	_expObjectAnimTbl1 = _staticres->loadRawData(kEoBBaseExpObjectTbl1, _expObjectAnimTbl1Size);
	_expObjectAnimTbl2 = _staticres->loadRawData(kEoBBaseExpObjectTbl2, _expObjectAnimTbl2Size);
	_expObjectAnimTbl3 = _staticres->loadRawData(kEoBBaseExpObjectTbl3, _expObjectAnimTbl3Size);

	_sparkEffectDefSteps = _staticres->loadRawData(kEoBBaseSparkDefSteps, temp);
	_sparkEffectDefSubSteps = _staticres->loadRawData(kEoBBaseSparkDefSubSteps, temp);
	_sparkEffectDefShift = _staticres->loadRawData(kEoBBaseSparkDefShift, temp);
	_sparkEffectDefAdd = _staticres->loadRawData(kEoBBaseSparkDefAdd, temp);
	_sparkEffectDefX = _staticres->loadRawData(kEoBBaseSparkDefX, temp);
	_sparkEffectDefY = _staticres->loadRawData(kEoBBaseSparkDefY, temp);
	_sparkEffectOfFlags1 = _staticres->loadRawDataBe32(kEoBBaseSparkOfFlags1, temp);
	_sparkEffectOfFlags2 = _staticres->loadRawDataBe32(kEoBBaseSparkOfFlags2, temp);
	_sparkEffectOfShift = _staticres->loadRawData(kEoBBaseSparkOfShift, temp);
	_sparkEffectOfX = _staticres->loadRawData(kEoBBaseSparkOfX, temp);
	_sparkEffectOfY = _staticres->loadRawData(kEoBBaseSparkOfY, temp);
	_magicFlightObjectProperties = _staticres->loadRawData(kEoBBaseMagicFlightProps, temp);
	_turnUndeadEffect = _staticres->loadRawData(kEoBBaseTurnUndeadEffect, temp);
	_burningHandsDest = _staticres->loadRawData(kEoBBaseBurningHandsDest, temp);
	_coneOfColdDest1 = (const int8 *)_staticres->loadRawData(kEoBBaseConeOfColdDest1, temp);
	_coneOfColdDest2 = (const int8 *)_staticres->loadRawData(kEoBBaseConeOfColdDest2, temp);
	_coneOfColdDest3 = (const int8 *)_staticres->loadRawData(kEoBBaseConeOfColdDest3, temp);
	_coneOfColdDest4 = (const int8 *)_staticres->loadRawData(kEoBBaseConeOfColdDest4, temp);
	_coneOfColdGfxTbl = _staticres->loadRawData(kEoBBaseConeOfColdGfxTbl, _coneOfColdGfxTblSize);

	// Hard code the following strings, since EOB I doesn't have them in the original.
	// EOB I doesn't have load and save menus, because there is only one single
	// save slot. Instead of emulating this we provide a menu similiar to EOB II.

	static const char *const saveLoadStrings[3][4] = {
		{   "Cancel",   "Empty Slot",   "Save Game",    "Load Game"     },
		{   "Abbr.",    "Leerer Slot",  "Speichern",    "  Laden"       },
		{   0,          0,              0,              0               }
	};

	static const char *const errorSlotEmptyString[3] = {
		"There is no game\rsaved in that slot!",
		"Hier ist noch kein\rSpiel gespeichert!",
		0
	};

	_saveLoadStrings = saveLoadStrings[(_flags.lang == Common::EN_ANY) ? 0 : ((_flags.lang == Common::DE_DEU) ? 1 : 2)];
	_errorSlotEmptyString = errorSlotEmptyString[(_flags.lang == Common::EN_ANY) ? 0 : ((_flags.lang == Common::DE_DEU) ? 1 : 2)];
	_menuOkString = "OK";
}

void EoBCoreEngine::initButtonData() {
	static const EoBGuiButtonDef buttonDefs[] = {
		{ 112, 0, 0x1100, 184, 2, 63, 50, 0 },
		{ 113, 0, 0x1100, 256, 2, 63, 50, 1 },
		{ 114, 0, 0x1100, 184, 54, 63, 50, 2 },
		{ 115, 0, 0x1100, 256, 54, 63, 50, 3 },
		{ 48, 110, 0x1100, 289, 177, 31, 21, 0 },
		{ 0, 0, 0x1100, 0, 102, 88, 18, 0 },
		{ 0, 0, 0x1100, 89, 102, 88, 18, 1 },
		{ 0, 0, 0x1100, 0, 72, 88, 29, 2 },
		{ 0, 0, 0x1100, 89, 72, 88, 29, 3 },
		{ 24, 0, 0x1100, 184, 10, 33, 33, 0 },
		{ 0, 0, 0x1100, 256, 10, 33, 33, 1 },
		{ 0, 0, 0x1100, 184, 62, 33, 33, 2 },
		{ 0, 0, 0x1100, 256, 62, 33, 33, 3 },
		{ 0, 0, 0x1100, 216, 10, 31, 33, 0 },
		{ 0, 0, 0x1100, 288, 10, 31, 33, 1 },
		{ 0, 0, 0x1100, 216, 62, 31, 33, 2 },
		{ 0, 0, 0x1100, 288, 62, 31, 33, 3 },
		{ 368, 0, 0x1000, 184, 2, 63, 8, 0 },
		{ 369, 0, 0x1000, 256, 2, 63, 8, 1 },
		{ 370, 0, 0x1000, 184, 54, 63, 8, 2 },
		{ 371, 0, 0x1000, 256, 54, 63, 8, 3 },
		{ 0, 0, 0x1100, 230, 116, 16, 16, 0 },
		{ 0, 0, 0x1100, 278, 116, 16, 16, 1 },
		{ 0, 0, 0x1100, 181, 40, 16, 16, 2 },
		{ 0, 0, 0x1100, 199, 40, 16, 16, 3 },
		{ 0, 0, 0x1100, 181, 58, 16, 16, 4 },
		{ 0, 0, 0x1100, 199, 58, 16, 16, 5 },
		{ 0, 0, 0x1100, 181, 76, 16, 16, 6 },
		{ 0, 0, 0x1100, 199, 76, 16, 16, 7 },
		{ 0, 0, 0x1100, 181, 94, 16, 16, 8 },
		{ 0, 0, 0x1100, 199, 94, 16, 16, 9 },
		{ 0, 0, 0x1100, 181, 112, 16, 16, 10 },
		{ 0, 0, 0x1100, 199, 112, 16, 16, 11 },
		{ 0, 0, 0x1100, 181, 130, 16, 16, 12 },
		{ 0, 0, 0x1100, 199, 130, 16, 16, 13 },
		{ 0, 0, 0x1100, 181, 148, 16, 16, 14 },
		{ 0, 0, 0x1100, 199, 148, 16, 16, 15 },
		{ 0, 0, 0x1100, 225, 55, 16, 16, 16 },
		{ 0, 0, 0x1100, 224, 76, 16, 16, 17 },
		{ 0, 0, 0x1100, 225, 96, 16, 16, 18 },
		{ 0, 0, 0x1100, 298, 55, 16, 16, 19 },
		{ 0, 0, 0x1100, 287, 75, 16, 16, 20 },
		{ 0, 0, 0x1100, 277, 137, 16, 16, 21 },
		{ 0, 0, 0x1100, 300, 94, 16, 16, 22 },
		{ 0, 0, 0x1100, 300, 112, 16, 16, 23 },
		{ 0, 0, 0x1100, 300, 130, 16, 16, 24 },
		{ 0, 0, 0x1100, 236, 37, 31, 16, 25 },
		{ 26, 0, 0x1100, 291, 149, 25, 17, 25 },
		{ 110, 24, 0x1100, 181, 3, 32, 32, 25 },
		{ 96, 352, 0x1100, 24, 128, 21, 16, 25 },
		{ 98, 97, 0x1100, 24, 144, 21, 16, 25 },
		{ 92, 348, 0x1100, 3, 144, 21, 16, 25 },
		{ 102, 358, 0x1100, 45, 144, 21, 16, 25 },
		{ 91, 0, 0x1100, 3, 128, 21, 16, 25 },
		{ 101, 0, 0x1100, 45, 128, 21, 16, 25 },
		{ 110, 0, 0x1100, 184, 0, 136, 120, 0 },
		{ 0, 0, 0x1100, 0, 8, 88, 48, 0 },
		{ 0, 0, 0x1100, 88, 8, 88, 48, 1 },
		{ 0, 0, 0x1100, 24, 8, 128, 96, 1 },
		{ 112, 113, 0x1100, 274, 35, 20, 15, 1 },
		{ 114, 115, 0x1100, 297, 35, 20, 15, 1 },
		{ 2, 0, 0x1100, 68, 121, 18, 10, 0 },
		{ 3, 0, 0x1100, 86, 121, 18, 10, 1 },
		{ 4, 0, 0x1100, 104, 121, 15, 10, 2 },
		{ 5, 0, 0x1100, 122, 121, 15, 10, 3 },
		{ 6, 0, 0x1100, 140, 121, 15, 10, 4 },
		{ 0, 0, 0x1100, 75, 131, 97, 6, 0 },
		{ 0, 0, 0x1100, 75, 137, 97, 6, 1 },
		{ 0, 0, 0x1100, 75, 143, 97, 6, 2 },
		{ 0, 0, 0x1100, 75, 149, 97, 6, 3 },
		{ 0, 0, 0x1100, 75, 155, 97, 6, 4 },
		{ 0, 0, 0x1100, 75, 161, 97, 6, 5 },
		{ 112, 0, 0x1100, 184, 2, 63, 50, 0 },
		{ 113, 0, 0x1100, 256, 2, 63, 50, 1 },
		{ 114, 0, 0x1100, 184, 54, 63, 50, 2 },
		{ 115, 0, 0x1100, 256, 54, 63, 50, 3 },
		{ 53, 54, 0x1100, 320, 200, 0, 0, 6 },
		{ 61, 0, 0x1100, 320, 200, 0, 0, 7 },
		{ 0, 0, 0x1100, 184, 114, 33, 33, 4 },
		{ 0, 0, 0x1100, 256, 114, 33, 33, 5 },
		{ 0, 0, 0x1100, 216, 114, 31, 33, 4 },
		{ 0, 0, 0x1100, 288, 114, 31, 33, 5 },
		{ 372, 0, 0x1000, 184, 106, 63, 8, 4 },
		{ 373, 0, 0x1000, 256, 106, 63, 8, 5 },
		{ 0, 0, 0x1100, 227, 135, 10, 10, 25 },
		{ 0, 0, 0x1100, 239, 135, 10, 10, 26 },
		{ 116, 0, 0x1100, 184, 106, 63, 50, 4 },
		{ 117, 0, 0x1100, 256, 106, 63, 50, 5 },
		{ 110, 0, 0x1100, 68, 168, 78, 10, 0 },
		{ 110, 0, 0x1100, 68, 168, 78, 10, 65535 },
		{ 116, 0, 0x1100, 184, 106, 63, 50, 4 },
		{ 117, 0, 0x1100, 256, 106, 63, 50, 5 },
		{ 116, 117, 0x1100, 320, 200, 1, 1, 2 },
		{ 7, 0, 0x1100, 158, 121, 15, 10, 5 },
		{ 0, 0, 0x1100, 146, 168, 32, 10, 0 },

		// EOB1 spellbook modifications
		{ 2, 0, 0x1100, 71, 122, 20, 8, 0 },
		{ 3, 0, 0x1100, 92, 122, 20, 8, 1 },
		{ 4, 0, 0x1100, 113, 122, 20, 8, 2 },
		{ 5, 0, 0x1100, 134, 122, 20, 8, 3 },
		{ 6, 0, 0x1100, 155, 122, 20, 8, 4 },
		{ 110, 0, 0x1100, 75, 168, 97, 6, 0 }
	};

	_buttonDefs = buttonDefs;
	_buttonCallbacks.clear();
	_buttonCallbacks.reserve(ARRAYSIZE(buttonDefs));

#define EOB_CBN(x, y) _buttonCallbacks.push_back(BUTTON_FUNCTOR(EoBCoreEngine, this, &EoBCoreEngine::y)); for (int l = 0; l < (x - 1); l++) { _buttonCallbacks.push_back(_buttonCallbacks[_buttonCallbacks.size() - 1 - l]); }
#define EOB_CBI(x, y) for (int l = x; l; l--) { _buttonCallbacks.push_back(_buttonCallbacks[y]); }
	EOB_CBN(4, clickedCharPortraitDefault);
	EOB_CBN(1, clickedCamp);
	EOB_CBN(4, clickedSceneDropPickupItem);
	EOB_CBN(4, clickedCharPortrait2);
	EOB_CBN(4, clickedWeaponSlot);
	EOB_CBN(4, clickedCharNameLabelRight);
	EOB_CBN(25, clickedInventorySlot);
	EOB_CBN(1, clickedEatItem);
	EOB_CBN(1, clickedInventoryNextPage);
	EOB_CBN(1, clickedPortraitRestore);
	EOB_CBN(1, clickedUpArrow);
	EOB_CBN(1, clickedDownArrow);
	EOB_CBN(1, clickedLeftArrow);
	EOB_CBN(1, clickedRightArrow);
	EOB_CBN(1, clickedTurnLeftArrow);
	EOB_CBN(1, clickedTurnRightArrow);
	EOB_CBN(1, clickedAbortCharSwitch);
	EOB_CBN(2, clickedSceneThrowItem);
	EOB_CBN(1, clickedSceneSpecial);
	EOB_CBN(1, clickedInventoryPrevChar);
	EOB_CBN(1, clickedInventoryNextChar);
	EOB_CBN(5, clickedSpellbookTab);
	EOB_CBN(6, clickedSpellbookList);
	EOB_CBN(4, clickedCastSpellOnCharacter);
	EOB_CBI(2, 66);
	EOB_CBI(2, 9);
	EOB_CBI(2, 13);
	EOB_CBI(2, 17);
	EOB_CBI(2, 21);
	EOB_CBI(2, 72);
	EOB_CBN(1, clickedSpellbookAbort);
	EOB_CBI(1, 72);
	EOB_CBI(2, 0);
	EOB_CBI(1, 60);
	EOB_CBI(1, 61);
	EOB_CBN(1, clickedSpellbookScroll);
	EOB_CBI(5, 61);
	EOB_CBI(1, 88);
#undef EOB_CBI
#undef EOB_CBN
}

void EoBCoreEngine::initMenus() {
	static const EoBMenuButtonDef buttonDefs[] = {
		{  2,   12,  20, 158,  14,  20,  3  },
		{  3,   12,  37, 158,  14,  52,  3  },
		{  4,   12,  54, 158,  14,  26,  3  },
		{  5,   12,  71, 158,  14,  32,  3  },
		{  6,   12,  88, 158,  14,   0,  3  },
		{  7,   12, 105, 158,  14,  35,  3  },
		{  8,  128, 122,  40,  14,  19,  7  },
		{  9,   12,  20, 158,  14,  39,  3  },
		{  10,  12,  37, 158,  14,  32,  3  },
		{  11,  12,  54, 158,  14,  33,  3  },
		{  12,  12,  71, 158,  14,  17,  3  },
		{  8,  128, 122,  40,  14,  19,  7  },
		{  18,  12,  20, 158,  14,  32,  3  },
		{  19,  12,  37, 158,  14,  50,  3  },
		{  8,  128, 122,  40,  14,  19,  7  },
		{  8,  128, 122,  40,  14,  19,  5  },
		{  0,  184,   0,  64,  48, 112,  0  },
		{  0,  256,   0,  64,  48, 113,  0  },
		{  0,  184,  56,  64,  48, 114,  0  },
		{  0,  256,  56,  64,  48, 115,  0  },
		{  0,  184, 112,  64,  48, 116,  0  },
		{  0,  256, 112,  64,  48, 117,  0  },
		{  36,   8, 126,  48,  14,  48,  5  },
		{  8,  128, 126,  40,  14,  19,  5  },
		{  0,    0,  50, 168,  72,  61,  0  },
		{  31,  11,  16,  20,  18,   2,  5  },
		{  32,  38,  16,  20,  18,   3,  5  },
		{  33,  65,  16,  20,  18,   4,  5  },
		{  34,  92,  16,  20,  18,   5,  5  },
		{  35, 119,  16,  20,  18,   6,  5  },
		{  60, 146,  16,  20,  18,   7,  5  },
		{  61, 150,  16,  20,  18,   8,  5  },
		{  38,  16,  57,  32,  14,  22,  7  },
		{  39, 128,  57,  32,  14,  51,  7  },
		{  8,  128, 126,  40,  14,  19,  7  },
		{  0,    0,  50, 168,  72,  61,  0  },
		// EOB 1 memorize/pray menu:
		{  36,   8, 126,  48,  14,  48,  5  },
		{  8,  128, 126,  40,  14,  19,  5  },
		{  0,    0,  50, 168,  72,  61,  0  },
		{  31,   8,  16,  24,  20,   2,  5  },
		{  32,  40,  16,  24,  20,   3,  5  },
		{  33,  72,  16,  24,  20,   4,  5  },
		{  34, 104,  16,  24,  20,   5,  5  },
		{  35, 136,  16,  24,  20,   6,  5  }
	};

	_menuButtonDefs = buttonDefs;

	static const EoBMenuDef menuDefs[] = {
		{  1, 10,  0, 7,  9 },
		{  1, 10,  7, 5,  9 },
		{  1, 10, 12, 3,  9 },
		{  0, 10, 15, 7, 15 },
		{ 37, 10, 22, 9,  9 },
		{  0, 11, 32, 2, 15 },
		{ 48, 10, 34, 2,  9 }
	};

	delete[] _menuDefs;
	_menuDefs = new EoBMenuDef[ARRAYSIZE(menuDefs)];
	memcpy(_menuDefs, menuDefs, sizeof(menuDefs));

	if (_flags.gameID == GI_EOB1) {
		// assign EOB 1 style memorize/pray menu
		_menuDefs[4].numButtons = 8;
		_menuDefs[4].firstButtonStrId = 36;
	}
}


void EoBCoreEngine::initSpells() {
#define mpn magicTimingParaAssign.push_back(0);
#define mp1n if (_flags.gameID == GI_EOB1) magicTimingParaAssign.push_back(0);
#define mp2n if (_flags.gameID == GI_EOB2) magicTimingParaAssign.push_back(0);
#define mp(x) magicTimingParaAssign.push_back(&magicTimingPara[x << 2]);
#define mp1(x) if (_flags.gameID == GI_EOB1) magicTimingParaAssign.push_back(&magicTimingPara[x << 2]);
#define mp2(x) if (_flags.gameID == GI_EOB2) magicTimingParaAssign.push_back(&magicTimingPara[x << 2]);

#define sc(x) startCallback.push_back(&EoBCoreEngine::spellCallback_start_##x);
#define sc1(x) if (_flags.gameID == GI_EOB1) startCallback.push_back(&EoBCoreEngine::spellCallback_start_##x);
#define sc2(x) if (_flags.gameID == GI_EOB2) startCallback.push_back(&EoBCoreEngine::spellCallback_start_##x);
#define ec(x) endCallback.push_back(&EoBCoreEngine::spellCallback_end_##x);
#define ec1(x) if (_flags.gameID == GI_EOB1) endCallback.push_back(&EoBCoreEngine::spellCallback_end_##x);
#define ec2(x) if (_flags.gameID == GI_EOB2) endCallback.push_back(&EoBCoreEngine::spellCallback_end_##x);

	static const uint16 magicTimingPara[] = {
		0, 546, 2, 1, // 0 detect magic
		0, 546, 5, 1, // 1 shield, detect invis, magical vestment
		0, 546, 1, 1, // 2 shocking grasp, vamp touch, true seeing, prayer
		3, 546, 1, 1, // 3 blur, haste
		5, 546, 1, 1, // 4 imp invisibility
		6, 546, 0, 1, // 5 bless
		0, 546, 3, 1, // 6 prot from evil
		1, 546, 1, 1, // 7 aid
		4, 546, 1, 1, // 8 flame blade
		0, 32760, 1, 1, // 9 slow poison
		1, 546, 0, 1, // 10 mystic defense
	};

	Common::Array<const uint16 *> magicTimingParaAssign;
	mpn;
	mpn;
	mpn;
	mp(0);  // Detect Magic
	mpn;    // Magic Missile
	mp1n;
	mp(1);  // Shield
	mp(2);  // Shocking Grasp
	mp2(3); // Blur
	mp2(1); // Detect Invis
	mp2n;   // Imp Identify
	mpn;    // Invis
	mp1n;
	mpn;    // Melf
	mp1n;   // Stinking Cloud
	mpn;    // Dispel Magic
	mpn;    // Fireball
	mp1n;   // Flame Arrow
	mp(3);  // Haste
	mpn;    // Hold Person
	mpn;    // Invisibility
	mpn;    // Lightning Bolt
	mp(2);  // Vampiric Touch
	mpn;    // Fear
	mpn;    // Ice Storm
	mp1n;   // Stone Skin
	mp1n;   // Cloud Kill
	mp2(4); // Improved Invisibility
	mp2n;   // remove Curse
	mpn;    // Cone of Cold
	mpn;    // Hold Monster
	mp2n;   // Wall of Force
	mp2n;   // Disintegrate
	mp2n;   // Flesh To Stone
	mp2n;   // Stone To Flesh
	mp2(2); // True Seeing
	mp2n;   // Finger of Death
	mp2n;   // Power Word Stun
	mp2n;   // Bigby's Fist
	mp2n;   // empty
	mp(5);  // Bless
	mpn;                        // EOB1: cure, EOB2: cause
	mpn;                        // EOB1: cause, EOB2: cure
	mp(0);  // Detect Magic
	mp(6);  // Prot from Evil
	mp(7);  // Aid
	mp(8);  // Flame Blad
	mpn;    // Hold Person
	mp(9);  // Slow Poison
	mpn;    // Create Food
	mpn;    // Dispel Magic
	mp(1);  // Magical Vestment
	mp(2);  // Prayer
	mpn;    // Remove Paralysis
	mpn;                        // EOB1: cure, EOB2: cause
	mpn;                        // EOB1: cause, EOB2: cure
	mpn;    // Neutral Poison
	mp(6);  // Prot From Evil 10'
	mp1n;   // Prot From Lightning
	mpn;                        // EOB1: cure, EOB2: cause
	mpn;                        // EOB1: cause, EOB2: cure
	mpn;    // Flame Strike
	mpn;    // Raise Dead
	mp2n;   // Slay Living
	mp2(2); // True Seeing
	mp2n;   // Harm
	mp2n;   // Heal
	mp2n;   // Resurrect
	mpn;    // Lay on Hands
	mp2n;   // Turn Undead
	mpn;    // Lightning Bolt (EOB1) / Fireball 1(EOB2) passive
	mp2(10);// Mystic Defense
	mp2n;   // Fireball 2 passive
	mpn;    // death spell passive
	mpn;    // disintegrate passive
	mp2n;   // cause critical passive
	mp2n;   // flesh to stone passive

	Common::Array<SpellStartCallback> startCallback;
	sc(empty);
	sc(armor);
	sc(burningHands);
	sc(detectMagic);
	sc(magicMissile);
	sc1(empty);
	sc(empty);
	sc(shockingGrasp);
	sc(empty);
	sc2(empty);
	sc2(improvedIdentify);
	sc(empty);
	sc(melfsAcidArrow);
	sc1(empty);     // Stinking Cloud
	sc(dispelMagic);
	sc(fireball);
	sc1(flameArrow);
	sc(empty);
	sc(holdPerson);
	sc(empty);
	sc(lightningBolt);
	sc(vampiricTouch);
	sc(fear);
	sc(iceStorm);
	sc1(stoneSkin); // stone skin
	sc2(empty); // imp invisibility
	sc1(empty); // Cloudkill
	sc2(removeCurse);
	sc(coneOfCold);
	sc(holdMonster);
	sc2(wallOfForce);
	sc2(disintegrate);
	sc2(fleshToStone);
	sc2(stoneToFlesh);
	sc2(trueSeeing);
	sc2(slayLiving);
	sc2(powerWordStun);
	sc2(empty);
	sc2(empty);
	sc(empty);  // Bless
	sc2(causeLightWounds);
	sc(cureLightWounds);
	sc1(causeLightWounds);
	sc(detectMagic);
	sc(empty);
	sc(aid);
	sc(flameBlade);
	sc(holdPerson);
	sc(slowPoison);
	sc(createFood);
	sc(dispelMagic);
	sc(empty);
	sc(empty);
	sc(removeParalysis);
	sc2(causeSeriousWounds);
	sc(cureSeriousWounds);
	sc1(causeSeriousWounds);
	sc(neutralizePoison);
	sc(empty);
	sc1(empty);
	sc2(causeCriticalWounds);
	sc(cureCriticalWounds);
	sc1(causeCriticalWounds);
	sc(flameStrike);
	sc(raiseDead);
	sc2(slayLiving);
	sc2(trueSeeing);
	sc2(harm);
	sc2(heal);
	sc2(empty);
	sc(layOnHands);
	sc2(turnUndead);
	sc(empty);
	sc2(empty);
	sc2(empty);
	sc(empty);
	sc(empty);
	sc2(empty);
	sc2(empty);

	Common::Array<SpellEndCallback> endCallback;
	ec(empty);
	ec(empty);
	ec(empty);
	ec(detectMagic);
	ec(magicMissile);
	ec1(empty);
	ec(empty);
	ec(shockingGraspFlameBlade);
	ec(empty);
	ec(empty);
	ec2(empty);
	ec2(empty);
	ec(melfsAcidArrow);
	ec1(empty);     // Stinking Cloud
	ec(empty);
	ec(fireball);
	ec1(flameArrow);
	ec(empty);
	ec(holdPerson);
	ec(empty);
	ec(lightningBolt);
	ec(vampiricTouch);
	ec(empty);
	ec(iceStorm);
	ec(empty);      // EOB1: stone skin, EOB2: imp invisibility
	ec(empty);      // EOB1: cloud kill, EOB2: remove curse
	ec(empty);
	ec(holdMonster);
	ec2(empty);
	ec2(empty);
	ec2(empty);
	ec2(empty);
	ec2(trueSeeing);
	ec2(empty);
	ec2(empty);
	ec2(empty);
	ec2(empty);
	ec(empty);  // Bless
	ec(empty);
	ec(empty);
	ec(detectMagic);
	ec(empty);
	ec(aid);
	ec(shockingGraspFlameBlade);
	ec(holdPerson);
	ec(slowPoison);
	ec(empty);
	ec(empty);
	ec(empty);
	ec(empty);
	ec(empty);
	ec(empty);
	ec(empty);
	ec(empty);
	ec(empty);
	ec1(empty); // Prot from Lightning
	ec(empty);
	ec(empty);
	ec(flameStrike);
	ec(empty);
	ec2(empty);
	ec2(trueSeeing);
	ec2(empty);
	ec2(empty);
	ec2(empty);
	ec(empty);
	ec2(empty);
	ec1(monster_lightningBolt);
	ec2(monster_fireball1);
	ec2(empty);
	ec2(monster_fireball2);
	ec(monster_deathSpell);
	ec(monster_disintegrate);
	ec2(monster_causeCriticalWounds);
	ec2(monster_fleshToStone);

	_spells = new EoBSpell[_numSpells];
	memset(_spells, 0, _numSpells * sizeof(EoBSpell));

	for (int i = 0, n = 0; i < _numSpells; i++, n++) {
		EoBSpell *s = &_spells[i];

		// Fix EoB 1 spell names
		bool skip = false;
		if (i == 5 || i == 9) {
			n--;
			skip = true;
		}

		s->name = _flags.gameID == GI_EOB2 ? ((i == 0 || i == _mageSpellListSize) ? _mageSpellList[0] : ((i < (_mageSpellListSize + 1)) ? _spellNames[i - 1] : _spellNames[i - 2])) : (skip ? _spellNames[0] : _spellNames[n]);
		s->startCallback = startCallback[i];
		s->timingPara = magicTimingParaAssign[i];
		s->endCallback = endCallback[i];
	}

	magicTimingParaAssign.clear();
	startCallback.clear();
	endCallback.clear();

	_clericSpellOffset = _mageSpellListSize;

#undef mpn
#undef mp1n
#undef mp2n
#undef mp
#undef mp1
#undef mp2
#undef sc
#undef sc1
#undef sc2
#undef ec
#undef ec1
#undef ec2
}

void EoBEngine::initStaticResource() {
	int temp;
	_mainMenuStrings = _staticres->loadStrings(kEoB1MainMenuStrings, temp);
	_finBonusStrings = _staticres->loadStrings(kEoB1BonusStrings, temp);

	_doorShapeEncodeDefs = _staticres->loadRawData(kEoB1DoorShapeDefs, temp);
	_doorSwitchShapeEncodeDefs = _staticres->loadRawData(kEoB1DoorSwitchShapeDefs, temp);
	_doorSwitchCoords = _staticres->loadRawData(kEoB1DoorSwitchCoords, temp);

	_dscDoorScaleMult4 = _staticres->loadRawData(kEoBBaseDscDoorScaleMult4, temp);
	_dscDoorScaleMult5 = _staticres->loadRawData(kEoBBaseDscDoorScaleMult5, temp);
	_dscDoorScaleMult6 = _staticres->loadRawData(kEoBBaseDscDoorScaleMult6, temp);
	_dscDoorY3 = _staticres->loadRawData(kEoBBaseDscDoorY3, temp);
	_dscDoorY4 = _staticres->loadRawData(kEoBBaseDscDoorY4, temp);
	_dscDoorY5 = _staticres->loadRawData(kEoBBaseDscDoorY5, temp);
	_dscDoorY6 = _staticres->loadRawData(kEoBBaseDscDoorY6, temp);
	_dscDoorY7 = _staticres->loadRawData(kEoBBaseDscDoorY7, temp);
	_dscDoorCoordsExt = (const int16 *)_staticres->loadRawDataBe16(kEoBBaseDscDoorCoordsExt, temp);

	_enemyMageSpellList = _staticres->loadRawData(kEoB1EnemyMageSpellList, temp);
	_enemyMageSfx = _staticres->loadRawData(kEoB1EnemyMageSfx, temp);
	_beholderSpellList = _staticres->loadRawData(kEoB1BeholderSpellList, temp);
	_beholderSfx = _staticres->loadRawData(kEoB1BeholderSfx, temp);

	_cgaMappingDefault = _staticres->loadRawData(kEoB1CgaMappingDefault, temp);
	_cgaMappingAlt = _staticres->loadRawData(kEoB1CgaMappingAlt, temp);
	_cgaMappingInv = _staticres->loadRawData(kEoB1CgaMappingInv, temp);
	_cgaMappingItemsL = _staticres->loadRawData(kEoB1CgaMappingItemsL, temp);
	_cgaMappingItemsS = _staticres->loadRawData(kEoB1CgaMappingItemsS, temp);
	_cgaMappingThrown = _staticres->loadRawData(kEoB1CgaMappingThrown, temp);
	_cgaMappingIcons = _staticres->loadRawData(kEoB1CgaMappingIcons, temp);
	_cgaMappingDeco = _staticres->loadRawData(kEoB1CgaMappingDeco, temp);
	_cgaLevelMappingIndex = _staticres->loadRawData(kEoB1CgaLevelMappingIndex, temp);
	for (int i = 0; i < 5; i++)
		_cgaMappingLevel[i] = _staticres->loadRawData(kEoB1CgaMappingLevel0 + i, temp);

	_turnUndeadString = _staticres->loadStrings(kEoB1TurnUndeadString, temp);

	_npcShpData = _staticres->loadRawData(kEoB1NpcShpData, temp);
	_npcSubShpIndex1 = _staticres->loadRawData(kEoB1NpcSubShpIndex1, temp);
	_npcSubShpIndex2 = _staticres->loadRawData(kEoB1NpcSubShpIndex2, temp);
	_npcSubShpY = _staticres->loadRawData(kEoB1NpcSubShpY, temp);
	for (int i = 0; i < 11; i++)
		_npcStrings[i] = _staticres->loadStrings(kEoB1Npc0Strings + i, temp);

	const uint8 *ps = _staticres->loadRawData(kEoB1MonsterProperties, temp);
	temp /= 27;
	_monsterProps = new EoBMonsterProperty[temp];
	memset(_monsterProps, 0, temp * sizeof(EoBMonsterProperty));
	// Convert EOB1 (hard coded) monster properties to EOB2 type monster properties.
	for (int i = 0; i < temp; i++) {
		EoBMonsterProperty *p = &_monsterProps[i];
		p->armorClass = (int8)*ps++;
		p->hitChance = (int8)*ps++;
		p->level = (int8)*ps++;
		p->attacksPerRound = *ps++;
		p->dmgDc[0].times = *ps++;
		p->dmgDc[0].pips = *ps++;
		p->dmgDc[0].base = (int8)*ps++;
		p->dmgDc[1].times = *ps++;
		p->dmgDc[1].pips = *ps++;
		p->dmgDc[1].base = (int8)*ps++;
		p->dmgDc[2].times = *ps++;
		p->dmgDc[2].pips = *ps++;
		p->dmgDc[2].base = (int8)*ps++;
		ps++;
		p->capsFlags = *ps++;
		p->typeFlags = READ_LE_UINT16(ps);
		ps += 2;
		ps++;
		ps++;
		p->experience = READ_LE_UINT16(ps);
		ps += 2;
		p->u30 = *ps++;
		p->sound1 = (int8)*ps++;
		p->sound2 = (int8)*ps++;
		p->numRemoteAttacks = *ps++;
		p->tuResist = (int8)*ps++;
		p->dmgModifierEvade = *ps++;
	}

	static const char *const errorSlotNoNameString[3] = {
		" You must specify\r a name for your\r save game!",
		" Spielstaende mues-\r sen einen Namen\r haben!",
		0
	};

	_errorSlotNoNameString = errorSlotNoNameString[(_flags.lang == Common::EN_ANY) ? 0 : ((_flags.lang == Common::DE_DEU) ? 1 : 2)];
}

void EoBEngine::initSpells() {
	EoBCoreEngine::initSpells();

	struct FlagTableEntry {
		uint16 typeFlag;
		uint32 effectFlag;
		uint8 damageFlag;
	};

	static const FlagTableEntry flagTable[] = {
		{ 0x0000, 0x000000, 0x00 }, // dummy
		{ 0x0033, 0x000001, 0x00 }, // armor
		{ 0x0100, 0x000000, 0x21 }, // burning hands
		{ 0x004c, 0x000002, 0x00 }, // detect magic
		{ 0x0100, 0x000000, 0x01 }, // magic missile
		{ 0x0000, 0x000000, 0x00 }, // dummy
		{ 0x008b, 0x000008, 0x00 }, // shield
		{ 0x0488, 0x000000, 0x03 }, // shocking grasp
		{ 0x0021, 0x000040, 0x00 }, // invisibility
		{ 0x0000, 0x000000, 0x00 }, // dummy
		{ 0x0100, 0x000000, 0x11 }, // melf's acid arrow
		{ 0x0000, 0x000000, 0x00 }, // STINKING CLOUD
		{ 0x1000, 0x000000, 0x00 }, // dispel magic
		{ 0x0100, 0x000000, 0x21 }, // fireball
		{ 0x0100, 0x000000, 0x11 }, // FLAME ARROW
		{ 0x0248, 0x010000, 0x00 }, // haste
		{ 0x0100, 0x000000, 0x00 }, // hold person
		{ 0x0240, 0x000040, 0x00 }, // inv 10'
		{ 0x0100, 0x000000, 0x03 }, // lightning bolt
		{ 0x0488, 0x000000, 0x01 }, // vampiric touch
		{ 0x0100, 0x000000, 0x00 }, // fear
		{ 0x0100, 0x000000, 0x41 }, // ice storm
		{ 0x0033, 0x000001, 0x00 }, // STONE SKIN
		{ 0x0000, 0x000000, 0x00 }, // CLOUD KILL
		{ 0x0100, 0x000000, 0x41 }, // cone of cold
		{ 0x0100, 0x000000, 0x00 }, // hold monster
		{ 0x005c, 0x000400, 0x00 }, // bless
		{ 0x0020, 0x000000, 0x00 }, // cure light wounds
		{ 0x0100, 0x000000, 0x01 }, // cause light wounds
		{ 0x004c, 0x000002, 0x00 }, // detect magic
		{ 0x0029, 0x000800, 0x00 }, // prot from evil
		{ 0x0039, 0x000000, 0x00 }, // aid
		{ 0x2408, 0x000000, 0x21 }, // flame blade
		{ 0x0100, 0x000000, 0x00 }, // hold person
		{ 0x0028, 0x002000, 0x00 }, // slow poison
		{ 0x0040, 0x000000, 0x00 }, // create food
		{ 0x1000, 0x000000, 0x00 }, // dispel magic
		{ 0x0099, 0x004000, 0x00 }, // magical vestment
		{ 0x004c, 0x008000, 0x00 }, // prayer
		{ 0x0040, 0x000000, 0x00 }, // remove paralysis
		{ 0x0020, 0x000000, 0x00 }, // cure serious
		{ 0x0100, 0x000000, 0x01 }, // cause serious
		{ 0x0020, 0x000000, 0x00 }, // neutralize poison
		{ 0x0248, 0x000800, 0x00 }, // prot from evil 10'
		{ 0x0000, 0x000000, 0x00 }, // PROT FROM LIGHTNING
		{ 0x0020, 0x000000, 0x00 }, // cure critical
		{ 0x0100, 0x000000, 0x01 }, // cause critical
		{ 0x0100, 0x000000, 0x21 }, // flame strike
		{ 0x0020, 0x000000, 0x00 }, // raise dead
		{ 0x0020, 0x000000, 0x00 }, // lay on hands
		{ 0x0000, 0x000000, 0x00 }, // obj hit passive
		{ 0x0000, 0x000000, 0x00 }, // disintegrate passive
		{ 0x0000, 0x000000, 0x00 }  // death spell passive
	};

	int temp;
	const uint8 *src = _staticres->loadRawData(kEoBBaseSpellProperties, temp);
	_clericSpellOffset -= 1;

	for (int i = 0; i < _numSpells; i++) {
		EoBSpell *s = &_spells[i];
		src += 4;
		s->flags = flagTable[i].typeFlag;
		s->damageFlags = flagTable[i].damageFlag;
		s->effectFlags = flagTable[i].effectFlag;
		s->sound = src[13];
		src += 15;
	}
}

const KyraRpgGUISettings EoBEngine::_guiSettingsVGA = {
	{ 9, 15, 95, 9, 7, { 285, 139 }, { 189, 162 }, { 31, 31 } },
	{ 135, 130, 132, 133, 133, 17, 23, 20, 184, 177, 180, 184, 177, 180 }
};

const KyraRpgGUISettings EoBEngine::_guiSettingsEGA = {
	{ 9, 15, 95, 9, 7, { 285, 139 }, { 189, 162 }, { 31, 31 } },
	{ 13, 9, 2, 133, 2, 6, 13, 8, 13, 15, 14, 13, 15, 14 }
};

const uint8 EoBEngine::_egaDefaultPalette[] = {
	0, 5, 3, 2, 10, 14, 12, 6, 4, 11, 9, 1, 0, 8, 7, 15
};

void DarkMoonEngine::initStaticResource() {
	int temp;
	_mainMenuStrings = _staticres->loadStrings(kEoB2MainMenuStrings, temp);
	_introStrings = _staticres->loadStrings(kEoB2IntroStrings, temp);
	_cpsFilesIntro = _staticres->loadStrings(kEoB2IntroCPSFiles, temp);

	_animIntro = new const DarkMoonAnimCommand*[44];
	for (int i = 0; i < 44; i++)
		_animIntro[i] = _staticres->loadEoB2SeqData(kEob2IntroAnimData00 + i, temp);

	_shapesIntro = new const DarkMoonShapeDef*[13];
	memset(_shapesIntro, 0, sizeof(DarkMoonShapeDef *) * 13);
	_shapesIntro[0] = _staticres->loadEoB2ShapeData(kEoB2IntroShapes00, temp);
	_shapesIntro[1] = _staticres->loadEoB2ShapeData(kEoB2IntroShapes01, temp);
	_shapesIntro[4] = _staticres->loadEoB2ShapeData(kEoB2IntroShapes04, temp);
	_shapesIntro[7] = _staticres->loadEoB2ShapeData(kEoB2IntroShapes07, temp);

	_finaleStrings = _staticres->loadStrings(kEoB2FinaleStrings, temp);
	_creditsData = _staticres->loadRawData(kEoB2CreditsData, temp);
	_cpsFilesFinale = _staticres->loadStrings(kEoB2FinaleCPSFiles, temp);

	_animFinale = new const DarkMoonAnimCommand*[21];
	for (int i = 0; i < 21; i++)
		_animFinale[i] = _staticres->loadEoB2SeqData(kEob2FinaleAnimData00 + i, temp);

	_shapesFinale = new const DarkMoonShapeDef*[13];
	memset(_shapesFinale, 0, sizeof(DarkMoonShapeDef *) * 13);
	_shapesFinale[0] = _staticres->loadEoB2ShapeData(kEoB2FinaleShapes00, temp);
	_shapesFinale[3] = _staticres->loadEoB2ShapeData(kEoB2FinaleShapes03, temp);
	_shapesFinale[7] = _staticres->loadEoB2ShapeData(kEoB2FinaleShapes07, temp);
	_shapesFinale[9] = _staticres->loadEoB2ShapeData(kEoB2FinaleShapes09, temp);
	_shapesFinale[10] = _staticres->loadEoB2ShapeData(kEoB2FinaleShapes10, temp);

	_dscDoorType5Offs = _staticres->loadRawData(kEoBBaseDscDoorType5Offs, temp);

	_npcShpData = _staticres->loadRawData(kEoB2NpcShapeData, temp);
	_npcStrings[0] = _staticres->loadStrings(kEoB2Npc1Strings, temp);
	_npcStrings[1] = _staticres->loadStrings(kEoB2Npc2Strings, temp);
	_monsterDustStrings = _staticres->loadStrings(kEoB2MonsterDustStrings, temp);
	_dreamSteps = (const int8 *)_staticres->loadRawData(kEoB2DreamSteps, temp);
	_kheldranStrings = _staticres->loadStrings(kEoB2KheldranStrings, temp);
	_hornStrings = _staticres->loadStrings(kEoB2HornStrings, temp);
	_hornSounds = _staticres->loadRawData(kEoB2HornSounds, temp);

	_wallOfForceDsX = (const int16 *)_staticres->loadRawDataBe16(kEoB2WallOfForceDsX, temp);
	_wallOfForceDsY = _staticres->loadRawData(kEoB2WallOfForceDsY, temp);
	_wallOfForceDsNumW = _staticres->loadRawData(kEoB2WallOfForceNumW, temp);
	_wallOfForceDsNumH = _staticres->loadRawData(kEoB2WallOfForceNumH, temp);
	_wallOfForceShpId = _staticres->loadRawData(kEoB2WallOfForceShpId, temp);

	static const char *const errorSlotNoNameString[3] = {
		" You must specify\r a name for your\r save game!",
		" Spielst[nde m]ssen\r einen Namen haben!",
		0
	};

	_errorSlotNoNameString = errorSlotNoNameString[(_flags.lang == Common::EN_ANY) ? 0 : ((_flags.lang == Common::DE_DEU) ? 1 : 2)];

	// ScummVM specific
	static const char *const transferStringsScummVM[3][5] = {
		{
			"\r We cannot find any EOB save game\r file. Please make sure that the\r save game file with the party\r you wish to transfer is located\r in your ScummVM save game\r directory. If you have set up\r multiple save directories you\r have to copy the EOB save file\r into your EOB II save directory.\r Do you wish to try again?",
			"Game ID",
			"\r It seems that you have already\r defeated Xanathar here. Do you\r wish to transfer the party that\r finished the game? If not, you\r will be able to select a save\r game from the save game\r dialogue.",
			"Select File",
			"\r\r   Please wait..."
		},
		{
			"\r Kein EOB-Spielstand zu finden.\r Bitte Spielstandsdatei mit der\r zu ]bernehmenden Gruppe in das\r ScummVM Spielstands-Verzeichnis\r kopieren. Bei mehreren Spiel-\r stands-Verzeichnissen bitte\r den EOB-Spielstand in das\r EOB II-Spielstands-Verzeichnis\r kopieren. Nochmal versuchen?",
			"Game ID",
			"\r Wie es scheint, wurde Xanathar\r hier bereits besiegt. Soll die\r Gruppe, mit der das Spiel be-\r endet wurde, ]bernommen werden?\r Falls nicht, kann ein Spielstand\r aus der Spielstandsliste gew[hlt\r werden.",
			"Spiel W[hlen",
			"\r\r  Bitte warten..."
		},
		{
			0, 0, 0, 0
		}
	};

	_transferStringsScummVM = transferStringsScummVM[(_flags.lang == Common::EN_ANY) ? 0 : ((_flags.lang == Common::DE_DEU) ? 1 : 2)];
}

void DarkMoonEngine::initSpells() {
	EoBCoreEngine::initSpells();

	int temp;
	const uint8 *src = _staticres->loadRawData(kEoBBaseSpellProperties, temp);

	for (int i = 0; i < _numSpells; i++) {
		EoBSpell *s = &_spells[i];
		src += 8;
		s->flags = READ_LE_UINT16(src);
		src += 10;
		s->sound = *src++;
		s->effectFlags = READ_LE_UINT32(src);
		src += 4;
		s->damageFlags = READ_LE_UINT16(src);
		src += 2;
	}
}

const char *const DarkMoonEngine::_palFilesIntroVGA[] = {
	"PALETTE1.PAL",
	"PALETTE3.PAL",
	"PALETTE2.PAL",
	"PALETTE4.PAL",
	0
};

const char *const DarkMoonEngine::_palFilesIntroEGA[] = {
	"PALETTE0.PAL",
	"PALETTE3.PAL",
	"PALETTE2.PAL",
	"PALETTE4.PAL",
	0
};

const char *const DarkMoonEngine::_palFilesFinaleVGA[] = {
	"FINALE_0.PAL",
	"FINALE_0.PAL",
	"FINALE_1.PAL",
	"FINALE_2.PAL",
	"FINALE_3.PAL",
	"FINALE_4.PAL",
	"FINALE_5.PAL",
	"FINALE_6.PAL",
	"FINALE_7.PAL",
	0
};

const char *const DarkMoonEngine::_palFilesFinaleEGA[] = {
	"FINALE_0.PAL",
	"FINALE_0.PAL",
	"FINALE_1.PAL",
	"FINALE_2.PAL",
	"FINALE_3.PAL",
	"FINALE_4.PAL",
	"FINALE_5.PAL",
	"FINALE_0.PAL",
	"FINALE_0.PAL",
	0
};

const KyraRpgGUISettings DarkMoonEngine::_guiSettings = {
	{ 9, 15, 95, 9, 7, { 221, 76 }, { 189, 162 }, { 95, 95 } },
	{ 186, 181, 183, 133, 184, 17, 23, 20, 186, 181, 183, 182, 177, 180 }
};

const uint8 DarkMoonEngine::_egaDefaultPalette[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

#endif // ENABLE_EOB

} // End of namespace Kyra
