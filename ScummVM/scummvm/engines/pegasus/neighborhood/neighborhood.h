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

#ifndef PEGASUS_NEIGHBORHOOD_H
#define PEGASUS_NEIGHBORHOOD_H

#include "common/queue.h"
#include "common/str.h"

#include "pegasus/fader.h"
#include "pegasus/hotspot.h"
#include "pegasus/input.h"
#include "pegasus/movie.h"
#include "pegasus/notification.h"
#include "pegasus/sound.h"
#include "pegasus/timers.h"
#include "pegasus/transition.h"
#include "pegasus/util.h"
#include "pegasus/neighborhood/door.h"
#include "pegasus/neighborhood/exit.h"
#include "pegasus/neighborhood/extra.h"
#include "pegasus/neighborhood/hotspotinfo.h"
#include "pegasus/neighborhood/spot.h"
#include "pegasus/neighborhood/turn.h"
#include "pegasus/neighborhood/view.h"
#include "pegasus/neighborhood/zoom.h"

namespace Pegasus {

class PegasusEngine;

// Pegasus Prime neighborhood id's
static const NeighborhoodID kCaldoriaID = 0;
static const NeighborhoodID kFullTSAID = 1;
static const NeighborhoodID kFinalTSAID = 2;
static const NeighborhoodID kTinyTSAID = 3;
static const NeighborhoodID kPrehistoricID = 4;
static const NeighborhoodID kMarsID = 5;
static const NeighborhoodID kWSCID = 6;
static const NeighborhoodID kNoradAlphaID = 7;
static const NeighborhoodID kNoradDeltaID = 8;
// The sub chase is not really a neighborhood, but we define a constant that is used
// to allow an easy transition out of Norad Alpha.
static const NeighborhoodID kNoradSubChaseID = 1000;

static const TimeScale kDefaultLoopFadeScale = kThirtyTicksPerSecond;
static const TimeValue kDefaultLoopFadeOut = kHalfSecondPerThirtyTicks;
static const TimeValue kDefaultLoopFadeIn = kHalfSecondPerThirtyTicks;

enum QueueRequestType {
	kNavExtraRequest,
	kSpotSoundRequest,
	kDelayRequest
};

// For delay requests, start is interpreted as the total delay and stop is interpreted
// as the scale the delay is in.
// For extra requests, start and stop are not used.
struct QueueRequest {
	QueueRequestType requestType;
	ExtraID extra;
	TimeValue start, stop;
	InputBits interruptionFilter;
	bool playing;
	NotificationFlags flags;
	Notification *notification;
};

bool operator==(const QueueRequest &arg1, const QueueRequest &arg2);
bool operator!=(const QueueRequest &arg1, const QueueRequest &arg2);

class GameInteraction;
class Item;
class Neighborhood;

class StriderCallBack : public TimeBaseCallBack {
public:
	StriderCallBack(Neighborhood *);
	virtual ~StriderCallBack() {}

protected:
	virtual void callBack();

	Neighborhood *_neighborhood;
};

typedef Common::Queue<QueueRequest> NeighborhoodActionQueue;

class Neighborhood : public IDObject, public NotificationReceiver, public InputHandler, public Idler {
friend class StriderCallBack;

public:
	Neighborhood(InputHandler *nextHandler, PegasusEngine *vm, const Common::String &resName, NeighborhoodID id);
	virtual ~Neighborhood();

	virtual void init();
	virtual void start();
	virtual void moveNavTo(const CoordType, const CoordType);
	virtual void checkContinuePoint(const RoomID, const DirectionConstant) = 0;
	void makeContinuePoint();

	virtual void activateHotspots();
	virtual void clickInHotspot(const Input &, const Hotspot *);

	virtual CanMoveForwardReason canMoveForward(ExitTable::Entry &entry);
	virtual CanTurnReason canTurn(TurnDirection turn, DirectionConstant &nextDir);
	virtual CanOpenDoorReason canOpenDoor(DoorTable::Entry &entry);

