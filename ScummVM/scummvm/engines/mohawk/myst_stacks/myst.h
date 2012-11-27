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

#ifndef MYST_SCRIPTS_MYST_H
#define MYST_SCRIPTS_MYST_H

#include "common/scummsys.h"
#include "common/util.h"
#include "mohawk/myst_scripts.h"

namespace Mohawk {

struct MystScriptEntry;

namespace MystStacks {

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 var, uint16 argc, uint16 *argv)

class Myst : public MystScriptParser {
public:
	Myst(MohawkEngine_Myst *vm);
	~Myst();

	virtual void disablePersistentScripts();
	virtual void runPersistentScripts();

private:
	void setupOpcodes();
	uint16 getVar(uint16 var);
	void toggleVar(uint16 var);
	bool setVarValue(uint16 var, uint16 value);

	virtual uint16 getMap() { return 9934; }

	void towerRotationMap_run();
	void libraryBookcaseTransform_run();
	void generatorControlRoom_run();
	void opcode_212_run();
	void libraryCombinationBook_run();
	void clockWheel_run();
	void matchBurn_run();
	void boilerPressureIncrease_run();
	void boilerPressureDecrease_run();
	void basementPressureIncrease_run();
	void basementPressureDecrease_run();
	void tree_run();
	void imagerValidation_run();
	void imager_run();
	void observatory_run();
	void observatoryMonthChange_run();
	void observatoryDayChange_run();
	void observatoryYearChange_run();
	void observatoryTimeChange_run();
	void greenBook_run();
	void clockGears_run();
	void gullsFly1_run();
	void gullsFly2_run();
	void gullsFly3_run();

	DECLARE_OPCODE(o_libraryBookPageTurnLeft);
	DECLARE_OPCODE(o_libraryBookPageTurnRight);
	DECLARE_OPCODE(o_fireplaceToggleButton);
	DECLARE_OPCODE(o_fireplaceRotation);
	DECLARE_OPCODE(o_courtyardBoxesCheckSolution);
	DECLARE_OPCODE(o_towerRotationStart);
	DECLARE_OPCODE(o_towerRotationEnd);
	DECLARE_OPCODE(o_imagerChangeSelection);
	DECLARE_OPCODE(o_dockVaultOpen);
	DECLARE_OPCODE(o_dockVaultClose);
	DECLARE_OPCODE(o_bookGivePage);
	DECLARE_OPCODE(o_clockWheelsExecute);
	DECLARE_OPCODE(o_imagerPlayButton);
	DECLARE_OPCODE(o_imagerEraseButton);
	DECLARE_OPCODE(o_towerElevatorAnimation);
	DECLARE_OPCODE(o_generatorButtonPressed);
	DECLARE_OPCODE(o_cabinSafeChangeDigit);
	DECLARE_OPCODE(o_cabinSafeHandleStartMove);
	DECLARE_OPCODE(o_cabinSafeHandleMove);
	DECLARE_OPCODE(o_cabinSafeHandleEndMove);
	DECLARE_OPCODE(o_treePressureReleaseStart);
	DECLARE_OPCODE(o_observatoryMonthChangeStart);
	DECLARE_OPCODE(o_observatoryDayChangeStart);
	DECLARE_OPCODE(o_observatoryGoButton);
	DECLARE_OPCODE(o_observatoryMonthSliderMove);
	DECLARE_OPCODE(o_observatoryDaySliderMove);
	DECLARE_OPCODE(o_observatoryYearSliderMove);
	DECLARE_OPCODE(o_observatoryTimeSliderMove);
	DECLARE_OPCODE(o_circuitBreakerStartMove);
	DECLARE_OPCODE(o_circuitBreakerMove);
	DECLARE_OPCODE(o_circuitBreakerEndMove);
	DECLARE_OPCODE(o_boilerIncreasePressureStart);
	DECLARE_OPCODE(o_boilerLightPilot);
	DECLARE_OPCODE(o_boilerIncreasePressureStop);
	DECLARE_OPCODE(o_boilerDecreasePressureStart);
	DECLARE_OPCODE(o_boilerDecreasePressureStop);
	DECLARE_OPCODE(o_basementIncreasePressureStart);
	DECLARE_OPCODE(o_basementIncreasePressureStop);
	DECLARE_OPCODE(o_basementDecreasePressureStart);
	DECLARE_OPCODE(o_basementDecreasePressureStop);
	DECLARE_OPCODE(o_rocketPianoStart);
	DECLARE_OPCODE(o_rocketPianoMove);
	DECLARE_OPCODE(o_rocketPianoStop);
	DECLARE_OPCODE(o_rocketSoundSliderStartMove);
	DECLARE_OPCODE(o_rocketSoundSliderMove);
	DECLARE_OPCODE(o_rocketSoundSliderEndMove);
	DECLARE_OPCODE(o_rocketLeverStartMove);
	DECLARE_OPCODE(o_rocketOpenBook);
	DECLARE_OPCODE(o_rocketLeverMove);
	DECLARE_OPCODE(o_rocketLeverEndMove);
	DECLARE_OPCODE(o_cabinLeave);
	DECLARE_OPCODE(o_treePressureReleaseStop);
	DECLARE_OPCODE(o_observatoryMonthSliderStartMove);
	DECLARE_OPCODE(o_observatoryMonthSliderEndMove);
	DECLARE_OPCODE(o_observatoryDaySliderStartMove);
	DECLARE_OPCODE(o_observatoryDaySliderEndMove);
	DECLARE_OPCODE(o_observatoryYearSliderStartMove);
	DECLARE_OPCODE(o_observatoryYearSliderEndMove);
	DECLARE_OPCODE(o_observatoryTimeSliderStartMove);
	DECLARE_OPCODE(o_observatoryTimeSliderEndMove);
	DECLARE_OPCODE(o_libraryCombinationBookStop);
	DECLARE_OPCODE(o_cabinMatchLight);
	DECLARE_OPCODE(o_courtyardBoxEnter);
	DECLARE_OPCODE(o_courtyardBoxLeave);
	DECLARE_OPCODE(o_clockMinuteWheelStartTurn);
	DECLARE_OPCODE(o_clockWheelEndTurn);
	DECLARE_OPCODE(o_clockHourWheelStartTurn);
	DECLARE_OPCODE(o_clockLeverStartMove);
	DECLARE_OPCODE(o_clockLeverMove);
	DECLARE_OPCODE(o_clockLeverEndMove);
	DECLARE_OPCODE(o_clockResetLeverStartMove);
	DECLARE_OPCODE(o_clockResetLeverMove);
	DECLARE_OPCODE(o_clockResetLeverEndMove);

