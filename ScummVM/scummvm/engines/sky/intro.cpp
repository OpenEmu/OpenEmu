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


#include "common/endian.h"
#include "common/util.h"
#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "sky/disk.h"
#include "sky/intro.h"
#include "sky/music/musicbase.h"
#include "sky/screen.h"
#include "sky/sky.h"
#include "sky/sound.h"
#include "sky/struc.h"
#include "sky/text.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace Sky {

#define SHOWSCREEN		 0
#define COMMANDEND		 0 // end of COMMANDFLIRT block
#define FADEUP			 1 // fade up palette
#define FADEDOWN		 2
#define DELAY			 3
#define DOFLIRT			 4 // start flirt sequence (and wait for it to finish)
#define SCROLLFLIRT		 5 // start special floppy intro flirt sequence (and wait for it)
#define COMMANDFLIRT	 6 // start flirt sequence and wait for it, while processing command block
#define BGFLIRT			 7 // start flirt sequence without waiting for it
#define WAITFLIRT		 8 // wait for sequence started by BGFLIRT
#define STOPFLIRT		 9
#define STARTMUSIC		10
#define WAITMUSIC		11
#define PLAYVOICE		12
#define WAITVOICE		13
#define LOADBG			14 // load new background sound
#define PLAYBG			15 // play background sound
#define LOOPBG			16 // loop background sound
#define STOPBG			17 // stop background sound
#define SEQEND		 65535 // end of intro sequence

#define IC_PREPARE_TEXT 20 // commands used in COMMANDFLIRT block
#define IC_SHOW_TEXT    21
#define IC_REMOVE_TEXT  22
#define IC_MAKE_SOUND   23
#define IC_FX_VOLUME    24

#define FRAME_SIZE (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT)
#define INTRO_TEXT_WIDTH 128
//CD intro file defines
#define CDV_00	59500
#define CD_PAL	59501
#define CD_1_LOG	59502
#define CD_1	59503
#define CDV_01	59504
#define CDV_02	59505
#define CD_2	59506
#define CDV_03	59507
#define CDV_04	59508
#define CD_3	59509
#define CDV_05	59510
#define CDV_06	59511
#define CD_5	59512
#define CDV_07	59513
#define CDV_08	59514
#define CDV_09	59515
#define CD_7	59516
#define CDV_10	59518
#define CD_11	59519
#define CDV_11	59520
#define CD_11_PAL	59521
#define CD_11_LOG	59522
#define CDV_12	59523
#define CD_13	59524
#define CDV_13	59525
#define CDV_14	59527
#define CDV_15	59528
#define CD_15_PAL	59529
#define CD_15_LOG	59530
#define CDV_16	59531
#define CD_17_LOG	59532
#define CD_17	59533
#define CDV_17	59534
#define CDV_18	59535
#define CDV_19	59536
#define CD_19_PAL	59537
#define CD_19_LOG	59538
#define CDV_20	59539
#define CD_20_LOG	59540
#define CDV_21	59541
#define CD_21_LOG	59542
#define CDV_22	59545
#define CDV_23	59546
#define CD_23_PAL	59547
#define CD_24_LOG	59550
#define CDV_24	59551
#define CDV_25	59554
#define CDV_26	59556
#define CD_27	59557
#define CDV_27	59558
#define CD_27_PAL	59559
#define CD_27_LOG	59560
#define CDV_28	59561
#define CDV_29	59562
#define CDV_30	59563
#define CDV_31	59565
#define CDV_32	59566
#define CDV_33	59567
#define CDV_34	59568
#define CD_35	59569
#define CDV_35	59570
#define CD_35_PAL	59571
#define CD_35_LOG	59572
#define CDV_36	59574
#define CD_37	59575
#define CDV_37	59576
#define CD_37_PAL	59577
#define CD_37_LOG	59578
#define CDV_38	59579
#define CDV_39	59581
#define CDV_40	59583
#define CD_40_PAL	59584
#define CD_40_LOG	59585
#define CDV_41	59587
#define CDV_42	59588
#define CD_43	59589
#define CDV_43	59590
#define CD_43_PAL	59591
#define CD_43_LOG	59592
#define CDV_44	59594
#define CD_45	59595
#define CDV_45	59596
#define CD_45_PAL	59597
#define CD_45_LOG	59598
#define CDV_46	59600
#define CDV_47	59602
#define CD_47_PAL	59603
#define CD_47_LOG	59604
#define CD_48	59605
#define CDV_48	59606
#define CD_48_PAL	59607
#define CD_48_LOG	59608
#define CD_49	59609
#define CDV_49	59610
#define CD_50	59611
#define CDV_50	59612
#define CDV_51	59613
#define CDV_52	59614
#define CDV_53	59615
#define CDV_54	59616
#define CDV_55	59618
#define CD_55_PAL	59619
#define CD_55_LOG	59620
#define CDV_56	59621
#define CDV_57	59622
#define CD_58	59623
#define CDV_58	59624
#define CD_58_PAL	59625
#define CD_58_LOG	59626
#define CDV_59	59627
#define CDV_60	59628
#define CDV_61	59629
#define CDV_62	59630
#define CDV_63	59631
#define CDV_64	59632
#define CDV_65	59633
#define CDV_66	59635
#define CD_66_PAL	59636
#define CD_66_LOG	59637
#define CDV_67	59639
#define CD_67_PAL	59640
#define CD_67_LOG	59641
#define CDV_68	59642
#define CD_69	59643
#define CDV_69	59644
#define CD_69_PAL	59645
#define CD_69_LOG	59646
#define CDV_70	59647
#define CDV_71	59648
#define CDV_72	59649
#define CD_72_PAL	59650
#define CD_72_LOG	59651
#define CD_73_PAL	59652
#define CD_73_LOG	59653
#define CDV_73	59654
#define CDV_74	59655
#define CDV_75	59656
#define CD_76_PAL	59657
#define CD_76_LOG	59658
#define CDV_76	59659
#define CDV_77	59660
#define CD_78_PAL	59661
#define CD_78_LOG	59662
#define CDV_78	59663
#define CDV_79	59664
#define CDV_80	59665
#define CDV_81	59666
#define CDV_82	59667
#define CDV_83	59668
#define CDV_84	59669
#define CDV_85	59670
#define CDV_86	59671
#define CDV_87	59672
#define CD_100	60087
#define CD_101_LOG	60088
#define CD_101	60099
#define CD_102_LOG	60090
#define CD_102	60091
#define CD_103_PAL	60092
#define CD_103_LOG	60093
#define CD_103	60094
#define CD_104_PAL	60095
#define CD_104_LOG	60096
#define CD_104	60097
#define CD_105	60098


