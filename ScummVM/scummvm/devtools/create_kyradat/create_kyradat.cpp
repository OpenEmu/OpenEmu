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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include "create_kyradat.h"

#include "search.h"
#include "pak.h"

#include "tables.h"
#include "extract.h"

#include "md5.h"
#include "common/language.h"
#include "common/platform.h"

#include <string>
#include <map>
#include <algorithm>
#include <map>

enum {
	kKyraDatVersion = 83
};

const ExtractFilename extractFilenames[] = {
	// GENERIC ID MAP
	{ kIdMap, -1, true },

	// INTRO / OUTRO sequences
	{ k1ForestSeq, kTypeForestSeqData, false },
	{ k1KallakWritingSeq, kTypeRawData, false },
	{ k1KyrandiaLogoSeq, kTypeRawData, false },
	{ k1KallakMalcolmSeq, kTypeRawData, false },
	{ k1MalcolmTreeSeq, kTypeRawData, false },
	{ k1WestwoodLogoSeq, kTypeRawData, false },
	{ k1Demo1Seq, kTypeRawData, false },
	{ k1Demo2Seq, kTypeRawData, false },
	{ k1Demo3Seq, kTypeRawData, false },
	{ k1Demo4Seq, kTypeRawData, false },
	{ k1OutroReunionSeq, kTypeRawData, false },

	// INTRO / OUTRO strings
	{ k1IntroCPSStrings, kTypeStringList, false },
	{ k1IntroCOLStrings, kTypeStringList, false },
	{ k1IntroWSAStrings, kTypeStringList, false },
	{ k1IntroStrings, kTypeStringList, true },
	{ k1OutroHomeString, kTypeStringList, true },

	// INGAME strings
	{ k1ItemNames, kTypeStringList, true },
	{ k1TakenStrings, kTypeStringList, true },
	{ k1PlacedStrings, kTypeStringList, true },
	{ k1DroppedStrings, kTypeStringList, true },
	{ k1NoDropStrings, kTypeStringList, true },
	{ k1PutDownString, kTypeStringList, true },
	{ k1WaitAmuletString, kTypeStringList, true },
	{ k1BlackJewelString, kTypeStringList, true },
	{ k1PoisonGoneString, kTypeStringList, true },
	{ k1HealingTipString, kTypeStringList, true },
	{ k1ThePoisonStrings, kTypeStringList, true },
	{ k1FluteStrings, kTypeStringList, true },
	{ k1WispJewelStrings, kTypeStringList, true },
	{ k1MagicJewelStrings, kTypeStringList, true },
	{ k1FlaskFullString, kTypeStringList, true },
	{ k1FullFlaskString, kTypeStringList, true },
	{ k1VeryCleverString, kTypeStringList, true },
	{ k1NewGameString, kTypeStringList, true },

	// GUI strings table
	{ k1GUIStrings, kTypeStringList, true },
	{ k1ConfigStrings, kTypeStringList, true },

	// ROOM table/filenames
	{ k1RoomList, kTypeRoomList, false },
	{ k1RoomFilenames, kTypeStringList, false },

	// SHAPE tables
	{ k1DefaultShapes, kTypeShapeList, false },
	{ k1Healing1Shapes, kTypeShapeList, false },
	{ k1Healing2Shapes, kTypeShapeList, false },
	{ k1PoisonDeathShapes, kTypeShapeList, false },
	{ k1FluteShapes, kTypeShapeList, false },
	{ k1Winter1Shapes, kTypeShapeList, false },
	{ k1Winter2Shapes, kTypeShapeList, false },
	{ k1Winter3Shapes, kTypeShapeList, false },
	{ k1DrinkShapes, kTypeShapeList, false },
	{ k1WispShapes, kTypeShapeList, false },
	{ k1MagicAnimShapes, kTypeShapeList, false },
	{ k1BranStoneShapes, kTypeShapeList, false },

	// IMAGE filename table
	{ k1CharacterImageFilenames, kTypeStringList, false },

	// AUDIO filename table
	{ k1AudioTracks, kTypeStringList, false },
	{ k1AudioTracks2, kTypeStringList, false },
	{ k1AudioTracksIntro, kTypeStringList, false },

	// AMULET anim
	{ k1AmuleteAnimSeq, kTypeRawData, false },

	// PALETTE table
	{ k1SpecialPalette1, kTypeRawData, false },
	{ k1SpecialPalette2, kTypeRawData, false },
	{ k1SpecialPalette3, kTypeRawData, false },
	{ k1SpecialPalette4, kTypeRawData, false },
	{ k1SpecialPalette5, kTypeRawData, false },
	{ k1SpecialPalette6, kTypeRawData, false },
	{ k1SpecialPalette7, kTypeRawData, false },
	{ k1SpecialPalette8, kTypeRawData, false },
	{ k1SpecialPalette9, kTypeRawData, false },
	{ k1SpecialPalette10, kTypeRawData, false },
	{ k1SpecialPalette11, kTypeRawData, false },
	{ k1SpecialPalette12, kTypeRawData, false },
	{ k1SpecialPalette13, kTypeRawData, false },
	{ k1SpecialPalette14, kTypeRawData, false },
	{ k1SpecialPalette15, kTypeRawData, false },
	{ k1SpecialPalette16, kTypeRawData, false },
	{ k1SpecialPalette17, kTypeRawData, false },
	{ k1SpecialPalette18, kTypeRawData, false },
	{ k1SpecialPalette19, kTypeRawData, false },
	{ k1SpecialPalette20, kTypeRawData, false },
	{ k1SpecialPalette21, kTypeRawData, false },
	{ k1SpecialPalette22, kTypeRawData, false },
	{ k1SpecialPalette23, kTypeRawData, false },
	{ k1SpecialPalette24, kTypeRawData, false },
	{ k1SpecialPalette25, kTypeRawData, false },
	{ k1SpecialPalette26, kTypeRawData, false },
	{ k1SpecialPalette27, kTypeRawData, false },
	{ k1SpecialPalette28, kTypeRawData, false },
	{ k1SpecialPalette29, kTypeRawData, false },
	{ k1SpecialPalette30, kTypeRawData, false },
	{ k1SpecialPalette31, kTypeRawData, false },
	{ k1SpecialPalette32, kTypeRawData, false },
	{ k1SpecialPalette33, kTypeRawData, false },

	// CREDITS (used in FM-TOWNS and AMIGA)
	{ k1CreditsStrings, kTypeRawData, true },

	// FM-TOWNS specific
	{ k1TownsMusicFadeTable, k3TypeRaw16to8, false },
	{ k1TownsSFXwdTable, kTypeTownsWDSfxTable, false },
	{ k1TownsSFXbtTable, kTypeRawData, false },
	{ k1TownsCDATable, kTypeRawData, false },

	// PC98 specific
	{ k1PC98StoryStrings, kTypeStringList, true },
	{ k1PC98IntroSfx, kTypeRawData, false },

	// AMIGA specific
	{ k1AmigaIntroSFXTable, kTypeAmigaSfxTable, false },
	{ k1AmigaGameSFXTable, kTypeAmigaSfxTable, false },

	// HAND OF FATE

	// Sequence Player
	{ k2SeqplayPakFiles, kTypeStringList, false },
	{ k2SeqplayCredits, kTypeRawData, false },
	{ k2SeqplayCreditsSpecial, kTypeStringList, false },
	{ k2SeqplayStrings, kTypeStringList, true },
	{ k2SeqplaySfxFiles, k2TypeSoundList, false },
	{ k2SeqplayTlkFiles, k2TypeLangSoundList, true },
	{ k2SeqplaySeqData, k2TypeSeqData, false },
	{ k2SeqplayIntroTracks, kTypeStringList, false },
	{ k2SeqplayFinaleTracks, kTypeStringList, false },
	{ k2SeqplayIntroCDA, kTypeRawData, false },
	{ k2SeqplayFinaleCDA, kTypeRawData, false },
	{ k2SeqplayShapeAnimData, k2TypeShpDataV1, false },

	// Ingame
	{ k2IngamePakFiles, kTypeStringList, false },
	{ k2IngameSfxFiles, k2TypeSize10StringList, false },
	{ k2IngameSfxIndex, kTypeRawData, false },
	{ k2IngameTracks, kTypeStringList, false },
	{ k2IngameCDA, kTypeRawData, false },
	{ k2IngameTalkObjIndex, kTypeRawData, false },
	{ k2IngameTimJpStrings, kTypeStringList, false },
	{ k2IngameShapeAnimData, k2TypeShpDataV2, false },
	{ k2IngameTlkDemoStrings, kTypeStringList, true },


	// MALCOLM'S REVENGE
	{ k3MainMenuStrings, kTypeStringList, false },
	{ k3MusicFiles, k2TypeSoundList, false },
	{ k3ScoreTable, kTypeRawData, false },
	{ k3SfxFiles, k2TypeSfxList, false },
	{ k3SfxMap, k3TypeRaw16to8, false },
	{ k3ItemAnimData, k3TypeShpData, false },
	{ k3ItemMagicTable, k3TypeRaw16to8, false },
	{ k3ItemStringMap, kTypeRawData, false },

	// EYE OF THE BEHOLDER COMMON
	{ kEoBBaseChargenStrings1, kTypeStringList, true },
	{ kEoBBaseChargenStrings2, kTypeStringList, true },
	{ kEoBBaseChargenStartLevels, kTypeRawData, false },
	{ kEoBBaseChargenStatStrings, kTypeStringList, true},
	{ kEoBBaseChargenRaceSexStrings, kTypeStringList, true },
	{ kEoBBaseChargenClassStrings, kTypeStringList, true },
	{ kEoBBaseChargenAlignmentStrings, kTypeStringList, true },
	{ kEoBBaseChargenEnterGameStrings, kTypeStringList, true },
	{ kEoBBaseChargenClassMinStats, k3TypeRaw16to8, false },
	{ kEoBBaseChargenRaceMinStats, k3TypeRaw16to8, false },
	{ kEoBBaseChargenRaceMaxStats, kLoLTypeRaw16, false },

	{ kEoBBaseSaveThrowTable1, kTypeRawData, false },
	{ kEoBBaseSaveThrowTable2, kTypeRawData, false },
	{ kEoBBaseSaveThrowTable3, kTypeRawData, false },
	{ kEoBBaseSaveThrowTable4, kTypeRawData, false },
	{ kEoBBaseSaveThrwLvlIndex, kTypeRawData, false },
	{ kEoBBaseSaveThrwModDiv, kTypeRawData, false },
	{ kEoBBaseSaveThrwModExt, kTypeRawData, false },

	{ kEoBBasePryDoorStrings, kTypeStringList, true },
	{ kEoBBaseWarningStrings, kTypeStringList, true },

	{ kEoBBaseItemSuffixStringsRings, kTypeStringList, true },
	{ kEoBBaseItemSuffixStringsPotions, kTypeStringList, true },
	{ kEoBBaseItemSuffixStringsWands, kTypeStringList, true },

	{ kEoBBaseRipItemStrings, kTypeStringList, true },
	{ kEoBBaseCursedString, kTypeStringList, true },
	{ kEoBBaseEnchantedString, kTypeStringList, false },
	{ kEoBBaseMagicObjectStrings, kTypeStringList, true },
	{ kEoBBaseMagicObject5String, kTypeStringList, true },
	{ kEoBBasePatternSuffix, kTypeStringList, true },
	{ kEoBBasePatternGrFix1, kTypeStringList, true },
	{ kEoBBasePatternGrFix2, kTypeStringList, true },
	{ kEoBBaseValidateArmorString, kTypeStringList, true },
	{ kEoBBaseValidateCursedString, kTypeStringList, true },
	{ kEoBBaseValidateNoDropString, kTypeStringList, true },
	{ kEoBBasePotionStrings, kTypeStringList, true },
	{ kEoBBaseWandString, kTypeStringList, true },
	{ kEoBBaseItemMisuseStrings, kTypeStringList, true },

	{ kEoBBaseTakenStrings, kTypeStringList, true },
	{ kEoBBasePotionEffectStrings, kTypeStringList, true },

	{ kEoBBaseYesNoStrings, kTypeStringList, true },
	{ kRpgCommonMoreStrings, kTypeStringList, true },
	{ kEoBBaseNpcMaxStrings, kTypeStringList, true },
	{ kEoBBaseOkStrings, kTypeStringList, true },
	{ kEoBBaseNpcJoinStrings, kTypeStringList, true },
	{ kEoBBaseCancelStrings, kTypeStringList, true },
	{ kEoBBaseAbortStrings, kTypeStringList, true },

	{ kEoBBaseMenuStringsMain, kTypeStringList, true },
	{ kEoBBaseMenuStringsSaveLoad, kTypeStringList, true },
	{ kEoBBaseMenuStringsOnOff, kTypeStringList, true },
	{ kEoBBaseMenuStringsSpells, kTypeStringList, true },
	{ kEoBBaseMenuStringsRest, kTypeStringList, true },
	{ kEoBBaseMenuStringsDrop, kTypeStringList, true },
	{ kEoBBaseMenuStringsExit, kTypeStringList, true },
	{ kEoBBaseMenuStringsStarve, kTypeStringList, true },
	{ kEoBBaseMenuStringsScribe, kTypeStringList, true },
	{ kEoBBaseMenuStringsDrop2, kTypeStringList, true },
	{ kEoBBaseMenuStringsHead, kTypeStringList, true },
	{ kEoBBaseMenuStringsPoison, kTypeStringList, true },
	{ kEoBBaseMenuStringsMgc, kTypeStringList, true },
	{ kEoBBaseMenuStringsPrefs, kTypeStringList, true },
	{ kEoBBaseMenuStringsRest2, kTypeStringList, true },
	{ kEoBBaseMenuStringsRest3, kTypeStringList, true },
	{ kEoBBaseMenuStringsRest4, kTypeStringList, true },
	{ kEoBBaseMenuStringsDefeat, kTypeStringList, true },
	{ kEoBBaseMenuStringsTransfer, kTypeStringList, true },
	{ kEoBBaseMenuStringsSpec, kTypeStringList, true },
	{ kEoBBaseMenuStringsSpellNo, kTypeStringList, false },
	{ kEoBBaseMenuYesNoStrings, kTypeStringList, true },

	{ kEoBBaseSpellLevelsMage, kTypeRawData, false },
	{ kEoBBaseSpellLevelsCleric, kTypeRawData, false },
	{ kEoBBaseNumSpellsCleric, kTypeRawData, false },
	{ kEoBBaseNumSpellsWisAdj, kTypeRawData, false },
	{ kEoBBaseNumSpellsPal, kTypeRawData, false },
	{ kEoBBaseNumSpellsMage, kTypeRawData, false },

	{ kEoBBaseCharGuiStringsHp, kTypeStringList, true },
	{ kEoBBaseCharGuiStringsWp1, kTypeStringList, true },
	{ kEoBBaseCharGuiStringsWp2, kTypeStringList, true },
	{ kEoBBaseCharGuiStringsWr, kTypeStringList, true },
	{ kEoBBaseCharGuiStringsSt1, kTypeStringList, true },
	{ kEoBBaseCharGuiStringsSt2, kTypeStringList, true },
	{ kEoBBaseCharGuiStringsIn, kTypeStringList, true },

	{ kEoBBaseCharStatusStrings7, kTypeStringList, true },
	{ kEoBBaseCharStatusStrings81, kTypeStringList, true },
	{ kEoBBaseCharStatusStrings82, kTypeStringList, true },
	{ kEoBBaseCharStatusStrings9, kTypeStringList, true },
	{ kEoBBaseCharStatusStrings12, kTypeStringList, true },
	{ kEoBBaseCharStatusStrings131, kTypeStringList, true },
	{ kEoBBaseCharStatusStrings132, kTypeStringList, true },

	{ kEoBBaseLevelGainStrings, kTypeStringList, true },
	{ kEoBBaseExperienceTable0, kLoLTypeRaw32, false },
	{ kEoBBaseExperienceTable1, kLoLTypeRaw32, false },
	{ kEoBBaseExperienceTable2, kLoLTypeRaw32, false },
	{ kEoBBaseExperienceTable3, kLoLTypeRaw32, false },
	{ kEoBBaseExperienceTable4, kLoLTypeRaw32, false },

	{ kEoBBaseWllFlagPreset, kTypeRawData, false },
	{ kEoBBaseDscShapeCoords, kLoLTypeRaw16, false },
	{ kEoBBaseDscDoorScaleOffs, kTypeRawData, false },
	{ kEoBBaseDscDoorScaleMult1, kTypeRawData, false },
	{ kEoBBaseDscDoorScaleMult2, kTypeRawData, false },
	{ kEoBBaseDscDoorScaleMult3, kTypeRawData, false },
	{ kEoBBaseDscDoorScaleMult4, kTypeRawData, false },
	{ kEoBBaseDscDoorScaleMult5, kTypeRawData, false },
	{ kEoBBaseDscDoorScaleMult6, kTypeRawData, false },
	{ kEoBBaseDscDoorType5Offs, kTypeRawData, false },
	{ kEoBBaseDscDoorXE, kTypeRawData, false },
	{ kEoBBaseDscDoorY1, kTypeRawData, false },
	{ kEoBBaseDscDoorY3, kTypeRawData, false },
	{ kEoBBaseDscDoorY4, kTypeRawData, false },
	{ kEoBBaseDscDoorY5, kTypeRawData, false },
	{ kEoBBaseDscDoorY6, kTypeRawData, false },
	{ kEoBBaseDscDoorY7, kTypeRawData, false },
	{ kEoBBaseDscDoorCoordsExt, kLoLTypeRaw16, false },

	{ kEoBBaseDscItemPosIndex, kTypeRawData, false },
	{ kEoBBaseDscItemShpX, kLoLTypeRaw16, false },
	{ kEoBBaseDscItemPosUnk, kTypeRawData, false },
	{ kEoBBaseDscItemTileIndex, kTypeRawData, false },
	{ kEoBBaseDscItemShapeMap, kTypeRawData, false },
	{ kEoBBaseDscTelptrShpCoords, kTypeRawData, false },

	{ kEoBBasePortalSeqData, kTypeRawData, false },
	{ kEoBBaseManDef, kTypeRawData, true },
	{ kEoBBaseManWord, kTypeStringList, true },
	{ kEoBBaseManPrompt, kTypeStringList, true },

	{ kEoBBaseDscMonsterFrmOffsTbl1, kTypeRawData, false },
	{ kEoBBaseDscMonsterFrmOffsTbl2, kTypeRawData, false },

	{ kEoBBaseInvSlotX, kLoLTypeRaw16, false },
	{ kEoBBaseInvSlotY, kTypeRawData, false },
	{ kEoBBaseSlotValidationFlags, kLoLTypeRaw16, false },

	{ kEoBBaseProjectileWeaponTypes, kTypeRawData, false },
	{ kEoBBaseWandTypes, kTypeRawData, false },

	{ kEoBBaseDrawObjPosIndex, kTypeRawData, false },
	{ kEoBBaseFlightObjFlipIndex, kTypeRawData, false },
	{ kEoBBaseFlightObjShpMap, kTypeRawData, false },
	{ kEoBBaseFlightObjSclIndex, kTypeRawData, false },

	{ kEoBBaseBookNumbers, kTypeStringList, true },
	{ kEoBBaseMageSpellsList, kTypeStringList, true },
	{ kEoBBaseClericSpellsList, kTypeStringList, true },
	{ kEoBBaseSpellNames, kTypeStringList, true },

	{ kEoBBaseMagicStrings1, kTypeStringList, true },
	{ kEoBBaseMagicStrings2, kTypeStringList, true },
	{ kEoBBaseMagicStrings3, kTypeStringList, true },
	{ kEoBBaseMagicStrings4, kTypeStringList, true },
	{ kEoBBaseMagicStrings6, kTypeStringList, true },
	{ kEoBBaseMagicStrings7, kTypeStringList, true },
	{ kEoBBaseMagicStrings8, kTypeStringList, true },

	{ kEoBBaseExpObjectTlMode, kTypeRawData, false },
	{ kEoBBaseExpObjectTblIndex, kTypeRawData, false },
	{ kEoBBaseExpObjectShpStart, kTypeRawData, false },
	{ kEoBBaseExpObjectTbl1, kTypeRawData, false },
	{ kEoBBaseExpObjectTbl2, kTypeRawData, false },
	{ kEoBBaseExpObjectTbl3, kTypeRawData, false },
	{ kEoBBaseExpObjectY, k3TypeRaw16to8, false },

	{ kEoBBaseSparkDefSteps, kTypeRawData, false },
	{ kEoBBaseSparkDefSubSteps, kTypeRawData, false },
	{ kEoBBaseSparkDefShift, kTypeRawData, false },
	{ kEoBBaseSparkDefAdd, kTypeRawData, false },
	{ kEoBBaseSparkDefX, k3TypeRaw16to8, false },
	{ kEoBBaseSparkDefY, kTypeRawData, false },
	{ kEoBBaseSparkOfFlags1, kLoLTypeRaw32, false },
	{ kEoBBaseSparkOfFlags2, kLoLTypeRaw32, false },
	{ kEoBBaseSparkOfShift, kTypeRawData, false },
	{ kEoBBaseSparkOfX, kTypeRawData, false },
	{ kEoBBaseSparkOfY, kTypeRawData, false },
	{ kEoBBaseSpellProperties, kTypeRawData, false },
	{ kEoBBaseMagicFlightProps, kTypeRawData, false },
	{ kEoBBaseTurnUndeadEffect, kTypeRawData, false },
	{ kEoBBaseBurningHandsDest, kTypeRawData, false },
	{ kEoBBaseConeOfColdDest1, kTypeRawData, false },
	{ kEoBBaseConeOfColdDest2, kTypeRawData, false },
	{ kEoBBaseConeOfColdDest3, kTypeRawData, false },
	{ kEoBBaseConeOfColdDest4, kTypeRawData, false },
	{ kEoBBaseConeOfColdGfxTbl, k3TypeRaw16to8, false },

	// EYE OF THE BEHOLDER I
	{ kEoB1MainMenuStrings, kTypeStringList, true },
	{ kEoB1BonusStrings, kTypeStringList, true },

	{ kEoB1IntroFilesOpening, kTypeStringList, false },
	{ kEoB1IntroFilesTower, kTypeStringList, false },
	{ kEoB1IntroFilesOrb, kTypeStringList, false },
	{ kEoB1IntroFilesWdEntry, kTypeStringList, false },
	{ kEoB1IntroFilesKing, kTypeStringList, false },
	{ kEoB1IntroFilesHands, kTypeStringList, false },
	{ kEoB1IntroFilesWdExit, kTypeStringList, false },
	{ kEoB1IntroFilesTunnel, kTypeStringList, false },
	{ kEoB1IntroOpeningFrmDelay, k3TypeRaw16to8, false },
	{ kEoB1IntroWdEncodeX, kTypeRawData, false },
	{ kEoB1IntroWdEncodeY, kTypeRawData, false },
	{ kEoB1IntroWdEncodeWH, kTypeRawData, false },
	{ kEoB1IntroWdDsX, kLoLTypeRaw16, false },
	{ kEoB1IntroWdDsY, kTypeRawData, false },
	{ kEoB1IntroTvlX1, kTypeRawData, false },
	{ kEoB1IntroTvlY1, kTypeRawData, false },
	{ kEoB1IntroTvlX2, kTypeRawData, false },
	{ kEoB1IntroTvlY2, kTypeRawData, false },
	{ kEoB1IntroTvlW, kTypeRawData, false },
	{ kEoB1IntroTvlH, kTypeRawData, false },

	{ kEoB1DoorShapeDefs, kTypeRawData, false },
	{ kEoB1DoorSwitchShapeDefs, kTypeRawData, false },
	{ kEoB1DoorSwitchCoords, kTypeRawData, false },
	{ kEoB1MonsterProperties, kTypeRawData, false },
	{ kEoB1EnemyMageSpellList, kTypeRawData, false },
	{ kEoB1EnemyMageSfx, kTypeRawData, false },
	{ kEoB1BeholderSpellList, kTypeRawData, false },
	{ kEoB1BeholderSfx, kTypeRawData, false },
	{ kEoB1TurnUndeadString, kTypeStringList, true },

	{ kEoB1CgaMappingDefault, kTypeRawData, false },
	{ kEoB1CgaMappingAlt, kTypeRawData, false },
	{ kEoB1CgaMappingInv, kTypeRawData, false },
	{ kEoB1CgaMappingItemsL, kTypeRawData, false },
	{ kEoB1CgaMappingItemsS, kTypeRawData, false },
	{ kEoB1CgaMappingThrown, kTypeRawData, false },
	{ kEoB1CgaMappingIcons, kTypeRawData, false },
	{ kEoB1CgaMappingDeco, kTypeRawData, false },
	{ kEoB1CgaLevelMappingIndex, kTypeRawData, false },
	{ kEoB1CgaMappingLevel0, kTypeRawData, false },
	{ kEoB1CgaMappingLevel1, kTypeRawData, false },
	{ kEoB1CgaMappingLevel2, kTypeRawData, false },
	{ kEoB1CgaMappingLevel3, kTypeRawData, false },
	{ kEoB1CgaMappingLevel4, kTypeRawData, false },

	{ kEoB1NpcShpData, kTypeRawData, false },
	{ kEoB1NpcSubShpIndex1, kTypeRawData, false },
	{ kEoB1NpcSubShpIndex2, kTypeRawData, false },
	{ kEoB1NpcSubShpY, kTypeRawData, false },
	{ kEoB1Npc0Strings, kTypeStringList, true },
	{ kEoB1Npc11Strings, kTypeStringList, true },
	{ kEoB1Npc12Strings, kTypeStringList, true },
	{ kEoB1Npc21Strings, kTypeStringList, true },
	{ kEoB1Npc22Strings, kTypeStringList, true },
	{ kEoB1Npc31Strings, kTypeStringList, true },
	{ kEoB1Npc32Strings, kTypeStringList, true },
	{ kEoB1Npc4Strings, kTypeStringList, true },
	{ kEoB1Npc5Strings, kTypeStringList, true },
	{ kEoB1Npc6Strings, kTypeStringList, true },
	{ kEoB1Npc7Strings, kTypeStringList, true },

	// EYE OF THE BEHOLDER II
	{ kEoB2MainMenuStrings, kTypeStringList, true },

	{ kEoB2TransferPortraitFrames, kLoLTypeRaw16, false },
	{ kEoB2TransferConvertTable, kTypeRawData, false },
	{ kEoB2TransferItemTable, kTypeRawData, false },
	{ kEoB2TransferExpTable, kLoLTypeRaw32, false },
	{ kEoB2TransferStrings1, kTypeStringList, true },
	{ kEoB2TransferStrings2, kTypeStringList, true },
	{ kEoB2TransferLabels, kTypeStringList, true },

	{ kEoB2IntroStrings, k2TypeSfxList, true },
	{ kEoB2IntroCPSFiles, kTypeStringList, true },
	{ kEob2IntroAnimData00, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData01, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData02, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData03, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData04, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData05, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData06, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData07, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData08, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData09, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData10, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData11, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData12, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData13, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData14, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData15, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData16, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData17, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData18, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData19, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData20, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData21, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData22, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData23, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData24, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData25, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData26, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData27, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData28, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData29, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData30, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData31, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData32, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData33, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData34, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData35, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData36, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData37, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData38, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData39, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData40, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData41, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData42, kEoB2TypeSeqData, false },
	{ kEob2IntroAnimData43, kEoB2TypeSeqData, false },
	{ kEoB2IntroShapes00, kEoB2TypeShapeData, false },
	{ kEoB2IntroShapes01, kEoB2TypeShapeData, false },
	{ kEoB2IntroShapes04, kEoB2TypeShapeData, false },
	{ kEoB2IntroShapes07, kEoB2TypeShapeData, false },

	{ kEoB2FinaleStrings, k2TypeSfxList, true },
	{ kEoB2CreditsData, kTypeRawData, true },
	{ kEoB2FinaleCPSFiles, kTypeStringList, true },
	{ kEob2FinaleAnimData00, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData01, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData02, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData03, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData04, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData05, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData06, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData07, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData08, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData09, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData10, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData11, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData12, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData13, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData14, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData15, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData16, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData17, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData18, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData19, kEoB2TypeSeqData, false },
	{ kEob2FinaleAnimData20, kEoB2TypeSeqData, false },
	{ kEoB2FinaleShapes00, kEoB2TypeShapeData, false },
	{ kEoB2FinaleShapes03, kEoB2TypeShapeData, false },
	{ kEoB2FinaleShapes07, kEoB2TypeShapeData, false },
	{ kEoB2FinaleShapes09, kEoB2TypeShapeData, false },
	{ kEoB2FinaleShapes10, kEoB2TypeShapeData, false },
	{ kEoB2NpcShapeData, kTypeRawData, false },
	{ kEoBBaseClassModifierFlags, kTypeRawData, false },
	{ kEoBBaseMonsterStepTable01, kTypeRawData, false },
	{ kEoBBaseMonsterStepTable02, kTypeRawData, false },
	{ kEoBBaseMonsterStepTable1, kTypeRawData, false },
	{ kEoBBaseMonsterStepTable2, k3TypeRaw16to8, false },
	{ kEoBBaseMonsterStepTable3, k3TypeRaw16to8, false },
	{ kEoBBaseMonsterCloseAttPosTable1, kTypeRawData, false },
	{ kEoBBaseMonsterCloseAttPosTable21, kTypeRawData, false },
	{ kEoBBaseMonsterCloseAttPosTable22, kTypeRawData, false },
	{ kEoBBaseMonsterCloseAttUnkTable, kTypeRawData, false },
	{ kEoBBaseMonsterCloseAttChkTable1, kTypeRawData, false },
	{ kEoBBaseMonsterCloseAttChkTable2, kTypeRawData, false },
	{ kEoBBaseMonsterCloseAttDstTable1, kTypeRawData, false },
	{ kEoBBaseMonsterCloseAttDstTable2, kTypeRawData, false },
	{ kEoBBaseMonsterProximityTable, kTypeRawData, false },
	{ kEoBBaseFindBlockMonstersTable, kTypeRawData, false },
	{ kEoBBaseMonsterDirChangeTable, kTypeRawData, false },
	{ kEoBBaseMonsterDistAttStrings, kTypeStringList, true },
	{ kEoBBaseEncodeMonsterDefs, kLoLTypeRaw16, false },
	{ kEoBBaseNpcPresets, kEoBTypeNpcData, false },
	{ kEoB2Npc1Strings, kTypeStringList, true },
	{ kEoB2Npc2Strings, kTypeStringList, true },
	{ kEoB2MonsterDustStrings, kTypeStringList, true },
	{ kEoB2DreamSteps, kTypeRawData, false },
	{ kEoB2KheldranStrings, kTypeStringList, true },
	{ kEoB2HornStrings, kTypeStringList, true },
	{ kEoB2HornSounds, kTypeRawData, false },
	{ kEoB2WallOfForceDsX, kLoLTypeRaw16, false },
	{ kEoB2WallOfForceDsY, kTypeRawData, false },
	{ kEoB2WallOfForceNumW, kTypeRawData, false },
	{ kEoB2WallOfForceNumH, kTypeRawData, false },
	{ kEoB2WallOfForceShpId, kTypeRawData, false },

	// LANDS OF LORE

	// Ingame
	{ kLoLIngamePakFiles, kTypeStringList, false },

	{ kLoLCharacterDefs, kLoLTypeCharData, true },
	{ kLoLIngameSfxFiles, k2TypeSfxList, false },
	{ kLoLIngameSfxIndex, kTypeRawData, false },
	{ kLoLMusicTrackMap, kTypeRawData, false },
	{ kLoLIngameGMSfxIndex, kTypeRawData, false },
	{ kLoLIngameMT32SfxIndex, kTypeRawData, false },
	{ kLoLIngamePcSpkSfxIndex, kTypeRawData, false },
	{ kLoLSpellProperties, kLoLTypeSpellData, false },
	{ kLoLGameShapeMap, kTypeRawData, false },
	{ kLoLSceneItemOffs, kTypeRawData, false },
	{ kLoLCharInvIndex, k3TypeRaw16to8, false },
	{ kLoLCharInvDefs, kTypeRawData, false },
	{ kLoLCharDefsMan, kLoLTypeRaw16, false },
	{ kLoLCharDefsWoman, kLoLTypeRaw16, false },
	{ kLoLCharDefsKieran, kLoLTypeRaw16, false },
	{ kLoLCharDefsAkshel, kLoLTypeRaw16, false },
	{ kLoLExpRequirements, kLoLTypeRaw32, false },
	{ kLoLMonsterModifiers, kLoLTypeRaw16, false },
	{ kLoLMonsterShiftOffsets, kTypeRawData, false },
	{ kLoLMonsterDirFlags, kTypeRawData, false },
	{ kLoLMonsterScaleY, kTypeRawData, false },
	{ kLoLMonsterScaleX, kTypeRawData, false },
	{ kLoLMonsterScaleWH, kLoLTypeRaw16, false },
	{ kLoLFlyingObjectShp, kLoLTypeFlightShpData, false },
	{ kLoLInventoryDesc, kLoLTypeRaw16, false },
	{ kLoLLevelShpList, kTypeStringList, false },
	{ kLoLLevelDatList, kTypeStringList, false },
	{ kLoLCompassDefs, kLoLTypeCompassData, false },
	{ kLoLItemPrices, kLoLTypeRaw16, false },
	{ kLoLStashSetup, kTypeRawData, false },

	{ kLoLDscWalls, kTypeRawData, false },
	{ kRpgCommonDscShapeIndex, kTypeRawData, false },
	{ kLoLDscOvlMap, kTypeRawData, false },
	{ kLoLDscScaleWidthData, kLoLTypeRaw16, false },
	{ kLoLDscScaleHeightData, kLoLTypeRaw16, false },
	{ kRpgCommonDscX, kLoLTypeRaw16, false },
	{ kLoLDscY, kTypeRawData, false },
	{ kRpgCommonDscTileIndex, kTypeRawData, false },
	{ kRpgCommonDscUnk2, kTypeRawData, false },
	{ kRpgCommonDscDoorShapeIndex, kTypeRawData, false },
	{ kRpgCommonDscDimData1, kTypeRawData, false },
	{ kRpgCommonDscDimData2, kTypeRawData, false },
	{ kRpgCommonDscBlockMap, kTypeRawData, false },
	{ kRpgCommonDscDimMap, kTypeRawData, false },
	{ kLoLDscDoorScale, kLoLTypeRaw16, false },
	{ kLoLDscOvlIndex, k3TypeRaw16to8, false },
	{ kRpgCommonDscBlockIndex, kTypeRawData, false },
	{ kLoLDscDoor4, kLoLTypeRaw16, false },
	{ kRpgCommonDscDoorY2, kTypeRawData, false },
	{ kRpgCommonDscDoorFrameY1, kTypeRawData, false },
	{ kRpgCommonDscDoorFrameY2, kTypeRawData, false },
	{ kRpgCommonDscDoorFrameIndex1, kTypeRawData, false },
	{ kRpgCommonDscDoorFrameIndex2, kTypeRawData, false },
	{ kLoLDscDoorX, kLoLTypeRaw16, false },
	{ kLoLDscDoorY, kLoLTypeRaw16, false },

	{ kLoLScrollXTop, k3TypeRaw16to8, false },
	{ kLoLScrollYTop, k3TypeRaw16to8, false },
	{ kLoLScrollXBottom, k3TypeRaw16to8, false },
	{ kLoLScrollYBottom, k3TypeRaw16to8, false },

	{ kLoLButtonDefs, kLoLTypeButtonDef, false },
	{ kLoLButtonList1, kLoLTypeRaw16, false },
	{ kLoLButtonList2, kLoLTypeRaw16, false },
	{ kLoLButtonList3, kLoLTypeRaw16, false },
	{ kLoLButtonList4, kLoLTypeRaw16, false },
	{ kLoLButtonList5, kLoLTypeRaw16, false },
	{ kLoLButtonList6, kLoLTypeRaw16, false },
	{ kLoLButtonList7, kLoLTypeRaw16, false },
	{ kLoLButtonList8, kLoLTypeRaw16, false },

	{ kLoLLegendData, kTypeRawData, false },
	{ kLoLMapCursorOvl, kTypeRawData, false },
	{ kLoLMapStringId, kLoLTypeRaw16, false },

	{ kLoLSpellbookAnim, k3TypeRaw16to8, false },
	{ kLoLSpellbookCoords, k3TypeRaw16to8, false },
	{ kLoLHealShapeFrames, kTypeRawData, false },
	{ kLoLLightningDefs, kTypeRawData, false },
	{ kLoLFireballCoords, kLoLTypeRaw16, false },

	{ kLoLCredits, kTypeRawData, false },

	{ kLoLHistory, kTypeRawData, false },

	{ -1, 0, 0 }
};

