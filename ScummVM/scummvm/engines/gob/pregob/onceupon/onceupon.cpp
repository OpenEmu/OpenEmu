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

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/surface.h"
#include "gob/draw.h"
#include "gob/video.h"
#include "gob/anifile.h"
#include "gob/aniobject.h"

#include "gob/sound/sound.h"

#include "gob/pregob/txtfile.h"
#include "gob/pregob/gctfile.h"

#include "gob/pregob/onceupon/onceupon.h"
#include "gob/pregob/onceupon/palettes.h"
#include "gob/pregob/onceupon/title.h"
#include "gob/pregob/onceupon/parents.h"
#include "gob/pregob/onceupon/chargenchild.h"

static const uint kLanguageCount = 5;

static const uint kCopyProtectionHelpStringCount = 3;

static const char *kCopyProtectionHelpStrings[Gob::OnceUpon::OnceUpon::kLanguageCount][kCopyProtectionHelpStringCount] = {
	{ // French
		"Consulte le livret des animaux, rep\212re la",
		"page correspondant \205 la couleur de l\'\202cran",
		"et clique le symbole associ\202 \205 l\'animal affich\202.",
	},
	{ // German
		"Suche im Tieralbum die Seite, die der Farbe auf",
		"dem Bildschirm entspricht und klicke auf das",
		"Tiersymbol.",
	},
	{ // English
		"Consult the book of animals, find the page",
		"corresponding to the colour of screen and click",
		"the symbol associated with the animal displayed.",
	},
	{ // Spanish
		"Consulta el libro de los animales, localiza la ",
		"p\240gina que corresponde al color de la pantalla.",
		"Cliquea el s\241mbolo asociado al animal que aparece.",
	},
	{ // Italian
		"Guarda il libretto degli animali, trova la",
		"pagina che corrisponde al colore dello schermo,",
		"clicca il simbolo associato all\'animale presentato",
	}
};

static const char *kCopyProtectionWrongStrings[Gob::OnceUpon::OnceUpon::kLanguageCount] = {
	"Tu t\'es tromp\202, dommage...", // French
	"Schade, du hast dich geirrt."  , // German
	"You are wrong, what a pity!"   , // English
	"Te equivocas, l\240stima..."   , // Spanish
	"Sei Sbagliato, peccato..."       // Italian
};

static const uint kCopyProtectionShapeCount = 5;

static const int16 kCopyProtectionShapeCoords[kCopyProtectionShapeCount][6] = {
	{  0,  51,  26,  75,  60, 154},
	{ 28,  51,  58,  81,  96, 151},
	{ 60,  51,  94,  79, 136, 152},
	{ 96,  51, 136,  71, 180, 155},
	{140,  51, 170,  77, 228, 153}
};

enum ClownAnimation {
	kClownAnimationClownCheer = 0,
	kClownAnimationClownStand = 1,
	kClownAnimationClownCry   = 6
};

// 12 seconds delay for one area full of GCT text
static const uint32 kGCTDelay = 12000;

namespace Gob {

namespace OnceUpon {

const OnceUpon::MenuButton OnceUpon::kMainMenuDifficultyButton[] = {
	{false,  29, 18,  77, 57, 0, 0, 0, 0, 0, 0, (int)kDifficultyBeginner},
	{false, 133, 18, 181, 57, 0, 0, 0, 0, 0, 0, (int)kDifficultyIntermediate},
	{false, 241, 18, 289, 57, 0, 0, 0, 0, 0, 0, (int)kDifficultyAdvanced},
};

const OnceUpon::MenuButton OnceUpon::kSectionButtons[] = {
	{false,  27, 121,  91, 179,   0, 0,   0,  0,   0,   0,  0},
	{ true,  95, 121, 159, 179,   4, 1,  56, 49, 100, 126,  2},
	{ true, 163, 121, 227, 179,  64, 1, 120, 49, 168, 126,  6},
	{ true, 231, 121, 295, 179, 128, 1, 184, 49, 236, 126, 10}
};

const OnceUpon::MenuButton OnceUpon::kIngameButtons[] = {
	{true, 108, 83, 139, 116,   0,   0,  31,  34, 108,  83, 0},
	{true, 144, 83, 175, 116,  36,   0,  67,  34, 144,  83, 1},
	{true, 180, 83, 211, 116,  72,   0, 103,  34, 180,  83, 2}
};

const OnceUpon::MenuButton OnceUpon::kAnimalNamesBack = {
	true, 19, 13, 50, 46, 36, 0, 67, 34, 19, 13, 1
};

const OnceUpon::MenuButton OnceUpon::kLanguageButtons[] = {
	{true,  43,  80,  93, 115,   0, 55,  50, 90,  43,  80, 0},
	{true, 132,  80, 182, 115,  53, 55, 103, 90, 132,  80, 1},
	{true, 234,  80, 284, 115, 106, 55, 156, 90, 234,  80, 2},
	{true,  43, 138,  93, 173, 159, 55, 209, 90,  43, 138, 3},
	{true, 132, 138, 182, 173, 212, 55, 262, 90, 132, 138, 4},
	{true, 234, 138, 284, 173, 265, 55, 315, 90, 234, 138, 2}
};

const char *OnceUpon::kSound[kSoundCount] = {
	"diamant.snd", // kSoundClick
	"cigogne.snd", // kSoundStork
	"saute.snd"    // kSoundJump
};

const OnceUpon::SectionFunc OnceUpon::kSectionFuncs[kSectionCount] = {
	&OnceUpon::sectionStork,
	&OnceUpon::sectionChapter1,
	&OnceUpon::sectionParents,
	&OnceUpon::sectionChapter2,
	&OnceUpon::sectionForest0,
	&OnceUpon::sectionChapter3,
	&OnceUpon::sectionEvilCastle,
	&OnceUpon::sectionChapter4,
	&OnceUpon::sectionForest1,
	&OnceUpon::sectionChapter5,
	&OnceUpon::sectionBossFight,
	&OnceUpon::sectionChapter6,
	&OnceUpon::sectionForest2,
	&OnceUpon::sectionChapter7,
	&OnceUpon::sectionEnd
};


OnceUpon::ScreenBackup::ScreenBackup() : palette(-1), changedCursor(false), cursorVisible(false) {
	screen = new Surface(320, 200, 1);
}

OnceUpon::ScreenBackup::~ScreenBackup() {
	delete screen;
}


OnceUpon::OnceUpon(GobEngine *vm) : PreGob(vm), _openedArchives(false),
	_jeudak(0), _lettre(0), _plettre(0), _glettre(0) {

}

OnceUpon::~OnceUpon() {
	deinit();
}

void OnceUpon::init() {
	deinit();

	// Open data files

	bool hasSTK1 = _vm->_dataIO->openArchive("stk1.stk", true);
	bool hasSTK2 = _vm->_dataIO->openArchive("stk2.stk", true);
	bool hasSTK3 = _vm->_dataIO->openArchive("stk3.stk", true);

	if (!hasSTK1 || !hasSTK2 || !hasSTK3)
		error("OnceUpon::OnceUpon(): Failed to open archives");

	_openedArchives = true;

	// Open fonts

	_jeudak  = _vm->_draw->loadFont("jeudak.let");
	_lettre  = _vm->_draw->loadFont("lettre.let");
	_plettre = _vm->_draw->loadFont("plettre.let");
	_glettre = _vm->_draw->loadFont("glettre.let");

	if (!_jeudak || !_lettre || !_plettre || !_glettre)
		error("OnceUpon::OnceUpon(): Failed to fonts (%d, %d, %d, %d)",
		      _jeudak != 0, _lettre != 0, _plettre != 0, _glettre != 0);

	// Verify the language

	if (_vm->_global->_language == kLanguageAmerican)
		_vm->_global->_language = kLanguageBritish;

	if ((_vm->_global->_language >= kLanguageCount))
		error("We do not support the language \"%s\".\n"
		      "If you are certain that your game copy includes this language,\n"
		      "please contact the ScummVM team with details about this version.\n"
		      "Thanks", _vm->getLangDesc(_vm->_global->_language));

	// Load all our sounds and init the screen

	loadSounds(kSound, kSoundCount);
	initScreen();

	// We start with an invalid palette
	_palette = -1;

	// No quit requested at start
	_quit = false;

	// We start with no selected difficulty and at section 0
	_difficulty = kDifficultyCount;
	_section    = 0;

	// Default name
	_name = "Nemo";

	// Default character properties
	_house         = 0;
	_head          = 0;
	_colorHair     = 0;
	_colorJacket   = 0;
	_colorTrousers = 0;
}

void OnceUpon::deinit() {
	// Free sounds
	freeSounds();

	// Free fonts

	delete _jeudak;
	delete _lettre;
	delete _plettre;
	delete _glettre;

	_jeudak  = 0;
	_lettre  = 0;
	_plettre = 0;
	_glettre = 0;

	// Close archives

	if (_openedArchives) {
		_vm->_dataIO->closeArchive(true);
		_vm->_dataIO->closeArchive(true);
		_vm->_dataIO->closeArchive(true);
	}

	_openedArchives = false;
}

void OnceUpon::setGamePalette(uint palette) {
	if (palette >= kPaletteCount)
		return;

	_palette = palette;

	setPalette(kGamePalettes[palette], kPaletteSize);
}

void OnceUpon::setGameCursor() {
	Surface cursor(320, 16, 1);

	// Set the default game cursor
	_vm->_video->drawPackedSprite("icon.cmp", cursor);
	setCursor(cursor, 105, 0, 120, 15, 0, 0);
}

void OnceUpon::drawLineByLine(const Surface &src, int16 left, int16 top, int16 right, int16 bottom,
                              int16 x, int16 y) const {

	// A special way of drawing something:
	// Draw every other line "downwards", wait a bit after each line
	// Then, draw the remaining lines "upwards" and again wait a bit after each line.

	if (_vm->shouldQuit())
		return;

	const int16 width  = right  - left + 1;
	const int16 height = bottom - top  + 1;

	if ((width <= 0) || (height <= 0))
		return;

	// Draw the even lines downwards
	for (int16 i = 0; i < height; i += 2) {
		if (_vm->shouldQuit())
			return;

		_vm->_draw->_backSurface->blit(src, left, top + i, right, top + i, x, y + i);

		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, x, y + i, x + width - 1, y + 1);
		_vm->_draw->blitInvalidated();

		_vm->_util->longDelay(1);
	}

