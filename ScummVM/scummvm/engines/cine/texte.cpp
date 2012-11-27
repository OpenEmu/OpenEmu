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

#include "common/file.h"
#include "common/textconsole.h"

#include "cine/cine.h"
#include "cine/various.h"

namespace Cine {

const char *const *failureMessages;
const CommandeType *defaultActionCommand;
const CommandeType *systemMenu;
const CommandeType *confirmMenu;
const char *const *otherMessages;
const char *defaultCommandPreposition;
const char *const *commandPrepositionTable;

/**
 * Loads font data from the given file.
 * The number of characters used in the font varies between game versions:
 * 78 (Most PC, Amiga and Atari ST versions of Future Wars, but also Operation Stealth's Amiga demo),
 * 85 (All observed versions of German Future Wars (Amiga and PC), possibly Spanish Future Wars too),
 * 90 (Most PC, Amiga and Atari ST versions of Operation Stealth),
 * 93 (All observed versions of German Operation Stealth (Amiga and PC)).
 */
void loadTextData(const char *filename) {
	Common::File fileHandle;
	assert(filename);

	if (!fileHandle.open(filename))
		error("loadTextData(): Cannot open file %s", filename);

	static const uint headerSize = 2 + 2;              // The entry size (16-bit) and entry count (16-bit).
	const uint entrySize = fileHandle.readUint16BE();  // Observed values: 8.
	const uint entryCount = fileHandle.readUint16BE(); // Observed values: 624, 680, 720, 744.
	const uint fontDataSize = entryCount * entrySize;  // Observed values: 4992, 5440, 5760, 5952.
	const uint numChars = entryCount / entrySize;      // Observed values: 78, 85, 90, 93.
	const uint bytesPerChar = fontDataSize / numChars; // Observed values: 64.
	static const uint bytesPerRow = FONT_WIDTH / 2;    // The input font data is 4-bit so it takes only half the space

	if (headerSize + fontDataSize != (uint)fileHandle.size()) {
		warning("loadTextData: file '%s' (entrySize = %d, entryCount = %d) is of incorrect size %d", filename, entrySize, entryCount, fileHandle.size());
	}

	Common::Array<byte> source;
	source.resize(fontDataSize);
	fileHandle.read(source.begin(), fontDataSize);

	if (g_cine->getGameType() == Cine::GType_FW) {
		loadRelatedPalette(filename);
	}

	for (uint i = 0; i < numChars; i++) {
		gfxConvertSpriteToRaw(g_cine->_textHandler.textTable[i][FONT_DATA], &source[i * bytesPerChar], bytesPerRow, FONT_HEIGHT);
		generateMask(g_cine->_textHandler.textTable[i][FONT_DATA], g_cine->_textHandler.textTable[i][FONT_MASK], FONT_WIDTH * FONT_HEIGHT, 0);
	}

	fileHandle.close();
}

static const CharacterEntry fontParamTable_standard[NUM_FONT_CHARS] = {
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, {63, 1}, {69, 5}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, {68, 3},
	{64, 3}, {65, 3}, { 0, 0}, { 0, 0}, {62, 2}, {74, 6}, {66, 1}, {67, 6},
	{52, 6}, {53, 6}, {54, 6}, {55, 6}, {56, 6}, {57, 6}, {58, 6}, {59, 6},
	{60, 6}, {61, 6}, {76, 3}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, {75, 6},
	{ 0, 0}, { 0, 6}, //a
	                  { 1, 6}, { 2, 6}, { 3, 6}, { 4, 6}, { 5, 6}, { 6, 6},
	{ 7, 6}, { 8, 3}, { 9, 6}, {10, 6}, {11, 6}, {12, 7}, {13, 6}, {14, 6},
	{15, 6}, {16, 6}, {17, 6}, {18, 6}, {19, 6}, {20, 6}, {21, 6}, {22, 7},
	{23, 6}, {24, 6}, {25, 6}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, {26, 6}, //a
	                  {27, 6}, {28, 5}, {29, 6}, {30, 6}, {31, 5}, {32, 6},
	{33, 6}, {34, 4}, {35, 4}, {36, 5}, {37, 3}, {38, 7}, {39, 6}, {40, 6},
	{41, 6}, {42, 6}, {43, 6}, {44, 6}, {45, 6}, {46, 6}, {47, 6}, {48, 7},
	{49, 6}, {50, 6}, {51, 6}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, {70, 6}, { 0, 0}, { 0, 0}, {72, 6}, { 0, 0}, {73, 5},
	{77, 6}, { 0, 0}, {71, 6}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, {77, 6},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}
};

