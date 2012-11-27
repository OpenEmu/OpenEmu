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

#ifndef CREATE_KYRADAT_CREATE_KYRADAT_H
#define CREATE_KYRADAT_CREATE_KYRADAT_H

#include "util.h"

// This list has to match orderwise (and thus value wise) the static data list of "engines/kyra/resource.h"!
enum kExtractID {
	kIdMap = 0,

	k1ForestSeq = 1,
	k1KallakWritingSeq,
	k1KyrandiaLogoSeq,
	k1KallakMalcolmSeq,
	k1MalcolmTreeSeq,
	k1WestwoodLogoSeq,

	k1Demo1Seq,
	k1Demo2Seq,
	k1Demo3Seq,
	k1Demo4Seq,

	k1AmuleteAnimSeq,

	k1OutroReunionSeq,

	k1IntroCPSStrings,
	k1IntroCOLStrings,
	k1IntroWSAStrings,
	k1IntroStrings,

	k1OutroHomeString,

	k1RoomFilenames,
	k1RoomList,

	k1CharacterImageFilenames,

	k1ItemNames,
	k1TakenStrings,
	k1PlacedStrings,
	k1DroppedStrings,
	k1NoDropStrings,

	k1PutDownString,
	k1WaitAmuletString,
	k1BlackJewelString,
	k1PoisonGoneString,
	k1HealingTipString,
	k1WispJewelStrings,
	k1MagicJewelStrings,

	k1ThePoisonStrings,
	k1FluteStrings,

	k1FlaskFullString,
	k1FullFlaskString,

	k1VeryCleverString,
	k1NewGameString,

	k1DefaultShapes,
	k1Healing1Shapes,
	k1Healing2Shapes,
	k1PoisonDeathShapes,
	k1FluteShapes,
	k1Winter1Shapes,
	k1Winter2Shapes,
	k1Winter3Shapes,
	k1DrinkShapes,
	k1WispShapes,
	k1MagicAnimShapes,
	k1BranStoneShapes,

	k1SpecialPalette1,
	k1SpecialPalette2,
	k1SpecialPalette3,
	k1SpecialPalette4,
	k1SpecialPalette5,
	k1SpecialPalette6,
	k1SpecialPalette7,
	k1SpecialPalette8,
	k1SpecialPalette9,
	k1SpecialPalette10,
	k1SpecialPalette11,
	k1SpecialPalette12,
	k1SpecialPalette13,
	k1SpecialPalette14,
	k1SpecialPalette15,
	k1SpecialPalette16,
	k1SpecialPalette17,
	k1SpecialPalette18,
	k1SpecialPalette19,
	k1SpecialPalette20,
	k1SpecialPalette21,
	k1SpecialPalette22,
	k1SpecialPalette23,
	k1SpecialPalette24,
	k1SpecialPalette25,
	k1SpecialPalette26,
	k1SpecialPalette27,
	k1SpecialPalette28,
	k1SpecialPalette29,
	k1SpecialPalette30,
	k1SpecialPalette31,
	k1SpecialPalette32,
	k1SpecialPalette33,

	k1GUIStrings,
	k1ConfigStrings,

	k1AudioTracks,
	k1AudioTracks2,
	k1AudioTracksIntro,

	k1CreditsStrings,

	k1TownsMusicFadeTable,
	k1TownsSFXwdTable,
	k1TownsSFXbtTable,
	k1TownsCDATable,

	k1PC98StoryStrings,
	k1PC98IntroSfx,

	k1AmigaIntroSFXTable,
	k1AmigaGameSFXTable,

	k2SeqplayPakFiles,
	k2SeqplayCredits,
	k2SeqplayCreditsSpecial,
	k2SeqplayStrings,
	k2SeqplaySfxFiles,
	k2SeqplayTlkFiles,
	k2SeqplaySeqData,
	k2SeqplayIntroTracks,
	k2SeqplayFinaleTracks,
	k2SeqplayIntroCDA,
	k2SeqplayFinaleCDA,
	k2SeqplayShapeAnimData,