const ExtractFilename *getFilenameDesc(const int id) {
	for (const ExtractFilename *i = extractFilenames; i->id != -1; ++i) {
		if (i->id == id)
			return i;
	}
	return 0;
}

bool isLangSpecific(const int id) {
	const ExtractFilename *desc = getFilenameDesc(id);
	if (!desc)
		return false;
	return desc->langSpecific;
}

// misc tables

const TypeTable gameTable[] = {
	{ kKyra1, 0 },
	{ kKyra2, 1 },
	{ kKyra3, 2 },
	{ kEoB1, 3 },
	{ kEoB2, 4 },
	{ kLoL, 5 },
	{ -1, -1 }
};

byte getGameID(int game) {
	return std::find(gameTable, ARRAYEND(gameTable) - 1, game)->value;
}

const TypeTable languageTable[] = {
	{ UNK_LANG, 0 },
	{ EN_ANY, 1 },
	{ FR_FRA, 2 },
	{ DE_DEU, 3 },
	{ ES_ESP, 4 },
	{ IT_ITA, 5 },
	{ JA_JPN, 6 },
	{ RU_RUS, 7 },
	{ -1, -1 }
};

byte getLanguageID(int lang) {
	return std::find(languageTable, ARRAYEND(languageTable) - 1, lang)->value;
}