	DECLARE_OPCODE(o_libraryCombinationBookStartRight);
	DECLARE_OPCODE(o_libraryCombinationBookStartLeft);
	DECLARE_OPCODE(o_observatoryTimeChangeStart);
	DECLARE_OPCODE(o_observatoryChangeSettingStop);
	DECLARE_OPCODE(o_observatoryYearChangeStart);
	DECLARE_OPCODE(o_dockVaultForceClose);
	DECLARE_OPCODE(o_imagerEraseStop);

	DECLARE_OPCODE(o_libraryBook_init);
	DECLARE_OPCODE(o_courtyardBox_init);
	DECLARE_OPCODE(o_towerRotationMap_init);
	DECLARE_OPCODE(o_forechamberDoor_init);
	DECLARE_OPCODE(o_shipAccess_init);
	DECLARE_OPCODE(o_butterflies_init);
	DECLARE_OPCODE(o_imager_init);
	DECLARE_OPCODE(o_libraryBookcaseTransform_init);
	DECLARE_OPCODE(o_generatorControlRoom_init);
	DECLARE_OPCODE(o_fireplace_init);
	DECLARE_OPCODE(o_clockGears_init);
	DECLARE_OPCODE(o_gulls1_init);
	DECLARE_OPCODE(o_observatory_init);
	DECLARE_OPCODE(o_gulls2_init);
	DECLARE_OPCODE(o_treeCard_init);
	DECLARE_OPCODE(o_treeEntry_init);
	DECLARE_OPCODE(opcode_218);
	DECLARE_OPCODE(o_rocketSliders_init);
	DECLARE_OPCODE(o_rocketLinkVideo_init);
	DECLARE_OPCODE(o_greenBook_init);
	DECLARE_OPCODE(o_gulls3_init);

	DECLARE_OPCODE(o_bookAddSpecialPage_exit);
	DECLARE_OPCODE(o_treeCard_exit);
	DECLARE_OPCODE(o_treeEntry_exit);
	DECLARE_OPCODE(o_generatorControlRoom_exit);


	MystGameState::Myst &_state;

	bool _generatorControlRoomRunning;
	uint16 _generatorVoltage; // 58

	MystResourceType10 *_rocketSlider1; // 248
	MystResourceType10 *_rocketSlider2; // 252
	MystResourceType10 *_rocketSlider3; // 256
	MystResourceType10 *_rocketSlider4; // 260
	MystResourceType10 *_rocketSlider5; // 264
	uint16 _rocketSliderSound; // 294
	uint16 _rocketLeverPosition; // 296
	VideoHandle _rocketLinkBook;

	bool _libraryCombinationBookPagesTurning;
	int16 _libraryBookPage; // 86
	uint16 _libraryBookNumPages; // 88
	uint16 _libraryBookBaseImage; // 90

	bool _butterfliesMoviePlayed; // 100

	bool _gullsFlying1;
	bool _gullsFlying2;
	bool _gullsFlying3;
	uint32 _gullsNextTime; // 216