	k2IngamePakFiles,
	k2IngameSfxFiles,
	k2IngameSfxIndex,
	k2IngameTracks,
	k2IngameCDA,
	k2IngameTalkObjIndex,
	k2IngameTimJpStrings,
	k2IngameShapeAnimData,
	k2IngameTlkDemoStrings,

	k3MainMenuStrings,
	k3MusicFiles,
	k3ScoreTable,
	k3SfxFiles,
	k3SfxMap,
	k3ItemAnimData,
	k3ItemMagicTable,
	k3ItemStringMap,

	kRpgCommonMoreStrings,
	kRpgCommonDscShapeIndex,
	kRpgCommonDscX,
	kRpgCommonDscTileIndex,
	kRpgCommonDscUnk2,
	kRpgCommonDscDoorShapeIndex,
	kRpgCommonDscDimData1,
	kRpgCommonDscDimData2,
	kRpgCommonDscBlockMap,
	kRpgCommonDscDimMap,
	kRpgCommonDscDoorY2,
	kRpgCommonDscDoorFrameY1,
	kRpgCommonDscDoorFrameY2,
	kRpgCommonDscDoorFrameIndex1,
	kRpgCommonDscDoorFrameIndex2,
	kRpgCommonDscBlockIndex,

	kEoBBaseChargenStrings1,
	kEoBBaseChargenStrings2,
	kEoBBaseChargenStartLevels,
	kEoBBaseChargenStatStrings,
	kEoBBaseChargenRaceSexStrings,
	kEoBBaseChargenClassStrings,
	kEoBBaseChargenAlignmentStrings,
	kEoBBaseChargenEnterGameStrings,
	kEoBBaseChargenClassMinStats,
	kEoBBaseChargenRaceMinStats,
	kEoBBaseChargenRaceMaxStats,

	kEoBBaseSaveThrowTable1,
	kEoBBaseSaveThrowTable2,
	kEoBBaseSaveThrowTable3,
	kEoBBaseSaveThrowTable4,
	kEoBBaseSaveThrwLvlIndex,
	kEoBBaseSaveThrwModDiv,
	kEoBBaseSaveThrwModExt,

	kEoBBasePryDoorStrings,
	kEoBBaseWarningStrings,

	kEoBBaseItemSuffixStringsRings,
	kEoBBaseItemSuffixStringsPotions,
	kEoBBaseItemSuffixStringsWands,

	kEoBBaseRipItemStrings,
	kEoBBaseCursedString,
	kEoBBaseEnchantedString,
	kEoBBaseMagicObjectStrings,
	kEoBBaseMagicObject5String,
	kEoBBasePatternSuffix,
	kEoBBasePatternGrFix1,
	kEoBBasePatternGrFix2,
	kEoBBaseValidateArmorString,
	kEoBBaseValidateCursedString,
	kEoBBaseValidateNoDropString,
	kEoBBasePotionStrings,
	kEoBBaseWandString,
	kEoBBaseItemMisuseStrings,

	kEoBBaseTakenStrings,
	kEoBBasePotionEffectStrings,

	kEoBBaseYesNoStrings,
	kEoBBaseNpcMaxStrings,
	kEoBBaseOkStrings,
	kEoBBaseNpcJoinStrings,
	kEoBBaseCancelStrings,
	kEoBBaseAbortStrings,

	kEoBBaseMenuStringsMain,
	kEoBBaseMenuStringsSaveLoad,
	kEoBBaseMenuStringsOnOff,
	kEoBBaseMenuStringsSpells,
	kEoBBaseMenuStringsRest,
	kEoBBaseMenuStringsDrop,
	kEoBBaseMenuStringsExit,
	kEoBBaseMenuStringsStarve,
	kEoBBaseMenuStringsScribe,
	kEoBBaseMenuStringsDrop2,
	kEoBBaseMenuStringsHead,
	kEoBBaseMenuStringsPoison,
	kEoBBaseMenuStringsMgc,
	kEoBBaseMenuStringsPrefs,
	kEoBBaseMenuStringsRest2,
	kEoBBaseMenuStringsRest3,
	kEoBBaseMenuStringsRest4,
	kEoBBaseMenuStringsDefeat,
	kEoBBaseMenuStringsTransfer,
	kEoBBaseMenuStringsSpec,
	kEoBBaseMenuStringsSpellNo,
	kEoBBaseMenuYesNoStrings,