	// Draw the odd lines upwards
	for (int16 i = (height & 1) ? height : (height - 1); i >= 0; i -= 2) {
		if (_vm->shouldQuit())
			return;

		_vm->_draw->_backSurface->blit(src, left, top + i, right, top + i, x, y + i);

		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, x, y + i, x + width - 1, y + 1);
		_vm->_draw->blitInvalidated();

		_vm->_util->longDelay(1);
	}
}

void OnceUpon::backupScreen(ScreenBackup &backup, bool setDefaultCursor) {
	// Backup the screen and palette
	backup.screen->blit(*_vm->_draw->_backSurface);
	backup.palette = _palette;

	// Backup the cursor

	backup.cursorVisible = isCursorVisible();

	backup.changedCursor = false;
	if (setDefaultCursor) {
		backup.changedCursor = true;

		addCursor();
		setGameCursor();
	}
}

void OnceUpon::restoreScreen(ScreenBackup &backup) {
	if (_vm->shouldQuit())
		return;

	// Restore the screen
	_vm->_draw->_backSurface->blit(*backup.screen);
	_vm->_draw->forceBlit();

	// Restore the palette
	if (backup.palette >= 0)
		setGamePalette(backup.palette);

	// Restore the cursor

	if (!backup.cursorVisible)
		hideCursor();

	if (backup.changedCursor)
		removeCursor();

	backup.changedCursor = false;
}

void OnceUpon::fixTXTStrings(TXTFile &txt) const {
	TXTFile::LineArray &lines = txt.getLines();
	for (uint i = 0; i < lines.size(); i++)
		lines[i].text = fixString(lines[i].text);
}

#include "gob/pregob/onceupon/brokenstrings.h"
Common::String OnceUpon::fixString(const Common::String &str) const {
	const BrokenStringLanguage &broken = kBrokenStrings[_vm->_global->_language];

	for (uint i = 0; i < broken.count; i++) {
		if (str == broken.strings[i].wrong)
			return broken.strings[i].correct;
	}

	return str;
}

enum ClownAnimation {
	kClownAnimationStand = 0,
	kClownAnimationCheer = 1,
	kClownAnimationCry   = 2
};

const PreGob::AnimProperties OnceUpon::kClownAnimations[] = {
	{ 1, 0, ANIObject::kModeContinuous, true, false, false, 0, 0},
	{ 0, 0, ANIObject::kModeOnce      , true, false, false, 0, 0},
	{ 6, 0, ANIObject::kModeOnce      , true, false, false, 0, 0}
};

enum CopyProtectionState {
	kCPStateSetup,     // Set up the screen
	kCPStateWaitUser,  // Waiting for the user to pick a shape
	kCPStateWaitClown, // Waiting for the clown animation to finish
	kCPStateFinish     // Finishing
};

bool OnceUpon::doCopyProtection(const uint8 colors[7], const uint8 shapes[7 * 20], const uint8 obfuscate[4]) {
	fadeOut();
	setPalette(kCopyProtectionPalette, kPaletteSize);

	// Load the copy protection sprites
	Surface sprites[2] = {Surface(320, 200, 1), Surface(320, 200, 1)};

	_vm->_video->drawPackedSprite("grille1.cmp", sprites[0]);
	_vm->_video->drawPackedSprite("grille2.cmp", sprites[1]);

	// Load the clown animation
	ANIFile ani  (_vm, "grille.ani", 320);
	ANIList anims;

	loadAnims(anims, ani, 1, &kClownAnimations[kClownAnimationStand]);

	// Set the copy protection cursor
	setCursor(sprites[1], 5, 110, 20, 134, 3, 0);

	// We start with 2 tries left, not having a correct answer and the copy protection not set up yet
	CopyProtectionState state = kCPStateSetup;

	uint8 triesLeft   =  2;
	int8  animalShape = -1;
	bool  hasCorrect  = false;

	while (!_vm->shouldQuit() && (state != kCPStateFinish)) {
		clearAnim(anims);

		// Set up the screen
		if (state == kCPStateSetup) {
			animalShape = cpSetup(colors, shapes, obfuscate, sprites);

			setAnim(*anims[0], kClownAnimations[kClownAnimationStand]);
			state = kCPStateWaitUser;
		}

		drawAnim(anims);

		// If we're waiting for the clown and he finished, evaluate if we're finished
		if (!anims[0]->isVisible() && (state == kCPStateWaitClown))
			state = (hasCorrect || (--triesLeft == 0)) ? kCPStateFinish : kCPStateSetup;

		showCursor();
		fadeIn();

		endFrame(true);

		int16 mouseX, mouseY;
		MouseButtons mouseButtons;

		checkInput(mouseX, mouseY, mouseButtons);

		if (state == kCPStateWaitUser) {
			// Look if we clicked a shaped and got it right

			int8 guessedShape = -1;
			if (mouseButtons == kMouseButtonsLeft)
				guessedShape = cpFindShape(mouseX, mouseY);

			if (guessedShape >= 0) {
				hasCorrect  = guessedShape == animalShape;
				animalShape = -1;

				setAnim(*anims[0], kClownAnimations[hasCorrect ? kClownAnimationCheer : kClownAnimationCry]);
				state = kCPStateWaitClown;
			}
		}
	}

	freeAnims(anims);

	fadeOut();
	hideCursor();
	clearScreen();

	// Display the "You are wrong" screen
	if (!hasCorrect)
		cpWrong();

	return hasCorrect;
}