const TypeTable platformTable[] = {
	{ kPlatformPC, 0 },
	{ kPlatformAmiga, 1 },
	{ kPlatformFMTowns, 2 },
	{ kPlatformPC98, 3 },
	{ kPlatformMacintosh, 4 },
	{ -1, -1 }
};

byte getPlatformID(int platform) {
	return std::find(platformTable, ARRAYEND(platformTable) - 1, platform)->value;
}

const TypeTable specialTable[] = {
	{ kNoSpecial, 0 },
	{ kTalkieVersion, 1 },
	{ kDemoVersion, 2 },
	{ kTalkieDemoVersion, 3 },
	{ kOldFloppy, 4 },
	{ -1, -1 }
};

byte getSpecialID(int special) {
	return std::find(specialTable, ARRAYEND(specialTable) - 1, special)->value;
}

// filename processing

uint32 getFilename(const ExtractInformation *info, const int id) {
	const ExtractFilename *fDesc = getFilenameDesc(id);

	if (!fDesc)
		return 0;

	// GAME, PLATFORM, SPECIAL, ID, LANG
	return ((getGameID(info->game) & 0xF) << 24) |
	       ((getPlatformID(info->platform) & 0xF) << 20) |
	       ((getSpecialID(info->special) & 0xF) << 16) |
	       ((id & 0xFFF) << 4) |
	       ((getLanguageID(fDesc->langSpecific ? info->lang : UNK_LANG) & 0xF) << 0);
}

// TODO: Get rid of this
bool getFilename(char *dstFilename, const ExtractInformation *info, const int id) {
	sprintf(dstFilename, "%08X", getFilename(info, id));
	return true;
}

// index generation

typedef uint16 GameDef;

GameDef createGameDef(const ExtractInformation *eI) {
	return ((getGameID(eI->game) & 0xF) << 12) |
	       ((getPlatformID(eI->platform) & 0xF) << 8) |
	       ((getSpecialID(eI->special) & 0xF) << 4) |
	       ((getLanguageID(eI->lang) & 0xF) << 0);
}