	kEoBBaseSpellLevelsMage,
	kEoBBaseSpellLevelsCleric,
	kEoBBaseNumSpellsCleric,
	kEoBBaseNumSpellsWisAdj,
	kEoBBaseNumSpellsPal,
	kEoBBaseNumSpellsMage,

	kEoBBaseCharGuiStringsHp,
	kEoBBaseCharGuiStringsWp1,
	kEoBBaseCharGuiStringsWp2,
	kEoBBaseCharGuiStringsWr,
	kEoBBaseCharGuiStringsSt1,
	kEoBBaseCharGuiStringsSt2,
	kEoBBaseCharGuiStringsIn,

	kEoBBaseCharStatusStrings7,
	kEoBBaseCharStatusStrings81,
	kEoBBaseCharStatusStrings82,
	kEoBBaseCharStatusStrings9,
	kEoBBaseCharStatusStrings12,
	kEoBBaseCharStatusStrings131,
	kEoBBaseCharStatusStrings132,

	kEoBBaseLevelGainStrings,
	kEoBBaseExperienceTable0,
	kEoBBaseExperienceTable1,
	kEoBBaseExperienceTable2,
	kEoBBaseExperienceTable3,
	kEoBBaseExperienceTable4,

	kEoBBaseClassModifierFlags,

	kEoBBaseMonsterStepTable01,
	kEoBBaseMonsterStepTable02,
	kEoBBaseMonsterStepTable1,
	kEoBBaseMonsterStepTable2,
	kEoBBaseMonsterStepTable3,
	kEoBBaseMonsterCloseAttPosTable1,
	kEoBBaseMonsterCloseAttPosTable21,
	kEoBBaseMonsterCloseAttPosTable22,
	kEoBBaseMonsterCloseAttUnkTable,
	kEoBBaseMonsterCloseAttChkTable1,
	kEoBBaseMonsterCloseAttChkTable2,
	kEoBBaseMonsterCloseAttDstTable1,
	kEoBBaseMonsterCloseAttDstTable2,

	kEoBBaseMonsterProximityTable,
	kEoBBaseFindBlockMonstersTable,
	kEoBBaseMonsterDirChangeTable,
	kEoBBaseMonsterDistAttStrings,

	kEoBBaseEncodeMonsterDefs,
	kEoBBaseNpcPresets,

	kEoBBaseWllFlagPreset,
	kEoBBaseDscShapeCoords,

	kEoBBaseDscDoorScaleOffs,
	kEoBBaseDscDoorScaleMult1,
	kEoBBaseDscDoorScaleMult2,
	kEoBBaseDscDoorScaleMult3,
	kEoBBaseDscDoorScaleMult4,
	kEoBBaseDscDoorScaleMult5,
	kEoBBaseDscDoorScaleMult6,
	kEoBBaseDscDoorType5Offs,
	kEoBBaseDscDoorXE,
	kEoBBaseDscDoorY1,
	kEoBBaseDscDoorY3,
	kEoBBaseDscDoorY4,
	kEoBBaseDscDoorY5,
	kEoBBaseDscDoorY6,
	kEoBBaseDscDoorY7,
	kEoBBaseDscDoorCoordsExt,

	kEoBBaseDscItemPosIndex,
	kEoBBaseDscItemShpX,
	kEoBBaseDscItemPosUnk,
	kEoBBaseDscItemTileIndex,
	kEoBBaseDscItemShapeMap,

	kEoBBaseDscMonsterFrmOffsTbl1,
	kEoBBaseDscMonsterFrmOffsTbl2,