uint16 Intro::_mainIntroSeq[] = {
	DELAY,       3000, // keep virgin screen up
	FADEDOWN,
	SHOWSCREEN, 60112, // revo screen + palette
	FADEUP,     60113,
	DELAY,       8000,
	FADEDOWN,
	SHOWSCREEN, 60114, // gibbo screen + palette
	FADEUP,     60115,
	DELAY,       2000,
	FADEDOWN,
	SEQEND
};

uint16 Intro::_cdIntroSeq[] = {
	/* black screen */
	PLAYVOICE,	CDV_00,	// Foster: "The old man was trying to tell the future. Looking for pictures in the campfire..."
	LOADBG,		59499,	// Fire crackle
	LOOPBG,
	WAITVOICE,
	PLAYVOICE,	CDV_01,	// Shaman: "ohhh, I see evil..."
	/* Fade up shaman image while he says his line... */
	SHOWSCREEN,	CD_1_LOG,
	FADEUP,		CD_PAL,
	/* And then play the animation showing the shadows of the fire on his face */
	BGFLIRT,	CD_1,
		WAITVOICE,
		PLAYVOICE,	CDV_02,	// Shaman: "Evil born deep beneath the city... far from the light of day."
		WAITVOICE,
	STOPFLIRT,
	BGFLIRT,	CD_2,
		PLAYVOICE,	CDV_03, // Shaman: "I see it growing, safe beneath a sky of steel..."
		WAITVOICE,
		PLAYVOICE,	CDV_04, // Shaman: "Scheming in the dark... gathering strength."
	WAITFLIRT,
	WAITVOICE,
	PLAYVOICE,	CDV_05,		// Shaman: "And now... ohhh.... now the evil spreads..."
	DELAY,		2000,
	BGFLIRT,	CD_3,
		WAITVOICE,
		PLAYVOICE,	CDV_06,	// Shaman: "It sends deadly feelers over the land above..."
	WAITFLIRT,
	WAITVOICE,
	PLAYVOICE,	CDV_07,		// Shaman: "Across the gap... reaching towards this very place!"
	BGFLIRT,	CD_5,
		WAITVOICE,
		PLAYVOICE,	CDV_08,	// Foster: "I'd seen him do this a hundred times, but I humoured him."
		WAITVOICE,
		PLAYVOICE,	CDV_09,	// Foster: "After all, he'd been like a father to me."
	WAITFLIRT,
	WAITVOICE,
	PLAYVOICE,	CDV_10,		// Foster: "And what does this evil want here?"
	BGFLIRT,	CD_7,
		WAITVOICE,
		PLAYVOICE,	CDV_11, // Shaman: "Oh, my son. I fear..."
	WAITFLIRT,
	FADEDOWN,
	SHOWSCREEN,	CD_11_LOG,
	FADEUP,		CD_11_PAL,
	WAITVOICE,
	PLAYVOICE,	CDV_12,		// Shaman: "I fear the evil wants you!"
	DELAY,		1600,
	BGFLIRT,	CD_11,
		WAITVOICE,
		PLAYVOICE,	CDV_13,	// Foster: "That was when Joey piped up..."
		WAITVOICE,
	WAITFLIRT,
	WAITVOICE,
	PLAYVOICE,	CDV_14,		// Joey: "Foster! Sensors detect incoming audio source!"
	LOADBG,		59498, // fire crackle to heli start
	PLAYBG,
	DOFLIRT,	CD_13,
	WAITVOICE,
	PLAYVOICE,	CDV_15,		// Shaman: "The evil! The evil is nearly here...!"
	FADEDOWN,
	SHOWSCREEN,	CD_15_LOG,
	FADEUP,		CD_15_PAL,
	WAITVOICE,
	LOADBG,		59496, // quiet heli
	LOOPBG,
	PLAYVOICE,	CDV_16,		// Foster: "It sounded more like a 'copter than a demon."
	WAITVOICE,
	PLAYVOICE,	CDV_17,		// Foster: "But next thing, all hell let loose anyway..."
	DELAY,		2000,
	SHOWSCREEN,	CD_17_LOG,
	WAITVOICE,
	BGFLIRT,	CD_17,
		PLAYVOICE,	CDV_18,	// Shaman: "Run, Foster! Run! Hide from the evil!"
	LOADBG,		59497, // loud heli
	LOOPBG,
	WAITFLIRT,
	WAITVOICE,
	FADEDOWN,
	SHOWSCREEN,	CD_19_LOG,
	FADEUP,		CD_19_PAL,
	PLAYVOICE,	CDV_19,		// Joey: "Foster! (zzzt) H-Help!"
	WAITVOICE,
	PLAYVOICE,	CDV_20,		// Joey: "Better make my next body move faster, Foster..."
	FADEDOWN,
	SHOWSCREEN,	CD_20_LOG,
	FADEUP,		CD_19_PAL,
	WAITVOICE,
	LOADBG,		59496, // quiet heli
	LOOPBG,
	PLAYVOICE,	CDV_21,		// Foster: "He was only a robot, but, well, I loved the little guy."
	FADEDOWN,
	SHOWSCREEN,	CD_21_LOG,
	FADEUP,		CD_19_PAL,
	WAITVOICE,
	PLAYVOICE,	CDV_22,		// Foster: "Then, as suddenly as it started, the shooting stopped."
	LOADBG,		59494, // heli whine
	PLAYBG,
	WAITVOICE,
	PLAYVOICE,	CDV_23,		// Foster: "There was a moment's silence as the copter cut its rotors, then..."
	/* fade down while Foster's saying his line */
	FADEDOWN,
	WAITVOICE,
	SHOWSCREEN,	CD_24_LOG,
	FADEUP,		CD_23_PAL,
	PLAYVOICE,	CDV_24,		// Reich: "Whoever is in charge here, come forward..."
	WAITVOICE,
	PLAYVOICE,	CDV_25,		// Reich: "Now!!"
	WAITVOICE,
	PLAYVOICE,	CDV_26,		// Foster: "Only a fool would have argued with that firepower."
	WAITVOICE,
	FADEDOWN,
	SHOWSCREEN,	CD_27_LOG,
	FADEUP,		CD_27_PAL,
	PLAYVOICE,	CDV_27,		// Shaman: "... I am the leader of these people... We are peaceful..."
	WAITVOICE,
	PLAYVOICE,	CDV_29,		// Reich: "Bring him here."
	WAITVOICE,
	PLAYVOICE,	CDV_30,		// Guard: "At once, Commander Reich."
	WAITVOICE,
	BGFLIRT,	CD_27,
		PLAYVOICE,	CDV_31,	// Reich: "We're looking for someone..."
		WAITVOICE,
		PLAYVOICE,	CDV_32,	// Reich: "Someone who doesn't belong here..."
		WAITVOICE,
		PLAYVOICE,	CDV_33,	// Reich: "Who wasn't born in this garbage dump..."
		WAITVOICE,
		PLAYVOICE,	CDV_34,	// Reich: "Who came from the city as a child..."
	WAITFLIRT,
	WAITVOICE,
	PLAYVOICE,	CDV_35,		// Reich: "We want to take him home again."
	WAITVOICE,
	PLAYVOICE,	CDV_36,		// Foster: "My mind racing, I remembered where I'd seen that symbol before..."
		FADEDOWN,
		SHOWSCREEN,	CD_35_LOG,
		FADEUP,		CD_35_PAL,
	WAITVOICE,
	PLAYVOICE,	CDV_37,		// Foster: "It was the day the tribe found me..."
		DOFLIRT,	CD_35,
	WAITVOICE,
	PLAYVOICE,	CDV_38,		// Foster: "The day of the crash..."
		DOFLIRT,	CD_37,
	WAITVOICE,
	PLAYVOICE,	CDV_39,		// Foster: "The day my mother died."
	WAITVOICE,
	FADEDOWN,
	SHOWSCREEN,	CD_40_LOG,
	FADEUP,		CD_40_PAL,
	PLAYVOICE,	CDV_40,		// Shaman: "You alright, city boy?"
	WAITVOICE,
	PLAYVOICE,	CDV_41,		// Shaman: "Got a name, son?"
	WAITVOICE,
	PLAYVOICE,	CDV_42,		// Foster: "R-Robert."
	WAITVOICE,
	FADEDOWN,
	SHOWSCREEN,	CD_43_LOG,
	FADEUP,		CD_43_PAL,
	PLAYVOICE,	CDV_43,		// Shaman: "Hah! Welcome to the Gap, Robert!"
	WAITVOICE,
	DOFLIRT,	CD_43,
	PLAYVOICE,	CDV_45,		// Foster: "As he patched me up, the old man had gently explained that there was no way back into the City..."
	FADEDOWN,
	SHOWSCREEN,	CD_45_LOG,
	FADEUP,		CD_45_PAL,
	WAITVOICE,
	PLAYVOICE,	CDV_46,		// Foster: "And I already knew there was nothing he could do for mother."
	DOFLIRT,	CD_45,
	WAITVOICE,
	FADEDOWN,
	SHOWSCREEN,	CD_47_LOG,
	FADEUP,		CD_47_PAL,
	PLAYVOICE,	CDV_47,		// Foster: "His tribe was poor, but they treated me like one of their own..."
	WAITVOICE,
	PLAYVOICE,	CDV_48,		// Foster: "I learned how to survive in the wasteland they called the Gap..."
	FADEDOWN,
	SHOWSCREEN,	CD_48_LOG,
	FADEUP,		CD_48_PAL,
	WAITVOICE,
	BGFLIRT,	CD_48,
		PLAYVOICE,	CDV_49,	// Foster: "And scavenging from the City dumps."
		WAITVOICE,
		PLAYVOICE,	CDV_50,	// Foster: "As the years passed, I forgot my life in the City."
	WAITFLIRT,
	WAITVOICE,
	PLAYVOICE,	CDV_51,		// Foster: "Discovered new talents..."
	BGFLIRT,	CD_49,
		WAITVOICE,
		PLAYVOICE,	CDV_52,	// Foster: "Hah!"
		WAITVOICE,
		PLAYVOICE,	CDV_53,	// Joey: "I'm your (zzzt) friend... call me (zzzt) Joey."
		WAITVOICE,
	WAITFLIRT,
	PLAYVOICE,	CDV_54,		// Foster: "And got a second name."
	DOFLIRT,	CD_50,
	WAITVOICE,
	PLAYVOICE,	CDV_55,		// Shaman: "This is what we'll call you now you've come of age, son."
	WAITVOICE,
	PLAYVOICE,	CDV_56,		// Shaman: "We found you... we fostered you..."
		FADEDOWN,
		SHOWSCREEN,	CD_55_LOG,
		FADEUP,		CD_55_PAL,
	WAITVOICE,
	PLAYVOICE,	CDV_57,		// Shaman: "So that makes you Robert Foster."
	WAITVOICE,
	FADEDOWN,
	SHOWSCREEN,	CD_58_LOG,
	FADEUP,		CD_58_PAL,
	PLAYVOICE,	CDV_58,		// Reich: "...Wasted enough time!"
	WAITVOICE,
	PLAYVOICE,	CDV_59,		// Reich: "Give us the runaway or we'll shoot everyone..."
	WAITVOICE,
	PLAYVOICE,	CDV_60,		// Reich: "Starting with you, grandad!"
	WAITVOICE,
	PLAYVOICE,	CDV_61,		// Foster: "The old man had been right, for once..."
	WAITVOICE,
	PLAYVOICE,	CDV_62,		// Foster: "It was me they wanted."
	BGFLIRT,	CD_58,
		WAITVOICE,
		PLAYVOICE,	CDV_63,	// Shaman: "No, my son! Don't let the evil take you! Run!"
		WAITVOICE,
		PLAYVOICE,	CDV_64,	// Guard: "DNA scan confirms it's him, sir."
	WAITFLIRT,
	WAITVOICE,
	PLAYVOICE,	CDV_65,		// Foster: "Evil had come to the Gap, just as he said."
	FADEDOWN,
	WAITVOICE,
	SHOWSCREEN,	CD_66_LOG,
	FADEUP,		CD_66_PAL,
	PLAYVOICE,	CDV_66,		// Reich: "Take him."
	WAITVOICE,
	PLAYVOICE,	CDV_67,		// Foster: "But had the old man seen why it wanted me?"
		FADEDOWN,
		SHOWSCREEN,	CD_67_LOG,
		FADEUP,		CD_67_PAL,
	WAITVOICE,
	PLAYVOICE,	CDV_68,		// Foster: "Or what it would do next?"
	WAITVOICE,
	PLAYVOICE,	CDV_69,		// Foster: "It was too late to ask him now."
		FADEDOWN,
		SHOWSCREEN,	CD_69_LOG,
		FADEUP,		CD_69_PAL,
	WAITVOICE,
	PLAYVOICE,	CDV_70,		// Guard: "Leaving destruction zone, Commander Reich."
	DOFLIRT,	CD_69,
	WAITVOICE,
	FADEDOWN,
	PLAYVOICE,	CDV_71,		// Reich: "Good. Detonate."
	WAITVOICE,
	SHOWSCREEN,	CD_72_LOG,
	FADEUP,		CD_72_PAL,
	PLAYVOICE,	CDV_72,		// Foster: "Much too late."
	WAITVOICE,
	FADEDOWN,
	SHOWSCREEN,	CD_73_LOG,
	FADEUP,		CD_73_PAL,
	PLAYVOICE,	CDV_73,		// Foster: "Why, you murdering..."
	WAITVOICE,
	PLAYVOICE,	CDV_74,		// Reich: "Keep him quiet."
	WAITVOICE,
	PLAYVOICE,	CDV_75,		// Foster: "All I could do was wait."
	FADEDOWN,
	SHOWSCREEN,	CD_76_LOG,
	FADEUP,		CD_76_PAL,
	WAITVOICE,
	PLAYVOICE,	CDV_76,		// Foster: "Just like on a hunt. Just like the old man taught me."
	WAITVOICE,
	PLAYVOICE,	CDV_77,		// Foster: "Wait... and be ready."
	WAITVOICE,
	FADEDOWN,
	SHOWSCREEN,	CD_78_LOG,
	FADEUP,		CD_78_PAL,
	PLAYVOICE,	CDV_78,		// Foster: "It was dawn when we reached the City."
	WAITVOICE,
	PLAYVOICE,	CDV_79,		// Reich: "Land in the central Security compound."
	WAITVOICE,
	PLAYVOICE,	CDV_80,		// Foster: "A dawn my tribe would never see."
	BGFLIRT,	CD_100,
		WAITVOICE,
		PLAYVOICE,	CDV_81,	// Foster: "They were no more than a note in Reich's book now."
		WAITVOICE,
		PLAYVOICE,	CDV_82,	// Guard: "Yes, sir. Locking on automatic landing beacon."
		WAITVOICE,
	WAITFLIRT,
	SHOWSCREEN,	CD_101_LOG,
	BGFLIRT,	CD_101,
		PLAYVOICE,	CDV_83,	// Foster: "But what was I? Why did..."
		WAITVOICE,
		PLAYVOICE,	CDV_84,	// Guard: "Sir! The guidance system! It's gone crazy!"
		WAITVOICE,
		PLAYVOICE,	CDV_85,	// Guard: "We're going to HIT!"
		WAITVOICE,
	WAITFLIRT,
	SHOWSCREEN,	CD_102_LOG,
	PLAYVOICE,	CDV_86,		// Foster: "Maybe I'd get some answers now."
	DOFLIRT,	CD_102,
	FADEDOWN,
	SHOWSCREEN,	CD_103_LOG,
	FADEUP,		CD_103_PAL,
	BGFLIRT,	CD_103,
	WAITVOICE,
	PLAYVOICE,	CDV_87,		// Foster: "If I survived another 'copter crash."
	WAITFLIRT,
	WAITVOICE,
	STARTMUSIC,	2,
	FADEDOWN,
	SHOWSCREEN,	CD_104_LOG,
	FADEUP,		CD_104_PAL,
	DOFLIRT,	CD_104,
	DOFLIRT,	CD_105,
	SEQEND
};