struct Index {
	Index() : version(0), includedGames(0), gameList() {}

	uint32 version;
	uint32 includedGames;

	typedef std::list<GameDef> GameList;
	GameList gameList;
};

Index parseIndex(const uint8 *data, uint32 size) {
	Index result;

	if (size < 8)
		return result;

	result.version = READ_BE_UINT32(data); data += 4;
	result.includedGames = READ_BE_UINT32(data); data += 4;

	if (result.includedGames * 2 + 8 != size) {
		result.version = result.includedGames = 0;
		return result;
	}

	for (uint32 i = 0; i < result.includedGames; ++i) {
		GameDef game = READ_BE_UINT16(data); data += 2;
		result.gameList.push_back(game);
	}

	return result;
}

bool updateIndex(PAKFile &out, const ExtractInformation *eI) {
	uint32 size = 0;
	const uint8 *data = out.getFileData("INDEX", &size);

	Index index;
	if (data)
		index = parseIndex(data, size);

	GameDef gameDef = createGameDef(eI);
	if (index.version == kKyraDatVersion) {
		if (std::find(index.gameList.begin(), index.gameList.end(), gameDef) == index.gameList.end()) {
			++index.includedGames;
			index.gameList.push_back(gameDef);
		} else {
			// Already included in the game list, thus we do not need any further processing here.
			return true;
		}
	} else {
		index.version = kKyraDatVersion;
		index.includedGames = 1;
		index.gameList.push_back(gameDef);
	}

	const uint32 indexBufferSize = 8 + index.includedGames * 2;
	uint8 *indexBuffer = new uint8[indexBufferSize];
	assert(indexBuffer);
	uint8 *dst = indexBuffer;
	WRITE_BE_UINT32(dst, index.version); dst += 4;
	WRITE_BE_UINT32(dst, index.includedGames); dst += 4;
	for (Index::GameList::const_iterator i = index.gameList.begin(); i != index.gameList.end(); ++i) {
		WRITE_BE_UINT16(dst, *i); dst += 2;
	}

	out.removeFile("INDEX");
	if (!out.addFile("INDEX", indexBuffer, indexBufferSize)) {
		fprintf(stderr, "ERROR: couldn't update kyra.dat INDEX\n");
		delete[] indexBuffer;
		return false;
	}

	return true;
}

bool checkIndex(PAKFile &file) {
	uint32 size = 0;
	const uint8 *data = file.getFileData("INDEX", &size);
	if (!data)
		return false;

	Index index = parseIndex(data, size);

	if (index.version != kKyraDatVersion)
		return false;
	if (index.includedGames * 2 + 8 != size)
		return false;

	return true;
}

// main processing

void printHelp(const char *f) {
	printf("Usage:\n");
	printf("%s output inputfiles ...\n", f);
}

bool process(PAKFile &out, const Game *g, const byte *data, const uint32 size);
const Game *findGame(const byte *buffer, const uint32 size);

typedef std::map<std::string, std::string> MD5Map;
MD5Map createMD5Sums(int files, const char * const *filenames);

struct File {
	File() : data(0), size(0) {}
	File(uint8 *d, uint32 s) : data(d), size(s) {}

	uint8 *data;
	uint32 size;
};
typedef std::map<const Game *, File> GameMap;
GameMap createGameMap(const MD5Map &map);

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printHelp(argv[0]);
		return -1;
	}

	// Special case for developer mode of this tool:
	// With "--create filename offset size" the tool will output
	// a search entry for the specifed data in the specified file.
	if (!strcmp(argv[1], "--create")) {
		if (argc < 5) {
			printf("Developer usage: %s --create input_file hex_offset hex_size\n", argv[0]);
			return -1;
		}

		uint32 offset, size;
		sscanf(argv[3], "%x", &offset);
		sscanf(argv[4], "%x", &size);

		FILE *input = fopen(argv[2], "rb");
		if (!input)
			error("Couldn't open file '%s'", argv[2]);

		byte *buffer = new byte[size];
		fseek(input, offset, SEEK_SET);
		if (fread(buffer, 1, size, input) != size) {
			delete[] buffer;
			error("Couldn't read from file '%s'", argv[2]);
		}

		fclose(input);

		SearchData d = SearchCreator::create(buffer, size);
		delete[] buffer;

		printf("{ 0x%.08X, 0x%.08X, { {", d.size, d.byteSum);
		for (int j = 0; j < 16; ++j) {
			printf(" 0x%.2X", d.hash.digest[j]);
			if (j != 15)
				printf(",");
			else
				printf(" } } }\n");
		}

		return 0;
	}

	PAKFile out;
	out.loadFile(argv[1], false);

	// When the output file is no valid kyra.dat file, we will delete
	// all the output.
	if (!checkIndex(out))
		out.clearFile();

	MD5Map inputFiles = createMD5Sums(argc - 2, &argv[2]);

	GameMap games = createGameMap(inputFiles);

	// Check for unused input files
	MD5Map unusedFiles = inputFiles;
	for (GameMap::const_iterator i = games.begin(); i != games.end(); ++i) {
		unusedFiles.erase(i->first->md5[0]);
		if (i->first->md5[1])
			unusedFiles.erase(i->first->md5[1]);
	}

	for (MD5Map::const_iterator i = unusedFiles.begin(); i != unusedFiles.end(); ++i)
		printf("Input file '%s' with md5 sum '%s' is not known.\n", i->second.c_str(), i->first.c_str());

	unusedFiles.clear();

	// Short circuit, in case no games are found.
	if (games.empty()) {
		printf("No games found. Exiting prematurely\n");
		return -1;
	}

	// Process all games found
	for (GameMap::const_iterator i = games.begin(); i != games.end(); ++i) {
		MD5Map::const_iterator f1 = inputFiles.find(i->first->md5[0]);
		MD5Map::const_iterator f2 = inputFiles.end();
		if (i->first->md5[1])
			f2 = inputFiles.find(i->first->md5[1]);

		if (f2 != inputFiles.end())
			printf("Processing files '%s' and '%s'...\n", f1->second.c_str(), f2->second.c_str());
		else
			printf("Processing file '%s'...\n", f1->second.c_str());

		if (!process(out, i->first, i->second.data, i->second.size))
			printf("FAILED\n");
		else
			printf("OK\n");
	}

	// Free up memory
	for (GameMap::iterator i = games.begin(); i != games.end(); ++i)
		delete[] i->second.data;
	games.clear();
	inputFiles.clear();

	if (!out.saveFile(argv[1]))
	error("couldn't save changes to '%s'", argv[1]);

	uint8 digest[16];
	if (!md5_file(argv[1], digest, 0))
		error("couldn't calc. md5 for file '%s'", argv[1]);
	FILE *f = fopen(argv[1], "ab");
	if (!f)
		error("couldn't open file '%s'", argv[1]);
	if (fwrite(digest, 1, 16, f) != 16)
		error("couldn't write md5sum to file '%s'", argv[1]);
	fclose(f);

	return 0;
}

MD5Map createMD5Sums(int files, const char * const *filenames) {
	MD5Map result;

	while (files--) {
		const char *inputFile = *filenames++;
		FILE *input = fopen(inputFile, "rb");

		uint32 size = fileSize(input);
		fseek(input, 0, SEEK_SET);

		byte *buffer = new uint8[size];
		assert(buffer);

		if (fread(buffer, 1, size, input) != size) {
			warning("couldn't read from file '%s', skipping it", inputFile);
			delete[] buffer;
			fclose(input);
			continue;
		}
		fclose(input);

		md5_context ctx;
		uint8 digest[16];
		char md5Str[33];

		md5_starts(&ctx);
		md5_update(&ctx, buffer, size);
		md5_finish(&ctx, digest);

		for (int j = 0; j < 16; ++j)
			sprintf(md5Str + j*2, "%02x", (int)digest[j]);

		delete[] buffer;

		result[md5Str] = inputFile;
	}

	return result;
}

GameMap createGameMap(const MD5Map &map) {
	GameMap result;

	for (const Game * const *g = gameDescs; *g != 0; ++g) {
		for (const Game *sub = *g; sub->game != -1; ++sub) {
			MD5Map::const_iterator file1 = map.find(sub->md5[0]);
			if (file1 == map.end())
				continue;

			MD5Map::const_iterator file2 = map.end();
			if (sub->md5[1] != 0) {
				file2 = map.find(sub->md5[1]);
				if (file2 == map.end())
					continue;
			}

			FILE *f1 = fopen(file1->second.c_str(), "rb");
			FILE *f2 = 0;

			if (file2 != map.end())
				f2 = fopen(file2->second.c_str(), "rb");

			uint32 file1Size = fileSize(f1);
			uint32 file2Size = 0;
			if (f2)
				file2Size = fileSize(f2);

			uint8 *buffer = new uint8[file1Size + file2Size];
			assert(buffer);

			fread(buffer, 1, file1Size, f1);
			if (f2)
				fread(buffer + file1Size, 1, file2Size, f2);

			fclose(f1);
			if (f2)
				fclose(f2);

			result[sub] = File(buffer, file1Size + file2Size);
		}
	}

	return result;
}