	kEoBBaseInvSlotX,
	kEoBBaseInvSlotY,
	kEoBBaseSlotValidationFlags,

	kEoBBaseProjectileWeaponTypes,
	kEoBBaseWandTypes,

	kEoBBaseDrawObjPosIndex,
	kEoBBaseFlightObjFlipIndex,
	kEoBBaseFlightObjShpMap,
	kEoBBaseFlightObjSclIndex,

	kEoBBaseDscTelptrShpCoords,

	kEoBBasePortalSeqData,
	kEoBBaseManDef,
	kEoBBaseManWord,
	kEoBBaseManPrompt,

	kEoBBaseBookNumbers,
	kEoBBaseMageSpellsList,
	kEoBBaseClericSpellsList,
	kEoBBaseSpellNames,
	kEoBBaseMagicStrings1,
	kEoBBaseMagicStrings2,
	kEoBBaseMagicStrings3,
	kEoBBaseMagicStrings4,
	kEoBBaseMagicStrings6,
	kEoBBaseMagicStrings7,
	kEoBBaseMagicStrings8,

	kEoBBaseExpObjectTlMode,
	kEoBBaseExpObjectTblIndex,
	kEoBBaseExpObjectShpStart,
	kEoBBaseExpObjectTbl1,
	kEoBBaseExpObjectTbl2,
	kEoBBaseExpObjectTbl3,
	kEoBBaseExpObjectY,

	kEoBBaseSparkDefSteps,
	kEoBBaseSparkDefSubSteps,
	kEoBBaseSparkDefShift,
	kEoBBaseSparkDefAdd,
	kEoBBaseSparkDefX,
	kEoBBaseSparkDefY,
	kEoBBaseSparkOfFlags1,
	kEoBBaseSparkOfFlags2,
	kEoBBaseSparkOfShift,
	kEoBBaseSparkOfX,
	kEoBBaseSparkOfY,

	kEoBBaseSpellProperties,
	kEoBBaseMagicFlightProps,
	kEoBBaseTurnUndeadEffect,
	kEoBBaseBurningHandsDest,
	kEoBBaseConeOfColdDest1,
	kEoBBaseConeOfColdDest2,
	kEoBBaseConeOfColdDest3,
	kEoBBaseConeOfColdDest4,
	kEoBBaseConeOfColdGfxTbl,

	kEoB1MainMenuStrings,
	kEoB1BonusStrings,

	kEoB1IntroFilesOpening,
	kEoB1IntroFilesTower,
	kEoB1IntroFilesOrb,
	kEoB1IntroFilesWdEntry,
	kEoB1IntroFilesKing,
	kEoB1IntroFilesHands,
	kEoB1IntroFilesWdExit,
	kEoB1IntroFilesTunnel,
	kEoB1IntroOpeningFrmDelay,
	kEoB1IntroWdEncodeX,
	kEoB1IntroWdEncodeY,
	kEoB1IntroWdEncodeWH,
	kEoB1IntroWdDsX,
	kEoB1IntroWdDsY,
	kEoB1IntroTvlX1,
	kEoB1IntroTvlY1,
	kEoB1IntroTvlX2,
	kEoB1IntroTvlY2,
	kEoB1IntroTvlW,
	kEoB1IntroTvlH,

	kEoB1DoorShapeDefs,
	kEoB1DoorSwitchShapeDefs,
	kEoB1DoorSwitchCoords,
	kEoB1MonsterProperties,

	kEoB1EnemyMageSpellList,
	kEoB1EnemyMageSfx,
	kEoB1BeholderSpellList,
	kEoB1BeholderSfx,
	kEoB1TurnUndeadString,

