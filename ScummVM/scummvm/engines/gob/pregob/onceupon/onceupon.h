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

#ifndef GOB_PREGOB_ONCEUPON_ONCEUPON_H
#define GOB_PREGOB_ONCEUPON_ONCEUPON_H

#include "common/system.h"
#include "common/str.h"

#include "gob/pregob/pregob.h"

#include "gob/pregob/onceupon/stork.h"

namespace Gob {

class Surface;
class Font;

class ANIObject;

namespace OnceUpon {

class OnceUpon : public PreGob {
public:
	/** Number of languages we support. */
	static const uint kLanguageCount = 5;


	OnceUpon(GobEngine *vm);
	~OnceUpon();


protected:
	/** A description of a menu button. */
	struct MenuButton {
		bool needDraw; ///< Does the button need drawing?

		int16 left;   ///< Left   coordinate of the button.
		int16 top;    ///< Top    coordinate of the button.
		int16 right;  ///< Right  coordinate of the button.
		int16 bottom; ///< Bottom coordinate of the button.

		int16 srcLeft;   ///< Left  coordinate of the button's sprite.
		int16 srcTop;    ///< Top   coordinate of the button's sprite.
		int16 srcRight;  ///< Right coordinate of the button's sprite.
		int16 srcBottom; ///< Right coordinate of the button's sprite.

		int16 dstX; ///< Destination X coordinate of the button's sprite.
		int16 dstY; ///< Destination Y coordinate of the button's sprite.

		uint id; ///< The button's ID.
	};

	/** Parameters for the stork section. */
	struct StorkParam {
		const char *backdrop; ///< Backdrop image file.

		uint  houseCount;         ///< Number of houses.
		const MenuButton *houses; ///< House button definitions.

		const Stork::BundleDrop *drops; ///< The bundle drop parameters.
	};

	void init();
	void deinit();

	/** Handle the copy protection.
	 *
	 *  @param  colors    Colors the copy protection animals can be.
	 *  @param  shapes    The shape that's the correct answer for each animal in each color.
	 *  @param  obfuscate Extra obfuscate table. correctShape = shapes[colors][obfuscate[animal]].
	 *  @return true if the user guessed the correct shape, false otherwise.
	 */
	bool doCopyProtection(const uint8 colors[7], const uint8 shapes[7 * 20], const uint8 obfuscate[4]);

	/** Show the intro. */
	void showIntro();

	/** Handle the start menu.
	 *
	 *  @param animalsButton Definition of the menu button that leads to the animal names screen. Can be 0.
	 *  @param animalCount   Number of animals in the animal names screen.
	 *  @param animalButtons Definition of the buttons that make up the animals in the animal names screen.
	 *  @param animalNames   File prefixes for the name of each animal.
	 */
	void doStartMenu(const MenuButton *animalsButton, uint animalCount,
	                 const MenuButton *animalButtons, const char * const *animalNames);

	/** Play the game proper. */
	void playGame();


	/** Return the parameters for the stork section. */
	virtual const StorkParam &getStorkParameters() const = 0;


private:
	/** All actions a user can request in a menu. */
	enum MenuAction {
		kMenuActionNone = 0, ///< No action.
		kMenuActionAnimals , ///< Do the animal names.
		kMenuActionPlay    , ///< Play the game.
		kMenuActionRestart , ///< Restart the section.
		kMenuActionMainMenu, ///< Go to the main menu.
		kMenuActionQuit      ///< Quit the game.
	};

	/** Difficulty levels. */
	enum Difficulty {
		kDifficultyBeginner     = 0,
		kDifficultyIntermediate = 1,
		kDifficultyAdvanced     = 2,
		kDifficultyCount
	};

	/** The different sounds common in the game. */
	enum Sound {
		kSoundClick = 0,
		kSoundStork    ,
		kSoundJump     ,
		kSoundCount
	};

	/** Action the character generation wants us to take. */
	enum CharGenAction {
		kCharGenDone    = 0, ///< Created a character, move on.
		kCharGenAbort      , ///< Aborted the character generation.
		kCharGenRestart      ///< Restart the character generation.
	};

	/** A complete screen backup. */
	struct ScreenBackup {
		Surface *screen; ///< Screen contents.
		int palette;     ///< Screen palette.

		bool changedCursor; ///< Did we change the cursor?
		bool cursorVisible; ///< Was the cursor visible?

		ScreenBackup();
		~ScreenBackup();
	};


	/** The number of game sections. */
	static const int kSectionCount = 15;

	static const MenuButton kMainMenuDifficultyButton[]; ///< Difficulty buttons.
	static const MenuButton kSectionButtons[];           ///< Section buttons.

	static const MenuButton kIngameButtons[];            ///< Ingame menu buttons.

	static const MenuButton kAnimalNamesBack;   ///< "Back" button in the animal names screens.
	static const MenuButton kLanguageButtons[]; ///< Language buttons in the animal names screen.

	static const MenuButton kSectionStorkHouses[];

	static const MenuButton kCharGenHeadButtons[];
	static const MenuButton kCharGenHeads[];
	static const MenuButton kCharGenHairButtons[];
	static const MenuButton kCharGenJacketButtons[];
	static const MenuButton kCharGenTrousersButtons[];
	static const MenuButton kCharGenNameEntry[];