static const CharacterEntry fontParamTable_alt[NUM_FONT_CHARS] = {
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, {63, 1}, {69, 5}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, {68, 3},
	{64, 3}, {65, 3}, { 0, 0}, { 0, 0}, {62, 2}, {74, 6}, {66, 1}, {67, 6},
	{52, 6}, {53, 6}, {54, 6}, {55, 6}, {56, 6}, {57, 6}, {58, 6}, {59, 6},
	{60, 6}, {61, 6}, {76, 3}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, {75, 6},
	{ 0, 0}, { 0, 6}, { 1, 6}, { 2, 6}, { 3, 6}, { 4, 6}, { 5, 6}, { 6, 6},
	{ 7, 6}, { 8, 3}, { 9, 6}, {10, 6}, {11, 6}, {12, 7}, {13, 6}, {14, 6},
	{15, 6}, {16, 6}, {17, 6}, {18, 6}, {19, 6}, {20, 6}, {21, 6}, {22, 7},
	{23, 6}, {24, 6}, {25, 6}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, {26, 6}, {27, 6}, {28, 5}, {29, 6}, {30, 6}, {31, 5}, {32, 6},
	{33, 6}, {34, 4}, {35, 4}, {36, 5}, {37, 3}, {38, 7}, {39, 6}, {40, 6},
	{41, 6}, {42, 6}, {43, 6}, {44, 6}, {45, 6}, {46, 6}, {47, 6}, {48, 7},
	{49, 6}, {50, 6}, {51, 6}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, {82, 6}, {70, 6}, { 0, 0}, {78, 6}, {72, 6}, { 0, 0}, {73, 5},
	{77, 6}, {79, 6}, {71, 6}, {80, 4}, { 0, 0}, { 0, 0}, {78, 6}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, {81, 6}, { 0, 0}, { 0, 0}, {77, 6},
	{83, 6}, {81, 6}, {82, 6}, { 0, 0}, { 0, 0}, { 0, 0}, {84, 6}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, {84, 6}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0},
	{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}
};