uint16 Intro::_floppyIntroSeq[] = {
	SHOWSCREEN,   60081,
	FADEUP,       60080,
	DOFLIRT,      60082,
	DOFLIRT,      60083,
	DOFLIRT,      60084, // Beneath a Steel Sky
	DOFLIRT,      60085,
	DOFLIRT,      60086,
	SCROLLFLIRT,
	COMMANDFLIRT, 60087, // => command list 4a
		136, IC_MAKE_SOUND,  1, 70,
		 90, IC_FX_VOLUME,  80,
		 50, IC_FX_VOLUME,  90,
		  5, IC_FX_VOLUME, 100,
	COMMANDEND,
	SHOWSCREEN,   60088,
	COMMANDFLIRT, 60089, // => command list 4b (cockpit)
		1000, IC_PREPARE_TEXT,  77,
		 220, IC_SHOW_TEXT,     20, 160, // radar detects jamming signal
		 105, IC_REMOVE_TEXT,
		 105, IC_PREPARE_TEXT,  81,
		 105, IC_SHOW_TEXT,    170,  86, // well switch to override you fool
		  35, IC_REMOVE_TEXT,
		  35, IC_PREPARE_TEXT, 477,
		  35, IC_SHOW_TEXT,     30, 160,
		   3, IC_REMOVE_TEXT,
	COMMANDEND,
	SHOWSCREEN,   60090,
	COMMANDFLIRT, 60091, // => command list 4c
		1000, IC_FX_VOLUME, 100,
		  25, IC_FX_VOLUME, 110,
		  15, IC_FX_VOLUME, 120,
		   4, IC_FX_VOLUME, 127,
	COMMANDEND,
	FADEDOWN,
	SHOWSCREEN,  60093,
	FADEUP,       60092,
	COMMANDFLIRT, 60094, // => command list 5
		31, IC_MAKE_SOUND, 2, 127,
	COMMANDEND,
	WAITMUSIC,
	FADEDOWN,
	SHOWSCREEN,   60096,
	STARTMUSIC,       2,
	FADEUP,       60095,
	COMMANDFLIRT, 60097, // => command list 6a
		1000, IC_PREPARE_TEXT, 478,
		  13, IC_SHOW_TEXT,    175, 155,
	COMMANDEND,
	COMMANDFLIRT, 60098, // => command list 6b
		131, IC_REMOVE_TEXT,
		131, IC_PREPARE_TEXT, 479,
		 74, IC_SHOW_TEXT,    175, 155,
		 45, IC_REMOVE_TEXT,
		 45, IC_PREPARE_TEXT, 162,
		 44, IC_SHOW_TEXT,    175, 155,
		  4, IC_REMOVE_TEXT,
	COMMANDEND,
	SEQEND
};