const char *getIdString(const int id) {
	switch (id) {
	case k1ForestSeq:
		return "k1ForestSeq";
	case k1KallakWritingSeq:
		return "k1KallakWritingSeq";
	case k1KyrandiaLogoSeq:
		return "k1KyrandiaLogoSeq";
	case k1KallakMalcolmSeq:
		return "k1KallakMalcolmSeq";
	case k1MalcolmTreeSeq:
		return "k1MalcolmTreeSeq";
	case k1WestwoodLogoSeq:
		return "k1WestwoodLogoSeq";
	case k1Demo1Seq:
		return "k1Demo1Seq";
	case k1Demo2Seq:
		return "k1Demo2Seq";
	case k1Demo3Seq:
		return "k1Demo3Seq";
	case k1Demo4Seq:
		return "k1Demo4Seq";
	case k1AmuleteAnimSeq:
		return "k1AmuleteAnimSeq";
	case k1OutroReunionSeq:
		return "k1OutroReunionSeq";
	case k1IntroCPSStrings:
		return "k1IntroCPSStrings";
	case k1IntroCOLStrings:
		return "k1IntroCOLStrings";
	case k1IntroWSAStrings:
		return "k1IntroWSAStrings";
	case k1IntroStrings:
		return "k1IntroStrings";
	case k1OutroHomeString:
		return "k1OutroHomeString";
	case k1RoomFilenames:
		return "k1RoomFilenames";
	case k1RoomList:
		return "k1RoomList";
	case k1CharacterImageFilenames:
		return "k1CharacterImageFilenames";
	case k1AudioTracks:
		return "k1AudioTracks";
	case k1AudioTracks2:
		return "k1AudioTracks2";
	case k1AudioTracksIntro:
		return "k1AudioTracksIntro";
	case k1ItemNames:
		return "k1ItemNames";
	case k1TakenStrings:
		return "k1TakenStrings";
	case k1PlacedStrings:
		return "k1PlacedStrings";
	case k1DroppedStrings:
		return "k1DroppedStrings";
	case k1NoDropStrings:
		return "k1NoDropStrings";
	case k1PutDownString:
		return "k1PutDownString";
	case k1WaitAmuletString:
		return "k1WaitAmuletString";
	case k1BlackJewelString:
		return "k1BlackJewelString";
	case k1PoisonGoneString:
		return "k1PoisonGoneString";
	case k1HealingTipString:
		return "k1HealingTipString";
	case k1WispJewelStrings:
		return "k1WispJewelStrings";
	case k1MagicJewelStrings:
		return "k1MagicJewelStrings";
	case k1ThePoisonStrings:
		return "k1ThePoisonStrings";
	case k1FluteStrings:
		return "k1FluteStrings";
	case k1FlaskFullString:
		return "k1FlaskFullString";
	case k1FullFlaskString:
		return "k1FullFlaskString";
	case k1VeryCleverString:
		return "k1VeryCleverString";
	case k1NewGameString:
		return "k1NewGameString";
	case k1DefaultShapes:
		return "k1DefaultShapes";
	case k1Healing1Shapes:
		return "k1Healing1Shapes";
	case k1Healing2Shapes:
		return "k1Healing2Shapes";
	case k1PoisonDeathShapes:
		return "k1PoisonDeathShapes";
	case k1FluteShapes:
		return "k1FluteShapes";
	case k1Winter1Shapes:
		return "k1Winter1Shapes";
	case k1Winter2Shapes:
		return "k1Winter2Shapes";
	case k1Winter3Shapes:
		return "k1Winter3Shapes";
	case k1DrinkShapes:
		return "k1DrinkShapes";
	case k1WispShapes:
		return "k1WispShapes";
	case k1MagicAnimShapes:
		return "k1MagicAnimShapes";
	case k1BranStoneShapes:
		return "k1BranStoneShapes";
	case k1SpecialPalette1:
		return "k1SpecialPalette1";
	case k1SpecialPalette2:
		return "k1SpecialPalette2";
	case k1SpecialPalette3:
		return "k1SpecialPalette3";
	case k1SpecialPalette4:
		return "k1SpecialPalette4";
	case k1SpecialPalette5:
		return "k1SpecialPalette5";
	case k1SpecialPalette6:
		return "k1SpecialPalette6";
	case k1SpecialPalette7:
		return "k1SpecialPalette7";
	case k1SpecialPalette8:
		return "k1SpecialPalette8";
	case k1SpecialPalette9:
		return "k1SpecialPalette9";
	case k1SpecialPalette10:
		return "k1SpecialPalette10";
	case k1SpecialPalette11:
		return "k1SpecialPalette11";
	case k1SpecialPalette12:
		return "k1SpecialPalette12";
	case k1SpecialPalette13:
		return "k1SpecialPalette13";
	case k1SpecialPalette14:
		return "k1SpecialPalette14";
	case k1SpecialPalette15:
		return "k1SpecialPalette15";
	case k1SpecialPalette16:
		return "k1SpecialPalette16";
	case k1SpecialPalette17:
		return "k1SpecialPalette17";
	case k1SpecialPalette18:
		return "k1SpecialPalette18";
	case k1SpecialPalette19:
		return "k1SpecialPalette19";
	case k1SpecialPalette20:
		return "k1SpecialPalette20";
	case k1SpecialPalette21:
		return "k1SpecialPalette21";
	case k1SpecialPalette22:
		return "k1SpecialPalette22";
	case k1SpecialPalette23:
		return "k1SpecialPalette23";
	case k1SpecialPalette24:
		return "k1SpecialPalette24";
	case k1SpecialPalette25:
		return "k1SpecialPalette25";
	case k1SpecialPalette26:
		return "k1SpecialPalette26";
	case k1SpecialPalette27:
		return "k1SpecialPalette27";
	case k1SpecialPalette28:
		return "k1SpecialPalette28";
	case k1SpecialPalette29:
		return "k1SpecialPalette29";
	case k1SpecialPalette30:
		return "k1SpecialPalette30";
	case k1SpecialPalette31:
		return "k1SpecialPalette31";
	case k1SpecialPalette32:
		return "k1SpecialPalette32";
	case k1SpecialPalette33:
		return "k1SpecialPalette33";
	case k1GUIStrings:
		return "k1GUIStrings";
	case k1ConfigStrings:
		return "k1ConfigStrings";
	case k1TownsMusicFadeTable:
		return "k1TownsMusicFadeTable";
	case k1TownsSFXwdTable:
		return "k1TownsSFXwdTable";
	case k1TownsSFXbtTable:
		return "k1TownsSFXbtTable";
	case k1TownsCDATable:
		return "k1TownsCDATable";
	case k1PC98StoryStrings:
		return "k1PC98StoryStrings";
	case k1PC98IntroSfx:
		return "k1PC98IntroSfx";
	case k1CreditsStrings:
		return "k1CreditsStrings";
	case k1AmigaIntroSFXTable:
		return "k1AmigaIntroSFXTable";
	case k1AmigaGameSFXTable:
		return "k1AmigaGameSFXTable";
	case k2SeqplayPakFiles:
		return "k2SeqplayPakFiles";
	case k2SeqplayStrings:
		return "k2SeqplayStrings";
	case k2SeqplaySfxFiles:
		return "k2SeqplaySfxFiles";
	case k2SeqplayTlkFiles:
		return "k2SeqplayTlkFiles";
	case k2SeqplaySeqData:
		return "k2SeqplaySeqData";
	case k2SeqplayCredits:
		return "k2SeqplayCredits";
	case k2SeqplayCreditsSpecial:
		return "k2SeqplayCreditsSpecial";
	case k2SeqplayIntroTracks:
		return "k2SeqplayIntroTracks";
	case k2SeqplayFinaleTracks:
		return "k2SeqplayFinaleTracks";
	case k2SeqplayIntroCDA:
		return "k2SeqplayIntroCDA";
	case k2SeqplayFinaleCDA:
		return "k2SeqplayFinaleCDA";
	case k2SeqplayShapeAnimData:
		return "k2SeqplayShapeAnimData";
	case k2IngamePakFiles:
		return "k2IngamePakFiles";
	case k2IngameSfxFiles:
		return "k2IngameSfxFiles";
	case k2IngameSfxIndex:
		return "k2IngameSfxIndex";
	case k2IngameTracks:
		return "k2IngameTracks";
	case k2IngameCDA:
		return "k2IngameCDA";
	case k2IngameTalkObjIndex:
		return "k2IngameTalkObjIndex";
	case k2IngameTimJpStrings:
		return "k2IngameTimJpStrings";
	case k2IngameShapeAnimData:
		return "k2IngameShapeAnimData";
	case k2IngameTlkDemoStrings:
		return "k2IngameTlkDemoStrings";
	case k3MainMenuStrings:
		return "k3MainMenuStrings";
	case k3MusicFiles:
		return "k3MusicFiles";
	case k3ScoreTable:
		return "k3ScoreTable";
	case k3SfxFiles:
		return "k3SfxFiles";
	case k3SfxMap:
		return "k3SfxMap";
	case k3ItemAnimData:
		return "k3ItemAnimData";
	case k3ItemMagicTable:
		return "k3ItemMagicTable";
	case k3ItemStringMap:
		return "k3ItemStringMap";
	case kEoBBaseChargenStrings1:
		return "kEoBBaseChargenStrings1";
	case kEoBBaseChargenStrings2:
		return "kEoBBaseChargenStrings2";
	case kEoBBaseChargenStartLevels:
		return "kEoBBaseChargenStartLevels";
	case kEoBBaseChargenStatStrings:
		return "kEoBBaseChargenStatStrings";
	case kEoBBaseChargenRaceSexStrings:
		return "kEoBBaseChargenRaceSexStrings";
	case kEoBBaseChargenClassStrings:
		return "kEoBBaseChargenClassStrings";
	case kEoBBaseChargenAlignmentStrings:
		return "kEoBBaseChargenAlignmentStrings";
	case kEoBBaseChargenEnterGameStrings:
		return "kEoBBaseChargenEnterGameStrings";
	case kEoBBaseChargenClassMinStats:
		return "kEoBBaseChargenClassMinStats";
	case kEoBBaseChargenRaceMinStats:
		return "kEoBBaseChargenRaceMinStats";
	case kEoBBaseChargenRaceMaxStats:
		return "kEoBBaseChargenRaceMaxStats";
	case kEoBBaseSaveThrowTable1:
		return "kEoBBaseSaveThrowTable1";
	case kEoBBaseSaveThrowTable2:
		return "kEoBBaseSaveThrowTable2";
	case kEoBBaseSaveThrowTable3:
		return "kEoBBaseSaveThrowTable3";
	case kEoBBaseSaveThrowTable4:
		return "kEoBBaseSaveThrowTable4";
	case kEoBBaseSaveThrwLvlIndex:
		return "kEoBBaseSaveThrwLvlIndex";
	case kEoBBaseSaveThrwModDiv:
		return "kEoBBaseSaveThrwModDiv";
	case kEoBBaseSaveThrwModExt:
		return "kEoBBaseSaveThrwModExt";
	case kEoBBasePryDoorStrings:
		return "kEoBBasePryDoorStrings";
	case kEoBBaseWarningStrings:
		return "kEoBBaseWarningStrings";
	case kEoBBaseItemSuffixStringsRings:
		return "kEoBBaseItemSuffixStringsRings";
	case kEoBBaseItemSuffixStringsPotions:
		return "kEoBBaseItemSuffixStringsPotions";
	case kEoBBaseItemSuffixStringsWands:
		return "kEoBBaseItemSuffixStringsWands";
	case kEoBBaseRipItemStrings:
		return "kEoBBaseRipItemStrings";
	case kEoBBaseCursedString:
		return "kEoBBaseCursedString";
	case kEoBBaseEnchantedString:
		return "kEoBBaseEnchantedString";
	case kEoBBaseMagicObjectStrings:
		return "kEoBBaseMagicObjectStrings";
	case kEoBBaseMagicObject5String:
		return "kEoBBaseMagicObject5String";
	case kEoBBasePatternSuffix:
		return "kEoBBasePatternSuffix";
	case kEoBBasePatternGrFix1:
		return "kEoBBasePatternGrFix1";
	case kEoBBasePatternGrFix2:
		return "kEoBBasePatternGrFix2";
	case kEoBBaseValidateArmorString:
		return "kEoBBaseValidateArmorString";
	case kEoBBaseValidateCursedString:
		return "kEoBBaseValidateCursedString";
	case kEoBBaseValidateNoDropString:
		return "kEoBBaseValidateNoDropString";
	case kEoBBasePotionStrings:
		return "kEoBBasePotionStrings";
	case kEoBBaseWandString:
		return "kEoBBaseWandString";
	case kEoBBaseItemMisuseStrings:
		return "kEoBBaseItemMisuseStrings";
	case kEoBBaseTakenStrings:
		return "kEoBBaseTakenStrings";
	case kEoBBasePotionEffectStrings:
		return "kEoBBasePotionEffectStrings";
	case kEoBBaseYesNoStrings:
		return "kEoBBaseYesNoStrings";
	case kRpgCommonMoreStrings:
		return "kRpgCommonMoreStrings";
	case kEoBBaseNpcMaxStrings:
		return "kEoBBaseNpcMaxStrings";
	case kEoBBaseOkStrings:
		return "kEoBBaseOkStrings";
	case kEoBBaseNpcJoinStrings:
		return "kEoBBaseNpcJoinStrings";
	case kEoBBaseCancelStrings:
		return "kEoBBaseCancelStrings";
	case kEoBBaseAbortStrings:
		return "kEoBBaseAbortStrings";
	case kEoBBaseMenuStringsMain:
		return "kEoBBaseMenuStringsMain";
	case kEoBBaseMenuStringsSaveLoad:
		return "kEoBBaseMenuStringsSaveLoad";
	case kEoBBaseMenuStringsOnOff:
		return "kEoBBaseMenuStringsOnOff";
	case kEoBBaseMenuStringsSpells:
		return "kEoBBaseMenuStringsSpells";
	case kEoBBaseMenuStringsRest:
		return "kEoBBaseMenuStringsRest";
	case kEoBBaseMenuStringsDrop:
		return "kEoBBaseMenuStringsDrop";
	case kEoBBaseMenuStringsExit:
		return "kEoBBaseMenuStringsExit";
	case kEoBBaseMenuStringsStarve:
		return "kEoBBaseMenuStringsStarve";
	case kEoBBaseMenuStringsScribe:
		return "kEoBBaseMenuStringsScribe";
	case kEoBBaseMenuStringsDrop2:
		return "kEoBBaseMenuStringsDrop2";
	case kEoBBaseMenuStringsHead:
		return "kEoBBaseMenuStringsHead";
	case kEoBBaseMenuStringsPoison:
		return "kEoBBaseMenuStringsPoison";
	case kEoBBaseMenuStringsMgc:
		return "kEoBBaseMenuStringsMgc";
	case kEoBBaseMenuStringsPrefs:
		return "kEoBBaseMenuStringsPrefs";
	case kEoBBaseMenuStringsRest2:
		return "kEoBBaseMenuStringsRest2";
	case kEoBBaseMenuStringsRest3:
		return "kEoBBaseMenuStringsRest3";
	case kEoBBaseMenuStringsRest4:
		return "kEoBBaseMenuStringsRest4";
	case kEoBBaseMenuStringsDefeat:
		return "kEoBBaseMenuStringsDefeat";
	case kEoBBaseMenuStringsTransfer:
		return "kEoBBaseMenuStringsTransfer";
	case kEoBBaseMenuStringsSpec:
		return "kEoBBaseMenuStringsSpec";
	case kEoBBaseMenuStringsSpellNo:
		return "kEoBBaseMenuStringsSpellNo";
	case kEoBBaseMenuYesNoStrings:
		return "kEoBBaseMenuYesNoStrings";
	case kEoBBaseSpellLevelsMage:
		return "kEoBBaseSpellLevelsMage";
	case kEoBBaseSpellLevelsCleric:
		return "kEoBBaseSpellLevelsCleric";
	case kEoBBaseNumSpellsCleric:
		return "kEoBBaseNumSpellsCleric";
	case kEoBBaseNumSpellsWisAdj:
		return "kEoBBaseNumSpellsWisAdj";
	case kEoBBaseNumSpellsPal:
		return "kEoBBaseNumSpellsPal";
	case kEoBBaseNumSpellsMage:
		return "kEoBBaseNumSpellsMage";
	case kEoBBaseCharGuiStringsHp:
		return "kEoBBaseCharGuiStringsHp";
	case kEoBBaseCharGuiStringsWp1:
		return "kEoBBaseCharGuiStringsWp1";
	case kEoBBaseCharGuiStringsWp2:
		return "kEoBBaseCharGuiStringsWp2";
	case kEoBBaseCharGuiStringsWr:
		return "kEoBBaseCharGuiStringsWr";
	case kEoBBaseCharGuiStringsSt1:
		return "kEoBBaseCharGuiStringsSt1";
	case kEoBBaseCharGuiStringsSt2:
		return "kEoBBaseCharGuiStringsSt2";
	case kEoBBaseCharGuiStringsIn:
		return "kEoBBaseCharGuiStringsIn";
	case kEoBBaseCharStatusStrings7:
		return "kEoBBaseCharStatusStrings7";
	case kEoBBaseCharStatusStrings81:
		return "kEoBBaseCharStatusStrings81";
	case kEoBBaseCharStatusStrings82:
		return "kEoBBaseCharStatusStrings82";
	case kEoBBaseCharStatusStrings9:
		return "kEoBBaseCharStatusStrings9";
	case kEoBBaseCharStatusStrings12:
		return "kEoBBaseCharStatusStrings12";
	case kEoBBaseCharStatusStrings131:
		return "kEoBBaseCharStatusStrings131";
	case kEoBBaseCharStatusStrings132:
		return "kEoBBaseCharStatusStrings132";
	case kEoBBaseLevelGainStrings:
		return "kEoBBaseLevelGainStrings";
	case kEoBBaseExperienceTable0:
		return "kEoBBaseExperienceTable0";
	case kEoBBaseExperienceTable1:
		return "kEoBBaseExperienceTable1";
	case kEoBBaseExperienceTable2:
		return "kEoBBaseExperienceTable2";
	case kEoBBaseExperienceTable3:
		return "kEoBBaseExperienceTable3";
	case kEoBBaseExperienceTable4:
		return "kEoBBaseExperienceTable4";
	case kEoBBaseWllFlagPreset:
		return "kEoBBaseWllFlagPreset";
	case kEoBBaseDscShapeCoords:
		return "kEoBBaseDscShapeCoords";
	case kEoBBaseDscDoorScaleOffs:
		return "kEoBBaseDscDoorScaleOffs";
	case kEoBBaseDscDoorScaleMult1:
		return "kEoBBaseDscDoorScaleMult1";
	case kEoBBaseDscDoorScaleMult2:
		return "kEoBBaseDscDoorScaleMult2";
	case kEoBBaseDscDoorScaleMult3:
		return "kEoBBaseDscDoorScaleMult3";
	case kEoBBaseDscDoorScaleMult4:
		return "kEoBBaseDscDoorScaleMult4";
	case kEoBBaseDscDoorScaleMult5:
		return "kEoBBaseDscDoorScaleMult5";
	case kEoBBaseDscDoorScaleMult6:
		return "kEoBBaseDscDoorScaleMult6";
	case kEoBBaseDscDoorType5Offs:
		return "kEoBBaseDscDoorType5Offs";
	case kEoBBaseDscDoorXE:
		return "kEoBBaseDscDoorXE";
	case kEoBBaseDscDoorY1:
		return "kEoBBaseDscDoorY1";
	case kEoBBaseDscDoorY3:
		return "kEoBBaseDscDoorY3";
	case kEoBBaseDscDoorY4:
		return "kEoBBaseDscDoorY4";
	case kEoBBaseDscDoorY5:
		return "kEoBBaseDscDoorY5";
	case kEoBBaseDscDoorY6:
		return "kEoBBaseDscDoorY6";
	case kEoBBaseDscDoorY7:
		return "kEoBBaseDscDoorY7";
	case kEoBBaseDscDoorCoordsExt:
		return "kEoBBaseDscDoorCoordsExt";
	case kEoBBaseDscItemPosIndex:
		return "kEoBBaseDscItemPosIndex";
	case kEoBBaseDscItemShpX:
		return "kEoBBaseDscItemShpX";
	case kEoBBaseDscItemPosUnk:
		return "kEoBBaseDscItemPosUnk";
	case kEoBBaseDscItemTileIndex:
		return "kEoBBaseDscItemTileIndex";
	case kEoBBaseDscItemShapeMap:
		return "kEoBBaseDscItemShapeMap";
	case kEoBBaseDscMonsterFrmOffsTbl1:
		return "kEoBBaseDscMonsterFrmOffsTbl1";
	case kEoBBaseDscMonsterFrmOffsTbl2:
		return "kEoBBaseDscMonsterFrmOffsTbl2";
	case kEoBBaseInvSlotX:
		return "kEoBBaseInvSlotX";
	case kEoBBaseInvSlotY:
		return "kEoBBaseInvSlotY";
	case kEoBBaseSlotValidationFlags:
		return "kEoBBaseSlotValidationFlags";
	case kEoBBaseProjectileWeaponTypes:
		return "kEoBBaseProjectileWeaponTypes";
	case kEoBBaseWandTypes:
		return "kEoBBaseWandTypes";
	case kEoBBaseDrawObjPosIndex:
		return "kEoBBaseDrawObjPosIndex";
	case kEoBBaseFlightObjFlipIndex:
		return "kEoBBaseFlightObjFlipIndex";
	case kEoBBaseFlightObjShpMap:
		return "kEoBBaseFlightObjShpMap";
	case kEoBBaseFlightObjSclIndex:
		return "kEoBBaseFlightObjSclIndex";
	case kEoBBaseDscTelptrShpCoords:
		return "kEoBBaseDscTelptrShpCoords";
	case kEoBBasePortalSeqData:
		return "kEoBBasePortalSeqData";
	case kEoBBaseManDef:
		return "kEoBBaseManDef";
	case kEoBBaseManWord:
		return "kEoBBaseManWord";
	case kEoBBaseManPrompt:
		return "kEoBBaseManPrompt";
	case kEoBBaseBookNumbers:
		return "kEoBBaseBookNumbers";
	case kEoBBaseMageSpellsList:
		return "kEoBBaseMageSpellsList";
	case kEoBBaseClericSpellsList:
		return "kEoBBaseClericSpellsList";
	case kEoBBaseSpellNames:
		return "kEoBBaseSpellNames";

	case kEoBBaseMagicStrings1:
		return "kEoBBaseMagicStrings1";
	case kEoBBaseMagicStrings2:
		return "kEoBBaseMagicStrings2";
	case kEoBBaseMagicStrings3:
		return "kEoBBaseMagicStrings3";
	case kEoBBaseMagicStrings4:
		return "kEoBBaseMagicStrings4";
	case kEoBBaseMagicStrings6:
		return "kEoBBaseMagicStrings6";
	case kEoBBaseMagicStrings7:
		return "kEoBBaseMagicStrings7";
	case kEoBBaseMagicStrings8:
		return "kEoBBaseMagicStrings8";
	case kEoBBaseExpObjectTlMode:
		return "kEoBBaseExpObjectTlMode";
	case kEoBBaseExpObjectTblIndex:
		return "kEoBBaseExpObjectTblIndex";
	case kEoBBaseExpObjectShpStart:
		return "kEoBBaseExpObjectShpStart";
	case kEoBBaseExpObjectTbl1:
		return "kEoBBaseExpObjectTbl1";
	case kEoBBaseExpObjectTbl2:
		return "kEoBBaseExpObjectTbl2";
	case kEoBBaseExpObjectTbl3:
		return "kEoBBaseExpObjectTbl3";
	case kEoBBaseExpObjectY:
		return "kEoBBaseExpObjectY";
	case kEoBBaseSparkDefSteps:
		return "kEoBBaseSparkDefSteps";
	case kEoBBaseSparkDefSubSteps:
		return "kEoBBaseSparkDefSubSteps";
	case kEoBBaseSparkDefShift:
		return "kEoBBaseSparkDefShift";
	case kEoBBaseSparkDefAdd:
		return "kEoBBaseSparkDefAdd";
	case kEoBBaseSparkDefX:
		return "kEoBBaseSparkDefX";
	case kEoBBaseSparkDefY:
		return "kEoBBaseSparkDefY";
	case kEoBBaseSparkOfFlags1:
		return "kEoBBaseSparkOfFlags1";
	case kEoBBaseSparkOfFlags2:
		return "kEoBBaseSparkOfFlags2";
	case kEoBBaseSparkOfShift:
		return "kEoBBaseSparkOfShift";
	case kEoBBaseSparkOfX:
		return "kEoBBaseSparkOfX";
	case kEoBBaseSparkOfY:
		return "kEoBBaseSparkOfY";
	case kEoBBaseSpellProperties:
		return "kEoBBaseSpellProperties";
	case kEoBBaseMagicFlightProps:
		return "kEoBBaseMagicFlightProps";
	case kEoBBaseTurnUndeadEffect:
		return "kEoBBaseTurnUndeadEffect";
	case kEoBBaseBurningHandsDest:
		return "kEoBBaseBurningHandsDest";
	case kEoBBaseConeOfColdDest1:
		return "kEoBBaseConeOfColdDest1";
	case kEoBBaseConeOfColdDest2:
		return "kEoBBaseConeOfColdDest2";
	case kEoBBaseConeOfColdDest3:
		return "kEoBBaseConeOfColdDest3";
	case kEoBBaseConeOfColdDest4:
		return "kEoBBaseConeOfColdDest4";
	case kEoBBaseConeOfColdGfxTbl:
		return "kEoBBaseConeOfColdGfxTbl";
	case kEoB1MainMenuStrings:
		return "kEoB1MainMenuStrings";
	case kEoB1BonusStrings:
		return "kEoB1BonusStrings";
	case kEoB1IntroFilesOpening:
		return "kEoB1IntroFilesOpening";
	case kEoB1IntroFilesTower:
		return "kEoB1IntroFilesTower";
	case kEoB1IntroFilesOrb:
		return "kEoB1IntroFilesOrb";
	case kEoB1IntroFilesWdEntry:
		return "kEoB1IntroFilesWdEntry";
	case kEoB1IntroFilesKing:
		return "kEoB1IntroFilesKing";
	case kEoB1IntroFilesHands:
		return "kEoB1IntroFilesHands";
	case kEoB1IntroFilesWdExit:
		return "kEoB1IntroFilesWdExit";
	case kEoB1IntroFilesTunnel:
		return "kEoB1IntroFilesTunnel";
	case kEoB1IntroOpeningFrmDelay:
		return "kEoB1IntroOpeningFrmDelay";
	case kEoB1IntroWdEncodeX:
		return "kEoB1IntroWdEncodeX";
	case kEoB1IntroWdEncodeY:
		return "kEoB1IntroWdEncodeY";
	case kEoB1IntroWdEncodeWH:
		return "kEoB1IntroWdEncodeWH";
	case kEoB1IntroWdDsX:
		return "kEoB1IntroWdDsX";
	case kEoB1IntroWdDsY:
		return "kEoB1IntroWdDsY";
	case kEoB1IntroTvlX1:
		return "kEoB1IntroTvlX1";
	case kEoB1IntroTvlY1:
		return "kEoB1IntroTvlY1";
	case kEoB1IntroTvlX2:
		return "kEoB1IntroTvlX2";
	case kEoB1IntroTvlY2:
		return "kEoB1IntroTvlY2";
	case kEoB1IntroTvlW:
		return "kEoB1IntroTvlW";
	case kEoB1IntroTvlH:
		return "kEoB1IntroTvlH";
	case kEoB1DoorShapeDefs:
		return "kEoB1DoorShapeDefs";
	case kEoB1DoorSwitchCoords:
		return "kEoB1DoorSwitchCoords";
	case kEoB1MonsterProperties:
		return "kEoB1MonsterProperties";
	case kEoB1EnemyMageSpellList:
		return "kEoB1EnemyMageSpellList";
	case kEoB1EnemyMageSfx:
		return "kEoB1EnemyMageSfx";
	case kEoB1BeholderSpellList:
		return "kEoB1BeholderSpellList";
	case kEoB1BeholderSfx:
		return "kEoB1BeholderSfx";
	case kEoB1TurnUndeadString:
		return "kEoB1TurnUndeadString";
	case kEoB1CgaMappingDefault:
		return "kEoB1CgaMappingDefault";
	case kEoB1CgaMappingAlt:
		return "kEoB1CgaMappingAlt";
	case kEoB1CgaMappingInv:
		return "kEoB1CgaMappingInv";
	case kEoB1CgaMappingItemsL:
		return "kEoB1CgaMappingItemsL";
	case kEoB1CgaMappingItemsS:
		return "kEoB1CgaMappingItemsS";
	case kEoB1CgaMappingThrown:
		return "kEoB1CgaMappingThrown";
	case kEoB1CgaMappingIcons:
		return "kEoB1CgaMappingIcons";
	case kEoB1CgaMappingDeco:
		return "kEoB1CgaMappingDeco";
	case kEoB1CgaLevelMappingIndex:
		return "kEoB1CgaLevelMappingIndex";
	case kEoB1CgaMappingLevel0:
		return "kEoB1CgaMappingLevel0";
	case kEoB1CgaMappingLevel1:
		return "kEoB1CgaMappingLevel1";
	case kEoB1CgaMappingLevel2:
		return "kEoB1CgaMappingLevel2";
	case kEoB1CgaMappingLevel3:
		return "kEoB1CgaMappingLevel3";
	case kEoB1CgaMappingLevel4:
		return "kEoB1CgaMappingLevel4";
	case kEoB1NpcShpData:
		return "kEoB1NpcShpData";
	case kEoB1NpcSubShpIndex1:
		return "kEoB1NpcSubShpIndex1";
	case kEoB1NpcSubShpIndex2:
		return "kEoB1NpcSubShpIndex2";
	case kEoB1NpcSubShpY:
		return "kEoB1NpcSubShpY";
	case kEoB1Npc0Strings:
		return "kEoB1Npc0Strings";
	case kEoB1Npc11Strings:
		return "kEoB1Npc11Strings";
	case kEoB1Npc12Strings:
		return "kEoB1Npc12Strings";
	case kEoB1Npc21Strings:
		return "kEoB1Npc21Strings";
	case kEoB1Npc22Strings:
		return "kEoB1Npc22Strings";
	case kEoB1Npc31Strings:
		return "kEoB1Npc31Strings";
	case kEoB1Npc32Strings:
		return "kEoB1Npc32Strings";
	case kEoB1Npc4Strings:
		return "kEoB1Npc4Strings";
	case kEoB1Npc5Strings:
		return "kEoB1Npc5Strings";
	case kEoB1Npc6Strings:
		return "kEoB1Npc6Strings";
	case kEoB1Npc7Strings:
		return "kEoB1Npc7Strings";
	case kEoB2MainMenuStrings:
		return "kEoB2MainMenuStrings";
	case kEoB2TransferPortraitFrames:
		return "kEoB2TransferPortraitFrames";
	case kEoB2TransferConvertTable:
		return "kEoB2TransferConvertTable";
	case kEoB2TransferItemTable:
		return "kEoB2TransferItemTable";
	case kEoB2TransferExpTable:
		return "kEoB2TransferExpTable";
	case kEoB2TransferStrings1:
		return "kEoB2TransferStrings1";
	case kEoB2TransferStrings2:
		return "kEoB2TransferStrings2";
	case kEoB2TransferLabels:
		return "kEoB2TransferLabels";
	case kEoB2IntroStrings:
		return "kEoB2IntroStrings";
	case kEoB2IntroCPSFiles:
		return "kEoB2IntroCPSFiles";
	case kEob2IntroAnimData00:
		return "kEob2IntroAnimData00";
	case kEob2IntroAnimData01:
		return "kEob2IntroAnimData01";
	case kEob2IntroAnimData02:
		return "kEob2IntroAnimData02";
	case kEob2IntroAnimData03:
		return "kEob2IntroAnimData03";
	case kEob2IntroAnimData04:
		return "kEob2IntroAnimData04";
	case kEob2IntroAnimData05:
		return "kEob2IntroAnimData05";
	case kEob2IntroAnimData06:
		return "kEob2IntroAnimData06";
	case kEob2IntroAnimData07:
		return "kEob2IntroAnimData07";
	case kEob2IntroAnimData08:
		return "kEob2IntroAnimData08";
	case kEob2IntroAnimData09:
		return "kEob2IntroAnimData09";
	case kEob2IntroAnimData10:
		return "kEob2IntroAnimData10";
	case kEob2IntroAnimData11:
		return "kEob2IntroAnimData11";
	case kEob2IntroAnimData12:
		return "kEob2IntroAnimData12";
	case kEob2IntroAnimData13:
		return "kEob2IntroAnimData13";
	case kEob2IntroAnimData14:
		return "kEob2IntroAnimData14";
	case kEob2IntroAnimData15:
		return "kEob2IntroAnimData15";
	case kEob2IntroAnimData16:
		return "kEob2IntroAnimData16";
	case kEob2IntroAnimData17:
		return "kEob2IntroAnimData17";
	case kEob2IntroAnimData18:
		return "kEob2IntroAnimData18";
	case kEob2IntroAnimData19:
		return "kEob2IntroAnimData19";
	case kEob2IntroAnimData20:
		return "kEob2IntroAnimData20";
	case kEob2IntroAnimData21:
		return "kEob2IntroAnimData21";
	case kEob2IntroAnimData22:
		return "kEob2IntroAnimData22";
	case kEob2IntroAnimData23:
		return "kEob2IntroAnimData23";
	case kEob2IntroAnimData24:
		return "kEob2IntroAnimData24";
	case kEob2IntroAnimData25:
		return "kEob2IntroAnimData25";
	case kEob2IntroAnimData26:
		return "kEob2IntroAnimData26";
	case kEob2IntroAnimData27:
		return "kEob2IntroAnimData27";
	case kEob2IntroAnimData28:
		return "kEob2IntroAnimData28";
	case kEob2IntroAnimData29:
		return "kEob2IntroAnimData29";
	case kEob2IntroAnimData30:
		return "kEob2IntroAnimData30";
	case kEob2IntroAnimData31:
		return "kEob2IntroAnimData31";
	case kEob2IntroAnimData32:
		return "kEob2IntroAnimData32";
	case kEob2IntroAnimData33:
		return "kEob2IntroAnimData33";
	case kEob2IntroAnimData34:
		return "kEob2IntroAnimData34";
	case kEob2IntroAnimData35:
		return "kEob2IntroAnimData35";
	case kEob2IntroAnimData36:
		return "kEob2IntroAnimData36";
	case kEob2IntroAnimData37:
		return "kEob2IntroAnimData37";
	case kEob2IntroAnimData38:
		return "kEob2IntroAnimData38";
	case kEob2IntroAnimData39:
		return "kEob2IntroAnimData39";
	case kEob2IntroAnimData40:
		return "kEob2IntroAnimData40";
	case kEob2IntroAnimData41:
		return "kEob2IntroAnimData41";
	case kEob2IntroAnimData42:
		return "kEob2IntroAnimData42";
	case kEob2IntroAnimData43:
		return "kEob2IntroAnimData43";
	case kEoB2IntroShapes00:
		return "kEoB2IntroShapes00";
	case kEoB2IntroShapes01:
		return "kEoB2IntroShapes01";
	case kEoB2IntroShapes04:
		return "kEoB2IntroShapes04";
	case kEoB2IntroShapes07:
		return "kEoB2IntroShapes07";
	case kEoB2FinaleStrings:
		return "kEoB2FinaleStrings";
	case kEoB2CreditsData:
		return "kEoB2CreditsData";
	case kEoB2FinaleCPSFiles:
		return "kEoB2FinaleCPSFiles";
	case kEob2FinaleAnimData00:
		return "kEob2FinaleAnimData00";
	case kEob2FinaleAnimData01:
		return "kEob2FinaleAnimData01";
	case kEob2FinaleAnimData02:
		return "kEob2FinaleAnimData02";
	case kEob2FinaleAnimData03:
		return "kEob2FinaleAnimData03";
	case kEob2FinaleAnimData04:
		return "kEob2FinaleAnimData04";
	case kEob2FinaleAnimData05:
		return "kEob2FinaleAnimData05";
	case kEob2FinaleAnimData06:
		return "kEob2FinaleAnimData06";
	case kEob2FinaleAnimData07:
		return "kEob2FinaleAnimData07";
	case kEob2FinaleAnimData08:
		return "kEob2FinaleAnimData08";
	case kEob2FinaleAnimData09:
		return "kEob2FinaleAnimData09";
	case kEob2FinaleAnimData10:
		return "kEob2FinaleAnimData10";
	case kEob2FinaleAnimData11:
		return "kEob2FinaleAnimData11";
	case kEob2FinaleAnimData12:
		return "kEob2FinaleAnimData12";
	case kEob2FinaleAnimData13:
		return "kEob2FinaleAnimData13";
	case kEob2FinaleAnimData14:
		return "kEob2FinaleAnimData14";
	case kEob2FinaleAnimData15:
		return "kEob2FinaleAnimData15";
	case kEob2FinaleAnimData16:
		return "kEob2FinaleAnimData16";
	case kEob2FinaleAnimData17:
		return "kEob2FinaleAnimData17";
	case kEob2FinaleAnimData18:
		return "kEob2FinaleAnimData18";
	case kEob2FinaleAnimData19:
		return "kEob2FinaleAnimData19";
	case kEob2FinaleAnimData20:
		return "kEob2FinaleAnimData20";
	case kEoB2FinaleShapes00:
		return "kEoB2FinaleShapes00";
	case kEoB2FinaleShapes03:
		return "kEoB2FinaleShapes03";
	case kEoB2FinaleShapes07:
		return "kEoB2FinaleShapes07";
	case kEoB2FinaleShapes09:
		return "kEoB2FinaleShapes09";
	case kEoB2FinaleShapes10:
		return "kEoB2FinaleShapes10";
	case kEoB2NpcShapeData:
		return "kEoB2NpcShapeData";
	case kEoBBaseClassModifierFlags:
		return "kEoBBaseClassModifierFlags";
	case kEoBBaseMonsterStepTable01:
		return "kEoBBaseMonsterStepTable01";
	case kEoBBaseMonsterStepTable02:
		return "kEoBBaseMonsterStepTable02";
	case kEoBBaseMonsterStepTable1:
		return "kEoBBaseMonsterStepTable1";
	case kEoBBaseMonsterStepTable2:
		return "kEoBBaseMonsterStepTable2";
	case kEoBBaseMonsterStepTable3:
		return "kEoBBaseMonsterStepTable3";
	case kEoBBaseMonsterCloseAttPosTable1:
		return "kEoBBaseMonsterCloseAttPosTable1";
	case kEoBBaseMonsterCloseAttPosTable21:
		return "kEoBBaseMonsterCloseAttPosTable21";
	case kEoBBaseMonsterCloseAttPosTable22:
		return "kEoBBaseMonsterCloseAttPosTable22";
	case kEoBBaseMonsterCloseAttUnkTable:
		return "kEoBBaseMonsterCloseAttUnkTable";
	case kEoBBaseMonsterCloseAttChkTable1:
		return "kEoBBaseMonsterCloseAttChkTable1";
	case kEoBBaseMonsterCloseAttChkTable2:
		return "kEoBBaseMonsterCloseAttChkTable2";
	case kEoBBaseMonsterCloseAttDstTable1:
		return "kEoBBaseMonsterCloseAttDstTable1";
	case kEoBBaseMonsterCloseAttDstTable2:
		return "kEoBBaseMonsterCloseAttDstTable2";
	case kEoBBaseMonsterProximityTable:
		return "kEoBBaseMonsterProximityTable";
	case kEoBBaseFindBlockMonstersTable:
		return "kEoBBaseFindBlockMonstersTable";
	case kEoBBaseMonsterDirChangeTable:
		return "kEoBBaseMonsterDirChangeTable";
	case kEoBBaseMonsterDistAttStrings:
		return "kEoBBaseMonsterDistAttStrings";
	case kEoBBaseEncodeMonsterDefs:
		return "kEoBBaseEncodeMonsterDefs";
	case kEoBBaseNpcPresets:
		return "kEoBBaseNpcPresets";
	case kEoB2Npc1Strings:
		return "kEoB2Npc1Strings";
	case kEoB2Npc2Strings:
		return "kEoB2Npc2Strings";
	case kEoB2MonsterDustStrings:
		return "kEoB2MonsterDustStrings";
	case kEoB2DreamSteps:
		return "kEoB2DreamSteps";
	case kEoB2KheldranStrings:
		return "kEoB2KheldranStrings";
	case kEoB2HornStrings:
		return "kEoB2HornStrings";
	case kEoB2HornSounds:
		return "kEoB2HornSounds";
	case kEoB2WallOfForceDsX:
		return "kEoB2WallOfForceDsX";
	case kEoB2WallOfForceDsY:
		return "kEoB2WallOfForceDsY";
	case kEoB2WallOfForceNumW:
		return "kEoB2WallOfForceNumW";
	case kEoB2WallOfForceNumH:
		return "kEoB2WallOfForceNumH";
	case kEoB2WallOfForceShpId:
		return "kEoB2WallOfForceShpId";
	case kLoLIngamePakFiles:
		return "kLoLIngamePakFiles";
	case kLoLCharacterDefs:
		return "kLoLCharacterDefs";
	case kLoLIngameSfxFiles:
		return "kLoLIngameSfxFiles";
	case kLoLIngameSfxIndex:
		return "kLoLIngameSfxIndex";
	case kLoLMusicTrackMap:
		return "kLoLMusicTrackMap";
	case kLoLIngameGMSfxIndex:
		return "kLoLIngameGMSfxIndex";
	case kLoLIngameMT32SfxIndex:
		return "kLoLIngameMT32SfxIndex";
	case kLoLIngamePcSpkSfxIndex:
		return "kLoLIngamePcSpkSfxIndex";
	case kLoLSpellProperties:
		return "kLoLSpellProperties";
	case kLoLGameShapeMap:
		return "kLoLGameShapeMap";
	case kLoLSceneItemOffs:
		return "kLoLSceneItemOffs";
	case kLoLCharInvIndex:
		return "kLoLCharInvIndex";
	case kLoLCharInvDefs:
		return "kLoLCharInvDefs";
	case kLoLCharDefsMan:
		return "kLoLCharDefsMan";
	case kLoLCharDefsWoman:
		return "kLoLCharDefsWoman";
	case kLoLCharDefsKieran:
		return "kLoLCharDefsKieran";
	case kLoLCharDefsAkshel:
		return "kLoLCharDefsAkshel";
	case kLoLExpRequirements:
		return "kLoLExpRequirements";
	case kLoLMonsterModifiers:
		return "kLoLMonsterModifiers";
	case kLoLMonsterShiftOffsets:
		return "kLoLMonsterShiftOffsets";
	case kLoLMonsterDirFlags:
		return "kLoLMonsterDirFlags";
	case kLoLMonsterScaleY:
		return "kLoLMonsterScaleY";
	case kLoLMonsterScaleX:
		return "kLoLMonsterScaleX";
	case kLoLMonsterScaleWH:
		return "kLoLMonsterScaleWH";
	case kLoLFlyingObjectShp:
		return "kLoLFlyingObjectShp";
	case kLoLInventoryDesc:
		return "kLoLInventoryDesc";
	case kLoLLevelShpList:
		return "kLoLLevelShpList";
	case kLoLLevelDatList:
		return "kLoLLevelDatList";
	case kLoLCompassDefs:
		return "kLoLCompassDefs";
	case kLoLItemPrices:
		return "kLoLItemPrices";
	case kLoLStashSetup:
		return "kLoLStashSetup";
	case kLoLDscWalls:
		return "kLoLDscWalls";
	case kRpgCommonDscShapeIndex:
		return "kRpgCommonDscShapeIndex";
	case kLoLDscOvlMap:
		return "kLoLDscOvlMap";
	case kLoLDscScaleWidthData:
		return "kLoLDscScaleWidthData";
	case kLoLDscScaleHeightData:
		return "kLoLDscScaleHeightData";
	case kRpgCommonDscX:
		return "kRpgCommonDscX";
	case kLoLDscY:
		return "kLoLDscY";
	case kRpgCommonDscTileIndex:
		return "kRpgCommonDscTileIndex";
	case kRpgCommonDscUnk2:
		return "kRpgCommonDscUnk2";
	case kRpgCommonDscDoorShapeIndex:
		return "kRpgCommonDscDoorShapeIndex";
	case kRpgCommonDscDimData1:
		return "kRpgCommonDscDimData1";
	case kRpgCommonDscDimData2:
		return "kRpgCommonDscDimData2";
	case kRpgCommonDscBlockMap:
		return "kRpgCommonDscBlockMap";
	case kRpgCommonDscDimMap:
		return "kRpgCommonDscDimMap";
	case kLoLDscOvlIndex:
		return "kLoLDscOvlIndex";
	case kRpgCommonDscBlockIndex:
		return "kRpgCommonDscBlockIndex";
	case kRpgCommonDscDoorY2:
		return "kRpgCommonDscDoorY2";
	case kRpgCommonDscDoorFrameY1:
		return "kRpgCommonDscDoorFrameY1";
	case kRpgCommonDscDoorFrameY2:
		return "kRpgCommonDscDoorFrameY2";
	case kRpgCommonDscDoorFrameIndex1:
		return "kRpgCommonDscDoorFrameIndex1";
	case kRpgCommonDscDoorFrameIndex2:
		return "kRpgCommonDscDoorFrameIndex2";
	case kLoLDscDoorScale:
		return "kLoLDscDoorScale";
	case kLoLDscDoor4:
		return "kLoLDscDoor4";
	case kLoLDscDoorX:
		return "kLoLDscDoorX";
	case kLoLDscDoorY:
		return "kLoLDscDoorY";
	case kLoLScrollXTop:
		return "kLoLScrollXTop";
	case kLoLScrollYTop:
		return "kLoLScrollYTop";
	case kLoLScrollXBottom:
		return "kLoLScrollXBottom";
	case kLoLScrollYBottom:
		return "kLoLScrollYBottom";
	case kLoLButtonDefs:
		return "kLoLButtonDefs";
	case kLoLButtonList1:
		return "kLoLButtonList1";
	case kLoLButtonList2:
		return "kLoLButtonList2";
	case kLoLButtonList3:
		return "kLoLButtonList3";
	case kLoLButtonList4:
		return "kLoLButtonList4";
	case kLoLButtonList5:
		return "kLoLButtonList5";
	case kLoLButtonList6:
		return "kLoLButtonList6";
	case kLoLButtonList7:
		return "kLoLButtonList7";
	case kLoLButtonList8:
		return "kLoLButtonList8";
	case kLoLLegendData:
		return "kLoLLegendData";
	case kLoLMapCursorOvl:
		return "kLoLMapCursorOvl";
	case kLoLMapStringId:
		return "kLoLMapStringId";
	case kLoLSpellbookAnim:
		return "kLoLSpellbookAnim";
	case kLoLSpellbookCoords:
		return "kLoLSpellbookCoords";
	case kLoLHealShapeFrames:
		return "kLoLHealShapeFrames";
	case kLoLLightningDefs:
		return "kLoLLightningDefs";
	case kLoLFireballCoords:
		return "kLoLFireballCoords";
	case kLoLHistory:
		return "kLoLHistory";
	default:
		return "Unknown";
	}
}