int8 OnceUpon::cpSetup(const uint8 colors[7], const uint8 shapes[7 * 20], const uint8 obfuscate[4],
                       const Surface sprites[2]) {

	fadeOut();
	hideCursor();

	// Get a random animal and animal color
	int8 animalColor = _vm->_util->getRandom(7);
	while ((colors[animalColor] == 1) || (colors[animalColor] == 7) || (colors[animalColor] == 11))
		animalColor = _vm->_util->getRandom(7);

	int8 animal = _vm->_util->getRandom(20);

	int8 animalShape = shapes[animalColor * 20 + animal];
	if (animal < 4)
		animal = obfuscate[animal];

	// Get the position of the animal sprite
	int16 animalLeft = (animal % 4) * 80;
	int16 animalTop  = (animal / 4) * 50;

	uint8 sprite = 0;
	if (animalTop >= 200) {
		animalTop -= 200;
		sprite = 1;
	}

	int16 animalRight  = animalLeft + 80 - 1;
	int16 animalBottom = animalTop  + 50 - 1;

	// Fill with the animal color
	_vm->_draw->_backSurface->fill(colors[animalColor]);

	// Print the help line strings
	for (uint i = 0; i < kCopyProtectionHelpStringCount; i++) {
		const char * const helpString = kCopyProtectionHelpStrings[_vm->_global->_language][i];

		const int x = 160 - (strlen(helpString) * _plettre->getCharWidth()) / 2;
		const int y = i * 10 + 5;

		_plettre->drawString(helpString, x, y, 8, 0, true, *_vm->_draw->_backSurface);
	}

	// White rectangle with black border
	_vm->_draw->_backSurface->fillRect( 93, 43, 226, 134, 15);
	_vm->_draw->_backSurface->drawRect( 92, 42, 227, 135,  0);

	// Draw the animal in the animal color
	_vm->_draw->_backSurface->fillRect(120, 63, 199, 112, colors[animalColor]);
	_vm->_draw->_backSurface->blit(sprites[sprite], animalLeft, animalTop, animalRight, animalBottom, 120, 63, 0);

	// Draw the shapes
	for (uint i = 0; i < kCopyProtectionShapeCount; i++) {
		const int16 * const coords = kCopyProtectionShapeCoords[i];

		_vm->_draw->_backSurface->blit(sprites[1], coords[0], coords[1], coords[2], coords[3], coords[4], coords[5], 0);
	}

	_vm->_draw->forceBlit();

	return animalShape;
}

int8 OnceUpon::cpFindShape(int16 x, int16 y) const {
	// Look through all shapes and check if the coordinates are inside one of them
	for (uint i = 0; i < kCopyProtectionShapeCount; i++) {
		const int16 * const coords = kCopyProtectionShapeCoords[i];

		const int16 left   = coords[4];
		const int16 top    = coords[5];
		const int16 right  = coords[4] + (coords[2] - coords[0] + 1) - 1;
		const int16 bottom = coords[5] + (coords[3] - coords[1] + 1) - 1;

		if ((x >= left) && (x <= right) && (y >= top) && (y <= bottom))
			return i;
	}

	return -1;
}

void OnceUpon::cpWrong() {
	// Display the "You are wrong" string, centered

	const char * const wrongString = kCopyProtectionWrongStrings[_vm->_global->_language];
	const int          wrongX      = 160 - (strlen(wrongString) * _plettre->getCharWidth()) / 2;

	_vm->_draw->_backSurface->clear();
	_plettre->drawString(wrongString, wrongX, 100, 15, 0, true, *_vm->_draw->_backSurface);

	_vm->_draw->forceBlit();

	fadeIn();

	waitInput();

	fadeOut();
	clearScreen();
}

void OnceUpon::showIntro() {
	// Show all intro parts

	// "Loading"
	showWait(10);
	if (_vm->shouldQuit())
		return;

	// Quote about fairy tales
	showQuote();
	if (_vm->shouldQuit())
		return;

	// Once Upon A Time title
	showTitle();
	if (_vm->shouldQuit())
		return;

	// Game title screen
	showChapter(0);
	if (_vm->shouldQuit())
		return;

	// "Loading"
	showWait(17);
}

void OnceUpon::showWait(uint palette) {
	// Show the loading floppy

	fadeOut();
	clearScreen();
	setGamePalette(palette);

	Surface wait(320, 43, 1);

	_vm->_video->drawPackedSprite("wait.cmp", wait);
	_vm->_draw->_backSurface->blit(wait, 0, 0, 72, 33, 122, 84);

	_vm->_draw->forceBlit();

	fadeIn();
}

void OnceUpon::showQuote() {
	// Show the quote about fairytales

	fadeOut();
	clearScreen();
	setGamePalette(11);

	static const Font *fonts[3] = { _plettre, _glettre, _plettre };

	TXTFile *quote = loadTXT(getLocFile("gene.tx"), TXTFile::kFormatStringPositionColorFont);
	quote->draw(*_vm->_draw->_backSurface, fonts, ARRAYSIZE(fonts));
	delete quote;

	_vm->_draw->forceBlit();

	fadeIn();

	waitInput();

	fadeOut();
}

const PreGob::AnimProperties OnceUpon::kTitleAnimation = {
	8, 0, ANIObject::kModeContinuous, true, false, false, 0, 0
};

void OnceUpon::showTitle() {
	fadeOut();
	setGamePalette(10);

	Title title(_vm);
	title.play();
}

void OnceUpon::showChapter(int chapter) {
	// Display the intro text to a chapter

	fadeOut();
	clearScreen();
	setGamePalette(11);

	// Parchment background
	_vm->_video->drawPackedSprite("parch.cmp", *_vm->_draw->_backSurface);

	static const Font *fonts[3] = { _plettre, _glettre, _plettre };

	const Common::String chapterFile = getLocFile(Common::String::format("gene%d.tx", chapter));

	TXTFile *gameTitle = loadTXT(chapterFile, TXTFile::kFormatStringPositionColorFont);
	gameTitle->draw(*_vm->_draw->_backSurface, fonts, ARRAYSIZE(fonts));
	delete gameTitle;

	_vm->_draw->forceBlit();

	fadeIn();

	waitInput();

	fadeOut();
}

void OnceUpon::showByeBye() {
	fadeOut();
	hideCursor();
	clearScreen();
	setGamePalette(1);

	_plettre->drawString("Bye Bye....", 140, 80, 2, 0, true, *_vm->_draw->_backSurface);
	_vm->_draw->forceBlit();

	fadeIn();

	_vm->_util->longDelay(1000);

	fadeOut();
}

void OnceUpon::doStartMenu(const MenuButton *animalsButton, uint animalCount,
                           const MenuButton *animalButtons, const char * const *animalNames) {
	clearScreen();

	// Wait until we clicked on of the difficulty buttons and are ready to start playing
	while (!_vm->shouldQuit()) {
		MenuAction action = handleStartMenu(animalsButton);
		if (action == kMenuActionPlay)
			break;

		// If we pressed the "listen to animal names" button, handle that screen
		if (action == kMenuActionAnimals)
			handleAnimalNames(animalCount, animalButtons, animalNames);
	}
}