Intro::Intro(Disk *disk, Screen *screen, MusicBase *music, Sound *sound, Text *text, Audio::Mixer *mixer, OSystem *system) {
	_skyDisk = disk;
	_skyScreen = screen;
	_skyMusic = music;
	_skySound = sound;
	_skyText = text;
	_mixer = mixer;
	_system = system;
	_textBuf = (uint8 *)malloc(10000);
	_saveBuf = (uint8 *)malloc(10000);
	_bgBuf = NULL;
	_relDelay = 0;
}

Intro::~Intro() {
	if (_skyScreen->sequenceRunning())
		_skyScreen->stopSequence();

	free(_textBuf);
	free(_saveBuf);
	_mixer->stopID(SOUND_BG);
	free(_bgBuf);
}

bool Intro::doIntro(bool floppyIntro) {
	if (!SkyEngine::isCDVersion())
		floppyIntro = true;

	_skyMusic->loadSection(0);
	_skySound->loadSection(0);

	if (!escDelay(3000))
		return false;

	if (floppyIntro)
		_skyMusic->startMusic(1);

	uint16 *seqData = _mainIntroSeq;
	while (*seqData != SEQEND) {
		if (!nextPart(seqData))
			return false;
	}
	if (floppyIntro)
		seqData = _floppyIntroSeq;
	else
		seqData = _cdIntroSeq;

	while (*seqData != SEQEND) {
		if (!nextPart(seqData))
			return false;
	}
	return true;
}

