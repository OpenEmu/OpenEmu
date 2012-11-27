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

#ifndef LASTEXPRESS_HELPERS_H
#define LASTEXPRESS_HELPERS_H

//////////////////////////////////////////////////////////////////////////
// Misc helpers
//////////////////////////////////////////////////////////////////////////

#define LOW_BYTE(w)           ((unsigned char)(((unsigned long)(w)) & 0xff))

// Misc
#define getArchive(name) _engine->getResourceManager()->getFileStream(name)
#define rnd(value) _engine->getRandom().getRandomNumber(value - 1)

// Engine subclasses
#define getLogic() _engine->getGameLogic()
#define getMenu() _engine->getGameMenu()

// Logic
#define getAction() getLogic()->getGameAction()
#define getBeetle() getLogic()->getGameBeetle()
#define getFight() getLogic()->getGameFight()
#define getEntities() getLogic()->getGameEntities()
#define getSaveLoad() getLogic()->getGameSaveLoad()
#define isNight() getLogic()->getGameState()->isNightTime()

// State
#define getState() getLogic()->getGameState()->getGameState()
#define getEvent(id) getState()->events[id]
#define getFlags() getLogic()->getGameState()->getGameFlags()
#define getInventory() getLogic()->getGameState()->getGameInventory()
#define getObjects() getLogic()->getGameState()->getGameObjects()
#define getProgress() getState()->progress
#define getSavePoints() getLogic()->getGameState()->getGameSavePoints()
#define getGlobalTimer() getLogic()->getGameState()->getTimer()
#define setGlobalTimer(timer) getLogic()->getGameState()->setTimer(timer)
#define setCoords(coords) getLogic()->getGameState()->setCoordinates(coords)
#define getCoords() getLogic()->getGameState()->getCoordinates()
#define setFrameCount(count) _engine->setFrameCounter(count)
#define getFrameCount() _engine->getFrameCounter()

// Scenes
#define getScenes() _engine->getSceneManager()

// Sound
#define getSound() _engine->getSoundManager()
#define getSoundQueue() _engine->getSoundManager()->getQueue()

// Others
#define getEntityData(entity) getEntities()->getData(entity)

//////////////////////////////////////////////////////////////////////////
// Graphics
//////////////////////////////////////////////////////////////////////////

// Sequences
#define loadSequence(name) Sequence::load(name, getArchive(name))
#define loadSequence1(name, field30) Sequence::load(name, getArchive(name), field30)

#define clearBg(type) _engine->getGraphicsManager()->clear(type)
#define showScene(index, type) _engine->getGraphicsManager()->draw(getScenes()->get(index), type);

#define askForRedraw() _engine->getGraphicsManager()->change()
#define redrawScreen() do { _engine->getGraphicsManager()->update(); _engine->_system->updateScreen(); } while (false)

// Used to delete entity sequences
#define SAFE_DELETE(_p) do { delete (_p); (_p) = NULL; } while (false)

//////////////////////////////////////////////////////////////////////////
// Output
//////////////////////////////////////////////////////////////////////////
extern const char *g_actionNames[];
extern const char *g_directionNames[];
extern const char *g_entityNames[];

#define ACTION_NAME(action) (action > 18 ? Common::String::format("%d", action).c_str() : g_actionNames[action])
#define DIRECTION_NAME(direction) (direction >= 6 ? "INVALID" : g_directionNames[direction])
#define ENTITY_NAME(index) (index >= 40 ? "INVALID" : g_entityNames[index])


#endif // LASTEXPRESS_HELPERS_H