	virtual void cantMoveThatWay(CanMoveForwardReason);
	virtual void cantTurnThatWay(CanTurnReason) {}
	virtual void cantOpenDoor(CanOpenDoorReason);
	virtual void arriveAt(const RoomID room, const DirectionConstant direction);
	virtual void turnTo(const DirectionConstant);
	virtual void spotCompleted();
	virtual void doorOpened();
	virtual void closeDoorOffScreen(const RoomID, const DirectionConstant) {}

	virtual void moveForward();
	virtual void turn(const TurnDirection);
	virtual void turnLeft();
	virtual void turnRight();
	virtual void turnUp();
	virtual void turnDown();
	virtual void openDoor();
	virtual void zoomTo(const Hotspot *);

	virtual void updateViewFrame();

	void requestExtraSequence(const ExtraID, const NotificationFlags, const InputBits interruptionFilter);
	void requestSpotSound(const TimeValue, const TimeValue, const InputBits interruptionFilter, const NotificationFlags);
	void playSpotSoundSync(const TimeValue in, const TimeValue out);
	void requestDelay(const TimeValue, const TimeScale, const InputBits interruptionFilter, const NotificationFlags);

	Notification *getNeighborhoodNotification() { return &_neighborhoodNotification; }

	virtual void getExtraEntry(const uint32 id, ExtraTable::Entry &extraEntry);
	virtual void startSpotLoop(TimeValue, TimeValue, NotificationFlags = 0);
	virtual bool actionQueueEmpty() { return _actionQueue.empty(); }
	virtual void showViewFrame(TimeValue);
	virtual void findSpotEntry(const RoomID room, const DirectionConstant direction, SpotFlags flags, SpotTable::Entry &spotEntry);
	virtual void startExtraSequence(const ExtraID, const NotificationFlags, const InputBits interruptionFilter);
	bool startExtraSequenceSync(const ExtraID, const InputBits);
	virtual void loopExtraSequence(const uint32, NotificationFlags = 0);
	int32 getLastExtra() const { return _lastExtra; }
	virtual void scheduleNavCallBack(NotificationFlags);

	Movie *getNavMovie() { return &_navMovie; }
	bool navMoviePlaying();

	void setCurrentAlternate(const AlternateID alt) { _currentAlternate = alt; }
	AlternateID getCurrentAlternate() const { return _currentAlternate; }

	void setCurrentActivation(const HotSpotActivationID a) { _currentActivation = a; }
	HotSpotActivationID getCurrentActivation() { return _currentActivation; }

	virtual void playDeathExtra(ExtraID, DeathReason);
	virtual void die(const DeathReason);

	virtual void setSoundFXLevel(const uint16);
	virtual void setAmbienceLevel(const uint16);

	void forceStridingStop(const RoomID, const DirectionConstant, const AlternateID);
	void restoreStriding(const RoomID, const DirectionConstant, const AlternateID);

	HotspotInfoTable::Entry *findHotspotEntry(const HotSpotID);

	Push *getTurnPush() { return &_turnPush; }
	Picture *getTurnPushPicture() { return &_pushIn; }

	void hideNav();
	void showNav();

	virtual void loadAmbientLoops() {}

	virtual void flushGameState() {}

	virtual Common::String getBriefingMovie();
	virtual Common::String getEnvScanMovie();
	virtual uint getNumHints();
	virtual Common::String getHintMovie(uint);
	virtual bool canSolve();
	virtual void prepareForAIHint(const Common::String &) {}
	virtual void cleanUpAfterAIHint(const Common::String &) {}
	virtual void doSolve();

	virtual bool okayToJump();

	virtual AirQuality getAirQuality(const RoomID);
	virtual void checkAirMask() {}
	virtual void checkFlashlight() {}
	virtual void shieldOn() {}
	virtual void shieldOff() {}