bool Intro::nextPart(uint16 *&data) {
	uint8 *vData = NULL;
	Audio::RewindableAudioStream *stream = 0;

	// return false means cancel intro
	uint16 command = *data++;
	switch (command) {
	case SHOWSCREEN:
		_skyScreen->showScreen(*data++);
		return true;
	case FADEUP:
		_skyScreen->paletteFadeUp(*data++);
		_relDelay += 32 * 20; // hack: the screen uses a seperate delay function for the
							  // blocking fadeups. So add 32*20 msecs to out delay counter.
		return true;
	case FADEDOWN:
		_skyScreen->fnFadeDown(0);
		_relDelay += 32 * 20; // hack: see above.
		return true;
	case DELAY:
		if (!escDelay(*data++))
			return false;
		return true;
	case DOFLIRT:
		_skyScreen->startSequence(*data++);
		while (_skyScreen->sequenceRunning())
			if (!escDelay(50))
				return false;
		return true;
	case SCROLLFLIRT:
		return floppyScrollFlirt();
	case COMMANDFLIRT:
		return commandFlirt(data);
	case STOPFLIRT:
		_skyScreen->stopSequence();
		return true;
	case STARTMUSIC:
		_skyMusic->startMusic(*data++);
		return true;
	case WAITMUSIC:
		while (_skyMusic->musicIsPlaying())
			if (!escDelay(50))
				return false;
		return true;
	case BGFLIRT:
		_skyScreen->startSequence(*data++);
		return true;
	case WAITFLIRT:
		while (_skyScreen->sequenceRunning())
			if (!escDelay(50))
				return false;
		return true;
	case PLAYVOICE:
		if (!escDelay(200))
			return false;
		vData = _skyDisk->loadFile(*data++);
		// HACK: Fill the header with silence. We should
		// probably use _skySound instead of calling playStream()
		// directly, but this will have to do for now.
		memset(vData, 127, sizeof(DataFileHeader));

		stream = Audio::makeRawStream(vData, _skyDisk->_lastLoadedFileSize, 11025, Audio::FLAG_UNSIGNED);
		_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_voice, stream, SOUND_VOICE);
		return true;
	case WAITVOICE:
		while (_mixer->isSoundHandleActive(_voice))
			if (!escDelay(50))
				return false;
		return true;
	case LOADBG:
		_mixer->stopID(SOUND_BG);
		free(_bgBuf);
		_bgBuf = _skyDisk->loadFile(*data++);
		_bgSize = _skyDisk->_lastLoadedFileSize;
		return true;
	case LOOPBG:
		_mixer->stopID(SOUND_BG);
		stream = Audio::makeRawStream(_bgBuf + 256, _bgSize - 768, 11025, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_bgSfx, Audio::makeLoopingAudioStream(stream, 0), SOUND_BG);
		return true;
	case PLAYBG:
		_mixer->stopID(SOUND_BG);
		stream = Audio::makeRawStream(_bgBuf + 256, _bgSize - 768, 11025, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_bgSfx, stream, SOUND_BG);
		return true;
	case STOPBG:
		_mixer->stopID(SOUND_BG);
		return true;
	default:
		error("Unknown intro command %X", command);
	}
	return true;
}