	bool _libraryBookcaseMoving;
	MystResourceType6 *_libraryBookcaseMovie; // 104
	uint16 _libraryBookcaseSoundId; // 284
	bool _libraryBookcaseChanged; // 288
	uint16 _libraryBookSound1; // 298
	uint16 _libraryBookSound2; // 300

	uint16 _courtyardBoxSound; // 302

	bool _imagerValidationRunning;
	MystResourceType8 *_imagerRedButton; // 304
	uint16 _imagerSound[4]; // 308 to 314
	uint16 _imagerValidationCard; // 316
	uint16 _imagerValidationStep; // 318

	bool _imagerRunning;
	MystResourceType6 *_imagerMovie; // 64

	uint16 _fireplaceLines[6]; // 74 to 84

	uint16 _clockTurningWheel;

	VideoHandle _clockGearsVideos[3]; // 148 to 156
	VideoHandle _clockWeightVideo; // 160
	uint16 _clockGearsPositions[3]; // 164 to 168
	uint16 _clockWeightPosition; // 172
	bool _clockMiddleGearMovedAlone; // 176
	bool _clockLeverPulled; // 328

	uint16 _dockVaultState; // 92

	bool _towerRotationMapRunning;
	bool _towerRotationBlinkLabel;
	uint16 _towerRotationBlinkLabelCount;
	uint16 _towerRotationMapInitialized; // 292
	MystResourceType11 *_towerRotationMapTower; // 108
	MystResourceType8 *_towerRotationMapLabel; // 112
	uint16 _towerRotationSpeed; // 124
	bool _towerRotationMapClicked; // 132
	bool _towerRotationOverSpot; // 136

	bool _matchBurning;
	uint16 _matchGoOutCnt;
	uint16 _cabinDoorOpened; // 56
	uint16 _cabinMatchState; // 60
	uint32 _matchGoOutTime; // 144

	bool _boilerPressureIncreasing;
	bool _boilerPressureDecreasing;
	bool _basementPressureIncreasing;
	bool _basementPressureDecreasing;

	bool _treeStopped; // 236
	MystResourceType8 *_tree; // 220
	MystResourceType5 *_treeAlcove; // 224
	uint16 _treeMinPosition; // 228
	uint16 _treeMinAccessiblePosition; // 230
	uint16 _treeMaxAccessiblePosition; // 232

	bool _observatoryRunning;
	bool _observatoryMonthChanging;
	bool _observatoryDayChanging;
	bool _observatoryYearChanging;
	bool _observatoryTimeChanging;
	MystResourceType8 *_observatoryVisualizer; // 184
	MystResourceType8 *_observatoryGoButton; // 188
	MystResourceType10 *_observatoryDaySlider; // 192
	MystResourceType10 *_observatoryMonthSlider; // 196
	MystResourceType10 *_observatoryYearSlider; // 200
	MystResourceType10 *_observatoryTimeSlider; // 204
	uint32 _observatoryLastTime; // 208
	bool _observatoryNotInitialized; // 212
	int16 _observatoryIncrement; // 346
	MystResourceType10 *_observatoryCurrentSlider; // 348

	bool _greenBookRunning;

	void generatorRedrawRocket();
	void generatorButtonValue(MystResource *button, uint16 &offset, uint16 &value);

	void rocketSliderMove();
	uint16 rocketSliderGetSound(uint16 pos);
	void rocketCheckSolution();

	void libraryCombinationBookTurnRight();
	void libraryCombinationBookTurnLeft();

	uint16 bookCountPages(uint16 var);

	void clockWheelStartTurn(uint16 wheel);
	void clockWheelTurn(uint16 var);

	void clockGearForwardOneStep(uint16 gear);
	void clockWeightDownOneStep();
	void clockGearsCheckSolution();
	void clockReset();
	void clockResetWeight();
	void clockResetGear(uint16 gear);

	void towerRotationMapRotate();
	void towerRotationDrawBuildings();
	uint16 towerRotationMapComputeAngle();
	Common::Point towerRotationMapComputeCoords(const Common::Point &center, uint16 angle);
	void towerRotationMapDrawLine(const Common::Point &center, const Common::Point &end);

	void treeSetAlcoveAccessible();
	uint32 treeNextMoveDelay(uint16 pressure);

	bool observatoryIsDDMMYYYY2400();
	void observatorySetTargetToSetting();
	void observatoryUpdateVisualizer(uint16 x, uint16 y);
	void observatoryIncrementMonth(int16 increment);
	void observatoryIncrementDay(int16 increment);
	void observatoryIncrementYear(int16 increment);
	void observatoryIncrementTime(int16 increment);
	void observatoryUpdateMonth();
	void observatoryUpdateDay();
	void observatoryUpdateYear();
	void observatoryUpdateTime();
};

} // End of namespace MystStacks
} // End of namespace Mohawk

#undef DECLARE_OPCODE

#endif