OnceUpon::MenuAction OnceUpon::handleStartMenu(const MenuButton *animalsButton) {
	ScreenBackup screenBackup;
	backupScreen(screenBackup, true);

	fadeOut();
	setGamePalette(17);
	drawStartMenu(animalsButton);
	showCursor();
	fadeIn();

	MenuAction action = kMenuActionNone;
	while (!_vm->shouldQuit() && (action == kMenuActionNone)) {
		endFrame(true);

		// Check user input

		int16 mouseX, mouseY;
		MouseButtons mouseButtons;

		int16 key = checkInput(mouseX, mouseY, mouseButtons);
		if (key == kKeyEscape)
			// ESC -> Quit
			return kMenuActionQuit;

		if (mouseButtons != kMouseButtonsLeft)
			continue;

		playSound(kSoundClick);

		// If we clicked on a difficulty button, show the selected difficulty and start the game
		int diff = checkButton(kMainMenuDifficultyButton, ARRAYSIZE(kMainMenuDifficultyButton), mouseX, mouseY);
		if (diff >= 0) {
			_difficulty = (Difficulty)diff;
			action      = kMenuActionPlay;

			drawStartMenu(animalsButton);
			_vm->_util->longDelay(1000);
		}

		if (animalsButton && (checkButton(animalsButton, 1, mouseX, mouseY) != -1))
			action = kMenuActionAnimals;

	}

	fadeOut();
	restoreScreen(screenBackup);

	return action;
}

OnceUpon::MenuAction OnceUpon::handleMainMenu() {
	ScreenBackup screenBackup;
	backupScreen(screenBackup, true);

	fadeOut();
	setGamePalette(17);
	drawMainMenu();
	showCursor();
	fadeIn();

	MenuAction action = kMenuActionNone;
	while (!_vm->shouldQuit() && (action == kMenuActionNone)) {
		endFrame(true);

		// Check user input

		int16 mouseX, mouseY;
		MouseButtons mouseButtons;

		int16 key = checkInput(mouseX, mouseY, mouseButtons);
		if (key == kKeyEscape)
			// ESC -> Quit
			return kMenuActionQuit;

		if (mouseButtons != kMouseButtonsLeft)
			continue;

		playSound(kSoundClick);

		// If we clicked on a difficulty button, change the current difficulty level
		int diff = checkButton(kMainMenuDifficultyButton, ARRAYSIZE(kMainMenuDifficultyButton), mouseX, mouseY);
		if ((diff >= 0) && (diff != (int)_difficulty)) {
			_difficulty = (Difficulty)diff;

			drawMainMenu();
		}

		// If we clicked on a section button, restart the game from this section
		int section = checkButton(kSectionButtons, ARRAYSIZE(kSectionButtons), mouseX, mouseY);
		if ((section >= 0) && (section <= _section)) {
			_section = section;
			action   = kMenuActionRestart;
		}

	}

	fadeOut();
	restoreScreen(screenBackup);

	return action;
}

OnceUpon::MenuAction OnceUpon::handleIngameMenu() {
	ScreenBackup screenBackup;
	backupScreen(screenBackup, true);

	drawIngameMenu();
	showCursor();

	MenuAction action = kMenuActionNone;
	while (!_vm->shouldQuit() && (action == kMenuActionNone)) {
		endFrame(true);

		// Check user input

		int16 mouseX, mouseY;
		MouseButtons mouseButtons;

		int16 key = checkInput(mouseX, mouseY, mouseButtons);
		if ((key == kKeyEscape) || (mouseButtons == kMouseButtonsRight))
			// ESC or right mouse button -> Dismiss the menu
			action = kMenuActionPlay;

		if (mouseButtons != kMouseButtonsLeft)
			continue;

		int button = checkButton(kIngameButtons, ARRAYSIZE(kIngameButtons), mouseX, mouseY);
		if      (button == 0)
			action = kMenuActionQuit;
		else if (button == 1)
			action = kMenuActionMainMenu;
		else if (button == 2)
			action = kMenuActionPlay;

	}

	clearIngameMenu(*screenBackup.screen);
	restoreScreen(screenBackup);

	return action;
}

void OnceUpon::drawStartMenu(const MenuButton *animalsButton) {
	// Draw the background
	_vm->_video->drawPackedSprite("menu2.cmp", *_vm->_draw->_backSurface);

	// Draw the "Listen to animal names" button
	if (animalsButton) {
		Surface elements(320, 38, 1);
		_vm->_video->drawPackedSprite("elemenu.cmp", elements);
		_vm->_draw->_backSurface->fillRect(animalsButton->left , animalsButton->top,
		                                   animalsButton->right, animalsButton->bottom, 0);
		drawButton(*_vm->_draw->_backSurface, elements, *animalsButton);
	}

	// Highlight the current difficulty
	drawMenuDifficulty();

	_vm->_draw->forceBlit();
}

void OnceUpon::drawMainMenu() {
	// Draw the background
	_vm->_video->drawPackedSprite("menu.cmp", *_vm->_draw->_backSurface);

	// Highlight the current difficulty
	drawMenuDifficulty();

	// Draw the section buttons
	Surface elements(320, 200, 1);
	_vm->_video->drawPackedSprite("elemenu.cmp", elements);

	for (uint i = 0; i < ARRAYSIZE(kSectionButtons); i++) {
		const MenuButton &button = kSectionButtons[i];

		if (!button.needDraw)
			continue;

		if (_section >= (int)button.id)
			drawButton(*_vm->_draw->_backSurface, elements, button);
	}

	_vm->_draw->forceBlit();
}

void OnceUpon::drawIngameMenu() {
	Surface menu(320, 34, 1);
	_vm->_video->drawPackedSprite("icon.cmp", menu);

	// Draw the menu in a special way, button by button
	for (uint i = 0; i < ARRAYSIZE(kIngameButtons); i++) {
		const MenuButton &button = kIngameButtons[i];

		drawLineByLine(menu, button.srcLeft, button.srcTop, button.srcRight, button.srcBottom,
		               button.dstX, button.dstY);
	}

	_vm->_draw->forceBlit();
	_vm->_video->retrace();
}

void OnceUpon::drawMenuDifficulty() {
	if (_difficulty == kDifficultyCount)
		return;

	TXTFile *difficulties = loadTXT(getLocFile("diffic.tx"), TXTFile::kFormatStringPositionColor);

	// Draw the difficulty name
	difficulties->draw((uint) _difficulty, *_vm->_draw->_backSurface, &_plettre, 1);

	// Draw a border around the current difficulty
	drawButtonBorder(kMainMenuDifficultyButton[_difficulty], difficulties->getLines()[_difficulty].color);

	delete difficulties;
}

void OnceUpon::clearIngameMenu(const Surface &background) {
	if (_vm->shouldQuit())
		return;

	// Find the area encompassing the whole ingame menu

	int16 left   = 0x7FFF;
	int16 top    = 0x7FFF;
	int16 right  = 0x0000;
	int16 bottom = 0x0000;

	for (uint i = 0; i < ARRAYSIZE(kIngameButtons); i++) {
		const MenuButton &button = kIngameButtons[i];

		if (!button.needDraw)
			continue;

		left   = MIN<int16>(left  , button.dstX);
		top    = MIN<int16>(top   , button.dstY);
		right  = MAX<int16>(right , button.dstX + (button.srcRight  - button.srcLeft + 1) - 1);
		bottom = MAX<int16>(bottom, button.dstY + (button.srcBottom - button.srcTop  + 1) - 1);
	}

	if ((left > right) || (top > bottom))
		return;

	// Clear it line by line
	drawLineByLine(background, left, top, right, bottom, left, top);
}

OnceUpon::MenuAction OnceUpon::doIngameMenu() {
	// Show the ingame menu
	MenuAction action = handleIngameMenu();

	if ((action == kMenuActionQuit) || _vm->shouldQuit()) {

		// User pressed the quit button, or quit ScummVM
		_quit  = true;
		action = kMenuActionQuit;

	} else if (action == kMenuActionPlay) {

		// User pressed the return to game button
		action = kMenuActionPlay;

	} else if (kMenuActionMainMenu) {

		// User pressed the return to main menu button
		action = handleMainMenu();
	}

	return action;
}

