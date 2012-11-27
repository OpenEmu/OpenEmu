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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "teenagent/teenagent.h"
#include "teenagent/scene.h"
#include "teenagent/inventory.h"
#include "teenagent/resources.h"
#include "teenagent/dialog.h"

#include "common/textconsole.h"

namespace TeenAgent {

#define CHECK_FLAG(addr, v) (res->dseg.get_byte(addr) == (v))
#define SET_FLAG(addr, v) (res->dseg.set_byte((addr), (v)))
#define GET_FLAG(addr) (res->dseg.get_byte(addr))
#define INC_FLAG(addr) (++*res->dseg.ptr(addr))

void TeenAgentEngine::fnIntro() {
	hideActor();

	loadScene(41, 139, 156, 3);
	playSound(41, 12);
	playAnimation(912, 1);
	setOns(0, 108);
	playSound(62, 8);
	playSound(58, 40);
	playAnimation(913, 1);
	setOns(1, 109);
	setLan(2, 1);
	dialog->show(192, scene, 914, 915, textColorGoldDriver, textColorBankGuard, 2, 1);
	displayCredits(dsAddr_introCredits1);

	loadScene(42, 139, 156, 3);
	playSound(15, 20);
	playAnimation(916, 1);
	playSound(40, 18);
	playSound(40, 22);
	for (byte i = 27; i < 37; i += 2)
		playSound(40, i);
	playSound(29, 44);
	playAnimation(918, 0, true);
	playAnimation(917, 1, true);
	waitAnimation();
	displayCredits(dsAddr_introCredits2);

	loadScene(40, 139, 156, 3);
	playMusic(3);
	dialog->show(193, scene, 920, 924, textColorRGBBoss, textColorFortuneTeller, 1, 2);
	playSound(26, 50);
	playAnimation(925, 0, true);
	playAnimation(926, 1, true);
	waitAnimation();
	dialog->show(194, scene, 927, 920, textColorFortuneTeller, textColorRGBBoss, 2, 1);
	displayCredits(dsAddr_introCredits3);

	loadScene(39, 139, 156, 3);
	playMusic(11);
	playSound(81, 2);
	playSound(81, 8);
	playSound(81, 11);
	playSound(81, 14);
	playSound(81, 16);
	playSound(81, 18);
	playSound(81, 20);
	playSound(81, 21);
	playAnimation(928, 1);
	setOns(0, 112);
	dialog->showMono(195, scene, 929, textColorMark, 1);
	showActor();
	moveTo(319, 150, 1, true);
	moveTo(63, 150, 1);
	displayAsyncMessage(dsAddr_HeyWtmQMsg, 4, 62, 18, 36); // hey, what's the matter?
	playAnimation(851, 0, true);
	playActorAnimation(930, true);
	waitAnimation();
	playSound(24, 11);
	playActorAnimation(931);

	displayCredits(dsAddr_introCredits4);

	playMusic(3);
	loadScene(40, 50, 186, 1);
	setOns(0, 113);
	dialog->show(196, scene, 919, 0, textColorRGBBoss, textColorMark, 1, 0);
	moveTo(196, 186, 1);
	dialog->show(197, scene, 0, 920, textColorMark, textColorRGBBoss, 0, 1);
	playActorAnimation(932);
	dialog->show(198, scene, 0, 920, textColorMark, textColorRGBBoss, 0, 1);
	playActorAnimation(932);
	dialog->show(199, scene, 0, 920, textColorMark, textColorRGBBoss, 0, 1);
	playActorAnimation(932);
	dialog->show(200, scene, 0, 922, textColorMark, textColorRGBBoss, 0, 1);
	playActorAnimation(933);
	dialog->show(201, scene, 0, 920, textColorMark, textColorRGBBoss, 0, 1);
	moveTo(174, 186, 1);
	playAnimation(851, 0, true);
	playActorAnimation(934, true);
	waitAnimation();
	loadScene(10, 136, 153, 3);
}

void TeenAgentEngine::fnPoleClimbFail() {
	moveTo(86, 195, 1, true);
	playActorAnimation(868);
}

void TeenAgentEngine::fnGotAnchor() {
	SET_FLAG(dsAddr_timedCallbackState, 0);
	setTimerCallback(0, 0);
	scene->getActorAnimation()->free();
	playSound(64, 7);
	playActorAnimation(618);
	disableObject(5);
	setOns(0, 0);
	playSound(31, 1);
	playActorAnimation(619);
	fnGetOutOfLake();
	inventory->add(kInvItemAnchor);
	displayMessage(dsAddr_hookedAnchorMsg); // "I was really hooked on this anchor!"
}

void TeenAgentEngine::fnGetOutOfLake() {
	loadScene(15, 156, 180, 3);
	playSound(5, 5);
	playSound(38, 14);
	playSound(38, 20);
	playSound(5, 25);
	playActorAnimation(616);
}

void TeenAgentEngine::fnGuardDrinking() {
	SET_FLAG(dsAddr_timedCallbackState, 0);
	setTimerCallback(0, 0);
	scene->getAnimation(0)->free();
	SET_FLAG(dsAddr_scaredGuardAlreadyFlag, 1);

	displayAsyncMessage(dsAddr_BooMsg, 300, 130, 1, 5); // "Booo!"
	setOns(0, 16);
	enableObject(2);

	playSound(17, 5);
	playAnimation(545, 0);

	dialog->show(5, scene, 0, 546, textColorMark, textColorMansionGuard, 0, 1);
	SET_FLAG(dsAddr_spokenWithMansionGuardFlag, 1);
	SET_FLAG(dsAddr_haveNotSpokenWithMansionGuardFlag, 0);
}

void TeenAgentEngine::fnEgoDefaultPosition() {
	if (scene->getPosition().y <= 149)
		moveTo(94, 115, 4);
	else
		moveTo(51, 149, 4);
}

void TeenAgentEngine::fnEnterCave() {
	loadScene(24, 230, 170, 1);
	playSound(52, 3);
	playSound(52, 7);
	playSound(52, 11);
	playSound(52, 14);
	playSound(52, 18);
	playSound(52, 21);
	playSound(52, 25);
	playActorAnimation(601);
	moveTo(230, 179, 3);
	if (!CHECK_FLAG(dsAddr_lightOnFlag, 1))
		displayMessage(dsAddr_kindaDarkMsg); // "It's kinda dark here"
}

void TeenAgentEngine::fnEgoScaredBySpider() {
	if (CHECK_FLAG(dsAddr_egoAlreadyScaredBySpiderFlag, 1)) {
		fnMoveToLadderAndLeaveCellar();
		dialog->showMark(75, scene);
	} else {
		dialog->showMark(73, scene);
		fnMoveToLadderAndLeaveCellar();
		wait(100);
		dialog->showMark(74, scene);
		SET_FLAG(dsAddr_egoAlreadyScaredBySpiderFlag, 1);
	}
}

void TeenAgentEngine::fnMoveToLadderAndLeaveCellar() {
	Object *objTemp = scene->getObject(3);
	moveTo(objTemp);
	fnLeaveCellar();
	moveTo(48, 190, 3);
}

void TeenAgentEngine::fnLeaveCellar() {
	playSound(52, 10);
	playSound(52, 14);
	playSound(52, 18);
	playSound(52, 21);
	playSound(52, 25);
	playSound(52, 28);
	playSound(52, 32);
	playActorAnimation(600);
	loadScene(21, 297, 178, 3);
}

void TeenAgentEngine::fnPutRockInHole() {
	if (CHECK_FLAG(dsAddr_timedCallbackState, 0)) {
		playSound(5, 2);
		playSound(15, 12);
		playActorAnimation(638);
		inventory->remove(kInvItemMouse);
		setTimerCallback(csAddr_mouseOutOfHoleTimeout, 100);
		SET_FLAG(dsAddr_timedCallbackState, 1);
	} else if (CHECK_FLAG(dsAddr_timedCallbackState, 1)) {
		playSound(5, 2);
		playSound(52, 13);
		playActorAnimation(648);
		setOns(1, 46);
		inventory->remove(kInvItemRock);
		setTimerCallback(csAddr_mouseOutOfHoleTimeout, 100);
		SET_FLAG(dsAddr_timedCallbackState, 2);
	} else if (CHECK_FLAG(dsAddr_timedCallbackState, 2)) {
		playActorAnimation(649);
		setOns(1, 47);
		wait(300);
		for (byte i = 1; i <= 37; i += 4)
			playSound(68, i);
		playAnimation(639, 2);
		setOns(0, 42);
		enableObject(6);
		disableObject(5);
		SET_FLAG(dsAddr_mouseGotGoldNuggetFlag, 1);
		SET_FLAG(dsAddr_timedCallbackState, 0);
		setTimerCallback(0, 0);
	}
}

void TeenAgentEngine::fnEgoBottomRightTurn() {
	Common::Point p = scene->getPosition();
	if (p.x == 208 && p.y == 151)
		moveRel(0, 0, 2);
	else
		moveTo(208, 151, 1);
}

bool TeenAgentEngine::fnCheckingDrawers() {
	uint16 v = GET_FLAG(dsAddr_drawerPuzzleBookValue) - 1;
	if (GET_FLAG(dsAddr_blueDrawerOpenFlag + v) != 1)
		return false;
	else {
		uint16 sum = 0;
		for (uint i = 0; i < 6; ++i)
			sum += GET_FLAG(dsAddr_blueDrawerOpenFlag + i);
		if (sum != 1)
			return false;
		else
			return true;
	}
}

void TeenAgentEngine::fnDrawerOpenMessage() {
	if (CHECK_FLAG(dsAddr_drawerPuzzleHintGivenFlag, 1))
		displayMessage(dsAddr_drawerOpenMsg); // "I cannot open the drawer if the next one is open!"
	else {
		displayMessage(dsAddr_strangeDrawerMsg); // "Strange, but the drawer is stuck if the next drawer is open"
		displayMessage(dsAddr_notOrdinaryDrawersMsg); // "Maybe these are not just ordinary drawers!"
		SET_FLAG(dsAddr_drawerPuzzleHintGivenFlag, 1);
	}
}

bool TeenAgentEngine::fnRobotSafeAlreadyUnlockedCheck() {
	if (CHECK_FLAG(dsAddr_MansionRobotSafeUnlockedFlag, 1)) {
		return true;
	} else {
		displayMessage(dsAddr_noReasonMsg); // "There's no reason to do it"
		return false;
	}
}

void TeenAgentEngine::fnRobotSafeUnlockCheck() {
	if (CHECK_FLAG(dsAddr_MansionRobotSafeVoiceTestPassedFlag, 1) && 
	    CHECK_FLAG(dsAddr_MansionRobotSafeScentTestPassedFlag, 1) &&
	    CHECK_FLAG(dsAddr_MansionRobotSafeViewTestPassedFlag, 1)) {
		waitLanAnimationFrame(1, 1);
		playSound(89, 2);
		playActorAnimation(731);
		setOns(0, 70);
		setLan(1, 0);
		disableObject(1);
		enableObject(2);
		enableObject(3);
	}
}

bool TeenAgentEngine::fnMansionIntrusionAttempt() {
	wait(50);
	byte attempts = res->dseg.get_byte(dsAddr_mansionEntryCount) + 1;
	res->dseg.set_byte(dsAddr_mansionEntryCount, attempts);
	debugC(0, kDebugCallbacks, "mansion intrusion attempt #%u", attempts);
	if (attempts >= 7)
		return false;
	else {
		byte id = scene->getId();

		playMusic(11);
		displayCutsceneMessage(dsAddr_cutsceneMsg2, 84, 95); // "Meanwhile in the mansion"
		switch (attempts) {
		case 2:
			fnSecondMansionIntrusion();
			break;
		case 3:
			fnThirdMansionIntrusion();
			break;
		case 4:
			fnFourthMansionIntrusion();
			break;
		case 5:
			fnFifthMansionIntrusion();
			break;
		case 6:
			fnSixthMansionIntrusion();
			break;
		default:
			error("mansion intrusion attempts out of range!");
			break;
		}
		playMusic(6);
		if (getFlag(dsAddr_johnNotyOutsideMansionDoorFlag) != 1 || attempts != 6)
			loadScene(id, scene->getPosition());
		return true;
	}
}

void TeenAgentEngine::fnSecondMansionIntrusion() {
	hideActor();
	loadScene(34, scene->getPosition());
	playAnimation(986, 0, true);
	playAnimation(987, 1, true);
	waitAnimation();
	dialog->show(178, scene, 988, 989, textColorMansionGuard, textColorJohnNoty, 1, 2);
	playAnimation(990, 0, true);
	playAnimation(991, 1, true);
	waitAnimation();
	showActor();
}

void TeenAgentEngine::fnThirdMansionIntrusion() {
	hideActor();
	loadScene(30, scene->getPosition());
	playAnimation(887, 1);
	playAnimation(888, 2, true, true, true);
	//waitAnimation();
	dialog->showMono(179, scene, 889, textColorMansionGuard, 2);
	playSound(26, 3);
	playAnimation(891, 1, true, true, true);
	playAnimation(892, 2);
	waitAnimation();
	dialog->show(180, scene, 890, 889, textColorJohnNoty, textColorMansionGuard, 3, 2);
	showActor();
}

void TeenAgentEngine::fnFourthMansionIntrusion() {
	hideActor();
	loadScene(32, scene->getPosition());
	playAnimation(894, 1, true, true, true);
	playAnimation(893, 2, true);
	waitAnimation();
	dialog->showMono(181, scene, 895, textColorMansionGuard, 3);
	playSound(75, 9);
	playAnimation(898, 1, true);
	playAnimation(897, 2, true);
	dialog->show(182, scene, 896, 895, textColorJohnNoty, textColorMansionGuard, 2, 3);
	showActor();
}

void TeenAgentEngine::fnFifthMansionIntrusion() {
	hideActor();
	loadScene(29, scene->getPosition());
	playActorAnimation(901, true);
	playAnimation(900, 1, true);
	waitAnimation();
	dialog->show(183, scene, 903, 902, textColorJohnNoty, textColorMansionGuard, 2, 3);
	for (byte i = 3; i <= 9; i += 2)
		playSound(56, i);

	playActorAnimation(905, true);
	playAnimation(904, 1, true);
	dialog->show(184, scene, 903, 902, textColorJohnNoty, textColorMansionGuard, 2, 3);
	showActor();
}

void TeenAgentEngine::fnSixthMansionIntrusion() {
	hideActor();
	loadScene(35, scene->getPosition());
	playAnimation(907, 2, true);
	playAnimation(906, 3, true);
	waitAnimation();
	dialog->show(185, scene, 908, 909, textColorMansionGuard, textColorJohnNoty, 2, 3);
	dialog->show(186, scene, 910, 908, textColorJohnNoty, textColorMansionGuard, 3, 2);
	loadScene(11, scene->getPosition());
	showActor();
	setOns(3, 51);
	playAnimation(911, 1);
	playAnimation(899, 1);
	setFlag(dsAddr_johnNotyOutsideMansionDoorFlag, 1);
	reloadLan();
	wait(200);
	enableObject(8);
	setLan(2, 8);
}

void TeenAgentEngine::fnTooDark() {
	displayMessage(dsAddr_TooDarkMsg); // "It's too dark to see clearly"
}

bool TeenAgentEngine::fnIsCookGone() {
	if (CHECK_FLAG(dsAddr_MansionCookGoneFlag, 1)) {
		return true;
	} else {
		displayMessage(dsAddr_cookAroundMsg); // "I can't do anything with this cook around"
		return false;
	}
}

void TeenAgentEngine::fnEgoSuspiciousPosition() {
	Common::Point p = scene->getPosition();
	if (p.x != 203 && p.y != 171)
		moveTo(203, 169, 2);
	else
		moveTo(203, 169, 1);
}

void TeenAgentEngine::fnGivingFlowerToOldLady() {
	playSound(5, 2);
	dialog->show(37, scene, 0, 523, textColorMark, textColorOldLady, 0, 1);
	playActorAnimation(537, true);
	playAnimation(538, 0, true);
	waitAnimation();
	wait(100);
	dialog->show(38, scene, 0, 523, textColorMark, textColorOldLady, 0, 1);
}

void TeenAgentEngine::fnGiveAnotherFlowerToOldLady() {
		dialog->pop(scene, dsAddr_dialogStackOldLady, 0, 523, textColorMark, textColorOldLady, 0, 1);
}

void TeenAgentEngine::fnGivingFlowerToAnne() {
	dialog->show(53, scene, 0, 524, textColorMark, textColorAnne, 0, 2);
	playSound(5, 10);
	playActorAnimation(540, true);
	playAnimation(539, 1, true);
	waitAnimation();
	wait(100);
	dialog->show(54, scene, 0, 524, textColorMark, textColorAnne, 0, 2);
	wait(50);
	dialog->show(55, scene, 0, 524, textColorMark, textColorAnne, 0, 2);
	dialog->show(56, scene, 0, 524, textColorMark, textColorAnne, 0, 2);
	wait(50);
	moveRel(0, 1, 0);
	dialog->show(57, scene, 0, 524, textColorMark, textColorAnne, 0, 2);
	moveRel(0, -1, 0);
	wait(50);
}

void TeenAgentEngine::fnGiveAnotherFlowerToAnne() {
	dialog->pop(scene, dsAddr_dialogStackAnotherFlowerToAnne, 0, 524, textColorMark, textColorAnne, 0, 2);
}

void TeenAgentEngine::rejectMessage() {
	uint i = _rnd.getRandomNumber(3);
	switch (i) {
	case 0:
		displayMessage(dsAddr_rejectMsg0); // "I have no idea what to do with it"
		break;
	case 1:
		displayMessage(dsAddr_rejectMsg1); // "I can't imagine what I could do with this"
		break;
	case 2:
		displayMessage(dsAddr_rejectMsg2); // "I can't figure out what I should do with this"
		break;
	case 3:
		displayMessage(dsAddr_rejectMsg3); // "I can't find any reason to mess with it"
		break;
	default:
		error("rejectMessage() index out of range");
		break;
	}
}

bool TeenAgentEngine::processCallback(uint16 addr) {
	if (addr == 0)
		return false;

	debugC(0, kDebugCallbacks, "processCallback(%04x)", addr);

	bool retVal = true;
	switch (addr) {
	case csAddr_intro: // intro
		fnIntro();
		break;

	case 0x3fed:
		loadScene(3, Common::Point(305, 104));
		scene->setOrientation(4);
		break;

	case 0x4007:
		loadScene(5, Common::Point(300, 131));
		scene->setOrientation(3);
		break;

	case 0x4021:
		// pulling out mysterious object
		if (CHECK_FLAG(dsAddr_cutFenceFlag, 1)) {
			playActorAnimation(844);
			playActorAnimation(846);
			playActorAnimation(845);
			displayMessage(dsAddr_pullObjMsg1); // "I can't pull it out"
		} else {
			displayMessage(dsAddr_pullObjMsg2); // "I can't reach it"
		}
		break;

	case 0x4048:
		displayMessage(dsAddr_dontWantToTouchMsg); // "I don't want to touch it - I might get hurt"
		break;

	case 0x404f:
		displayMessage(dsAddr_notWantToSleepMsg); // "I don't want to sleep"
		break;

	case 0x4056:
		// FIXME - This is the bird use callback in the first act at 
		//         the mudpool. Current Code based on behaviour. Need to analyse cseg data.
		dialog->popMark(scene, dsAddr_dialogStackMudpoolBird);
		break;

	case 0x4060:
		loadScene(2, Common::Point(28, 180));
		scene->setOrientation(2);
		break;

	case 0x407a:
		loadScene(4, Common::Point(297, 128));
		scene->setOrientation(4);
		break;

	case 0x4094: // climbing to the pole near mudpool
		if (CHECK_FLAG(dsAddr_gotMugOfMudFlag, 1)) {
			displayMessage(dsAddr_poleClimbDoneMsg); // "Never Again!"
		} else {
			for (byte i = 11; i <= 27; i += 4)
				playSound(76, i);

			playSound(56, 35);
			playSound(19, 59);
			playActorAnimation(864);
			playAnimation(865, 1);
			playActorAnimation(866);
			//InventoryObject *obj = inventory->selectedObject();
			//if (obj != NULL && obj->id == kInvItemMug) {

			// FIXME: implement pause in mudpool and using of Mug object, as per original interpreter
			if (inventory->has(kInvItemMug)) {
				playSound(5, 4);
				playSound(5, 19);
				playSound(64, 11);
				playActorAnimation(867);
				inventory->remove(kInvItemMug);
				inventory->add(kInvItemMugOfMud);
				moveTo(86, 195, 1, true);
				playActorAnimation(868);
				SET_FLAG(dsAddr_gotMugOfMudFlag, 1);
			} else {
				fnPoleClimbFail();
				dialog->popMark(scene, dsAddr_dialogStackFallIntoMudpool);
			}
		}
		break;

	case csAddr_poleClimbFail:
		fnPoleClimbFail();
		break;

	case 0x4195:
		displayMessage(dsAddr_preferWaterMsg); // "I prefer water"
		break;

	case 0x419c: // getting the bird
		setOns(0, 0);
		playSound(56, 10);
		playActorAnimation(875);
		disableObject(6);
		inventory->add(kInvItemBird);
		break;

	case 0x41c3:
		displayMessage(dsAddr_pullObjMsg2); // "I can't reach it"
		break;

	case 0x41ca:
		rejectMessage();
		break;

	case 0x41ce:
		moveTo(197, 159, 4);
		setOns(0, 0);
		playSound(71, 8);
		playActorAnimation(833);
		moveTo(225, 159, 4);
		inventory->add(kInvItemDelicatePlant);
		disableObject(3);
		break;

	case 0x422c:
		displayMessage(dsAddr_tooWeakToClimbMsg); // "I'm too weak to climb it"
		break;

	case 0x4233:
		loadScene(3, Common::Point(216, 199));
		scene->setOrientation(1);
		break;

	case 0x424d:
		loadScene(5, Common::Point(18, 174));
		scene->setOrientation(2);
		break;

	case 0x4267:
		hideActor();
		playSound(23, 8);
		playSound(24, 13);
		setOns(1, 0);
		playActorAnimation(841);
		setOns(1, 0x61);
		setOns(2, 0);
		playSound(63, 12);
		playSound(5, 26);
		playActorAnimation(842);
		wait(100);
		// shown in different positions
		displayMessage(dsAddr_fnMsg2, textColorMark, 16, 68); // "And how am I supposed to get back?"
		wait(50);
		displayMessage(dsAddr_fnMsg3, textColorMark, 92, 68); // "Great"
		wait(50);
		displayMessage(dsAddr_fnMsg4, textColorMark, 62, 68); // "Oh, yeah, right"
		wait(50);
		playActorAnimation(843);
		showActor();
		moveTo(223, 149, 0, true);
		disableObject(7);
		disableObject(1);
		inventory->add(kInvItemShovelAct1);
		displayMessage(dsAddr_fnMsg1); // "Piece of cake"
		break;

	case 0x433a:
		loadScene(10, Common::Point(294, 183));
		scene->setOrientation(4);
		break;

	case 0x4354:
		loadScene(4, Common::Point(300, 185));
		scene->setOrientation(4);
		break;

	case 0x436e:
		loadScene(2, Common::Point(219, 199));
		scene->setOrientation(1);
		break;

	case 0x4388:
		playSound(80, 4);
		playActorAnimation(961);
		loadScene(8, 155, 199, 1);
		break;

	case 0x43b5: // HQ, first trial - prison
		playSound(70, 6);
		playActorAnimation(962);
		loadScene(7, 30, 184, 2);
		if (res->dseg.get_byte(dsAddr_FirstActTrialState) < 2) {
			wait(150);
			moveTo(134, 167, 2);
			displayMessage(dsAddr_firstTrialMsg); // "Sir, I'm Mark. A rookie"
			setLan(1, 0);
			playAnimation(812, 0, true);
			playActorAnimation(811);

			dialog->show(148, scene, 0, 813, textColorMark, textColorCaptain, 0, 1);
			loadScene(6, 230, 184);
			playMusic(5);
			dialog->show(149, scene, 0, 814, textColorMark, textColorCaptain, 0, 1);
			playSound(4, 14);
			playAnimation(815, 0);
			setOns(1, 0);

			dialog->showMono(150, scene, 0, textColorMark, 0);

			SET_FLAG(dsAddr_FirstActTrialState, 1);
		}
		break;

	case 0x4482:
		if (CHECK_FLAG(dsAddr_FirstActTrialState, 0)) {
			playActorAnimation(968);
			displayMessage(dsAddr_lockedMsg); // "It's Locked!"
		} else {
			playSound(80, 3);
			playSound(79, 4);
			playActorAnimation(968);
			loadScene(6, 280, 186, 4);
		}
		break;

	case 0x44fc: // pull out spring from bed
		playSound(53, 25);
		playSound(24, 27);
		playSound(5, 36);
		playActorAnimation(839);
		moveTo(278, scene->getPosition().y, 0, true);
		inventory->add(kInvItemSpring);
		disableObject(1);
		break;

	case 0x44cb:
		if (CHECK_FLAG(dsAddr_gotRopeAct1Flag, 1)) {
			displayMessage(dsAddr_vacMsg); // "What am I? A vacuum cleaner?!"
		} else {
			playSound(49, 14);
			playSound(5, 21);
			playActorAnimation(869);
			inventory->add(kInvItemRopeAct1);
			SET_FLAG(dsAddr_gotRopeAct1Flag, 1);
		}
		break;

	case 0x4532:
		displayMessage(dsAddr_springPrickMsg); // "The springs would prick my back"
		break;

	case 0x4539: // prison cell: use crates
		if (CHECK_FLAG(dsAddr_JailCableAndBowlState, 2)) {
			// finished the meal - trap
			displayMessage(dsAddr_mealFinishedMsg); // "Hey! I finished my meal."
			moveTo(306, 196, 2);
			wait(50);
			//playAnimation(825, 1); //very long empty animation. what for?
			wait(50);
			setLan(1, 0);
			playSound(71, 4);
			playActorAnimation(823);

			loadScene(5, scene->getPosition());
			playSound(74, 1);
			playSound(74, 3);
			playSound(74, 6);
			playActorAnimation(826);
			loadScene(6, scene->getPosition());
			setOns(3, 0x5b);
			wait(50);
			displayMessage(dsAddr_bowlWeldedMsg); // "Wow. He got welded to the bowl"
			SET_FLAG(dsAddr_JailCableAndBowlState, 3);
			scene->getObject(4)->setName("body");
		} else {
			if (dialog->pop(scene, dsAddr_dialogStackJailDoorGrates, 0, 0, textColorMark, textColorMark, 0, 0) == 0x636b) { // 'im getting hungry'
				wait(100);
				playSound(52, 8);
				playSound(52, 13);
				playAnimation(820, 1);
				setOns(3, 0x59);
				wait(50);
				moveTo(scene->getPosition().x, scene->getPosition().y + 1, 3);
				wait(150);
				moveTo(scene->getPosition().x, scene->getPosition().y - 1, 2);
				wait(100);
				displayMessage(dsAddr_ThanksMsg); // "Thanks."
				enableObject(4);
				SET_FLAG(dsAddr_GotFoodBowlInJailFlag, 1);
			}
		}
		break;

	case 0x4662:
		if (CHECK_FLAG(dsAddr_JailCableAndBowlState, 3)) {
			if (CHECK_FLAG(dsAddr_GotJailKeyFlag, 1)) {
				displayMessage(dsAddr_noPocketMsg); // "I don't want to touch his pockets again."
			} else {
				moveTo(280, 179, 2);
				playSound(49, 7);
				playSound(5, 17);
				playActorAnimation(827);
				inventory->add(kInvItemJailKey);
				SET_FLAG(dsAddr_GotJailKeyFlag, 1);
			}
		} else
			displayMessage(dsAddr_foodAliveMsg); // "No, thanks. This food seems still alive"
		break;

	case 0x46af: // prison cell: use live cable
		if (CHECK_FLAG(dsAddr_GotFoodBowlInJailFlag, 1)) {
			displayMessage(dsAddr_ideaMsg); // "That gives me an idea"
			setOns(2, 0);
			playActorAnimation(821);
			setOns(2, 0x5a);
			setOns(3, 0);
			playSound(22, 2);
			playActorAnimation(822);
			displayMessage(dsAddr_checkWorksMsg); // "Now I got to check if it works"
			disableObject(5);
			SET_FLAG(dsAddr_JailCableAndBowlState, 1);
		} else
			displayMessage(dsAddr_unkUsageMsg); // "I don't have any idea what to do with it right now"
		break;

	case 0x4705: // prison: getting lamp bulb
		wait(50);
		moveTo(144, 185, 4);
		playSound(56, 15);
		setOns(0, 86); // hiding lamp
		playActorAnimation(816, true);
		playAnimation(817, 0, true);
		waitAnimation();
		setOns(0, 87);

		playSound(34, 1);
		playSound(5, 15);
		playActorAnimation(818, true);
		playAnimation(819, 0, true);
		waitAnimation();

		moveTo(160, 188, 1, true);
		setOns(2, 88);

		disableObject(6);
		enableObject(5);
		inventory->add(kInvItemBulb);
		break;

	case 0x4794: // prison cell door
		if (res->dseg.get_byte(dsAddr_FirstActTrialState) >= 2) {
			loadScene(5, 287, 143);
		} else {
			displayMessage(dsAddr_doorClosedMsg); // "The door is closed. What a surprise."
		}
		break;

	case 0x47bc: // prison: examining trash can
		playSound(79, 5);
		playSound(1, 14);
		playActorAnimation(966);
		displayMessage(dsAddr_emptyMsg); // "It's Empty"
		break;

	case 0x47db: // prison: use switch
		if (CHECK_FLAG(dsAddr_FirstActTrialState, 1)) {
			playSound(71, 4);
			playActorAnimation(823);
			if (CHECK_FLAG(dsAddr_JailCableAndBowlState, 0)) {
				displayMessage(dsAddr_NotHappenMsg); // "Nothing happened"
			} else {
				playSound(74, 1);
				playAnimation(824, 1);
				if (CHECK_FLAG(dsAddr_JailCableAndBowlState, 1)) {
					wait(100);
					displayMessage(dsAddr_timeToCallMsg); // "I think it is time to call captain"
					SET_FLAG(dsAddr_JailCableAndBowlState, 2);
				}
			}
		} else {
			displayMessage(dsAddr_nahMsg); // "Nah"
		}
		break;

	case 0x4836:
		rejectMessage();
		break;

	case 0x4871:
		playActorAnimation(965);
		displayMessage(dsAddr_lockedMsg); // "It's Locked!"
		break;

	case 0x487e:
		displayMessage(dsAddr_geographyClassMsg); // "I should have paid more attention in geography classes."
		break;

	case 0x4885:
		displayMessage(dsAddr_dontNeedMessMsg); // "I don't need this mess"
		break;

	case 0x488c:
		displayMessage(dsAddr_seenSofterRocksMsg); // "Thanks, but I've seen softer rocks"
		break;

	case 0x4893: // taking pills
		if (CHECK_FLAG(dsAddr_captainDrawerState, 1)) {
			SET_FLAG(dsAddr_captainDrawerState, 2);
			setOns(1, 0x67);
			playSound(5, 9);
			playActorAnimation(872);
			inventory->add(kInvItemMedicine);
			disableObject(7);
		} else {
			playActorAnimation(964);
			displayMessage(dsAddr_lockedMsg); // "It's Locked!"
		}
		break;

	case 0x48d4:
		displayMessage(dsAddr_tooBluntMsg); // "They are too blunt to be of any use"
		break;

	case 0x48db:
		displayMessage(dsAddr_uselessModelsMsg); // "What's the use of the models?"
		break;

	case 0x48e2:
	case 0x48e6:
		rejectMessage();
		break;

	case 0x4911:
		displayMessage(dsAddr_barmanWillNoticeMsg); // "The barman will surely notice its disappearing"
		break;

	case 0x4918: // talking with barmen
		if (CHECK_FLAG(dsAddr_birdOnBarRadioAntennaFlag, 1)) {
			moveTo(140, 152, 1);
			if (CHECK_FLAG(dsAddr_swappedBarmanMugFlag, 1)) {
				dialog->showMono(177, scene, 0, textColorMark, 0);
				displayMessage(dsAddr_yeahRightMsg, textColorBarman, 32, 67); // "Yeah right!"
				//reloadLan();
				setLan(1, 0);
				playAnimation(882, 0);
				playSound(75, 10);
				setOns(2, 0);
				playSound(75, 10);
				playSound(24, 15);
				playAnimation(883, 0);
				shakeScreen();
				disableObject(1);
				disableObject(2);
				SET_FLAG(dsAddr_barmanPassedOutFlag, 1);
			} else
				displayMessage(dsAddr_talkNotNowMsg); // "I've got no reason to talk to him right now."
		} else {
			if (CHECK_FLAG(dsAddr_FirstActTrialState, 3)) {
				if (CHECK_FLAG(dsAddr_spokeToBarmanAboutThirdTrialFlag, 1)) {
					dialog->show(168, scene, 0, 857, textColorMark, textColorBarman, 0, 1);
				} else {
					dialog->show(166, scene, 0, 857, textColorMark, textColorBarman, 0, 1); // taking mug
					playActorAnimation(859, true);
					playAnimation(858, 0, true);
					waitAnimation();
					playSound(75, 6);
					playActorAnimation(860);
					dialog->show(167, scene, 0, 857, textColorMark, textColorBarman, 0, 1);
					inventory->add(kInvItemMug);
					SET_FLAG(dsAddr_spokeToBarmanAboutThirdTrialFlag, 1);
					SET_FLAG(dsAddr_gotPasswordNeedSpeakBarmanFlag, 0);
				}
			} else {
				dialog->pop(scene, dsAddr_dialogStackBarman, 0, 857, textColorMark, textColorBarman, 0, 1);
			}
		}
		break;

	case 0x4d7d:
	case 0x4d81:
		rejectMessage();
		break;

	case 0x4d89:
		displayMessage(dsAddr_getRidOfGuardFirstMsg); // "If I want to get inside I must get rid of this guard first..."
		break;

	case 0x4d90:
		rejectMessage();
		break;

	case 0x4e47:
		loadScene(13, Common::Point(9, 172));
		scene->setOrientation(2);
		break;

	case 0x4e85:
		loadScene(15, Common::Point(291, 162));
		scene->setOrientation(4);
		break;

	case 0x4e9f:
		loadScene(12, Common::Point(310, 152));
		scene->setOrientation(4);
		break;

	case 0x4f14: // use the hollow
		if (CHECK_FLAG(dsAddr_mansionTreeHollowEmptyFlag, 1))
			displayMessage(dsAddr_totalEmptyMsg); // "I can see it's totally empty"
		else
			displayMessage(dsAddr_noHandsMsg); // "I'd better not put my hands in there..."
		break;

	case 0x4a64:
		if (CHECK_FLAG(dsAddr_gotPasswordNeedSpeakBarmanFlag, 1))
			displayMessage(dsAddr_firstBusinessMsg); // "First I've got some business to take care of"
		else
			loadScene(5, 35, 162);
		break;

	case 0x4bf5:
		playActorAnimation(959);
		loadScene(8, 40, 152, 3);
		break;

	case 0x4c18:
		rejectMessage();
		break;

	case 0x4c29:
		displayMessage(dsAddr_tooManyToSearchMsg); // "There are too many of them to search"
		break;

	case 0x4c30:
	case 0x4c37:
		displayMessage(dsAddr_captainWouldNotFitMsg); // "Captain surely wouldn't fit them. I must look elsewhere"
		break;

	case 0x483a:
		dialog->popMark(scene, dsAddr_dialogStackInterrogateCaptain);
		break;

	case 0x4844:
		playSound(80, 4);
		playActorAnimation(963);
		loadScene(5, 166, 158);
		break;

	case 0x48ea:
		setOns(0, 0);
		playSound(5, 9);
		playActorAnimation(836);
		inventory->add(kInvItemSwissArmyKnife);
		disableObject(12);
		break;

	case 0x4a8c:
		if (CHECK_FLAG(dsAddr_barmanPassedOutFlag, 1)) {
			playSound(89, 5);
			playActorAnimation(958);
			loadScene(9, 240, 182, 4);
		} else if (CHECK_FLAG(dsAddr_birdOnBarRadioAntennaFlag, 1)) {
			displayMessage(dsAddr_barmanTooCloseMsg); // "The barman is too close"
		} else {
			dialog->pop(scene, dsAddr_dialogStackBarCellarDoor, 0, 857, textColorMark, textColorBarman, 0, 1);
		}
		break;

	case 0x4aed:
		displayMessage(dsAddr_tooBigMsg); // "It's too big and I doubt if I'll ever need it"
		break;

	case 0x4af4: // taking the crumbs
		setOns(0, 0);
		playSound(49, 6);
		playSound(5, 13);
		playActorAnimation(861);
		inventory->add(kInvItemCrumbs);
		disableObject(6);
		break;

	case 0x4b23:
		rejectMessage();
		break;

	case 0x4b27:
		displayMessage(dsAddr_tooMuchToDrinkMsg); // "It'd take too much time to drink it..."
		break;

	case 0x4b2e:
		displayMessage(dsAddr_notThiefMsg); // "I'm not a thief. And it's empty, by the way."
		break;

	case 0x4b35:
		playSound(15, 7);
		playActorAnimation(884);
		playSound(55, 1);
		playSound(24, 12);
		playAnimation(885, 0);
		dialog->show(164, scene, 886, 0, textColorJohnNoty, textColorMark, 1, 0);
		playMusic(3);
		loadScene(40, 198, 186, 1);
		dialog->show(202, scene, 0, 920, textColorMark, textColorRGBBoss, 0, 1);
		inventory->clear();
		inventory->add(kInvItemSuperGlue);
		displayCredits(dsAddr_credits5);
		loadScene(1, 198, 186);
		hideActor();
		playActorAnimation(956);
		dialog->showMono(212, scene, 957, textColorMark, 1);
		waitAnimation();
		loadScene(15, 157, 199, 1);
		playMusic(6);
		break;

	case 0x4c3e: // get the grenade
		playSound(32, 24);
		playActorAnimation(862);
		reloadLan();
		playAnimation(863, 1);
		inventory->add(kInvItemGrenade);
		disableObject(1);
		SET_FLAG(dsAddr_act1GuardState, 2);
		break;

	case 0x4c70:
		if (CHECK_FLAG(dsAddr_act1GuardState, 0)) {
			if (CHECK_FLAG(dsAddr_ShownPassToGuardFlag, 1)) { // papers are shown
				dialog->pop(scene, dsAddr_dialogStackCampGuardReadingNews, 0, 809, textColorMark, textColorCampGuard, 0, 1);
			} else {
				dialog->pop(scene, dsAddr_dialogStackCampGuardWantsDocuments, 0, 809, textColorMark, textColorCampGuard, 0, 1);
			}
		} else {
			displayMessage(dsAddr_helloQMsg); // "Hello?"
			wait(100);
			displayMessage(dsAddr_totallyAddictedMsg); // "He's totally addicted"
		}
		break;

	case 0x4c1c:
		playActorAnimation(960);
		displayMessage(dsAddr_lockedMsg); // "It's Locked!"
		break;

	case 0x4ca5:
		displayMessage(dsAddr_chickenNeverMsg); // "Chickening? Me? Never!"
		break;

	case 0x4cac:
		if (CHECK_FLAG(dsAddr_ShownPassToGuardFlag, 1)) { // papers are shown
			loadScene(5, 124, 199);
		} else {
			dialog->show(144, scene, 0, 809, textColorMark, textColorCampGuard, 0, 1);
			moveTo(269, 175, 4);
			dialog->pop(scene, dsAddr_dialogStackCampGuardShowPass, 0, 809, textColorMark, textColorCampGuard, 0, 1);
		}
		break;

	case 0x4cf1: // talking with mansion guard
		SET_FLAG(dsAddr_spokenWithMansionGuardFlag, 1);
		if (dialog->pop(scene, dsAddr_dialogStackPleadingToMansionGuard, 0, 529, textColorMark, textColorMansionGuard, 0, 1) == 0x01b4) { // 2nd try
			Common::Point p = scene->getPosition();
			moveTo(189, 159, 0);
			//waitLanAnimationFrame(1, 1);

			playSound(5, 2);
			playSound(5, 19);
			playActorAnimation(550, true);
			playAnimation(551, 0, true);
			waitAnimation();

			moveTo(p, 2);
			inventory->add(kInvItemChocCandy);
			dialog->pop(scene, dsAddr_dialogStackPleadingToMansionGuard, 0, 529, textColorMark, textColorMansionGuard, 0, 1);
		}
		break;

	case 0x4d94: // talking with fatso
		dialog->show(87, scene, 0, 666, textColorMark, textColorJohnNoty, 0, 2);
		displayAsyncMessage(dsAddr_BribeMsg, 120, 109, 1, 10, textColorJohnNoty); // FIXME: Original (x,y) was (100, 78), rather than (120, 109)?
		playSound(5, 3);
		playAnimation(667, 1);
		playAnimation(668, 1);
		setOns(2, 50);
		dialog->show(88, scene, 0, 666, textColorMark, textColorJohnNoty, 0, 2);
		setOns(3, 0);
		setFlag(dsAddr_johnNotyOutsideMansionDoorFlag, 0);
		reloadLan();
		playSound(82, 19);
		playAnimation(669, 1);
		dialog->showMark(89, scene);
		enableObject(15);
		disableObject(8);
		break;

	case 0x4e61:
		loadScene(14, 280, 198);
		break;

	case 0x4ee5:
		setOns(2, 0);
		playSound(5, 12);
		playActorAnimation(676);
		displayMessage(dsAddr_WimpMsg); // "I'm a pathetic little wimp"
		disableObject(15);
		inventory->add(kInvItemBanknote);
		break;

	case 0x4d56:
		inventory->add(kInvItemWhisky);
		disableObject(2);
		setOns(0, 0);
		playSound(5, 12);
		playActorAnimation(547);
		break;

	case 0x4d85:
		rejectMessage();
		break;

	case 0x4eb9: // Pick up wrapper
		playSound(5, 12);
		playSound(5, 18);
		inventory->add(kInvItemWrapper);
		setOns(1, 0);
		playActorAnimation(549);
		disableObject(13);
		break;

	case 0x4ee1:
		rejectMessage();
		break;

	case 0x4f25:
		playActorAnimation(967);
		displayMessage(dsAddr_tooHardWoodMsg); // "This wood is too hard to break"
		break;

	case 0x4f32: // use tree near the mansion
		if (CHECK_FLAG(dsAddr_mansionTreeHollowEmptyFlag, 1)) {
			if (CHECK_FLAG(dsAddr_climbedMansionTreeAlreadyFlag, 1)) {
				displayMessage(dsAddr_noChanceMsg); // "I won't take my chances a second time"
			} else {
				playSound(26, 13);
				playSound(26, 15);
				playSound(26, 23);
				playSound(26, 25);
				playSound(26, 32);
				playSound(26, 34);
				playSound(26, 36);
				playActorAnimation(590);
				moveTo(204, 178, 3, true);
				playSound(59, 1);
				playSound(60, 16);
				playActorAnimation(591);
				wait(50);
				displayMessage(dsAddr_oneSmallStepMsg); // "One small step for man, one big pain in the head"
				SET_FLAG(dsAddr_climbedMansionTreeAlreadyFlag, 1);
				fnMansionIntrusionAttempt();
			}
		} else {
			playActorAnimation(49);
			playSound(56, 8);
			playSound(56, 12);
			playSound(49, 10);
			displayAsyncMessage(dsAddr_laughterMsg, 219, 98, 16, 24); // "(laughter)"
			playActorAnimation(587);
			moveRel(0, 0, 2);
			wait(100);
			displayMessage(dsAddr_tickledMsg); // "Something tickled me!"
		}
		break;

	case 0x500d: // picking up wild plant
		if (CHECK_FLAG(dsAddr_gotPotatoAlreadyFlag, 1)) {
			displayMessage(dsAddr_noPotatoMsg); // "There are no more potatoes"
		} else {
			SET_FLAG(dsAddr_gotPotatoAlreadyFlag, 1);
			setOns(2, 0);
			playSound(21, 9);
			playSound(34, 21);
			playSound(26, 30);
			playActorAnimation(552);
			setOns(2, 0x12);
			inventory->add(kInvItemPotato);
		}
		break;

	case 0x505f:
		displayMessage(dsAddr_wallTooSmoothMsg); // "The wall surface is too smooth to climb"
		break;

	case 0x5066:
		loadScene(11, Common::Point(183, 109));
		scene->setOrientation(3);
		break;

	case 0x5080:
		loadScene(13, Common::Point(290, 181));
		scene->setOrientation(4);
		break;

	case 0x50f6:
		displayMessage(dsAddr_tooMuchResinToClimbMsg); // "I could climb it if there wasn't so much resin"
		break;

	case 0x50fd:
		displayMessage(dsAddr_onlyGreenRectMsg); // "The only green stuff that I like is that rectangular piece of paper with..."
		break;

	case 0x5104:
		loadScene(11, 319, 198, 4);
		if (!CHECK_FLAG(dsAddr_scaredGuardAlreadyFlag, 1)) {
			// guard is drinking
			SET_FLAG(dsAddr_timedCallbackState, 3);
			setTimerCallback(csAddr_guardScareTimeout, 40);
			playAnimation(544, 0, true, true); // ignore busy flag for this animation
		}
		break;

	case csAddr_guardScareTimeout: // too late to scare guard, resetting
		SET_FLAG(dsAddr_timedCallbackState, 0);
		break;

	case csAddr_guardDrinking:
		fnGuardDrinking();
		break;

	case 0x51c8:
		displayMessage(dsAddr_wallTooSmoothMsg); // "The wall surface is too smooth to climb"
		break;

	case 0x51cf:
		loadScene(12, Common::Point(15, 189));
		scene->setOrientation(2);
		break;

	case 0x51e9:
		displayMessage(dsAddr_dontWannaTouchHedgehogMsg); // "I don't wanna touch it. Its spines could hurt my delicate hands"
		break;

	case 0x51f0:
		setOns(0, 0);
		playSound(5, 11);
		playActorAnimation(637);
		disableObject(7);
		inventory->add(kInvItemRock);
		break;

	case 0x5217:
		if (CHECK_FLAG(dsAddr_beesGoneFlag, 1))
			displayMessage(dsAddr_notHungryMsg); // "Thanks, I'm not hungry"
		else
			displayMessage(dsAddr_avoidBeesMsg); // "I'm going to stay at least five meters away from these bees!"
		break;

	case 0x522c:
		displayMessage(dsAddr_avoidBeesMsg); // "I'm going to stay at least five meters away from these bees!"
		break;

	case 0x5233:
		rejectMessage();
		break;

	case 0x5237:
		if (!CHECK_FLAG(dsAddr_beesGoneFlag, 1)) {
			displayMessage(dsAddr_avoidBeesMsg); // "I'm going to stay at least five meters away from these bees!"
		} else if (CHECK_FLAG(dsAddr_mansionTunnelDoneFlag, 1))
			displayMessage(dsAddr_roadNowhereMsg); // "Nah. It's a road to nowhere"
		else {
			moveTo(173, 138, 2);
			playSound(28, 5);
			playActorAnimation(583);
			playActorAnimation(584);

			loadScene(0, 0, 0, 0); // clear background

			playSound(72, 18);
			playSound(73, 39);
			playActorAnimation(585);

			loadScene(11, 194, 160, 2);
			playSound(28, 2);
			playActorAnimation(586);
			moveTo(138, 163, 3);
			displayMessage(dsAddr_lifeBrutalMsg); // "Life is really brutal"
			SET_FLAG(dsAddr_mansionTunnelDoneFlag, 1);
			fnMansionIntrusionAttempt();
		}
		break;

	case 0x5320:
		loadScene(11, Common::Point(30, 124));
		scene->setOrientation(2);
		break;

	case 0x533a:
		displayMessage(dsAddr_noLongHandsMsg); // "I really don't have such long hands"
		break;

	case 0x5341:
		displayMessage(dsAddr_tooFarToSwimMsg); // "It's too far to swim there"
		break;

	case 0x5403:
		displayMessage(dsAddr_noBucketMsg); // "It's not a barrel-organ. And there's no bucket."
		break;

	case 0x540a:
		loadScene(20, Common::Point(10, 185));
		scene->setOrientation(2);
		break;

	case 0x5424:
		loadScene(11, Common::Point(30, 170));
		scene->setOrientation(2);
		break;

	case 0x543e:
		loadScene(18, Common::Point(224, 199));
		scene->setOrientation(4);
		break;

	case 0x5547:
		loadScene(15, Common::Point(15, 172));
		scene->setOrientation(2);
		break;

	case 0x55a8:
		{
			uint16 d = dialog->popMark(scene, dsAddr_dialogStackSquirrel);
			if (d == 0x2c5d) { // 4th try - Throw Nut
				waitLanAnimationFrame(1, 0x23);
				setOns(0, 0);
				playSound(52, 9);
				playSound(52, 11);
				playSound(52, 13);
				playSound(53, 32);
				playAnimation(570, 0);
				wait(50);
				displayMessage(dsAddr_ThanksMsg); // "Thanks."
				disableObject(5);
				SET_FLAG(dsAddr_squirrelNutState, 1);
			} else if (d != 0x2c9b) { // 5th (last) try
				waitLanAnimationFrame(1, 0x23);
				playSound(52, 9);
				playSound(52, 11);
				playSound(52, 13);
				playAnimation(569, 0);
			}
		}
		break;

	case 0x5663:
		if (CHECK_FLAG(dsAddr_squirrelNutState, 1))
			displayMessage(dsAddr_findNutMsg); // "I won't find the nut just like that. The grass is too dense"
		else
			displayMessage(dsAddr_hmmGrassMsg); // "Hmmm. Grass..."
		break;

	case 0x5674:
		loadScene(18, Common::Point(94, 115));
		scene->setOrientation(3);
		break;

	case 0x568e:
		displayMessage(dsAddr_notHornyMsg); // "I'm not horny"
		break;

	case 0x5695:
		displayMessage(dsAddr_dontNeedToOpenMsg); // "I don't need to open it"
		break;

	case 0x569c:
		playSound(67, 5);
		playActorAnimation(983);
		displayMessage(dsAddr_emptyMsg); // "It's Empty"
		break;

	case 0x56b3:
		rejectMessage();
		break;

	case 0x56b7:
		playSound(66, 5);
		playSound(67, 11);
		playActorAnimation(984);
		displayMessage(dsAddr_emptyMsg); // "It's Empty"
		break;

	case 0x56d6:
		displayMessage(dsAddr_CantJumpMsg); // "No way I can jump so high, cause, err, white men can't jump"
		break;

	case 0x56dd:
		displayMessage(dsAddr_dontNeedItMsg); // "I don't need it"
		break;

	case 0x56e4:
		displayMessage(dsAddr_notSantaClausMsg); // "I'm not Santa Claus"
		break;

	case 0x56eb:
		displayMessage(dsAddr_noPlasticImitationsMsg); // "I don't need plastic imitations"
		break;

	case 0x56f2:
		rejectMessage();
		break;

	case 0x5721:
		displayMessage(dsAddr_dontNeedItMsg); // "I don't need it"
		break;

	case 0x5728:
		inventory->add(kInvItemChainsaw);
		disableObject(14);
		setOns(0, 0);
		playSound(5, 10);
		playActorAnimation(566);
		break;

	case 0x574f:
		displayMessage(dsAddr_tooFragileMsg); // "It's too fragile to carry around"
		break;

	case 0x5793:
		if (!CHECK_FLAG(dsAddr_alreadyPulledTrunkReleaseLeverFlag, 1)) {
			displayMessage(dsAddr_shutTightMsg); // "It's shut tight"
		} else if (CHECK_FLAG(dsAddr_carTrunkEmptyFlag, 1)) {
			displayMessage(dsAddr_bootEmptyMsg); // "There's nothing else in the boot"
		} else {
			SET_FLAG(dsAddr_carTrunkEmptyFlag, 1);
			moveTo(188, 179, 0);
			playSound(7, 16);
			playActorAnimation(519);
			wait(150);
			moveTo(168, 179, 2);
			inventory->add(kInvItemToolboxFull);
		}
		break;

	case 0x57fa:
		displayMessage(dsAddr_dontNeedItMsg); // "I don't need it"
		break;

	case 0x5801:
		rejectMessage();
		break;

	case 0x583f:
	case 0x5846:
		displayMessage(dsAddr_dontNeedToOpenMsg);
		break;

	case 0x584d:
		displayMessage(dsAddr_pullObjMsg2);
		break;

	case 0x5854:
		loadScene(15, Common::Point(157, 199));
		scene->setOrientation(1);
		break;

	case 0x586e:
		loadScene(21, Common::Point(24, 187));
		scene->setOrientation(2);
		break;

	case 0x5888:
		loadScene(27, Common::Point(108, 199));
		scene->setOrientation(2);
		break;

	case 0x5903:
		displayMessage(dsAddr_keepItOpenMsg); // "I'd like to keep it open"
		break;

	case 0x590a:
		loadScene(20, Common::Point(304, 190));
		scene->setOrientation(4);
		break;

	case 0x5924:
		loadScene(25, Common::Point(298, 146));
		scene->setOrientation(4);
		break;

	case 0x5978:
		displayMessage(dsAddr_notTakingSocksMsg); // "I really don't want to walk around with someone else's socks"
		break;

	case 0x597f:
	case 0x5986:
	case 0x598d:
		displayMessage(dsAddr_dontNeedToOpenMsg); // "I don't need to open it"
		break;

	case 0x5b44:
		// FIXME - This is the doorbell use callback on House #2 
		// i.e. Granny and Anne's House. Need to analyse cseg data properly.
		// Current code inferred from behaviour.
		// FIXME - Add animation call for Ego pushing doorbell.
		displayMessage(dsAddr_ItsOpenMsg);
		break;

	case 0x5c72:
		displayMessage(dsAddr_notTiredMsg); // "Thanks, I'm not tired"
		break;

	case 0x5c79:
		displayMessage(dsAddr_dontNeedToOpenMsg); // "I don't need to open it"
		break;

	case 0x5c80:
		rejectMessage();
		break;

	case 0x5cdb:
	case 0x5ce2:
		displayMessage(dsAddr_dontNeedItMsg); // "I don't need it"
		break;

	case 0x5ce9:
		displayMessage(dsAddr_tooBigMsg); // "It's too big and I doubt if I'll ever need it"
		break;

	case 0x5d1d:
		displayMessage(dsAddr_CantJumpMsg); // "No way I can jump so high, cause, err, white men can't jump"
		break;

	case 0x5d88:
		if (CHECK_FLAG(dsAddr_laundryState, 1)) { // dry laundry
			SET_FLAG(dsAddr_laundryState, 2);
			dialog->show(46, scene, 0, 523, textColorMark, textColorOldLady, 0, 1);
			//waitLanAnimationFrame(1, 1); // another long waiting
			playAnimation(604, 0);

			loadScene(21, scene->getPosition());
			setOns(0, 0);
			disableObject(4);
			enableObject(12);
			playSound(46, 5);
			playAnimation(606, 1);
			setOns(0, 33);
			loadScene(23, scene->getPosition());
			playAnimation(605, 0);
			dialog->show(47, scene, 0, 523, textColorMark, textColorOldLady, 0, 1);
		} else {
			uint16 d = dialog->pop(scene, dsAddr_dialogStackAskOldLadyOK, 0, 523, textColorMark, textColorOldLady, 0, 1);
			if (d == 0x1913) { // 3rd time
				wait(100);
				moveRel(0, 0, 3);
				wait(50);
				displayMessage(dsAddr_giveUpMsg); // "I give up"
				wait(50);
			}
		}
		break;

	case 0x5f9a:
	case 0x5fa1:
		displayMessage(dsAddr_dontNeedToOpenMsg); // "I don't need to open it"
		break;

	case 0x5fa8:
		displayMessage(dsAddr_CantJumpMsg); // "No way I can jump so high, cause, err, white men can't jump"
		break;

	case 0x5faf:
		displayMessage(dsAddr_noSecretPassageMsg); // "I don't think there's any secret passage inside"
		break;

	case 0x5fe5:
		displayMessage(dsAddr_jugMeMsg); // "They can jug me if I steal this"
		break;

	case 0x5fec:
		displayMessage(dsAddr_leaveFlowersAloneMsg); // "I'd better leave it. Women are really oversensitive about flowers."
		break;

	case 0x5ff3: // get duster
		if (CHECK_FLAG(dsAddr_givenFlowerToOldLadyAlreadyFlag, 0)) {
			dialog->pop(scene, dsAddr_dialogStackBorrowDusterFromOldLady, 0, 523, textColorMark, textColorOldLady, 0, 1);
		} else {
			dialog->show(43, scene, 0, 523, textColorMark, textColorOldLady, 0, 1);
			wait(50);
			inventory->add(kInvItemFeatherDusterClean);
			disableObject(12);
			setOns(0, 0);
			playSound(5, 6);
			playActorAnimation(541);
		}
		break;

	case 0x603a:
		rejectMessage();
		break;

	case 0x603e:
		if (CHECK_FLAG(dsAddr_spokenToMirrorFlag, 1)) {
			displayMessage(dsAddr_busyThinkingMsg); // "I'd better not interrupt it's thought process"
		} else {
			displayMessage(dsAddr_mirrorMirrorMsg); // "Mirror, Mirror on the wall...."
			wait(150);
			displayMessage(dsAddr_thinkTooLongMsg); // "Hey, don't think too long"
			wait(150);
			displayMessage(dsAddr_HintMaleMsg); // "A hint: Someone in this room, a male"
			wait(150);
			displayMessage(dsAddr_okWaitMsg); // "OK, take your time"
			wait(150);
			SET_FLAG(dsAddr_spokenToMirrorFlag, 1);
		}
		break;

	case 0x6074:
		rejectMessage();
		break;

	case 0x6078:
		displayMessage(dsAddr_tooBigMsg); // "It's too big and I doubt if I'll ever need it"
		break;

	case 0x6205:
		if (CHECK_FLAG(dsAddr_lightOnFlag, 1))
			displayMessage(dsAddr_tooHeavyMsg); // "It's too heavy. Not that I'm wimp"
		else
			fnTooDark();
		break;

	case 0x6217:
		if (CHECK_FLAG(dsAddr_lightOnFlag, 1))
			displayMessage(dsAddr_noDentistsMsg); // "I don't want to have anything in common with dentists"
		else
			fnTooDark();
		break;

	case 0x62c1:
		if (CHECK_FLAG(dsAddr_lightOnFlag, 1))
			retVal = false;
		else
			fnTooDark();
		break;

	case 0x634a:
		displayMessage(dsAddr_noHandsSharpThornsMsg); // "I can't remove it with my hands. these thorns look really sharp"
		break;

	case 0x637f:
		loadScene(21, Common::Point(201, 199));
		scene->setOrientation(1);
		break;

	case 0x6399:
		displayMessage(dsAddr_rockWalkingGeeMsg); // "Yeah, great idea. Let's take this rock and walk around a bit. Gee..."
		break;

	case 0x63a0:
	case 0x63a7:
		displayMessage(dsAddr_butterflyMsg); // "I'd better leave them alone, they make this place beautiful"
		break;

	case 0x63ae:
		displayMessage(dsAddr_notSureIfAliveMsg); // "I'm not sure if it's alive"
		break;

	case 0x63bc:
		playMusic(6);
		loadScene(25, 151, 156, 2);
		break;

	case 0x63dc:
		dialog->showMono(86, scene, 0, textColorMark, 0);
		break;

	case 0x63e3:
		displayMessage(dsAddr_holeTooNarrowMsg); // "The hole is too narrow to fit my hand"
		break;

	case 0x646e:
	case 0x6475:
		dialog->showMono(85, scene, 0, textColorMark, 0);
		break;

	case 0x6479:
		dialog->showMono(84, scene, 0, textColorMark, 0);
		break;

	case 0x6507:
		if (CHECK_FLAG(dsAddr_birdsGoneFromScarecrowFlag, 1))
			rejectMessage();
		else
			displayMessage(dsAddr_birdAttackMsg); // "Hey You! Wake up! Bird attack!"
		break;

	case 0x6541:
		loadScene(20, Common::Point(10, 131));
		scene->setOrientation(3);
		break;

	case 0x6635:
		displayMessage(dsAddr_uninterestingHaystackMsg); // "I don't see anything interesting about this haystack"
		break;

	case 0x666a:
		displayMessage(dsAddr_moreComplicatedMsg); // "It's more complicated than that"
		break;

	case 0x65c3:
		if (CHECK_FLAG(dsAddr_mouseHoleState, 1)) {
			playActorAnimation(635);
			setOns(5, 0);
			playSound(63, 11);
			playSound(15, 20);
			playSound(32, 31);
			playActorAnimation(636);
			inventory->add(kInvItemHandkerchief);
			inventory->add(kInvItemMouse);
			moveTo(scene->getPosition().x - 1, 139, 1, true);
			displayMessage(dsAddr_yikesMsg); // "Yikes!"
			SET_FLAG(dsAddr_mouseHoleState, 2);
			SET_FLAG(dsAddr_HankerchiefInMouseholeFlag, 0);
		} else
			displayMessage(dsAddr_noSearchWarrantMsg); // "I don't have a search-warrant"
		break;

	case 0x6671:
		displayMessage(dsAddr_cantOpenItMsg); // "I can't open it"
		break;

	case 0x6678:
		rejectMessage();
		break;

	case 0x670f:
		displayMessage(dsAddr_dontNeedThemMsg); // "I don't need them"
		break;

	case 0x6716:
		displayMessage(dsAddr_pullObjMsg2); // "I can't reach it"
		break;

	case 0x6772:
		loadScene(31, Common::Point(20, 188));
		scene->setOrientation(2);
		break;

	case 0x678c:
		loadScene(28, Common::Point(189, 153));
		scene->setOrientation(4);
		break;

	case 0x67fa:
		rejectMessage();
		break;

	case 0x67fe:
		displayMessage(dsAddr_troubleWithStairsMsg); // "If I put it on I might have trouble walking up the stairs"
		break;

	case 0x6911:
		displayMessage(dsAddr_9LivesToReadMsg); // "I'd need 9 lives to read them all"
		break;

	case 0x6954:
		displayMessage(dsAddr_thanksNotTiredMsg); // "Thanks, I'm not so tired"
		break;

	case 0x695b:
		displayMessage(dsAddr_noNeedToTurnOnMsg); // "There's no need to turn it on"
		break;

	case 0x6ba6:
		displayMessage(dsAddr_wontBearWeightMsg); // "It won't bear my weight"
		break;

	case 0x6bda:
		displayMessage(dsAddr_peepingTomMsg); // "What am I? A Peeping Tom?"
		break;

	case 0x6c1c:
	case 0x6c20:
		rejectMessage();
		break;

	case 0x6c24:
		displayMessage(dsAddr_dontNeedThemMsg); // "I don't need them"
		break;

	case 0x6c2b:
		loadScene(29, Common::Point(300, 188));
		scene->setOrientation(4);
		break;

	case 0x6c7c:
		displayMessage(dsAddr_bigPocketsMsg); // "I have big pockets, but there are limits"
		break;

	case 0x724e:
		displayMessage(dsAddr_soSharpMsg); // "They're so sharp they'd rip my trousers!"
		break;

	case 0x72be:
		rejectMessage();
		break;

	case 0x7305:
		rejectMessage();
		break;

	case 0x7328:
		displayMessage(dsAddr_noTimeForPleasuresMsg); // "I don't have time for pleasures"
		break;

	case 0x732f:
		displayMessage(dsAddr_notSocksWithBareHandsMsg); // "I won't touch these socks with my bare hands!"
		break;

	case 0x739c:
		displayMessage(dsAddr_notHalloweenMsg); // "It's not Halloween"
		break;

	case 0x7401:
		displayMessage(dsAddr_NotManualMsg); // "It can't be controlled manually! I hate it!"
		break;

	case 0x746f:
		displayMessage(dsAddr_nothingToPlayMsg); // "I have nothing to play"
		break;

	case 0x74b3:
		loadScene(29, Common::Point(256, 171));
		scene->setOrientation(3);
		break;

	case 0x74cd:
		rejectMessage();
		break;

	case 0x74f9:
		loadScene(38, Common::Point(160, 199));
		scene->setOrientation(1);
		break;

	case 0x784a:
		displayMessage(dsAddr_notMineMsg); // "I can't take it. It's not mine."
		break;

	case 0x7851:
		displayMessage(dsAddr_lockedMsg); // "It's Locked!"
		break;

	case 0x7858:
		displayMessage(dsAddr_lockedMsg); // "It's Locked!"
		break;

	case 0x785f:
		displayMessage(dsAddr_pullObjMsg2); // "I can't reach it"
		break;

	case 0x7866:
		if (CHECK_FLAG(dsAddr_JailCableAndBowlState, 3))
			displayMessage(dsAddr_gotchaMsg); // "Gotcha"
		else
			retVal = false;
		break;

	case 0x7878:
		{
			byte v = res->dseg.get_byte(dsAddr_graffitiMsgId) + 1;
			if (v <= 6)
				SET_FLAG(dsAddr_graffitiMsgId, v);

			switch (v) {
			case 1:
				displayMessage(dsAddr_SavingFineMsg); // "Saving is a very fine thing..."
				break;
			case 2:
				displayMessage(dsAddr_loveCaptainMsg); // "I love captain"
				break;
			case 3:
				displayMessage(dsAddr_soccerRulzMsg); // "Soccer rulz"
				break;
			case 4:
				displayMessage(dsAddr_treeCutMsg); // "Don't cut the trees..."
				break;
			case 5:
				displayMessage(dsAddr_visaAcceptedMsg); // "VISA Accepted"
				break;
			default:
				displayMessage(dsAddr_otherGraffitiMsg); // "The rest of graffiti is obscene"
				break;
			}
		}
		break;

	case 0x78a9:
		if (CHECK_FLAG(dsAddr_captainDrawerState, 1))
			displayMessage(dsAddr_nowOpenMsg); // "Now it's open"
		else
			retVal = false;
		break;

	case 0x78bb:
		if (CHECK_FLAG(dsAddr_swappedBarmanMugFlag, 1))
			displayMessage(dsAddr_yuckMsg); // "Yuck!"
		else
			retVal = false;
		break;

	case 0x78ce:
		if (!CHECK_FLAG(dsAddr_mansionTreeHollowEmptyFlag, 1))
			displayMessage(dsAddr_monstersMsg); // "Who knows what monsters live in there"
		else
			retVal = false;
		break;

	case 0x792b: // left click on ann
		moveTo(245, 198, 1);
		if (!CHECK_FLAG(dsAddr_alreadySaidAnneBeautifulFlag, 1)) {
			dialog->showMono(50, scene, 0, textColorMark, 0);
			SET_FLAG(dsAddr_alreadySaidAnneBeautifulFlag, 1);
		} else
			retVal = false;
		break;

	case 0x79c3:
		if (CHECK_FLAG(dsAddr_lightOnFlag, 1))
			retVal = false;
		else
			fnTooDark();
		break;

	case 0x7b26: // cutting the fence
		setOns(0, 0);
		playSound(5, 2);
		playSound(51, 11);
		playSound(51, 23);
		playActorAnimation(837);
		playSound(51, 3);
		playSound(51, 19);
		playSound(23, 26);
		playActorAnimation(838);
		setOns(0, 0x60);
		moveTo(281, scene->getPosition().y, 0, true);
		disableObject(4);
		SET_FLAG(dsAddr_cutFenceFlag, 1);
		break;

	case 0x7b89: // digging mysterious object
		if (CHECK_FLAG(dsAddr_cutFenceFlag, 1)) {
			playActorAnimation(844);
			setOns(1, 0);
			playSound(5, 5);
			playSound(26, 19);
			playSound(24, 25);
			playActorAnimation(847);
			playSound(5, 11);
			playActorAnimation(848);
			setOns(1, 0x64);
			playActorAnimation(845);
			disableObject(3);
			inventory->add(kInvItemKaleidoscope);
			inventory->remove(kInvItemShovelAct1);
		} else
			displayMessage(dsAddr_fenceBlocksMsg); // "The fence blocks the way"
		break;

	case 0x7bf6:
		displayMessage(dsAddr_noDiggingKnifeMsg); // "Digging it out with the knife could take a hundred years"
		break;

	case 0x7bfd:
		playSound(76, 18);
		playSound(76, 22);
		playSound(76, 26);
		playSound(76, 30);
		playSound(76, 34);
		playSound(76, 47);
		playSound(76, 51);
		playSound(76, 55);
		playSound(76, 59);
		playSound(76, 63);
		playActorAnimation(873);
		moveTo(240, 163, 4);
		displayMessage(dsAddr_cmonBabyMsg); // "C'mon baby, it's all yours!"
		waitLanAnimationFrame(1, 0x22);
		playSound(77, 2);
		playSound(77, 12);
		playSound(77, 16);
		playSound(77, 20);
		playSound(77, 34);
		playSound(78, 41);
		playSound(78, 51);
		playSound(56, 63);
		playSound(24, 67);
		playSound(23, 76);
		setLan(1, 0);
		playAnimation(874, 1);
		setOns(0, 0x68);
		inventory->remove(kInvItemDruggedFood);
		enableObject(6);
		disableObject(1);
		break;

	case 0x7cc9:
	case 0x7cd0:
		displayMessage(dsAddr_throwCrumbsToBirdQMsg); // "Should I throw the crumbs to the bird?"
		break;

	case 0x7cd7:
		displayMessage(dsAddr_dontWasteCrumbs); // "I don't want to waste these tasty crumbs"
		break;

	case 0x7cde:
		displayMessage(dsAddr_mightSlipFallInMsg); // "Better not... I might slip and fall in..."
		break;

	case 0x7ce5: // put spring on the solid ground
		playSound(5, 2);
		playSound(19, 11);
		playActorAnimation(840);
		setOns(1, 0x61);
		inventory->remove(kInvItemSpring);
		disableObject(2);
		enableObject(7);
		break;

	case 0x7d1a: // captain's key + door
		if (res->dseg.get_byte(dsAddr_FirstActTrialState) <= 1) {
			playSound(5, 2);
			playSound(57, 12);
			playSound(70, 19);
			playActorAnimation(828);
			moveTo(262, 160, 1, true);
			disableObject(4);
			disableObject(3);
			setOns(0, 0);
			setOns(1, 85);
			setOns(2, 0);
			setOns(3, 0);
			loadScene(5, scene->getPosition());
			setOns(0, 92);
			playAnimation(829, 1, true, true, true);
			wait(200);
			playAnimation(0, 1);
			setOns(0, 0);
			dialog->showMono(156, scene, 830, textColorShockedCaptain, 1);
			loadScene(7, 130, 195, 2);
			playMusic(4);
			setLan(1, 1);
			wait(100);
			dialog->show(157, scene, 0, 832, textColorMark, textColorCaptain, 0, 1);

			//playAnimation(831, 1);

			SET_FLAG(dsAddr_FirstActTrialState, 2);
		} else
			displayMessage(dsAddr_nahMsg); // "Nah"
		break;

	case 0x7e02: // tickling the captain
		if (CHECK_FLAG(dsAddr_AlreadyTickledCaptainFlag, 1)) {
			displayMessage(dsAddr_doesNotWorkMsg); // "That doesn't work"
		} else {
			playSound(5, 6);
			playSound(27, 49);
			playActorAnimation(834, true);
			playAnimation(835, 1, true);
			waitAnimation();

			setOns(0, 94);
			dialog->show(161, scene, 0, 832, textColorMark, textColorCaptain, 0, 1);
			enableObject(12);
			SET_FLAG(dsAddr_AlreadyTickledCaptainFlag, 1);
		}
		break;

	case 0x7e4f: // giving magazine to captain
		dialog->show(162, scene, 0, 856, textColorMark, textColorCaptain, 0, 1);
		playSound(5, 3);
		playActorAnimation(852, true);
		playActorAnimation(853, true);
		displayMessage(dsAddr_whatAboutMsg); // "What about a new"
		displayMessage(dsAddr_hotOffMsg); // "hot off the press"
		displayMessage(dsAddr_fullColorMsg); // "full-color"
		displayMessage(dsAddr_specialEdMsg); // "special edition"
		displayMessage(dsAddr_soldierNewsMsg); // "of Soldier News?!"
		playAnimation(856, 1);
		playSound(5, 3);
		//playActorAnimation(854);
		dialog->show(163, scene, 0, 856, textColorMark, textColorCaptain, 0, 1);
		playAnimation(855, 1);
		wait(200);
		moveTo(30, 181, 0);
		disableObject(1);
		setLan(1, 0);
		SET_FLAG(dsAddr_FirstActTrialState, 3);
		SET_FLAG(dsAddr_gotPasswordNeedSpeakBarmanFlag, 1);
		loadScene(8, 155, 199);
		break;

	case 0x7fbd: // using bird & bartender
		playSound(5, 3);
		playActorAnimation(876);
		setOns(1, 0);
		playSound(26, 7);
		playSound(79, 15);
		playAnimation(877, 1);
		playAnimation(880, 1, true);

		dialog->show(176, scene, 0, 857, textColorMark, textColorBarman, 0, 1);
		setOns(2, 0x6a);
		reloadLan();
		playAnimation(878, 0);
		//playAnimation(879, 0); // background bartender animation
		inventory->remove(kInvItemBird);
		enableObject(1);
		SET_FLAG(dsAddr_birdOnBarRadioAntennaFlag, 1);
		break;

	case 0x8047:
		playSound(32, 5);
		playSound(5, 17);
		playSound(52, 23);
		playActorAnimation(881);
		setOns(2, 0x6b);
		inventory->remove(kInvItemMugOfMud);
		inventory->add(kInvItemMug);
		SET_FLAG(dsAddr_swappedBarmanMugFlag, 1);
		break;

	case 0x808b:
		if (CHECK_FLAG(dsAddr_ShownPassToGuardFlag, 1)) {
			displayMessage(dsAddr_gotPermissionMsg); // "I already got the permission"
		} else {
			displayMessage(dsAddr_showPapersMsg); // "Here are my papers"
			playSound(5, 2);
			playSound(5, 18);
			playActorAnimation(810);
			dialog->show(147, scene, 0, 809, textColorMark, textColorCampGuard, 0, 1);
			SET_FLAG(dsAddr_ShownPassToGuardFlag, 1);
		}
		break;

	case 0x80c3: // show kaleidoscope to the guard
		dialog->show(165, scene, 0, 809, textColorMark, textColorCampGuard, 0, 1);
		playSound(5, 3);
		playSound(5, 30);
		playSound(26, 14);
		hideActor();
		playAnimation(849, 0);
		showActor();
		playAnimation(851, 0);
		playAnimation(850, 0);
		reloadLan();
		inventory->add(kInvItemSoldierNews);
		inventory->remove(kInvItemKaleidoscope);
		enableObject(1);
		SET_FLAG(dsAddr_act1GuardState, 1);
		break;

	case 0x8398:
		displayMessage(dsAddr_trySomewhereElseMsg); // "I'd better try somewhere else - I suppose this side is heavily guarded"
		break;

	case 0x85dd:
		displayMessage(dsAddr_branchNotPaddleMsg); // "This branch is not a paddle. It doesn't even look like one"
		break;

	case 0x85e4:
		displayMessage(dsAddr_sharpenNotPulverizeMsg); // "I needed to sharpen it, not pulverize"
		break;

	case 0x8d42:
		displayMessage(dsAddr_bluntSickleMsg); // "The sickle is too blunt"
		break;

	case 0x8d49:
		displayMessage(dsAddr_noChainsawFuelMsg); // "There's no fuel in the chainsaw"
		break;

	case 0x8d50:
		displayMessage(dsAddr_thornsTooThinMsg); // "Thorns are too thin, the chainsaw is useless here"
		break;

	// Shore

	case 0x5348:
		if (CHECK_FLAG(dsAddr_alreadyGotBrokenPaddleFlag, 1)) { // got broken paddle from boat
			displayMessage(dsAddr_boatEmptyMsg); // "There's nothing else in the boat"
		} else {
			SET_FLAG(dsAddr_alreadyGotBrokenPaddleFlag, 1);
			playSound(57, 6);
			playActorAnimation(536);
			dialog->showMono(77, scene, 0, textColorMark, 0);
			inventory->add(kInvItemBrokenPaddle);
		}
		break;

	case 0x53a1:
		if (CHECK_FLAG(dsAddr_spokenToManInWellFlag, 1)) { // spoken to man in well
			displayMessage(dsAddr_stillThereMsg); // "Are you still there?"
		} else {
			displayMessage(dsAddr_echoMsg); // "Echo!"
			displayMessage(dsAddr_loudEchoMsg, textColorWellEcho, 248, 164); // "ECHO!"
			displayMessage(dsAddr_whoThereMsg); // "Who's there?!"
			displayMessage(dsAddr_loudWhoThereMsg, textColorWellEcho, 225, 164); // "WHO'S THERE?!"
			displayMessage(dsAddr_dontCopyMsg); // "DON'T COPY ME!"
			displayMessage(dsAddr_loudDontCopyMsg, textColorWellEcho, 172, 164); // "DON'T COPY ME!!!"
			displayMessage(dsAddr_throwRockMsg); // "OR I WILL THROW A ROCK DOWN THERE!"
			displayMessage(dsAddr_orIWillMsg, textColorWellEcho, 232, 164); // "OR I WILL"
			wait(100);
			displayMessage(dsAddr_loudEchoMsg, textColorWellEcho, 248, 164);
			SET_FLAG(dsAddr_spokenToManInWellFlag, 1);
		}
		break;

	case 0x5458:
		{
			setOns(2, 0);
			playSound(34, 7);
			playActorAnimation(535);
			inventory->add(kInvItemSecondFlower);
			disableObject(1);

			byte *scene_15_ons = scene->getOns(15);  // patch ons for the scene 15
			scene_15_ons[0] = 0;

			byte f = GET_FLAG(dsAddr_flowerIsleState) + 1;
			SET_FLAG(dsAddr_flowerIsleState, f);
			if (f >= 2) {
				// disable object boat for scene 15!!
				disableObject(1, 15);
			}
		}
		break;

	case 0x54b3:
		{
			setOns(1, 0);
			setOns(3, 0);
			playSound(33, 6);
			playActorAnimation(534);
			inventory->add(kInvItemFirstFlower);
			disableObject(2);
			setOns(1, 10);
			setOns(1, 0, 15);
			byte f = GET_FLAG(dsAddr_flowerIsleState) + 1;
			SET_FLAG(dsAddr_flowerIsleState, f);
			if (f >= 2) {
				// disable object boat for scene 15!!
				disableObject(1, 15);
			}
		}
		break;

	case 0x5502:
		setOns(0, 0);
		loadScene(15, 115, 180, 1);
		playMusic(6);
		playActorAnimation(568);
		break;

	case 0x5561: // Enter lakeside house
		fnEgoDefaultPosition();
		loadScene(19, 223, 199, 1);
		break;

	case 0x55a1:
		fnEgoDefaultPosition();
		rejectMessage();
		break;

	case csAddr_egoDefaultPosition:
		fnEgoDefaultPosition();
		break;

	case 0x563b:
		playSound(5, 10);
		setOns(1, 0);
		playActorAnimation(561);
		inventory->add(kInvItemNut);
		disableObject(6);
		break;

	case 0x56f6:
		playSound(32, 7);
		setOns(1, 0);
		playActorAnimation(626);
		disableObject(12);
		inventory->add(kInvItemCheese);
		displayMessage(dsAddr_foundFoodMsg); // "People leave food in unbelievable places"
		break;

	case 0x5756: // Open car door
		playSound(11, 4);
		playActorAnimation(514);
		setOns(4, 8);
		setOns(2, 5);
		enableObject(14);
		enableObject(15);
		enableObject(16);
		disableObject(1);
		break;

	case 0x5805: // Enter basketball house
		playSound(70, 6);
		playActorAnimation(513);
		loadScene(22, 51, 180, 2);
		break;

	case 0x5832: // Ring doorbell
		playActorAnimation(509);
		displayMessage(dsAddr_outOfOrderMsg); // "It's out of order"
		break;

	case 0x58a2:
		dialog->pop(scene, dsAddr_dialogStackSonny, 0, 502, textColorMark, textColorSonny, 0, 1);
		scene->getObject(13)->setName((const char *)res->dseg.ptr(dsAddr_scnObjNameSonny));
		break;

	case 0x58b7: // Get comb from car
		disableObject(14);
		setOns(4, 0);
		playSound(5, 7);
		playActorAnimation(521);
		setOns(4, 0);
		inventory->add(kInvItemComb);
		break;

	case 0x58df: // Pull trunk lever in car
		SET_FLAG(dsAddr_alreadyPulledTrunkReleaseLeverFlag, 1);
		playSound(6, 1);
		setOns(3, 6);
		playActorAnimation(515);
		break;

	case 0x593e: // Enter annes house
		playSound(89, 4);
		playActorAnimation(980);
		loadScene(23, 76, 199, 1);
		if (CHECK_FLAG(dsAddr_lovestruckByAnneFlag, 1))
			playMusic(7);
		break;

	case 0x5994:
		fnEnterCave();
		break;

	case csAddr_caveNOP:
		break;

	case csAddr_enterCave:
		fnEnterCave();
		break;

	case 0x5a8b:
		if (!CHECK_FLAG(dsAddr_dogHasBoneFlag, 1)) {
			playSound(43, 4); // grrrrrr
			playSound(42, 15);
			playSound(42, 17);
			playSound(42, 19);
			playAnimation(656, 0);
			wait(50);
			displayMessage(dsAddr_goodDoggyMsg); // "I understand. Good doggy"
		} else if (!CHECK_FLAG(dsAddr_cellarDoorOpenFlag, 1)) { // Dog has bone
			playSound(28, 3);
			playActorAnimation(596);
			setOns(1, 30);
			SET_FLAG(dsAddr_cellarDoorOpenFlag, 1);
			enableObject(8);
		} else {
			setOns(1, 0);
			playSound(4, 4);
			playActorAnimation(597);
			SET_FLAG(dsAddr_cellarDoorOpenFlag, 0);
			disableObject(8);
			displayMessage(dsAddr_wallShakenMsg); // "Wow! This must have shaken all the nearby walls!"
			setOns(1, 32, 24);
			enableObject(4, 24);
		}
		break;

	case 0x5b3a: // Click on dog
		dialog->popMark(scene, dsAddr_dialogStackDog);
		break;

	case 0x5b59: // picking up the rope
		dialog->showMark(70, scene);
		wait(150);
		dialog->showMark(71, scene);
		moveRel(0, -12, 0);
		playSound(34, 5);
		playActorAnimation(607);
		setOns(0, 0);
		playActorAnimation(608);
		playActorAnimation(609);
		playActorAnimation(610);
		playSound(5, 25);
		playActorAnimation(611);
		moveTo(16, scene->getPosition().y, 4, true);
		inventory->add(kInvItemRopeAct2);
		disableObject(12);
		break;

	case 0x5be1: // Talk to grandpa
		dialog->pop(scene, dsAddr_dialogStackGrandpa, 0, 522, textColorMark, textColorGrandpa, 0, 1);
		break;

	case 0x5bee:
		playSound(89, 5);
		playSound(67, 11);
		playActorAnimation(982);
		displayMessage(dsAddr_emptyMsg); // "It's Empty"
		break;

	case 0x5c0d: // grandpa - drawers
		if (CHECK_FLAG(dsAddr_SearchedGrandpaDrawersFlag, 1)) {
			displayMessage(dsAddr_drawersEmptyMsg); // "There's nothing else in the drawers"
		} else {
			if (!CHECK_FLAG(dsAddr_alreadyAdjustedHoopPoleFlag, 1))
				dialog->show(24, scene, 0, 522, textColorMark, textColorGrandpa, 0, 1);

			playSound(66, 5);
			playSound(67, 20);
			playSound(5, 23);
			playActorAnimation(631);
			inventory->add(kInvItemHandkerchief);
			SET_FLAG(dsAddr_SearchedGrandpaDrawersFlag, 1);
		}
		break;

	case 0x5c84:
		if (CHECK_FLAG(dsAddr_alreadyAdjustedHoopPoleFlag, 1)) {
			inventory->add(kInvItemShotgun);
			disableObject(7);
			playSound(32, 7);
			setOns(0, 0);
			playActorAnimation(520);
		} else {
			dialog->pop(scene, dsAddr_dialogStackGrandpaShotgun, 0, 522, textColorMark, textColorGrandpa, 0, 1);
		}
		break;

	case 0x5cf0:// Exit basketball house
		playSound(88, 5);
		playActorAnimation(981);
		loadScene(20, 161, 165);
		break;

	case 0x5d24: // getting the fan
		if (CHECK_FLAG(dsAddr_alreadyAdjustedHoopPoleFlag, 1)) {
			setLan(2, 0);
			playSound(32, 7);
			playActorAnimation(508);
			disableObject(13);
			inventory->add(kInvItemFan);
		} else {
			dialog->pop(scene, dsAddr_dialogStackGrandpaFan, 0, 522, textColorMark, textColorGrandpa, 0, 1);
		}
		break;

	case 0x5e4d: // right click on ann
		if (!CHECK_FLAG(dsAddr_alreadySpokenToAnneFlag, 0)) {
			displayMessage(dsAddr_girlTalkMsg); // "I really don't know how to talk to girls"
		} else {
			moveTo(245, 198, 1);
			dialog->show(51, scene, 0, 524, textColorMark, textColorAnne, 0, 2);
			//waitLanAnimationFrame(2, 1); // too long, about 200 frames! seems to be present in original game (sic)
			SET_FLAG(dsAddr_alreadySpokenToAnneFlag, 1);
			for (byte i = 10; i <= 20; i += 2)
				playSound(13, i);
			playAnimation(528, 1);
			wait(50);
			playMusic(7);
			SET_FLAG(dsAddr_lovestruckByAnneFlag, 1);
			for (byte i = 3; i <= 17; i += 2)
				playSound(56, i);
			playActorAnimation(525);
			for (byte i = 1; i <= 13; i += 2)
				playSound(56, i);
			playSound(40, 15);
			playSound(40, 18);
			playSound(40, 22);
			playActorAnimation(526);
			playSound(54, 1);
			playSound(55, 5);
			playActorAnimation(527);
			wait(50);
			dialog->show(52, scene, 0, 524, textColorMark, textColorAnne, 0, 2);
			scene->getObject(2)->setName((const char *)res->dseg.ptr(dsAddr_scnObjNameAnne));
		}
		break;

	case 0x5f73: // exiting ann's house
		if (CHECK_FLAG(dsAddr_lovestruckByAnneFlag, 1))
			playMusic(6);
		loadScene(21, 99, 180, 3);
		break;

	case 0x5fba:
		if (CHECK_FLAG(dsAddr_nutSwappedForAppleFlag, 1)) {
			displayMessage(dsAddr_noFruitMsg); // "There are no more interesting fruits here"
		} else {
			dialog->pop(scene, dsAddr_dialogStackGetAppleOldLady, 0, 523, textColorMark, textColorOldLady, 0, 1);
		}
		break;

	case 0x607f:
		fnEgoScaredBySpider();
		break;

	case 0x6083:
		if (CHECK_FLAG(dsAddr_lightOnFlag, 1)) {
			setOns(0, 0);
			playSound(56, 10);
			playActorAnimation(599);
			inventory->add(kInvItemShovelAct2);
			disableObject(2);
		} else
			fnEgoScaredBySpider();
		break;

	case csAddr_egoScaredBySpider:
		fnEgoScaredBySpider();
		break;

	case csAddr_moveToLadderAndLeaveCellar:
		fnMoveToLadderAndLeaveCellar();
		break;

	case csAddr_leaveCellar:
		fnLeaveCellar();
		break;

	case 0x6176:
		if (CHECK_FLAG(dsAddr_lightOnFlag, 1)) {
			displayMessage(dsAddr_notInDarkMsg); // "I'm not going to wander here in the dark again"
		} else {
			playSound(71, 6);
			playActorAnimation(598);
			loadScene(24, scene->getPosition());
			setOns(2, 0);
			setLan(1, 0);
			playAnimation(660, 0);
			disableObject(1);
			SET_FLAG(dsAddr_lightOnFlag, 1);
			loadScene(24, scene->getPosition());
		}
		break;

	case 0x61e9:
		if (CHECK_FLAG(dsAddr_lightOnFlag, 1))
			dialog->popMark(scene, dsAddr_dialogStackTakeAxe);
		else
			fnTooDark();
		break;

	case csAddr_TooDark:
		displayMessage(dsAddr_TooDarkMsg); // "It's too dark to see clearly"
		break;

	case 0x6229: // shelves in cellar
		if (CHECK_FLAG(dsAddr_lightOnFlag, 1)) {
			Common::Point p = scene->getPosition();
			byte v = GET_FLAG(dsAddr_cellarShelfExamineCount);
			switch (v) {
			case 0:
				displayMessage(dsAddr_whatGotMsg); // "Let's look what we've got here"
				moveRel(-34, 0, 1);
				displayMessage(dsAddr_strawberryJamMsg); // "Strawberry jam"
				moveRel(20, 0, 1);
				displayMessage(dsAddr_gooseberryJamMsg); // "Gooseberry jam"
				moveRel(20, 0, 1);
				displayMessage(dsAddr_blackberryJamMsg); // "Blackberry jam"
				moveRel(20, 0, 1);
				displayMessage(dsAddr_bilberryJamMsg); // "Bilberry jam"
				moveTo(p, 3);
				displayMessage(dsAddr_getMeOutJamMsg); // "Get me out of this jam!"
				SET_FLAG(dsAddr_cellarShelfExamineCount, 1);
				break;
			case 1:
				displayMessage(dsAddr_rosemaryJamMsg); // "Oh, and there is Rosemary jam"
				wait(100);
				displayMessage(dsAddr_knowRosemaryMsg); // "I used to know someone called Rosemary"
				SET_FLAG(dsAddr_cellarShelfExamineCount, 2);
				break;
			default:
				displayMessage(dsAddr_unwantedJamsMsg); // "I don't want those jams"
				break;
			}
		} else
			fnTooDark();
		break;

	case 0x6480: // dive mask
		if (CHECK_FLAG(dsAddr_birdsGoneFromScarecrowFlag, 1)) {
			playSound(56, 7);
			playSound(5, 15);
			playActorAnimation(613);
			setOns(3, 36);
			inventory->add(kInvItemMask);
			disableObject(5);
			displayMessage(dsAddr_needSunglassesMsg); // "Sorry buddy, but I need your sunglasses"
		} else
			displayMessage(dsAddr_crowKillMsg); // "I'm sure these crows will kill me"
		break;

	case 0x64c4: // flippers
		if (CHECK_FLAG(dsAddr_birdsGoneFromScarecrowFlag, 1)) {
			setOns(2, 35);
			playSound(63, 8);
			playSound(24, 10);
			playActorAnimation(612);
			inventory->add(kInvItemFins);
			disableObject(6);
		} else
			displayMessage(dsAddr_crowKillMsg); // "I'm sure these crows will kill me"
		break;

	case 0x7907: // Describe car lever
		if (CHECK_FLAG(dsAddr_alreadyPulledTrunkReleaseLeverFlag, 1)) { // Already pulled lever?
			displayMessage(dsAddr_openBootMsg); // "It opens the boot"
		} else
			retVal = false;
		break;

	case 0x62d0: // Get bone from under rock
		displayAsyncMessage(dsAddr_yeowMsg, 218, 96, 16, 24); // "YEEEOOOWWWW!"
		playSound(26, 6);
		playSound(26, 10);
		playSound(24, 13);
		playSound(46, 37);
		setOns(0, 0);
		playActorAnimation(594);
		setOns(0, 29);
		disableObject(1);
		inventory->add(kInvItemBone);
		playSound(5, 2);
		playActorAnimation(595);
		displayMessage(dsAddr_dinoBoneMsg); // "I really hope this is DINOSAUR bone"
		break;

	case 0x6351:
		if (CHECK_FLAG(dsAddr_caveThornsCutDownFlag, 1)) { // cave bush is cut down
			playMusic(8);
			loadScene(26, 319, 169, 4);
		} else
			displayMessage(dsAddr_ridBushMsg); // "I must get rid of this bush first"
		break;

	case 0x63ea:
		playSound(5, 10);
		setOns(0, 0);
		playActorAnimation(640);
		inventory->add(kInvItemNugget);
		disableObject(6);
		break;

	case 0x6411: // Kick hen
		if (CHECK_FLAG(dsAddr_alreadyKickedHenFlag, 1)) { // already kicked hen
			displayMessage(dsAddr_ridFrustationsMsg); // "I'd already got rid of my frustrations"
		} else {
			SET_FLAG(dsAddr_alreadyKickedHenFlag, 1);
			displayMessage(dsAddr_henFlyMsg); // "I wonder if hens can fly. Come here, baby"
			waitLanAnimationFrame(1, 87);
			playSound(30, 26);
			playSound(29, 49);
			playActorAnimation(500, true);
			playAnimation(501, 0, true);
			waitAnimation();
			setOns(0, 1);
			enableObject(14);
			displayMessage(dsAddr_firstTestFailMsg); // "First test failed"
		}
		break;

	case 0x6592: // Rake
		setOns(1, 0);
		playSound(18, 10);
		playActorAnimation(553);
		inventory->add(kInvItemRakeBroken);
		wait(50);
		displayMessage(dsAddr_trousersMsg); // "Good I always asked mum for trousers with BIG pockets"
		disableObject(11);
		break;

	case 0x66b5:
		playSound(89, 5);
		playActorAnimation(969);
		loadScene(33, 319, 181, 4);
		break;

	case 0x6519: // Sickle
		setOns(4, 0);
		playSound(5, 11);
		playActorAnimation(625);
		inventory->add(kInvItemSickleBlunt);
		disableObject(8);
		break;

	case 0x655b: // Get needle from haystack
		if (CHECK_FLAG(dsAddr_gotNeedleAlreadyFlag, 1)) { // already have needle
			displayMessage(dsAddr_dontPushLuckMsg); // "I don't think I should push my luck"
		} else {
			SET_FLAG(dsAddr_gotNeedleAlreadyFlag, 1);
			playSound(49, 3);
			playActorAnimation(548);
			inventory->add(kInvItemNeedle);
			displayMessage(dsAddr_needleHaystackMsg); // "And they say you can't find a needle in a haystack"
		}
		break;

	case 0x663c: // Feather
		setOns(0, 0);
		playSound(5, 9);
		playActorAnimation(511);
		inventory->add(kInvItemFeather);
		disableObject(15);
		break;

	case 0x667c:
		playSound(70, 4);
		playActorAnimation(972);
		loadScene(29, 160, 199, 1);
		break;

	case 0x66a9:
		displayMessage(dsAddr_dontLeaveMansionMsg); // "I don't want to leave the mansion, I want blood!"
		disableObject(4);
		break;

	case 0x66e2:
		playSound(88, 4);
		playActorAnimation(970);
		loadScene(35, 160, 199, 1);
		break;

	case 0x70bb:
		dialog->pop(scene, dsAddr_dialogStackBusyCook, 0, 709, textColorMark, textColorCook, 0, 1);
		break;

	case 0x71ae:
		if (CHECK_FLAG(dsAddr_MansionRadioBrokenFlag, 1)) {
			if (CHECK_FLAG(dsAddr_MansionGotRadioBatteriesFlag, 1)) {
				displayMessage(dsAddr_restUselessMsg); // "The rest is useless"
			} else {
				displayMessage(dsAddr_twoBatteriesMsg); // "Wow! Two 1.5V batteries!"
				playSound(32, 6);
				playActorAnimation(717);
				inventory->add(kInvItemBatteries);
				SET_FLAG(dsAddr_MansionGotRadioBatteriesFlag, 1);
			}
		} else
			dialog->showMark(97, scene);
		break;

	case 0x70c8:
		if (fnIsCookGone()) {
			moveTo(81, 160, 4);
			displayMessage(dsAddr_cognacMsg); // "Pfui! The cognac really didn't do any good"
		}
		break;

	case csAddr_isCookGone:
		retVal = fnIsCookGone();
		break;

	case 0x70ef:
		if (fnIsCookGone())
			displayMessage(dsAddr_tooHotMsg); // "It's too hot to touch!"
		break;

	case 0x70f9:
		if (inventory->has(kInvItemBurningPaper)) {
			inventory->remove(kInvItemBurningPaper);
			loadScene(29, 40, 176, 2);
			displayMessage(dsAddr_paperBurntMsg); // "The paper burnt out completely!"
		} else
			loadScene(29, 40, 176, 2);
		break;

	case 0x712c:
		if (fnIsCookGone()) {
			if (CHECK_FLAG(dsAddr_MansionHaveOpenedFridgeBeforeFlag, 1)) {
				playSound(89, 4);
				playActorAnimation(719);
				setOns(4, 67);
				++ *res->dseg.ptr(READ_LE_UINT16(res->dseg.ptr(dsAddr_sceneWalkboxTablePtr + (scene->getId() - 1) * 2)));
				disableObject(5);
				enableObject(12);
			} else {
				playSound(89, 4);
				playSound(89, 4);
				playSound(87, 45);
				displayAsyncMessage(dsAddr_oneTakenMsg, 112, 108, 11, 35, textColorEskimo); // "This one's taken, OK?"
				playActorAnimation(718);
				wait(100);
				displayMessage(dsAddr_slightMadMsg); // "It finally happened. I'm slightly mad"
				SET_FLAG(dsAddr_MansionHaveOpenedFridgeBeforeFlag, 1);
			}
		}
		break;

	case 0x71eb:
		setOns(2, 0);
		playSound(32, 7);
		playActorAnimation(710);
		inventory->add(kInvItemChilliWithLabel);
		disableObject(7);
		enableObject(8);
		break;

	case 0x7244:
		if (fnIsCookGone())
			displayMessage(dsAddr_neverLearntMsg); // "I never learnt to how use one"
		break;

	case 0x7255:
		if (CHECK_FLAG(dsAddr_MansionPutBurningPaperInFridgeFlag, 1)) {
			setOns(4, 69);
			playSound(32, 5);
			playActorAnimation(725);
			disableObject(12);
			inventory->add(kInvItemMeat);
		} else {
			playActorAnimation(721);
			displayMessage(dsAddr_frozenShelfMsg); // "It has frozen hard onto the shelf!"
		}
		break;

	case 0x721c:
		setOns(3, 0);
		playSound(32, 7);
		playActorAnimation(715);
		inventory->add(kInvItemPastryRoller);
		disableObject(9);
		break;

	case 0x7336:
		setOns(1, 0);
		playSound(5, 42);
		displayAsyncMessage(dsAddr_noDepraveMsg, 2, 102, 20, 38); // "Nah, I don't want to deprave the kids"
		playActorAnimation(697);
		inventory->add(kInvItemCognac);
		disableObject(1);
		break;

	case 0x7381:
		playSound(5, 12);
		playActorAnimation(704);
		disableObject(2);
		inventory->add(kInvItemIceTongs);
		break;

	case 0x7408:
		if (CHECK_FLAG(dsAddr_mansionReadNewspaperFlag, 1)) {
			displayMessage(dsAddr_noReadAgainMsg); // "I don't want to read it again. I might like it."
		} else {
			setOns(0, 0);
			playSound(26, 17);
			playSound(26, 23);
			playSound(26, 30);
			playSound(26, 37);
			playSound(26, 43);
			playSound(52, 34);
			playActorAnimation(698);
			setOns(0, 52);
			setOns(2, 61);
			dialog->showMark(92, scene);
			enableObject(11);
			SET_FLAG(dsAddr_mansionReadNewspaperFlag, 1);
		}
		break;

	case 0x7476:
		if (CHECK_FLAG(dsAddr_mansionExaminedCouchBeforeFlag, 1)) {
			displayMessage(dsAddr_noSleepMsg); // "I don't want to sleep"
		} else {
			SET_FLAG(dsAddr_mansionExaminedCouchBeforeFlag, 1);
			dialog->showMark(94, scene);
			playSound(61, 5);
			playSound(5, 14);
			playActorAnimation(705);
			displayMessage(dsAddr_justCorkMsg); // "It's just a cork"
			inventory->add(kInvItemCork);
		}
		break;

	case 0x74d1:
		setOns(2, 0);
		playSound(5, 12);
		playActorAnimation(699);
		inventory->add(kInvItemRemoteControl);
		disableObject(11);
		break;

	case 0x7513: // fatso + doctor: pre-final
		if (CHECK_FLAG(dsAddr_MansionThruFanByTimePillFlag, 1)) {
			if (CHECK_FLAG(dsAddr_MansionVentFanStoppedFlag, 1)) {
				playSound(88, 4);
				playActorAnimation(979);
				loadScene(37, 51, 183);

				dialog->show(125, scene, 768, 769, textColorMansionGuard, textColorProfessor, 1, 2);
				playAnimation(770, 0, true, true, true);
				playAnimation(771, 1, true, true, true);
				dialog->showMono(126, scene, 0, textColorMark, 0);
				playAnimation(770, 0, true, true, true);
				playAnimation(771, 1, true, true, true);
				playSound(5, 3);
				playSound(56, 12);
				playSound(23, 20);
				playSound(75, 25);
				playActorAnimation(772);

				playActorAnimation(773, true);
				playAnimation(774, 0, true);
				waitAnimation();
				setOns(0, 74);
				hideActor();
				dialog->showMono(127, scene, 775, textColorJohnNoty, 1);
				playAnimation(771, 1, true, true, true);
				playAnimation(776, 0);

				dialog->show(128, scene, 777, 778, textColorJohnNoty, textColorProfessor, 1, 2);

				playAnimation(779, 0, true, true, true);
				playAnimation(780, 1, true, true, true);

				for (byte i = 1; i <= 6; ++i)
					playSound(58, i);
				playSound(58, 10);
				playSound(2, 7);
				playSound(55, 11);
				playSound(54, 15);
				playAnimation(781, 2, true);
				playAnimation(782, 3, true);
				waitAnimation();
				setOns(1, 75);
				setOns(2, 76);

				for (byte i = 1; i <= 6; ++i)
					playSound(58, i);
				playSound(58, 9);
				playSound(2, 7);
				playSound(2, 15);
				playSound(55, 10);

				playAnimation(783, 2, true);
				playAnimation(784, 3, true);
				waitAnimation();
				setOns(1, 77);
				setOns(2, 78);

				playAnimation(785, 2, true);
				playAnimation(786, 3, true);
				waitAnimation();

				moveTo(112, 183, 2, true);

				setOns(3, 79);
				setOns(0, 0);

				showActor();
				playAnimation(0, 0);
				playAnimation(787, 2, true);
				playAnimation(788, 3, true);
				waitAnimation();

				playAnimation(0, 1);

				playSound(32, 2);
				playSound(24, 7);

				playAnimation(790, 3, true);
				playAnimation(789, 0, true);
				waitAnimation();

				setOns(0, 80);

				playAnimation(792, 3, true, true, true);
				dialog->show(129, scene, 0, 791, textColorMark, textColorJohnNoty, 0, 4);
				playAnimation(792, 3, true, true, true);

				moveTo(40, 171, 4);

				setOns(3, 81, 35);
				enableObject(12, 35);
				playAnimation(0, 3);

				loadScene(31, 298, 177, 4);
				SET_FLAG(dsAddr_MansionJohnNotyEscapingFlag, 1);
			} else
				displayMessage(dsAddr_ventFirstMsg); // "I'd better stop this ventilator first"
		} else
			displayMessage(dsAddr_noSaladMsg); // "I don't want to turn myself into a salad"
		break;

	case 0x783d:
		dialog->pop(scene, dsAddr_dialogStackJohnNotyEndgame, 0, 797, textColorMark, textColorJohnNoty, 0, 1);
		break;

	case 0x7966:
		if (CHECK_FLAG(dsAddr_lightOnFlag, 1))
			retVal = false;
		else
			fnEgoScaredBySpider();
		break;

	case 0x7ad0:
	case 0x7ad7:
		retVal = !fnIsCookGone();
		break;

	case 0x7ab9:
		if (CHECK_FLAG(dsAddr_vgaArtistQuipAlreadySaidFlag, 1))
			retVal = false;
		else {
			dialog->showMono(90, scene, 0, textColorMark, 0);
			SET_FLAG(dsAddr_vgaArtistQuipAlreadySaidFlag, 1);
		}
		break;

	case 0x7ade:
		if (CHECK_FLAG(dsAddr_MansionRadioBrokenFlag, 1))
			displayMessage(dsAddr_whatInsideMsg); // "I was always curious what's inside these things"
		else
			retVal = false;
		break;

	case 0x7f23: // Use grenade on captains drawer
		if (CHECK_FLAG(dsAddr_FirstActTrialState, 3)) {
			enableOn(false);
			playSound(5, 3);
			playSound(58, 11);
			playSound(46, 56);
			playSound(46, 85);
			playSound(46, 117);
			playActorAnimation(870);
			playSound(54, 15);
			playActorAnimation(871);
			SET_FLAG(dsAddr_captainDrawerState, 1);
			setOns(1, 0x66);
			moveTo(224, 194, 0, true);
			displayCutsceneMessage(dsAddr_cutsceneMsg1, 23, 95); // "sixty seven rude words later"
			inventory->remove(kInvItemRopeAndGrenade);
			enableOn(true);
		} else
			displayMessage(dsAddr_captainWatchingMsg); // "with captain watching? Better not"
		break;

	case csAddr_egoSuspiciousPosition:
		fnEgoSuspiciousPosition();
		break;

	case 0x509a:
		fnEgoSuspiciousPosition();
		setOns(1, 0);
		playSound(5, 10);
		playActorAnimation(543);
		inventory->add(kInvItemBranch);
		disableObject(9);
		break;

	case 0x7802:
		if (CHECK_FLAG(dsAddr_MansionThruFanByTimePillFlag, 1)) {
			if (CHECK_FLAG(dsAddr_MansionVentFanStoppedFlag, 1))
				displayMessage(dsAddr_nahMsg); // "Nah"
			else {
				playSound(71, 4);
				playActorAnimation(796);
				setLan(1, 0);
				SET_FLAG(dsAddr_MansionVentFanStoppedFlag, 1);
			}
		} else
			displayMessage(dsAddr_noSaladMsg); // "I don't want to turn myself into a salad"
		break;

	case 0x78e0:
		fnEgoSuspiciousPosition();
		retVal = false;
		break;

	case 0x78e7:
	case 0x78ee:
		fnEgoDefaultPosition();
		retVal = false;
		break;

	case 0x78f5:
		if (CHECK_FLAG(dsAddr_carTrunkEmptyFlag, 1)) {
			displayMessage(dsAddr_bootEmptyMsg); // "There's nothing else in the boot"
		} else
			retVal = false;
		break;

	case 0x7919:
		if (!CHECK_FLAG(dsAddr_laundryState, 1))
			retVal = false;
		else
			displayMessage(dsAddr_clothesDryMsg); // "The clothes are dry now."
		break;

	case 0x7950:
		if (CHECK_FLAG(dsAddr_nutSwappedForAppleFlag, 1))
			displayMessage(dsAddr_nutRealMsg); // "Only the nut is real"
		else
			retVal = false;
		break;

	case 0x7975:
		if (CHECK_FLAG(dsAddr_lightOnFlag, 1))
			retVal = false;
		else
			displayMessage(dsAddr_shutValveMsg); // "Shutting the valve shook the dirt from the wall..."
		break;

	case 0x7987:
	case 0x7996:
	case 0x79a5:
	case 0x79b4:
		if (CHECK_FLAG(dsAddr_lightOnFlag, 1))
			retVal = false;
		else
			fnTooDark();
		break;

	case 0x79d2:
		if (!CHECK_FLAG(dsAddr_gotNeedleAlreadyFlag, 1))
			retVal = false;
		else
			displayMessage(dsAddr_ordinaryHaystackMsg); // "Just an ordinary hay stack. Now."
		break;

	case 0x7af0:
		if (fnIsCookGone())
			retVal = false;
		break;

	case 0x8117:
		dialog->show(9, scene, 0, 529, textColorMark, textColorMansionGuard, 0, 1);
		playSound(5, 2);
		playSound(5, 44);
		playAnimation(642, 0, true);
		playActorAnimation(641, true);
		waitAnimation();
		dialog->show(10, scene, 0, 529, textColorMark, textColorMansionGuard, 0, 1);
		wait(170);
		dialog->show(11, scene, 0, 529, textColorMark, textColorMansionGuard, 0, 1);
		moveRel(0, 1, 0);
		wait(100);
		dialog->show(12, scene, 0, 529, textColorMark, textColorMansionGuard, 0, 1);
		inventory->remove(kInvItemNugget);
		fnMansionIntrusionAttempt();
		break;

	case 0x8174:
		setOns(0, 0);
		playSound(5, 2);
		playSound(5, 5);
		playSound(5, 9);
		playSound(14, 19);
		playSound(5, 50);
		playActorAnimation(542);
		setOns(1, 15);
		disableObject(3);
		enableObject(9);
		break;

	case 0x81c2:
		playSound(56, 11);
		playSound(36, 13);
		playSound(48, 22);
		playSound(56, 57);
		playSound(36, 59);
		playSound(48, 68);
		playSound(54, 120);
		playSound(56, 141);
		playSound(56, 144);
		playSound(56, 147);
		playAnimation(589, 1, true);
		playActorAnimation(588, true);
		waitAnimation();
		wait(50);
		displayMessage(dsAddr_itsGoneMsg); // "At least it's gone"
		inventory->remove(kInvItemPaintedPotato);
		SET_FLAG(dsAddr_mansionTreeHollowEmptyFlag, 1);
		break;

	case 0x823d: // grappling hook on the wall
		playSound(5, 3);
		for (byte i = 16; i <= 28; i += 2)
			playSound(65, i);
		playSound(47, 33);
		playActorAnimation(620);
		for (byte i = 3; i <= 18; i += 3)
			playSound(56, i);

		displayAsyncMessage(dsAddr_heyLetGoMsg, 38, 12, 20, 37, textColorMansionGuard); // "Hey, let go, will ya?!"
		playActorAnimation(621, true);
		playAnimation(623, 1, true);
		waitAnimation();

		displayAsyncMessage(dsAddr_aaahhhMsg, 30, 12, 1, 9, textColorMansionGuard); // "Aaaaaaaaaaaaahhh!"
		playSound(35, 1);
		playActorAnimation(622, true);
		playAnimation(624, 0, true);
		waitAnimation();

		wait(150);
		displayMessage(dsAddr_oopsMsg); // "Oops"

		inventory->remove(kInvItemGrapplingHook);
		fnMansionIntrusionAttempt();
		break;

	case 0x8312: // hedgehog + plastic apple
		dialog->showMark(76, scene);
		setLan(1, 0);
		playSound(5, 24);
		playSound(26, 32);
		playSound(5, 42);
		playSound(15, 77);
		playSound(15, 79);
		playSound(15, 82);
		playSound(22, 91);
		playSound(22, 102);
		playSound(26, 114);
		playSound(24, 124);
		waitLanAnimationFrame(1, 0x1a);
		playActorAnimation(562, true);
		playAnimation(563, 1, true);
		waitAnimation();

		disableObject(6);
		displayMessage(dsAddr_lifeIsBrutalMsg); // "Life is brutal"
		inventory->remove(kInvItemPlasticApple);
		inventory->add(kInvItemCone);
		break;

	case 0x839f:
		inventory->remove(kInvItemDart);
		playSound(37, 14);
		playSound(16, 17);
		playActorAnimation(564, true);
		playAnimation(565, 2, true);
		waitAnimation();
		setOns(0, 24);
		playSound(39, 5);
		playActorAnimation(582);
		moveTo(63, 195, 1);
		playAnimation(571, 1);
		playAnimation(572, 1);
		playAnimation(573, 1);
		for (byte i = 1; i <= 7; i += 2)
			playSound(40, i);
		playAnimation(574, 1);
		setLan(1, 0);
		playAnimation(575, 1);
		playAnimation(576, 1);
		playAnimation(577, 1);
		playAnimation(578, 1);
		playAnimation(579, 1);
		playAnimation(580, 1);
		playSound(55, 18);
		playAnimation(581, 1);
		disableObject(2);
		SET_FLAG(dsAddr_beesGoneFlag, 1);
		break;

	case 0x84c7: // using paddle on boat
		playSound(20, 9);
		playActorAnimation(530);
		loadScene(16, 236, 95, 1);
		playMusic(9);
		playActorAnimation(531);
		playSound(36, 4);
		playActorAnimation(532);
		playActorAnimation(533);
		setOns(0, 9);
		moveTo(236, 95, 1, true);
		break;

	case 0x8538: // Sharpen sickle on well
		moveTo(236, 190, 0);
		setOns(2, 0);
		// FIXME: Add code to Remove handle sprite (visible GFX glitch)
		playSound(5, 4);
		playSound(14, 14);
		playSound(14, 33);
		playSound(5, 43);
		playActorAnimation(643);
		setOns(2, 43);
		moveTo(236, 179, 3);
		inventory->remove(kInvItemSickleBlunt);
		inventory->add(kInvItemSickleSharp);
		break;

	case 0x85d6:
		displayMessage(dsAddr_paddleBrokenMsg); // "The paddle is BROKEN"
		break;

	case 0x85eb:
		if (CHECK_FLAG(dsAddr_squirrelNutState, 1)) {
			enableObject(6);
			playSound(25, 10);
			playSound(25, 14);
			playSound(25, 18);
			playActorAnimation(559);
			setOns(1, 23);
			SET_FLAG(dsAddr_squirrelNutState, 2);
		} else
			displayMessage(dsAddr_dontWorkPurposeMsg); // "I usually don't work without a purpose"
		break;

	case 0x863d:
		playSound(12, 4);
		playSound(50, 20);
		playSound(50, 29);
		playActorAnimation(554);
		inventory->remove(kInvItemChocCandy);
		inventory->add(kInvItemHeartShapedCandy);
		break;

	case 0x8665:
		playSound(5, 3);
		for (byte i = 12; i <= 24; i += 2)
			playSound(56, i);
		playActorAnimation(567);
		inventory->remove(kInvItemFeatherDusterClean);
		inventory->add(kInvItemFeatherDusterDirty);
		break;

	case 0x862c:
		if (CHECK_FLAG(dsAddr_squirrelNutState, 1))
			displayMessage(dsAddr_nutRakeMsg); // "It's pointless, the nut will slip between the rake's teeth"
		else
			displayMessage(dsAddr_objErrorMsg); // "That's no good"
		break;

	case 0x86a9: // correcting height of the pole with spanner
		if (CHECK_FLAG(dsAddr_alreadyAdjustedHoopPoleFlag, 1)) {
			displayMessage(dsAddr_noNeedMsg); // "No need to do it again"
		} else {
			SET_FLAG(dsAddr_alreadyAdjustedHoopPoleFlag, 1);
			dialog->show(17, scene, 0, 502, textColorMark, textColorSonny, 0, 1);
			waitLanAnimationFrame(1, 7);
			playSound(5, 16);
			playSound(1, 25);
			playSound(1, 29);
			playSound(1, 34);
			playAnimation(506, 0, true);
			playActorAnimation(504, true);
			waitAnimation();

			setOns(0, 0);
			playSound(24, 2);
			playSound(22, 24);
			playSound(1, 28);
			playSound(1, 32);
			playSound(1, 37);
			playSound(5, 43);
			playSound(61, 70);
			playSound(61, 91);
			displayAsyncMessage(dsAddr_ConfusionMsg, 77, 90, 6, 17); // "!?&!"
			playActorAnimation(505, true);
			playAnimation(507, 0, true);
			waitAnimation();

			setOns(0, 4);
			{
				Object *obj = scene->getObject(3);
				obj->rect.top += 20;
				obj->rect.bottom += 20;
				obj->rect.save();
			}
			playSound(10, 3);
			playAnimation(503, 0);
			setLan(1, 0, 22);
			disableObject(1, 22);
			disableObject(13, 20);
			setLan(1, 0);
			disableObject(1);
			disableObject(2);
			disableObject(14);
			disableObject(15);
			disableObject(16);
			moveTo(162, 164, 2);
			displayMessage(dsAddr_grandpaPromiseMsg, textColorSonny, 70, 76); // "But grandpa, you promised!"
			displayMessage(dsAddr_ohLetsGoMsg, textColorGrandpa, 90, 76); // "Oh all right. Let's go"
			moveTo(162, 191, 2);
			setOns(1, 0);
			setOns(2, 0);
			setOns(3, 0);
			setOns(4, 0);

			{
				Walkbox *w = scene->getWalkbox(0);
				w->rect.clear();
				w->save();
			}

			playSound(62, 1);
			playSound(9, 8);
			setLan(1, 0);
			playAnimation(512, 0);

			wait(100);
			displayMessage(dsAddr_byeMsg); // "Bye."
			{
				Object *obj = scene->getObject(7);
				obj->actorRect.left = obj->actorRect.right = 228;
				obj->actorRect.top = obj->actorRect.bottom = 171;
				obj->actorRect.save();
			}
			{
				Object *obj = scene->getObject(8);
				obj->actorRect.left = obj->actorRect.right = 290;
				obj->actorRect.top = obj->actorRect.bottom = 171;
				obj->actorRect.save();
			}
		}
		break;

	case 0x88c9: // give flower to old lady
		if (CHECK_FLAG(dsAddr_givenFlowerToOldLadyAlreadyFlag, 1))
			fnGiveAnotherFlowerToOldLady();
		else {
			inventory->remove(kInvItemFirstFlower);
			SET_FLAG(dsAddr_givenFlowerToOldLadyAlreadyFlag, 1);
			fnGivingFlowerToOldLady();
		}
		break;

	case csAddr_givingFlowerToOldLady:
		fnGivingFlowerToOldLady();
		break;

	case csAddr_giveAnotherFlowerToOldLady:
		fnGiveAnotherFlowerToOldLady();
		break;

	case 0x8918: // give flower to old lady
		if (CHECK_FLAG(dsAddr_givenFlowerToOldLadyAlreadyFlag, 1))
			fnGiveAnotherFlowerToOldLady();
		else {
			inventory->remove(kInvItemSecondFlower);
			SET_FLAG(dsAddr_givenFlowerToOldLadyAlreadyFlag, 1);
			fnGivingFlowerToOldLady();
		}
		break;

	case 0x892d:
		if (CHECK_FLAG(dsAddr_givenFlowerToAnneAlreadyFlag, 1))
			fnGiveAnotherFlowerToAnne();
		else {
			fnGivingFlowerToAnne();
			inventory->remove(kInvItemFirstFlower);
			SET_FLAG(dsAddr_givenFlowerToAnneAlreadyFlag, 1);
		}
		break;

	case csAddr_givingFlowerToAnne:
		fnGivingFlowerToAnne();
		break;

	case csAddr_giveAnotherFlowerToAnne:
		fnGiveAnotherFlowerToAnne();
		break;

	case 0x89b7:
		if (CHECK_FLAG(dsAddr_givenFlowerToAnneAlreadyFlag, 1))
			fnGiveAnotherFlowerToAnne();
		else {
			fnGivingFlowerToAnne();
			inventory->remove(kInvItemSecondFlower);
			SET_FLAG(dsAddr_givenFlowerToAnneAlreadyFlag, 1);
		}
		break;

	case 0x89cc:
		inventory->remove(kInvItemWrappedCandy);
		playSound(5, 6);
		dialog->show(60, scene, 0, 524, textColorMark, textColorAnne, 0, 2);
		// FIXME - Dialog #61 not explicitly called. Does Dialog #60 run on somehow?
		playActorAnimation(555, true);
		playAnimation(556, 1, true);
		waitAnimation();
		playActorAnimation(557, true);
		playAnimation(558, 1, true);
		waitAnimation();
		dialog->show(62, scene, 0, 524, textColorMark, textColorAnne, 0, 2);
		inventory->add(kInvItemRibbon);
		break;

	case 0x8a22:
		playSound(45, 16);
		playActorAnimation(560);
		inventory->remove(kInvItemNut);
		inventory->add(kInvItemPlasticApple);
		wait(50);
		dialog->show(44, scene, 0, 523, textColorMark, textColorOldLady, 0, 1);
		dialog->show(45, scene, 0, 523, textColorMark, textColorOldLady, 0, 1);
		SET_FLAG(dsAddr_nutSwappedForAppleFlag, 1);
		break;

	case 0x8a6f: // banknote + ann
		if (CHECK_FLAG(dsAddr_examinedBanknoteFlag, 1)) {
			dialog->show(63, scene, 0, 524, textColorMark, textColorAnne, 0, 2);
			playSound(5, 3);
			playSound(5, 20);
			playAnimation(671, 1, true);
			playActorAnimation(670, true);
			waitAnimation();
			//playAnimation(672, 1);
			dialog->show(64, scene, 524, 672, textColorMark, textColorAnne, 0, 2);
			//playAnimation(672, 1);

			playSound(83, 12);
			displayAsyncMessage(dsAddr_hundredBucksMsg, 204, 114, 23, 38, textColorAnne); // "A hundred bucks!!!"
			playActorAnimation(673);
			loadScene(11, scene->getPosition());
			playSound(24, 31);
			playSound(24, 48);
			playSound(79, 50);
			playActorAnimation(674, true);
			playAnimation(675, 0, true);
			waitAnimation();
			loadScene(28, 0, 167, 2);
			playMusic(10);
			moveTo(66, 167, 2);
			displayMessage(dsAddr_wantBloodMsg); // "I want Blood!"
			inventory->clear();
			inventory->add(kInvItemSuperGlue);
		} else
			displayMessage(dsAddr_showHerMoneyMsg); // "If I just show her the money, she might take it"
		break;

	case 0x8b82: // use fan on laundry
		setOns(0, 0);
		playSound(5, 3);
		playSound(5, 6);
		playSound(5, 10);
		playSound(92, 20);
		playSound(92, 38);
		playSound(92, 58);
		displayAsyncMessage(dsAddr_yawnMsg, 30, 114, 58, 67); // "(yawn)"
		playActorAnimation(602);
		playSound(5, 3);
		playActorAnimation(603);
		setOns(0, 27);
		SET_FLAG(dsAddr_laundryState, 1);
		break;

	case 0x8bfc: // Give bone to dog
		displayMessage(dsAddr_hereBoyMsg); // "Here, boy"
		playSound(5, 3);
		playSound(26, 13);
		playActorAnimation(657, true);
		playAnimation(658, 0, true);
		waitAnimation();

		reloadLan();
		playAnimation(659, 0);

		inventory->remove(kInvItemBone);
		SET_FLAG(dsAddr_dogHasBoneFlag, 1);
		{
			Object *o = scene->getObject(7);
			o->actorRect.left = o->actorRect.right = 297;
			o->actorRect.top = o->actorRect.bottom = 181;
			o->actorOrientation = 1;
			o->save();
		}
		{
			Object *o = scene->getObject(9);
			o->actorRect.left = o->actorRect.right = 297;
			o->actorRect.top = o->actorRect.bottom = 181;
			o->actorOrientation = 1;
			o->save();
		}
		{
			Walkbox *w = scene->getWalkbox(0);
			w->rect.right = 266;
			w->rect.bottom = 193;
			w->save();
		}
		wait(100);
		displayMessage(dsAddr_friendsNowMsg); // "I hope we're friends now"
		break;

	case 0x8c6e: // Use car jack on rock
		playSound(5, 3);
		playSound(26, 13);
		playSound(24, 22);
		playActorAnimation(592);
		playSound(1, 5);
		playSound(1, 9);
		playSound(1, 13);
		setOns(0, 0);
		playActorAnimation(593);
		setOns(0, 28);
		enableObject(1);
		inventory->remove(kInvItemCarJack);
		break;

	case 0x8cc8: // Cut bush with sickle
		playSound(5, 3);
		playActorAnimation(644);
		setOns(1, 45);
		playSound(56, 2);
		playSound(26, 4);
		playActorAnimation(645);
		playSound(56, 1);
		playSound(56, 6);
		playSound(26, 3);
		playSound(26, 8);
		playActorAnimation(646);
		playSound(5, 21);
		playActorAnimation(647);
		SET_FLAG(dsAddr_caveThornsCutDownFlag, 1);
		inventory->remove(kInvItemSickleSharp);
		disableObject(2);
		scene->getObject(3)->actorRect.right = 156;
		scene->getObject(3)->save();
		break;

	case csAddr_mouseOutOfHoleTimeout: // mouse falls back from the hole (cave)
		if (CHECK_FLAG(dsAddr_timedCallbackState, 1)) {
			inventory->add(kInvItemMouse);
			playSound(24, 26);
			playActorAnimation(650, true);
			playAnimation(651, 2, true);
			waitAnimation();
		} else {
			playSound(27, 5);
			setOns(1, 0);
			playActorAnimation(652);
			playSound(24, 4);
			playSound(5, 12);
			moveTo(186, 179, 1, true);
			playActorAnimation(653);
			playActorAnimation(654, true);
			playAnimation(655, 2, true);
			waitAnimation();
			displayMessage(dsAddr_mouseGoneMsg); // "The mouse has gone!"
			inventory->add(kInvItemRock);
			setLan(2, 4, 27);
			enableObject(4, 27);
			SET_FLAG(dsAddr_mouseHoleState, 0);
		}
		SET_FLAG(dsAddr_timedCallbackState, 0);
		break;

	case csAddr_putRockInHole:
		fnPutRockInHole();
		break;

	case 0x8f1d:
		dialog->showMark(72, scene);
		for (uint i = 16; i <= 30; i += 2)
			playSound(56, i);
		playSound(2, 64);
		playSound(3, 74);
		displayAsyncMessage(dsAddr_lastChanceMsg, 212, 80, 35, 50); // "Last chance?"
		playActorAnimation(516, true);
		playAnimation(517, 2, true);
		playAnimation(518, 3, true);
		waitAnimation();
		setLan(3, 0);
		setLan(4, 0);
		disableObject(2);
		disableObject(3);
		inventory->remove(kInvItemShotgun);
		SET_FLAG(dsAddr_birdsGoneFromScarecrowFlag, 1);
		break;

	case 0x8fc8:
		displayMessage(dsAddr_comeHereMsg); // "Come here, I've got something for you"
		waitLanAnimationFrame(2, 4);
		playSound(5, 3);
		playActorAnimation(627, true);
		playAnimation(629, 1, true);
		waitAnimation();
		playSound(41, 10);
		playSound(41, 47);
		playSound(55, 52);
		if (CHECK_FLAG(dsAddr_HankerchiefInMouseholeFlag, 1)) {
			setLan(2, 0);
			playActorAnimation(628, true);
			playAnimation(634, 1, true);
			waitAnimation();
			disableObject(4);
			displayMessage(dsAddr_trappedMouseMsg); // "The mouse is trapped!"
			SET_FLAG(dsAddr_mouseHoleState, 1);
		} else {
			playActorAnimation(628, true);
			playAnimation(630, 1, true);
			waitAnimation();
			displayMessage(dsAddr_cantCatchMsg); // "I can't catch it!"
		}
		break;

	case 0x9054: // mouse hole
		if (CHECK_FLAG(dsAddr_mouseGotGoldNuggetFlag, 1)) {
			displayMessage(dsAddr_nonsenseMsg); // "Nonsense"
		} else {
			playSound(5, 11);
			playSound(49, 21);
			playActorAnimation(632);
			setOns(5, 40);
			moveTo(239, 139, 0, true);
			playActorAnimation(633);
			SET_FLAG(dsAddr_HankerchiefInMouseholeFlag, 1);
			inventory->remove(kInvItemHandkerchief);
			if (!CHECK_FLAG(dsAddr_mouseNerveMsgSaidFlag, 1)) {
				SET_FLAG(dsAddr_mouseNerveMsgSaidFlag, 1);
				displayMessage(dsAddr_mouseNerveMsg); // "Boy, this mouse has some nerve!"
			}
		}
		break;

	case 0x933d:
		if (fnIsCookGone()) {
			if (CHECK_FLAG(dsAddr_MansionRadioBrokenFlag, 1))
				displayMessage(dsAddr_breakFlattenMsg); // "I wanted to break it, not to flatten it!"
			else {
				setOns(1, 0);
				playSound(5, 3);
				playSound(5, 33);
				playSound(24, 13);
				playSound(24, 19);
				playSound(24, 23);
				playSound(24, 26);
				playSound(24, 29);
				playSound(23, 21);
				playSound(74, 25);
				playActorAnimation(716);
				setOns(1, 66);
				SET_FLAG(dsAddr_MansionRadioBrokenFlag, 1);
			}
		}
		break;

	case 0x93af: // sheet + hot plate
		if (fnIsCookGone()) {
			playSound(5, 3);
			playSound(86, 11);
			playActorAnimation(720);
			inventory->add(kInvItemBurningPaper);
			inventory->remove(kInvItemSheetOfPaper);
		}
		break;

	case 0x93d5: // burning sheet + plate
		setOns(4, 0);
		playSound(87, 7);
		playActorAnimation(722);
		playSound(5, 3);
		playSound(88, 12);
		playSound(87, 24);
		playActorAnimation(723);
		displayMessage(dsAddr_burnBabyMsg); // "Burn, baby, burn!"
		wait(100);
		playSound(89, 4);
		playActorAnimation(724);
		setOns(4, 68);
		displayMessage(dsAddr_voilaMsg); // "Voila"
		inventory->remove(kInvItemBurningPaper);
		SET_FLAG(dsAddr_MansionPutBurningPaperInFridgeFlag, 1);
		break;

	case csAddr_openFullToolbox: // Right click to open toolbox
		inventory->remove(kInvItemToolboxFull);
		inventory->add(kInvItemToolboxHalfEmpty);
		inventory->add(kInvItemCarJack);
		inventory->activate(false);
		inventory->resetSelectedObject();
		displayMessage(dsAddr_carJackMsg); // "Wow! There's a car jack inside! Great!"
		break;

	case csAddr_openHalfEmptyToolbox:
		inventory->remove(kInvItemToolboxHalfEmpty);
		inventory->add(kInvItemSpanner);
		inventory->activate(false);
		inventory->resetSelectedObject();
		displayMessage(dsAddr_spannerMsg); // "There's something else inside the toolbox! A spanner!"
		break;

	case 0x671d: // very last part of the game
		moveTo(153, 163, 4);
		playActorAnimation(973);
		if (CHECK_FLAG(dsAddr_drawerPuzzleBookValue, 0)) {
			SET_FLAG(dsAddr_drawerPuzzleBookValue, _rnd.getRandomNumber(5) + 1);
		}
		loadScene(30, 18, 159, 2);
		break;

	case 0x67a6:
		loadScene(29, 149, 163, 1);
		playActorAnimation(974);
		moveTo(160, 188, 0);
		break;

	case 0x6805:
		fnEgoBottomRightTurn();
		playSound(32, 12);
		playActorAnimation(694);
		playSound(15, 8);
		playAnimation(693, 0);
		setOns(6, 0);
		displayMessage(dsAddr_fullAutomaticMsg); // "Fully Automatic"
		inventory->add(kInvItemVideoTape);
		disableObject(4);
		break;

	case csAddr_egoBottomRightTurn:
		fnEgoBottomRightTurn();
		break;

	case 0x687a: // using the book
		if (CHECK_FLAG(dsAddr_drawerPuzzleSolvedFlag, 1)) {
			displayMessage(dsAddr_dontMessMsg); // "I don't need to mess with it anymore"
		} else {
			playSound(49, 5);
			playSound(49, 17);
			playActorAnimation(691);
			if (!fnCheckingDrawers()) {
				if (!CHECK_FLAG(dsAddr_drawerPuzzleBookMessageFlag, 1)) {
					displayMessage(dsAddr_bookHeldMsg); // "Something's got hold of the book!"
					SET_FLAG(dsAddr_drawerPuzzleBookMessageFlag, 1);
				}
			} else {
				playSound(15, 8); // secret compartment
				playAnimation(692, 0);
				setOns(6, 59);
				enableObject(4);
				displayMessage(dsAddr_secretCompartmentMsg); // "Wow! A secret compartment!"
				SET_FLAG(dsAddr_drawerPuzzleSolvedFlag, 1);
			}
		}
		break;

	case csAddr_checkingDrawers:
		fnCheckingDrawers();
		break;

	case 0x6918:
		if (inventory->has(kInvItemSheetOfPaper))
			displayMessage(dsAddr_noMoreSheetsMsg); // "Right now I don't need any more sheets"
		else {
			if (!CHECK_FLAG(dsAddr_mansionTrashcanSearchedFlag, 1)) {
				playActorAnimation(695);
				dialog->showMark(91, scene);
				SET_FLAG(dsAddr_mansionTrashcanSearchedFlag, 1);
			}

			playSound(5, 11);
			playActorAnimation(696);
			inventory->add(kInvItemSheetOfPaper);
		}
		break;

	case 0x6962:
		if (CHECK_FLAG(dsAddr_blueDrawerOpenFlag, 1)) {
			setOns(0, 0);
			playSound(67, 4);
			playActorAnimation(678);
			SET_FLAG(dsAddr_blueDrawerOpenFlag, 0);
		} else if (CHECK_FLAG(dsAddr_redDrawerOpenFlag, 1)) {
			fnDrawerOpenMessage();
		} else {
			playSound(66, 4);
			playActorAnimation(677);
			setOns(0, 53);
			SET_FLAG(dsAddr_blueDrawerOpenFlag, 1);
		}
		break;

	case 0x69b8:
		if (CHECK_FLAG(dsAddr_redDrawerOpenFlag, 1)) {
			setOns(1, 0);
			playSound(67, 4);
			playActorAnimation(680);
			SET_FLAG(dsAddr_redDrawerOpenFlag, 0);
		} else if (CHECK_FLAG(dsAddr_blueDrawerOpenFlag, 1)) {
			fnDrawerOpenMessage();
		} else if (CHECK_FLAG(dsAddr_greyDrawerOpenFlag, 1)) {
			fnDrawerOpenMessage();
		} else {
			playSound(66, 5);
			playActorAnimation(679);
			setOns(1, 54);
			SET_FLAG(dsAddr_redDrawerOpenFlag, 1);
		}
		break;

	case 0x6a1b:
		if (CHECK_FLAG(dsAddr_greyDrawerOpenFlag, 1)) {
			setOns(2, 0);
			playSound(67, 5);
			playActorAnimation(682);
			SET_FLAG(dsAddr_greyDrawerOpenFlag, 0);
		} else if (CHECK_FLAG(dsAddr_redDrawerOpenFlag, 1)) {
			fnDrawerOpenMessage();
		} else {
			playSound(67, 5);
			playActorAnimation(681);
			setOns(2, 55);
			SET_FLAG(dsAddr_greyDrawerOpenFlag, 1);
		}
		break;

	case 0x6a73:
		if (CHECK_FLAG(dsAddr_greenDrawerOpenFlag, 1)) {
			setOns(3, 0);
			playSound(67, 4);
			playActorAnimation(684);
			SET_FLAG(dsAddr_greenDrawerOpenFlag, 0);
		} else if (!CHECK_FLAG(dsAddr_brownDrawerOpenFlag, 1)) {
			playSound(66, 4);
			playActorAnimation(683);
			setOns(3, 56);
			SET_FLAG(dsAddr_greenDrawerOpenFlag, 1);
		} else
			fnDrawerOpenMessage();
		break;

	case 0x6acb:
		if (CHECK_FLAG(dsAddr_brownDrawerOpenFlag, 1)) {
			setOns(4, 0);
			playSound(67, 4);
			playActorAnimation(686);
			SET_FLAG(dsAddr_brownDrawerOpenFlag, 0);
		} else if (CHECK_FLAG(dsAddr_greenDrawerOpenFlag, 1)) {
			fnDrawerOpenMessage();
		} else if (CHECK_FLAG(dsAddr_pinkDrawerOpenFlag, 1)) {
			fnDrawerOpenMessage();
		} else {
			playSound(66, 5);
			playActorAnimation(685);
			setOns(4, 57);
			SET_FLAG(dsAddr_brownDrawerOpenFlag, 1);
		}
		break;

	case 0x6b2e:
		if (CHECK_FLAG(dsAddr_pinkDrawerOpenFlag, 1)) {
			setOns(5, 0);
			playSound(67, 5);
			playActorAnimation(688);
			SET_FLAG(dsAddr_pinkDrawerOpenFlag, 0);
		} else if (CHECK_FLAG(dsAddr_brownDrawerOpenFlag, 1)) {
			fnDrawerOpenMessage();
		} else {
			playSound(66, 6);
			playActorAnimation(687);
			setOns(5, 58);
			SET_FLAG(dsAddr_pinkDrawerOpenFlag, 1);
		}
		break;

	case csAddr_DrawerOpenMessage:
		fnDrawerOpenMessage();
		break;

	case 0x6be1: // handle to the bathroom
		if (CHECK_FLAG(dsAddr_MansionJohnNotyEscapingFlag, 1))
			displayMessage(dsAddr_catchJohnFirstMsg); // "I'd better catch John Noty first"
		else {
			playSound(88, 4);
			playActorAnimation(808);
			loadScene(36, 41, 195, 2);
		}
		break;

	case 0x6bad:
		playSound(80, 4);
		playActorAnimation(971);
		loadScene(32, 139, 199, 1);
		break;

	case 0x6c45:
		playSound(89, 6);
		playActorAnimation(CHECK_FLAG(dsAddr_mansionHandleInDoorHoleFlag, 1) ? 985 : 806);
		loadScene(34, 40, 133, 2);
		break;

	case 0x6c83:
		waitLanAnimationFrame(1, 1);
		dialog->pop(scene, dsAddr_dialogStackRobotSafe, 0, 727, textColorMark, textColorMike, 0, 1);
		scene->getObject(1)->setName((const char *)res->dseg.ptr(dsAddr_scnObjNameMike));
		SET_FLAG(dsAddr_MansionRobotSafeUnlockedFlag, 1);
		break;

	case 0x6c9d: // getting jar
		setOns(0, 71);
		playSound(32, 5);
		playActorAnimation(732);
		disableObject(2);
		inventory->add(kInvItemTimePills);
		break;

	case 0x6cc4: // secret diary
		playActorAnimation(754);
		hideActor();

		displayCutsceneMessage(dsAddr_cutsceneMsg0, 30, 95); // "A secret diary of ..."

		playMusic(3);
		loadScene(11, scene->getPosition());

		playAnimation(750, 2);
		dialog->show(117, scene, 751, 529, textColorProfessor, textColorMansionGuard, 2, 1);

		playAnimation(752, 0, true);
		playAnimation(753, 1, true);
		waitAnimation();
		dialog->show(118, scene, 529, 751, textColorMansionGuard, textColorProfessor, 1, 2);

		loadScene(30, scene->getPosition());
		dialog->show(108, scene, 733, 734, textColorProfessor, textColorJohnNoty, 2, 3);

		playSound(75, 13);
		playSound(32, 22);
		playAnimation(735, 1, true);
		playAnimation(736, 2, true);
		waitAnimation();
		dialog->show(109, scene, 737, 738, textColorJohnNoty, textColorProfessor, 3, 2);

		playSound(32, 1);
		playAnimation(739, 1, true);
		playAnimation(740, 2, true);
		waitAnimation();
		dialog->show(110, scene, 733, 734, textColorProfessor, textColorJohnNoty, 2, 3);

		playAnimation(742, 1, true);
		playAnimation(741, 2, true);
		waitAnimation();
		dialog->show(111, scene, 743, 733, textColorJohnNoty, textColorProfessor, 3, 2);

		playAnimation(744, 1, true);
		playAnimation(745, 2, true);
		waitAnimation();
		dialog->show(112, scene, 734, 733, textColorJohnNoty, textColorProfessor, 3, 2);

		playAnimation(746, 1, true);
		playAnimation(747, 2, true);
		waitAnimation();

		dialog->show(113, scene, 734, 734, textColorJohnNoty, textColorJohnNoty, 3, 3);
		dialog->show(114, scene, 748, 748, textColorJohnNoty, textColorJohnNoty, 3, 3);
		dialog->show(115, scene, 749, 749, textColorJohnNoty, textColorJohnNoty, 3, 3);
		dialog->show(116, scene, 748, 748, textColorJohnNoty, textColorJohnNoty, 3, 3);

		playMusic(10);
		loadScene(32, scene->getPosition());
		showActor();
		playSound(26, 10);
		playActorAnimation(755);
		moveRel(0, 0, 3);

		dialog->showMark(119, scene);

		hideActor();
		loadScene(31, scene->getPosition());
		dialog->show(123, scene, 763, 764, textColorMansionGuard, textColorJohnNoty, 1, 2);

		loadScene(32, scene->getPosition());
		showActor();
		dialog->showMark(120, scene);
		disableObject(3);
		enableObject(7);

		SET_FLAG(dsAddr_MansionJohnNotyOutsideBathroomFlag, 1);
		break;

	case 0x6f20:
		if (CHECK_FLAG(dsAddr_MansionJohnNotyOutsideBathroomFlag, 1))
			displayMessage(dsAddr_cantHideMsg); // "I can't hide here!"
		else
			rejectMessage();
		break;

	case 0x6f75: // hiding in left corner
		moveRel(0, 0, 3);
		playActorAnimation(756);
		hideActor();
		playAnimation(758, 1);
		dialog->show(121, scene, 759, 759, textColorJohnNoty, textColorJohnNoty, 2, 2);

		playSound(40, 5);
		playSound(52, 13);
		playSound(52, 17);
		playSound(52, 21);
		playAnimation(760, 1);
		setOns(1, 72);
		setOns(2, 73);
		loadScene(31, scene->getPosition());
		playSound(58, 5);
		playSound(58, 8);
		playSound(58, 10);
		playSound(58, 12);
		playSound(58, 14);
		playAnimation(765, 1);
		dialog->show(124, scene, 766, 766, textColorMansionGuard, textColorMansionGuard, 1, 1);
		loadScene(32, scene->getPosition());
		dialog->show(122, scene, 761, 761, textColorJohnNoty, textColorJohnNoty, 2, 2);
		playAnimation(762, 1);
		setOns(2, 0);
		showActor();
		playActorAnimation(757);
		moveRel(0, 0, 1);
		displayMessage(dsAddr_wasCloseMsg); // "That was close"
		enableObject(8);
		disableObject(7);

		SET_FLAG(dsAddr_MansionJohnNotyOutsideBathroomFlag, 0);
		break;

	case 0x6f4d:
		if (CHECK_FLAG(dsAddr_MansionJohnNotyOutsideBathroomFlag, 1))
			displayMessage(dsAddr_johnOutsideMsg); // "There's John Noty outside! I can't go out!"
		else
			loadScene(31, 139, 172, 3);
		break;

	case 0x6f32:
		if (CHECK_FLAG(dsAddr_MansionJohnNotyOutsideBathroomFlag, 1)) {
			displayMessage(dsAddr_cantHideMsg); // "I can't hide here!"
		} else {
			playActorAnimation(977);
			displayMessage(dsAddr_lockedMsg); // "It's Locked!"
		}
		break;

	case 0x7096:
		playSound(32, 5);
		playActorAnimation(767);
		setOns(1, 0);
		inventory->add(kInvItemHandle);
		disableObject(8);
		break;

	case 0x7218:
		rejectMessage();
		break;

	case 0x7291:
		playSound(89, 3);
		playActorAnimation(975);
		loadScene(31, 298, 177, 4);
		break;

	case 0x72c2:
		if (CHECK_FLAG(dsAddr_MansionSinkState, 2)) {
			displayMessage(dsAddr_enoughWaterMsg); // "There's enough water in the sink"
		} else {
			playSound(79, 6);
			playSound(84, 9);
			playActorAnimation(801);
			wait(50);
			if (CHECK_FLAG(dsAddr_MansionSinkState, 1)) {
				displayMessage(dsAddr_sinkFullMsg); // "The sink is full of hot water"
				SET_FLAG(dsAddr_MansionSinkState, 2);
			} else
				displayMessage(dsAddr_waterHotMsg); // "The water looks very hot"
		}
		break;

	case 0x7309:
		playSound(66, 5);
		playSound(67, 11);
		playActorAnimation(976);
		displayMessage(dsAddr_emptyMsg); // "It's Empty"
		break;

	case 0x77d5:
		if (CHECK_FLAG(dsAddr_MansionThruFanByTimePillFlag, 1) && !CHECK_FLAG(dsAddr_MansionVentFanStoppedFlag, 1)) { // disallow exiting through the first door until switch turned on, not present in original game
			displayMessage(dsAddr_noSaladMsg); // "I don't want to turn myself into a salad"
		} else {
			playSound(89, 6);
			playActorAnimation(978);
			loadScene(31, 298, 177, 4);
		}
		break;

	case 0x79e4:
		fnEgoBottomRightTurn();
		retVal = false;
		break;

	case 0x79eb: // color of the book
		// FIXME - Replace with internal lookup and switch
		displayMessage(res->dseg.get_word(dsAddr_bookColorMsgPtr + GET_FLAG(dsAddr_drawerPuzzleBookValue) * 2 - 2));
		break;

	case 0x79fd:
		if (CHECK_FLAG(dsAddr_blueDrawerOpenFlag, 1))
			displayMessage(dsAddr_blueInteriorMsg); // "It's got a blue interior"
		else
			retVal = false;
		break;

	case 0x7a0f:
		if (CHECK_FLAG(dsAddr_redDrawerOpenFlag, 1)) {
			if (!CHECK_FLAG(dsAddr_drawerGotPolaroidFlag, 1)) {
				displayMessage(dsAddr_foundPolaroidMsg); // "There's a polaroid inside! I might need that"
				playSound(5, 11);
				playActorAnimation(690);
				inventory->add(kInvItemPolaroidCamera);
				SET_FLAG(dsAddr_drawerGotPolaroidFlag, 1);
			}
			displayMessage(dsAddr_redInteriorMsg); // "It's got a red interior"
		} else
			retVal = false;
		break;

	case 0x7a49:
		if (CHECK_FLAG(dsAddr_greyDrawerOpenFlag, 1))
			displayMessage(dsAddr_greyInteriorMsg); // "It's got a grey interior"
		else
			retVal = false;
		break;

	case 0x7a5b:
		if (CHECK_FLAG(dsAddr_greenDrawerOpenFlag, 1))
			displayMessage(dsAddr_greenInteriorMsg); // "It's got a green interior"
		else
			retVal = false;
		break;

	case 0x7a6d:
		if (CHECK_FLAG(dsAddr_brownDrawerOpenFlag, 1))
			displayMessage(dsAddr_brownInteriorMsg); // "It's got a brown interior"
		else
			retVal = false;
		break;

	case 0x7a7f:
		if (CHECK_FLAG(dsAddr_pinkDrawerOpenFlag, 1)) {
			if (!CHECK_FLAG(dsAddr_drawerGotDictaphoneFlag, 1)) {
				displayMessage(dsAddr_dictaphoneInsideMsg); // "Wow! There's a dictaphone inside!"
				playSound(5, 12);
				playActorAnimation(689);
				inventory->add(kInvItemDictaphoneNoBatteries);
				SET_FLAG(dsAddr_drawerGotDictaphoneFlag, 1);
			}
			displayMessage(dsAddr_pinkInteriorMsg); // "It's got a pink interior"
		} else
			retVal = false;
		break;

	case 0x7af7:
		if (CHECK_FLAG(dsAddr_MansionPutBurningPaperInFridgeFlag, 1))
			displayMessage(dsAddr_yummyMsg); // "Yummy"
		else
			retVal = false;
		break;

	case 0x7b09:
		{
			byte v = GET_FLAG(dsAddr_MansionSinkState);
			switch (v) {
			case 1:
				displayMessage(dsAddr_corkInHoleMsg); // "The cork is stuck in the hole"
				break;
			case 2:
				displayMessage(dsAddr_sinkFullMsg); // "The sink is full of hot water"
				break;
			default:
				retVal = false;
				break;
			}
		}
		break;

	case csAddr_robotSafeAlreadyUnlockedCheck:
		fnRobotSafeAlreadyUnlockedCheck();
		break;

	case csAddr_robotSafeUnlockCheck:
		fnRobotSafeUnlockCheck();
		break;

	case 0x90bc: // handle on the hole
		playSound(5, 3);
		playSound(6, 9);
		playActorAnimation(807);
		setOns(0, 83);
		inventory->remove(kInvItemHandle);
		disableObject(2);
		enableObject(3);
		SET_FLAG(dsAddr_mansionHandleInDoorHoleFlag, 1);
		break;

	case 0x90fc: // dictaphone on robot
		if (fnRobotSafeAlreadyUnlockedCheck()) {
			if (CHECK_FLAG(dsAddr_MansionRobotSafeVoiceTestPassedFlag, 1)) {
				displayMessage(dsAddr_fooledOnceMsg); // "I'd already fooled him once"
			} else {
				if (!CHECK_FLAG(dsAddr_usedDictaphoneOnTVFlag, 1)) {
					displayMessage(dsAddr_notMyVoiceMsg); // "I won't cheat Mike with MY voice"
				} else {
					displayMessage(dsAddr_mikeVoiceTestMsg); // "Mike, activate the voice test"
					waitLanAnimationFrame(1, 1);

					playSound(5, 3);
					playSound(5, 39);
					displayAsyncMessage(dsAddr_singingMsg, 68, 126, 9, 35, textColorJohnNoty); // "siiiiinging!"
					playActorAnimation(728);

					waitLanAnimationFrame(1, 1);
					dialog->show(98, scene, 0, 727, textColorMark, textColorMike, 0, 1);
					SET_FLAG(dsAddr_MansionRobotSafeVoiceTestPassedFlag, 1);
					fnRobotSafeUnlockCheck();
				}
			}
		}
		break;

	case 0x91cb: // use socks on robot
		if (fnRobotSafeAlreadyUnlockedCheck()) {
			if (CHECK_FLAG(dsAddr_MansionRobotSafeScentTestPassedFlag, 1)) {
				displayMessage(dsAddr_fooledOnceMsg); // "I'd already fooled him once"
			} else {
				displayMessage(dsAddr_mikeScentTestMsg); // "Mike, let's get on with the scent test"

				waitLanAnimationFrame(1, 1);
				playSound(5, 3);
				playSound(5, 23);
				playActorAnimation(729);

				waitLanAnimationFrame(1, 1);
				dialog->show(99, scene, 0, 727, textColorMark, textColorMike, 0, 1);
				SET_FLAG(dsAddr_MansionRobotSafeScentTestPassedFlag, 1);
				fnRobotSafeUnlockCheck();
			}
		}
		break;

	case 0x9209: // photo on robot
		if (fnRobotSafeAlreadyUnlockedCheck()) {
			if (CHECK_FLAG(dsAddr_MansionRobotSafeViewTestPassedFlag, 1)) {
				displayMessage(dsAddr_fooledOnceMsg); // "I'd already fooled him once"
			} else {
				displayMessage(dsAddr_mikeViewTestMsg); // "Mike, run the view test"
				waitLanAnimationFrame(1, 1);

				playSound(5, 3);
				playSound(5, 25);
				playActorAnimation(730);

				waitLanAnimationFrame(1, 1);
				dialog->show(100, scene, 0, 727, textColorMark, textColorMike, 0, 1);
				SET_FLAG(dsAddr_MansionRobotSafeViewTestPassedFlag, 1);
				fnRobotSafeUnlockCheck();
			}
		}
		break;

	case 0x9247:
		displayMessage(dsAddr_sameBottleMsg); // "The bottle's the same, but I doubt if it's enough to fool anyone"
		break;

	case 0x924e:
		setOns(2, 64);
		playSound(5, 3);
		playSound(52, 10);
		playActorAnimation(711);
		moveRel(0, 0, 4);
		dialog->show(95, scene, 0, 709, textColorMark, textColorCook, 0, 1);
		moveTo(300, 190, 4);
		inventory->remove(kInvItemFakeChilli);
		disableObject(8);
		playAnimation(712, 0);
		setOns(2, 0);
		playSound(15, 26);
		playSound(15, 28);
		playSound(16, 37);
		playAnimation(713, 0);
		dialog->show(96, scene, 0, 709, textColorMark, textColorCook, 0, 1);
		playSound(85, 2);
		playAnimation(714, 0);
		setLan(1, 0);
		disableObject(1);
		{
			Object *obj = scene->getObject(2);
			obj->actorRect.left = obj->actorRect.right = 81;
			obj->actorRect.top = obj->actorRect.bottom = 160;
			obj->actorOrientation = 4;
			obj->save();
		}
		{
			Object *obj = scene->getObject(3);
			obj->actorRect.left = obj->actorRect.right = 64;
			obj->actorRect.top = obj->actorRect.bottom = 168;
			obj->actorOrientation = 4;
			obj->save();
		}
		{
			Object *obj = scene->getObject(10);
			obj->actorRect.left = obj->actorRect.right = 105;
			obj->actorRect.top = obj->actorRect.bottom = 160;
			obj->actorOrientation = 1;
			obj->save();
		}
		SET_FLAG(dsAddr_MansionCookGoneFlag, 1);
		break;

	case 0x9472:
		playSound(5, 4);
		playSound(19, 14);
		playActorAnimation(793);
		displayMessage(dsAddr_fitsPerfectMsg); // "It fits perfectly!"
		inventory->remove(kInvItemWrappedCork);
		SET_FLAG(dsAddr_MansionSinkState, 1);
		break;

	case 0x9449: // meat + stew
		playSound(5, 4);
		playSound(63, 12);
		playActorAnimation(726);
		displayMessage(dsAddr_dislikeVealMsg); // "I never liked veal anyway"
		inventory->remove(kInvItemMeat);
		inventory->add(kInvItemPlasticBag);
		break;

	case 0x949b:
		if (CHECK_FLAG(dsAddr_MansionSinkState, 2)) {
			playSound(5, 4);
			playSound(5, 25);
			playActorAnimation(802);
			displayMessage(dsAddr_labelOffMsg); // "The label has come off!"
			inventory->remove(kInvItemChilliWithLabel);
			inventory->add(kInvItemChilliNoLabel);
			inventory->add(kInvItemLabel);
		} else
			displayMessage(dsAddr_noHotWaterMsg); // "There's no hot water in the sink"
		break;

	case 0x94d4:
		if (inventory->has(kInvItemPlasticBag)) {
			setOns(0, 0);
			playSound(5, 3);
			playSound(5, 18);
			playSound(13, 12);
			playActorAnimation(803);
			disableObject(7);
			inventory->remove(kInvItemPlasticBag);
			inventory->add(kInvItemSocks);
		} else
			displayMessage(dsAddr_noSockStoreMsg); // "I don't have anything to store these socks in"
		break;

	case 0x951b:
		playSound(5, 4);
		playSound(5, 22);
		playActorAnimation(804);
		displayMessage(dsAddr_corkTooSmallMsg); // "The cork is a bit too small"
		break;

	case 0x73a3:
		if (CHECK_FLAG(dsAddr_mansionTVOnFlag, 1)) {
			SET_FLAG(dsAddr_mansionTVOnFlag, 0);

			//call 73e6
			playSound(71, 3);
			playActorAnimation(700);
			playAnimation(0, 0, true);
			reloadLan();

			if (CHECK_FLAG(dsAddr_mansionVCRPlayingTapeFlag, 1)) {
				displayMessage(dsAddr_muchBetterMsg); // "That's much better"
			}
		} else {
			SET_FLAG(dsAddr_mansionTVOnFlag, 1);

			//call 73e6
			playSound(71, 3);
			playActorAnimation(700);
			reloadLan();
		}
		break;

	case 0x9537: // using remote on VCR
		playSound(5, 3);
		playSound(5, 16);
		playActorAnimation(703);
		if (!CHECK_FLAG(dsAddr_mansionVCRTapeLoadedFlag, 1))
			displayMessage(dsAddr_NotHappenMsg); // "Nothing happened"
		else {
			//0x955a
			if (CHECK_FLAG(dsAddr_mansionVCRPlayingTapeFlag, 0)) {
				if (CHECK_FLAG(dsAddr_mansionTVOnFlag, 1)) {
					if (!CHECK_FLAG(dsAddr_mansionVCRPlayedTapeBeforeFlag, 1))
						displayMessage(dsAddr_tapeStartedMsg); // "The tape started!"

					SET_FLAG(dsAddr_mansionVCRPlayingTapeFlag, 1);
					reloadLan();
					if (!CHECK_FLAG(dsAddr_mansionVCRPlayedTapeBeforeFlag, 1)) {
						dialog->show(93, scene, 0, 702, textColorMark, textColorJohnNoty, 0, 1);
						SET_FLAG(dsAddr_mansionVCRPlayedTapeBeforeFlag, 1);
					}
				} else
					displayMessage(dsAddr_tvOffMsg); // "I just realised that the TV is off"
			} else {
				SET_FLAG(dsAddr_mansionVCRPlayingTapeFlag, 0);
				if (CHECK_FLAG(dsAddr_mansionTVOnFlag, 1)) {
					reloadLan();
					displayMessage(dsAddr_muchBetterMsg); // "That's much better"
				}
			}
		}
		break;

	case 0x95eb: // polaroid + tv
		if (CHECK_FLAG(dsAddr_mansionVCRPlayingTapeFlag, 1)) {
			if (CHECK_FLAG(dsAddr_usedPolaroidOnTVFlag, 1)) {
				displayMessage(dsAddr_enoughPhotosMsg); // "I don't need any more photos"
			} else {
				playSound(5, 3);
				playSound(5, 24);
				playSound(90, 18);
				playActorAnimation(707);
				inventory->add(kInvItemPhoto);
				SET_FLAG(dsAddr_usedPolaroidOnTVFlag, 1);
			}
		} else
			displayMessage(dsAddr_notRightMomentMsg); // "I don't think this is the right moment"
		break;

	case 0x962f: // dictaphone + tv
		if (CHECK_FLAG(dsAddr_mansionVCRPlayingTapeFlag, 1)) {
			if (CHECK_FLAG(dsAddr_usedDictaphoneOnTVFlag, 1)) {
				displayMessage(dsAddr_alreadyRecordedMsg); // "I already recorded what I wanted to"
			} else {
				displayMessage(dsAddr_recordScareMsg); // "Yeah, I can record this and scare the cats"
				playSound(5, 3);
				playSound(5, 27);
				playActorAnimation(708);
				SET_FLAG(dsAddr_usedDictaphoneOnTVFlag, 1);
			}
		} else
			displayMessage(dsAddr_notRightMomentMsg); // "I don't think this is the right moment"
		break;

	case 0x95c8:
		playSound(5, 3);
		playSound(91, 12);
		playActorAnimation(706);
		inventory->remove(kInvItemVideoTape);
		SET_FLAG(dsAddr_mansionVCRTapeLoadedFlag, 1);
		break;

	case 0x966c:
		displayMessage(dsAddr_cantRecordNoBatteriesMsg); // "I can't record anything until I find some batteries"
		break;

	case 0x9673: // hit fatso - final scene
		playSound(5, 3);
		playSound(24, 10);
		playActorAnimation(798);
		playSound(63, 11);
		playSound(19, 20);
		playAnimation(799, 0);
		moveTo(50, 170, 1);
		playAnimation(800, 0, true, true, true);
		wait(100);
		playActorAnimation(805);
		moveTo(50, 170, 3);
		displayMessage(dsAddr_onlyChilliMsg); // "Good this red stuff is only a chilli"
		//moveTo(105, 157, 0, true);
		playMusic(3);
		loadScene(11, 105, 157, 4);

		dialog->show(203, scene, 0, 938, textColorMark, textColorCaptain, 0, 1);

		playAnimation(939, 0, true, true);
		playActorAnimation(942, true);
		waitAnimation();

		playAnimation(939, 0, true, true);
		playAnimation(935, 1, true, true);
		playActorAnimation(943, true);
		waitAnimation();

		playAnimation(940, 0, true, true);
		playAnimation(936, 1, true, true);
		playActorAnimation(944, true);
		waitAnimation();

		playAnimation(941, 0, true, true);
		playAnimation(937, 1, true, true);
		playActorAnimation(945, true);
		waitAnimation();

		dialog->show(204, scene, 0, 938, textColorMark, textColorCaptain, 0, 1);
		playAnimation(946, 0);
		dialog->show(205, scene, 0, 938, textColorMark, textColorCaptain, 0, 1);

		playSound(24, 7);
		playAnimation(948, 0, true);
		playActorAnimation(947, true);
		waitAnimation();

		loadScene(40, 198, 186, 1);
		dialog->show(206, scene, 0, 920, textColorMark, textColorRGBBoss, 0, 1);
		dialog->show(207, scene, 0, 921, textColorMark, textColorRGBBoss, 0, 1);
		playAnimation(923, 0);
		dialog->show(208, scene, 0, 920, textColorMark, textColorRGBBoss, 0, 1);

		moveTo(237, 186, 0);
		moveTo(237, 177, 0);
		moveTo(192, 177, 4);
		playAnimation(949, 0);
		dialog->showMono(209, scene, 950, textColorRGBBoss, 1);

		playSound(32, 5);
		playSound(40, 14);

		playAnimation(951, 0, true);
		playActorAnimation(952, true);
		waitAnimation();

		playMusic(11);
		displayCredits();
		loadScene(39, 192, 177, 0);
		hideActor();
		dialog->showMono(210, scene, 953, textColorMarkEnd, 1);
		playSound(5, 15);
		playAnimation(954, 0);
		dialog->showMono(211, scene, 955, textColorMarkEnd, 1);
		playMusic(2);
		displayCredits(dsAddr_finalCredits6, 4500); // 3 minutes (infinite until key pressed in original)
		scene->push(SceneEvent(SceneEvent::kQuit));
		break;

	case csAddr_useDivingEquipment: // using diving eq
		// FIXME - Some code is missing here as displayMessage(dsAddr_cantTalkUnderwaterMsg),
		//         displayMessage(dsAddr_notSwimmingThereMsg), displayMessage(dsAddr_tooLittleAirMsg)
		//         displayMessage(dsAddr_fishDontWorryMsg) are never called.
		{
			int id = scene->getId();
			if (id != 15) {
				if (id == 16)
					displayMessage(dsAddr_notHereMsg); // "Not here"
				else
					displayMessage(dsAddr_notBestPlaceMsg); // "It's not the best place for diving"
			} else {
				playSound(5, 3);
				playSound(38, 16);
				playSound(38, 22);
				playActorAnimation(614);
				playSound(5, 3);
				playSound(44, 10);
				playSound(20, 26);
				playActorAnimation(615);
				loadScene(17, 156, 180, 3);
				SET_FLAG(dsAddr_timedCallbackState, 4);
				playSound(64, 7);
				playSound(64, 21);
				playSound(64, 42);
				playSound(64, 63);
				setTimerCallback(csAddr_noAnchorTimeout, 30);
				playActorAnimation(617, false, true);
			}
		}
		break;

	case csAddr_noAnchorTimeout: // no anchor, timeout
		SET_FLAG(dsAddr_timedCallbackState, 0);
		fnGetOutOfLake();
		INC_FLAG(dsAddr_lakeDivingExitMessage);
		switch (GET_FLAG(dsAddr_lakeDivingExitMessage)) {
		case 1:
			displayMessage(dsAddr_seaweedMsg); // "This seaweed is just like the flowers I gave mum on her last birthday"
			break;
		case 2:
			displayMessage(dsAddr_fishBoatMsg); // "I wonder what fish do inside this boat at night"
			break;
		case 3:
			displayMessage(dsAddr_fishSomethingMsg); // "I think I have to fish out something down there"
			break;
		case 4:
			displayMessage(dsAddr_notRedHerringMsg); // "I hope all this fish stuff is not a red herring"
			break;
		case 5:
			displayMessage(dsAddr_seaweedMsg); // "This seaweed is just like the flowers I gave mum on her last birthday"
			break;
		default:
			displayMessage(dsAddr_niceDownMsg); // "It's nice down there"
			break;
		}
		break;

	case csAddr_gotAnchor:
		fnGotAnchor();
		break;

	case csAddr_getOutOfLake:
		fnGetOutOfLake();
		break;

	case csAddr_digMansionWall:
		if (scene->getId() == 13) {
			moveTo(172, 181, 1);
			playSound(26, 19);
			for (uint i = 0; i < 8; ++i)
				playSound(26, 30 + i * 11);
			playActorAnimation(661);
			displayCutsceneMessage(dsAddr_cutsceneMsgA, 84, 95); // "Hundred moments later"
			playSound(56, 10);
			playSound(56, 21);

			playSound(8, 48);
			for (uint i = 0; i < 7; ++i)
				playSound(26, 117 + i * 11);

			moveRel(-20, 0, 0, true);
			playActorAnimation(662, true);
			playAnimation(663, 2, true);
			waitAnimation();
			setOns(1, 49);

			displayCutsceneMessage(dsAddr_cutsceneMsgB, 53, 95); // "Another hundred moments later"
			moveTo(162, 184, 0, true);
			playSound(26, 6);
			playSound(26, 17);
			playSound(56, 10);
			playSound(56, 21);
			playSound(19, 27);
			playSound(24, 38);
			playSound(23, 44);
			playActorAnimation(664);
			playAnimation(665, 1);
			wait(100);
			displayMessage(dsAddr_foundCrudeOilMsg); // "At least I found crude oil and I'll be rich"
			wait(100);
			displayMessage(dsAddr_myLifeMsg); // "That's my life"
			inventory->remove(kInvItemShovelAct2);
			fnMansionIntrusionAttempt();
		} else
			displayMessage(dsAddr_notThinkRightPlaceMsg); // "I don't think this is the right place"
		break;

	case csAddr_tooDarkHere:
		displayMessage(dsAddr_cantDoTooDarkMsg); // "I can't do anything here, it's too dark"
		break;

	case csAddr_examineBanknote:
		displayMessage(dsAddr_bankNoteMsg); // "It's a note from some bank..."
		SET_FLAG(dsAddr_examinedBanknoteFlag, 1);
		retVal = false;
		break;

	case csAddr_useTimePills: // use pills
		if (scene->getId() != 36) {
			displayMessage(dsAddr_notTryNowMsg); // "There's no need to try them now"
		} else if (CHECK_FLAG(dsAddr_mansionAlreadyUsedTimePillsFlag, 1)) {
			displayMessage(dsAddr_nahMsg); // "Nah"
		} else {
			SET_FLAG(dsAddr_mansionAlreadyUsedTimePillsFlag, 1);
			moveTo(102, 195, 2);
			playSound(5, 3);
			playSound(75, 12);
			playActorAnimation(794);
			wait(100);
			setLan(1, 0);
			moveTo(151, 197, 2);
			playActorAnimation(795);
			moveTo(186, 198, 2, true);
			moveTo(220, 198, 4);
			{
				Walkbox *w = scene->getWalkbox(0);
				w->rect.left = 0;
				w->rect.bottom = kScreenHeight-1;
				w->save();
			}
			setLan(1, 0xff);

			dialog->showMark(130, scene);

			Object *obj = scene->getObject(1);
			obj->actorRect.left = obj->actorRect.right = 270;
			obj->actorRect.top = obj->actorRect.bottom = 193;
			obj->actorOrientation = 2;
			obj->save();

			obj = scene->getObject(3);
			obj->actorRect.left = obj->actorRect.right = 254;
			obj->actorRect.top = obj->actorRect.bottom = 193;
			obj->actorOrientation = 1;
			obj->save();

			SET_FLAG(dsAddr_MansionThruFanByTimePillFlag, 1);
		}
		break;

	case csAddr_mansionIntrusionAttempt:
		retVal = fnMansionIntrusionAttempt();
		break;

	case csAddr_secondMansionIntrusion:
		fnSecondMansionIntrusion();
		break;

	case csAddr_thirdMansionIntrusion:
		fnThirdMansionIntrusion();
		break;

	case csAddr_fourthMansionIntrusion:
		fnFourthMansionIntrusion();
		break;

	case csAddr_fifthMansionIntrusion:
		fnFifthMansionIntrusion();
		break;

	case csAddr_sixthMansionIntrusion:
		fnSixthMansionIntrusion();
		break;

	default:
		error("unknown callback 0x%04x called", addr);
		break;
	}

	return retVal;
}

} // End of namespace TeenAgent