	virtual void loadLoopSound1(const Common::String &, const uint16 volume = 0x100,
			const TimeValue fadeOut = kDefaultLoopFadeOut, const TimeValue fadeIn = kDefaultLoopFadeIn,
			const TimeScale fadeScale = kDefaultLoopFadeScale);
	virtual void loadLoopSound2(const Common::String &, const uint16 volume = 0x100,
			const TimeValue fadeOut = kDefaultLoopFadeOut, const TimeValue fadeIn = kDefaultLoopFadeIn,
			const TimeScale fadeScale = kDefaultLoopFadeScale);
	bool loop1Loaded(const Common::String &soundName) { return _loop1SoundString == soundName; }
	bool loop2Loaded(const Common::String &soundName) { return _loop2SoundString == soundName; }
	void startLoop1Fader(const FaderMoveSpec &);
	void startLoop2Fader(const FaderMoveSpec &);

	virtual void takeItemFromRoom(Item *);
	virtual void dropItemIntoRoom(Item *, Hotspot *);
	virtual Hotspot *getItemScreenSpot(Item *, DisplayElement *) { return 0; }

	virtual GameInteraction *makeInteraction(const InteractionID);
	virtual void requestDeleteCurrentInteraction() { _doneWithInteraction = true; }

	virtual uint16 getDateResID() const = 0;

	virtual void showExtraView(uint32);
	virtual void startExtraLongSequence(const uint32, const uint32, NotificationFlags, const InputBits interruptionFilter);

	void openCroppedMovie(const Common::String &, CoordType, CoordType);
	void loopCroppedMovie(const Common::String &, CoordType, CoordType);
	void closeCroppedMovie();
	void playCroppedMovieOnce(const Common::String &, CoordType, CoordType, const InputBits interruptionFilter = kFilterNoInput);

	void playMovieSegment(Movie *, TimeValue = 0, TimeValue = 0xffffffff);

	virtual void recallToTSASuccess();
	virtual void recallToTSAFailure();

	virtual void pickedUpItem(Item *) {}

	virtual void handleInput(const Input &, const Hotspot *);
protected:
	PegasusEngine *_vm;
	Common::String _resName;

	virtual Common::String getSoundSpotsName() = 0;
	virtual Common::String getNavMovieName() = 0;

	// Notification function.
	virtual void receiveNotification(Notification *, const NotificationFlags);

	// Map info functions.
	virtual void getExitEntry(const RoomID room, const DirectionConstant direction, ExitTable::Entry &entry);
	virtual TimeValue getViewTime(const RoomID room, const DirectionConstant direction);
	virtual void getDoorEntry(const RoomID room, const DirectionConstant direction, DoorTable::Entry &doorEntry);
	virtual DirectionConstant getTurnEntry(const RoomID room, const DirectionConstant direction, const TurnDirection turn);
	virtual void getZoomEntry(const HotSpotID id, ZoomTable::Entry &zoomEntry);
	virtual void getHotspotEntry(const HotSpotID id, HotspotInfoTable::Entry &hotspotEntry);

	// Nav movie sequences.
	virtual void startExitMovie(const ExitTable::Entry &);
	virtual void keepStriding(ExitTable::Entry &);
	virtual void stopStriding();
	virtual void checkStriding();
	virtual bool stillMoveForward();
	virtual void scheduleStridingCallBack(const TimeValue, NotificationFlags flags);
	virtual void startZoomMovie(const ZoomTable::Entry &);
	virtual void startDoorOpenMovie(const TimeValue, const TimeValue);
	virtual void startTurnPush(const TurnDirection, const TimeValue, const DirectionConstant);
	virtual void playExtraMovie(const ExtraTable::Entry &, const NotificationFlags, const InputBits interruptionFilter);

	virtual void activateCurrentView(const RoomID, const DirectionConstant, SpotFlags);

	virtual void activateOneHotspot(HotspotInfoTable::Entry &, Hotspot *);

	virtual void startSpotOnceOnly(TimeValue, TimeValue);