OnceUpon::MenuAction OnceUpon::doIngameMenu(int16 &key, MouseButtons &mouseButtons) {
	if ((key != kKeyEscape) && (mouseButtons != kMouseButtonsRight))
		return kMenuActionNone;

	key = 0;
	mouseButtons = kMouseButtonsNone;

	MenuAction action = doIngameMenu();
	if (action == kMenuActionPlay)
		action = kMenuActionNone;

	return action;
}

int OnceUpon::checkButton(const MenuButton *buttons, uint count, int16 x, int16 y, int failValue) const {
	// Look through all buttons, and return the ID of the button we're in

	for (uint i = 0; i < count; i++) {
		const MenuButton &button = buttons[i];

		if ((x >= button.left) && (x <= button.right) && (y >= button.top) && (y <= button.bottom))
			return (int)button.id;
	}

	// We're in none of these buttons, return the fail value
	return failValue;
}

void OnceUpon::drawButton(Surface &dest, const Surface &src, const MenuButton &button, int transp) const {
	dest.blit(src, button.srcLeft, button.srcTop, button.srcRight, button.srcBottom, button.dstX, button.dstY, transp);
}

void OnceUpon::drawButtons(Surface &dest, const Surface &src, const MenuButton *buttons, uint count, int transp) const {
	for (uint i = 0; i < count; i++) {
		const MenuButton &button = buttons[i];

		if (!button.needDraw)
			continue;

		drawButton(dest, src, button, transp);
	}
}

void OnceUpon::drawButtonBorder(const MenuButton &button, uint8 color) {
	_vm->_draw->_backSurface->drawRect(button.left, button.top, button.right, button.bottom, color);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, button.left, button.top, button.right, button.bottom);
}

enum AnimalNamesState {
	kANStateChoose, // We're in the animal chooser
	kANStateNames,  // We're in the language chooser
	kANStateFinish  // We're finished
};

void OnceUpon::handleAnimalNames(uint count, const MenuButton *buttons, const char * const *names) {
	fadeOut();
	clearScreen();
	setGamePalette(19);

	bool cursorVisible = isCursorVisible();

	// Set the cursor
	addCursor();
	setGameCursor();

	anSetupChooser();

	int8 _animal = -1;

	AnimalNamesState state = kANStateChoose;
	while (!_vm->shouldQuit() && (state != kANStateFinish)) {
		showCursor();
		fadeIn();

		endFrame(true);

		// Check user input

		int16 mouseX, mouseY;
		MouseButtons mouseButtons;

		checkInput(mouseX, mouseY, mouseButtons);

		// If we moused over an animal button, draw a border around it
		int animal = checkButton(buttons, count, mouseX, mouseY);
		if ((state == kANStateChoose) && (animal != _animal)) {
			// Erase the old border
			if (_animal >= 0)
				drawButtonBorder(buttons[_animal], 15);

			_animal = animal;

			// Draw the new border
			if (_animal >= 0)
				drawButtonBorder(buttons[_animal], 10);
		}

		if (mouseButtons != kMouseButtonsLeft)
			continue;

		playSound(kSoundClick);

		// We clicked on a language button, play the animal name
		int language = checkButton(kLanguageButtons, ARRAYSIZE(kLanguageButtons), mouseX, mouseY);
		if ((state == kANStateNames) && (language >= 0))
			anPlayAnimalName(names[_animal], language);

		// We clicked on an animal
		if ((state == kANStateChoose) && (_animal >= 0)) {
			anSetupNames(buttons[_animal]);

			state = kANStateNames;
		}

		// If we clicked on the back button, go back
		if (checkButton(&kAnimalNamesBack, 1, mouseX, mouseY) != -1) {
			if        (state == kANStateNames) {
				anSetupChooser();

				state = kANStateChoose;
			} else if (state == kANStateChoose)
				state = kANStateFinish;
		}
	}

	fadeOut();

	// Restore the cursor
	if (!cursorVisible)
		hideCursor();
	removeCursor();
}

void OnceUpon::anSetupChooser() {
	fadeOut();

	_vm->_video->drawPackedSprite("dico.cmp", *_vm->_draw->_backSurface);

	// Draw the back button
	Surface menu(320, 34, 1);
	_vm->_video->drawPackedSprite("icon.cmp", menu);
	drawButton(*_vm->_draw->_backSurface, menu, kAnimalNamesBack);

	// "Choose an animal"
	TXTFile *choose = loadTXT(getLocFile("choisi.tx"), TXTFile::kFormatStringPosition);
	choose->draw(*_vm->_draw->_backSurface, &_plettre, 1, 14);
	delete choose;

	_vm->_draw->forceBlit();
}

void OnceUpon::anSetupNames(const MenuButton &animal) {
	fadeOut();

	Surface background(320, 200, 1);

	_vm->_video->drawPackedSprite("dico.cmp", background);

	// Draw the background and clear what we don't need
	_vm->_draw->_backSurface->blit(background);
	_vm->_draw->_backSurface->fillRect(19, 19, 302, 186, 15);

	// Draw the back button
	Surface menu(320, 34, 1);
	_vm->_video->drawPackedSprite("icon.cmp", menu);
	drawButton(*_vm->_draw->_backSurface, menu, kAnimalNamesBack);

	// Draw the animal
	drawButton(*_vm->_draw->_backSurface, background, animal);

	// Draw the language buttons
	Surface elements(320, 200, 1);
	_vm->_video->drawPackedSprite("elemenu.cmp", elements);
	drawButtons(*_vm->_draw->_backSurface, elements, kLanguageButtons, ARRAYSIZE(kLanguageButtons));

	// Draw the language names
	_plettre->drawString("Fran\207ais",  43,  70, 10, 15, true, *_vm->_draw->_backSurface);
	_plettre->drawString("Deutsch"    , 136,  70, 10, 15, true, *_vm->_draw->_backSurface);
	_plettre->drawString("English"    , 238,  70, 10, 15, true, *_vm->_draw->_backSurface);
	_plettre->drawString("Italiano"   ,  43, 128, 10, 15, true, *_vm->_draw->_backSurface);
	_plettre->drawString("Espa\244ol" , 136, 128, 10, 15, true, *_vm->_draw->_backSurface);
	_plettre->drawString("English"    , 238, 128, 10, 15, true, *_vm->_draw->_backSurface);

	_vm->_draw->forceBlit();
}

void OnceUpon::anPlayAnimalName(const Common::String &animal, uint language) {
	// Sound file to play
	Common::String soundFile = animal + "_" + kLanguageSuffixLong[language] + ".snd";

	// Get the name of the animal
	TXTFile *names = loadTXT(animal + ".anm", TXTFile::kFormatString);
	Common::String name = names->getLines()[language].text;
	delete names;

	// It should be centered on the screen
	const int nameX = 160 - (name.size() * _plettre->getCharWidth()) / 2;

	// Backup the screen surface
	Surface backup(162, 23, 1);
	backup.blit(*_vm->_draw->_backSurface, 78, 123, 239, 145, 0, 0);

	// Draw the name border
	Surface nameBorder(162, 23, 1);
	_vm->_video->drawPackedSprite("mot.cmp", nameBorder);
	_vm->_draw->_backSurface->blit(nameBorder, 0, 0, 161, 22, 78, 123);

	// Print the animal name
	_plettre->drawString(name, nameX, 129, 10, 0, true, *_vm->_draw->_backSurface);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 78, 123, 239, 145);

	playSoundFile(soundFile);

	// Restore the screen
	_vm->_draw->_backSurface->blit(backup, 0, 0, 161, 22, 78, 123);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 78, 123, 239, 145);
}

void OnceUpon::playGame() {
	while (!_vm->shouldQuit() && !_quit) {
		// Play a section and advance to the next section if we finished it
		if (playSection())
			_section = MIN(_section + 1, kSectionCount - 1);
	}

	// If we quit through the game and not through ScummVM, show the "Bye Bye" screen
	if (!_vm->shouldQuit())
		showByeBye();
}