struct ExtractData {
	ExtractData() : desc(), offset() {}
	ExtractData(ExtractEntrySearchData d, uint32 o) : desc(d), offset(o) {}

	ExtractEntrySearchData desc;
	uint32 offset;
};

typedef std::pair<int, ExtractEntrySearchData> SearchMapEntry;
typedef std::multimap<int, ExtractEntrySearchData> SearchMap;

typedef std::pair<int, ExtractData> ExtractMapEntry;
typedef std::multimap<int, ExtractData> ExtractMap;

bool getExtractionData(const Game *g, Search &search, ExtractMap &map);

bool createIDMap(PAKFile &out, const ExtractInformation *eI, const int *needList);

bool process(PAKFile &out, const Game *g, const byte *data, const uint32 size) {
	char filename[128];

	Search search(data, size);
	ExtractMap ids;

	if (!getExtractionData(g, search, ids))
		return false;

	const int *needList = getNeedList(g);
	if (!needList) {
		fprintf(stderr, "ERROR: No entry need list available\n");
		return false;
	}

	ExtractInformation extractInfo;
	extractInfo.game = g->game;
	extractInfo.platform = g->platform;
	extractInfo.special = g->special;

	for (ExtractMap::const_iterator i = ids.begin(); i != ids.end(); ++i) {
		const int id = i->first;
		extractInfo.lang = i->second.desc.lang;

		const ExtractFilename *fDesc = getFilenameDesc(id);

		if (!fDesc) {
			fprintf(stderr, "ERROR: couldn't find file description for id %d/%s\n", id, getIdString(id));
			return false;
		}

		filename[0] = 0;
		if (!getFilename(filename, &extractInfo, id)) {
			fprintf(stderr, "ERROR: couldn't get filename for id %d/%s\n", id, getIdString(id));
			return false;
		}

		const ExtractType *tDesc = findExtractType(fDesc->type);

		if (!tDesc) {
			fprintf(stderr, "ERROR: couldn't find type description for id %d/%s (%d)\n", id, getIdString(id), fDesc->type);
			return false;
		}

		PAKFile::cFileList *list = out.getFileList();
		if (list && list->findEntry(filename) != 0)
			continue;

		if (!tDesc->extract(out, &extractInfo, data + i->second.offset, i->second.desc.hint.size, filename, id)) {
			fprintf(stderr, "ERROR: couldn't extract id %d/%s\n", id, getIdString(id));
			return false;
		}
	}

	for (int i = 0; i < 3; ++i) {
		if (g->lang[i] == -1)
			continue;

		extractInfo.lang = g->lang[i];
		if (!createIDMap(out, &extractInfo, needList))
			return false;

		if (!updateIndex(out, &extractInfo)) {
			error("couldn't update INDEX file, stop processing of all files");
			return false;
		}
	}

	return true;
}

