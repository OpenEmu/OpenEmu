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

#ifndef ENGINES_GAME_H
#define ENGINES_GAME_H

#include "common/array.h"
#include "common/hash-str.h"
#include "common/str.h"
#include "common/language.h"
#include "common/platform.h"

/**
 * A simple structure used to map gameids (like "monkey", "sword1", ...) to
 * nice human readable and descriptive game titles (like "The Secret of Monkey Island").
 * This is a plain struct to make it possible to declare NULL-terminated C arrays
 * consisting of PlainGameDescriptors.
 */
struct PlainGameDescriptor {
	const char *gameid;
	const char *description;
};

/**
 * Given a list of PlainGameDescriptors, returns the first PlainGameDescriptor
 * matching the given gameid. If not match is found return 0.
 * The end of the list must be marked by an entry with gameid 0.
 */
const PlainGameDescriptor *findPlainGameDescriptor(const char *gameid, const PlainGameDescriptor *list);

/**
 * Ths is an enum to describe how done a game is. This also indicates what level of support is expected.
 */
enum GameSupportLevel {
	kStableGame = 0, // the game is fully supported
	kTestingGame, // the game is not supposed to end up in releases yet but is ready for public testing
	kUnstableGame // the game is not even ready for public testing yet
};

/**
 * A hashmap describing details about a given game. In a sense this is a refined
 * version of PlainGameDescriptor, as it also contains a gameid and a description string.
 * But in addition, platform and language settings, as well as arbitrary other settings,
 * can be contained in a GameDescriptor.
 * This is an essential part of the glue between the game engines and the launcher code.
 */
class GameDescriptor : public Common::StringMap {
public:
	GameDescriptor();
	GameDescriptor(const PlainGameDescriptor &pgd, Common::String guioptions = Common::String());
	GameDescriptor(const Common::String &gameid,
	              const Common::String &description,
	              Common::Language language = Common::UNK_LANG,
				  Common::Platform platform = Common::kPlatformUnknown,
				  Common::String guioptions = Common::String(),
				  GameSupportLevel gsl = kStableGame);

	/**
	 * Update the description string by appending (EXTRA/PLATFORM/LANG) to it.
	 * Values that are missing are omitted, so e.g. (EXTRA/LANG) would be
	 * added if no platform has been specified but a language and an extra string.
	 */
	void updateDesc(const char *extra = 0);

	void setGUIOptions(Common::String options);
	void appendGUIOptions(const Common::String &str);

	/**
	 * What level of support is expected of this game
	 */
	GameSupportLevel getSupportLevel();
	void setSupportLevel(GameSupportLevel gsl);

	Common::String &gameid() { return getVal("gameid"); }
	Common::String &description() { return getVal("description"); }
	const Common::String &gameid() const { return getVal("gameid"); }
	const Common::String &description() const { return getVal("description"); }
	Common::Language language() const { return contains("language") ? Common::parseLanguage(getVal("language")) : Common::UNK_LANG; }
	Common::Platform platform() const { return contains("platform") ? Common::parsePlatform(getVal("platform")) : Common::kPlatformUnknown; }

	const Common::String &preferredtarget() const {
		return contains("preferredtarget") ? getVal("preferredtarget") : getVal("gameid");
	}
};

/** List of games. */
class GameList : public Common::Array<GameDescriptor> {
public:
	GameList() {}
	GameList(const GameList &list) : Common::Array<GameDescriptor>(list) {}
	GameList(const PlainGameDescriptor *g) {
		while (g->gameid) {
			push_back(GameDescriptor(*g));
			g++;
		}
	}
};

#endif