bool OnceUpon::playSection() {
	if ((_section < 0) || (_section >= ARRAYSIZE(kSectionFuncs))) {
		_quit = true;
		return false;
	}

	return (this->*kSectionFuncs[_section])();
}

const PreGob::AnimProperties OnceUpon::kSectionStorkAnimations[] = {
	{ 0, 0, ANIObject::kModeContinuous, true, false, false, 0, 0},
	{ 1, 0, ANIObject::kModeContinuous, true, false, false, 0, 0},
	{ 2, 0, ANIObject::kModeContinuous, true, false, false, 0, 0},
	{ 3, 0, ANIObject::kModeContinuous, true, false, false, 0, 0},
	{ 4, 0, ANIObject::kModeContinuous, true, false, false, 0, 0},
	{ 5, 0, ANIObject::kModeContinuous, true, false, false, 0, 0},
	{ 6, 0, ANIObject::kModeContinuous, true, false, false, 0, 0},
	{ 7, 0, ANIObject::kModeContinuous, true, false, false, 0, 0},
	{ 8, 0, ANIObject::kModeContinuous, true, false, false, 0, 0},
	{17, 0, ANIObject::kModeContinuous, true, false, false, 0, 0},
	{16, 0, ANIObject::kModeContinuous, true, false, false, 0, 0},
	{15, 0, ANIObject::kModeContinuous, true, false, false, 0, 0}
};

enum StorkState {
	kStorkStateWaitUser,
	kStorkStateWaitBundle,
	kStorkStateFinish
};

bool OnceUpon::sectionStork() {
	fadeOut();
	hideCursor();
	setGamePalette(0);
	setGameCursor();

	const StorkParam &param = getStorkParameters();

	Surface backdrop(320, 200, 1);

	// Draw the frame
	_vm->_video->drawPackedSprite("cadre.cmp", *_vm->_draw->_backSurface);

	// Draw the backdrop
	_vm->_video->drawPackedSprite(param.backdrop, backdrop);
	_vm->_draw->_backSurface->blit(backdrop, 0, 0, 288, 175, 16, 12);

	// "Where does the stork go?"
	TXTFile *whereStork = loadTXT(getLocFile("ouva.tx"), TXTFile::kFormatStringPositionColor);
	whereStork->draw(*_vm->_draw->_backSurface, &_plettre, 1);

	// Where the stork actually goes
	GCTFile *thereStork = loadGCT(getLocFile("choix.gc"));
	thereStork->setArea(17, 18, 303, 41);

	ANIFile ani(_vm, "present.ani", 320);
	ANIList anims;

	Stork *stork = new Stork(_vm, ani);

	loadAnims(anims, ani, ARRAYSIZE(kSectionStorkAnimations), kSectionStorkAnimations);
	anims.push_back(stork);

	drawAnim(anims);

	_vm->_draw->forceBlit();

	int8 storkSoundWait = 0;

	StorkState state  = kStorkStateWaitUser;
	MenuAction action = kMenuActionNone;
	while (!_vm->shouldQuit() && (state != kStorkStateFinish)) {
		// Play the stork sound
		if (--storkSoundWait == 0)
			playSound(kSoundStork);
		if (storkSoundWait <= 0)
			storkSoundWait = 50 - _vm->_util->getRandom(30);

		// Check if the bundle landed
		if ((state == kStorkStateWaitBundle) && stork->hasBundleLanded())
			state = kStorkStateFinish;

		// Check user input

		int16 mouseX, mouseY;
		MouseButtons mouseButtons;

		int16 key = checkInput(mouseX, mouseY, mouseButtons);

		action = doIngameMenu(key, mouseButtons);
		if (action != kMenuActionNone) {
			state = kStorkStateFinish;
			break;
		}

		clearAnim(anims);

		if (mouseButtons == kMouseButtonsLeft) {
			stopSound();
			playSound(kSoundClick);

			int house = checkButton(param.houses, param.houseCount, mouseX, mouseY);
			if ((state == kStorkStateWaitUser) && (house >= 0)) {

				_house = house;

				stork->dropBundle(param.drops[house]);
				state = kStorkStateWaitBundle;

				// Remove the "Where does the stork go?" text
				int16 left, top, right, bottom;
				if (whereStork->clear(*_vm->_draw->_backSurface, left, top, right, bottom))
					_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);

				// Print the text where the stork actually goes
				thereStork->selectLine(3, house); // The house
				thereStork->selectLine(4, house); // The house's inhabitants
				if (thereStork->draw(*_vm->_draw->_backSurface, 2, *_plettre, 10, left, top, right, bottom))
					_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);
			}
		}

		drawAnim(anims);
		showCursor();
		fadeIn();

		endFrame(true);
	}

	freeAnims(anims);
	delete thereStork;
	delete whereStork;

	fadeOut();
	hideCursor();

	// Didn't complete the section
	if (action != kMenuActionNone)
		return false;

	// Move on to the character generator

	CharGenAction charGenAction = kCharGenRestart;
	while (charGenAction == kCharGenRestart)
		charGenAction = characterGenerator();

	// Did we successfully create a character?
	return charGenAction == kCharGenDone;
}

const OnceUpon::MenuButton OnceUpon::kCharGenHeadButtons[] = {
	{true, 106, 146, 152, 180,   0,  0,  47, 34, 106, 146, 0},
	{true, 155, 146, 201, 180,  49,  0,  96, 34, 155, 146, 1},
	{true, 204, 146, 250, 180,  98,  0, 145, 34, 204, 146, 2},
	{true, 253, 146, 299, 180, 147,  0, 194, 34, 253, 146, 3}
};

const OnceUpon::MenuButton OnceUpon::kCharGenHeads[] = {
	{true,   0,   0,   0,   0,  29,  4,  68, 31,  40,  51, 0},
	{true,   0,   0,   0,   0,  83,  4, 113, 31,  45,  51, 1},
	{true,   0,   0,   0,   0, 132,  4, 162, 31,  45,  51, 2},
	{true,   0,   0,   0,   0, 182,  4, 211, 31,  45,  51, 3}
};

const OnceUpon::MenuButton OnceUpon::kCharGenHairButtons[] = {
	{true, 105,  55, 124,  70, 271,  1, 289, 15, 105,  55, 0x04},
	{true, 105,  74, 124,  89, 271, 20, 289, 34, 105,  74, 0x07}
};

const OnceUpon::MenuButton OnceUpon::kCharGenJacketButtons[] = {
	{true, 105,  90, 124, 105, 271, 39, 289, 53, 105,  90, 0x06},
	{true, 105, 109, 124, 124, 271, 58, 289, 72, 105, 109, 0x02}
};

const OnceUpon::MenuButton OnceUpon::kCharGenTrousersButtons[] = {
	{true, 105, 140, 124, 155, 271, 77, 289,  91, 105, 140, 0x01},
	{true, 105, 159, 124, 174, 271, 96, 289, 110, 105, 159, 0x03}
};

const OnceUpon::MenuButton OnceUpon::kCharGenNameEntry[] = {
	{true, 0, 0, 0, 0,   0,  38,  54,  48, 140, 145, 0},
	{true, 0, 0, 0, 0, 106,  38, 159,  48, 195, 145, 0},
	{true, 0, 0, 0, 0,   0, 105,  54, 121, 140, 156, 0},
	{true, 0, 0, 0, 0, 106, 105, 159, 121, 195, 156, 0}
};

enum CharGenState {
	kCharGenStateHead     = 0, // Choose a head
	kCharGenStateHair        , // Choose hair color
	kCharGenStateJacket      , // Choose jacket color
	kCharGenStateTrousers    , // Choose trousers color
	kCharGenStateName        , // Choose name
	kCharGenStateSure        , // "Are you sure?"
	kCharGenStateStoryName   , // "We're going to tell the story of $NAME"
	kCharGenStateFinish        // Finished
};