	kEoB1CgaMappingDefault,
	kEoB1CgaMappingAlt,
	kEoB1CgaMappingInv,
	kEoB1CgaMappingItemsL,
	kEoB1CgaMappingItemsS,
	kEoB1CgaMappingThrown,
	kEoB1CgaMappingIcons,
	kEoB1CgaMappingDeco,
	kEoB1CgaLevelMappingIndex,
	kEoB1CgaMappingLevel0,
	kEoB1CgaMappingLevel1,
	kEoB1CgaMappingLevel2,
	kEoB1CgaMappingLevel3,
	kEoB1CgaMappingLevel4,

	kEoB1NpcShpData,
	kEoB1NpcSubShpIndex1,
	kEoB1NpcSubShpIndex2,
	kEoB1NpcSubShpY,
	kEoB1Npc0Strings,
	kEoB1Npc11Strings,
	kEoB1Npc12Strings,
	kEoB1Npc21Strings,
	kEoB1Npc22Strings,
	kEoB1Npc31Strings,
	kEoB1Npc32Strings,
	kEoB1Npc4Strings,
	kEoB1Npc5Strings,
	kEoB1Npc6Strings,
	kEoB1Npc7Strings,

	kEoB2MainMenuStrings,

	kEoB2TransferPortraitFrames,
	kEoB2TransferConvertTable,
	kEoB2TransferItemTable,
	kEoB2TransferExpTable,
	kEoB2TransferStrings1,
	kEoB2TransferStrings2,
	kEoB2TransferLabels,

	kEoB2IntroStrings,
	kEoB2IntroCPSFiles,
	kEob2IntroAnimData00,
	kEob2IntroAnimData01,
	kEob2IntroAnimData02,
	kEob2IntroAnimData03,
	kEob2IntroAnimData04,
	kEob2IntroAnimData05,
	kEob2IntroAnimData06,
	kEob2IntroAnimData07,
	kEob2IntroAnimData08,
	kEob2IntroAnimData09,
	kEob2IntroAnimData10,
	kEob2IntroAnimData11,
	kEob2IntroAnimData12,
	kEob2IntroAnimData13,
	kEob2IntroAnimData14,
	kEob2IntroAnimData15,
	kEob2IntroAnimData16,
	kEob2IntroAnimData17,
	kEob2IntroAnimData18,
	kEob2IntroAnimData19,
	kEob2IntroAnimData20,
	kEob2IntroAnimData21,
	kEob2IntroAnimData22,
	kEob2IntroAnimData23,
	kEob2IntroAnimData24,
	kEob2IntroAnimData25,
	kEob2IntroAnimData26,
	kEob2IntroAnimData27,
	kEob2IntroAnimData28,
	kEob2IntroAnimData29,
	kEob2IntroAnimData30,
	kEob2IntroAnimData31,
	kEob2IntroAnimData32,
	kEob2IntroAnimData33,
	kEob2IntroAnimData34,
	kEob2IntroAnimData35,
	kEob2IntroAnimData36,
	kEob2IntroAnimData37,
	kEob2IntroAnimData38,
	kEob2IntroAnimData39,
	kEob2IntroAnimData40,
	kEob2IntroAnimData41,
	kEob2IntroAnimData42,
	kEob2IntroAnimData43,
	kEoB2IntroShapes00,
	kEoB2IntroShapes01,
	kEoB2IntroShapes04,
	kEoB2IntroShapes07,

	kEoB2FinaleStrings,
	kEoB2CreditsData,
	kEoB2FinaleCPSFiles,
	kEob2FinaleAnimData00,
	kEob2FinaleAnimData01,
	kEob2FinaleAnimData02,
	kEob2FinaleAnimData03,
	kEob2FinaleAnimData04,
	kEob2FinaleAnimData05,
	kEob2FinaleAnimData06,
	kEob2FinaleAnimData07,
	kEob2FinaleAnimData08,
	kEob2FinaleAnimData09,
	kEob2FinaleAnimData10,
	kEob2FinaleAnimData11,
	kEob2FinaleAnimData12,
	kEob2FinaleAnimData13,
	kEob2FinaleAnimData14,
	kEob2FinaleAnimData15,
	kEob2FinaleAnimData16,
	kEob2FinaleAnimData17,
	kEob2FinaleAnimData18,
	kEob2FinaleAnimData19,
	kEob2FinaleAnimData20,
	kEoB2FinaleShapes00,
	kEoB2FinaleShapes03,
	kEoB2FinaleShapes07,
	kEoB2FinaleShapes09,
	kEoB2FinaleShapes10,