	/** All general game sounds we know about. */
	static const char *kSound[kSoundCount];


	static const AnimProperties kClownAnimations[];
	static const AnimProperties kTitleAnimation;
	static const AnimProperties kSectionStorkAnimations[];
	static const AnimProperties kSectionEndAnimations[];


	/** Function pointer type for a section handler. */
	typedef bool (OnceUpon::*SectionFunc)();
	/** Section handler function. */
	static const SectionFunc kSectionFuncs[kSectionCount];


	/** Did we open the game archives? */
	bool _openedArchives;

	// Fonts
	Font *_jeudak;
	Font *_lettre;
	Font *_plettre;
	Font *_glettre;

	/** The current palette. */
	int _palette;

	bool _quit; ///< Did the user request a normal game quit?

	Difficulty _difficulty; ///< The current difficulty.
	int        _section;    ///< The current game section.

	Common::String _name; ///< The name of the child.

	uint8 _house;

	uint8 _head;
	uint8 _colorHair;
	uint8 _colorJacket;
	uint8 _colorTrousers;


	// -- General helpers --

	void setGamePalette(uint palette); ///< Set a game palette.
	void setGameCursor();              ///< Set the default game cursor.

	/** Draw this sprite in a fancy, animated line-by-line way. */
	void drawLineByLine(const Surface &src, int16 left, int16 top, int16 right, int16 bottom,
	                    int16 x, int16 y) const;

	/** Backup the screen contents. */
	void backupScreen(ScreenBackup &backup, bool setDefaultCursor = false);
	/** Restore the screen contents with a previously made backup. */
	void restoreScreen(ScreenBackup &backup);

	Common::String fixString(const Common::String &str) const; ///< Fix a string if necessary.
	void fixTXTStrings(TXTFile &txt) const;                    ///< Fix all strings in a TXT.


	// -- Copy protection helpers --

	/** Set up the copy protection. */
	int8 cpSetup(const uint8 colors[7], const uint8 shapes[7 * 20],
	             const uint8 obfuscate[4], const Surface sprites[2]);
	/** Find the shape under these coordinates. */
	int8 cpFindShape(int16 x, int16 y) const;
	/** Display the "You are wrong" screen. */
	void cpWrong();


	// -- Show different game screens --

	void showWait(uint palette = 0xFFFF); ///< Show the wait / loading screen.
	void showQuote();                     ///< Show the quote about fairytales.
	void showTitle();                     ///< Show the Once Upon A Time title.
	void showChapter(int chapter);        ///< Show a chapter intro text.
	void showByeBye();                    ///< Show the "bye bye" screen.

	/** Handle the "listen to animal names" part. */
	void handleAnimalNames(uint count, const MenuButton *buttons, const char * const *names);


	// -- Menu helpers --

	MenuAction handleStartMenu(const MenuButton *animalsButton); ///< Handle the start  menu.
	MenuAction handleMainMenu();                                 ///< Handle the main   menu.
	MenuAction handleIngameMenu();                               ///< Handle the ingame menu.

	void drawStartMenu(const MenuButton *animalsButton); ///< Draw the start  menu.
	void drawMainMenu();                                 ///< Draw the main   menu.
	void drawIngameMenu();                               ///< Draw the ingame menu.

	/** Draw the difficulty label. */
	void drawMenuDifficulty();

	/** Clear the ingame menu in an animated way. */
	void clearIngameMenu(const Surface &background);

	/** Handle the whole ingame menu. */
	MenuAction doIngameMenu();
	/** Handle the whole ingame menu if ESC or right mouse button was pressed. */
	MenuAction doIngameMenu(int16 &key, MouseButtons &mouseButtons);


	// -- Menu button helpers --

	/** Find the button under these coordinates. */
	int checkButton(const MenuButton *buttons, uint count, int16 x, int16 y, int failValue = -1) const;

	/** Draw a menu button. */
	void drawButton (Surface &dest, const Surface &src, const MenuButton &button, int transp = -1) const;
	/** Draw multiple menu buttons. */
	void drawButtons(Surface &dest, const Surface &src, const MenuButton *buttons, uint count, int transp = -1) const;

	/** Draw a border around a button. */
	void drawButtonBorder(const MenuButton &button, uint8 color);


	// -- Animal names helpers --

	/** Set up the animal chooser. */
	void anSetupChooser();
	/** Set up the language chooser for one animal. */
	void anSetupNames(const MenuButton &animal);
	/** Play / Display the name of an animal in one language. */
	void anPlayAnimalName(const Common::String &animal, uint language);


	// -- Game sections --

	bool playSection();

	bool sectionStork();
	bool sectionChapter1();
	bool sectionParents();
	bool sectionChapter2();
	bool sectionForest0();
	bool sectionChapter3();
	bool sectionEvilCastle();
	bool sectionChapter4();
	bool sectionForest1();
	bool sectionChapter5();
	bool sectionBossFight();
	bool sectionChapter6();
	bool sectionForest2();
	bool sectionChapter7();
	bool sectionEnd();

	CharGenAction characterGenerator();
	void charGenSetup(uint stage);
	void charGenDrawName();

	static bool enterString(Common::String &name, int16 key, uint maxLength, const Font &font);
};

} // End of namespace OnceUpon

} // End of namespace Gob

#endif // GOB_PREGOB_ONCEUPON_ONCEUPON_H