bool createIDMap(PAKFile &out, const ExtractInformation *eI, const int *needList) {
	int dataEntries = 0;
	// Count entries in the need list
	for (const int *n = needList; *n != -1; ++n)
		++dataEntries;

	const int mapSize = 2 + dataEntries * (2 + 1 + 4);
	uint8 *map = new uint8[mapSize];
	uint8 *dst = map;

	WRITE_BE_UINT16(dst, dataEntries); dst += 2;
	for (const int *id = needList; *id != -1; ++id) {
		WRITE_BE_UINT16(dst, *id); dst += 2;
		const ExtractFilename *fDesc = getFilenameDesc(*id);
		if (!fDesc)
			return false;
		*dst++ = getTypeID(fDesc->type);
		WRITE_BE_UINT32(dst, getFilename(eI, *id)); dst += 4;
	}

	char filename[12];
	if (!getFilename(filename, eI, 0)) {
		fprintf(stderr, "ERROR: Could not create ID map for game\n");
		return false;
	}

	out.removeFile(filename);
	if (!out.addFile(filename, map, mapSize)) {
		fprintf(stderr, "ERROR: Could not add ID map \"%s\" to kyra.dat\n", filename);
		return false;
	}

	return true;
}

// Uncomment this to get various debug information about the detection table entries.
//#define DEBUG_EXTRACTION_TABLES