void initLanguage(Common::Language lang) {
	static const char *const failureMessages_EN[] = {
		// EXAMINE
		"I don't see anything unusual.",
		"There's nothing of interest here.",
		"This isn't particularly interesting.",
		"You won't find anything.",
		// TAKE
		"I can't take that.",
		"I find it difficult.",
		"I don't see what I am supposed to take.",
		"I have difficulty in following you.",
		// INVENTORY
		"There's no point.",
		"You have better things to do.",
		"Come on, don't let's waste any time.",
		"That doesn't seem to me to be a good idea.",
		// USE
		"I don't see why I should do that.",
		"It's had no effect whatsoever.",
		"It won't produce any results.",
		"Try and find something else.",
		// OPERATE
		"It doesn't work.",
		"Let suppose you are trying and don't let's mention it again.",
		"Nothing happens.",
		"You have better things to do.",
		// SPEAK
		"No answer.",
		"More action , less talking !",
		"I'd be very surprised if you got an answer",
		"A wall of silence ..."
	};

	static const CommandeType defaultActionCommand_EN[] = {
		"EXAMINE",
		"TAKE",
		"INVENTORY",
		"USE",
		"OPERATE",
		"SPEAK",
		"NOACTION"
	};

	static const char *const commandPrepositionTable_EN[] = {
		"",   // EXAMINE
		"",   // TAKE
		"",   // INVENTORY
		"on", // USE
		"",   // OPERATE
		"to", // SPEAK
		""    // NOACTION
	};

	static const CommandeType systemMenu_EN[] = {
		"Pause",
		"Restart Game",
		"Quit",
		"Backup Drive is A:",
		"Restore game",
		"Save game"
	};

	static const char *const otherMessages_EN[] = {
		"This backup doesn't exist ...",
		"Could not create save file ...",
		"PAUSE",
		"Loading | %s",
		"Loading canceled ...",
		"No backup in the drive...",
		"Please enter the backup name"
	};

	static const CommandeType confirmMenu_EN[] = {
		"Ok, go ahead ...",
		"Absolutely Not!"
	};

	static const char *const failureMessages_FR[] = {
		// EXAMINER
		"Je ne vois rien de special.",
		"Il n'y a rien d'int\x82ressant.",
		"Cela pr\x82sente peu d'int\x82r\x88ts.",
		"Vous ne trouvez rien.",
		// PRENDRE
		"Je ne peux pas prendre cela.",
		"Cela me semble difficile",
		"Je ne vois pas ce qu'il y a \x85 prendre",
		"j'ai du mal \x85 vous suivre.",
		// INVENTAIRE
		"C'est inutile",
		"Vous avez mieux \x85 faire",
		"Allons, ne perdons pas de temps",
		"\x87""a ne me semble pas \x88tre une bonne id\x82""e",
		// UTILISER
		"Je ne vois pas pourquoi je ferais cela.",
		"C'est absolument sans effets",
		"Cela n'amenerait \x85 rien",
		"Essayez de trouver autre chose.",
		// ACTIONNER
		"Ca ne marche pas",
		"Supposons que vous essayez et n'en parlons plus.",
		"Rien n'y fait.",
		"Vous avez mieux \x85 faire.",
		// PARLER
		"Vous lui parlez . Sans r\x82ponse.",
		"Plus d'actes et moins de Paroles !",
		"Je serais bien surpris si vous obteniez une r\x82ponse.",
		"Un mur de silence ..."
	};

	static const CommandeType defaultActionCommand_FR[] = {
		"EXAMINER",
		"PRENDRE",
		"INVENTAIRE",
		"UTILISER",
		"ACTIONNER",
		"PARLER",
		"NOACTION"
	};

	static const char *const commandPrepositionTable_FR[] = {
		"",    // EXAMINER
		"",    // PRENDRE
		"",    // INVENTAIRE
		"sur", // UTILISER
		"",    // ACTIONNER
		"a",   // PARLER
		""     // NOACTION
	};

	static const CommandeType systemMenu_FR[] = {
		"Pause",
		"Nouvelle partie",
		"Quitter",
		"Lecteur de Svg. A:",
		"Charger une partie",
		"Sauver la partie"
	};

	static const CommandeType confirmMenu_FR[] = {
		"Ok , Vas-y ...",
		"Surtout Pas !"
	};

	static const char *const otherMessages_FR[] = {
		"Cette sauvegarde n'existe pas ...",
		"Could not create save file ...", //
		"PAUSE",
		"Sauvegarde de | %s",
		"Sauvegarde Annul\x82""e ...",
		"Aucune sauvegarde dans le lecteur ...",
		"Veuillez entrer le Nom de la Sauvegarde ."
	};

	static const char *const failureMessages_ES[] = {
		// EXAMINE
		"No veo nada especial",
		"No hay nada interesante",
		"No tiene mucho interes",
		"No encuentras nada",
		// TAKE
		"No puedo coger eso",
		"Eso parece dificil",
		"No veo nada mas para coger",
		"No he debido entenderte",
		// INVENTORY
		"Es inutil",
		"Tienes algo mejor que hacer",
		"Vamos. No perdamos tiempo",
		"Esa no es una buena idea",
		// USE
		"No veo porque hacerlo",
		"No ha tenido efecto",
		"Eso no arreglara nada",
		"Intenta encontrar otra cosa",
		// OPERATE
		"Eso no funciona",
		"Suponfamos que pruebas y no hablamos mas",
		"Nada ha pasado",
		"Tienes cosas mejores que hacer",
		// SPEAK
		"Le hablas. Sin respuesta.",
		"Menos hablar y mas trabajar",
		"Me sorprenderia si tuvieras otra repuesta",
		"Un muro de silencio ..."
	};

	static const CommandeType defaultActionCommand_ES[] = {
		"EXAMINAR",
		"COGER",
		"INVENTARIO",
		"USAR",
		"ACCIONAR",
		"HABLAR",
		"NOACTION"
	};

	static const char *const commandPrepositionTable_ES[] = {
		"",      // EXAMINAR
		"",      // COGER
		"",      // INVENTARIO
		"donde", // USAR
		"",      // ACCIONAR
		"a",     // HABLAR
		""       // NOACTION
	};

	static const CommandeType systemMenu_ES[] = {
		"Pause",
		"Nueva partida",
		"Abandonar",
		"Unidad grabar.  A:",
		"Cargar una partida",
		"Salvar la partida"
	};

	static const CommandeType confirmMenu_ES[] = {
		"Ok , Vas a ...",
		"Nade de nada !"
	};

	static const char *const otherMessages_ES[] = {
		"Esta granacion no existe",
		"Could not create save file ...", //
		"PAUSE",
		"Gabacion de| %s",
		"Rrabacion anulada",
		"No hay partidas grabadas en este disco...",
		"Teclea el nombre de la partida grabada"
	};

	static const char *const failureMessages_DE[] = {
		// EXAMINE
		"Ich sehe nichts Besonderes",
		"Es gibt hier nichts Interessantes",
		"Das ist nicht besonders interessant",
		"Sie werden nichts finden",
		// TAKE
		"Ich Kann das nicht nehmen",
		"Das finde ich schwierig'",
		"Ich wei\x9e nicht, was ich nehmen soll",
		"Ich kann Ihnen nicht folgen",
		// INVENTORY
		"Das bringt nichts",
		"Sie haben wirklich was Besseres zu tun",
		"Los, wir sollten keine Zeit verschwenden",
		"Das scheint mir eine gute Idee zu sein",
		// USE
		"Ich wei\x9e nicht, warum ich das tun soll",
		"Es hat so oder so nichts begracht",
		"Davon haben wir nichts",
		"Versuchen Sie, etwas anderes zu finden",
		// OPERATE
		"Es geht nicht",
		"Sagen wir, das war ein Versuch, und reden wir nicht mehr dr\x81""ber",
		"Nichts passiert",
		"Sie haben wirklich was Besseres zu tun",
		// SPEAK
		"Sie sprechen m it ihm. Keine Antwort",
		"Nicht reden, sondern handeln!",
		"Wenn Sie eine Antwork bek\x84men, w\x81rde es mich sehr wundern",
		"Eine Wand des Schweigens..."
	};

	static const CommandeType defaultActionCommand_DE[] = {
		"Pr\x81""fe", // FIXME? The third letter should be Latin Small Letter U with diaeresis
		"Nimm",
		"Bestand",
		"Benutze",
		"Bet\x84tige", // FIXME? The fourth letter should be Latin Small Letter A with diaeresis
		"Sprich",
		"NOACTION"
	};

	static const char *const commandPrepositionTable_DE[] = {
		"",      // Prufe
		"",      // Nimm
		"",      // Bestand
		"gegen", // Benutze
		"",      // Betatige
		"a",     // Sprich
		""       // NOACTION
	};

	static const CommandeType systemMenu_DE[] = {
		"Pause",
		"Spiel Neu Starten",
		"Lassen",
		"Backuplaufwerk A:",
		"Spiel Laden",
		"Spiel Speichern"
	};

	static const CommandeType confirmMenu_DE[] = {
		"Gut, Weitermachen",
		"Absolut Nicht!"
	};

	static const char *const otherMessages_DE[] = {
		"Diese Sicherungskopie gibt es nicht",
		"Could not create save file ...", //
		"PAUSE",
		"Er L\x84""dt | %s",
		"Ladevorgang Abgebrochen...",
		"Kein Backup im Laufwerk...",
		"Geben Sie den Namen|der Sicherungsdiskette ein"
	};

	static const char *const failureMessages_IT[] = {
		// EXAMINE
		"Non vedo nula di speciale",
		"Non c'\x8a niente di interessante",
		"E' di poco interesse",
		"Non trovate nulla",
		// TAKE
		"Non poso prendere quello",
		"Quello mi sembra difficile",
		"Non vedo cosa ci sia da prendere",
		"Faccio fatica a seguirvi",
		// INVENTORY
		"E' inutile",
		"Avete di meglio da fare",
		"Allora, no perdiamo tempo",
		"Non mi pare che sia una buona idea",
		// USE
		"Non vedo perch\x82 dovrei farlo",
		"E' assolutamente privo di effetti",
		"Cio non portera a nulla",
		"Provate a trovare qualcosa d'altro",
		// OPERATE
		"Non funziona",
		"Supponiamo che voi proviate e non ne parliamo piu",
		"Niente di fatto",
		"Avete di meglio da fare",
		// SPEAK
		"Gli parlate. Senza risposta",
		"Piu fatti e meno parole",
		"Sarei sorpreso se voi otterreste una risposta",
		"Un muro di silenzio ..."
	};

	static const CommandeType defaultActionCommand_IT[] = {
		"ESAMINARE",
		"PRENDERE",
		"INVENTARIO",
		"UTILIZZARE",
		"AZIONARE",
		"PARLARE",
		"NOACTION"
	};

	static const char *const commandPrepositionTable_IT[] = {
		"",   // ESAMINARE
		"",   // PRENDERE
		"",   // INVENTARIO
		"su", // UTILIZZARE
		"",   // AZIONARE
		"a",  // PARLARE
		""    // NOACTION
	};

	static const CommandeType systemMenu_IT[] = {
		"Pausa",
		"Parte nuova",
		"Quit",
		"Drive di svg. A:",
		"Caricare una parte",
		"Salvare una parte"
	};

	static const CommandeType confirmMenu_IT[] = {
		"Ok, vacci ...",
		"Supratutto non!"
	};

	static const char *const otherMessages_IT[] = {
		"Questo salvataggio non esiste...",
		"Could not create save file ...", //
		"PAUSE",
		"Caricamento di| %s",
		"Caricamento annullato...",
		"Nessun salvataggio su questo disco...",
		"Vogliate accedere con il nome del salvataggio"
	};

	switch (lang) {
	case Common::FR_FRA:
		failureMessages = failureMessages_FR;
		defaultActionCommand = defaultActionCommand_FR;
		systemMenu = systemMenu_FR;
		confirmMenu = confirmMenu_FR;
		otherMessages = otherMessages_FR;
		defaultCommandPreposition = commandPrepositionTable_FR[3];
		commandPrepositionTable = commandPrepositionTable_FR;
		break;

	case Common::ES_ESP:
		failureMessages = failureMessages_ES;
		defaultActionCommand = defaultActionCommand_ES;
		systemMenu = systemMenu_ES;
		confirmMenu = confirmMenu_ES;
		otherMessages = otherMessages_ES;
		defaultCommandPreposition = commandPrepositionTable_ES[3];
		commandPrepositionTable = commandPrepositionTable_ES;
		break;

	case Common::DE_DEU:
		failureMessages = failureMessages_DE;
		defaultActionCommand = defaultActionCommand_DE;
		systemMenu = systemMenu_DE;
		confirmMenu = confirmMenu_DE;
		otherMessages = otherMessages_DE;
		defaultCommandPreposition = commandPrepositionTable_DE[3];
		commandPrepositionTable = commandPrepositionTable_DE;
		break;

	case Common::IT_ITA:
		failureMessages = failureMessages_IT;
		defaultActionCommand = defaultActionCommand_IT;
		systemMenu = systemMenu_IT;
		confirmMenu = confirmMenu_IT;
		otherMessages = otherMessages_IT;
		defaultCommandPreposition = commandPrepositionTable_IT[3];
		commandPrepositionTable = commandPrepositionTable_IT;
		break;

	default:
		failureMessages = failureMessages_EN;
		defaultActionCommand = defaultActionCommand_EN;
		systemMenu = systemMenu_EN;
		confirmMenu = confirmMenu_EN;
		otherMessages = otherMessages_EN;
		defaultCommandPreposition = commandPrepositionTable_EN[3];
		commandPrepositionTable = commandPrepositionTable_EN;
		break;
	}

	if (g_cine->getFeatures() & GF_ALT_FONT) {
		// Copy alternative font parameter table to the current font parameter table
		Common::copy(fontParamTable_alt, fontParamTable_alt + NUM_FONT_CHARS, g_cine->_textHandler.fontParamTable);
	} else {
		// Copy standard font parameter to the current font parameter table
		Common::copy(fontParamTable_standard, fontParamTable_standard + NUM_FONT_CHARS, g_cine->_textHandler.fontParamTable);
	}
}

