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

#include "sci/engine/kernel.h"
#include "sci/engine/object.h"
#include "sci/engine/state.h"
#include "sci/engine/vm.h"
#include "sci/engine/workarounds.h"

#define SCI_WORKAROUNDENTRY_TERMINATOR { (SciGameId)0, -1, -1, 0, NULL, NULL, -1, 0, { WORKAROUND_NONE, 0 } }

namespace Sci {

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,             workaround
const SciWorkaroundEntry arithmeticWorkarounds[] = {
	{ GID_CAMELOT,         92,   92,  0,     "endingCartoon2", "changeState", 0x20d,    0, { WORKAROUND_FAKE,   0 } }, // op_lai: during the ending, sub gets called with no parameters, uses parameter 1 which is theGrail in this case - bug #3044734
	{ GID_ECOQUEST2,      100,    0,  0,               "Rain", "points",      0xcc6,    0, { WORKAROUND_FAKE,   0 } }, // op_or: when giving the papers to the customs officer, gets called against a pointer instead of a number - bug #3034464
	{ GID_ECOQUEST2,      100,    0,  0,               "Rain", "points",      0xce0,    0, { WORKAROUND_FAKE,   0 } }, // Same as above, for the Spanish version - bug #3313962
	{ GID_FANMADE,        516,  983,  0,             "Wander", "setTarget",      -1,    0, { WORKAROUND_FAKE,   0 } }, // op_mul: The Legend of the Lost Jewel Demo (fan made): called with object as second parameter when attacked by insects - bug #3038913
	{ GID_GK1,            800,64992,  0,                "Fwd", "doit",           -1,    0, { WORKAROUND_FAKE,   1 } }, // op_gt: when Mosely finds Gabriel and Grace near the end of the game, compares the Grooper object with 7
	{ GID_ICEMAN,         199,  977,  0,            "Grooper", "doit",           -1,    0, { WORKAROUND_FAKE,   0 } }, // op_add: While dancing with the girl
	{ GID_MOTHERGOOSE256,  -1,  999,  0,              "Event", "new",            -1,    0, { WORKAROUND_FAKE,   0 } }, // op_and: constantly during the game (SCI1 version)
	{ GID_MOTHERGOOSE256,  -1,    4,  0,              "rm004", "doit",           -1,    0, { WORKAROUND_FAKE,   0 } }, // op_or: when going north and reaching the castle (rooms 4 and 37) - bug #3038228
	{ GID_MOTHERGOOSEHIRES,90,   90,  0,      "newGameButton", "select",         -1,    0, { WORKAROUND_FAKE,   0 } }, // op_ge: MUMG Deluxe, when selecting "New Game" in the main menu. It tries to compare an integer with a list. Needs to return false for the game to continue.
	{ GID_PHANTASMAGORIA, 902,    0,  0,                   "", "export 7",       -1,    0, { WORKAROUND_FAKE,   0 } }, // op_shr: when starting a chapter in Phantasmagoria
	{ GID_QFG1VGA,        301,  928,  0,              "Blink", "init",           -1,    0, { WORKAROUND_FAKE,   0 } }, // op_div: when entering the inn, gets called with 1 parameter, but 2nd parameter is used for div which happens to be an object
	{ GID_QFG2,           200,  200,  0,              "astro", "messages",       -1,    0, { WORKAROUND_FAKE,   0 } }, // op_lsi: when getting asked for your name by the astrologer bug #3039879
	{ GID_QFG4,           710,64941,  0,          "RandCycle", "doit",           -1,    0, { WORKAROUND_FAKE,   1 } }, // op_gt: when the tentacle appears in the third room of the caves
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,  workaround
const SciWorkaroundEntry uninitializedReadWorkarounds[] = {
	{ GID_CASTLEBRAIN,   280,   280,  0,         "programmer", "dispatchEvent",  -1,    0, { WORKAROUND_FAKE, 0xf } }, // pressing 'q' on the computer screen in the robot room, and closing the help dialog that pops up (bug #3039656). Moves the cursor to the view with the ID returned (in this case, the robot hand)
	{ GID_CNICK_KQ,      200,     0,  1,          "Character", "say",            -1,   -1, { WORKAROUND_FAKE,   0 } }, // checkers, like in hoyle 3 - temps 504 and 505
	{ GID_CNICK_KQ,       -1,   700,  0,           "gcWindow", "open",           -1,   -1, { WORKAROUND_FAKE,   0 } }, // when entering control menu, like in hoyle 3
	{ GID_CNICK_LONGBOW,   0,     0,  0,          "RH Budget", "init",           -1,    1, { WORKAROUND_FAKE,   0 } }, // when starting the game
	{ GID_ECOQUEST,       -1,    -1,  0,                 NULL, "doVerb",         -1,    0, { WORKAROUND_FAKE,   0 } }, // almost clicking anywhere triggers this in almost all rooms
	{ GID_FANMADE,       516,   979,  0,                   "", "export 0",       -1,   20, { WORKAROUND_FAKE,   0 } }, // Happens in Grotesteing after the logos
	{ GID_FANMADE,       528,   990,  0,            "GDialog", "doit",           -1,    4, { WORKAROUND_FAKE,   0 } }, // Happens in Cascade Quest when closing the glossary - bug #3038757
	{ GID_FANMADE,       488,     1,  0,         "RoomScript", "doit",       0x1f17,    1, { WORKAROUND_FAKE,   0 } }, // Happens in Ocean Battle while playing - bug #3059871
	{ GID_FREDDYPHARKAS,  -1,    24,  0,              "gcWin", "open",           -1,    5, { WORKAROUND_FAKE, 0xf } }, // is used as priority for game menu
	{ GID_FREDDYPHARKAS,  -1,    31,  0,            "quitWin", "open",           -1,    5, { WORKAROUND_FAKE, 0xf } }, // is used as priority for game menu
	{ GID_FREDDYPHARKAS, 540,   540,  0,          "WaverCode", "init",           -1,   -1, { WORKAROUND_FAKE,   0 } }, // Gun pratice mini-game (bug #3044218)
	{ GID_GK1,            -1, 64950, -1,            "Feature", "handleEvent",    -1,    0, { WORKAROUND_FAKE,   0 } }, // sometimes when walk-clicking
	{ GID_GK2,            -1,    11,  0,                   "", "export 10",      -1,    3, { WORKAROUND_FAKE,   0 } }, // called when the game starts
	{ GID_GK2,            -1,    11,  0,                   "", "export 10",      -1,    4, { WORKAROUND_FAKE,   0 } }, // called during the game
	{ GID_HOYLE1,          4,   104,  0,   "GinRummyCardList", "calcRuns",       -1,    4, { WORKAROUND_FAKE,   0 } }, // Gin Rummy / right when the game starts
	{ GID_HOYLE1,          5,   204,  0,            "tableau", "checkRuns",      -1,    2, { WORKAROUND_FAKE,   0 } }, // Cribbage / during the game
	{ GID_HOYLE1,          3,    16,  0,                   "", "export 0",    0x37c,    3, { WORKAROUND_FAKE,   0 } }, // Hearts / during the game - bug #3052359
	{ GID_HOYLE1,         -1,   997,  0,            "MenuBar", "doit",           -1,    0, { WORKAROUND_FAKE,   0 } }, // When changing game speed settings - bug #3108012
	{ GID_HOYLE3,         -1,     0,  1,          "Character", "say",            -1,   -1, { WORKAROUND_FAKE,   0 } }, // when starting checkers or dominoes, first time a character says something - temps 504 and 505
	{ GID_HOYLE3,         -1,   700,  0,           "gcWindow", "open",           -1,   -1, { WORKAROUND_FAKE,   0 } }, // when entering control menu
	{ GID_HOYLE3,        100,   100,  0,        "dominoHand2", "cue",            -1,    1, { WORKAROUND_FAKE,   0 } }, // while playing domino - bug #3036918
	{ GID_HOYLE4,         -1,     0,  0,                 NULL, "open",           -1,   -1, { WORKAROUND_FAKE,   0 } }, // when selecting "Control" from the menu (temp vars 0-3) - bug #3039294
	{ GID_HOYLE4,        910,    18,  0,                 NULL, "init",           -1,    0, { WORKAROUND_FAKE,   0 } }, // during tutorial - bug #3042756
	{ GID_HOYLE4,        910,   910,  0,                 NULL, "setup",          -1,    3, { WORKAROUND_FAKE,   0 } }, // when selecting "Tutorial" from the main menu - bug #3039294
	{ GID_HOYLE4,        700,   718,  0,       "compete_tree", "doit",           -1,   75, { WORKAROUND_FAKE,   0 } }, // when placing a bid in bridge - bug #3292332
	{ GID_HOYLE4,        700,   716,  0,        "other1_tree", "doit",           -1,   46, { WORKAROUND_FAKE,   0 } }, // sometimes when placing a bid in bridge
	{ GID_HOYLE4,        700,   700,  1,         "BridgeHand", "calcQTS",        -1,    3, { WORKAROUND_FAKE,   0 } }, // sometimes when placing a bid in bridge
	{ GID_HOYLE4,        300,   300,  0,                   "", "export 2",   0x1d4d,    0, { WORKAROUND_FAKE,   0 } }, // after passing around cards in hearts
	{ GID_HOYLE4,        400,   400,  1,            "GinHand", "calcRuns",       -1,    4, { WORKAROUND_FAKE,   0 } }, // sometimes while playing Gin Rummy (e.g. when knocking and placing a card) - bug #3292334
	{ GID_HOYLE4,        500,    17,  1,          "Character", "say",            -1,  504, { WORKAROUND_FAKE,   0 } }, // sometimes while playing Cribbage (e.g. when the opponent says "Last Card") - bug #3292327
	{ GID_ISLANDBRAIN,   100,   937,  0,            "IconBar", "dispatchEvent",  -1,   58, { WORKAROUND_FAKE,   0 } }, // when using ENTER at the startup menu - bug #3045225
	{ GID_ISLANDBRAIN,   140,   140,  0,              "piece", "init",           -1,    3, { WORKAROUND_FAKE,   1 } }, // first puzzle right at the start, some initialization variable. bnt is done on it, and it should be non-0
	{ GID_ISLANDBRAIN,   200,   268,  0,          "anElement", "select",         -1,    0, { WORKAROUND_FAKE,   0 } }, // elements puzzle, gets used before super TextIcon
	{ GID_JONES,           1,   232,  0,        "weekendText", "draw",        0x3d3,    0, { WORKAROUND_FAKE,   0 } }, // jones/cd only - gets called during the game
	{ GID_JONES,           1,   255,  0,                   "", "export 0",       -1,   -1, { WORKAROUND_FAKE,   0 } }, // jones/cd only - called when a game ends, temps 13 and 14
	{ GID_JONES,         764,   255,  0,                   "", "export 0",       -1,   -1, { WORKAROUND_FAKE,   0 } }, // jones/ega&vga only - called when the game starts, temps 13 and 14
	//{ GID_KQ5,            -1,     0,  0,                   "", "export 29",      -1,    3, { WORKAROUND_FAKE,   0xf } }, // called when playing harp for the harpies or when aborting dialog in toy shop, is used for kDoAudio - bug #3034700
	// ^^ shouldn't be needed anymore, we got a script patch instead (kq5PatchCdHarpyVolume)
	{ GID_KQ5,            25,    25,  0,              "rm025", "doit",           -1,    0, { WORKAROUND_FAKE,   0 } }, // inside witch forest, when going to the room where the walking rock is
	{ GID_KQ5,            55,    55,  0,         "helpScript", "doit",           -1,    0, { WORKAROUND_FAKE,   0 } }, // when giving the tambourine to the monster in the labyrinth (only happens at one of the locations) - bug #3041262
	{ GID_KQ5,            -1,   755,  0,              "gcWin", "open",           -1,   -1, { WORKAROUND_FAKE,   0 } }, // when entering control menu in the FM-Towns version
	{ GID_KQ6,            -1,    30,  0,               "rats", "changeState",    -1,   -1, { WORKAROUND_FAKE,   0 } }, // rats in the catacombs (temps 1 - 5) - bugs #3034597, #3035495, #3035824
	{ GID_KQ6,           210,   210,  0,              "rm210", "scriptCheck",    -1,    0, { WORKAROUND_FAKE,   1 } }, // using inventory in that room - bug #3034565
	{ GID_KQ6,           500,   500,  0,              "rm500", "init",           -1,    0, { WORKAROUND_FAKE,   0 } }, // going to island of the beast
	{ GID_KQ6,           520,   520,  0,              "rm520", "init",           -1,    0, { WORKAROUND_FAKE,   0 } }, // going to boiling water trap on beast isle
	{ GID_KQ6,            -1,   903,  0,         "controlWin", "open",           -1,    4, { WORKAROUND_FAKE,   0 } }, // when opening the controls window (save, load etc)
	{ GID_KQ6,            -1,   907,  0,             "tomato", "doVerb",         -1,    2, { WORKAROUND_FAKE,   0 } }, // when looking at the rotten tomato in the inventory - bug #3059544
	{ GID_KQ7,            -1, 64996,  0,               "User", "handleEvent",    -1,    1, { WORKAROUND_FAKE,   0 } }, // called when pushing a keyboard key
	{ GID_LAURABOW,       37,     0,  0,                "CB1", "doit",           -1,    1, { WORKAROUND_FAKE,   0 } }, // when going up the stairs (bug #3037694)
	{ GID_LAURABOW,       -1,   967,  0,             "myIcon", "cycle",          -1,    1, { WORKAROUND_FAKE,   0 } }, // having any portrait conversation coming up (initial bug #3034985)
	{ GID_LAURABOW2,      -1,    24,  0,              "gcWin", "open",           -1,    5, { WORKAROUND_FAKE, 0xf } }, // is used as priority for game menu
	{ GID_LAURABOW2,      -1,    21,  0,      "dropCluesCode", "doit",           -1,    1, { WORKAROUND_FAKE, 0x7fff } }, // when asking some questions (e.g. the reporter about the burglary, or the policeman about Ziggy). Must be big, as the game scripts perform lt on it and start deleting journal entries - bugs #3035068, #3036274
	{ GID_LAURABOW2,      -1,    90,  1,        "MuseumActor", "init",           -1,    6, { WORKAROUND_FAKE,   0 } }, // Random actors in museum (bug #3041257)
	{ GID_LAURABOW2,     240,   240,  0,     "sSteveAnimates", "changeState",    -1,    0, { WORKAROUND_FAKE,   0 } }, // Steve Dorian's idle animation at the docks - bug #3036291
	{ GID_LONGBOW,        -1,     0,  0,            "Longbow", "restart",        -1,    0, { WORKAROUND_FAKE,   0 } }, // When canceling a restart game - bug #3046200
	{ GID_LONGBOW,        -1,   213,  0,              "clear", "handleEvent",    -1,    0, { WORKAROUND_FAKE,   0 } }, // When giving an answer using the druid hand sign code in any room
	{ GID_LONGBOW,        -1,   213,  0,             "letter", "handleEvent",  0xa8,    1, { WORKAROUND_FAKE,   0 } }, // When using the druid hand sign code in any room - bug #3036601
	{ GID_LSL1,          250,   250,  0,           "increase", "handleEvent",    -1,    2, { WORKAROUND_FAKE,   0 } }, // casino, playing game, increasing bet
	{ GID_LSL1,          720,   720,  0,              "rm720", "init",           -1,    0, { WORKAROUND_FAKE,   0 } }, // age check room
	{ GID_LSL2,           38,    38,  0,        "cloudScript", "changeState",    -1,    1, { WORKAROUND_FAKE,   0 } }, // entering the room in the middle deck of the ship - bug #3036483
	{ GID_LSL3,          340,   340,  0,        "ComicScript", "changeState",    -1,   -1, { WORKAROUND_FAKE,   0 } }, // right after entering the 3 ethnic groups inside comedy club (temps 200, 201, 202, 203)
	{ GID_LSL3,           -1,   997,  0,         "TheMenuBar", "handleEvent",    -1,    1, { WORKAROUND_FAKE, 0xf } }, // when setting volume the first time, this temp is used to set volume on entry (normally it would have been initialized to 's')
	{ GID_LSL6,          820,    82,  0,                   "", "export 0",       -1,   -1, { WORKAROUND_FAKE,   0 } }, // when touching the electric fence - bug #3038326
	{ GID_LSL6,           -1,    85,  0,          "washcloth", "doVerb",         -1,    0, { WORKAROUND_FAKE,   0 } }, // washcloth in inventory
	{ GID_LSL6,           -1,   928, -1,           "Narrator", "startText",      -1,    0, { WORKAROUND_FAKE,   0 } }, // used by various objects that are even translated in foreign versions, that's why we use the base-class
	{ GID_LSL6HIRES,       0,    85,  0,             "LL6Inv", "init",           -1,    0, { WORKAROUND_FAKE,   0 } }, // on startup
	{ GID_LSL6HIRES,      -1, 64950,  1,            "Feature", "handleEvent",    -1,    0, { WORKAROUND_FAKE,   0 } }, // at least when entering swimming pool area
	{ GID_LSL6HIRES,      -1, 64964,  0,              "DPath", "init",           -1,    1, { WORKAROUND_FAKE,   0 } }, // during the game
	{ GID_MOTHERGOOSE256, -1,     0,  0,                 "MG", "doit",           -1,    5, { WORKAROUND_FAKE,   0 } }, // SCI1.1: When moving the cursor all the way to the left during the game (bug #3043955)
	{ GID_MOTHERGOOSE256, -1,   992,  0,             "AIPath", "init",           -1,    0, { WORKAROUND_FAKE,   0 } }, // Happens in the demo and full version. In the demo, it happens when walking two screens from mother goose's house to the north. In the full version, it happens in rooms 7 and 23 - bug #3049146
	{ GID_MOTHERGOOSE256, 90,    90,  0,        "introScript", "changeState",    -1,   65, { WORKAROUND_FAKE,   0 } }, // SCI1(CD): At the very end, after the game is completed and restarted - bug #3268076
	{ GID_MOTHERGOOSE256, 94,    94,  0,            "sunrise", "changeState",    -1,  367, { WORKAROUND_FAKE,   0 } }, // At the very end, after the game is completed - bug #3051163
	{ GID_MOTHERGOOSEHIRES,-1,64950,  1,            "Feature", "handleEvent",    -1,    0, { WORKAROUND_FAKE,   0 } }, // right when clicking on a child at the start and probably also later
	{ GID_MOTHERGOOSEHIRES,-1,64950,  1,               "View", "handleEvent",    -1,    0, { WORKAROUND_FAKE,   0 } }, // see above
	{ GID_PEPPER,         -1,   894,  0,            "Package", "doVerb",         -1,    3, { WORKAROUND_FAKE,   0 } }, // using the hand on the book in the inventory - bug #3040012
	{ GID_PEPPER,        150,   928,  0,           "Narrator", "startText",      -1,    0, { WORKAROUND_FAKE,   0 } }, // happens during the non-interactive demo of Pepper
	{ GID_PQ4,            -1,    25,  0,         "iconToggle", "select",         -1,    1, { WORKAROUND_FAKE,   0 } }, // when toggling the icon bar to auto-hide or not
	{ GID_PQSWAT,         -1, 64950,  0,               "View", "handleEvent",    -1,    0, { WORKAROUND_FAKE,   0 } }, // Using the menu in the beginning
	{ GID_QFG1,           -1,   210,  0,          "Encounter", "init",        0xbd0,    0, { WORKAROUND_FAKE,   0 } }, // hq1: going to the brigands hideout
	{ GID_QFG1,           -1,   210,  0,          "Encounter", "init",        0xbe4,    0, { WORKAROUND_FAKE,   0 } }, // qfg1: going to the brigands hideout
	{ GID_QFG1VGA,        16,    16,  0,        "lassoFailed", "changeState",    -1,   -1, { WORKAROUND_FAKE,   0 } }, // qfg1vga: casting the "fetch" spell in the screen with the flowers, temps 0 and 1 - bug #3053268
	{ GID_QFG1VGA,        -1,   210,  0,          "Encounter", "init",        0xcee,    0, { WORKAROUND_FAKE,   0 } }, // qfg1vga: going to the brigands hideout - bug #3109299
	{ GID_QFG1VGA,        -1,   210,  0,          "Encounter", "init",        0xce7,    0, { WORKAROUND_FAKE,   0 } }, // qfg1vga: going to room 92
	{ GID_QFG2,           -1,    71,  0,        "theInvSheet", "doit",           -1,    1, { WORKAROUND_FAKE,   0 } }, // accessing the inventory
	{ GID_QFG2,           -1,   701, -1,              "Alley", "at",             -1,    0, { WORKAROUND_FAKE,   0 } }, // when walking inside the alleys in the town - bug #3035835 & #3038367
	{ GID_QFG2,           -1,   990,  0,            "Restore", "doit",           -1,  364, { WORKAROUND_FAKE,   0 } }, // when pressing enter in restore dialog w/o any saved games present
	{ GID_QFG2,          260,   260,  0,             "abdulS", "changeState",0x2d22,   -1, { WORKAROUND_FAKE,   0 } }, // During the thief's first mission (in the house), just before Abdul is about to enter the house (where you have to hide in the wardrobe), bug #3039891, temps 1 and 2
	{ GID_QFG2,          260,   260,  0,            "jabbarS", "changeState",0x2d22,   -1, { WORKAROUND_FAKE,   0 } }, // During the thief's first mission (in the house), just before Jabbar is about to enter the house (where you have to hide in the wardrobe), bug #3040469, temps 1 and 2
	{ GID_QFG2,          500,   500,  0,   "lightNextCandleS", "changeState",    -1,   -1, { WORKAROUND_FAKE,   0 } }, // Inside the last room, while Ad Avis performs the ritual to summon the genie - bug #3148418
	{ GID_QFG2,           -1,   700,  0,                 NULL, "showSign",       -1,   10, { WORKAROUND_FAKE,   0 } }, // Occurs sometimes when reading a sign in Raseir, Shapeir et al - bugs #3272735, #3275413
	{ GID_QFG3,          510,   510,  0,         "awardPrize", "changeState",    -1,    0, { WORKAROUND_FAKE,   0 } }, // Simbani warrior challenge, after throwing the spears and retrieving the ring - bug #3049435
	{ GID_QFG3,          140,   140,  0,              "rm140", "init",       0x1008,    0, { WORKAROUND_FAKE,   0 } }, // when importing a character and selecting the previous profession - bug #3040460
	{ GID_QFG3,          330,   330, -1,             "Teller", "doChild",        -1,   -1, { WORKAROUND_FAKE,   0 } }, // when talking to King Rajah about "Rajah" (bug #3036390, temp 1) or "Tarna" (temp 0), or when clicking on yourself and saying "Greet" (bug #3039774, temp 1)
	{ GID_QFG3,          700,   700, -1,      "monsterIsDead", "changeState",    -1,    0, { WORKAROUND_FAKE,   0 } }, // in the jungle, after winning any fight, bug #3040624
	{ GID_QFG3,          470,   470, -1,              "rm470", "notify",         -1,    0, { WORKAROUND_FAKE,   0 } }, // closing the character screen in the Simbani village in the room with the bridge, bug #3040565
	{ GID_QFG3,          490,   490, -1,      "computersMove", "changeState",    -1,    0, { WORKAROUND_FAKE,   0 } }, // when finishing awari game, bug #3040579
	{ GID_QFG3,          490,   490, -1,      "computersMove", "changeState", 0xf53,    4, { WORKAROUND_FAKE,   0 } }, // also when finishing awari game
	{ GID_QFG3,          851,    32, -1,            "ProjObj", "doit",           -1,    1, { WORKAROUND_FAKE,   0 } }, // near the end, when throwing the spear of death, bug #3050122
	{ GID_QFG4,           -1,    15, -1,     "charInitScreen", "dispatchEvent",  -1,    5, { WORKAROUND_FAKE,   0 } }, // floppy version, when viewing the character screen
	{ GID_QFG4,           -1, 64917, -1,       "controlPlane", "setBitmap",      -1,    3, { WORKAROUND_FAKE,   0 } }, // floppy version, when entering the game menu
	{ GID_QFG4,           -1, 64917, -1,              "Plane", "setBitmap",      -1,    3, { WORKAROUND_FAKE,   0 } }, // floppy version, happens sometimes in fight scenes
	{ GID_RAMA,           12, 64950, -1,   "InterfaceFeature", "handleEvent",    -1,    0, { WORKAROUND_FAKE,   0 } }, // Demo, right when it starts
	{ GID_RAMA,           12, 64950, -1,      "hiliteOptText", "handleEvent",    -1,    0, { WORKAROUND_FAKE,   0 } }, // Demo, right when it starts
	{ GID_RAMA,           12, 64950, -1,               "View", "handleEvent",    -1,    0, { WORKAROUND_FAKE,   0 } }, // Demo, right when it starts
	{ GID_SHIVERS,        -1,   952,  0,       "SoundManager", "stop",           -1,    2, { WORKAROUND_FAKE,   0 } }, // Just after Sierra logo
	{ GID_SHIVERS,        -1, 64950,  0,            "Feature", "handleEvent",    -1,    0, { WORKAROUND_FAKE,   0 } }, // When clicking on the locked door at the beginning
	{ GID_SHIVERS,        -1, 64950,  0,               "View", "handleEvent",    -1,    0, { WORKAROUND_FAKE,   0 } }, // When clicking on the gargoyle eye at the beginning
	{ GID_SHIVERS,     20311, 64964,  0,              "DPath", "init",           -1,    1, { WORKAROUND_FAKE,   0 } }, // Just after door puzzle is solved and the metal balls start to roll
	{ GID_SHIVERS,     29260, 29260,  0,             "spMars", "handleEvent",    -1,    4, { WORKAROUND_FAKE,   0 } }, // When clicking mars after seeing fortune to align earth etc...
	{ GID_SHIVERS,     29260, 29260,  0,            "spVenus", "handleEvent",    -1,    4, { WORKAROUND_FAKE,   0 } }, // When clicking venus after seeing fortune to align earth etc...
	{ GID_SQ1,           103,   103,  0,               "hand", "internalEvent",  -1,   -1, { WORKAROUND_FAKE,   0 } }, // Spanish (and maybe early versions?) only: when moving cursor over input pad, temps 1 and 2
	{ GID_SQ1,            -1,   703,  0,                   "", "export 1",       -1,    0, { WORKAROUND_FAKE,   0 } }, // sub that's called from several objects while on sarien battle cruiser
	{ GID_SQ1,            -1,   703,  0,         "firePulsar", "changeState", 0x18a,    0, { WORKAROUND_FAKE,   0 } }, // export 1, but called locally (when shooting at aliens)
	{ GID_SQ4,            -1,   398,  0,            "showBox", "changeState",    -1,    0, { WORKAROUND_FAKE,   0 } }, // CD: called when rummaging in Software Excess bargain bin
	{ GID_SQ4,            -1,   928, -1,           "Narrator", "startText",      -1, 1000, { WORKAROUND_FAKE,   1 } }, // CD: happens in the options dialog and in-game when speech and subtitles are used simultaneously
	{ GID_SQ5,           201,   201,  0,        "buttonPanel", "doVerb",         -1,    0, { WORKAROUND_FAKE,   1 } }, // when looking at the orange or red button - bug #3038563
	{ GID_SQ6,            -1,     0,  0,                "SQ6", "init",           -1,    2, { WORKAROUND_FAKE,   0 } }, // Demo and full version: called when the game starts (demo: room 0, full: room 100)
	{ GID_SQ6,            -1, 64950, -1,            "Feature", "handleEvent",    -1,    0, { WORKAROUND_FAKE,   0 } }, // called when pressing "Start game" in the main menu, when entering the Orion's Belt bar (room 300), and perhaps other places
	{ GID_SQ6,            -1, 64964,  0,              "DPath", "init",           -1,    1, { WORKAROUND_FAKE,   0 } }, // during the game
	{ GID_TORIN,          -1, 64017,  0,             "oFlags", "clear",          -1,    0, { WORKAROUND_FAKE,   0 } }, // entering Torin's home in the French version
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kAbs_workarounds[] = {
	{ GID_HOYLE1,          1,     1,  0,              "room1", "doit",           -1,    0, { WORKAROUND_FAKE,  0x3e9 } }, // crazy eights - called with objects instead of integers
	{ GID_HOYLE1,          2,     2,  0,              "room2", "doit",           -1,    0, { WORKAROUND_FAKE,  0x3e9 } }, // old maid - called with objects instead of integers
	{ GID_HOYLE1,          3,     3,  0,              "room3", "doit",           -1,    0, { WORKAROUND_FAKE,  0x3e9 } }, // hearts - called with objects instead of integers
	{ GID_QFG1VGA,        -1,    -1,  0,                 NULL, "doit",           -1,    0, { WORKAROUND_FAKE,  0x3e9 } }, // when the game is patched with the NRS patch
	{ GID_QFG3   ,        -1,    -1,  0,                 NULL, "doit",           -1,    0, { WORKAROUND_FAKE,  0x3e9 } }, // when the game is patched with the NRS patch (bugs #3528416, #3528542)
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kCelHigh_workarounds[] = {
	{ GID_KQ5,            -1,   255,  0,          "deathIcon", "setSize",        -1,    0, { WORKAROUND_STILLCALL, 0 } }, // english floppy: when getting beaten up in the inn and probably more, called with 2nd parameter as object - bug #3037003
	{ GID_PQ2,            -1,   255,  0,              "DIcon", "setSize",        -1,    0, { WORKAROUND_STILLCALL, 0 } }, // when showing picture within windows, called with 2nd/3rd parameters as objects
	{ GID_SQ1,             1,   255,  0,              "DIcon", "setSize",        -1,    0, { WORKAROUND_STILLCALL, 0 } }, // DEMO: Called with 2nd/3rd parameters as objects when clicking on the menu - bug #3035720
	{ GID_FANMADE,        -1,   979,  0,              "DIcon", "setSize",        -1,    0, { WORKAROUND_STILLCALL, 0 } }, // In The Gem Scenario and perhaps other fanmade games, this is called with 2nd/3rd parameters as objects - bug #3039679
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kCelWide_workarounds[] = {
	{ GID_KQ5,            -1,   255,  0,          "deathIcon", "setSize",        -1,    0, { WORKAROUND_STILLCALL, 0 } }, // english floppy: when getting beaten up in the inn and probably more, called with 2nd parameter as object - bug #3037003
	{ GID_PQ2,            -1,   255,  0,              "DIcon", "setSize",        -1,    0, { WORKAROUND_STILLCALL, 0 } }, // when showing picture within windows, called with 2nd/3rd parameters as objects
	{ GID_SQ1,             1,   255,  0,              "DIcon", "setSize",        -1,    0, { WORKAROUND_STILLCALL, 0 } }, // DEMO: Called with 2nd/3rd parameters as objects when clicking on the menu - bug #3035720
	{ GID_FANMADE,        -1,   979,  0,              "DIcon", "setSize",        -1,    0, { WORKAROUND_STILLCALL, 0 } }, // In The Gem Scenario and perhaps other fanmade games, this is called with 2nd/3rd parameters as objects - bug #3039679
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kDeviceInfo_workarounds[] = {
	{ GID_FANMADE,        -1,   994,  1,               "Game", "save",        0xd1c,    0, { WORKAROUND_STILLCALL, 0 } }, // In fanmade games, this is called with one parameter for CurDevice (Cascade Quest)
	{ GID_FANMADE,        -1,   994,  1,               "Game", "save",        0xe55,    0, { WORKAROUND_STILLCALL, 0 } }, // In fanmade games, this is called with one parameter for CurDevice (Demo Quest)
	{ GID_FANMADE,        -1,   994,  1,               "Game", "save",        0xe57,    0, { WORKAROUND_STILLCALL, 0 } }, // In fanmade games, this is called with one parameter for CurDevice (I Want My C64 Back)
	{ GID_FANMADE,        -1,   994,  1,               "Game", "save",        0xe5c,    0, { WORKAROUND_STILLCALL, 0 } }, // In fanmade games, this is called with one parameter for CurDevice (Most of them)
	{ GID_FANMADE,        -1,   994,  1,               "Game", "restore",     0xd1c,    0, { WORKAROUND_STILLCALL, 0 } }, // In fanmade games, this is called with one parameter for CurDevice (Cascade Quest)
	{ GID_FANMADE,        -1,   994,  1,               "Game", "restore",     0xe55,    0, { WORKAROUND_STILLCALL, 0 } }, // In fanmade games, this is called with one parameter for CurDevice (Demo Quest)
	{ GID_FANMADE,        -1,   994,  1,               "Game", "restore",     0xe57,    0, { WORKAROUND_STILLCALL, 0 } }, // In fanmade games, this is called with one parameter for CurDevice (I Want My C64 Back)
	{ GID_FANMADE,        -1,   994,  1,               "Game", "restore",     0xe5c,    0, { WORKAROUND_STILLCALL, 0 } }, // In fanmade games, this is called with one parameter for CurDevice (Most of them)
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kDisplay_workarounds[] = {
	{ GID_ISLANDBRAIN,   300,   300,  0,           "geneDude", "show",           -1,    0, { WORKAROUND_IGNORE,    0 } }, // when looking at the gene explanation chart - a parameter is an object
	{ GID_PQ2,            23,    23,  0,         "rm23Script", "elements",    0x4ae,    0, { WORKAROUND_IGNORE,    0 } }, // when looking at the 2nd page of pate's file - 0x75 as id
	{ GID_PQ2,            23,    23,  0,         "rm23Script", "elements",    0x4c1,    0, { WORKAROUND_IGNORE,    0 } }, // when looking at the 2nd page of pate's file - 0x75 as id (another pq2 version, bug #3043904)
	{ GID_QFG1,           11,    11,  0,             "battle", "<noname90>",     -1,    0, { WORKAROUND_IGNORE,    0 } }, // DEMO: When entering battle, 0x75 as id
	{ GID_SQ4,           397,     0,  0,                   "", "export 12",      -1,    0, { WORKAROUND_IGNORE,    0 } }, // FLOPPY: when going into the computer store (bug #3044044)
	{ GID_SQ4,           391,   391,  0,          "doCatalog", "mode",         0x84,    0, { WORKAROUND_IGNORE,    0 } }, // CD: clicking on catalog in roboter sale - a parameter is an object
	{ GID_SQ4,           391,   391,  0,         "choosePlug", "changeState",    -1,    0, { WORKAROUND_IGNORE,    0 } }, // CD: ordering connector in roboter sale - a parameter is an object
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kDirLoop_workarounds[] = {
	{ GID_KQ4,             4,   992,  0,              "Avoid", "doit",           -1,    0, { WORKAROUND_IGNORE,    0 } }, // when the ogre catches you in front of his house, second parameter points to the same object as the first parameter, instead of being an integer (the angle) - bug #3042964
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kDisposeScript_workarounds[] = {
	{ GID_LAURABOW,      777,   777,  0,             "myStab", "changeState",    -1,    0, { WORKAROUND_IGNORE,    0 } }, // DEMO: after the will is signed, parameter 0 is an object - bug #3034907
	{ GID_QFG1,           -1,    64,  0,               "rm64", "dispose",        -1,    0, { WORKAROUND_IGNORE,    0 } }, // when leaving graveyard, parameter 0 is an object
	{ GID_SQ4,           150,   151,  0,        "fightScript", "dispose",        -1,    0, { WORKAROUND_IGNORE,    0 } }, // during fight with Vohaul, parameter 0 is an object
	{ GID_SQ4,           150,   152,  0,       "driveCloseUp", "dispose",        -1,    0, { WORKAROUND_IGNORE,    0 } }, // when choosing "beam download", parameter 0 is an object
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kDoSoundFade_workarounds[] = {
	{ GID_KQ5,           213,   989,  0,       "globalSound3", "fade",           -1,    0, { WORKAROUND_STILLCALL, 0 } }, // english floppy: when bandits leave the secret temple, parameter 4 is an object - bug #3037594
	{ GID_KQ6,           105,   989,  0,        "globalSound", "fade",           -1,    0, { WORKAROUND_STILLCALL, 0 } }, // floppy: during intro, parameter 4 is an object
	{ GID_KQ6,           460,   989,  0,       "globalSound2", "fade",           -1,    0, { WORKAROUND_STILLCALL, 0 } }, // after pulling the black widow's web on the isle of wonder, parameter 4 is an object - bug #3034567
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kGetAngle_workarounds[] = {
	{ GID_FANMADE,       516,   992,  0,             "Motion", "init",           -1,    0, { WORKAROUND_FAKE,      0 } }, // The Legend of the Lost Jewel Demo (fan made): called with third/fourth parameters as objects
	{ GID_KQ6,            -1,   752,  0,        "throwDazzle", "changeState",    -1,    0, { WORKAROUND_STILLCALL, 0 } }, // room 740/790 after the Genie is exposed in the Palace (short and long ending), it starts shooting lightning bolts around. An extra 5th parameter is passed - bug #3034610 & #3041734
	{ GID_SQ1,            -1,   927,  0,           "PAvoider", "doit",           -1,    0, { WORKAROUND_FAKE,      0 } }, // all rooms in Ulence Flats after getting the Pilot Droid: called with a single parameter when the droid is in Roger's path - bug #3513207
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kFindKey_workarounds[] = {
	{ GID_ECOQUEST2,     100,   999,  0,            "myList", "contains",        -1,    0, { WORKAROUND_FAKE, 0 } }, // When Noah Greene gives Adam the Ecorder, and just before the game gives a demonstration, a null reference to a list is passed - bug #3035186
	{    GID_HOYLE4,     300,   999,  0,             "Piles", "contains",        -1,    0, { WORKAROUND_FAKE, 0 } }, // When passing the three cards in Hearts, a null reference to a list is passed - bug #3292333
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kGraphDrawLine_workarounds[] = {
	{ GID_ISLANDBRAIN,   300,   300,  0,         "dudeViewer", "show",           -1,    0, { WORKAROUND_STILLCALL, 0 } }, // when looking at the gene explanation chart, gets called with 1 extra parameter
	{ GID_SQ1,            43,    43,  0,        "someoneDied", "changeState",    -1,    0, { WORKAROUND_STILLCALL, 0 } }, // when ordering beer, gets called with 1 extra parameter
	{ GID_SQ1,            71,    71,  0,       "destroyXenon", "changeState",    -1,    0, { WORKAROUND_STILLCALL, 0 } }, // during the Xenon destruction cutscene (which results in death), gets called with 1 extra parameter - bug #3040894
	{ GID_SQ1,            53,    53,  0,           "blastEgo", "changeState",    -1,    0, { WORKAROUND_STILLCALL, 0 } }, // when Roger is found and zapped by the cleaning robot, gets called with 1 extra parameter - bug #3040905
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kGraphSaveBox_workarounds[] = {
	{ GID_CASTLEBRAIN,   420,   427,  0,          "alienIcon", "select",         -1,    0, { WORKAROUND_STILLCALL, 0 } }, // when selecting a card during the alien card game, gets called with 1 extra parameter
	{ GID_ISLANDBRAIN,   290,   291,  0,         "upElevator", "changeState",0x201f,    0, { WORKAROUND_STILLCALL, 0 } }, // when testing in the elevator puzzle, gets called with 1 argument less - 15 is on stack - bug #3034485
	{ GID_ISLANDBRAIN,   290,   291,  0,       "downElevator", "changeState",0x201f,    0, { WORKAROUND_STILLCALL, 0 } }, // see above
	{ GID_ISLANDBRAIN,   290,   291,  0,    "correctElevator", "changeState",0x201f,    0, { WORKAROUND_STILLCALL, 0 } }, // see above (when testing the correct solution)
	{ GID_PQ3,           202,   202,  0,            "MapEdit", "movePt",         -1,    0, { WORKAROUND_STILLCALL, 0 } }, // when plotting crimes, gets called with 2 extra parameters - bug #3038077
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kGraphRestoreBox_workarounds[] = {
	{ GID_LSL6,           -1,    86,  0,             "LL6Inv", "hide",           -1,    0, { WORKAROUND_STILLCALL, 0 } }, // happens during the game, gets called with 1 extra parameter
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kGraphFillBoxForeground_workarounds[] = {
	{ GID_LSL6,           -1,     0,  0,               "LSL6", "hideControls",   -1,    0, { WORKAROUND_STILLCALL, 0 } }, // happens when giving the bungee key to merrily (room 240) and at least in room 650 too - gets called with additional 5th parameter
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kGraphFillBoxAny_workarounds[] = {
	{ GID_ECOQUEST2,     100,   333,  0,        "showEcorder", "changeState",    -1,    0, { WORKAROUND_STILLCALL, 0 } }, // necessary workaround for our ecorder script patch, because there isn't enough space to patch the function
	{ GID_SQ4,            -1,   818,  0,     "iconTextSwitch", "show",           -1,    0, { WORKAROUND_STILLCALL, 0 } }, // CD: game menu "text/speech" display - parameter 5 is missing, but the right color number is on the stack
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kGraphRedrawBox_workarounds[] = {
	{ GID_SQ4,           405,   405,  0,       "swimAfterEgo", "changeState",    -1,    0, { WORKAROUND_STILLCALL, 0 } }, // skateOrama when "swimming" in the air - accidental additional parameter specified
	{ GID_SQ4,           406,   406,  0,        "egoFollowed", "changeState",    -1,    0, { WORKAROUND_STILLCALL, 0 } }, // FLOPPY: when getting shot by the police - accidental additional parameter specified
	{ GID_SQ4,           406,   406,  0,       "swimAndShoot", "changeState",    -1,    0, { WORKAROUND_STILLCALL, 0 } }, // skateOrama when "swimming" in the air - accidental additional parameter specified
	{ GID_SQ4,           410,   410,  0,       "swimAfterEgo", "changeState",    -1,    0, { WORKAROUND_STILLCALL, 0 } }, // skateOrama when "swimming" in the air - accidental additional parameter specified
	{ GID_SQ4,           411,   411,  0,       "swimAndShoot", "changeState",    -1,    0, { WORKAROUND_STILLCALL, 0 } }, // skateOrama when "swimming" in the air - accidental additional parameter specified
	{ GID_SQ4,           150,   150,  0,        "laserScript", "changeState",  0xb2,    0, { WORKAROUND_STILLCALL, 0 } }, // when visiting the pedestral where Roger Jr. is trapped, before trashing the brain icon in the programming chapter, accidental additional parameter specified - bug #3094235
	{ GID_SQ4,           150,   150,  0,        "laserScript", "changeState",  0x16,    0, { WORKAROUND_STILLCALL, 0 } }, // same as above, for the German version - bug #3116892
	{ GID_SQ4,            -1,   704,  0,           "shootEgo", "changeState",    -1,    0, { WORKAROUND_STILLCALL, 0 } }, // When shot by Droid in Super Computer Maze (Rooms 500, 505, 510...) - accidental additional parameter specified
	{ GID_KQ5,            -1,   981,  0,           "myWindow",     "dispose",    -1,    0, { WORKAROUND_STILLCALL, 0 } }, // Happens in the floppy version, when closing any dialog box, accidental additional parameter specified - bug #3036331
	{ GID_KQ5,            -1,   995,  0,               "invW",        "doit",    -1,    0, { WORKAROUND_STILLCALL, 0 } }, // Happens in the floppy version, when closing the inventory window, accidental additional parameter specified
	{ GID_KQ5,            -1,   995,  0,                   "",    "export 0",    -1,    0, { WORKAROUND_STILLCALL, 0 } }, // Happens in the floppy version, when opening the gem pouch, accidental additional parameter specified - bug #3039395
	{ GID_KQ5,            -1,   403,  0,          "KQ5Window",     "dispose",    -1,    0, { WORKAROUND_STILLCALL, 0 } }, // Happens in the FM Towns version when closing any dialog box, accidental additional parameter specified
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kGraphUpdateBox_workarounds[] = {
	{ GID_ECOQUEST2,     100,   333,  0,        "showEcorder", "changeState",    -1,    0, { WORKAROUND_STILLCALL, 0 } }, // necessary workaround for our ecorder script patch, because there isn't enough space to patch the function
	{ GID_PQ3,           202,   202,  0,            "MapEdit", "addPt",          -1,    0, { WORKAROUND_STILLCALL, 0 } }, // when plotting crimes, gets called with 2 extra parameters - bug #3038077
	{ GID_PQ3,           202,   202,  0,            "MapEdit", "movePt",         -1,    0, { WORKAROUND_STILLCALL, 0 } }, // when plotting crimes, gets called with 2 extra parameters - bug #3038077
	{ GID_PQ3,           202,   202,  0,            "MapEdit", "dispose",        -1,    0, { WORKAROUND_STILLCALL, 0 } }, // when plotting crimes, gets called with 2 extra parameters
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kIsObject_workarounds[] = {
	{ GID_GK1,           50,   999,  0,                "List", "eachElementDo",  -1,    0, { WORKAROUND_FAKE, 0 } }, // GK1 demo, when asking Grace for messages it gets called with an invalid parameter (type "error") - bug #3034519
	{ GID_ISLANDBRAIN,   -1,   999,  0,                "List", "eachElementDo",  -1,    0, { WORKAROUND_FAKE, 0 } }, // when going to the game options, choosing "Info" and selecting anything from the list, gets called with an invalid parameter (type "error") - bug #3035262
	{ GID_QFG3,          -1,   999,  0,                "List", "eachElementDo",  -1,    0, { WORKAROUND_FAKE, 0 } }, // when asking for something, gets called with type error parameter
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kMemory_workarounds[] = {
	{ GID_LAURABOW2,      -1,   999,  0,                   "", "export 6",       -1,    0, { WORKAROUND_FAKE,    0 } }, // during the intro, when exiting the train (room 160), talking to Mr. Augustini, etc. - bug #3034490
	{ GID_SQ1,            -1,   999,  0,                   "", "export 6",       -1,    0, { WORKAROUND_FAKE,    0 } }, // during walking Roger around Ulence Flats - bug #3513765
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kMoveCursor_workarounds[] = {
	{ GID_KQ5,            -1,   937,  0,            "IconBar", "handleEvent",    -1,    0, { WORKAROUND_IGNORE,  0 } }, // when pressing escape to open the menu, gets called with one parameter instead of 2 - bug #3156472
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kNewWindow_workarounds[] = {
	{ GID_ECOQUEST,       -1,   981,  0,          "SysWindow", "open",           -1,    0, { WORKAROUND_STILLCALL, 0 } }, // EcoQuest 1 demo uses an in-between interpreter from SCI1 to SCI1.1. It's SCI1.1, but uses the SCI1 semantics for this call - bug #3035057
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kPaletteUnsetFlag_workarounds[] = {
	{ GID_QFG4,          100,   100,  0,            "doMovie", "changeState",    -1,    0, { WORKAROUND_IGNORE,    0 } }, // after the Sierra logo, no flags are passed, thus the call is meaningless - bug #3034506
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kSetCursor_workarounds[] = {
	{ GID_KQ5,            -1,   768,  0,           "KQCursor", "init",           -1,    0, { WORKAROUND_STILLCALL, 0 } }, // CD: gets called with 4 additional "900d" parameters
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kSetPort_workarounds[] = {
	{ GID_LSL6,          740,   740,  0,              "rm740", "drawPic",        -1,    0, { WORKAROUND_IGNORE,    0 } }, // ending scene, is called with additional 3 (!) parameters
	{ GID_QFG3,          830,   830,  0,        "portalOpens", "changeState",    -1,    0, { WORKAROUND_IGNORE,    0 } }, // when the portal appears during the end, gets called with 4 parameters (bug #3040844)
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kStrAt_workarounds[] = {
	{ GID_CASTLEBRAIN,   220,   220,  0,         "robotJokes", "animateOnce",    -1,    0, { WORKAROUND_FAKE,      0 } }, // when trying to view the terminal at the end of the maze without having collected any robot jokes - bug #3039036
	{ GID_ISLANDBRAIN,   300,   310,  0,         "childBreed", "changeState",0x1c7c,    0, { WORKAROUND_FAKE,      0 } }, // when clicking Breed to get the second-generation cyborg hybrid (Standard difficulty), the two parameters are swapped - bug #3037835
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kStrLen_workarounds[] = {
	{ GID_QFG2,          210,     2,  0,                   "", "export 21",   0xdeb,    0, { WORKAROUND_FAKE,      0 } }, // When saying something incorrect at the WIT, an integer is passed instead of a reference - bug #3100292
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call,index,                workaround
const SciWorkaroundEntry kUnLoad_workarounds[] = {
	{ GID_ECOQUEST,      380,    61,  0,              "gotIt", "changeState",    -1,    0, { WORKAROUND_IGNORE, 0 } }, // CD version: after talking to the dolphin the first time, a 3rd parameter is passed by accident
	{ GID_ECOQUEST,      380,    69,  0,   "lookAtBlackBoard", "changeState",    -1,    0, { WORKAROUND_IGNORE, 0 } }, // German version, when closing the blackboard closeup in the dolphin room, a 3rd parameter is passed by accident - bug #3098353
	{ GID_LAURABOW2,      -1,    -1,  0,           "sCartoon", "changeState",    -1,    0, { WORKAROUND_IGNORE, 0 } }, // DEMO: during the intro, a 3rd parameter is passed by accident - bug #3034902
	{ GID_LSL6,          130,   130,  0,    "recruitLarryScr", "changeState",    -1,    0, { WORKAROUND_IGNORE, 0 } }, // during intro, a 3rd parameter is passed by accident
	{ GID_LSL6,          740,   740,  0,        "showCartoon", "changeState",    -1,    0, { WORKAROUND_IGNORE, 0 } }, // during ending, 4 additional parameters are passed by accident
	{ GID_LSL6HIRES,     130,   130,  0,    "recruitLarryScr", "changeState",    -1,    0, { WORKAROUND_IGNORE, 0 } }, // during intro, a 3rd parameter is passed by accident
	{ GID_SQ1,            43,   303,  0,            "slotGuy", "dispose",        -1,    0, { WORKAROUND_IGNORE, 0 } }, // when leaving ulence flats bar, parameter 1 is not passed - script error
	{ GID_QFG4,           -1,   110,  0,            "dreamer", "dispose",        -1,    0, { WORKAROUND_IGNORE, 0 } }, // during the dream sequence, a 3rd parameter is passed by accident
	SCI_WORKAROUNDENTRY_TERMINATOR
};

SciWorkaroundSolution trackOriginAndFindWorkaround(int index, const SciWorkaroundEntry *workaroundList, SciTrackOriginReply *trackOrigin) {
	// HACK for SCI3: Temporarily ignore this
	if (getSciVersion() == SCI_VERSION_3) {
		warning("SCI3 HACK: trackOriginAndFindWorkaround() called, ignoring");
		SciWorkaroundSolution sci3IgnoreForNow;
		sci3IgnoreForNow.type = WORKAROUND_FAKE;
		sci3IgnoreForNow.value = 0;
		return sci3IgnoreForNow;
	}

	const EngineState *state = g_sci->getEngineState();
	ExecStack *lastCall = state->xs;
	const Script *localScript = state->_segMan->getScriptIfLoaded(lastCall->local_segment);
	int curScriptNr = localScript->getScriptNumber();

	if (lastCall->debugLocalCallOffset != -1) {
		// if lastcall was actually a local call search back for a real call
		Common::List<ExecStack>::const_iterator callIterator = state->_executionStack.end();
		while (callIterator != state->_executionStack.begin()) {
			callIterator--;
			ExecStack loopCall = *callIterator;
			if ((loopCall.debugSelector != -1) || (loopCall.debugExportId != -1)) {
				lastCall->debugSelector = loopCall.debugSelector;
				lastCall->debugExportId = loopCall.debugExportId;
				break;
			}
		}
	}

	Common::String curObjectName = state->_segMan->getObjectName(lastCall->sendp);
	Common::String curMethodName;
	const SciGameId gameId = g_sci->getGameId();
	const int curRoomNumber = state->currentRoomNumber();

	if (lastCall->type == EXEC_STACK_TYPE_CALL) {
		if (lastCall->debugSelector != -1) {
			curMethodName = g_sci->getKernel()->getSelectorName(lastCall->debugSelector);
		} else if (lastCall->debugExportId != -1) {
			curObjectName = "";
			curMethodName = Common::String::format("export %d", lastCall->debugExportId);
		}
	}

	if (workaroundList) {
		// Search if there is a workaround for this one
		const SciWorkaroundEntry *workaround;
		int16 inheritanceLevel = 0;
		Common::String searchObjectName = curObjectName;
		reg_t searchObject = lastCall->sendp;
		do {
			workaround = workaroundList;
			while (workaround->methodName) {
				bool objectNameMatches = (workaround->objectName == NULL) ||
										 (workaround->objectName == g_sci->getSciLanguageString(searchObjectName, K_LANG_ENGLISH));

				// Special case: in the fanmade Russian translation of SQ4, all
				// of the object names have been deleted or renamed to Russian,
				// thus we disable checking of the object name. Fixes bug #3155550.
				if (g_sci->getLanguage() == Common::RU_RUS && g_sci->getGameId() == GID_SQ4)
					objectNameMatches = true;

				if (workaround->gameId == gameId
						&& ((workaround->scriptNr == -1) || (workaround->scriptNr == curScriptNr))
						&& ((workaround->roomNr == -1) || (workaround->roomNr == curRoomNumber))
						&& ((workaround->inheritanceLevel == -1) || (workaround->inheritanceLevel == inheritanceLevel))
						&& objectNameMatches
						&& workaround->methodName == g_sci->getSciLanguageString(curMethodName, K_LANG_ENGLISH)
						&& workaround->localCallOffset == lastCall->debugLocalCallOffset
						&& ((workaround->index == -1) || (workaround->index == index))) {
					// Workaround found
					return workaround->newValue;
				}
				workaround++;
			}

			// Go back to the parent
			inheritanceLevel++;
			searchObject = state->_segMan->getObject(searchObject)->getSuperClassSelector();
			if (!searchObject.isNull())
				searchObjectName = state->_segMan->getObjectName(searchObject);
		} while (!searchObject.isNull()); // no parent left?
	}

	// give caller origin data
	trackOrigin->objectName = curObjectName;
	trackOrigin->methodName = curMethodName;
	trackOrigin->scriptNr = curScriptNr;
	trackOrigin->localCallOffset = lastCall->debugLocalCallOffset;

	SciWorkaroundSolution noneFound;
	noneFound.type = WORKAROUND_NONE;
	noneFound.value = 0;
	return noneFound;
}

} // End of namespace Sci