bool setupSearch(const Game *g, const int *needList, Search &search, SearchMap &searchData) {
	for (const int *entry = needList; *entry != -1; ++entry) {
		ExtractEntryList providers = getProvidersForId(*entry);

		if (providers.empty()) {
			fprintf(stderr, "ERROR: No provider for id %d/%s\n", *entry, getIdString(*entry));
			return false;
		} else {
			for (ExtractEntryList::const_iterator i = providers.begin(); i != providers.end(); ++i) {
				// Only add generic or partly matching providers here.
#ifndef DEBUG_EXTRACTION_TABLES
				if ((i->lang == UNK_LANG || i->lang == g->lang[0] || i->lang == g->lang[1] || i->lang == g->lang[2]) &&
				    (i->platform == kPlatformUnknown || (i->platform == g->platform))) {
#endif
					search.addData(i->hint);
					searchData.insert(SearchMapEntry(*entry, *i));
#ifndef DEBUG_EXTRACTION_TABLES
				}
#endif
			}
		}
	}

	return true;
}

typedef std::list<ExtractMap::const_iterator> MatchList;
MatchList filterPlatformMatches(const Game *g, std::pair<ExtractMap::const_iterator, ExtractMap::const_iterator> range) {
	bool hasPlatformMatch = false;
	for (ExtractMap::const_iterator i = range.first; i != range.second; ++i) {
		if (i->second.desc.platform == g->platform) {
			hasPlatformMatch = true;
			break;
		}
	}

	MatchList result;
	if (hasPlatformMatch) {
		for (ExtractMap::const_iterator i = range.first; i != range.second; ++i) {
			if (i->second.desc.platform == g->platform)
				result.push_back(i);
		}
	} else {
		for (ExtractMap::const_iterator i = range.first; i != range.second; ++i)
			result.push_back(i);
	}

	return result;
}

MatchList filterLanguageMatches(const int lang, const MatchList &input) {
	std::list<ExtractMap::const_iterator> result;

	for (MatchList::const_iterator i = input.begin(); i != input.end(); ++i) {
		if ((*i)->second.desc.lang == lang)
			result.push_back(*i);
	}

	return result;
}

MatchList::const_iterator filterOutBestMatch(const MatchList &input) {
	MatchList::const_iterator result = input.begin();

	if (input.size() > 1)
		warning("Multiple entries found for id %d/%s", (*result)->first, getIdString((*result)->first));

	for (MatchList::const_iterator i = input.begin(); i != input.end(); ++i) {
		// Reduce all entries to one single entry.
		//
		// We use the following rules for this (in this order):
		// - Prefer the entry with the higest size
		// - Prefer the entry, which starts at the smallest offest
		//
		// TODO: These rules might not be safe for all games, but hopefully
		// they will work fine. If there are any problems it should be rather
		// easy to identify them, since we print out a warning for multiple
		// entries found.
		if ((*result)->second.desc.hint.size <= (*i)->second.desc.hint.size) {
			if ((*result)->second.offset >= (*i)->second.offset)
				result = i;
		}
	}

	return result;
}

bool getExtractionData(const Game *g, Search &search, ExtractMap &map) {
	SearchMap searchMap;

	const int *needList = getNeedList(g);
	if (!needList) {
		fprintf(stderr, "ERROR: No entry need list available\n");
		return false;
	}

	if (!setupSearch(g, needList, search, searchMap))
		return false;

	// Process the data search
	Search::ResultList results;
	search.search(results);

	if (results.empty()) {
		fprintf(stderr, "ERROR: Couldn't find any required data\n");
		return false;
	}

	ExtractMap temporaryExtractMap;
	for (const int *entry = needList; *entry != -1; ++entry) {
		typedef std::pair<SearchMap::const_iterator, SearchMap::const_iterator> KeyRange;
		KeyRange idRange = searchMap.equal_range(*entry);

		for (Search::ResultList::const_iterator i = results.begin(); i != results.end(); ++i) {
			for (SearchMap::const_iterator j = idRange.first; j != idRange.second; ++j) {
				if (j->second.hint == i->data)
					temporaryExtractMap.insert(ExtractMapEntry(*entry, ExtractData(j->second, i->offset)));
			}
		}
	}

	// Free up some memory
	results.clear();
	searchMap.clear();

	bool result = true;

	for (const int *entry = needList; *entry != -1; ++entry) {
		MatchList possibleMatches = filterPlatformMatches(g, temporaryExtractMap.equal_range(*entry));

		if (possibleMatches.empty()) {
			fprintf(stderr, "ERROR: No entry found for id %d/%s\n", *entry, getIdString(*entry));
			result = false;
			continue;
		}

		if (isLangSpecific(*entry)) {
			for (int i = 0; i < 3; ++i) {
				if (g->lang[i] == -1)
					continue;

				MatchList langMatches = filterLanguageMatches(g->lang[i], possibleMatches);
				MatchList::const_iterator bestMatch = filterOutBestMatch(langMatches);

				if (bestMatch == langMatches.end()) {
					// TODO: Add nice language name to output message.
					fprintf(stderr, "ERROR: No entry found for id %d/%s for language %d\n", *entry, getIdString(*entry), g->lang[i]);
					result = false;
					continue;
				}

#ifdef DEBUG_EXTRACTION_TABLES
				if (((*bestMatch)->second.desc.platform != kPlatformUnknown && (*bestMatch)->second.desc.platform != g->platform))
					printf("%s: %.8X %.8X %d %d\n", getIdString(*entry), (*bestMatch)->second.desc.hint.size, (*bestMatch)->second.desc.hint.byteSum, (*bestMatch)->second.desc.lang, (*bestMatch)->second.desc.platform);
#endif

				map.insert(**bestMatch);
			}
		} else {
			MatchList::const_iterator bestMatch = filterOutBestMatch(possibleMatches);

			if (bestMatch == possibleMatches.end()) {
				fprintf(stderr, "ERROR: No entry found for id %d/%s\n", *entry, getIdString(*entry));
				result = false;
				continue;
			}

#ifdef DEBUG_EXTRACTION_TABLES
			if (((*bestMatch)->second.desc.platform != kPlatformUnknown && (*bestMatch)->second.desc.platform != g->platform))
				printf("%s: %.8X %.8X %d %d\n", getIdString(*entry), (*bestMatch)->second.desc.hint.size, (*bestMatch)->second.desc.hint.byteSum, (*bestMatch)->second.desc.lang, (*bestMatch)->second.desc.platform);
#endif

			map.insert(**bestMatch);
		}
	}

	return result;
}
