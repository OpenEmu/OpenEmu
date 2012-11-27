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

#include "common/textconsole.h"

#include "drascula/drascula.h"

namespace Drascula {

void DrasculaEngine::playTalkSequence(int sequence) {
	bool seen = false;

	for (int i = 0; i < _talkSequencesSize; i++) {
		if (_talkSequences[i].chapter == currentChapter &&
			_talkSequences[i].sequence == sequence) {
			seen = true;

			doTalkSequenceCommand(_talkSequences[i]);
		} else if (seen) // Stop searching down the list
			break;
	}
}

void DrasculaEngine::doTalkSequenceCommand(TalkSequenceCommand cmd) {
	switch (cmd.commandType) {
	case kPause:
		pause(cmd.action);
		break;
	case kSetFlag:
		flags[cmd.action] = 1;
		break;
	case kClearFlag:
		flags[cmd.action] = 0;
		break;
	case kPickObject:
		pickObject(cmd.action);
		break;
	case kAddObject:
		addObject(cmd.action);
		break;
	case kBreakOut:
		breakOut = 1;
		break;
	case kConverse:
		converse(cmd.action);
		break;
	case kPlaceVB:
		placeVonBraun(cmd.action);
		break;
	case kUpdateRoom:
		updateRoom();
		break;
	case kUpdateScreen:
		updateScreen();
		break;
	case kTrackProtagonist:
		trackProtagonist = cmd.action;
		break;
	case kPlaySound:
		playSound(cmd.action);
		break;
	case kFinishSound:
		finishSound();
		break;
	case kTalkerGeneral:
		talk(cmd.action);
		break;
	case kTalkerDrunk:
		talk_drunk(cmd.action);
		break;
	case kTalkerPianist:
		talk_pianist(cmd.action);
		break;
	case kTalkerBJ:
		talk_bj(cmd.action);
		break;
	case kTalkerVBNormal:
		talk_vonBraun(cmd.action, kVonBraunNormal);
		break;
	case kTalkerVBDoor:
		talk_vonBraun(cmd.action, kVonBraunDoor);
		break;
	case kTalkerIgorSeated:
		talk_igor(cmd.action, kIgorSeated);
		break;
	case kTalkerWerewolf:
		talk_werewolf(cmd.action);
		break;
	case kTalkerMus:
		talk_mus(cmd.action);
		break;
	case kTalkerDrascula:
		talk_drascula(cmd.action, 1);
		break;
	case kTalkerBartender0:
		talk_bartender(cmd.action, 0);
		break;
	case kTalkerBartender1:
		talk_bartender(cmd.action, 1);
		break;
	default:
		error("doTalkSequenceCommand: Unknown command: %d", cmd.commandType);
	}
}

void DrasculaEngine::cleanupString(char *string) {
	uint len = strlen(string);
	for (uint h = 0; h < len; h++)
		if (string[h] == (char)0xa7)
			string[h] = ' ';
}

void DrasculaEngine::converse(int index) {
	debug(4, "converse(%d)", index);

	char fileName[20];
	sprintf(fileName, "op_%d.cal", index);
	Common::SeekableReadStream *stream = _archives.open(fileName);
	if (!stream)
		error("missing data file %s", fileName);

	int game1 = kDialogOptionUnselected,
		game2 = kDialogOptionUnselected,
		game3 = kDialogOptionUnselected;
	char phrase1[128], phrase2[128], phrase3[128], phrase4[128];
	char sound1[13], sound2[13], sound3[13], sound4[13];
	int phrase1_bottom, phrase2_bottom, phrase3_bottom, phrase4_bottom;
	int answer1, answer2, answer3;

	breakOut = 0;

	selectVerb(kVerbNone);

	TextResourceParser p(stream, DisposeAfterUse::YES);

	p.parseString(phrase1);
	p.parseString(phrase2);
	p.parseString(phrase3);
	p.parseString(phrase4);
	p.parseString(sound1);
	p.parseString(sound2);
	p.parseString(sound3);
	p.parseString(sound4);
	p.parseInt(answer1);
	p.parseInt(answer2);
	p.parseInt(answer3);

	// no need to delete the stream, since TextResourceParser takes ownership
	// delete stream;


	if (currentChapter == 2 && !strcmp(fileName, "op_5.cal") && flags[38] == 1 && flags[33] == 1) {
		strcpy(phrase3, _text[405]);
		strcpy(sound3, "405.als");
		answer3 = 31;
	}

	if (currentChapter == 6 && !strcmp(fileName, "op_12.cal") && flags[7] == 1) {
		strcpy(phrase3, _text[273]);
		strcpy(sound3, "273.als");
		answer3 = 14;
	}

	if (currentChapter == 6 && !strcmp(fileName, "op_12.cal") && flags[10] == 1) {
		strcpy(phrase3, _text[274]);
		strcpy(sound3, "274.als");
		answer3 = 15;
	}

	cleanupString(phrase1);
	cleanupString(phrase2);
	cleanupString(phrase3);
	cleanupString(phrase4);

	loadPic("car.alg", backSurface);
	// TODO code here should limit y position for mouse in dialog menu,
	// but we can't implement this as there is lack in backend functionality
	// from 1(top) to 31
	color_abc(kColorLightGreen);

	while (breakOut == 0 && !shouldQuit()) {
		updateRoom();

		if (musicStatus() == 0 && roomMusic != 0) {
			if (currentChapter == 3 || currentChapter == 5) {
				playMusic(roomMusic);
			} else {	// chapters 1, 2, 4, 6
				if (flags[11] == 0)
					playMusic(roomMusic);
			}
		}

		updateEvents();
		flushKeyBuffer();

		phrase1_bottom = 8 * print_abc_opc(phrase1, 2, game1);
		phrase2_bottom = phrase1_bottom + 8 * print_abc_opc(phrase2, phrase1_bottom + 2, game2);
		phrase3_bottom = phrase2_bottom + 8 * print_abc_opc(phrase3, phrase2_bottom + 2, game3);
		phrase4_bottom = phrase3_bottom + 8 * print_abc_opc(phrase4, phrase3_bottom + 2, kDialogOptionUnselected);

		if (mouseY > 0 && mouseY < phrase1_bottom) {
			if (game1 == kDialogOptionClicked && _color != kColorWhite)
				color_abc(kColorWhite);
			else if (game1 != kDialogOptionClicked && _color != kColorLightGreen)
				color_abc(kColorLightGreen);

			print_abc_opc(phrase1, 2, kDialogOptionSelected);

			if (leftMouseButton == 1) {
				delay(100);
				game1 = kDialogOptionClicked;
				talk(phrase1, sound1);
				response(answer1);
			}
		} else if (mouseY > phrase1_bottom && mouseY < phrase2_bottom) {
			if (game2 == kDialogOptionClicked && _color != kColorWhite)
				color_abc(kColorWhite);
			else if (game2 != kDialogOptionClicked && _color != kColorLightGreen)
				color_abc(kColorLightGreen);

			print_abc_opc(phrase2, phrase1_bottom + 2, kDialogOptionSelected);

			if (leftMouseButton == 1) {
				delay(100);
				game2 = kDialogOptionClicked;
				talk(phrase2, sound2);
				response(answer2);
			}
		} else if (mouseY > phrase2_bottom && mouseY < phrase3_bottom) {
			if (game3 == kDialogOptionClicked && _color != kColorWhite)
				color_abc(kColorWhite);
			else if (game3 != kDialogOptionClicked && _color != kColorLightGreen)
				color_abc(kColorLightGreen);

			print_abc_opc(phrase3, phrase2_bottom + 2, kDialogOptionSelected);

			if (leftMouseButton == 1) {
				delay(100);
				game3 = kDialogOptionClicked;
				talk(phrase3, sound3);
				response(answer3);
			}
		} else if (mouseY > phrase3_bottom && mouseY < phrase4_bottom) {
			print_abc_opc(phrase4, phrase3_bottom + 2, kDialogOptionSelected);

			if (leftMouseButton == 1) {
				delay(100);
				talk(phrase4, sound4);
				breakOut = 1;
			}
		} else if (_color != kColorLightGreen)
			color_abc(kColorLightGreen);

		_system->delayMillis(10);
		updateScreen();
	} // while (breakOut == 0)

	if (currentChapter == 2)
		loadPic(menuBackground, backSurface);
	else
		loadPic(99, backSurface);
}

void DrasculaEngine::response(int function) {
	debug(4, "response(%d)", function);

	if (function != 31)
		playTalkSequence(function);

	if (currentChapter == 2) {
		bool reloadConversationCharset = false;

		if (function == 16 || function == 20 || function == 23 || function == 29 || function == 31) {
			reloadConversationCharset = true;
			loadPic(menuBackground, backSurface);
		}

		if (function == 16)
			animation_16_2();
		else if (function == 20)
			animation_20_2();
		else if (function == 23)
			animation_23_2();
		else if (function == 29)
			animation_29_2();
		else if (function == 31)
			animation_31_2();

		if (reloadConversationCharset)
			loadPic("car.alg", backSurface);
	} else if (currentChapter == 3) {
		grr();
	}
}

} // End of namespace Drascula
