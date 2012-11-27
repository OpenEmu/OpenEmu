/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "common/error.h"
#include "common/stream.h"

#include "pegasus/constants.h"
#include "pegasus/gamestate.h"
#include "pegasus/scoring.h"

namespace Common {
DECLARE_SINGLETON(Pegasus::GameStateManager);
}

namespace Pegasus {

Common::Error GameStateManager::writeGameState(Common::WriteStream *stream) {
	stream->writeUint16BE(_currentNeighborhood);
	stream->writeUint16BE(_currentRoom);
	stream->writeByte(_currentDirection);
	stream->writeUint16BE(_nexNeighborhoodID);
	stream->writeUint16BE(_nextRoomID);
	stream->writeByte(_nextDirection);
	stream->writeUint16BE(_lastNeighborhood);
	stream->writeUint16BE(_lastRoom);
	stream->writeByte(_lastDirection);
	stream->writeUint16BE(_openDoorRoom);
	stream->writeByte(_openDoorDirection);

	_globalFlags.writeToStream(stream);
	_scoringFlags.writeToStream(stream);
	_itemTakenFlags.writeToStream(stream);

	writeCaldoriaState(stream);
	writeTSAState(stream);
	writePrehistoricState(stream);
	writeNoradState(stream);
	writeMarsState(stream);
	writeWSCState(stream);

	if (stream->err())
		return Common::kWritingFailed;

	return Common::kNoError;
}

Common::Error GameStateManager::readGameState(Common::ReadStream *stream) {
	_currentNeighborhood = stream->readUint16BE();
	_currentRoom = stream->readUint16BE();
	_currentDirection = stream->readByte();
	_nexNeighborhoodID = stream->readUint16BE();
	_nextRoomID = stream->readUint16BE();
	_nextDirection = stream->readByte();
	_lastNeighborhood = stream->readUint16BE();
	_lastRoom = stream->readUint16BE();
	_lastDirection = stream->readByte();
	_openDoorRoom = stream->readUint16BE();
	_openDoorDirection = stream->readByte();

	_globalFlags.readFromStream(stream);
	_scoringFlags.readFromStream(stream);
	_itemTakenFlags.readFromStream(stream);

	readCaldoriaState(stream);
	readTSAState(stream);
	readPrehistoricState(stream);
	readNoradState(stream);
	readMarsState(stream);
	readWSCState(stream);

	if (stream->err())
		return Common::kReadingFailed;

	return Common::kNoError;
}

void GameStateManager::resetGameState() {
	_currentNeighborhood = kNoNeighborhoodID;
	_currentRoom = kNoRoomID;
	_currentDirection = kNoDirection;
	_nexNeighborhoodID = kNoNeighborhoodID;
	_nextRoomID = kNoRoomID;
	_nextDirection = kNoDirection;
	_lastNeighborhood = kNoNeighborhoodID;
	_lastRoom = kNoRoomID;
	_lastDirection = kNoDirection;
	_openDoorRoom = kNoRoomID;
	_openDoorDirection = kNoDirection;

	_globalFlags.clearAllFlags();
	_scoringFlags.clearAllFlags();
	_itemTakenFlags.clearAllFlags();

	resetCaldoriaState();
	resetTSAState();
	resetPrehistoricState();
	resetNoradState();
	resetMarsState();
	resetWSCState();
}

void GameStateManager::getCurrentLocation(NeighborhoodID &neighborhood, RoomID &room, DirectionConstant &direction) {
	neighborhood = _currentNeighborhood;
	room = _currentRoom;
	direction = _currentDirection;
}

void GameStateManager::setCurrentLocation(const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction) {
	_lastNeighborhood = _currentNeighborhood;
	_lastRoom = _currentRoom;
	_lastDirection = _currentDirection;
	_currentNeighborhood = neighborhood;
	_currentRoom = room;
	_currentDirection = direction;
}

NeighborhoodID GameStateManager::getCurrentNeighborhood() {
	return _currentNeighborhood;
}

void GameStateManager::setCurrentNeighborhood(const NeighborhoodID neighborhood) {
	_lastNeighborhood = _currentNeighborhood;
	_currentNeighborhood = neighborhood;
}

RoomID GameStateManager::getCurrentRoom() {
	return _currentRoom;
}

void GameStateManager::setCurrentRoom(const RoomID room) {
	_lastRoom = _currentRoom;
	_currentRoom = room;
}

DirectionConstant GameStateManager::getCurrentDirection() {
	return _currentDirection;
}

void GameStateManager::setCurrentDirection(const DirectionConstant direction) {
	_lastDirection = _currentDirection;
	_currentDirection = direction;
}

RoomViewID GameStateManager::getCurrentRoomAndView() {
	return MakeRoomView(_currentRoom, _currentDirection);
}

void GameStateManager::getNextLocation(NeighborhoodID &neighborhood, RoomID &room, DirectionConstant &direction) {
	neighborhood = _nexNeighborhoodID;
	room = _nextRoomID;
	direction = _nextDirection;
}

void GameStateManager::setNextLocation(const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction) {
	_nexNeighborhoodID = neighborhood;
	_nextRoomID = room;
	_nextDirection = direction;
}

NeighborhoodID GameStateManager::getNextNeighborhood() {
	return _nexNeighborhoodID;
}

void GameStateManager::setNextNeighborhood(const NeighborhoodID neighborhood) {
	_nexNeighborhoodID = neighborhood;
}

RoomID GameStateManager::getNextRoom() {
	return _nextRoomID;
}

void GameStateManager::setNextRoom(const RoomID room) {
	_nextRoomID = room;
}

DirectionConstant GameStateManager::getNextDirection() {
	return _nextDirection;
}

void GameStateManager::setNextDirection(const DirectionConstant direction) {
	_nextDirection = direction;
}

void GameStateManager::getLastLocation(NeighborhoodID &neighborhood, RoomID &room, DirectionConstant &direction) {
	neighborhood = _currentNeighborhood;
	room = _currentRoom;
	direction = _currentDirection;
}

void GameStateManager::setLastLocation(const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction) {
	_currentNeighborhood = neighborhood;
	_currentRoom = room;
	_currentDirection = direction;
}

NeighborhoodID GameStateManager::getLastNeighborhood() {
	return _lastNeighborhood;
}

void GameStateManager::setLastNeighborhood(const NeighborhoodID neighborhood) {
	_lastNeighborhood = neighborhood;
}

RoomID GameStateManager::getLastRoom() {
	return _lastRoom;
}

void GameStateManager::setLastRoom(const RoomID room) {
	_lastRoom = room;
}

DirectionConstant GameStateManager::getLastDirection() {
	return _lastDirection;
}

void GameStateManager::setLastDirection(const DirectionConstant direction) {
	_lastDirection = direction;
}

RoomViewID GameStateManager::getLastRoomAndView() {
	return MakeRoomView(_lastRoom, _lastDirection);
}

void GameStateManager::getOpenDoorLocation(RoomID &room, DirectionConstant &direction) {
	room = _openDoorRoom;
	direction = _openDoorDirection;
}

void GameStateManager::setOpenDoorLocation(const RoomID room, const DirectionConstant direction) {
	_openDoorRoom = room;
	_openDoorDirection = direction;
}

RoomID GameStateManager::getOpenDoorRoom() {
	return _openDoorRoom;
}

void GameStateManager::setOpenDoorRoom(const RoomID room) {
	_openDoorRoom = room;
}

DirectionConstant GameStateManager::getOpenDoorDirection() {
	return _openDoorDirection;
}

void GameStateManager::setOpenDoorDirection(const DirectionConstant direction) {
	_openDoorDirection = direction;
}

RoomViewID GameStateManager::getDoorOpenRoomAndView() {
	return MakeRoomView(_openDoorRoom, _openDoorDirection);
}

bool GameStateManager::isCurrentDoorOpen() {
	return _openDoorRoom == _currentRoom && _openDoorDirection == _currentDirection;
}

GameScoreType GameStateManager::getCaldoriaTSAScore() {
	GameScoreType result = 0;

	if (_scoringFlags.getFlag(kScoringSawINNFlag))
		result += kSawINNScore;
	if (_scoringFlags.getFlag(kScoringTookShowerFlag))
		result += kTookShowerScore;
	if (_scoringFlags.getFlag(kScoringFixedHairFlag))
		result += kFixedHairScore;
	if (_scoringFlags.getFlag(kScoringGotKeyCardFlag))
		result += kGotKeyCardScore;
	if (_scoringFlags.getFlag(kScoringReadPaperFlag))
		result += kReadPaperScore;
	if (_scoringFlags.getFlag(kScoringLookThroughTelescopeFlag))
		result += kLookThroughTelescopeScore;
	if (_scoringFlags.getFlag(kScoringSawCaldoriaKioskFlag))
		result += kSawCaldoriaKioskScore;
	if (_scoringFlags.getFlag(kScoringGoToTSAFlag))
		result += kGoToTSAScore;
	if (_scoringFlags.getFlag(kScoringEnterTSAFlag))
		result += kEnterTSAScore;
	if (_scoringFlags.getFlag(kScoringSawBust1Flag))
		result += kSawBust1Score;
	if (_scoringFlags.getFlag(kScoringSawBust2Flag))
		result += kSawBust2Score;
	if (_scoringFlags.getFlag(kScoringSawBust3Flag))
		result += kSawBust3Score;
	if (_scoringFlags.getFlag(kScoringSawBust4Flag))
		result += kSawBust4Score;
	if (_scoringFlags.getFlag(kScoringSawBust5Flag))
		result += kSawBust5Score;
	if (_scoringFlags.getFlag(kScoringSawBust6Flag))
		result += kSawBust6Score;
	if (_scoringFlags.getFlag(kScoringSawTheoryFlag))
		result += kSawTheoryScore;
	if (_scoringFlags.getFlag(kScoringSawBackgroundFlag))
		result += kSawBackgroundScore;
	if (_scoringFlags.getFlag(kScoringSawProcedureFlag))
		result += kSawProcedureScore;
	if (_scoringFlags.getFlag(kScoringGotJourneymanKeyFlag))
		result += kGotJourneymanKeyScore;
	if (_scoringFlags.getFlag(kScoringGotPegasusBiochipFlag))
		result += kGotPegasusBiochipScore;
	if (_scoringFlags.getFlag(kScoringGotBiosuitFlag))
		result += kGotBiosuitScore;
	if (_scoringFlags.getFlag(kScoringGoToPrehistoricFlag))
		result += kGoToPrehistoricScore;
	if (_scoringFlags.getFlag(kScoringPutLogInReaderFlag))
		result += kPutLogInReaderScore;
	if (_scoringFlags.getFlag(kScoringSawCaldoriaNormalFlag))
		result += kSawCaldoriaNormalScore;
	if (_scoringFlags.getFlag(kScoringSawCaldoriaAlteredFlag))
		result += kSawCaldoriaAlteredScore;
	if (_scoringFlags.getFlag(kScoringSawNoradNormalFlag))
		result += kSawNoradNormalScore;
	if (_scoringFlags.getFlag(kScoringSawNoradAlteredFlag))
		result += kSawNoradAlteredScore;
	if (_scoringFlags.getFlag(kScoringSawMarsNormalFlag))
		result += kSawMarsNormalScore;
	if (_scoringFlags.getFlag(kScoringSawMarsAlteredFlag))
		result += kSawMarsAlteredScore;
	if (_scoringFlags.getFlag(kScoringSawWSCNormalFlag))
		result += kSawWSCNormalScore;
	if (_scoringFlags.getFlag(kScoringSawWSCAlteredFlag))
		result += kSawWSCAlteredScore;
	if (_scoringFlags.getFlag(kScoringWentToReadyRoom2Flag))
		result += kWentToReadyRoom2Score;
	if (_scoringFlags.getFlag(kScoringWentAfterSinclairFlag))
		result += kWentAfterSinclairScore;
	if (_scoringFlags.getFlag(kScoringUsedCardBombFlag))
		result += kUsedCardBombScore;
	if (_scoringFlags.getFlag(kScoringShieldedCardBombFlag))
		result += kShieldedCardBombScore;
	if (_scoringFlags.getFlag(kScoringStunnedSinclairFlag))
		result += kStunnedSinclairScore;
	if (_scoringFlags.getFlag(kScoringDisarmedNukeFlag))
		result += kDisarmedNukeScore;

	return result;
}

GameScoreType GameStateManager::getPrehistoricScore() {
	GameScoreType result = 0;

	if (_scoringFlags.getFlag(kScoringThrewBreakerFlag))
		result += kThrewBreakerScore;
	if (_scoringFlags.getFlag(kScoringExtendedBridgeFlag))
		result += kExtendedBridgeScore;
	if (_scoringFlags.getFlag(kScoringGotHistoricalLogFlag))
		result += kGotHistoricalLogScore;
	if (_scoringFlags.getFlag(kScoringFinishedPrehistoricFlag))
		result += kFinishedPrehistoricScore;

	return result;
}

GameScoreType GameStateManager::getMarsScore() {
	GameScoreType result = 0;

	if (_scoringFlags.getFlag(kScoringThrownByRobotFlag))
		result += kThrownByRobotScore;
	if (_scoringFlags.getFlag(kScoringGotMarsCardFlag))
		result += kGotMarsCardScore;
	if (_scoringFlags.getFlag(kScoringSawMarsKioskFlag))
		result += kSawMarsKioskScore;
	if (_scoringFlags.getFlag(kScoringSawTransportMapFlag))
		result += kSawTransportMapScore;
	if (_scoringFlags.getFlag(kScoringGotCrowBarFlag))
		result += kGotCrowBarScore;
	if (_scoringFlags.getFlag(kScoringTurnedOnTransportFlag))
		result += kTurnedOnTransportScore;
	if (_scoringFlags.getFlag(kScoringGotOxygenMaskFlag))
		result += kGotOxygenMaskScore;
	if (_scoringFlags.getFlag(kScoringAvoidedRobotFlag))
		result += kAvoidedRobotScore;
	if (_scoringFlags.getFlag(kScoringActivatedPlatformFlag))
		result += kActivatedPlatformScore;
	if (_scoringFlags.getFlag(kScoringUsedLiquidNitrogenFlag))
		result += kUsedLiquidNitrogenScore;
	if (_scoringFlags.getFlag(kScoringUsedCrowBarFlag))
		result += kUsedCrowBarScore;
	if (_scoringFlags.getFlag(kScoringFoundCardBombFlag))
		result += kFoundCardBombScore;
	if (_scoringFlags.getFlag(kScoringDisarmedCardBombFlag))
		result += kDisarmedCardBombScore;
	if (_scoringFlags.getFlag(kScoringGotCardBombFlag))
		result += kGotCardBombScore;
	if (_scoringFlags.getFlag(kScoringThreadedMazeFlag))
		result += kThreadedMazeScore;
	if (_scoringFlags.getFlag(kScoringThreadedGearRoomFlag))
		result += kThreadedGearRoomScore;
	if (_scoringFlags.getFlag(kScoringEnteredShuttleFlag))
		result += kEnteredShuttleScore;
	if (_scoringFlags.getFlag(kScoringEnteredLaunchTubeFlag))
		result += kEnteredLaunchTubeScore;
	if (_scoringFlags.getFlag(kScoringStoppedRobotsShuttleFlag))
		result += kStoppedRobotsShuttleScore;
	if (_scoringFlags.getFlag(kScoringGotMarsOpMemChipFlag))
		result += kGotMarsOpMemChipScore;
	if (_scoringFlags.getFlag(kScoringFinishedMarsFlag))
		result += kFinishedMarsScore;

	return result;
}

GameScoreType GameStateManager::getNoradScore() {
	GameScoreType result = 0;

	if (_scoringFlags.getFlag(kScoringSawSecurityMonitorFlag))
		result += kSawSecurityMonitorScore;
	if (_scoringFlags.getFlag(kScoringFilledOxygenCanisterFlag))
		result += kFilledOxygenCanisterScore;
	if (_scoringFlags.getFlag(kScoringFilledArgonCanisterFlag))
		result += kFilledArgonCanisterScore;
	if (_scoringFlags.getFlag(kScoringSawUnconsciousOperatorFlag))
		result += kSawUnconsciousOperatorScore;
	if (_scoringFlags.getFlag(kScoringWentThroughPressureDoorFlag))
		result += kWentThroughPressureDoorScore;
	if (_scoringFlags.getFlag(kScoringPreppedSubFlag))
		result += kPreppedSubScore;
	if (_scoringFlags.getFlag(kScoringEnteredSubFlag))
		result += kEnteredSubScore;
	if (_scoringFlags.getFlag(kScoringExitedSubFlag))
		result += kExitedSubScore;
	if (_scoringFlags.getFlag(kScoringSawRobotAt54NorthFlag))
		result += kSawRobotAt54NorthScore;
	if (_scoringFlags.getFlag(kScoringPlayedWithClawFlag))
		result += kPlayedWithClawScore;
	if (_scoringFlags.getFlag(kScoringUsedRetinalChipFlag))
		result += kUsedRetinalChipScore;
	if (_scoringFlags.getFlag(kScoringFinishedGlobeGameFlag))
		result += kFinishedGlobeGameScore;
	if (_scoringFlags.getFlag(kScoringStoppedNoradRobotFlag))
		result += kStoppedNoradRobotScore;
	if (_scoringFlags.getFlag(kScoringGotNoradOpMemChipFlag))
		result += kGotNoradOpMemChipScore;
	if (_scoringFlags.getFlag(kScoringFinishedNoradFlag))
		result += kFinishedNoradScore;

	return result;
}

GameScoreType GameStateManager::getWSCScore() {
	GameScoreType result = 0;

	if (_scoringFlags.getFlag(kScoringRemovedDartFlag))
		result += kRemovedDartScore;
	if (_scoringFlags.getFlag(kScoringAnalyzedDartFlag))
		result += kAnalyzedDartScore;
	if (_scoringFlags.getFlag(kScoringBuiltAntidoteFlag))
		result += kBuiltAntidoteScore;
	if (_scoringFlags.getFlag(kScoringGotSinclairKeyFlag))
		result += kGotSinclairKeyScore;
	if (_scoringFlags.getFlag(kScoringGotArgonCanisterFlag))
		result += kGotArgonCanisterScore;
	if (_scoringFlags.getFlag(kScoringGotNitrogenCanisterFlag))
		result += kGotNitrogenCanisterScore;
	if (_scoringFlags.getFlag(kScoringPlayedWithMessagesFlag))
		result += kPlayedWithMessagesScore;
	if (_scoringFlags.getFlag(kScoringSawMorphExperimentFlag))
		result += kSawMorphExperimentScore;
	if (_scoringFlags.getFlag(kScoringEnteredSinclairOfficeFlag))
		result += kEnteredSinclairOfficeScore;
	if (_scoringFlags.getFlag(kScoringSawBrochureFlag))
		result += kSawBrochureScore;
	if (_scoringFlags.getFlag(kScoringSawSinclairEntry1Flag))
		result += kSawSinclairEntry1Score;
	if (_scoringFlags.getFlag(kScoringSawSinclairEntry2Flag))
		result += kSawSinclairEntry2Score;
	if (_scoringFlags.getFlag(kScoringSawSinclairEntry3Flag))
		result += kSawSinclairEntry3Score;
	if (_scoringFlags.getFlag(kScoringSawWSCDirectoryFlag))
		result += kSawWSCDirectoryScore;
	if (_scoringFlags.getFlag(kScoringUsedCrowBarInWSCFlag))
		result += kUsedCrowBarInWSCScore;
	if (_scoringFlags.getFlag(kScoringFinishedPlasmaDodgeFlag))
		result += kFinishedPlasmaDodgeScore;
	if (_scoringFlags.getFlag(kScoringOpenedCatwalkFlag))
		result += kOpenedCatwalkScore;
	if (_scoringFlags.getFlag(kScoringStoppedWSCRobotFlag))
		result += kStoppedWSCRobotScore;
	if (_scoringFlags.getFlag(kScoringGotWSCOpMemChipFlag))
		result += kGotWSCOpMemChipScore;
	if (_scoringFlags.getFlag(kScoringFinishedWSCFlag))
		result += kFinishedWSCScore;

	return result;
}

GameScoreType GameStateManager::getGandhiScore() {
	GameScoreType result = 0;

	if (_scoringFlags.getFlag(kScoringMarsGandhiFlag))
		result += kMarsGandhiScore;
	if (_scoringFlags.getFlag(kScoringNoradGandhiFlag))
		result += kNoradGandhiScore;
	if (_scoringFlags.getFlag(kScoringWSCGandhiFlag))
		result += kWSCGandhiScore;

	return result;
}

GameScoreType GameStateManager::getTotalScore() {
	return	getCaldoriaTSAScore() +
			getPrehistoricScore() +
			getMarsScore() +
			getNoradScore() +
			getWSCScore() +
			getGandhiScore();
}

/////////////////////////////////////////////
//
// Caldoria data

void GameStateManager::writeCaldoriaState(Common::WriteStream *stream) {
	_caldoriaFlags.writeToStream(stream);
	stream->writeUint32BE(_caldoriaFuseTimeLimit);
}

void GameStateManager::readCaldoriaState(Common::ReadStream *stream) {
	_caldoriaFlags.readFromStream(stream);
	_caldoriaFuseTimeLimit = stream->readUint32BE();
}

void GameStateManager::resetCaldoriaState() {
	_caldoriaFlags.clearAllFlags();
	_caldoriaFuseTimeLimit = 0;
}

/////////////////////////////////////////////
//
// TSA data

void GameStateManager::writeTSAState(Common::WriteStream *stream) {
	_TSAFlags.writeToStream(stream);
	stream->writeUint32BE(_TSARipTimerTime);
	stream->writeUint32BE(_TSAFuseTimeLimit);
	stream->writeByte(_TSAState);
	stream->writeByte(_T0BMonitorMode);
	stream->writeUint32BE(_T0BMonitorStart);
}

void GameStateManager::readTSAState(Common::ReadStream *stream) {
	_TSAFlags.readFromStream(stream);
	_TSARipTimerTime = stream->readUint32BE();
	_TSAFuseTimeLimit = stream->readUint32BE();
	_TSAState = stream->readByte();
	_T0BMonitorMode = stream->readByte();
	_T0BMonitorStart = stream->readUint32BE();
}

void GameStateManager::resetTSAState() {
	_TSAFlags.clearAllFlags();
	_TSAState = 0;
	_T0BMonitorMode = 0;
	_T0BMonitorStart = 0;
	_TSARipTimerTime = 0;
	_TSAFuseTimeLimit = kTSAUncreatedTimeLimit;
}

/////////////////////////////////////////////
//
// Prehistoric data

void GameStateManager::writePrehistoricState(Common::WriteStream *stream) {
	_prehistoricFlags.writeToStream(stream);
}

void GameStateManager::readPrehistoricState(Common::ReadStream *stream) {
	_prehistoricFlags.readFromStream(stream);
}

void GameStateManager::resetPrehistoricState() {
	_prehistoricFlags.clearAllFlags();
}

/////////////////////////////////////////////
//
// Norad data

void GameStateManager::writeNoradState(Common::WriteStream *stream) {
	_noradFlags.writeToStream(stream);
	stream->writeUint16BE(_noradSubRoomPressure);
	stream->writeByte(_noradSubPrepState);
}

void GameStateManager::readNoradState(Common::ReadStream *stream) {
	_noradFlags.readFromStream(stream);
	_noradSubRoomPressure = stream->readUint16BE();
	_noradSubPrepState = (NoradSubPrepState)stream->readByte();
}

void GameStateManager::resetNoradState() {
	_noradFlags.clearAllFlags();
	_noradSubRoomPressure = 9;
	_noradSubPrepState = kSubNotPrepped;
}

/////////////////////////////////////////////
//
// Mars data

void GameStateManager::writeMarsState(Common::WriteStream *stream) {
	_marsFlags.writeToStream(stream);
}

void GameStateManager::readMarsState(Common::ReadStream *stream) {
	_marsFlags.readFromStream(stream);
}

void GameStateManager::resetMarsState() {
	_marsFlags.clearAllFlags();
}

/////////////////////////////////////////////
//
// WSC data

void GameStateManager::writeWSCState(Common::WriteStream *stream) {
	_WSCFlags.writeToStream(stream);
}

void GameStateManager::readWSCState(Common::ReadStream *stream) {
	_WSCFlags.readFromStream(stream);
}

void GameStateManager::resetWSCState() {
	_WSCFlags.clearAllFlags();
}

void GameStateManager::setScoringSawINN(const bool flag) {
	_scoringFlags.setFlag(kScoringSawINNFlag, flag);
}

void GameStateManager::setScoringTookShower(const bool flag) {
	_scoringFlags.setFlag(kScoringTookShowerFlag, flag);
}

void GameStateManager::setScoringFixedHair(const bool flag) {
	_scoringFlags.setFlag(kScoringFixedHairFlag, flag);
}

void GameStateManager::setScoringGotKeyCard(const bool flag) {
	_scoringFlags.setFlag(kScoringGotKeyCardFlag, flag);
}

void GameStateManager::setScoringReadPaper(const bool flag) {
	_scoringFlags.setFlag(kScoringReadPaperFlag, flag);
}

void GameStateManager::setScoringLookThroughTelescope(const bool flag) {
	_scoringFlags.setFlag(kScoringLookThroughTelescopeFlag, flag);
}

void GameStateManager::setScoringSawCaldoriaKiosk(const bool flag) {
	_scoringFlags.setFlag(kScoringSawCaldoriaKioskFlag, flag);
}

void GameStateManager::setScoringGoToTSA(const bool flag) {
	_scoringFlags.setFlag(kScoringGoToTSAFlag, flag);
}

void GameStateManager::setScoringEnterTSA(const bool flag) {
	_scoringFlags.setFlag(kScoringEnterTSAFlag, flag);
}

void GameStateManager::setScoringSawBust1(const bool flag) {
	_scoringFlags.setFlag(kScoringSawBust1Flag, flag);
}

void GameStateManager::setScoringSawBust2(const bool flag) {
	_scoringFlags.setFlag(kScoringSawBust2Flag, flag);
}

void GameStateManager::setScoringSawBust3(const bool flag) {
	_scoringFlags.setFlag(kScoringSawBust3Flag, flag);
}

void GameStateManager::setScoringSawBust4(const bool flag) {
	_scoringFlags.setFlag(kScoringSawBust4Flag, flag);
}

void GameStateManager::setScoringSawBust5(const bool flag) {
	_scoringFlags.setFlag(kScoringSawBust5Flag, flag);
}

void GameStateManager::setScoringSawBust6(const bool flag) {
	_scoringFlags.setFlag(kScoringSawBust6Flag, flag);
}

void GameStateManager::setScoringSawTheory(const bool flag) {
	_scoringFlags.setFlag(kScoringSawTheoryFlag, flag);
}

void GameStateManager::setScoringSawBackground(const bool flag) {
	_scoringFlags.setFlag(kScoringSawBackgroundFlag, flag);
}

void GameStateManager::setScoringSawProcedure(const bool flag) {
	_scoringFlags.setFlag(kScoringSawProcedureFlag, flag);
}

void GameStateManager::setScoringGotJourneymanKey(const bool flag) {
	_scoringFlags.setFlag(kScoringGotJourneymanKeyFlag, flag);
}

void GameStateManager::setScoringGotPegasusBiochip(const bool flag) {
	_scoringFlags.setFlag(kScoringGotPegasusBiochipFlag, flag);
}

void GameStateManager::setScoringGotBiosuit(const bool flag) {
	_scoringFlags.setFlag(kScoringGotBiosuitFlag, flag);
}

void GameStateManager::setScoringGoToPrehistoric(const bool flag) {
	_scoringFlags.setFlag(kScoringGoToPrehistoricFlag, flag);
}

void GameStateManager::setScoringPutLogInReader(const bool flag) {
	_scoringFlags.setFlag(kScoringPutLogInReaderFlag, flag);
}

void GameStateManager::setScoringSawCaldoriaNormal(const bool flag) {
	_scoringFlags.setFlag(kScoringSawCaldoriaNormalFlag, flag);
}

void GameStateManager::setScoringSawCaldoriaAltered(const bool flag) {
	_scoringFlags.setFlag(kScoringSawCaldoriaAlteredFlag, flag);
}

void GameStateManager::setScoringSawNoradNormal(const bool flag) {
	_scoringFlags.setFlag(kScoringSawNoradNormalFlag, flag);
}

void GameStateManager::setScoringSawNoradAltered(const bool flag) {
	_scoringFlags.setFlag(kScoringSawNoradAlteredFlag, flag);
}

void GameStateManager::setScoringSawMarsNormal(const bool flag) {
	_scoringFlags.setFlag(kScoringSawMarsNormalFlag, flag);
}

void GameStateManager::setScoringSawMarsAltered(const bool flag) {
	_scoringFlags.setFlag(kScoringSawMarsAlteredFlag, flag);
}

void GameStateManager::setScoringSawWSCNormal(const bool flag) {
	_scoringFlags.setFlag(kScoringSawWSCNormalFlag, flag);
}

void GameStateManager::setScoringSawWSCAltered(const bool flag) {
	_scoringFlags.setFlag(kScoringSawWSCAlteredFlag, flag);
}

void GameStateManager::setScoringWentToReadyRoom2(const bool flag) {
	_scoringFlags.setFlag(kScoringWentToReadyRoom2Flag, flag);
}

void GameStateManager::setScoringWentAfterSinclair(const bool flag) {
	_scoringFlags.setFlag(kScoringWentAfterSinclairFlag, flag);
}

void GameStateManager::setScoringUsedCardBomb(const bool flag) {
	_scoringFlags.setFlag(kScoringUsedCardBombFlag, flag);
}

void GameStateManager::setScoringShieldedCardBomb(const bool flag) {
	_scoringFlags.setFlag(kScoringShieldedCardBombFlag, flag);
}

void GameStateManager::setScoringStunnedSinclair(const bool flag) {
	_scoringFlags.setFlag(kScoringStunnedSinclairFlag, flag);
}

void GameStateManager::setScoringDisarmedNuke(const bool flag) {
	_scoringFlags.setFlag(kScoringDisarmedNukeFlag, flag);
}

void GameStateManager::setScoringThrewBreaker(const bool flag) {
	_scoringFlags.setFlag(kScoringThrewBreakerFlag, flag);
}

void GameStateManager::setScoringExtendedBridge(const bool flag) {
	_scoringFlags.setFlag(kScoringExtendedBridgeFlag, flag);
}

void GameStateManager::setScoringGotHistoricalLog(const bool flag) {
	_scoringFlags.setFlag(kScoringGotHistoricalLogFlag, flag);
}

void GameStateManager::setScoringFinishedPrehistoric(const bool flag) {
	_scoringFlags.setFlag(kScoringFinishedPrehistoricFlag, flag);
}

void GameStateManager::setScoringThrownByRobot(const bool flag) {
	_scoringFlags.setFlag(kScoringThrownByRobotFlag, flag);
}

void GameStateManager::setScoringGotMarsCard(const bool flag) {
	_scoringFlags.setFlag(kScoringGotMarsCardFlag, flag);
}

void GameStateManager::setScoringSawMarsKiosk(const bool flag) {
	_scoringFlags.setFlag(kScoringSawMarsKioskFlag, flag);
}

void GameStateManager::setScoringSawTransportMap(const bool flag) {
	_scoringFlags.setFlag(kScoringSawTransportMapFlag, flag);
}

void GameStateManager::setScoringGotCrowBar(const bool flag) {
	_scoringFlags.setFlag(kScoringGotCrowBarFlag, flag);
}

void GameStateManager::setScoringTurnedOnTransport(const bool flag) {
	_scoringFlags.setFlag(kScoringTurnedOnTransportFlag, flag);
}

void GameStateManager::setScoringGotOxygenMask(const bool flag) {
	_scoringFlags.setFlag(kScoringGotOxygenMaskFlag, flag);
}

void GameStateManager::setScoringAvoidedRobot(const bool flag) {
	_scoringFlags.setFlag(kScoringAvoidedRobotFlag, flag);
}

void GameStateManager::setScoringActivatedPlatform(const bool flag) {
	_scoringFlags.setFlag(kScoringActivatedPlatformFlag, flag);
}

void GameStateManager::setScoringUsedLiquidNitrogen(const bool flag) {
	_scoringFlags.setFlag(kScoringUsedLiquidNitrogenFlag, flag);
}

void GameStateManager::setScoringUsedCrowBar(const bool flag) {
	_scoringFlags.setFlag(kScoringUsedCrowBarFlag, flag);
}

void GameStateManager::setScoringFoundCardBomb(const bool flag) {
	_scoringFlags.setFlag(kScoringFoundCardBombFlag, flag);
}

void GameStateManager::setScoringDisarmedCardBomb(const bool flag) {
	_scoringFlags.setFlag(kScoringDisarmedCardBombFlag, flag);
}

void GameStateManager::setScoringGotCardBomb(const bool flag) {
	_scoringFlags.setFlag(kScoringGotCardBombFlag, flag);
}

void GameStateManager::setScoringThreadedMaze(const bool flag) {
	_scoringFlags.setFlag(kScoringThreadedMazeFlag, flag);
}

void GameStateManager::setScoringThreadedGearRoom(const bool flag) {
	_scoringFlags.setFlag(kScoringThreadedGearRoomFlag, flag);
}

void GameStateManager::setScoringEnteredShuttle(const bool flag) {
	_scoringFlags.setFlag(kScoringEnteredShuttleFlag, flag);
}

void GameStateManager::setScoringEnteredLaunchTube(const bool flag) {
	_scoringFlags.setFlag(kScoringEnteredLaunchTubeFlag, flag);
}

void GameStateManager::setScoringStoppedRobotsShuttle(const bool flag) {
	_scoringFlags.setFlag(kScoringStoppedRobotsShuttleFlag, flag);
}

void GameStateManager::setScoringGotMarsOpMemChip(const bool flag) {
	_scoringFlags.setFlag(kScoringGotMarsOpMemChipFlag, flag);
}

void GameStateManager::setScoringFinishedMars(const bool flag) {
	_scoringFlags.setFlag(kScoringFinishedMarsFlag, flag);
}

void GameStateManager::setScoringSawSecurityMonitor(const bool flag) {
	_scoringFlags.setFlag(kScoringSawSecurityMonitorFlag, flag);
}

void GameStateManager::setScoringFilledOxygenCanister(const bool flag) {
	_scoringFlags.setFlag(kScoringFilledOxygenCanisterFlag, flag);
}

void GameStateManager::setScoringFilledArgonCanister(const bool flag) {
	_scoringFlags.setFlag(kScoringFilledArgonCanisterFlag, flag);
}

void GameStateManager::setScoringSawUnconsciousOperator(const bool flag) {
	_scoringFlags.setFlag(kScoringSawUnconsciousOperatorFlag, flag);
}

void GameStateManager::setScoringWentThroughPressureDoor(const bool flag) {
	_scoringFlags.setFlag(kScoringWentThroughPressureDoorFlag, flag);
}

void GameStateManager::setScoringPreppedSub(const bool flag) {
	_scoringFlags.setFlag(kScoringPreppedSubFlag, flag);
}

void GameStateManager::setScoringEnteredSub(const bool flag) {
	_scoringFlags.setFlag(kScoringEnteredSubFlag, flag);
}

void GameStateManager::setScoringExitedSub(const bool flag) {
	_scoringFlags.setFlag(kScoringExitedSubFlag, flag);
}

void GameStateManager::setScoringSawRobotAt54North(const bool flag) {
	_scoringFlags.setFlag(kScoringSawRobotAt54NorthFlag, flag);
}

void GameStateManager::setScoringPlayedWithClaw(const bool flag) {
	_scoringFlags.setFlag(kScoringPlayedWithClawFlag, flag);
}

void GameStateManager::setScoringUsedRetinalChip(const bool flag) {
	_scoringFlags.setFlag(kScoringUsedRetinalChipFlag, flag);
}

void GameStateManager::setScoringFinishedGlobeGame(const bool flag) {
	_scoringFlags.setFlag(kScoringFinishedGlobeGameFlag, flag);
}

void GameStateManager::setScoringStoppedNoradRobot(const bool flag) {
	_scoringFlags.setFlag(kScoringStoppedNoradRobotFlag, flag);
}

void GameStateManager::setScoringGotNoradOpMemChip(const bool flag) {
	_scoringFlags.setFlag(kScoringGotNoradOpMemChipFlag, flag);
}

void GameStateManager::setScoringFinishedNorad(const bool flag) {
	_scoringFlags.setFlag(kScoringFinishedNoradFlag, flag);
}

void GameStateManager::setScoringRemovedDart(const bool flag) {
	_scoringFlags.setFlag(kScoringRemovedDartFlag, flag);
}

void GameStateManager::setScoringAnalyzedDart(const bool flag) {
	_scoringFlags.setFlag(kScoringAnalyzedDartFlag, flag);
}

void GameStateManager::setScoringBuiltAntidote(const bool flag) {
	_scoringFlags.setFlag(kScoringBuiltAntidoteFlag, flag);
}

void GameStateManager::setScoringGotSinclairKey(const bool flag) {
	_scoringFlags.setFlag(kScoringGotSinclairKeyFlag, flag);
}

void GameStateManager::setScoringGotArgonCanister(const bool flag) {
	_scoringFlags.setFlag(kScoringGotArgonCanisterFlag, flag);
}

void GameStateManager::setScoringGotNitrogenCanister(const bool flag) {
	_scoringFlags.setFlag(kScoringGotNitrogenCanisterFlag, flag);
}

void GameStateManager::setScoringPlayedWithMessages(const bool flag) {
	_scoringFlags.setFlag(kScoringPlayedWithMessagesFlag, flag);
}

void GameStateManager::setScoringSawMorphExperiment(const bool flag) {
	_scoringFlags.setFlag(kScoringSawMorphExperimentFlag, flag);
}

void GameStateManager::setScoringEnteredSinclairOffice(const bool flag) {
	_scoringFlags.setFlag(kScoringEnteredSinclairOfficeFlag, flag);
}

void GameStateManager::setScoringSawBrochure(const bool flag) {
	_scoringFlags.setFlag(kScoringSawBrochureFlag, flag);
}

void GameStateManager::setScoringSawSinclairEntry1(const bool flag) {
	_scoringFlags.setFlag(kScoringSawSinclairEntry1Flag, flag);
}

void GameStateManager::setScoringSawSinclairEntry2(const bool flag) {
	_scoringFlags.setFlag(kScoringSawSinclairEntry2Flag, flag);
}

void GameStateManager::setScoringSawSinclairEntry3(const bool flag) {
	_scoringFlags.setFlag(kScoringSawSinclairEntry3Flag, flag);
}

void GameStateManager::setScoringSawWSCDirectory(const bool flag) {
	_scoringFlags.setFlag(kScoringSawWSCDirectoryFlag, flag);
}

void GameStateManager::setScoringUsedCrowBarInWSC(const bool flag) {
	_scoringFlags.setFlag(kScoringUsedCrowBarInWSCFlag, flag);
}

void GameStateManager::setScoringFinishedPlasmaDodge(const bool flag) {
	_scoringFlags.setFlag(kScoringFinishedPlasmaDodgeFlag, flag);
}

void GameStateManager::setScoringOpenedCatwalk(const bool flag) {
	_scoringFlags.setFlag(kScoringOpenedCatwalkFlag, flag);
}

void GameStateManager::setScoringStoppedWSCRobot(const bool flag) {
	_scoringFlags.setFlag(kScoringStoppedWSCRobotFlag, flag);
}

void GameStateManager::setScoringGotWSCOpMemChip(const bool flag) {
	_scoringFlags.setFlag(kScoringGotWSCOpMemChipFlag, flag);
}

void GameStateManager::setScoringFinishedWSC(const bool flag) {
	_scoringFlags.setFlag(kScoringFinishedWSCFlag, flag);
}

void GameStateManager::setScoringMarsGandhi(const bool flag) {
	_scoringFlags.setFlag(kScoringMarsGandhiFlag, flag);
}

void GameStateManager::setScoringNoradGandhi(const bool flag) {
	_scoringFlags.setFlag(kScoringNoradGandhiFlag, flag);
}

void GameStateManager::setScoringWSCGandhi(const bool flag) {
	_scoringFlags.setFlag(kScoringWSCGandhiFlag, flag);
}

bool GameStateManager::getScoringSawINN() {
	return _scoringFlags.getFlag(kScoringSawINNFlag);
}

bool GameStateManager::getScoringTookShower() {
	return _scoringFlags.getFlag(kScoringTookShowerFlag);
}

bool GameStateManager::getScoringFixedHair() {
	return _scoringFlags.getFlag(kScoringFixedHairFlag);
}

bool GameStateManager::getScoringGotKeyCard() {
	return _scoringFlags.getFlag(kScoringGotKeyCardFlag);
}

bool GameStateManager::getScoringReadPaper() {
	return _scoringFlags.getFlag(kScoringReadPaperFlag);
}

bool GameStateManager::getScoringLookThroughTelescope() {
	return _scoringFlags.getFlag(kScoringLookThroughTelescopeFlag);
}

bool GameStateManager::getScoringSawCaldoriaKiosk() {
	return _scoringFlags.getFlag(kScoringSawCaldoriaKioskFlag);
}

bool GameStateManager::getScoringGoToTSA() {
	return _scoringFlags.getFlag(kScoringGoToTSAFlag);
}

bool GameStateManager::getScoringEnterTSA() {
	return _scoringFlags.getFlag(kScoringEnterTSAFlag);
}

bool GameStateManager::getScoringSawBust1() {
	return _scoringFlags.getFlag(kScoringSawBust1Flag);
}

bool GameStateManager::getScoringSawBust2() {
	return _scoringFlags.getFlag(kScoringSawBust2Flag);
}

bool GameStateManager::getScoringSawBust3() {
	return _scoringFlags.getFlag(kScoringSawBust3Flag);
}

bool GameStateManager::getScoringSawBust4() {
	return _scoringFlags.getFlag(kScoringSawBust4Flag);
}

bool GameStateManager::getScoringSawBust5() {
	return _scoringFlags.getFlag(kScoringSawBust5Flag);
}

bool GameStateManager::getScoringSawBust6() {
	return _scoringFlags.getFlag(kScoringSawBust6Flag);
}

bool GameStateManager::getScoringSawTheory() {
	return _scoringFlags.getFlag(kScoringSawTheoryFlag);
}

bool GameStateManager::getScoringSawBackground() {
	return _scoringFlags.getFlag(kScoringSawBackgroundFlag);
}

bool GameStateManager::getScoringSawProcedure() {
	return _scoringFlags.getFlag(kScoringSawProcedureFlag);
}

bool GameStateManager::getScoringGotJourneymanKey() {
	return _scoringFlags.getFlag(kScoringGotJourneymanKeyFlag);
}

bool GameStateManager::getScoringGotPegasusBiochip() {
	return _scoringFlags.getFlag(kScoringGotPegasusBiochipFlag);
}

bool GameStateManager::getScoringGotBiosuit() {
	return _scoringFlags.getFlag(kScoringGotBiosuitFlag);
}

bool GameStateManager::getScoringGoToPrehistoric() {
	return _scoringFlags.getFlag(kScoringGoToPrehistoricFlag);
}

bool GameStateManager::getScoringPutLogInReader() {
	return _scoringFlags.getFlag(kScoringPutLogInReaderFlag);
}

bool GameStateManager::getScoringSawCaldoriaNormal() {
	return _scoringFlags.getFlag(kScoringSawCaldoriaNormalFlag);
}

bool GameStateManager::getScoringSawCaldoriaAltered() {
	return _scoringFlags.getFlag(kScoringSawCaldoriaAlteredFlag);
}

bool GameStateManager::getScoringSawNoradNormal() {
	return _scoringFlags.getFlag(kScoringSawNoradNormalFlag);
}

bool GameStateManager::getScoringSawNoradAltered() {
	return _scoringFlags.getFlag(kScoringSawNoradAlteredFlag);
}

bool GameStateManager::getScoringSawMarsNormal() {
	return _scoringFlags.getFlag(kScoringSawMarsNormalFlag);
}

bool GameStateManager::getScoringSawMarsAltered() {
	return _scoringFlags.getFlag(kScoringSawMarsAlteredFlag);
}

bool GameStateManager::getScoringSawWSCNormal() {
	return _scoringFlags.getFlag(kScoringSawWSCNormalFlag);
}

bool GameStateManager::getScoringSawWSCAltered() {
	return _scoringFlags.getFlag(kScoringSawWSCAlteredFlag);
}

bool GameStateManager::getScoringWentToReadyRoom2() {
	return _scoringFlags.getFlag(kScoringWentToReadyRoom2Flag);
}

bool GameStateManager::getScoringWentAfterSinclair() {
	return _scoringFlags.getFlag(kScoringWentAfterSinclairFlag);
}

bool GameStateManager::getScoringUsedCardBomb() {
	return _scoringFlags.getFlag(kScoringUsedCardBombFlag);
}

bool GameStateManager::getScoringShieldedCardBomb() {
	return _scoringFlags.getFlag(kScoringShieldedCardBombFlag);
}

bool GameStateManager::getScoringStunnedSinclair() {
	return _scoringFlags.getFlag(kScoringStunnedSinclairFlag);
}

bool GameStateManager::getScoringDisarmedNuke() {
	return _scoringFlags.getFlag(kScoringDisarmedNukeFlag);
}

bool GameStateManager::getScoringThrewBreaker() {
	return _scoringFlags.getFlag(kScoringThrewBreakerFlag);
}

bool GameStateManager::getScoringExtendedBridge() {
	return _scoringFlags.getFlag(kScoringExtendedBridgeFlag);
}

bool GameStateManager::getScoringGotHistoricalLog() {
	return _scoringFlags.getFlag(kScoringGotHistoricalLogFlag);
}

bool GameStateManager::getScoringFinishedPrehistoric() {
	return _scoringFlags.getFlag(kScoringFinishedPrehistoricFlag);
}

bool GameStateManager::getScoringThrownByRobot() {
	return _scoringFlags.getFlag(kScoringThrownByRobotFlag);
}

bool GameStateManager::getScoringGotMarsCard() {
	return _scoringFlags.getFlag(kScoringGotMarsCardFlag);
}

bool GameStateManager::getScoringSawMarsKiosk() {
	return _scoringFlags.getFlag(kScoringSawMarsKioskFlag);
}

bool GameStateManager::getScoringSawTransportMap() {
	return _scoringFlags.getFlag(kScoringSawTransportMapFlag);
}

bool GameStateManager::getScoringGotCrowBar() {
	return _scoringFlags.getFlag(kScoringGotCrowBarFlag);
}

bool GameStateManager::getScoringTurnedOnTransport() {
	return _scoringFlags.getFlag(kScoringTurnedOnTransportFlag);
}

bool GameStateManager::getScoringGotOxygenMask() {
	return _scoringFlags.getFlag(kScoringGotOxygenMaskFlag);
}

bool GameStateManager::getScoringAvoidedRobot() {
	return _scoringFlags.getFlag(kScoringAvoidedRobotFlag);
}

bool GameStateManager::getScoringActivatedPlatform() {
	return _scoringFlags.getFlag(kScoringActivatedPlatformFlag);
}

bool GameStateManager::getScoringUsedLiquidNitrogen() {
	return _scoringFlags.getFlag(kScoringUsedLiquidNitrogenFlag);
}

bool GameStateManager::getScoringUsedCrowBar() {
	return _scoringFlags.getFlag(kScoringUsedCrowBarFlag);
}

bool GameStateManager::getScoringFoundCardBomb() {
	return _scoringFlags.getFlag(kScoringFoundCardBombFlag);
}

bool GameStateManager::getScoringDisarmedCardBomb() {
	return _scoringFlags.getFlag(kScoringDisarmedCardBombFlag);
}

bool GameStateManager::getScoringGotCardBomb() {
	return _scoringFlags.getFlag(kScoringGotCardBombFlag);
}

bool GameStateManager::getScoringThreadedMaze() {
	return _scoringFlags.getFlag(kScoringThreadedMazeFlag);
}

bool GameStateManager::getScoringThreadedGearRoom() {
	return _scoringFlags.getFlag(kScoringThreadedGearRoomFlag);
}

bool GameStateManager::getScoringEnteredShuttle() {
	return _scoringFlags.getFlag(kScoringEnteredShuttleFlag);
}

bool GameStateManager::getScoringEnteredLaunchTube() {
	return _scoringFlags.getFlag(kScoringEnteredLaunchTubeFlag);
}

bool GameStateManager::getScoringStoppedRobotsShuttle() {
	return _scoringFlags.getFlag(kScoringStoppedRobotsShuttleFlag);
}

bool GameStateManager::getScoringGotMarsOpMemChip() {
	return _scoringFlags.getFlag(kScoringGotMarsOpMemChipFlag);
}

bool GameStateManager::getScoringFinishedMars() {
	return _scoringFlags.getFlag(kScoringFinishedMarsFlag);
}

bool GameStateManager::getScoringSawSecurityMonitor() {
	return _scoringFlags.getFlag(kScoringSawSecurityMonitorFlag);
}

bool GameStateManager::getScoringFilledOxygenCanister() {
	return _scoringFlags.getFlag(kScoringFilledOxygenCanisterFlag);
}

bool GameStateManager::getScoringFilledArgonCanister() {
	return _scoringFlags.getFlag(kScoringFilledArgonCanisterFlag);
}

bool GameStateManager::getScoringSawUnconsciousOperator() {
	return _scoringFlags.getFlag(kScoringSawUnconsciousOperatorFlag);
}

bool GameStateManager::getScoringWentThroughPressureDoor() {
	return _scoringFlags.getFlag(kScoringWentThroughPressureDoorFlag);
}

bool GameStateManager::getScoringPreppedSub() {
	return _scoringFlags.getFlag(kScoringPreppedSubFlag);
}

bool GameStateManager::getScoringEnteredSub() {
	return _scoringFlags.getFlag(kScoringEnteredSubFlag);
}

bool GameStateManager::getScoringExitedSub() {
	return _scoringFlags.getFlag(kScoringExitedSubFlag);
}

bool GameStateManager::getScoringSawRobotAt54North() {
	return _scoringFlags.getFlag(kScoringSawRobotAt54NorthFlag);
}

bool GameStateManager::getScoringPlayedWithClaw() {
	return _scoringFlags.getFlag(kScoringPlayedWithClawFlag);
}

bool GameStateManager::getScoringUsedRetinalChip() {
	return _scoringFlags.getFlag(kScoringUsedRetinalChipFlag);
}

bool GameStateManager::getScoringFinishedGlobeGame() {
	return _scoringFlags.getFlag(kScoringFinishedGlobeGameFlag);
}

bool GameStateManager::getScoringStoppedNoradRobot() {
	return _scoringFlags.getFlag(kScoringStoppedNoradRobotFlag);
}

bool GameStateManager::getScoringGotNoradOpMemChip() {
	return _scoringFlags.getFlag(kScoringGotNoradOpMemChipFlag);
}

bool GameStateManager::getScoringFinishedNorad() {
	return _scoringFlags.getFlag(kScoringFinishedNoradFlag);
}

bool GameStateManager::getScoringRemovedDart() {
	return _scoringFlags.getFlag(kScoringRemovedDartFlag);
}

bool GameStateManager::getScoringAnalyzedDart() {
	return _scoringFlags.getFlag(kScoringAnalyzedDartFlag);
}

bool GameStateManager::getScoringBuiltAntidote() {
	return _scoringFlags.getFlag(kScoringBuiltAntidoteFlag);
}

bool GameStateManager::getScoringGotSinclairKey() {
	return _scoringFlags.getFlag(kScoringGotSinclairKeyFlag);
}

bool GameStateManager::getScoringGotArgonCanister() {
	return _scoringFlags.getFlag(kScoringGotArgonCanisterFlag);
}

bool GameStateManager::getScoringGotNitrogenCanister() {
	return _scoringFlags.getFlag(kScoringGotNitrogenCanisterFlag);
}

bool GameStateManager::getScoringPlayedWithMessages() {
	return _scoringFlags.getFlag(kScoringPlayedWithMessagesFlag);
}

bool GameStateManager::getScoringSawMorphExperiment() {
	return _scoringFlags.getFlag(kScoringSawMorphExperimentFlag);
}

bool GameStateManager::getScoringEnteredSinclairOffice() {
	return _scoringFlags.getFlag(kScoringEnteredSinclairOfficeFlag);
}

bool GameStateManager::getScoringSawBrochure() {
	return _scoringFlags.getFlag(kScoringSawBrochureFlag);
}

bool GameStateManager::getScoringSawSinclairEntry1() {
	return _scoringFlags.getFlag(kScoringSawSinclairEntry1Flag);
}

bool GameStateManager::getScoringSawSinclairEntry2() {
	return _scoringFlags.getFlag(kScoringSawSinclairEntry2Flag);
}

bool GameStateManager::getScoringSawSinclairEntry3() {
	return _scoringFlags.getFlag(kScoringSawSinclairEntry3Flag);
}

bool GameStateManager::getScoringSawWSCDirectory() {
	return _scoringFlags.getFlag(kScoringSawWSCDirectoryFlag);
}

bool GameStateManager::getScoringUsedCrowBarInWSC() {
	return _scoringFlags.getFlag(kScoringUsedCrowBarInWSCFlag);
}

bool GameStateManager::getScoringFinishedPlasmaDodge() {
	return _scoringFlags.getFlag(kScoringFinishedPlasmaDodgeFlag);
}

bool GameStateManager::getScoringOpenedCatwalk() {
	return _scoringFlags.getFlag(kScoringOpenedCatwalkFlag);
}

bool GameStateManager::getScoringStoppedWSCRobot() {
	return _scoringFlags.getFlag(kScoringStoppedWSCRobotFlag);
}

bool GameStateManager::getScoringGotWSCOpMemChip() {
	return _scoringFlags.getFlag(kScoringGotWSCOpMemChipFlag);
}

bool GameStateManager::getScoringFinishedWSC() {
	return _scoringFlags.getFlag(kScoringFinishedWSCFlag);
}

bool GameStateManager::getScoringMarsGandhi() {
	return _scoringFlags.getFlag(kScoringMarsGandhiFlag);
}

bool GameStateManager::getScoringNoradGandhi() {
	return _scoringFlags.getFlag(kScoringNoradGandhiFlag);
}

bool GameStateManager::getScoringWSCGandhi() {
	return _scoringFlags.getFlag(kScoringWSCGandhiFlag);
}

void GameStateManager::setWalkthroughMode(bool value) {
	_globalFlags.setFlag(kGlobalWalkthroughFlag, value);
}

bool GameStateManager::getWalkthroughMode() {
	return _globalFlags.getFlag(kGlobalWalkthroughFlag);
}

void GameStateManager::setShieldOn(bool value) {
	_globalFlags.setFlag(kGlobalShieldOnFlag, value);
}

bool GameStateManager::getShieldOn() {
	return _globalFlags.getFlag(kGlobalShieldOnFlag);
}

void GameStateManager::setEasterEgg(bool value) {
	_globalFlags.setFlag(kGlobalEasterEggFlag, value);
}

bool GameStateManager::getEasterEgg() {
	return _globalFlags.getFlag(kGlobalEasterEggFlag);
}

void GameStateManager::setBeenToWSC(bool value) {
	_globalFlags.setFlag(kGlobalBeenToWSCFlag, value);
}

bool GameStateManager::getBeenToWSC() {
	return _globalFlags.getFlag(kGlobalBeenToWSCFlag);
}

void GameStateManager::setBeenToMars(bool value) {
	_globalFlags.setFlag(kGlobalBeenToMarsFlag, value);
}

bool GameStateManager::getBeenToMars() {
	return _globalFlags.getFlag(kGlobalBeenToMarsFlag);
}

void GameStateManager::setBeenToNorad(bool value) {
	_globalFlags.setFlag(kGlobalBeenToNoradFlag, value);
}

bool GameStateManager::getBeenToNorad() {
	return _globalFlags.getFlag(kGlobalBeenToNoradFlag);
}

void GameStateManager::setWSCFinished(bool value) {
	_globalFlags.setFlag(kGlobalWSCFinishedFlag, value);
}

bool GameStateManager::getWSCFinished() {
	return _globalFlags.getFlag(kGlobalWSCFinishedFlag);
}

void GameStateManager::setMarsFinished(bool value) {
	_globalFlags.setFlag(kGlobalMarsFinishedFlag, value);
}

bool GameStateManager::getMarsFinished() {
	return _globalFlags.getFlag(kGlobalMarsFinishedFlag);
}

void GameStateManager::setNoradFinished(bool value) {
	_globalFlags.setFlag(kGlobalNoradFinishedFlag, value);
}

bool GameStateManager::getNoradFinished() {
	return _globalFlags.getFlag(kGlobalNoradFinishedFlag);
}

bool GameStateManager::allTimeZonesFinished() {
	return getWSCFinished() && getMarsFinished() && getNoradFinished();
}

void GameStateManager::setTakenItemID(ItemID id, bool value) {
	_itemTakenFlags.setFlag(id, value);
}

bool GameStateManager::isTakenItemID(ItemID id) {
	return _itemTakenFlags.getFlag(id);
}

void GameStateManager::setTakenItem(Item *item, bool value) {
	setTakenItemID(item->getObjectID(), value);
}

bool GameStateManager::isTakenItem(Item *item) {
	return isTakenItemID(item->getObjectID());
}

void GameStateManager::setCaldoriaFuseTimeLimit(const TimeValue timeLimit) {
	_caldoriaFuseTimeLimit = timeLimit;
}

TimeValue GameStateManager::getCaldoriaFuseTimeLimit() {
	return _caldoriaFuseTimeLimit;
}

void GameStateManager::setCaldoriaSeenPullback(bool value) {
	_caldoriaFlags.setFlag(kCaldoriaSeenPullbackFlag, value);
}

bool GameStateManager::getCaldoriaSeenPullback() {
	return _caldoriaFlags.getFlag(kCaldoriaSeenPullbackFlag);
}

void GameStateManager::setCaldoriaMadeOJ(bool value) {
	_caldoriaFlags.setFlag(kCaldoriaMadeOJFlag, value);
}

bool GameStateManager::getCaldoriaMadeOJ() {
	return _caldoriaFlags.getFlag(kCaldoriaMadeOJFlag);
}

void GameStateManager::setCaldoriaWokenUp(bool value) {
	_caldoriaFlags.setFlag(kCaldoriaWokenUpFlag, value);
}

bool GameStateManager::getCaldoriaWokenUp() {
	return _caldoriaFlags.getFlag(kCaldoriaWokenUpFlag);
}

void GameStateManager::setCaldoriaDidRecalibration(bool value) {
	_caldoriaFlags.setFlag(kCaldoriaDidRecalibrationFlag, value);
}

bool GameStateManager::getCaldoriaDidRecalibration() {
	return _caldoriaFlags.getFlag(kCaldoriaDidRecalibrationFlag);
}

void GameStateManager::setCaldoriaSeenSinclairInElevator(bool value) {
	_caldoriaFlags.setFlag(kCaldoriaSeenSinclairInElevatorFlag, value);
}

bool GameStateManager::getCaldoriaSeenSinclairInElevator() {
	return _caldoriaFlags.getFlag(kCaldoriaSeenSinclairInElevatorFlag);
}

void GameStateManager::setCaldoriaINNAnnouncing(bool value) {
	_caldoriaFlags.setFlag(kCaldoriaINNAnnouncingFlag, value);
}

bool GameStateManager::getCaldoriaINNAnnouncing() {
	return _caldoriaFlags.getFlag(kCaldoriaINNAnnouncingFlag);
}

void GameStateManager::setCaldoriaSeenINN(bool value) {
	_caldoriaFlags.setFlag(kCaldoriaSeenINNFlag, value);
}

bool GameStateManager::getCaldoriaSeenINN() {
	return _caldoriaFlags.getFlag(kCaldoriaSeenINNFlag);
}

void GameStateManager::setCaldoriaSeenMessages(bool value) {
	_caldoriaFlags.setFlag(kCaldoriaSeenMessagesFlag, value);
}

bool GameStateManager::getCaldoriaSeenMessages() {
	return _caldoriaFlags.getFlag(kCaldoriaSeenMessagesFlag);
}

void GameStateManager::setCaldoriaSinclairShot(bool value) {
	_caldoriaFlags.setFlag(kCaldoriaSinclairShotFlag, value);
}

bool GameStateManager::getCaldoriaSinclairShot() {
	return _caldoriaFlags.getFlag(kCaldoriaSinclairShotFlag);
}

void GameStateManager::setCaldoriaBombDisarmed(bool value) {
	_caldoriaFlags.setFlag(kCaldoriaBombDisarmedFlag, value);
}

bool GameStateManager::getCaldoriaBombDisarmed() {
	return _caldoriaFlags.getFlag(kCaldoriaBombDisarmedFlag);
}

void GameStateManager::setCaldoriaRoofDoorOpen(bool value) {
	_caldoriaFlags.setFlag(kCaldoriaRoofDoorOpenFlag, value);
}

bool GameStateManager::getCaldoriaRoofDoorOpen() {
	return _caldoriaFlags.getFlag(kCaldoriaRoofDoorOpenFlag);
}

void GameStateManager::setCaldoriaDoneHygiene(bool value) {
	_caldoriaFlags.setFlag(kCaldoriaDoneHygieneFlag, value);
}

bool GameStateManager::getCaldoriaDoneHygiene() {
	return _caldoriaFlags.getFlag(kCaldoriaDoneHygieneFlag);
}

void GameStateManager::setCaldoriaSawVoiceAnalysis(bool value) {
	_caldoriaFlags.setFlag(kCaldoriaSawVoiceAnalysisFlag, value);
}

bool GameStateManager::getCaldoriaSawVoiceAnalysis() {
	return _caldoriaFlags.getFlag(kCaldoriaSawVoiceAnalysisFlag);
}

void GameStateManager::setCaldoriaDoorBombed(bool value) {
	_caldoriaFlags.setFlag(kCaldoriaDoorBombedFlag, value);
}

bool GameStateManager::getCaldoriaDoorBombed() {
	return _caldoriaFlags.getFlag(kCaldoriaDoorBombedFlag);
}

void GameStateManager::setCaldoriaGunAimed(bool value) {
	_caldoriaFlags.setFlag(kCaldoriaGunAimedFlag, value);
}

bool GameStateManager::getCaldoriaGunAimed() {
	return _caldoriaFlags.getFlag(kCaldoriaGunAimedFlag);
}

void GameStateManager::setRipTimerTime(TimeValue limit) {
	_TSARipTimerTime = limit;
}

TimeValue GameStateManager::getRipTimerTime() {
	return _TSARipTimerTime;
}

void GameStateManager::setTSAFuseTimeLimit(TimeValue limit) {
	_TSAFuseTimeLimit = limit;
}

TimeValue GameStateManager::getTSAFuseTimeLimit() {
	return _TSAFuseTimeLimit;
}

void GameStateManager::setTSAState(byte state) {
	_TSAState = state;
}

byte GameStateManager::getTSAState() {
	return _TSAState;
}

void GameStateManager::setT0BMonitorMode(byte mode) {
	_T0BMonitorMode = mode;
}

byte GameStateManager::getT0BMonitorMode() {
	return _T0BMonitorMode;
}

void GameStateManager::setT0BMonitorStart(TimeValue start) {
	_T0BMonitorStart = start;
}

TimeValue GameStateManager::getT0BMonitorStart() {
	return _T0BMonitorStart;
}

void GameStateManager::setTSAIDedAtDoor(bool value) {
	_TSAFlags.setFlag(kTSAIDedAtDoorFlag, value);
}

bool GameStateManager::getTSAIDedAtDoor() {
	return _TSAFlags.getFlag(kTSAIDedAtDoorFlag);
}

void GameStateManager::setTSA0BZoomedIn(bool value) {
	_TSAFlags.setFlag(kTSA0BZoomedInFlag, value);
}

bool GameStateManager::getTSA0BZoomedIn() {
	return _TSAFlags.getFlag(kTSA0BZoomedInFlag);
}

void GameStateManager::setTSAFrontDoorUnlockedOutside(bool value) {
	_TSAFlags.setFlag(kTSAFrontDoorUnlockedOutsideFlag, value);
}

bool GameStateManager::getTSAFrontDoorUnlockedOutside() {
	return _TSAFlags.getFlag(kTSAFrontDoorUnlockedOutsideFlag);
}

void GameStateManager::setTSAFrontDoorUnlockedInside(bool value) {
	_TSAFlags.setFlag(kTSAFrontDoorUnlockedInsideFlag, value);
}

bool GameStateManager::getTSAFrontDoorUnlockedInside() {
	return _TSAFlags.getFlag(kTSAFrontDoorUnlockedInsideFlag);
}

void GameStateManager::setTSASeenRobotGreeting(bool value) {
	_TSAFlags.setFlag(kTSASeenRobotGreetingFlag, value);
}

bool GameStateManager::getTSASeenRobotGreeting() {
	return _TSAFlags.getFlag(kTSASeenRobotGreetingFlag);
}

void GameStateManager::setTSASeenTheory(bool value) {
	_TSAFlags.setFlag(kTSASeenTheoryFlag, value);
}

bool GameStateManager::getTSASeenTheory() {
	return _TSAFlags.getFlag(kTSASeenTheoryFlag);
}

void GameStateManager::setTSASeenBackground(bool value) {
	_TSAFlags.setFlag(kTSASeenBackgroundFlag, value);
}

bool GameStateManager::getTSASeenBackground() {
	return _TSAFlags.getFlag(kTSASeenBackgroundFlag);
}

void GameStateManager::setTSASeenProcedure(bool value) {
	_TSAFlags.setFlag(kTSASeenProcedureFlag, value);
}

bool GameStateManager::getTSASeenProcedure() {
	return _TSAFlags.getFlag(kTSASeenProcedureFlag);
}

void GameStateManager::setTSASeenAgent3AtDoor(bool value) {
	_TSAFlags.setFlag(kTSASeenAgent3AtDoorFlag, value);
}

bool GameStateManager::getTSASeenAgent3AtDoor() {
	return _TSAFlags.getFlag(kTSASeenAgent3AtDoorFlag);
}

void GameStateManager::setTSACommandCenterLocked(bool value) {
	_TSAFlags.setFlag(kTSACommandCenterLockedFlag, value);
}

bool GameStateManager::getTSACommandCenterLocked() {
	return _TSAFlags.getFlag(kTSACommandCenterLockedFlag);
}

void GameStateManager::setTSASeenCaldoriaNormal(bool value) {
	_TSAFlags.setFlag(kTSASeenCaldoriaNormalFlag, value);
}

bool GameStateManager::getTSASeenCaldoriaNormal() {
	return _TSAFlags.getFlag(kTSASeenCaldoriaNormalFlag);
}

void GameStateManager::setTSASeenCaldoriaAltered(bool value) {
	_TSAFlags.setFlag(kTSASeenCaldoriaAlteredFlag, value);
}

bool GameStateManager::getTSASeenCaldoriaAltered() {
	return _TSAFlags.getFlag(kTSASeenCaldoriaAlteredFlag);
}

void GameStateManager::setTSASeenNoradNormal(bool value) {
	_TSAFlags.setFlag(kTSASeenNoradNormalFlag, value);
}

bool GameStateManager::getTSASeenNoradNormal() {
	return _TSAFlags.getFlag(kTSASeenNoradNormalFlag);
}

void GameStateManager::setTSASeenNoradAltered(bool value) {
	_TSAFlags.setFlag(kTSASeenNoradAlteredFlag, value);
}

bool GameStateManager::getTSASeenNoradAltered() {
	return _TSAFlags.getFlag(kTSASeenNoradAlteredFlag);
}

void GameStateManager::setTSASeenMarsNormal(bool value) {
	_TSAFlags.setFlag(kTSASeenMarsNormalFlag, value);
}

bool GameStateManager::getTSASeenMarsNormal() {
	return _TSAFlags.getFlag(kTSASeenMarsNormalFlag);
}

void GameStateManager::setTSASeenMarsAltered(bool value) {
	_TSAFlags.setFlag(kTSASeenMarsAlteredFlag, value);
}

bool GameStateManager::getTSASeenMarsAltered() {
	return _TSAFlags.getFlag(kTSASeenMarsAlteredFlag);
}

void GameStateManager::setTSASeenWSCNormal(bool value) {
	_TSAFlags.setFlag(kTSASeenWSCNormalFlag, value);
}

bool GameStateManager::getTSASeenWSCNormal() {
	return _TSAFlags.getFlag(kTSASeenWSCNormalFlag);
}

void GameStateManager::setTSASeenWSCAltered(bool value) {
	_TSAFlags.setFlag(kTSASeenWSCAlteredFlag, value);
}

bool GameStateManager::getTSASeenWSCAltered() {
	return _TSAFlags.getFlag(kTSASeenWSCAlteredFlag);
}

void GameStateManager::setTSABiosuitOn(bool value) {
	_TSAFlags.setFlag(kTSABiosuitOnFlag, value);
}

bool GameStateManager::getTSABiosuitOn() {
	return _TSAFlags.getFlag(kTSABiosuitOnFlag);
}

void GameStateManager::setPrehistoricTriedToExtendBridge(bool value) {
	_prehistoricFlags.setFlag(kPrehistoricTriedToExtendBridgeFlag, value);
}

bool GameStateManager::getPrehistoricTriedToExtendBridge() {
	return _prehistoricFlags.getFlag(kPrehistoricTriedToExtendBridgeFlag);
}

void GameStateManager::setPrehistoricSeenTimeStream(bool value) {
	_prehistoricFlags.setFlag(kPrehistoricSeenTimeStreamFlag, value);
}

bool GameStateManager::getPrehistoricSeenTimeStream() {
	return _prehistoricFlags.getFlag(kPrehistoricSeenTimeStreamFlag);
}

void GameStateManager::setPrehistoricSeenFlyer1(bool value) {
	_prehistoricFlags.setFlag(kPrehistoricSeenFlyer1Flag, value);
}

bool GameStateManager::getPrehistoricSeenFlyer1() {
	return _prehistoricFlags.getFlag(kPrehistoricSeenFlyer1Flag);
}

void GameStateManager::setPrehistoricSeenFlyer2(bool value) {
	_prehistoricFlags.setFlag(kPrehistoricSeenFlyer2Flag, value);
}

bool GameStateManager::getPrehistoricSeenFlyer2() {
	return _prehistoricFlags.getFlag(kPrehistoricSeenFlyer2Flag);
}

void GameStateManager::setPrehistoricSeenBridgeZoom(bool value) {
	_prehistoricFlags.setFlag(kPrehistoricSeenBridgeZoomFlag, value);
}

bool GameStateManager::getPrehistoricSeenBridgeZoom() {
	return _prehistoricFlags.getFlag(kPrehistoricSeenBridgeZoomFlag);
}

void GameStateManager::setPrehistoricBreakerThrown(bool value) {
	_prehistoricFlags.setFlag(kPrehistoricBreakerThrownFlag, value);
}

bool GameStateManager::getPrehistoricBreakerThrown() {
	return _prehistoricFlags.getFlag(kPrehistoricBreakerThrownFlag);
}

void GameStateManager::setNoradSeenTimeStream(bool value) {
	_noradFlags.setFlag(kNoradSeenTimeStreamFlag, value);
}

bool GameStateManager::getNoradSeenTimeStream() {
	return _noradFlags.getFlag(kNoradSeenTimeStreamFlag);
}

void GameStateManager::setNoradGassed(bool value) {
	_noradFlags.setFlag(kNoradGassedFlag, value);
}

bool GameStateManager::getNoradGassed() {
	return _noradFlags.getFlag(kNoradGassedFlag);
}

void GameStateManager::setNoradFillingStationOn(bool value) {
	_noradFlags.setFlag(kNoradFillingStationOnFlag, value);
}

bool GameStateManager::getNoradFillingStationOn() {
	return _noradFlags.getFlag(kNoradFillingStationOnFlag);
}

void GameStateManager::setNoradN22MessagePlayed(bool value) {
	_noradFlags.setFlag(kNoradN22MessagePlayedFlag, value);
}

bool GameStateManager::getNoradN22MessagePlayed() {
	return _noradFlags.getFlag(kNoradN22MessagePlayedFlag);
}

void GameStateManager::setNoradPlayedGlobeGame(bool value) {
	_noradFlags.setFlag(kNoradPlayedGlobeGameFlag, value);
}

bool GameStateManager::getNoradPlayedGlobeGame() {
	return _noradFlags.getFlag(kNoradPlayedGlobeGameFlag);
}

void GameStateManager::setNoradBeatRobotWithClaw(bool value) {
	_noradFlags.setFlag(kNoradBeatRobotWithClawFlag, value);
}

bool GameStateManager::getNoradBeatRobotWithClaw() {
	return _noradFlags.getFlag(kNoradBeatRobotWithClawFlag);
}

void GameStateManager::setNoradBeatRobotWithDoor(bool value) {
	_noradFlags.setFlag(kNoradBeatRobotWithDoorFlag, value);
}

bool GameStateManager::getNoradBeatRobotWithDoor() {
	return _noradFlags.getFlag(kNoradBeatRobotWithDoorFlag);
}

void GameStateManager::setNoradRetScanGood(bool value) {
	_noradFlags.setFlag(kNoradRetScanGoodFlag, value);
}

bool GameStateManager::getNoradRetScanGood() {
	return _noradFlags.getFlag(kNoradRetScanGoodFlag);
}

void GameStateManager::setNoradWaitingForLaser(bool value) {
	_noradFlags.setFlag(kNoradWaitingForLaserFlag, value);
}

bool GameStateManager::getNoradWaitingForLaser() {
	return _noradFlags.getFlag(kNoradWaitingForLaserFlag);
}

void GameStateManager::setNoradSubRoomPressure(uint16 pressure) {
	_noradSubRoomPressure = pressure;
}

uint16 GameStateManager::getNoradSubRoomPressure() {
	return _noradSubRoomPressure;
}

void GameStateManager::setNoradSubPrepState(NoradSubPrepState state) {
	_noradSubPrepState = state;
}

NoradSubPrepState GameStateManager::getNoradSubPrepState() {
	return _noradSubPrepState;
}

void GameStateManager::setNoradArrivedFromSub(bool value) {
	_noradFlags.setFlag(kNoradArrivedFromSubFlag, value);
}

bool GameStateManager::getNoradArrivedFromSub() {
	return _noradFlags.getFlag(kNoradArrivedFromSubFlag);
}

void GameStateManager::setMarsSeenTimeStream(bool value) {
	_marsFlags.setFlag(kMarsSeenTimeStreamFlag, value);
}

bool GameStateManager::getMarsSeenTimeStream() {
	return _marsFlags.getFlag(kMarsSeenTimeStreamFlag);
}

void GameStateManager::setMarsHeardUpperPodMessage(bool value) {
	_marsFlags.setFlag(kMarsHeardUpperPodMessageFlag, value);
}

bool GameStateManager::getMarsHeardUpperPodMessage() {
	return _marsFlags.getFlag(kMarsHeardUpperPodMessageFlag);
}

void GameStateManager::setMarsRobotThrownPlayer(bool value) {
	_marsFlags.setFlag(kMarsRobotThrownPlayerFlag, value);
}

bool GameStateManager::getMarsRobotThrownPlayer() {
	return _marsFlags.getFlag(kMarsRobotThrownPlayerFlag);
}

void GameStateManager::setMarsHeardCheckInMessage(bool value) {
	_marsFlags.setFlag(kMarsHeardCheckInMessageFlag, value);
}

bool GameStateManager::getMarsHeardCheckInMessage() {
	return _marsFlags.getFlag(kMarsHeardCheckInMessageFlag);
}

void GameStateManager::setMarsPodAtUpperPlatform(bool value) {
	_marsFlags.setFlag(kMarsPodAtUpperPlatformFlag, value);
}

bool GameStateManager::getMarsPodAtUpperPlatform() {
	return _marsFlags.getFlag(kMarsPodAtUpperPlatformFlag);
}

void GameStateManager::setMarsSeenThermalScan(bool value) {
	_marsFlags.setFlag(kMarsSeenThermalScanFlag, value);
}

bool GameStateManager::getMarsSeenThermalScan() {
	return _marsFlags.getFlag(kMarsSeenThermalScanFlag);
}

void GameStateManager::setMarsArrivedBelow(bool value) {
	_marsFlags.setFlag(kMarsArrivedBelowFlag, value);
}

bool GameStateManager::getMarsArrivedBelow() {
	return _marsFlags.getFlag(kMarsArrivedBelowFlag);
}

void GameStateManager::setMarsSeenRobotAtReactor(bool value) {
	_marsFlags.setFlag(kMarsSeenRobotAtReactorFlag, value);
}

bool GameStateManager::getMarsSeenRobotAtReactor() {
	return _marsFlags.getFlag(kMarsSeenRobotAtReactorFlag);
}

void GameStateManager::setMarsAvoidedReactorRobot(bool value) {
	_marsFlags.setFlag(kMarsAvoidedReactorRobotFlag, value);
}

bool GameStateManager::getMarsAvoidedReactorRobot() {
	return _marsFlags.getFlag(kMarsAvoidedReactorRobotFlag);
}

void GameStateManager::setMarsSecurityDown(bool value) {
	_marsFlags.setFlag(kMarsSecurityDownFlag, value);
}

bool GameStateManager::getMarsSecurityDown() {
	return _marsFlags.getFlag(kMarsSecurityDownFlag);
}

void GameStateManager::setMarsInAirlock(bool value) {
	_marsFlags.setFlag(kMarsInAirlockFlag, value);
}

bool GameStateManager::getMarsInAirlock() {
	return _marsFlags.getFlag(kMarsInAirlockFlag);
}

void GameStateManager::setMarsAirlockOpen(bool value) {
	_marsFlags.setFlag(kMarsAirlockOpenFlag, value);
}

bool GameStateManager::getMarsAirlockOpen() {
	return _marsFlags.getFlag(kMarsAirlockOpenFlag);
}

void GameStateManager::setMarsMaskOnFiller(bool value) {
	_marsFlags.setFlag(kMarsMaskOnFillerFlag, value);
}

bool GameStateManager::getMarsMaskOnFiller() {
	return _marsFlags.getFlag(kMarsMaskOnFillerFlag);
}

void GameStateManager::setMarsLockFrozen(bool value) {
	_marsFlags.setFlag(kMarsLockFrozenFlag, value);
}

bool GameStateManager::getMarsLockFrozen() {
	return _marsFlags.getFlag(kMarsLockFrozenFlag);
}

void GameStateManager::setMarsLockBroken(bool value) {
	_marsFlags.setFlag(kMarsLockBrokenFlag, value);
}

bool GameStateManager::getMarsLockBroken() {
	return _marsFlags.getFlag(kMarsLockBrokenFlag);
}

void GameStateManager::setMarsMazeDoorPair1(bool value) {
	_marsFlags.setFlag(kMarsMazeDoorPair1Flag, value);
}

bool GameStateManager::getMarsMazeDoorPair1() {
	return _marsFlags.getFlag(kMarsMazeDoorPair1Flag);
}

void GameStateManager::setMarsMazeDoorPair2(bool value) {
	_marsFlags.setFlag(kMarsMazeDoorPair2Flag, value);
}

bool GameStateManager::getMarsMazeDoorPair2() {
	return _marsFlags.getFlag(kMarsMazeDoorPair2Flag);
}

void GameStateManager::setMarsMazeDoorPair3(bool value) {
	_marsFlags.setFlag(kMarsMazeDoorPair3Flag, value);
}

bool GameStateManager::getMarsMazeDoorPair3() {
	return _marsFlags.getFlag(kMarsMazeDoorPair3Flag);
}

void GameStateManager::setMarsSawRobotLeave(bool value) {
	_marsFlags.setFlag(kMarsSawRobotLeaveFlag, value);
}

bool GameStateManager::getMarsSawRobotLeave() {
	return _marsFlags.getFlag(kMarsSawRobotLeaveFlag);
}

void GameStateManager::setMarsHitRobotWithCannon(bool flag) {
	_marsFlags.setFlag(kMarsHitRobotWithCannonFlag, flag);
}

bool GameStateManager::getMarsHitRobotWithCannon() {
	return _marsFlags.getFlag(kMarsHitRobotWithCannonFlag);
}

void GameStateManager::setMarsReadyForShuttleTransport(bool value) {
	_marsFlags.setFlag(kMarsReadyForShuttleTransportFlag, value);
}

bool GameStateManager::getMarsReadyForShuttleTransport() {
	return _marsFlags.getFlag(kMarsReadyForShuttleTransportFlag);
}

void GameStateManager::setMarsFinishedCanyonChase(bool flag) {
	_marsFlags.setFlag(kMarsFinishedCanyonChaseFlag, flag);
}

bool GameStateManager::getMarsFinishedCanyonChase() {
	return _marsFlags.getFlag(kMarsFinishedCanyonChaseFlag);
}

void GameStateManager::setMarsThreadedMaze(bool flag) {
	_marsFlags.setFlag(kMarsThreadedMazeFlag, flag);
}

bool GameStateManager::getMarsThreadedMaze() {
	return _marsFlags.getFlag(kMarsThreadedMazeFlag);
}

void GameStateManager::setWSCSeenTimeStream(bool value) {
	_WSCFlags.setFlag(kWSCSeenTimeStreamFlag, value);
}

bool GameStateManager::getWSCSeenTimeStream() {
	return _WSCFlags.getFlag(kWSCSeenTimeStreamFlag);
}

void GameStateManager::setWSCPoisoned(bool value) {
	_WSCFlags.setFlag(kWSCPoisonedFlag, value);
}

bool GameStateManager::getWSCPoisoned() {
	return _WSCFlags.getFlag(kWSCPoisonedFlag);
}

void GameStateManager::setWSCAnsweredAboutDart(bool value) {
	_WSCFlags.setFlag(kWSCAnsweredAboutDartFlag, value);
}

bool GameStateManager::getWSCAnsweredAboutDart() {
	return _WSCFlags.getFlag(kWSCAnsweredAboutDartFlag);
}

void GameStateManager::setWSCRemovedDart(bool value) {
	_WSCFlags.setFlag(kWSCRemovedDartFlag, value);
}

bool GameStateManager::getWSCRemovedDart() {
	return _WSCFlags.getFlag(kWSCRemovedDartFlag);
}

void GameStateManager::setWSCAnalyzerOn(bool value) {
	_WSCFlags.setFlag(kWSCAnalyzerOnFlag, value);
}

bool GameStateManager::getWSCAnalyzerOn() {
	return _WSCFlags.getFlag(kWSCAnalyzerOnFlag);
}

void GameStateManager::setWSCDartInAnalyzer(bool value) {
	_WSCFlags.setFlag(kWSCDartInAnalyzerFlag, value);
}

bool GameStateManager::getWSCDartInAnalyzer() {
	return _WSCFlags.getFlag(kWSCDartInAnalyzerFlag);
}

void GameStateManager::setWSCAnalyzedDart(bool value) {
	_WSCFlags.setFlag(kWSCAnalyzedDartFlag, value);
}

bool GameStateManager::getWSCAnalyzedDart() {
	return _WSCFlags.getFlag(kWSCAnalyzedDartFlag);
}

void GameStateManager::setWSCSawMorph(bool value) {
	_WSCFlags.setFlag(kWSCSawMorphFlag, value);
}

bool GameStateManager::getWSCSawMorph() {
	return _WSCFlags.getFlag(kWSCSawMorphFlag);
}

void GameStateManager::setWSCDesignedAntidote(bool value) {
	_WSCFlags.setFlag(kWSCDesignedAntidoteFlag, value);
}

bool GameStateManager::getWSCDesignedAntidote() {
	return _WSCFlags.getFlag(kWSCDesignedAntidoteFlag);
}

void GameStateManager::setWSCPickedUpAntidote(bool value) {
	_WSCFlags.setFlag(kWSCPickedUpAntidoteFlag, value);
}

bool GameStateManager::getWSCPickedUpAntidote() {
	return _WSCFlags.getFlag(kWSCPickedUpAntidoteFlag);
}

void GameStateManager::setWSCOfficeMessagesOpen(bool value) {
	_WSCFlags.setFlag(kWSCOfficeMessagesOpenFlag, value);
}

bool GameStateManager::getWSCOfficeMessagesOpen() {
	return _WSCFlags.getFlag(kWSCOfficeMessagesOpenFlag);
}

void GameStateManager::setWSCSeenNerd(bool value) {
	_WSCFlags.setFlag(kWSCSeenNerdFlag, value);
}

bool GameStateManager::getWSCSeenNerd() {
	return _WSCFlags.getFlag(kWSCSeenNerdFlag);
}

void GameStateManager::setWSCHeardPage1(bool value) {
	_WSCFlags.setFlag(kWSCHeardPage1Flag, value);
}

bool GameStateManager::getWSCHeardPage1() {
	return _WSCFlags.getFlag(kWSCHeardPage1Flag);
}

void GameStateManager::setWSCHeardPage2(bool value) {
	_WSCFlags.setFlag(kWSCHeardPage2Flag, value);
}

bool GameStateManager::getWSCHeardPage2() {
	return _WSCFlags.getFlag(kWSCHeardPage2Flag);
}

void GameStateManager::setWSCHeardCheckIn(bool value) {
	_WSCFlags.setFlag(kWSCHeardCheckInFlag, value);
}

bool GameStateManager::getWSCHeardCheckIn() {
	return _WSCFlags.getFlag(kWSCHeardCheckInFlag);
}

void GameStateManager::setWSCDidPlasmaDodge(bool value) {
	_WSCFlags.setFlag(kWSCDidPlasmaDodgeFlag, value);
}

bool GameStateManager::getWSCDidPlasmaDodge() {
	return _WSCFlags.getFlag(kWSCDidPlasmaDodgeFlag);
}

void GameStateManager::setWSCSeenSinclairLecture(bool value) {
	_WSCFlags.setFlag(kWSCSeenSinclairLectureFlag, value);
}

bool GameStateManager::getWSCSeenSinclairLecture() {
	return _WSCFlags.getFlag(kWSCSeenSinclairLectureFlag);
}

void GameStateManager::setWSCBeenAtWSC93(bool value) {
	_WSCFlags.setFlag(kWSCBeenAtWSC93Flag, value);
}

bool GameStateManager::getWSCBeenAtWSC93() {
	return _WSCFlags.getFlag(kWSCBeenAtWSC93Flag);
}

void GameStateManager::setWSCCatwalkDark(bool value) {
	_WSCFlags.setFlag(kWSCCatwalkDarkFlag, value);
}

bool GameStateManager::getWSCCatwalkDark() {
	return _WSCFlags.getFlag(kWSCCatwalkDarkFlag);
}

void GameStateManager::setWSCRobotDead(bool value) {
	_WSCFlags.setFlag(kWSCRobotDeadFlag, value);
}

bool GameStateManager::getWSCRobotDead() {
	return _WSCFlags.getFlag(kWSCRobotDeadFlag);
}

void GameStateManager::setWSCRobotGone(bool value) {
	_WSCFlags.setFlag(kWSCRobotGoneFlag, value);
}

bool GameStateManager::getWSCRobotGone() {
	return _WSCFlags.getFlag(kWSCRobotGoneFlag);
}

} // End of namespace Pegasus