	virtual void startMovieSequence(const TimeValue, const TimeValue, NotificationFlags,
  			bool loopSequence, const InputBits interruptionFilter, const TimeValue strideStop = 0xffffffff);

	virtual void createNeighborhoodSpots();

	void resetLastExtra() { _lastExtra = -1; }

	virtual void throwAwayInterface();

	// Action queue stuff
	void popActionQueue();
	void serviceActionQueue();
	void requestAction(const QueueRequestType, const ExtraID, const TimeValue, const TimeValue, const InputBits, const NotificationFlags);

	virtual bool prepareExtraSync(const ExtraID);
	virtual bool waitMovieFinish(Movie *, const InputBits);

	virtual InputBits getInputFilter();

	// Misc.
	virtual int16 getStaticCompassAngle(const RoomID, const DirectionConstant dir);
	virtual void getExitCompassMove(const ExitTable::Entry &, FaderMoveSpec &);
	virtual void getZoomCompassMove(const ZoomTable::Entry &, FaderMoveSpec&);
	virtual void getExtraCompassMove(const ExtraTable::Entry &, FaderMoveSpec&);

	virtual void setUpAIRules();
	virtual void setHotspotFlags(const HotSpotID, const HotSpotFlags);
	virtual void setIsItemTaken(const ItemID);

	virtual void upButton(const Input &);
	virtual void leftButton(const Input &);
	virtual void rightButton(const Input &);
	virtual void downButton(const Input &);

	void initOnePicture(Picture *, const Common::String &, DisplayOrder, CoordType, CoordType, bool);
	void initOneMovie(Movie *, const Common::String &, DisplayOrder, CoordType, CoordType, bool);

	void reinstateMonocleInterface();

	virtual void newInteraction(const InteractionID);
	virtual void useIdleTime();
	virtual void bumpIntoWall();
	virtual void zoomUpOrBump();

	void scheduleEvent(const TimeValue, const TimeScale, const uint32);
	void cancelEvent();
	virtual void timerExpired(const uint32) {}
	bool isEventTimerRunning() { return _eventTimer.isFuseLit(); }
	uint32 getTimerEvent() { return _timerEvent; }
	void timerFunction();

	void pauseTimer();
	void resumeTimer();
	bool timerPaused();

	// Navigation Data
	DoorTable _doorTable;
	ExitTable _exitTable;
	ExtraTable _extraTable;
	HotspotInfoTable _hotspotInfoTable;
	SpotTable _spotTable;
	TurnTable _turnTable;
	ViewTable _viewTable;
	ZoomTable _zoomTable;
	AlternateID _currentAlternate;
	HotSpotActivationID _currentActivation;

	int32 _lastExtra;
	DeathReason _extraDeathReason;

	// Graphics
	Movie _navMovie;
	Picture _pushIn;
	Push _turnPush;

	// Callbacks
	Notification _neighborhoodNotification;
	NotificationCallBack _navMovieCallBack;
	StriderCallBack _stridingCallBack;
	NotificationCallBack _turnPushCallBack;
	NotificationCallBack _spotSoundCallBack;
	NotificationCallBack _delayCallBack;

	// Hotspots
	HotspotList _neighborhoodHotspots;

	// Sounds
	SoundTimeBase _spotSounds;

	// Action queue
	NeighborhoodActionQueue _actionQueue;
	TimeBase _delayTimer;

	// Interruptibility...
	InputBits _interruptionFilter;

	// Nav hiding (for info support...)
	bool _isRunning;

	GameInteraction *_currentInteraction;
	bool _doneWithInteraction;
	Movie _croppedMovie;

	Sound _soundLoop1;
	Common::String _loop1SoundString;
	SoundFader _loop1Fader;

	Sound _soundLoop2;
	Common::String _loop2SoundString;
	SoundFader _loop2Fader;

	// The event timer...
	FuseFunction _eventTimer;
	uint32 _timerEvent;
};

extern Neighborhood *g_neighborhood;

} // End of namespace Pegasus

#endif