bool Intro::floppyScrollFlirt() {
	uint8 *scrollScreen = (uint8 *)malloc(FRAME_SIZE * 2);
	memset(scrollScreen, 0, FRAME_SIZE);
	memcpy(scrollScreen + FRAME_SIZE, _skyScreen->giveCurrent(), FRAME_SIZE);
	uint8 *scrollPos = scrollScreen + FRAME_SIZE;
	uint8 *vgaData = _skyDisk->loadFile(60100);
	uint8 *diffData = _skyDisk->loadFile(60101);
	uint16 frameNum = READ_LE_UINT16(diffData);
	uint8 *diffPtr = diffData + 2;
	uint8 *vgaPtr = vgaData;
	bool doContinue = true;

	for (uint16 frameCnt = 1; (frameCnt < frameNum) && doContinue; frameCnt++) {
		uint8 scrollVal = *diffPtr++;
		if (scrollVal)
			scrollPos -= scrollVal * GAME_SCREEN_WIDTH;

		uint16 scrPos = 0;
		while (scrPos < FRAME_SIZE) {
			uint8 nrToDo, nrToSkip;
			do {
				nrToSkip = *diffPtr++;
				scrPos += nrToSkip;
			} while (nrToSkip == 255);
			do {
				nrToDo = *diffPtr++;
				memcpy(scrollPos + scrPos, vgaPtr, nrToDo);
				scrPos += nrToDo;
				vgaPtr += nrToDo;
			} while (nrToDo == 255);
		}
		_system->copyRectToScreen(scrollPos, GAME_SCREEN_WIDTH, 0, 0, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
		_system->updateScreen();
		if (!escDelay(60))
			doContinue = false;
	}
	memcpy(_skyScreen->giveCurrent(), scrollPos, FRAME_SIZE);
	free(diffData);
	free(vgaData);
	free(scrollScreen);
	return doContinue;
}

bool Intro::commandFlirt(uint16 *&data) {
	_skyScreen->startSequence(*data++);

	while ((*data != COMMANDEND) || _skyScreen->sequenceRunning()) {
		while ((_skyScreen->seqFramesLeft() < *data)) {
			data++;
			uint16 command = *data++;
			switch (command) {
			case IC_PREPARE_TEXT:
				_skyText->displayText(*data++, _textBuf, true, INTRO_TEXT_WIDTH, 255);
				break;
			case IC_SHOW_TEXT:
				((DataFileHeader *)_textBuf)->s_x = *data++;
				((DataFileHeader *)_textBuf)->s_y = *data++;
				showTextBuf();
				break;
			case IC_REMOVE_TEXT:
				restoreScreen();
				break;
			case IC_MAKE_SOUND:
				_skySound->playSound(data[0], data[1], 0);
				data += 2;
				break;
			case IC_FX_VOLUME:
				_skySound->playSound(1, *data++, 0);
				break;
			default:
				error("Unknown FLIRT command %X", command);
			}
		}

		if (!escDelay(50)) {
			_skyScreen->stopSequence();
			return false;
		}
	}

	data++; // move pointer over "COMMANDEND"
	return true;
}

void Intro::showTextBuf() {
	uint16 x = ((DataFileHeader *)_textBuf)->s_x;
	uint16 y = ((DataFileHeader *)_textBuf)->s_y;
	uint16 width = ((DataFileHeader *)_textBuf)->s_width;
	uint16 height = ((DataFileHeader *)_textBuf)->s_height;
	uint8 *screenBuf = _skyScreen->giveCurrent() + y * GAME_SCREEN_WIDTH + x;
	memcpy(_saveBuf, _textBuf, sizeof(DataFileHeader));
	uint8 *saveBuf = _saveBuf + sizeof(DataFileHeader);
	uint8 *textBuf = _textBuf + sizeof(DataFileHeader);
	for (uint16 cnty = 0; cnty < height; cnty++) {
		memcpy(saveBuf, screenBuf, width);
		for (uint16 cntx = 0; cntx < width; cntx++)
			if (textBuf[cntx])
				screenBuf[cntx] = textBuf[cntx];
		screenBuf += GAME_SCREEN_WIDTH;
		textBuf += width;
		saveBuf += width;
	}
	screenBuf = _skyScreen->giveCurrent() + y * GAME_SCREEN_WIDTH + x;
	_system->copyRectToScreen(screenBuf, GAME_SCREEN_WIDTH, x, y, width, height);
}

void Intro::restoreScreen() {
	uint16 x = ((DataFileHeader *)_saveBuf)->s_x;
	uint16 y = ((DataFileHeader *)_saveBuf)->s_y;
	uint16 width = ((DataFileHeader *)_saveBuf)->s_width;
	uint16 height = ((DataFileHeader *)_saveBuf)->s_height;
	uint8 *screenBuf = _skyScreen->giveCurrent() + y * GAME_SCREEN_WIDTH + x;
	uint8 *saveBuf = _saveBuf + sizeof(DataFileHeader);
	for (uint16 cnt = 0; cnt < height; cnt++) {
		memcpy(screenBuf, saveBuf, width);
		screenBuf += GAME_SCREEN_WIDTH;
		saveBuf += width;
	}
	_system->copyRectToScreen(_saveBuf + sizeof(DataFileHeader), width, x, y, width, height);
}

bool Intro::escDelay(uint32 msecs) {
	Common::EventManager *eventMan = _system->getEventManager();
	Common::Event event;

	if (_relDelay == 0) // first call, init with system time
		_relDelay = (int32)_system->getMillis();

	_relDelay += msecs; // now wait until _system->getMillis() >= _relDelay

	int32 nDelay = 0;
	do {
		while (eventMan->pollEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN) {
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					return false;
			} else if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RTL) {
				return false;
			}
		}
		nDelay = _relDelay - _system->getMillis();
		if (nDelay < 0)
			nDelay = 0;
		else if (nDelay > 20)
			nDelay = 20;

		_system->delayMillis(nDelay);

		_skyScreen->processSequence();
		_system->updateScreen();
	} while (nDelay == 20);

	return true;
}

} // End of namespace Sky