void loadErrmessDat(const char *fname) {
	Common::File in;

	in.open(fname);

	if (in.isOpen()) {
		// FIXME - This can leak in some situations in Operation Stealth
		//         Engine Restart - multiple allocations with no free?
		char **ptr = (char **)malloc(sizeof(char *) * 6 * 4 + 60 * 6 * 4);

		for (int i = 0; i < 6 * 4; i++) {
			ptr[i] = (char *)ptr + (sizeof(char *) * 6 * 4) + 60 * i;
			in.read(ptr[i], 60);
		}
		failureMessages = const_cast<const char *const *>(ptr);

		in.close();
	} else {
		error("Cannot open file %s for reading", fname);
	}
}

void freeErrmessDat() {
	free(const_cast<const char **>(failureMessages));
	failureMessages = 0;
}

void loadPoldatDat(const char *fname) {
	Common::File in;

	in.open(fname);

	if (in.isOpen()) {
		for (int i = 0; i < NUM_FONT_CHARS; i++) {
			g_cine->_textHandler.fontParamTable[i].characterIdx   = in.readByte();
			g_cine->_textHandler.fontParamTable[i].characterWidth = in.readByte();
		}
		in.close();
	} else {
		error("Cannot open file %s for reading", fname);
	}
}

/**
 * Fit a substring of text into one line of fixed width text box
 * @param str Text to fit
 * @param maxWidth Text box width
 * @param[out] words Number of words that fit
 * @param[out] width Total width of nonblank characters that fit
 * @return Length of substring which fits
 */
int fitLine(const char *str, int maxWidth, int &words, int &width) {
	int i, bkpWords = 0, bkpWidth = 0, bkpLen = 0;
	int charWidth = 0, fullWidth = 0;

	words = 0;
	width = 0;

	for (i = 0; str[i]; i++) {
		if (str[i] == 0x7C) {
			i++;
			break;
		} else if (str[i] == ' ') {
			charWidth = 5;
			bkpWords = words++;
			bkpWidth = width;
			bkpLen = i + 1;
		} else {
			charWidth = g_cine->_textHandler.fontParamTable[(unsigned char)str[i]].characterWidth + 1;
			width += charWidth;
		}

		if (!charWidth) {
			continue;
		}

		if (fullWidth + charWidth < maxWidth) {
			fullWidth += charWidth;
		} else if (fullWidth) {
			words = bkpWords;
			width = bkpWidth;
			i = bkpLen;
			break;
		}
	}

	return i;
}

} // End of namespace Cine