void OnceUpon::charGenSetup(uint stage) {
	Surface choix(320, 200, 1), elchoix(320, 200, 1), paperDoll(65, 137, 1);

	_vm->_video->drawPackedSprite("choix.cmp"  , choix);
	_vm->_video->drawPackedSprite("elchoix.cmp", elchoix);

	paperDoll.blit(choix, 200, 0, 264, 136, 0, 0);

	GCTFile *text = loadGCT(getLocFile("choix.gc"));
	text->setArea(17, 18, 303, 41);
	text->setText(9, _name);

	// Background
	_vm->_video->drawPackedSprite("cadre.cmp", *_vm->_draw->_backSurface);
	_vm->_draw->_backSurface->fillRect(16, 50, 303, 187, 5);

	// Character sprite frame
	_vm->_draw->_backSurface->blit(choix, 0, 38, 159, 121, 140, 54);

	// Recolor the paper doll parts
	if (_colorHair != 0xFF)
		elchoix.recolor(0x0C, _colorHair);

	if (_colorJacket != 0xFF)
		paperDoll.recolor(0x0A, _colorJacket);

	if (_colorTrousers != 0xFF)
		paperDoll.recolor(0x09, _colorTrousers);

	_vm->_draw->_backSurface->blit(paperDoll, 32, 51);

	// Paper doll head
	if (_head != 0xFF)
		drawButton(*_vm->_draw->_backSurface, elchoix, kCharGenHeads[_head], 0);

	if (stage == kCharGenStateHead) {
		// Head buttons
		drawButtons(*_vm->_draw->_backSurface, choix, kCharGenHeadButtons, ARRAYSIZE(kCharGenHeadButtons));

		// "Choose a head"
		int16 left, top, right, bottom;
		text->draw(*_vm->_draw->_backSurface, 5, *_plettre, 10, left, top, right, bottom);

	} else if (stage == kCharGenStateHair) {
		// Hair color buttons
		drawButtons(*_vm->_draw->_backSurface, choix, kCharGenHairButtons, ARRAYSIZE(kCharGenHairButtons));

		// "What color is the hair?"
		int16 left, top, right, bottom;
		text->draw(*_vm->_draw->_backSurface, 6, *_plettre, 10, left, top, right, bottom);

	} else if (stage == kCharGenStateJacket) {
		// Jacket color buttons
		drawButtons(*_vm->_draw->_backSurface, choix, kCharGenJacketButtons, ARRAYSIZE(kCharGenJacketButtons));

		// "What color is the jacket?"
		int16 left, top, right, bottom;
		text->draw(*_vm->_draw->_backSurface, 7, *_plettre, 10, left, top, right, bottom);

	} else if (stage == kCharGenStateTrousers) {
		// Trousers color buttons
		drawButtons(*_vm->_draw->_backSurface, choix, kCharGenTrousersButtons, ARRAYSIZE(kCharGenTrousersButtons));

		// "What color are the trousers?"
		int16 left, top, right, bottom;
		text->draw(*_vm->_draw->_backSurface, 8, *_plettre, 10, left, top, right, bottom);

	} else if (stage == kCharGenStateName) {
		// Name entry field
		drawButtons(*_vm->_draw->_backSurface, choix, kCharGenNameEntry, ARRAYSIZE(kCharGenNameEntry));

		// "Enter name"
		int16 left, top, right, bottom;
		text->draw(*_vm->_draw->_backSurface, 10, *_plettre, 10, left, top, right, bottom);

		charGenDrawName();
	} else if (stage == kCharGenStateSure) {
		// Name entry field
		drawButtons(*_vm->_draw->_backSurface, choix, kCharGenNameEntry, ARRAYSIZE(kCharGenNameEntry));

		// "Are you sure?"
		TXTFile *sure = loadTXT(getLocFile("estu.tx"), TXTFile::kFormatStringPositionColor);
		sure->draw(*_vm->_draw->_backSurface, &_plettre, 1);
		delete sure;

		charGenDrawName();
	} else if (stage == kCharGenStateStoryName) {

		// "We're going to tell the story of $NAME"
		int16 left, top, right, bottom;
		text->draw(*_vm->_draw->_backSurface, 11, *_plettre, 10, left, top, right, bottom);
	}

	delete text;
}

bool OnceUpon::enterString(Common::String &name, int16 key, uint maxLength, const Font &font) {
	if (key == 0)
		return true;

	if (key == kKeyBackspace) {
		name.deleteLastChar();
		return true;
	}

	if (key == kKeySpace)
		key = ' ';

	if ((key >= ' ') && (key <= 0xFF)) {
		if (name.size() >= maxLength)
			return false;

		if (!font.hasChar(key))
			return false;

		name += (char) key;
		return true;
	}

	return false;
}

void OnceUpon::charGenDrawName() {
	_vm->_draw->_backSurface->fillRect(147, 151, 243, 166, 1);

	const int16 nameY = 151 + ((166 - 151 + 1 -       _plettre->getCharHeight())  / 2);
	const int16 nameX = 147 + ((243 - 147 + 1 - (15 * _plettre->getCharWidth ())) / 2);

	_plettre->drawString(_name, nameX, nameY, 10, 0, true, *_vm->_draw->_backSurface);

	const int16 cursorLeft   = nameX + _name.size() * _plettre->getCharWidth();
	const int16 cursorTop    = nameY;
	const int16 cursorRight  = cursorLeft + _plettre->getCharWidth() - 1;
	const int16 cursorBottom = cursorTop + _plettre->getCharHeight() - 1;

	_vm->_draw->_backSurface->fillRect(cursorLeft, cursorTop, cursorRight, cursorBottom, 10);

	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 147, 151, 243, 166);
}