	kEoB2NpcShapeData,
	kEoB2Npc1Strings,
	kEoB2Npc2Strings,
	kEoB2MonsterDustStrings,

	kEoB2DreamSteps,
	kEoB2KheldranStrings,
	kEoB2HornStrings,
	kEoB2HornSounds,

	kEoB2WallOfForceDsX,
	kEoB2WallOfForceDsY,
	kEoB2WallOfForceNumW,
	kEoB2WallOfForceNumH,
	kEoB2WallOfForceShpId,

	kLoLIngamePakFiles,
	kLoLCharacterDefs,
	kLoLIngameSfxFiles,
	kLoLIngameSfxIndex,
	kLoLMusicTrackMap,
	kLoLIngameGMSfxIndex,
	kLoLIngameMT32SfxIndex,
	kLoLIngamePcSpkSfxIndex,
	kLoLSpellProperties,
	kLoLGameShapeMap,
	kLoLSceneItemOffs,
	kLoLCharInvIndex,
	kLoLCharInvDefs,
	kLoLCharDefsMan,
	kLoLCharDefsWoman,
	kLoLCharDefsKieran,
	kLoLCharDefsAkshel,
	kLoLExpRequirements,
	kLoLMonsterModifiers,
	kLoLMonsterShiftOffsets,
	kLoLMonsterDirFlags,
	kLoLMonsterScaleY,
	kLoLMonsterScaleX,
	kLoLMonsterScaleWH,
	kLoLFlyingObjectShp,
	kLoLInventoryDesc,

	kLoLLevelShpList,
	kLoLLevelDatList,
	kLoLCompassDefs,
	kLoLItemPrices,
	kLoLStashSetup,

	kLoLDscWalls,
	kLoLDscOvlMap,
	kLoLDscScaleWidthData,
	kLoLDscScaleHeightData,
	kLoLDscY,

	kLoLDscDoorScale,
	kLoLDscDoor4,
	kLoLDscDoorX,
	kLoLDscDoorY,
	kLoLDscOvlIndex,

	kLoLScrollXTop,
	kLoLScrollYTop,
	kLoLScrollXBottom,
	kLoLScrollYBottom,

	kLoLButtonDefs,
	kLoLButtonList1,
	kLoLButtonList2,
	kLoLButtonList3,
	kLoLButtonList4,
	kLoLButtonList5,
	kLoLButtonList6,
	kLoLButtonList7,
	kLoLButtonList8,

	kLoLLegendData,
	kLoLMapCursorOvl,
	kLoLMapStringId,

	kLoLSpellbookAnim,
	kLoLSpellbookCoords,
	kLoLHealShapeFrames,
	kLoLLightningDefs,
	kLoLFireballCoords,

	kLoLCredits,

	kLoLHistory,

	kMaxResIDs
};

struct ExtractFilename {
	int id;
	int type;
	bool langSpecific;
};

enum kSpecial {
	kNoSpecial = 0,
	kTalkieVersion,
	kDemoVersion,
	kTalkieDemoVersion,
	kOldFloppy
};

enum kGame {
	kKyra1 = 0,
	kKyra2,
	kKyra3,
	kLoL,
	kEoB1,
	kEoB2
};

struct Game {
	int game;
	int lang[3];
	int platform;
	int special;

	const char *md5[2];
};

#define GAME_DUMMY_ENTRY { -1, { -1, -1, -1 }, -1, -1, { 0, 0 } }

extern const Game * const gameDescs[];

const int *getNeedList(const Game *g);

struct TypeTable {
	int type;
	int value;

	bool operator==(int t) const {
		return (type == t);
	}
};

#endif
