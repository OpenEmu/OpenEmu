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

#include "drascula/drascula.h"

namespace Drascula {

void DrasculaEngine::updateAnim(int y, int destX, int destY, int width, int height, int count, byte* src, int delayVal, bool copyRectangle) {
	int x = 0;

	for (int n = 0; n < count; n++){
		x++;
		if (copyRectangle) {
			copyBackground(destX, destY, destX, destY, width, height, bgSurface, screenSurface);
			copyRect(x, y, destX, destY, width, height, src, screenSurface);
		} else {
			copyBackground(x, y, destX, destY, width, height, src, screenSurface);
		}
		updateScreen(destX, destY, destX, destY, width, height, screenSurface);
		x += width;
		updateEvents();
		pause(delayVal);
	}
}

// This is the game's introduction sequence
void DrasculaEngine::animation_1_1() {
	debug(4, "animation_1_1()");

	int l, l2, p;
	//int pixelPos[6];

	while (term_int == 0 && !shouldQuit()) {
		playMusic(29);
		playFLI("logoddm.bin", 9);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		delay(600);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		clearRoom();
		delay(340);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		playMusic(26);
		delay(500);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		playFLI("logoalc.bin", 8);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		clearRoom();
		loadPic("cielo.alg", screenSurface, COMPLETE_PAL);
		black();
		updateScreen();
		fadeFromBlack(2);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		delay(900);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		color_abc(kColorRed);
		centerText(_textmisc[1], 160, 100);
		updateScreen();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		delay(1000);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		delay(1200);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;

		playFLI("scrollb.bin", 9);

		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		clearRoom();
		playSound(5);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		if (animate("scr2.bin", 17))
			break;
		stopSound();
		if (animate("scr3.bin", 17))
			break;
		loadPic("cielo2.alg", screenSurface, COMPLETE_PAL);
		updateScreen();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		fadeToBlack(1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		clearRoom();

		loadPic(96, frontSurface, COMPLETE_PAL);
		loadPic(103, bgSurface, HALF_PAL);
		loadPic(104, drawSurface3);
		loadPic("aux104.alg", drawSurface2);

		playMusic(4);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		delay(400);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;

		for (l2 = 0; l2 < 3; l2++)
			for (l = 0; l < 7; l++) {
				copyBackground();
				copyBackground(interf_x[l], interf_y[l], 156, 45, 63, 31, drawSurface2, screenSurface);
				updateScreen();
				if (getScan() == Common::KEYCODE_ESCAPE || shouldQuit()) {
					term_int = 1;
					break;
				}
				pause(3);
			}
			if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
				break;

		l2 = 0; p = 0;

		for (l = 0; l < 180; l++) {
			copyBackground(0, 0, 320 - l, 0, l, 200, drawSurface3, screenSurface);
			copyBackground(l, 0, 0, 0, 320 - l, 200, bgSurface, screenSurface);

			copyRect(interf_x[l2], interf_y[l2], 156 - l, 45, 63, 31, drawSurface2, screenSurface);
			updateScreen();
			updateEvents();
			p++;
			if (p == 6) {
				p = 0;
				l2++;
			}
			if (l2 == 7)
				l2 = 0;
			if (getScan() == Common::KEYCODE_ESCAPE  || shouldQuit()) {
				term_int = 1;
				break;
			}
		}
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		copyBackground(0, 0, 0, 0, 320, 200, screenSurface, bgSurface);

		talk_drascula_big(1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;

		clearRoom();

		loadPic(100, bgSurface, HALF_PAL);
		loadPic("auxigor.alg", frontSurface);
		loadPic("auxdr.alg", backSurface);
		trackDrascula = 0;
		drasculaX = 129;
		drasculaY = 95;
		trackIgor = 1;
		igorX = 66;
		igorY = 97;

		copyBackground();
		placeIgor();
		placeDrascula();
		updateScreen();
		talk_igor(8, kIgorDch);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		copyBackground();
		placeIgor();
		placeDrascula();
		updateScreen();
		talk_drascula(2);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		talk_drascula(3);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		if (animate("lib.bin", 16))
			break;
		if (animate("lib2.bin", 16))
			break;
		clearRoom();
		color_solo = kColorRed;
		loadPic("plan1.alg", screenSurface, HALF_PAL);
		updateScreen();
		pause(10);
		talk_solo(_textd[4],"d4.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		loadPic("plan1.alg", screenSurface, HALF_PAL);
		updateScreen();
		talk_solo(_textd[5], "d5.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		if (animate("lib2.bin", 16))
			break;
		clearRoom();
		loadPic("plan2.alg", screenSurface, HALF_PAL);
		updateScreen();
		pause(20);
		talk_solo(_textd[6], "d6.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		if (animate("lib2.bin", 16))
			break;
		clearRoom();
		loadPic("plan3.alg", screenSurface, HALF_PAL);
		updateScreen();
		pause(20);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		talk_solo(_textd[7], "d7.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		loadPic("plan3.alg", screenSurface, HALF_PAL);
		updateScreen();
		talk_solo(_textd[8], "d8.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		clearRoom();
		loadPic(100, bgSurface, HALF_PAL);
		MusicFadeout();
		stopMusic();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		talk_igor(9, kIgorDch);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		talk_drascula(9);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		talk_igor(10, kIgorDch);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		playMusic(11);
		talk_drascula(10);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		if (animate("rayo1.bin", 16))
			break;
		playSound(5);
		if (animate("rayo2.bin", 15))
			break;
		if (animate("frel2.bin", 16))
			break;
		if (animate("frel.bin", 16))
			break;
		if (animate("frel.bin", 16))
			break;
		stopSound();
		clearRoom();
		black();
		playMusic(23);
		fadeFromBlack(0);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		trackDrascula = 1;
		talk_igor(1, kIgorDch);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		talk_drascula(11, 1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		trackDrascula = 3;
		copyBackground();
		placeIgor();
		placeDrascula();
		updateScreen();
		pause(1);
		trackDrascula = 0;
		copyBackground();
		placeIgor();
		placeDrascula();
		updateScreen();
		talk_drascula(12);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		trackDrascula = 3;
		copyBackground();
		placeIgor();
		placeDrascula();
		updateScreen();
		pause(1);
		trackDrascula = 1;
		copyBackground();
		placeIgor();
		placeDrascula();
		updateScreen();
		talk_igor(2, kIgorDch);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		pause(13);
		talk_drascula(13, 1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		trackDrascula = 3;
		copyBackground();
		placeIgor();
		placeDrascula();
		updateScreen();
		pause(1);
		trackDrascula = 0;
		copyBackground();
		placeIgor();
		placeDrascula();
		updateScreen();
		talk_drascula(14);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		talk_igor(3, kIgorDch);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		talk_drascula(15);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		talk_igor(4, kIgorDch);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		talk_drascula(16);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		talk_igor(5, kIgorDch);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		trackIgor = 3;
		talk_drascula(17);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		pause(18);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		talk_igor(6, kIgorFront);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		fadeToBlack(0);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		clearRoom();

		if (_lang == kSpanish)
			playMusic(31);
		else
			playMusic(2);

		pause(5);
		playFLI("intro.bin", 12);
		term_int = 1;
	}
	clearRoom();
	loadPic(96, frontSurface, COMPLETE_PAL);
	loadPic(99, backSurface);
}

// John falls in love with BJ, who is then abducted by Drascula
void DrasculaEngine::animation_2_1() {
	debug(4, "animation_2_1()");

	int l;

	gotoObject(231, 91);
	hare_se_ve = 0;

	term_int = 0;

	while (!shouldQuit()) {
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;

		if (animate("ag.bin", 14))
			break;

		loadPic("an11y13.alg", extraSurface);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;

		talk_bartender(22);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;

		loadPic(97, extraSurface);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;

		pause(4);
		playSound(1);
		hiccup(18);
		finishSound();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;

		clearRoom();
		stopMusic();
		musicStopped = 1;
		memset(screenSurface, 0, 64000);
		color_solo = kColorWhite;
		pause(80);

		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		talk_solo(_textbj[1], "BJ1.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		clearRoom();
		loadPic("bj.alg", screenSurface, HALF_PAL);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		black();
		updateScreen();
		fadeFromBlack(1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		color_solo = kColorYellow;
		talk_solo(_text[214], "214.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		clearRoom();

		loadPic(16, bgSurface, HALF_PAL);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		loadPic("auxbj.alg", drawSurface3);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;

		roomNumber = 16;

		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		for (l = 0; l < 200; l++)
			factor_red[l] = 99;
		bjX = 170;
		bjY = 90;
		trackBJ = 0;
		curX = 91;
		curY = 95;
		trackProtagonist = 1;
		hare_se_ve = 1;
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;

		loadPic("97g.alg", extraSurface);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;

		if (animate("lev.bin", 15))
			break;

		gotoObject(100 + curWidth / 2, 99 + curHeight);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		trackProtagonist = 1;
		curX = 100;
		curY = 95;

		playTalkSequence(2);	// sequence 2, chapter 1

		if (animate("gaf.bin", 15))
			break;
		if (animate("bjb.bin", 14))
			break;
		playMusic(9);
		loadPic(97, extraSurface);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		updateRoom();
		updateScreen();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		pause(120);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		talk_solo(_text[223], "223.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		color_solo = kColorWhite;
		updateRoom();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		updateScreen();
		pause(110);
		talk_solo(_textbj[11], "BJ11.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		updateRoom();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		updateScreen();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		pause(118);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		gotoObject(132, 97 + curHeight);
		pause(60);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		talk(224);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		talk_bj(12);
		gotoObject(157, 98 + curHeight);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		if (animate("bes.bin", 16))
			break;
		playMusic(11);
		if (animate("rap.bin", 16))
			break;
		trackProtagonist = 3;
		// The room number was originally changed here to "no_bj.alg",
		// which doesn't exist. In reality, this was just a hack to
		// set the room number to a non-existant one, so that BJ does
		// not appear again when the room is refreshed after the
		// animation where Count Drascula abducts her. We set the
		// room number to -1 for the same purpose
		// Also check animation_9_6(), where the same hack was used by
		// the original
		roomNumber = -1;
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		pause(8);
		updateRoom();
		updateScreen();
		talk(225);
		pause(76);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		trackProtagonist = 1;
		updateRoom();
		updateScreen();
		talk(226);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		updateRoom();
		updateScreen();
		pause(30);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE) || shouldQuit())
			break;
		talk(227);
		fadeToBlack(0);
		break;
	}
}

// John Hacker talks with the bartender to book a room
void DrasculaEngine::animation_3_1() {
	debug(4, "animation_3_1()");

	loadPic("an11y13.alg", extraSurface);

	playTalkSequence(3);	// sequence 3, chapter 1

	loadPic(97, extraSurface);
}

// John Hacker talks with the pianist
void DrasculaEngine::animation_4_1() {
	debug(4, "animation_4_1()");

	loadPic("an12.alg", extraSurface);

	talk(205);

	updateRefresh_pre();

	copyBackground(1, 139, 228, 112, 47, 60, extraSurface, screenSurface);
	updateScreen(228,112, 228,112, 47,60, screenSurface);

	pause(3);

	updateRefresh_pre();

	copyBackground(49, 139, 228, 112, 47, 60, extraSurface, screenSurface);
	moveCharacters();

	updateScreen();

	pause(3);
	stopMusic();
	flags[11] = 1;

	talk_pianist(1);
	talk(206);
	talk_pianist(2);
	talk(207);
	talk_pianist(3);
	talk(208);
	talk_pianist(4);
	talk(209);

	flags[11] = 0;
	loadPic(97, extraSurface);
}

void DrasculaEngine::animation_2_2() {
	debug(4, "animation_2_2()");

	trackProtagonist = 0;
	copyBackground();
	moveCharacters();
	updateRefresh();
	updateScreen();
	loadPic("an2_1.alg", frontSurface);
	loadPic("an2_2.alg", extraSurface);

	copyBackground();
	copyBackground(1, 1, 201, 87, 50, 52, frontSurface, screenSurface);
	updateScreen();

	updateAnim(1, 201, 87, 50, 52, 6, frontSurface);
	updateAnim(55, 201, 87, 50, 52, 6, frontSurface);
	updateAnim(109, 201, 87, 50, 52, 6, frontSurface);

	playSound(2);

	updateAnim(1, 201, 87, 50, 52, 6, extraSurface);
	updateAnim(55, 201, 87, 50, 52, 6, extraSurface);
	updateAnim(109, 201, 87, 50, 52, 2, extraSurface);

	copyBackground();
	updateScreen();

	finishSound();

	pause (4);

	playSound(1);
	hipo_sin_nadie(12);
	finishSound();
}

void DrasculaEngine::animation_4_2() {
	debug(4, "animation_4_2()");

	stopMusic();
	flags[9] = 1;

	pause(12);
	talk(60);
	pause(8);

	clearRoom();
	loadPic("ciego1.alg", bgSurface, HALF_PAL);	// ciego = blind
	loadPic("ciego2.alg", drawSurface3);
	loadPic("ciego3.alg", extraSurface);
	loadPic("ciego4.alg", backSurface);
	loadPic("ciego5.alg", frontSurface);

	copyBackground();
	updateScreen();

	pause(10);

	talk_blind(1);
	pause(5);
	talk_hacker(57);
	pause(6);
	talk_blind(2);
	pause(4);
	talk_hacker(58);
	talk_blind(3);
	delay(14);
	talk_hacker(59);
	talk_blind(4);
	talk_hacker(60);
	talk_blind(5);
	talk_hacker(61);
	talk_blind(6);
	talk_hacker(62);
	talk_blind(7);
	talk_hacker(63);
	talk_blind(8);
	copyBackground();
	updateScreen();
	_system->delayMillis(1000);
	talk_hacker(64);
	talk_blind(9);

	copyBackground();
	updateScreen();

	pause(14);

	clearRoom();

	playMusic(roomMusic);
	loadPic(9, bgSurface, HALF_PAL);
	loadPic("aux9.alg", drawSurface3);
	loadPic(96, frontSurface);
	loadPic(97, extraSurface);
	loadPic(99, backSurface);
	selectVerb(kVerbNone);

	flags[9] = 0;
	flags[4] = 1;
}

void DrasculaEngine::animation_14_2() {
	debug(4, "animation_14_2()");

	int cY = -160;
	int l = 0;

	loadPic("an14_2.alg", backSurface);

	for (int n = -160; n <= 0; n = n + 5 + l) {
		copyBackground();
		updateRefresh_pre();
		moveCharacters();
		moveVonBraun();
		cY = n;
		copyRect(150, 6, 69, cY, 158, 161, backSurface, screenSurface);
		updateRefresh();
		updateScreen();
		l++;
	}

	flags[24] = 1;

	memcpy(bgSurface, screenSurface, 64000);

	playSound(7);
	hiccup(15);

	finishSound();

	loadPic(99, backSurface);
}

void DrasculaEngine::asco() {
	loadPic(roomDisk, drawSurface3);
	loadPic(roomNumber, bgSurface, HALF_PAL);
	black();
	updateRoom();
	updateScreen();
	fadeFromBlack(0);
	if (roomMusic != 0)
		playMusic(roomMusic);
	else
		stopMusic();
}

// The drunk tells us about Von Braun
void DrasculaEngine::animation_16_2() {
	debug(4, "animation_16_2()");

	char curPic[20];
	talk_drunk(12);
	talk(371);

	clearRoom();

	if (_lang == kSpanish)
		playMusic(30);
	else
		playMusic(32);

	if (getScan() != 0) {
		asco();
		return;
	}

	color_abc(kColorDarkGreen);

	for (int i = 1; i <= 4; i++) {
		if (i < 4)
			sprintf(curPic, "his%i.alg", i);
		else
			strcpy(curPic, "his4_2.alg");

		loadPic(curPic, screenSurface, HALF_PAL);
		centerText(_texthis[i], 180, 180);
		updateScreen();

		if (getScan() != 0) {
			asco();
			return;
		}

		delay(3000);

		if (i < 4) {
			fadeToBlack(1);

			if (getScan() != 0) {
				asco();
				return;
			}

			clearRoom();
		}
	}

	loadPic("his4_1.alg", bgSurface, HALF_PAL);
	loadPic("his4_2.alg", drawSurface3);

	for (int l = 1; l < 200; l++) {
		copyBackground(0, 0, 0, l, 320, 200 - l, drawSurface3, screenSurface);
		copyBackground(0, 200 - l, 0, 0, 320, l, bgSurface, screenSurface);
		updateScreen();
		if (getScan() != 0) {
			asco();
			return;
		}
	}

	pause(5);
	fadeToBlack(2);
	clearRoom();

	asco();
}

void DrasculaEngine::animation_20_2() {
	debug(4, "animation_20_2()");

	talk_vonBraun(7, kVonBraunDoor);
	talk_vonBraun(8, kVonBraunDoor);
	talk(383);
	talk_vonBraun(9, kVonBraunDoor);
	talk(384);
	talk_vonBraun(10, kVonBraunDoor);
	talk(385);
	talk_vonBraun(11, kVonBraunDoor);
	if (flags[23] == 0) {
		talk(350);
	talk_vonBraun(57, kVonBraunDoor);
	} else {
		talk(386);
		talk_vonBraun(12, kVonBraunDoor);
		flags[18] = 0;
		flags[14] = 1;
		toggleDoor(15, 1, kOpenDoor);
		exitRoom(1);
		animation_23_2();
		exitRoom(0);
		flags[21] = 0;
		flags[24] = 0;
		trackVonBraun = 1;
		vonBraunX = 120;

		breakOut = 1;
	}
}

void DrasculaEngine::animation_23_2() {
	debug(4, "animation_23_2()");

	loadPic("an24.alg", frontSurface);

	flags[21] = 1;

	if (flags[25] == 0) {
		talk_vonBraun(13, kVonBraunDoor);
		talk_vonBraun(14, kVonBraunDoor);
		pause(10);
		talk(387);
	}

	talk_vonBraun(15, kVonBraunNormal);
	placeVonBraun(42);
	trackVonBraun = 1;
	talk_vonBraun(16, kVonBraunNormal);
	trackVonBraun = 2;
	gotoObject(157, 147);
	gotoObject(131, 149);
	trackProtagonist = 0;
	animation_14_2();
	if (flags[25] == 0)
		talk_vonBraun(17, kVonBraunNormal);
	pause(8);
	trackVonBraun = 1;
	talk_vonBraun(18, kVonBraunNormal);

	if (flags[29] == 0)
		animation_23_joined();
	else
		animation_23_joined2();

	trackVonBraun = 2;
	animation_25_2();
	placeVonBraun(99);

	if (flags[29] == 0) {
		talk_vonBraun(19, kVonBraunNormal);
		if (flags[25] == 0) {
			talk_vonBraun(20, kVonBraunNormal);
			if (removeObject(kItemMoney) == 0)
				flags[30] = 1;
			if (removeObject(kItemTwoCoins) == 0)
				flags[31] = 1;
			if (removeObject(kItemOneCoin) == 0)
				flags[32] = 1;
		}
		talk_vonBraun(21, kVonBraunNormal);
	} else
		animation_27_2();

	flags[25] = 1;
	breakOut = 1;
}

void DrasculaEngine::animation_23_joined() {
	debug(4, "animation_23_joined()");

	int p_x = curX + 2, p_y = curY - 3;
	int x[] = {1, 38, 75, 112, 75, 112, 75, 112, 149, 112, 149, 112, 149, 186, 223, 260,
				1, 38, 75, 112, 149, 112, 149, 112, 149, 112, 149, 186, 223, 260, 260, 260, 260, 223};
	int y[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 76, 76, 76, 76, 76, 76, 76,
				76, 76, 76, 76, 76, 76, 76, 1, 1, 1, 1};

	loadPic("an23.alg", backSurface);

	for (int n = 0; n < 34; n++) {
		copyRect(p_x, p_y, p_x, p_y, 36, 74, bgSurface, screenSurface);
		copyRect(x[n], y[n], p_x, p_y, 36, 74, backSurface, screenSurface);
		updateRefresh();
		updateScreen(p_x, p_y, p_x, p_y, 36, 74, screenSurface);
		updateEvents();
		pause(5);
	}

	loadPic(99, backSurface);
}

void DrasculaEngine::animation_23_joined2() {
	debug(4, "animation_23_joined2()");

	int p_x = curX + 4, p_y = curY;
	int x[] = {1, 35, 69, 103, 137, 171, 205, 239, 273, 1, 35, 69, 103, 137};
	int y[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 73, 73, 73, 73, 73};

	pause(50);

	loadPic("an23_2.alg", backSurface);

	for (int n = 0; n < 14; n++) {
		copyRect(p_x, p_y, p_x, p_y, 33, 71, bgSurface, screenSurface);
		copyRect(x[n], y[n], p_x, p_y, 33, 71, backSurface, screenSurface);
		updateRefresh();
		updateScreen(p_x,p_y, p_x,p_y, 33,71, screenSurface);
		updateEvents();
		pause(5);
	}

	loadPic(99, backSurface);
}

void DrasculaEngine::animation_25_2() {
	debug(4, "animation_25_2()");

	int cY = 0;

	loadPic("an14_2.alg", backSurface);
	loadPic(18, bgSurface);

	flags[24] = 0;

	playSound(6);

	for (int n = 0; n >= -160; n = n - 8) {
		copyBackground();

		updateRefresh_pre();
		moveCharacters();
		moveVonBraun();

		cY = n;

		copyRect(150, 6, 69, cY, 158, 161, backSurface, screenSurface);

		updateRefresh();
		updateScreen();
		updateEvents();
	}

	finishSound();

	loadPic(99, backSurface);
}

void DrasculaEngine::animation_27_2() {
	debug(4, "animation_27_2()");

	flags[22] = 1;

	selectVerb(kVerbNone);
	removeObject(kItemEarWithEarPlug);
	addObject(kItemEarplugs);

	talk_vonBraun(23, kVonBraunNormal);
	talk_vonBraun(24, kVonBraunNormal);
	if (flags[30] == 1)
		addObject(kItemMoney);
	if (flags[31] == 1)
		addObject(kItemTwoCoins);
	if (flags[32] == 1)
		addObject(kItemOneCoin);
	talk_vonBraun(25, kVonBraunNormal);
	talk_vonBraun(26, kVonBraunNormal);
}

void DrasculaEngine::animation_29_2() {
	debug(4, "animation_29_2()");

	if (flags[33] == 0) {
		playTalkSequence(29);	// sequence 29, chapter 2
	} else
		talk_vonBraun(43, kVonBraunNormal);

	talk(402);
	talk_vonBraun(42, kVonBraunNormal);

	if (flags[38] == 0) {
		talk(403);
		breakOut = 1;
	} else
		talk(386);
}

void DrasculaEngine::animation_31_2() {
	debug(4, "animation_31_2()");

	talk_vonBraun(44, kVonBraunNormal);
	placeVonBraun(-50);
	pause(15);
	gotoObject(159, 140);
	loadPic(99, backSurface);

	playTalkSequence(31);	// sequence 31, chapter 2

	selectVerb(kVerbNone);
	removeObject(kItemLeaves);
	removeObject(kItemBubbleGum);
	removeObject(kItemTissues);
	removeObject(kItemCigarettes);
	removeObject(kItemCandle);
	addObject(kItemReefer);
}

void DrasculaEngine::animation_35_2() {
	debug(4, "animation_35_2()");

	gotoObject(96, 165);
	gotoObject(79, 165);

	updateRoom();
	updateScreen();

	loadPic("an35_1.alg", backSurface);
	loadPic("an35_2.alg", frontSurface);

	updateAnim(1, 70, 90, 46, 80, 6, backSurface);
	updateAnim(82, 70, 90, 46, 80, 6, backSurface);
	updateAnim(1, 70, 90, 46, 80, 6, frontSurface);
	updateAnim(82, 70, 90, 46, 80, 2, frontSurface);

	copyBackground();

	updateScreen();

	pause(19);

	playSound(1);
	hipo_sin_nadie(18);
	finishSound();

	pause(10);

	fadeToBlack(2);
}

// Use cross on Yoda
void DrasculaEngine::animation_2_3() {
	debug(4, "animation_2_3()");

	flags[0] = 1;
	playMusic(13);
	animation_3_3();
	playMusic(13);
	animation_4_3();
	flags[1] = 1;
	updateRoom();
	updateScreen(120, 0, 120, 0, 200, 200, screenSurface);
	animation_5_3();
	flags[0] = 0;
	flags[1] = 1;

	loadPic(96, frontSurface);
	loadPic(97, extraSurface);
	loadPic(99, backSurface);

	gotoObject(332, 127);
}

void DrasculaEngine::animation_3_3() {
	debug(4, "animation_3_3()");

	int px = curX - 20, py = curY - 1;

	loadPic("an2y_1.alg", frontSurface);
	loadPic("an2y_2.alg", extraSurface);
	loadPic("an2y_3.alg", backSurface);

	updateAnim(2, px, py, 71, 72, 4, frontSurface, 3, true);
	updateAnim(75, px, py, 71, 72, 4, frontSurface, 3, true);
	updateAnim(2, px, py, 71, 72, 4, extraSurface, 3, true);
	updateAnim(75, px, py, 71, 72, 4, extraSurface, 3, true);
	updateAnim(2, px, py, 71, 72, 4, backSurface, 3, true);
	updateAnim(75, px, py, 71, 72, 4, backSurface, 3, true);
}

void DrasculaEngine::animation_4_3() {
	debug(4, "animation_4_3()");

	int px = 120, py = 63;

	loadPic("any_1.alg", frontSurface);
	loadPic("any_2.alg", extraSurface);
	loadPic("any_3.alg", backSurface);

	updateAnim(1, px, py, 77, 89, 4, frontSurface, 3, true);
	updateAnim(91, px, py, 77, 89, 4, frontSurface, 3, true);
	updateAnim(1, px, py, 77, 89, 4, extraSurface, 3, true);
	updateAnim(91, px, py, 77, 89, 4, extraSurface, 3, true);
	updateAnim(1, px, py, 77, 89, 4, backSurface, 3, true);
	updateAnim(91, px, py, 77, 89, 4, backSurface, 3, true);
}

void DrasculaEngine::animation_5_3() {
	debug(4, "animation_5_3()");

	int px = curX - 20, py = curY - 1;

	loadPic("an3y_1.alg", frontSurface);
	loadPic("an3y_2.alg", extraSurface);
	loadPic("an3y_3.alg", backSurface);

	updateAnim(2, px, py, 71, 72, 4, frontSurface, 3, true);
	updateAnim(75, px, py, 71, 72, 4, frontSurface, 3, true);
	updateAnim(2, px, py, 71, 72, 4, extraSurface, 3, true);
	updateAnim(75, px, py, 71, 72, 4, extraSurface, 3, true);
	updateAnim(2, px, py, 71, 72, 4, backSurface, 3, true);
	updateAnim(75, px, py, 71, 72, 4, backSurface, 3, true);
}

void DrasculaEngine::animation_6_3() {
	debug(4, "animation_6_3()");

	int frame = 0, px = 112, py = 62;
	int yoda_x[] = { 3 ,82, 161, 240, 3, 82 };
	int yoda_y[] = { 3, 3, 3, 3, 94, 94 };

	characterMoved = 0;
	flags[3] = 1;
	updateRoom();
	updateScreen();

	flags[1] = 0;

	loadPic("an4y.alg", frontSurface);

	for (frame = 0; frame < 6; frame++) {
		pause(3);
		copyBackground();
		copyRect(yoda_x[frame], yoda_y[frame], px, py,	78, 90,	frontSurface, screenSurface);
		updateScreen(px, py, px, py, 78, 90, screenSurface);
		updateEvents();
	}

	flags[2] = 1;

	loadPic(96, frontSurface);

	updateRoom();
	updateScreen();
}

void DrasculaEngine::animation_ray() {
	debug(4, "animation_ray()");

	loadPic("anr_1.alg", frontSurface, HALF_PAL);
	loadPic("anr_2.alg", extraSurface);
	loadPic("anr_3.alg", backSurface);
	loadPic("anr_4.alg", bgSurface);
	loadPic("anr_5.alg", drawSurface3);

	updateScreen(0, 0, 0, 0, 320, 200, frontSurface);

	pause(50);

	playSound(5);

	updateScreen(0, 0, 0, 0, 320, 200, extraSurface);
	pause(3);
	updateScreen(0, 0, 0, 0, 320, 200, backSurface);
	pause(3);
	updateScreen(0, 0, 0, 0, 320, 200, bgSurface);
	pause(3);
	updateScreen(0, 0, 0, 0, 320, 200, backSurface);
	pause(3);
	updateScreen(0, 0, 0, 0, 320, 200, drawSurface3);
	pause(3);
	updateScreen(0, 0, 0, 0, 320, 200, frontSurface);
	finishSound();
}

void DrasculaEngine::animation_7_4() {
	debug(4, "animation_7_4()");

	black();
	talk(427);
	fadeFromBlack(1);
	removeObject(8);
	removeObject(10);
	removeObject(12);
	removeObject(16);
	addObject(17);
	flags[30] = 0;
	flags[29] = 0;
}

void DrasculaEngine::animation_1_5() {
	debug(4, "animation_1_5()");

	if (flags[0] == 0) {
		talk(430);
		talk_bj(16);
		talk_bj(17);
		talk_bj(18);
		talk(217);
		talk_bj(19);
		talk(229);
		pause(5);
		gotoObject(114, 170);
		trackProtagonist = 3;
		talk(431);
		talk_bj(20);
		trackProtagonist = 2;
		pause(4);
		talk(438);
		roomX = 120;
		roomY = 157;
		walkToObject = 1;
		trackFinal = 1;
		startWalking();
		talk_bj(21);

		while (!shouldQuit()) {
			if (characterMoved == 0)
				break;
			updateRoom();
			updateScreen();
			updateEvents();
		}

		trackProtagonist = 1;
		talk(229);
		flags[0] = 1;
	}

	trackProtagonist = 1;
	converse(8);
}

void DrasculaEngine::animation_5_5(){
	debug(4, "animation_5_5(");

	int h;
	int frame = 0;
	int boneX[] = {1, 99, 197, 1, 99, 197, 1, 99, 197};
	int boneY[] = {1, 1, 1, 66, 66, 66, 131, 131, 131};
	int flyX[] = {1, 63, 125, 187, 249};
	int pixelX = curX - 53, pixelY = curY - 9;

	selectVerb(kVerbNone);
	removeObject(8);

	gotoObject(curX - 19, curY + curHeight);
	trackProtagonist = 1;
	updateRoom();
	updateScreen();

	loadPic("3an5_1.alg", backSurface);
	loadPic("3an5_2.alg", frontSurface);

	for (frame = 0; frame < 9; frame++) {
		pause(3);
		copyBackground();
		copyRect(boneX[frame], boneY[frame], pixelX, pixelY, 97, 64, backSurface, screenSurface);
		updateScreen(pixelX, pixelY, pixelX,pixelY, 97,64, screenSurface);
		updateEvents();
	}

	copyBackground(52, 161, 198, 81, 26, 24, drawSurface3, screenSurface);
	updateScreen(198, 81, 198, 81, 26, 24, screenSurface);

	for (frame = 0; frame < 9; frame++) {
		pause(3);
		copyBackground();
		copyRect(boneX[frame], boneY[frame], pixelX, pixelY, 97, 64, frontSurface, screenSurface);
		updateScreen(pixelX, pixelY, pixelX,pixelY, 97, 64, screenSurface);
		updateEvents();
	}

	flags[6] = 1;
	updateVisible();
	pause(12);

	loadPic(96, frontSurface);
	for (h = 0; h < (200 - 18); h++)
		copyBackground(0, 53, 0, h, 320, 19, frontSurface, screenSurface);

	updateScreen();

	loadPic(101, bgSurface, HALF_PAL);
	loadPic("3an5_3.alg", backSurface);
	loadPic("3an5_4.alg", extraSurface);

	updateScreen(0, 0, 0, 0, 320, 200, bgSurface);
	pause(9);
	for (frame = 0; frame < 5; frame++) {
		pause(3);
		copyBackground(flyX[frame], 1, 174, 79, 61, 109, backSurface, screenSurface);
		updateScreen(174, 79, 174, 79, 61, 109, screenSurface);
		updateEvents();
	}
	for (frame = 0; frame < 5; frame++) {
		pause(3);
		copyBackground(flyX[frame], 1, 174, 79, 61, 109, extraSurface, screenSurface);
		updateScreen(174, 79, 174, 79, 61, 109, screenSurface);
		updateEvents();
	}
	updateScreen(0, 0, 0, 0, 320, 200, bgSurface);

	playSound(1);
	finishSound();

	loadPic(99, backSurface);
	loadPic(97, extraSurface);

	clearRoom();

	loadPic(49, bgSurface, HALF_PAL);
}

void DrasculaEngine::animation_11_5() {
	debug(4, "animation_11_5()");

	flags[9] = 1;
	if (flags[2] == 1 && flags[3] == 1 && flags[4] == 1)
		animation_12_5();
	else {
		flags[9] = 0;
		talk(33);
	}
}

void DrasculaEngine::animation_12_5() {
	debug(4, "animation_12_5()");

	DacPalette256 bgPalette1;
	DacPalette256 bgPalette2;
	DacPalette256 bgPalette3;

	int frame;
	const int rayX[] = {1, 46, 91, 136, 181, 226, 271, 181};
	const int frusky_x[] = {100, 139, 178, 217, 100, 178, 217, 139, 100, 139};
	const int elfrusky_x[] = {1, 68, 135, 1, 68, 135, 1, 68, 135, 68, 1, 135, 68, 135, 68};
	int color, component;
	signed char fade;

	playMusic(26);
	updateRoom();
	updateScreen();
	pause(27);
	animate("rayo1.bin", 23);
	playSound(5);
	animate("rayo2.bin", 17);
	trackProtagonist = 1;
	updateRoom();
	updateScreen();

	setDefaultPalette(darkPalette);

	for (color = 0; color < 255; color++)
		for (component = 0; component < 3; component++) {
			bgPalette1[color][component] = gamePalette[color][component];
			bgPalette2[color][component] = gamePalette[color][component];
			bgPalette3[color][component] = gamePalette[color][component];
		}

	for (fade = 3; fade >= 0; fade--) {
		for (color = 0; color < 128; color++) {
			for (component = 0; component < 3; component++) {
				bgPalette3[color][component] = adjustToVGA(bgPalette3[color][component] - 8 + fade);
				if (fade <= 2)
					bgPalette2[color][component] = adjustToVGA(bgPalette2[color][component] - 8 + fade);
				if (fade <= 1)
					bgPalette1[color][component] = adjustToVGA(bgPalette1[color][component] - 8 + fade);
			}
		}
	}

	loadPic("3an11_1.alg", backSurface);

	for (frame = 0; frame < 8; frame++) {
		if (frame == 2 || frame == 4 || frame == 8 || frame==10)
			setPalette((byte *)&bgPalette1);
		else if (frame == 1 || frame == 5 || frame == 7 || frame == 9)
			setPalette((byte *)&bgPalette2);
		else
			setPalette((byte *)&bgPalette3);

		pause(4);
		updateRoom();
		copyRect(rayX[frame], 1, 41, 0, 44, 44, backSurface, screenSurface);
		copyRect(frusky_x[frame], 113, 205, 50, 38, 86, drawSurface3, screenSurface);
		updateScreen();
		updateEvents();
	}

	stopSound();

	for (frame = 0; frame < 15; frame++) {
		if (frame == 2 || frame == 4 || frame == 7 || frame == 9)
			setPalette((byte *)&bgPalette1);
		else if (frame == 1 || frame == 5)
			setPalette((byte *)&gamePalette);
		else
			setPalette((byte *)&bgPalette2);

		pause(4);
		updateRoom();
		copyRect(elfrusky_x[frame], 47, 192, 39, 66, 106, backSurface, screenSurface);
		updateScreen();
		updateEvents();
	}

	animate("frel.bin", 16);
	clearRoom();
	setDefaultPalette(brightPalette);
	setPalette((byte *)&gamePalette);

	flags[1] = 1;

	animation_13_5();
	playSound(1);
	hiccup(12);
	finishSound();

	loadPic(99, backSurface);

	gotoObject(40, 169);
	gotoObject(-14, 175);

	doBreak = 1;
	previousMusic = roomMusic;
	hare_se_ve = 1;
	clearRoom();
	trackProtagonist = 1;
	characterMoved = 0;
	curX = -1;
	objExit = 104;
	selectVerb(kVerbNone);
	enterRoom(57);
}

void DrasculaEngine::animation_13_5() {
	debug(4, "animation_13_5()");

	int frank_x = 199;
	int frame = 0;
	int frus_x[] = {1, 46, 91, 136, 181, 226, 271};
	int frus_y[] = {1, 1, 1, 1, 1, 1, 1, 89};

	loadPic("auxfr.alg", backSurface);

	updateRoom();
	copyRect(1, 1, frank_x, 81, 44, 87, backSurface, screenSurface);
	updateScreen();
	pause(15);

	playMusic(18);

	while (!shouldQuit()) {
		updateRoom();
		copyRect(frus_x[frame], frus_y[frame], frank_x, 81, 44, 87, backSurface, screenSurface);
		updateScreen();
		frank_x -= 5;
		frame++;
		if (frank_x <= -45)
			break;
		if (frame == 7) {
			frame = 0;
			trackProtagonist = 3;
		}
		updateEvents();
		pause(6);
	}
}

void DrasculaEngine::animation_14_5() {
	debug(4, "animation_14_5()");

	flags[11] = 1;
	playSound(3);
	updateRoom();
	updateScreen(0, 0, 0,0 , 320, 200, screenSurface);
	finishSound();
	pause(17);
	trackProtagonist = 3;
	talk(246);
	gotoObject(89, 160);
	flags[10] = 1;
	playSound(7);
	updateRoom();
	updateScreen();
	finishSound();
	pause(14);
	trackProtagonist = 3;
	updateRoom();
	updateScreen();
	talk_solo(_textd[18], "d18.als");
	fadeToBlack(1);
}

void DrasculaEngine::animation_1_6() {
	debug(4, "animation_1_6()");

	trackProtagonist = 0;
	curX = 103;
	curY = 108;
	flags[0] = 1;
	for (int l = 0; l < 200; l++)
		factor_red[l] = 98;

	loadPic("auxig2.alg", frontSurface);
	loadPic("auxdr.alg", drawSurface2);
	loadPic("car.alg", backSurface);
	talk_drascula(19, 1);
	talk(247);
	talk_drascula(20, 1);
	talk_drascula(21, 1);
	talk(248);
	talk_drascula(22, 1);
	talk(249);
	talk_drascula(23, 1);
	converse(11);
	talk_drascula(26, 1);

	animate("fum.bin", 15);

	talk_drascula(27, 1);
	talk(254);
	talk_drascula(28, 1);
	talk(255);
	talk_drascula(29, 1);
	updateEvents();
	fadeToBlack(1);
	updateEvents();
	clearRoom();
	loadPic("time1.alg", screenSurface);
	updateScreen();
	updateEvents();
	delay(930);
	updateEvents();
	clearRoom();
	black();
	hare_se_ve = 0;
	flags[0] = 0;
	updateRoom();
	updateScreen();
	fadeFromBlack(1);
	talk(256);
	talk_drascula(30, 1);
	talk(257);
	fadeToBlack(0);
	updateEvents();
	clearRoom();
	loadPic("time1.alg", screenSurface);
	updateScreen();
	updateEvents();
	delay(900);
	updateEvents();
	clearRoom();
	black();
	updateRoom();
	updateScreen();
	fadeFromBlack(1);
	talk(258);
	talk_drascula(31, 1);
	animation_5_6();
	talk_drascula(32, 1);
	talk_igor(11, kIgorDch);
	trackIgor = 3;
	talk_drascula(33, 1);
	talk_igor(12, kIgorFront);
	talk_drascula(34, 1);
	trackDrascula = 0;
	talk_drascula(35);

	clearRoom();
	enterRoom(102);
	activatePendulum();
}

void DrasculaEngine::animation_5_6() {
	debug(4, "animation_5_6()");

	int pY = -125;

	animate("man.bin", 14);

	for (int n = -125; n <= 0; n = n + 2) {
		copyBackground();
		updateRefresh_pre();
		pY = n;
		copyRect(1, 29, 204, pY, 18, 125, drawSurface3, screenSurface);

		updateRefresh();

		updateScreen();
		updateEvents();
		pause(2);
	}

	flags[3] = 1;
}

void DrasculaEngine::animation_6_6() {
	debug(4, "animation_6_6()");

	animate("rct.bin", 11);
	clearRoom();
	selectVerb(kVerbNone);
	removeObject(20);
	loadPic(96, frontSurface);
	loadPic(97, frontSurface);
	loadPic(97, extraSurface);
	loadPic(99, backSurface);
	doBreak = 1;
	objExit = 104;
	curX = -1;
	selectVerb(kVerbNone);
	enterRoom(58);
	hare_se_ve = 1;
	trackProtagonist = 1;
	animate("hbp.bin", 14);

	trackProtagonist = 3;
	flags[0] = 1;
	flags[1] = 0;
	flags[2] = 1;
}

void DrasculaEngine::animation_9_6() {
	debug(4, "animation_9_6()");

	int v_cd;

	animate("fin.bin", 14);
	playMusic(13);
	flags[5] = 1;
	animate("drf.bin", 16);
	fadeToBlack(0);
	clearRoom();
	curX = -1;
	objExit = 108;
	enterRoom(59);
	// The room number was originally changed here to "nada.alg",
	// which is a non-existant file. In reality, this was just a
	// hack to set the room number to a non-existant one, so that
	// room sprites do not appear again when the room is refreshed.
	// We set the room number to -1 for the same purpose.
	// Also check animation_2_1(), where the same hack was used
	// by the original
	roomNumber = -2;
	loadPic("nota2.alg", bgSurface, HALF_PAL);
	black();
	trackProtagonist = 1;
	curX -= 21;
	updateRoom();
	updateScreen();
	fadeFromBlack(0);
	pause(96);
	gotoObject(116, 178);
	trackProtagonist = 2;
	updateRoom();
	updateScreen();
	playMusic(9);
	clearRoom();
	loadPic("nota.alg", bgSurface, COMPLETE_PAL);
	color_abc(kColorWhite);
	talk_solo(_textbj[24], "bj24.als");
	talk_solo(_textbj[25], "bj25.als");
	talk_solo(_textbj[26], "bj26.als");
	talk_solo(_textbj[27], "bj27.als");
	talk_solo(_textbj[28], "bj28.als");
	trackProtagonist = 3;
	clearRoom();
	loadPic(96, frontSurface, COMPLETE_PAL);
	loadPic("nota2.alg", bgSurface, HALF_PAL);
	talk(296);
	talk(297);
	talk(298);
	trackProtagonist = 1;
	talk(299);
	talk(300);
	updateRoom();
	copyBackground(0, 0, 0, 0, 320, 200, screenSurface, bgSurface);
	updateScreen();
	color_abc(kColorLightGreen);
	talk_solo(_textmisc[2], "s15.als");
	loadPic("nota2.alg", bgSurface);
	trackProtagonist = 0;
	updateRoom();
	updateScreen();
	talk(301);
	v_cd = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 16;
	v_cd += 4;
	playMusic(17);
	fadeToBlack(1);
	clearRoom();
	playFLI("qpc.bin", 1);
	MusicFadeout();
	stopMusic();
	clearRoom();
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, v_cd * 16);
	playMusic(3);
	playFLI("crd.bin", 1);
	stopMusic();
}

void DrasculaEngine::animation_19_6() {
	debug(4, "animation_19_6()");

	copyBackground();
	copyBackground(140, 23, 161, 69, 35, 80, drawSurface3, screenSurface);

	updateRefresh_pre();
	moveCharacters();
	updateScreen();
	pause(6);
	updateRoom();
	updateScreen();
	playSound(4);
	pause(6);
	finishSound();
}

void DrasculaEngine::animation_12_2() {
	debug(4, "animation_12_2()");

	loadPic("an12.alg", extraSurface);

	talk(356);

	updateRefresh_pre();

	copyBackground(1, 139, 228, 112, 47, 60, extraSurface, screenSurface);
	updateScreen(228, 112, 228, 112, 47, 60, screenSurface);

	pause(3);

	updateRefresh_pre();

	copyBackground(49, 139, 228, 112, 47, 60, extraSurface, screenSurface);
	moveCharacters();

	updateScreen();

	pause(3);
	stopMusic();
	flags[11] = 1;

	talk_pianist(5);
	converse(1);

	flags[11] = 0;
	loadPic(974, extraSurface);
}

void DrasculaEngine::animation_26_2() {
	debug(4, "animation_26_2()");

	loadPic("an12.alg", extraSurface);

	talk(392);

	updateRefresh_pre();

	copyBackground(1, 139, 228, 112, 47, 60, extraSurface, screenSurface);
	updateScreen(228, 112, 228, 112, 47, 60, screenSurface);

	pause(3);

	updateRefresh_pre();

	copyBackground(49, 139, 228, 112, 47, 60, extraSurface, screenSurface);
	moveCharacters();

	updateScreen();

	pause(3);
	stopMusic();
	flags[11] = 1;

	talk_pianist(5);
	talk(393);
	talk_pianist(17);
	talk_pianist(18);
	talk_pianist(19);

	loadPic("an26.alg", extraSurface);

	updateAnim(1, 225, 113, 50, 59, 6, extraSurface);

	int	x = 0;
	for (int n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 61, 225, 113, 50, 59, extraSurface, screenSurface);
		updateScreen(225, 113, 225, 113, 50, 59, screenSurface);
		x = x + 50;
		if (n == 2)
			playSound(9);
		updateEvents();
		pause(3);
	}

	stopSound();

	updateAnim(121, 225, 113, 50, 59, 6, extraSurface);

	pickObject(11);
	removeObject(kItemBook);

	flags[11] = 0;
	flags[39] = 1;
	loadPic(974, extraSurface);
	roomMusic = 16;
}

void DrasculaEngine::animation_11_2() {
	debug(4, "animation_11_2()");

	loadPic("an11y13.alg", extraSurface);

	playTalkSequence(11);	// sequence 11, chapter 2

	loadPic(974, extraSurface);
}

void DrasculaEngine::animation_13_2() {
	debug(4, "animation_13_2()");

	loadPic("an11y13.alg", frontSurface);

	if (flags[41] == 0) {
		playTalkSequence(13);	// sequence 13, chapter 2
	}

	loadPic(964, frontSurface);
}

void DrasculaEngine::animation_24_2() {
	debug(4, "animation_24_2()");

	if (curX < 178)
		gotoObject(208, 136);
	trackProtagonist = 3;
	updateRoom();
	pause(3);
	trackProtagonist = 0;

	talk(356);

	loadPic("an24.alg", frontSurface);

	animation_32_2();

	flags[21] = 1;

	talk_vonBraun(22, kVonBraunNormal);

	if (flags[22] == 0)
		converse(4);
	else
		converse(5);

	exitRoom(0);
	flags[21] = 0;
	flags[24] = 0;
	trackVonBraun = 1;
	vonBraunX = 120;
}

void DrasculaEngine::animation_32_2() {
	debug(4, "animation_32_2()");

	loadPic("an32_1.alg", drawSurface3);
	loadPic("an32_2.alg", backSurface);

	updateAnim(1, 113, 53, 65, 81, 4, drawSurface3, 4);
	updateAnim(83, 113, 53, 65, 81, 4, drawSurface3, 4);
	updateAnim(1, 113, 53, 65, 81, 4, backSurface, 4);

	int x = 0;
	for (int n = 0; n < 3; n++) {
		x++;
		copyBackground(x, 83, 113, 53, 65, 81, backSurface, screenSurface);
		updateScreen(113, 53, 113, 53, 65, 81, screenSurface);
		x = x + 65;
		if (n < 2)
			pause(4);

		updateEvents();
	}

	loadPic("aux18.alg", drawSurface3);
}

void DrasculaEngine::animation_34_2() {
	debug(4, "animation_34_2()");

	trackProtagonist = 1;
	updateRoom();
	updateScreen();

	loadPic("an34_1.alg", backSurface);
	loadPic("an34_2.alg", extraSurface);

	updateAnim(1, 218, 79, 83, 75, 3, backSurface);
	updateAnim(77, 218, 79, 83, 75, 3, backSurface);

	playSound(8);

	updateAnim(1, 218, 79, 83, 75, 3, extraSurface);

	finishSound();

	pause(30);

	copyBackground(1, 77, 218, 79, 83, 75, extraSurface, screenSurface);
	updateScreen(218, 79, 218, 79, 83, 75, screenSurface);
	pause(3);

	loadPic(994, backSurface);
	loadPic(974, extraSurface);
}

void DrasculaEngine::animation_36_2() {
	debug(4, "animation_36_2()");

	loadPic("an11y13.alg", extraSurface);

	talk(404);
	talk_bartender(19);
	talk_bartender(20);
	talk_bartender(21);
	talk(355);
	pause(40);
	talk_bartender(82);

	loadPic(974, extraSurface);
}

// Use sickle on plant
void DrasculaEngine::animation_7_2() {
	debug(4, "animation_7_2()");

	loadPic("an7_1.alg", backSurface);
	loadPic("an7_2.alg", extraSurface);
	loadPic("an7_3.alg", frontSurface);

	if (flags[3] == 1)
		copyBackground(258, 110, 85, 44, 23, 53, drawSurface3, bgSurface);

	copyBackground();

	updateScreen();

	updateAnim(1, 80, 64, 51, 73, 6, backSurface, 3, true);
	updateAnim(75, 80, 64, 51, 73, 6, backSurface, 3, true);
	updateAnim(1, 80, 64, 51, 73, 6, extraSurface, 3, true);
	updateAnim(75, 80, 64, 51, 73, 6, extraSurface, 3, true);
	updateAnim(1, 80, 64, 51, 73, 6, frontSurface, 3, true);

	loadPic("an7_4.alg", backSurface);
	loadPic("an7_5.alg", extraSurface);
	loadPic("an7_6.alg", frontSurface);
	loadPic("an7_7.alg", drawSurface3);

	updateAnim(1, 80, 64, 51, 73, 6, backSurface, 3, true);
	updateAnim(75, 80, 64, 51, 73, 6, backSurface, 3, true);
	updateAnim(1, 80, 64, 51, 73, 6, extraSurface, 3, true);
	updateAnim(75, 80, 64, 51, 73, 6, extraSurface, 3, true);
	updateAnim(1, 80, 64, 51, 73, 6, frontSurface, 3, true);
	updateAnim(75, 80, 64, 51, 73, 6, extraSurface, 3, true);
	updateAnim(1, 80, 64, 51, 73, 6, frontSurface, 3, true);
	updateAnim(75, 80, 64, 51, 73, 6, frontSurface, 3, true);
	updateAnim(1, 80, 64, 51, 73, 6, drawSurface3, 3, true);
	updateAnim(75, 80, 64, 51, 73, 2, drawSurface3, 3, true);

	loadPic("an7_8.alg", backSurface);
	loadPic("an7_9.alg", extraSurface);

	updateAnim(1, 80, 64, 51, 73, 6, backSurface, 3, true);
	updateAnim(75, 80, 64, 51, 73, 6, backSurface, 3, true);
	updateAnim(1, 80, 64, 51, 73, 6, extraSurface, 3, true);

	copyBackground(80, 64, 80, 64, 51, 73, bgSurface, screenSurface);
	copyRect(1, 75, 80, 64, 51, 73, extraSurface, screenSurface);
	updateScreen(80, 64, 80, 64, 51, 73, screenSurface);

	flags[37] = 1;

	if (flags[7] == 1 && flags[26] == 1 && flags[34] == 1 && flags[35] == 1 && flags[37] == 1)
		flags[38] = 1;

	loadPic(99, backSurface);
	loadPic(97, extraSurface);
	loadPic(96, frontSurface);
	loadPic("aux3.alg", drawSurface3);
}

void DrasculaEngine::animation_5_2() {
	debug(4, "animation_5_2()");

	trackProtagonist = 0;
	updateRoom();
	updateScreen();

	loadPic("an5_1.alg", backSurface);
	loadPic("an5_2.alg", extraSurface);
	loadPic("an5_3.alg", frontSurface);
	loadPic("an5_4.alg", drawSurface3);

	copyBackground(1, 1, 213, 66,	53,84, backSurface, screenSurface);
	updateScreen();

	updateAnim(1, 213, 66, 52, 84, 6, backSurface);
	updateAnim(86, 213, 66, 52, 84, 6, backSurface);
	updateAnim(1, 213, 66, 52, 84, 6, extraSurface);
	updateAnim(1, 213, 66, 52, 84, 6, extraSurface);
	updateAnim(86, 213, 66, 52, 84, 6, extraSurface);
	updateAnim(1, 213, 66, 52, 84, 6, frontSurface);

	playSound(1);
	updateAnim(86, 213, 66, 52, 84, 6, frontSurface);
	stopSound();

	updateAnim(1, 213, 66, 52, 84, 6, drawSurface3);

	loadPic(994, backSurface);
	loadPic(974, extraSurface);
	loadPic(964, frontSurface);
	loadPic("aux5.alg", drawSurface3);
	flags[8] = 1;
	curX = curX - 4;
	talk_sync(_text[46], "46.als", "4442444244244");
	selectVerb(kVerbNone);
}

void DrasculaEngine::animation_6_2() {
	debug(4, "animation_6_2()");

	stopMusic();
	flags[9] = 1;

	clearRoom();
	loadPic("ciego1.alg", bgSurface, HALF_PAL);	// ciego = blind
	loadPic("ciego2.alg", drawSurface3);
	loadPic("ciego3.alg", extraSurface);
	loadPic("ciego4.alg", backSurface);
	loadPic("ciego5.alg", frontSurface);

	copyBackground();
	updateScreen();

	pause(1);

	if (flags[4] == 1)
		talk_hacker(66);
	pause(6);
	talk_blind(11);
	pause(4);
	talk_hacker(67);

	copyBackground();
	updateScreen();

	pause(10);

	clearRoom();

	playMusic(roomMusic);
	loadPic(9, bgSurface, HALF_PAL);
	loadPic("aux9.alg", drawSurface3);
	loadPic(96, frontSurface);
	loadPic(97, extraSurface);
	loadPic(99, backSurface);
	selectVerb(kVerbNone);

	flags[9] = 0;
}

void DrasculaEngine::animation_33_2() {
	debug(4, "animation_33_2()");

	stopMusic();
	flags[9] = 1;

	pause(12);
	talk(60);
	pause(8);

	clearRoom();
	loadPic("ciego1.alg", bgSurface, HALF_PAL);	// ciego = blind
	loadPic("ciego2.alg", drawSurface3);
	loadPic("ciego3.alg", extraSurface);
	loadPic("ciego4.alg", backSurface);
	loadPic("ciego5.alg", frontSurface);

	copyBackground();
	updateScreen();

	pause(10);

	talk_blind(1);
	pause(5);
	talk_hacker(57);
	pause(6);
	_system->delayMillis(1000);
	talk_blind(10);
	talk_hacker(65);

	copyBackground();
	updateScreen();

	pause(14);

	clearRoom();

	playMusic(roomMusic);
	loadPic(9, bgSurface, HALF_PAL);
	loadPic("aux9.alg", drawSurface3);
	loadPic(96, frontSurface);
	loadPic(97, extraSurface);
	loadPic(99, backSurface);
	selectVerb(kVerbNone);

	flags[33] = 1;
	flags[9] = 0;
}

void DrasculaEngine::animation_1_4() {
	debug(4, "animation_1_4()");

	if (flags[21] == 0) {
		strcpy(objName[2], "igor");
		talk(275);

		updateRefresh_pre();

		copyBackground(131, 133, 199, 95, 50, 66, drawSurface3, screenSurface);
		updateScreen(199, 95, 199, 95, 50, 66, screenSurface);

		pause(3);

		updateRefresh_pre();

		copyBackground(182, 133, 199, 95, 50, 66, drawSurface3, screenSurface);
		moveCharacters();

		updateScreen();

		pause(3);
		flags[18] = 1;
		flags[20] = 1;

		talk_igor(13, kIgorSeated);
		talk_igor(14, kIgorSeated);
		talk_igor(15, kIgorSeated);
		flags[21] = 1;
	} else {
		talk(356);

		updateRefresh_pre();

		copyBackground(131, 133, 199, 95, 50, 66, drawSurface3, screenSurface);
		updateScreen(199, 95, 199, 95, 50, 66, screenSurface);
		pause(2);

		updateRefresh_pre();

		copyBackground(182, 133, 199, 95, 50, 66, drawSurface3, screenSurface);
		moveCharacters();

		updateScreen();

		flags[18] = 1;
		flags[20] = 1;

		talk(276);
		pause(14);
		talk_igor(6, kIgorSeated);
	}

	converse(6);
	flags[20] = 0;
	flags[18] = 0;
}

void DrasculaEngine::animation_5_4(){
	debug(4, "animation_5_4(");

	trackProtagonist = 3;
	loadPic("anh_dr.alg", backSurface);
	gotoObject(99, 160);
	gotoObject(38, 177);
	hare_se_ve = 0;
	updateRoom();
	updateScreen();
	delay(800);
	animate("bio.bin", 14);
	flags[29] = 1;
	curX = 95;
	curY = 82;
	updateRoom();
	updateScreen();
	toggleDoor(2, 0, kOpenDoor);
	loadPic("auxigor.alg", frontSurface);
	igorX = 100;
	igorY = 65;
	talk_igor(29, kIgorFront);
	talk_igor(30, kIgorFront);
	loadPic(96, frontSurface);
	loadPic(99, backSurface);
	hare_se_ve = 1;
	fadeToBlack(0);
	exitRoom(0);
}

void DrasculaEngine::animation_6_4() {
	debug(4, "animation_6_4()");

	int prevRoom = roomNumber;

	roomNumber = 26;
	clearRoom();
	loadPic(26, bgSurface, HALF_PAL);
	loadPic("aux26.alg", drawSurface3);
	loadPic("auxigor.alg", frontSurface);
	copyBackground();
	update_26_pre();
	igorX = 104;
	igorY = 71;
	placeIgor();
	updateScreen();
	pause(40);
	talk_igor(26, kIgorFront);
	roomNumber = prevRoom;
	clearRoom();
	loadPic(96, frontSurface);
	loadPic(roomDisk, drawSurface3);
	loadPic(roomNumber, bgSurface, HALF_PAL);
	selectVerb(kVerbNone);
	updateRoom();
}

void DrasculaEngine::animation_8_4() {
	debug(4, "animation_8_4()");

	int bookcaseX[] = {1, 75, 149, 223, 1, 75, 149, 223, 149, 223, 149, 223, 149, 223};
	int bookcaseY[] = {1, 1, 1, 1, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74};

	loadPic("an_8.alg", frontSurface);

	for (int frame = 0; frame < 14; frame++) {
		pause(2);
		copyBackground(bookcaseX[frame], bookcaseY[frame], 77, 45, 73, 72, frontSurface, screenSurface);
		updateScreen(77, 45, 77, 45, 73, 72, screenSurface);
		updateEvents();
	}

	loadPic(96, frontSurface);
	toggleDoor(7, 2, kOpenDoor);
}

void DrasculaEngine::activatePendulum() {
	debug(4, "activatePendulum()");

	flags[1] = 2;
	hare_se_ve = 0;
	roomNumber = 102;
	loadPic(102, bgSurface, HALF_PAL);
	loadPic("an_p1.alg", drawSurface3);
	loadPic("an_p2.alg", extraSurface);
	loadPic("an_p3.alg", frontSurface);

	copyBackground(0, 171, 0, 0, OBJWIDTH, OBJHEIGHT, backSurface, drawSurface3);

	savedTime = getTime();
}

} // End of namespace Drascula