OnceUpon::CharGenAction OnceUpon::characterGenerator() {
	fadeOut();
	hideCursor();
	setGameCursor();

	showWait(1);

	_name.clear();

	_head          = 0xFF;
	_colorHair     = 0xFF;
	_colorJacket   = 0xFF;
	_colorTrousers = 0xFF;

	CharGenState state = kCharGenStateHead;
	charGenSetup(state);

	ANIFile ani(_vm, "ba.ani", 320);

	ani.recolor(0x0F, 0x0C);
	ani.recolor(0x0E, 0x0A);
	ani.recolor(0x08, 0x09);

	CharGenChild *child = new CharGenChild(ani);

	ANIList anims;
	anims.push_back(child);

	fadeOut();
	_vm->_draw->forceBlit();

	CharGenAction action = kCharGenRestart;
	while (!_vm->shouldQuit() && (state != kCharGenStateFinish)) {
		// Check user input

		int16 mouseX, mouseY;
		MouseButtons mouseButtons;

		int16 key = checkInput(mouseX, mouseY, mouseButtons);

		MenuAction menuAction = doIngameMenu(key, mouseButtons);
		if (menuAction != kMenuActionNone) {
			state  = kCharGenStateFinish;
			action = kCharGenAbort;
			break;
		}

		clearAnim(anims);

		if (state == kCharGenStateStoryName) {
			if ((mouseButtons != kMouseButtonsNone) || (key != 0)) {
				state = kCharGenStateFinish;
				action = kCharGenDone;
				break;
			}
		}

		if (state == kCharGenStateSure) {
			// Not sure => restart
			if ((key == 'N') || (key == 'n')) { // No / Nein / Non
				state = kCharGenStateFinish;
				action = kCharGenRestart;
				break;
			}

			if ((key == 'Y') || (key == 'y') || // Yes
			    (key == 'J') || (key == 'j') || // Ja
			    (key == 'S') || (key == 's') || // Si
			    (key == 'O') || (key == 'o')) { // Oui

				state = kCharGenStateStoryName;
				charGenSetup(state);
				_vm->_draw->forceBlit();
			}
		}

		if (state == kCharGenStateName) {
			if (enterString(_name, key, 14, *_plettre)) {
				_vm->_draw->_backSurface->fillRect(147, 151, 243, 166, 1);

				const int16 nameY = 151 + ((166 - 151 + 1 -       _plettre->getCharHeight())  / 2);
				const int16 nameX = 147 + ((243 - 147 + 1 - (15 * _plettre->getCharWidth ())) / 2);

				_plettre->drawString(_name, nameX, nameY, 10, 0, true, *_vm->_draw->_backSurface);

				const int16 cursorLeft   = nameX + _name.size() * _plettre->getCharWidth();
				const int16 cursorTop    = nameY;
				const int16 cursorRight  = cursorLeft + _plettre->getCharWidth() - 1;
				const int16 cursorBottom = cursorTop + _plettre->getCharHeight() - 1;

				_vm->_draw->_backSurface->fillRect(cursorLeft, cursorTop, cursorRight, cursorBottom, 10);

				_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 147, 151, 243, 166);
			}

			if ((key == kKeyReturn) && !_name.empty()) {
				_name.trim();
				_name.setChar(Util::toCP850Upper(_name[0]), 0);

				state = kCharGenStateSure;
				charGenSetup(state);
				_vm->_draw->forceBlit();
			}
		}

		if (mouseButtons == kMouseButtonsLeft) {
			stopSound();
			playSound(kSoundClick);

			int trousers = checkButton(kCharGenTrousersButtons, ARRAYSIZE(kCharGenTrousersButtons), mouseX, mouseY);
			if ((state == kCharGenStateTrousers) && (trousers >= 0)) {
				_colorTrousers = trousers;

				ani.recolor(0x09, _colorTrousers);

				state = kCharGenStateName;
				charGenSetup(state);
				_vm->_draw->forceBlit();
			}

			int jacket = checkButton(kCharGenJacketButtons, ARRAYSIZE(kCharGenJacketButtons), mouseX, mouseY);
			if ((state == kCharGenStateJacket) && (jacket >= 0)) {
				_colorJacket = jacket;

				ani.recolor(0x0A, _colorJacket);

				state = kCharGenStateTrousers;
				charGenSetup(state);
				_vm->_draw->forceBlit();
			}

			int hair = checkButton(kCharGenHairButtons, ARRAYSIZE(kCharGenHairButtons), mouseX, mouseY);
			if ((state == kCharGenStateHair) && (hair >= 0)) {
				_colorHair = hair;

				ani.recolor(0x0C, _colorHair);

				state = kCharGenStateJacket;
				charGenSetup(state);
				_vm->_draw->forceBlit();
			}

			int head = checkButton(kCharGenHeadButtons, ARRAYSIZE(kCharGenHeadButtons), mouseX, mouseY);
			if ((state == kCharGenStateHead) && (head >= 0)) {
				_head = head;

				state = kCharGenStateHair;
				charGenSetup(state);
				_vm->_draw->forceBlit();
			}
		}

		drawAnim(anims);

		// Play the child sounds
		CharGenChild::Sound childSound = child->shouldPlaySound();
		if        (childSound == CharGenChild::kSoundWalk) {
			beep(50, 10);
		} else if (childSound == CharGenChild::kSoundJump) {
			stopSound();
			playSound(kSoundJump);
		}

		showCursor();
		fadeIn();

		endFrame(true);
	}

	fadeOut();
	hideCursor();

	freeAnims(anims);

	if (_vm->shouldQuit())
		return kCharGenAbort;

	return action;
}

bool OnceUpon::sectionChapter1() {
	showChapter(1);
	return true;
}

bool OnceUpon::sectionParents() {
	fadeOut();
	setGamePalette(14);
	clearScreen();

	const Common::String seq = ((_house == 1) || (_house == 2)) ? "parents.seq" : "parents2.seq";
	const Common::String gct = getLocFile("mefait.gc");

	Parents parents(_vm, seq, gct, _name, _house, *_plettre, kGamePalettes[14], kGamePalettes[13], kPaletteSize);
	parents.play();

	warning("OnceUpon::sectionParents(): TODO: Item search");
	return true;
}

bool OnceUpon::sectionChapter2() {
	showChapter(2);
	return true;
}

bool OnceUpon::sectionForest0() {
	warning("OnceUpon::sectionForest0(): TODO");
	return true;
}

bool OnceUpon::sectionChapter3() {
	showChapter(3);
	return true;
}

bool OnceUpon::sectionEvilCastle() {
	warning("OnceUpon::sectionEvilCastle(): TODO");
	return true;
}

bool OnceUpon::sectionChapter4() {
	showChapter(4);
	return true;
}

bool OnceUpon::sectionForest1() {
	warning("OnceUpon::sectionForest1(): TODO");
	return true;
}

bool OnceUpon::sectionChapter5() {
	showChapter(5);
	return true;
}

bool OnceUpon::sectionBossFight() {
	warning("OnceUpon::sectionBossFight(): TODO");
	return true;
}

bool OnceUpon::sectionChapter6() {
	showChapter(6);
	return true;
}

bool OnceUpon::sectionForest2() {
	warning("OnceUpon::sectionForest2(): TODO");
	return true;
}

bool OnceUpon::sectionChapter7() {
	showChapter(7);
	return true;
}

const PreGob::AnimProperties OnceUpon::kSectionEndAnimations[] = {
	{ 0, 0, ANIObject::kModeContinuous, true, false, false, 0, 0},
	{ 6, 0, ANIObject::kModeContinuous, true, false, false, 0, 0},
	{ 9, 0, ANIObject::kModeContinuous, true, false, false, 0, 0},
	{11, 0, ANIObject::kModeContinuous, true, false, false, 0, 0}
};

bool OnceUpon::sectionEnd() {
	fadeOut();
	setGamePalette(9);

	_vm->_video->drawPackedSprite("cadre.cmp", *_vm->_draw->_backSurface);

	Surface endBackground(320, 200, 1);
	_vm->_video->drawPackedSprite("fin.cmp", endBackground);

	_vm->_draw->_backSurface->blit(endBackground, 0, 0, 288, 137, 16, 50);

	GCTFile *endText = loadGCT(getLocFile("final.gc"));
	endText->setArea(17, 18, 303, 41);
	endText->setText(1, _name);

	ANIFile ani(_vm, "fin.ani", 320);
	ANIList anims;

	loadAnims(anims, ani, ARRAYSIZE(kSectionEndAnimations), kSectionEndAnimations);
	drawAnim(anims);

	_vm->_draw->forceBlit();

	uint32 textStartTime = 0;

	MenuAction action = kMenuActionNone;
	while (!_vm->shouldQuit() && (action == kMenuActionNone)) {
		// Check user input

		int16 mouseX, mouseY;
		MouseButtons mouseButtons;

		int16 key = checkInput(mouseX, mouseY, mouseButtons);

		action = doIngameMenu(key, mouseButtons);
		if (action != kMenuActionNone)
			break;

		clearAnim(anims);

		// Pressed a key or mouse button => Skip to next area-full of text
		if ((mouseButtons == kMouseButtonsLeft) || (key != 0))
			textStartTime = 0;

		// Draw the next area-full of text
		uint32 now = _vm->_util->getTimeKey();
		if (!endText->finished() && ((textStartTime == 0) || (now >= (textStartTime + kGCTDelay)))) {
			textStartTime = now;

			int16 left, top, right, bottom;
			if (endText->clear(*_vm->_draw->_backSurface, left, top, right, bottom))
				_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);

			if (endText->draw(*_vm->_draw->_backSurface, 0, *_plettre, 10, left, top, right, bottom))
				_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);
		}

		drawAnim(anims);
		fadeIn();

		endFrame(true);
	}

	freeAnims(anims);
	delete endText;

	// Restart requested
	if (action == kMenuActionRestart)
		return false;

	// Last scene. Even if we didn't explicitly request a quit, the game ends here
	_quit = true;
	return false;
}

} // End of namespace OnceUpon

} // End of namespace Gob
