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

#ifndef TEENAGENT_RESOURCES_H
#define TEENAGENT_RESOURCES_H

#include "teenagent/pack.h"
#include "teenagent/segment.h"
#include "teenagent/font.h"
#include "graphics/surface.h"

struct ADGameDescription;

namespace TeenAgent {

// Code Segment Addresses (Read Only)
// Intro function : 0x024c
const uint16 csAddr_intro = 0x024c;
// Pole Climb Fail function : 0x4173
const uint16 csAddr_poleClimbFail = 0x4173;
// Move Ego (Mark) To Suspicious Position function : 0x505c
const uint16 csAddr_egoSuspiciousPosition = 0x505c;
// Guard Scare Timeout function : 0x516d
const uint16 csAddr_guardScareTimeout = 0x516d;
// Guard Drinking function : 0x5189
const uint16 csAddr_guardDrinking = 0x5189;
// Move Ego (Mark) To Default Position function : 0x557e
const uint16 csAddr_egoDefaultPosition = 0x557e;
// Cave NOP function : 0x599b
const uint16 csAddr_caveNOP = 0x599b;
// Enter Cave function : 0x5a21
const uint16 csAddr_enterCave = 0x5a21;
// Ego (Mark) Scared By Spider function : 0x60b5
const uint16 csAddr_egoScaredBySpider = 0x60b5;
// Move to Ladder and Leave Cellar function : 0x60d9
const uint16 csAddr_moveToLadderAndLeaveCellar = 0x60d9;
// Leave Cellar function : 0x612b
const uint16 csAddr_leaveCellar = 0x612b;
// Too Dark function : 0x61fe
const uint16 csAddr_TooDark = 0x61fe;
// Move Ego (Mark) To Bottom-Right or Turn function : 0x6849
const uint16 csAddr_egoBottomRightTurn = 0x6849;
// Checking Drawers function : 0x68e6
const uint16 csAddr_checkingDrawers = 0x68e6;
// Drawer Open Message function : 0x6b86
const uint16 csAddr_DrawerOpenMessage = 0x6b86;
// Is Cook Gone function : 0x70e0
const uint16 csAddr_isCookGone = 0x70e0;
// Giving Flower to Old Lady function : 0x88de
const uint16 csAddr_givingFlowerToOldLady = 0x88de;
// Give Another Flower to Old Lady function : 0x890b
const uint16 csAddr_giveAnotherFlowerToOldLady = 0x890b;
// Giving Flower to Anne function : 0x8942
const uint16 csAddr_givingFlowerToAnne = 0x8942;
// Give Another Flower to Anne function : 0x89aa
const uint16 csAddr_giveAnotherFlowerToAnne = 0x89aa;
// Putting Rock in Hole function : 0x8d57
const uint16 csAddr_putRockInHole = 0x8d57;
// Mouse Out Of Hole Timeout function : 0x8d79
const uint16 csAddr_mouseOutOfHoleTimeout = 0x8d79;
// Robot Safe (Mike) Already Unlocked Check function : 0x9166
const uint16 csAddr_robotSafeAlreadyUnlockedCheck = 0x9166;
// Robot Safe (Mike) Unlock Check function : 0x9175
const uint16 csAddr_robotSafeUnlockCheck = 0x9175;
// Open Full Toolbox function : 0x98fa
const uint16 csAddr_openFullToolbox = 0x98fa;
// Open Half Empty Toolbox function : 0x9910
const uint16 csAddr_openHalfEmptyToolbox = 0x9910;
// Use Diving Equipment function : 0x9921
const uint16 csAddr_useDivingEquipment = 0x9921;
// Successfully Got Anchor function : 0x99e0
const uint16 csAddr_gotAnchor = 0x99e0;
// No Anchor Timeout function : 0x9a1d
const uint16 csAddr_noAnchorTimeout = 0x9a1d;
// Get Out of Lake function : 0x9a7a
const uint16 csAddr_getOutOfLake = 0x9a7a;
// Dig Under Mansion Wall function : 0x9aca
const uint16 csAddr_digMansionWall = 0x9aca;
// Too Dark Here function : 0x9c66
const uint16 csAddr_tooDarkHere = 0x9c66;
// Examine Banknote function : 0x9c6d
const uint16 csAddr_examineBanknote = 0x9c6d;
// Use Time Pills function : 0x9c79
const uint16 csAddr_useTimePills = 0x9c79;
// Mansion Intrusion Attempt function : 0x9d45
const uint16 csAddr_mansionIntrusionAttempt = 0x9d45;
// Second Mansion Intrusion function : 0x9d90
const uint16 csAddr_secondMansionIntrusion = 0x9d90;
// Third Mansion Intrusion function : 0x9de5
const uint16 csAddr_thirdMansionIntrusion = 0x9de5;
// Fourth Mansion Intrusion function : 0x9e54
const uint16 csAddr_fourthMansionIntrusion = 0x9e54;
// Fifth Mansion Intrusion function : 0x9ec3
const uint16 csAddr_fifthMansionIntrusion = 0x9ec3;
// Sixth Mansion Intrusion function : 0x9f3e
const uint16 csAddr_sixthMansionIntrusion = 0x9f3e;
// Display Message function : 0xa055
const uint16 csAddr_displayMsg = 0xa055;
// Reject Message function : 0xa4d6
const uint16 csAddr_rejectMsg = 0xa4d6;

// Data Segment Addresses
// Timed Callback State Variable : 0x0000
const uint16 dsAddr_timedCallbackState = 0x0000; // 1 byte

// Cursor Graphic 8*12px : 0x00da to 0x0139 (Read Only)
const uint16 dsAddr_cursor = 0x00da;

// Reject Message Address Pointers : (4 * 2-byte) = 0x339e to 0x33a5
const uint16 dsAddr_rejectMsgPtr = 0x339e;
// Reject Message #0 : 0x33a6 to 0x33c9
const uint16 dsAddr_rejectMsg0 = 0x33a6; // "I have no idea what to do with it"
// Reject Message #1 : 0x33ca to 0x33f5
const uint16 dsAddr_rejectMsg1 = 0x33ca; // "I can't imagine what I could do with this"
// Reject Message #2 : 0x33f6 to 0x3425
const uint16 dsAddr_rejectMsg2 = 0x33f6; // "I can't figure out what I should do with this"
// Reject Message #3 : 0x3426 to 0x344f
const uint16 dsAddr_rejectMsg3 = 0x3426; // "I can't find any reason to mess with it"
// Cool Message : 0x3450 to 0x3456
const uint16 dsAddr_coolMsg = 0x3450; // "Cool."
// Object Usage Error Message : 0x3457 to 0x3467
const uint16 dsAddr_objErrorMsg = 0x3457; // "That's no good"
// Car Jack Message : 0x3468 to 0x348f
const uint16 dsAddr_carJackMsg = 0x3468; // "Wow! There's a car jack inside! Great!"
// Spanner Message : 0x3490 to 0x34c6
const uint16 dsAddr_spannerMsg = 0x3490; // "There's something else inside the toolbox! A spanner!"
// Last Chance Message : 0x34c7 to 0x34d4
const uint16 dsAddr_lastChanceMsg = 0x34c7; // "Last chance?"
// Give Up Message : 0x34d5 to 0x34e0
const uint16 dsAddr_giveUpMsg = 0x34d5; // "I give up"
// Avoid Bees Message : 0x34e1 to 0x351e
const uint16 dsAddr_avoidBeesMsg = 0x34e1; // "I'm going to stay at least five meters away from these bees!"
// Boat Empty Message : 0x351f to 0x3541
const uint16 dsAddr_boatEmptyMsg = 0x351f; // "There's nothing else in the boat"
// Too Hard Wood Message : 0x3542 to 0x3562
const uint16 dsAddr_tooHardWoodMsg = 0x3542; // "This wood is too hard to break"
// Boo Message : 0x3563 to 0x3569
const uint16 dsAddr_BooMsg = 0x3563; // "Booo!"
// Dont Push Luck Message : 0x356a to 0x358f
const uint16 dsAddr_dontPushLuckMsg = 0x356a; // "I don't think I should push my luck"
// Ordinary Haystack Message : 0x3590 to 0x35b1
const uint16 dsAddr_ordinaryHaystackMsg = 0x3590; // "Just an ordinary hay stack. Now."
// Needle Haystack Message : 0x35b2 to 0x35e7
const uint16 dsAddr_needleHaystackMsg = 0x35b2; // "And they say you can't find a needle in a haystack"
// No Potatoes Message : 0x35e8 to 0x3604
const uint16 dsAddr_noPotatoMsg = 0x35e8; // "There are no more potatoes"
// Trousers Message : 0x3605 to 0x363e
const uint16 dsAddr_trousersMsg = 0x3605; // "Good I always asked mum for trousers with BIG pockets"
// Life Is Brutal Message : 0x363f to 0x364f
const uint16 dsAddr_lifeIsBrutalMsg = 0x363f; // "Life is brutal"
// Life Brutal Message : 0x3650 to 0x3667
const uint16 dsAddr_lifeBrutalMsg = 0x3650; // "Life is really brutal"
// Tickled Message : 0x3668 to 0x367e
const uint16 dsAddr_tickledMsg = 0x3668; // "Something tickled me!"
// Its Gone Message : 0x367f to 0x3693
const uint16 dsAddr_itsGoneMsg = 0x367f; // "At least it's gone"
// Monsters Message : 0x3694 to 0x36c1
const uint16 dsAddr_monstersMsg = 0x3694; // "Who knows what monsters live in there"
// No Hands Message : 0x36c2 to 0x370e
const uint16 dsAddr_noHandsMsg = 0x36c2; // "I'd better not put my hands in there..."
// Totally Empty Message : 0x370f to 0x372d
const uint16 dsAddr_totalEmptyMsg = 0x370f; // "I can see it's totally empty"
// One Small Step Message : 0x372e to 0x3765
const uint16 dsAddr_oneSmallStepMsg = 0x372e; // "One small step for man, one big pain in the head"
// No Chance Message : 0x3766 to 0x378f
const uint16 dsAddr_noChanceMsg = 0x3766; // "I won't take my chances a second time"
// Dinosaur Bone Message : 0x3790 to 0x37b7
const uint16 dsAddr_dinoBoneMsg = 0x3790; // "I really hope this is DINOSAUR bone"
// Wall Shaken Message : 0x37b8 to 0x37e9
const uint16 dsAddr_wallShakenMsg = 0x37b8; // "Wow! This must have shaken all the nearby walls!"
// Kinda Dark Message : 0x37ea to 0x3800
const uint16 dsAddr_kindaDarkMsg = 0x37ea; // "It's kinda dark here"
// Not in Dark Message : 0x3801 to 0x3831
const uint16 dsAddr_notInDarkMsg = 0x3801; // "I'm not going to wander here in the dark again"
// Shut Valve Message : 0x3832 to 0x387b
const uint16 dsAddr_shutValveMsg = 0x3832; // "Shutting the valve shook the dirt from the wall..."
// Need Sunglasses Message : 0x387c to 0x38a6
const uint16 dsAddr_needSunglassesMsg = 0x387c; // "Sorry buddy, but I need your sunglasses"
// Not Best Place Message : 0x38a7 to 0x38cd
const uint16 dsAddr_notBestPlaceMsg = 0x38a7; // "It's not the best place for diving"
// Not Here Message : 0x38ce to 0x38da
const uint16 dsAddr_notHereMsg = 0x38ce; // "Not here"
// Can't Talk Underwater Message : 0x38db to 0x38fe
const uint16 dsAddr_cantTalkUnderwaterMsg = 0x38db; // "I really can't talk underwater!"
// Not Swimming There Message : 0x38ff to 0x3931
const uint16 dsAddr_notSwimmingThereMsg = 0x38ff; // "I don't think swimming there is worth the effort"
// Too Little Air Message : 0x3932 to 0x3988
const uint16 dsAddr_tooLittleAirMsg = 0x3932; // "If I want to get this anchor I have to swim there when I have more air in my lungs"
// Hooked Anchor Message : 0x3989 to 0x39ad
const uint16 dsAddr_hookedAnchorMsg = 0x3989; // "I was really hooked on this anchor!"
// Seaweed Message : 0x39ae to 0x39f5
const uint16 dsAddr_seaweedMsg = 0x39ae; // "This seaweed is just like the flowers I gave mum on her last birthday"
// Fish Boat Message : 0x39f6 to 0x3a27
const uint16 dsAddr_fishBoatMsg = 0x39f6; // "I wonder what fish do inside this boat at night"
// Fish Something Message : 0x3a28 to 0x3a59
const uint16 dsAddr_fishSomethingMsg = 0x3a28; // "I think I have to fish out something down there"
// Fish Don't Worry Message : 0x3a5a to 0x3a84
const uint16 dsAddr_fishDontWorryMsg = 0x3a5a; // "At least fish don't worry about the rain"
// Not Red Herring Message : 0x3a85 to 0x3ab6
const uint16 dsAddr_notRedHerringMsg = 0x3a85; // "I hope all this fish stuff is not a red herring"
// Nice Down Message : 0x3ab7 to 0x3acd
const uint16 dsAddr_niceDownMsg = 0x3ab7; // "It's nice down there"
// Hey Let Go Message : 0x3ace to 0x3ae5
const uint16 dsAddr_heyLetGoMsg = 0x3ace; // "Hey, let go, will ya?!"
// Aaahhh Message : 0x3ae6 to 0x3afc
const uint16 dsAddr_aaahhhMsg = 0x3ae6; // "Aaaaaaaaaaaaahhh!"
// Oops Message : 0x3afd to 0x3b03
const uint16 dsAddr_oopsMsg = 0x3afd; // "Oops"
// Found Food Message : 0x3b04 to 0x3b2e
const uint16 dsAddr_foundFoodMsg = 0x3b04; // "People leave food in unbelievable places"
// Come Here Message : 0x3b2f to 0x3b58
const uint16 dsAddr_comeHereMsg = 0x3b2f; // "Come here, I've got something for you"
// Cant Catch Message : 0x3b59 to 0x3b6b
const uint16 dsAddr_cantCatchMsg = 0x3b59; // "I can't catch it!"
// Trapped Mouse Message : 0x3b6c to 0x3b82
const uint16 dsAddr_trappedMouseMsg = 0x3b6c; // "The mouse is trapped!"
// Yikes Message : 0x3b83 to 0x3b8a
const uint16 dsAddr_yikesMsg = 0x3b83; // "Yikes!"
// Mouse Nerve Message : 0x3b8b to 0x3bab
const uint16 dsAddr_mouseNerveMsg = 0x3b8b; // "Boy, this mouse has some nerve!"
// Drawers Empty Message : 0x3bac to 0x3bd1
const uint16 dsAddr_drawersEmptyMsg = 0x3bac; // "There's nothing else in the drawers"
// Rid Bush Message 0x3bd2 to 0x3bf5
const uint16 dsAddr_ridBushMsg = 0x3bd2; // "I must get rid of this bush first"
// Mouse Gone Message : 0x3bf6 to 0x3c0a
const uint16 dsAddr_mouseGoneMsg = 0x3bf6; // "The mouse has gone!"
// Nonsense Message : 0x3c0b to 0x3c15
const uint16 dsAddr_nonsenseMsg = 0x3c0b; // "Nonsense"
// Good Doggy Message : 0x3c16 to 0x3c30
const uint16 dsAddr_goodDoggyMsg = 0x3c16; // "I understand. Good doggy"
// Here Boy Message : 0x3c31 to 0x3c3c
const uint16 dsAddr_hereBoyMsg = 0x3c31; // "Here, boy"
// Friends Now Message : 0x3c3d to 0x3c57
const uint16 dsAddr_friendsNowMsg = 0x3c3d; // "I hope we're friends now"
// Not Think Right Place Message : 0x3c58 to 0x3c7f
const uint16 dsAddr_notThinkRightPlaceMsg = 0x3c58; // "I don't think this is the right place"
// Cutscene Message A : 0x3c80 to 0x3c99
const uint16 dsAddr_cutsceneMsgA = 0x3c80; // "Hundred moments later"
// Cutscene Message B : 0x3c9a to 0x3cbb
const uint16 dsAddr_cutsceneMsgB = 0x3c9a; // "Another hundred moments later"
// Found Crude Oil Message : 0x3cbc to 0x3ce9
const uint16 dsAddr_foundCrudeOilMsg = 0x3cbc; // "At least I found crude oil and I'll be rich"
// My Life Message : 0x3cea to 0x3cfa
const uint16 dsAddr_myLifeMsg = 0x3cea; // "That's my life"
// Confusion Message : 0x3cfb to 0x3d00
const uint16 dsAddr_ConfusionMsg = 0x3cfb; // "!?&!"
// Grandpa Promise Message : 0x3d01 to 0x3d1f
const uint16 dsAddr_grandpaPromiseMsg = 0x3d01; // "But grandpa, you promised!"
// Oh Lets Go Message : 0x3d20 to 0x3d39
const uint16 dsAddr_ohLetsGoMsg = 0x3d20; // "Oh all right. Let's go"
// Bye Message : 0x3d3a to 0x3d3f
const uint16 dsAddr_byeMsg = 0x3d3a; // "Bye."
// No Need Message : 0x3d40 to 0x3d58
const uint16 dsAddr_noNeedMsg = 0x3d40; // "No need to do it again"
// Girl Talk Message : 0x3d59 to 0x3d85
const uint16 dsAddr_girlTalkMsg = 0x3d59; // "I really don't know how to talk to girls"
// Dont Work Purpose Message : 0x3d86 to 0x3dae
const uint16 dsAddr_dontWorkPurposeMsg = 0x3d86; // "I usually don't work without a purpose"
// Nut Real Message : 0x3daf to 0x3dc5
const uint16 dsAddr_nutRealMsg = 0x3daf; // "Only the nut is real"
// Hen Fly Message : 0x3dc6 to 0x3df3
const uint16 dsAddr_henFlyMsg = 0x3dc6; // "I wonder if hens can fly. Come here, baby"
// First Test Fail Message : 0x3df4 to 0x3e07
const uint16 dsAddr_firstTestFailMsg = 0x3df4; // "First test failed"
// Rid Frustations Message : 0x3e08 to 3e30
const uint16 dsAddr_ridFrustationsMsg = 0x3e08; // "I'd already got rid of my frustrations"
// Road Nowhere Message : 0x3e31 to 0x3e4e
const uint16 dsAddr_roadNowhereMsg = 0x3e31; // "Nah. It's a road to nowhere"
// Open Boot Message 0x3e4f to 0x3e62
const uint16 dsAddr_openBootMsg = 0x3e4f; // "It opens the boot"
// Shut Tight Message : 0x3e63 to 0x3e74
const uint16 dsAddr_shutTightMsg = 0x3e63; // "It's shut tight"
// Boot Empty Message : 0x3e75 to 0x3e97
const uint16 dsAddr_bootEmptyMsg = 0x3e75; // "There's nothing else in the boot"
// Clothes Dry Message : 0x3e98 to 0x3eb1
const uint16 dsAddr_clothesDryMsg = 0x3e98; // "The clothes are dry now."
// Crow Kill Message : 0x3eb2 to 0x3ed5
const uint16 dsAddr_crowKillMsg = 0x3eb2; // "I'm sure these crows will kill me"
// Get Rid Of Guard First Message : 0x3ed6 to 0x3f29
const uint16 dsAddr_getRidOfGuardFirstMsg = 0x3ed6; // "If I want to get inside I must get rid of this guard first..."
// Wall Too Smooth Message : 0x3f2a to 0x3f53
const uint16 dsAddr_wallTooSmoothMsg = 0x3f2a; // "The wall surface is too smooth to climb"
// Too Much Resin To Climb Message : 0x3f54 to 0x3f84
const uint16 dsAddr_tooMuchResinToClimbMsg = 0x3f54; // "I could climb it if there wasn't so much resin"
// Only Green Rect Message : 0x3f85 to 0x3feb
const uint16 dsAddr_onlyGreenRectMsg = 0x3f85; // "The only green stuff that I like is that rectangular piece of paper with..."
// Don't Wanna Touch Hedgehog Message : 0x3fec to 0x402d
const uint16 dsAddr_dontWannaTouchHedgehogMsg = 0x3fec; // "I don't wanna touch it. Its spines could hurt my delicate hands"
// Not Hungry Message : 0x402e to 0x4046
const uint16 dsAddr_notHungryMsg = 0x402e; // "Thanks, I'm not hungry"
// No Long Hands Message : 0x4047 to 0x406c
const uint16 dsAddr_noLongHandsMsg = 0x4047; // "I really don't have such long hands"
// Too Far To Swim Message : 0x406d to 0x4089
const uint16 dsAddr_tooFarToSwimMsg = 0x406d; // "It's too far to swim there"
// Echo Message : 0x408a to 0x4090
const uint16 dsAddr_echoMsg = 0x408a; // "Echo!"
// Loud Echo Message : 0x4091 to 0x4097
const uint16 dsAddr_loudEchoMsg = 0x4091; // "ECHO!"
// Who There Message : 0x4098 to 0x40a6
const uint16 dsAddr_whoThereMsg = 0x4098; // "Who's there?!"
// Loud Who There Message : 0x40a7 to 0x40b5
const uint16 dsAddr_loudWhoThereMsg = 0x40a7; // "WHO'S THERE?!"
// Dont Copy Message : 0x40b6 to 0x40cd
const uint16 dsAddr_dontCopyMsg = 0x40b6; // "DON'T COPY ME!"
// Loud Dont Copy Message : 0x40ce to 0x40e7
const uint16 dsAddr_loudDontCopyMsg = 0x40ce; // "DON'T COPY ME!!!"
// Throw Rock Message : 0x40e8 to 0x410e
const uint16 dsAddr_throwRockMsg = 0x40e8; // "OR I WILL THROW A ROCK DOWN THERE!"
// Or I Will Message : 0x410f to 0x411c
const uint16 dsAddr_orIWillMsg = 0x410f; // "OR I WILL"
// Still There Message : 0x411d to 0x4132
const uint16 dsAddr_stillThereMsg = 0x411d; // "Are you still there?"
// No Bucket Message : 0x4133 to 0x4163
const uint16 dsAddr_noBucketMsg = 0x4133; // "It's not a barrel-organ. And there's no bucket."
// Dont Need To Open Message : 0x4164 to 0x417d
const uint16 dsAddr_dontNeedToOpenMsg = 0x4164; // "I don't need to open it"
// Hmm Grass Message : 0x417e to 41b0
const uint16 dsAddr_hmmGrassMsg = 0x417e; // "Hmmm. Grass..."
// Find Nut Message : 0x41b1 to 0x41ee
const uint16 dsAddr_findNutMsg = 0x41b1; // "I won't find the nut just like that. The grass is too dense"
// Not Horny Message : 0x41ef to 0x41fe
const uint16 dsAddr_notHornyMsg = 0x41ef; // "I'm not horny"
// Can't Jump So High Message : 0x41ff to 0x423e
const uint16 dsAddr_CantJumpMsg = 0x41ff; // "No way I can jump so high, cause, err, white men can't jump"
// Don't Need It Message : 0x423f to 0x4250
const uint16 dsAddr_dontNeedItMsg = 0x423f; // "I don't need it"
// Not Santa Claus Message : 0x4251 to 0x4266
const uint16 dsAddr_notSantaClausMsg = 0x4251; // "I'm not Santa Claus"
// No Plastic Imitations Message : 0x4267 to 0x4288
const uint16 dsAddr_noPlasticImitationsMsg = 0x4267; // "I don't need plastic imitations"
// Too Fragile Message : 0x4289 to 0x42ab
const uint16 dsAddr_tooFragileMsg = 0x4289; // "It's too fragile to carry around"
// Keep It Open Message : 0x42ac to 0x42c6
const uint16 dsAddr_keepItOpenMsg = 0x42ac; // "I'd like to keep it open"
// Not Taking Socks Message : 0x42c7 to 0x4305
const uint16 dsAddr_notTakingSocksMsg = 0x42c7; // "I really don't want to walk around with someone else's socks"
// Not Tired Message : 0x4306 to 0x431d
const uint16 dsAddr_notTiredMsg = 0x4306; // "Thanks, I'm not tired"
// Too Big Message : 0x431e to 0x434d
const uint16 dsAddr_tooBigMsg = 0x431e; // "It's too big and I doubt if I'll ever need it"
// No Secret Passage Message : 0x434e to 0x437f
const uint16 dsAddr_noSecretPassageMsg = 0x434e; // "I don't think there's any secret passage inside"
// No Fruit Message : 0x4380 to 0x43ab
const uint16 dsAddr_noFruitMsg = 0x4380; // "There are no more interesting fruits here"
// Jug Me Message : 0x43ac to 0x43cd
const uint16 dsAddr_jugMeMsg = 0x43ac; // "They can jug me if I steal this"
// Leave Flowers Alone Message : 0x43ce to 0x4411
const uint16 dsAddr_leaveFlowersAloneMsg = 0x43ce; // "I'd better leave it. Women are really oversensitive about flowers."
// Mirror Mirror Message : 0x4412 to 0x444e
const uint16 dsAddr_mirrorMirrorMsg = 0x4412; // "Mirror, Mirror on the wall...."
// Think Too Long Message : 0x444f to 0x446a
const uint16 dsAddr_thinkTooLongMsg = 0x444f; // "Hey, don't think too long"
// Hint Male Message : 0x446b to 0x4491
const uint16 dsAddr_HintMaleMsg = 0x446b; // "A hint: Someone in this room, a male"
// OK Wait Message : 0x4492 to 0x44a6
const uint16 dsAddr_okWaitMsg = 0x4492; // "OK, take your time"
// Busy Thinking Message : 0x44a7 to 0x44d5
const uint16 dsAddr_busyThinkingMsg = 0x44a7; // "I'd better not interrupt it's thought process"
// No Dentists Message : 0x44d6 to 0x450d
const uint16 dsAddr_noDentistsMsg = 0x44d6; // "I don't want to have anything in common with dentists"
// Too Heavy Message : 0x450e to 0x4531
const uint16 dsAddr_tooHeavyMsg = 0x450e; // "It's too heavy. Not that I'm wimp"
// What Got Message : 0x4532 to 0x4554
const uint16 dsAddr_whatGotMsg = 0x4532; // "Let's look what we've got here"
// Strawberry Jam Message : 0x4555 to 0x4567
const uint16 dsAddr_strawberryJamMsg = 0x4555; // "Strawberry jam"
// Gooseberry Jam Message : 0x4568 to 0x457a
const uint16 dsAddr_gooseberryJamMsg = 0x4568; // "Gooseberry jam"
// Blackberry Jam Message : 0x457b to 0x458d
const uint16 dsAddr_blackberryJamMsg = 0x457b; // "Blackberry jam"
// Bilberry Jam Message : 0x458e to 0x459e
const uint16 dsAddr_bilberryJamMsg = 0x458e; // "Bilberry jam"
// Get Me Out Jam Message : 0x459f to 0x45b7
const uint16 dsAddr_getMeOutJamMsg = 0x459f; // "Get me out of this jam!"
// Rosemary Jam Message : 0x45b8 to 0x45d9
const uint16 dsAddr_rosemaryJamMsg = 0x45b8; // "Oh, and there is Rosemary jam"
// Know Rosemary Message : 0x45da to 0x4602
const uint16 dsAddr_knowRosemaryMsg = 0x45da; // "I used to know someone called Rosemary"
// Unwanted Jams Message : 0x4603 to 0x461c
const uint16 dsAddr_unwantedJamsMsg = 0x4603; // "I don't want those jams"
// Too Dark Message : 0x461d to 0x463b
const uint16 dsAddr_TooDarkMsg = 0x461d; // "It's too dark to see clearly"
// Yeow Message : 0x463c to 0x4649
const uint16 dsAddr_yeowMsg = 0x463c; // "YEEEOOOWWWW!"
// Yawn Message : 0x464a to 0x4651
const uint16 dsAddr_yawnMsg = 0x464a; // "(yawn)"
// Laughter Message : 0x4652 to 0x465d
const uint16 dsAddr_laughterMsg = 0x4652; // "(laughter)"
// No Hands Sharp Thorns Message : 0x465e to 0x46a0
const uint16 dsAddr_noHandsSharpThornsMsg = 0x465e; // "I can't remove it with my hands. these thorns look really sharp"
// No Chainsaw Fuel Message : 0x46a1 to 0x46c2
const uint16 dsAddr_noChainsawFuelMsg = 0x46a1; // "There's no fuel in the chainsaw"
// Thorns Too Thin Message : 0x46c3 to 0x46f6
const uint16 dsAddr_thornsTooThinMsg = 0x46c3; // "Thorns are too thin, the chainsaw is useless here"
// Rock Walking Gee Message : 0x46f7 to 0x473c
const uint16 dsAddr_rockWalkingGeeMsg = 0x46f7; // "Yeah, great idea. Let's take this rock and walk around a bit. Gee..."
// Butterfly Message : 0x473d to 0x477a
const uint16 dsAddr_butterflyMsg = 0x473d; // "I'd better leave them alone, they make this place beautiful"
// Not Sure If Alive Message : 0x477b to 0x4797
const uint16 dsAddr_notSureIfAliveMsg = 0x477b; // "I'm not sure if it's alive"

// FIXME - Unknown where this is used.. Talking to SOMETHING...
// Unknown Language Message : 0x4798 to 0x47be
const uint16 dsAddr_unknownLanguageMsg = 0x4798; // "I don't know what language it speaks"

// Hole Too Narrow Message : 0x47bf to 0x47e6
const uint16 dsAddr_holeTooNarrowMsg = 0x47bf; // "The hole is too narrow to fit my hand"
// Bird Attack Message : 0x47e7 to 0x4807
const uint16 dsAddr_birdAttackMsg = 0x47e7; // "Hey You! Wake up! Bird attack!"
// No Search Warrant Message : 0x4808 to 0x4827
const uint16 dsAddr_noSearchWarrantMsg = 0x4808; // "I don't have a search-warrant"
// Uninteresting Haystack Message : 0x4828 to 0x485f
const uint16 dsAddr_uninterestingHaystackMsg = 0x4828; // "I don't see anything interesting about this haystack"
// More Complicated Message : 0x4860 to 0x4881
const uint16 dsAddr_moreComplicatedMsg = 0x4860; // "It's more complicated than that"
// Nut Rake Message : 0x4882 to 0x48be
const uint16 dsAddr_nutRakeMsg = 0x4882; // "It's pointless, the nut will slip between the rake's teeth"
// Paddle Broken Message : 0x48bf to 0x48d5
const uint16 dsAddr_paddleBrokenMsg = 0x48bf; // "The paddle is BROKEN"
// Branch Not Paddle Message : 0x48d6 to 0x4912
const uint16 dsAddr_branchNotPaddleMsg = 0x48d6; // "This branch is not a paddle. It doesn't even look like one"
// Try Somewhere Else Message : 0x4913 to 0x495b
const uint16 dsAddr_trySomewhereElseMsg = 0x4913; // "I'd better try somewhere else - I suppose this side is heavily guarded"
// Sharpen Not Pulverize Message : 0x495c to 0x4983
const uint16 dsAddr_sharpenNotPulverizeMsg = 0x495c; // "I needed to sharpen it, not pulverize"
// Can't Do Anything Too Dark Message : 0x4984 to 0x49ad
const uint16 dsAddr_cantDoTooDarkMsg = 0x4984; // "I can't do anything here, it's too dark"
// Bribe Message : 0x49ae to 0x49d0
const uint16 dsAddr_BribeMsg = 0x49ae; // "Here, let's make your pocket fat."
// Bank Note Message : 0x49d1 to 0x4a28
const uint16 dsAddr_bankNoteMsg = 0x49d1; // "It's a note from some bank..."
// Show Her Money Message : 0x4a29 to 0x4a5a
const uint16 dsAddr_showHerMoneyMsg = 0x4a29; // "If I just show her the money, she might take it"
// Hundred Bucks Message : 0x4a5b to 0x4a6e
const uint16 dsAddr_hundredBucksMsg = 0x4a5b; // "A hundred bucks!!!"
// Want Blood Message : 0x4a6f to 0x4a7d
const uint16 dsAddr_wantBloodMsg = 0x4a6f; // "I want Blood!"
// Dont Leave Mansion Message : 0x4a7e to 0x4aaf
const uint16 dsAddr_dontLeaveMansionMsg = 0x4a7e; // "I don't want to leave the mansion, I want blood!"
// Wimp Message : 0x4ab0 to 0x4acc
const uint16 dsAddr_WimpMsg = 0x4ab0; // "I'm a pathetic little wimp"
// Strange Drawer Message : 0x4acd to 0x4b0c
const uint16 dsAddr_strangeDrawerMsg = 0x4acd; // "Strange, but the drawer is stuck if the next drawer is open"
// Not Ordinary Drawers Message : 0x4b0d to 0x4b38
const uint16 dsAddr_notOrdinaryDrawersMsg = 0x4b0d; // "Maybe these are not just ordinary drawers!"
// Drawer Open Message : 0x4b39 to 0x4b6b
const uint16 dsAddr_drawerOpenMsg = 0x4b39; // "I cannot open the drawer if the next one is open!"
// Blue Interior Message 0x4b6c to 0x4b86
const uint16 dsAddr_blueInteriorMsg = 0x4b6c; // "It's got a blue interior"
// Red Interior Message : 0x4b87 to 0x4ba0
const uint16 dsAddr_redInteriorMsg = 0x4b87; // "It's got a red interior"
// Grey Interior Message : 0x4ba1 to 0x4bbb
const uint16 dsAddr_greyInteriorMsg = 0x4ba1; // "It's got a grey interior"
// Green Interior Message : 0x4bbc to 0x4bd7
const uint16 dsAddr_greenInteriorMsg = 0x4bbc; // "It's got a green interior"
// Brown Interior Message : 0x4bd8 to 0x4bf3
const uint16 dsAddr_brownInteriorMsg = 0x4bd8; // "It's got a brown interior"
// Pink Interior Message : 0x4bf4 to 0x4c0e
const uint16 dsAddr_pinkInteriorMsg = 0x4bf4; // "It's got a pink interior"
// Dictaphone Inside Message : 0x4c0f to 0x4c31
const uint16 dsAddr_dictaphoneInsideMsg = 0x4c0f; // "Wow! There's a dictaphone inside!"
// Found Polaroid Message : 0x4c32 to 0x4c60
const uint16 dsAddr_foundPolaroidMsg = 0x4c32; // "There's a polaroid inside! I might need that"
// Book Held Message : 0x4c61 to 0x4c83
const uint16 dsAddr_bookHeldMsg = 0x4c61; // "Something's got hold of the book!"
// Secret Compartment Message : 0x4c84 to 0x4c9f
const uint16 dsAddr_secretCompartmentMsg = 0x4c84; // "Wow! A secret compartment!"
// Dont Mess Message : 0x4ca0 to 0x4cc6
const uint16 dsAddr_dontMessMsg = 0x4ca0; // "I don't need to mess with it anymore"
// Full Automatic Message : 0x4cc7 to 0x4cd8
const uint16 dsAddr_fullAutomaticMsg = 0x4cc7; // "Fully Automatic"
// No More Sheets Message : 0x4cd9 to 0x4d01
const uint16 dsAddr_noMoreSheetsMsg = 0x4cd9; // "Right now I don't need any more sheets"
// No Deprave Message : 0x4d02 to 0x4d29
const uint16 dsAddr_noDepraveMsg = 0x4d02; // "Nah, I don't want to deprave the kids"
// No Read Again Message : 0x4d2a to 0x4d5a
const uint16 dsAddr_noReadAgainMsg = 0x4d2a; // "I don't want to read it again. I might like it."
// TV Off Message : 0x4d5b to 0x4d7f
const uint16 dsAddr_tvOffMsg = 0x4d5b; // "I just realised that the TV is off"
// Not Happen Message : 0x4d80 to 0x4d92
const uint16 dsAddr_NotHappenMsg = 0x4d80; // "Nothing happened"
// Tape Started Message : 0x4d93 to 0x4da5
const uint16 dsAddr_tapeStartedMsg = 0x4d93; // "The tape started!"
// Much Better Message : 0x4da6 to 0x4dba
const uint16 dsAddr_muchBetterMsg = 0x4da6; // "That's much better"
// No Sleep Message : 0x4dbb to 0x4dd2
const uint16 dsAddr_noSleepMsg = 0x4dbb; // "I don't want to sleep"
// Just Cork Message : 0x4dd3 to 0x4de5
const uint16 dsAddr_justCorkMsg = 0x4dd3; // "It's just a cork"
// Enough Photos Message : 0x4de6 to 0x4e04
const uint16 dsAddr_enoughPhotosMsg = 0x4de6; // "I don't need any more photos"
// Record Scare Message : 0x4e05 to 0x4e31
const uint16 dsAddr_recordScareMsg = 0x4e05; // "Yeah, I can record this and scare the cats"
// Already Recorded Message : 0x4e32 to 0x4e57
const uint16 dsAddr_alreadyRecordedMsg = 0x4e32; // "I already recorded what I wanted to"
// Can't Record No Batteries Message : 0x4e58 to 0x4e8d
const uint16 dsAddr_cantRecordNoBatteriesMsg = 0x4e58; // "I can't record anything until I find some batteries"

// FIXME - Not sure how to get this message. Dictaphone with no batteries somewhere? Radio?
// No Batteries No Fun Message : 0x4e8e to 0x4ea4
const uint16 dsAddr_NoBatteriesNoFunMsg = 0x4e8e; // "No batteries, no fun"

// Not Right Moment Message : 0x4ea5 to 0x4ecd
const uint16 dsAddr_notRightMomentMsg = 0x4ea5; // "I don't think this is the right moment"
// Cook Around Message : 0x4ece to 0x4ef9
const uint16 dsAddr_cookAroundMsg = 0x4ece; // "I can't do anything with this cook around"
// Same Bottle Message : 0x4efa to 0x4f3c
const uint16 dsAddr_sameBottleMsg = 0x4efa; // "The bottle's the same, but I doubt if it's enough to fool anyone"
// Break Flatten Message : 0x4f3d to 0x4f68
const uint16 dsAddr_breakFlattenMsg = 0x4f3d; // "I wanted to break it, not to flatten it!"
// What Inside Message : 0x4f69 to 0x4f9a
const uint16 dsAddr_whatInsideMsg = 0x4f69; // "I was always curious what's inside these things"
// Rest Useless Message : 0x4f9b to 0x4fb0
const uint16 dsAddr_restUselessMsg = 0x4f9b; // "The rest is useless"
// Two Batteries Message : 0x4fb1 to 0x4fca
const uint16 dsAddr_twoBatteriesMsg = 0x4fb1; // "Wow! Two 1.5V batteries!"
// One Taken Message : 0x4fcb to 0x4fe1
const uint16 dsAddr_oneTakenMsg = 0x4fcb; // "This one's taken, OK?"
// Slight Mad Message : 0x4fe2 to 0x5009
const uint16 dsAddr_slightMadMsg = 0x4fe2; // "It finally happened. I'm slightly mad"
// Paper Burnt Message : 0x500a to 0x502a
const uint16 dsAddr_paperBurntMsg = 0x500a; // "The paper burnt out completely!"
// Burn Baby Message : 0x502b to 0x503d
const uint16 dsAddr_burnBabyMsg = 0x502b; // "Burn, baby, burn!"
// Voila Message : 0x503e to 0x5045
const uint16 dsAddr_voilaMsg = 0x503e; // "Voila"
// Too Hot Message : 0x5046 to 0x505d
const uint16 dsAddr_tooHotMsg = 0x5046; // "It's too hot to touch!"
// Frozen Shelf Message : 0x505e to 0x5081
const uint16 dsAddr_frozenShelfMsg = 0x505e; // "It has frozen hard onto the shelf!"
// Yummy Message : 0x5082 to 0x5089
const uint16 dsAddr_yummyMsg = 0x5082; // "Yummy"
// Dislike Veal Message : 0x508a to 0x50a5
const uint16 dsAddr_dislikeVealMsg = 0x508a; // "I never liked veal anyway"
// No Reason Message : 0x50a6 to 0x50c2
const uint16 dsAddr_noReasonMsg = 0x50a6; // "There's no reason to do it"
// Fooled Once Message : 0x50c3 to 0x50e0
const uint16 dsAddr_fooledOnceMsg = 0x50c3; // "I'd already fooled him once"
// Mike Voice Test Message : 0x50e1 to 0x5100
const uint16 dsAddr_mikeVoiceTestMsg = 0x50e1; // "Mike, activate the voice test"
// Not My Voice Message : 0x5101 to 0x5123
const uint16 dsAddr_notMyVoiceMsg = 0x5101; // "I won't cheat Mike with MY voice"
// Singing Message : 0x5124 to 0x5137
const uint16 dsAddr_singingMsg = 0x5124; // "siiiiinging!"
// Mike Scent Test Message : 0x5138 to 0x5160
const uint16 dsAddr_mikeScentTestMsg = 0x5138; // "Mike, let's get on with the scent test"
// Mike View Test Message : 0x5161 to 0x517a
const uint16 dsAddr_mikeViewTestMsg = 0x5161; // "Mike, run the view test"
// Cutscene Message #0 : 0x517b to 0x51a6
const uint16 dsAddr_cutsceneMsg0 = 0x517b; // "A secret diary of ..."
// Cant Hide Message : 0x51a7 to 0x51ba
const uint16 dsAddr_cantHideMsg = 0x51a7; // "I can't hide here!"
// John Outside Message : 0x51bb to 0x51e6
const uint16 dsAddr_johnOutsideMsg = 0x51bb; // "There's John Noty outside! I can't go out!"
// Was Close Message : 0x51e7 to 0x51f7
const uint16 dsAddr_wasCloseMsg = 0x51e7; // "That was close"
// Cork In Hole Message : 0x51f8 to 0x5217
const uint16 dsAddr_corkInHoleMsg = 0x51f8; // "The cork is stuck in the hole"
// Fits Perfect Message : 0x5218 to 0x522b
const uint16 dsAddr_fitsPerfectMsg = 0x5218; // "It fits perfectly!"
// Enough Water Message : 0x522c to 0x524e
const uint16 dsAddr_enoughWaterMsg = 0x522c; // "There's enough water in the sink"
// No Hot Water Message : 0x524f to 0x5271
const uint16 dsAddr_noHotWaterMsg = 0x524f; // "There's no hot water in the sink"
// Label Off Message : 0x5272 to 0x528a
const uint16 dsAddr_labelOffMsg = 0x5272; // "The label has come off!"
// Cork Too Small Message : 0x528b to 0x52a8
const uint16 dsAddr_corkTooSmallMsg = 0x528b; // "The cork is a bit too small"
// Not Try Now Message : 0x52a9 to 0x52ca
const uint16 dsAddr_notTryNowMsg = 0x52a9; // "There's no need to try them now"
// No Salad Message : 0x52cb to 0x52f5
const uint16 dsAddr_noSaladMsg = 0x52cb; // "I don't want to turn myself into a salad"
// Nah Message : 0x52f6 to 0x52fd
const uint16 dsAddr_nahMsg = 0x52f6; // "Nah"
// Vent First Message : 0x52fe to 0x5325
const uint16 dsAddr_ventFirstMsg = 0x52fe; // "I'd better stop this ventilator first"
// Catch John First Message : 0x5326 to 0x5348
const uint16 dsAddr_catchJohnFirstMsg = 0x5326; // "I'd better catch John Noty first"
// Only Chilli Message : 0x5349 to 0x5371
const uint16 dsAddr_onlyChilliMsg = 0x5349; // "Good this red stuff is only a chilli"
// Water Hot Message : 0x5372 to 0x538c
const uint16 dsAddr_waterHotMsg = 0x5372; // "The water looks very hot"
// Sink Full Message : 0x538d to 0x53ac
const uint16 dsAddr_sinkFullMsg = 0x538d; // "The sink is full of hot water"
// No Sock Store Message : 0x53ad to 0x53dc
const uint16 dsAddr_noSockStoreMsg = 0x53ad; // "I don't have anything to store these socks in"
// Show Papers Message : 0x53dd to 0x53f1
const uint16 dsAddr_showPapersMsg = 0x53dd; // "Here are my papers"
// Got Permission Message : 0x53f2 to 0x5410
const uint16 dsAddr_gotPermissionMsg = 0x53f2; // "I already got the permission"
// Saving Fine Message : 0x5411 to 0x5462
const uint16 dsAddr_SavingFineMsg = 0x5411; // "Saving is a very fine thing..."
// Love Captain Message : 0x5463 to 0x5474
const uint16 dsAddr_loveCaptainMsg = 0x5463; // "I love captain"
// Soccer Rulz Message : 0x5475 to 0x5483
const uint16 dsAddr_soccerRulzMsg = 0x5475; // "Soccer rulz"
// Tree Cut Message : 0x5484 to 0x54c3
const uint16 dsAddr_treeCutMsg = 0x5484; // "Don't cut the trees..."
// Visa Accepted Message : 0x54c4 to 0x54d4
const uint16 dsAddr_visaAcceptedMsg = 0x54c4; // "VISA Accepted"
// Other Graffiti Message : 0x54d5 to 0x54f6
const uint16 dsAddr_otherGraffitiMsg = 0x54d5; // "The rest of graffiti is obscene"
// First Trial Message : 0x54f7 to 0x5510
const uint16 dsAddr_firstTrialMsg = 0x54f7; // "Sir, I'm Mark. A rookie"
// Locked Message : 0x5511 to 0x551e
const uint16 dsAddr_lockedMsg = 0x5511; // "It's Locked!"
// Thanks Message : 0x551f to 0x5527
const uint16 dsAddr_ThanksMsg = 0x551f; // "Thanks."
// Unknown Usage Message : 0x5528 to 0x555c
const uint16 dsAddr_unkUsageMsg = 0x5528; // "I don't have any idea what to do with it right now"
// Idea Message : 0x555d to 0x5576
const uint16 dsAddr_ideaMsg = 0x555d; // "That gives me an idea"
// Check Works Message : 0x5577 to 0x5599
const uint16 dsAddr_checkWorksMsg = 0x5577; // "Now I got to check if it works"
// Time To Call Message : 0x559a to 0x55bf
const uint16 dsAddr_timeToCallMsg = 0x559a; // "I think it is time to call captain"
// Meal Finished Message : 0x55c0 to 0x55da
const uint16 dsAddr_mealFinishedMsg = 0x55c0; // "Hey! I finished my meal."
// Bowl Welded Message : 0x55db to 0x55fe
const uint16 dsAddr_bowlWeldedMsg = 0x55db; // "Wow. He got welded to the bowl"
// Gotcha Message : 0x55ff to 0x5607
const uint16 dsAddr_gotchaMsg = 0x55ff; // "Gotcha"
// No Pocket Message : 0x5608 to 0x5631
const uint16 dsAddr_noPocketMsg = 0x5608; // "I don't want to touch his pockets again."
// Does Not Work Message : 0x5632 to 0x5645
const uint16 dsAddr_doesNotWorkMsg = 0x5632; // "That doesn't work"
// Message : 0x5646 to 0x5655
const uint16 dsAddr_fnMsg1 = 0x5646; // "Piece of cake"
// Message : 0x5656 to 0x5679
const uint16 dsAddr_fnMsg2 = 0x5656; // "And how am I supposed to get back?"
// Message : 0x567a to 0x5681
const uint16 dsAddr_fnMsg3 = 0x567a; // "Great"
// Message : 0x5682 to 0x5695
const uint16 dsAddr_fnMsg4 = 0x5682; // "Oh, yeah, right"
// Pull Object Message #1 : 0x5696 to 0x56ab
const uint16 dsAddr_pullObjMsg1 = 0x5696; // "I can't pull it out"
// Dont Want To Touch Message : 0x56ac to 0x56d9
const uint16 dsAddr_dontWantToTouchMsg = 0x56ac; // "I don't want to touch it - I might get hurt"
// Fence Blocks Message : 0x56da to 0x56f6
const uint16 dsAddr_fenceBlocksMsg = 0x56da; // "The fence blocks the way"
// Not Want To Sleep Message : 0x56f7 to 0x570e
const uint16 dsAddr_notWantToSleepMsg = 0x56f7; // "I don't want to sleep"
// Pull Object Message #2 : 0x570f to 0x5721
const uint16 dsAddr_pullObjMsg2 = 0x570f; // "I can't reach it"
// Hello Question Message : 0x5722 to 0x5729
const uint16 dsAddr_helloQMsg = 0x5722; // "Hello?"
// Totally Addicted Message : 0x572a to 0x5741
const uint16 dsAddr_totallyAddictedMsg = 0x572a; // "He's totally addicted"
// What About Message : 0x5742 to 0x5756
const uint16 dsAddr_whatAboutMsg = 0x5742; // "What about a new"
// Hot Off Message : 0x5757 to 0x576f
const uint16 dsAddr_hotOffMsg = 0x5757; // "hot off the press"
// Full Color Message : 0x5770 to 0x5781
const uint16 dsAddr_fullColorMsg = 0x5770; // "full-color"
// Special Edition Message : 0x5782 to 0x5798
const uint16 dsAddr_specialEdMsg = 0x5782; // "special edition"
// Soldier News Message : 0x5799 to 0x57b1
const uint16 dsAddr_soldierNewsMsg = 0x5799; // "of Soldier News?!"
// Pole Climb Done Message : 0x57b2 to 0x57bf
const uint16 dsAddr_poleClimbDoneMsg = 0x57b2; // "Never Again!"
// Vac Message : 0x57c0 to 0x57de
const uint16 dsAddr_vacMsg = 0x57c0; // "What am I? A vacuum cleaner?!"
// Cutscene Message #1 : 0x57df to 0x5809
const uint16 dsAddr_cutsceneMsg1 = 0x57df; // "sixty seven rude words later"
// Cutscene Message #2 : 0x580a to 0x5826
const uint16 dsAddr_cutsceneMsg2 = 0x580a; // "Meanwhile in the mansion"
// Now Open Message : 0x5827 to 0x5836
const uint16 dsAddr_nowOpenMsg = 0x5827; // "Now it's open"
// Cmon Baby Message : 0x5837 to 0x5854
const uint16 dsAddr_cmonBabyMsg = 0x5837; // "C'mon baby, it's all yours!"
// Talk Not Now Message : 0x5855 to 0x5882
const uint16 dsAddr_talkNotNowMsg = 0x5855; // "I've got no reason to talk to him right now."
// Yeah Right Message : 0x5883 to 0x5893
const uint16 dsAddr_yeahRightMsg = 0x5883; // "Yeah right!"
// Barman Too Close Message : 0x5894 to 0x58af
const uint16 dsAddr_barmanTooCloseMsg = 0x5894; // "The barman is too close"
// Yuck Message : 0x58b0 to 0x58b6
const uint16 dsAddr_yuckMsg = 0x58b0; // "Yuck!"
// Prefer Water Message : 0x58b7 to 0x58c7
const uint16 dsAddr_preferWaterMsg = 0x58b7; // "I prefer water"
// Too Weak To Climb Message : 0x58c8 to 0x58e2
const uint16 dsAddr_tooWeakToClimbMsg = 0x58c8; // "I'm too weak to climb it"
// Spring Prick Message : 0x58e3 to 0x5904
const uint16 dsAddr_springPrickMsg = 0x58e3; // "The springs would prick my back"
// Food Alive Message : 0x5905 to 0x592e
const uint16 dsAddr_foodAliveMsg = 0x5905; // "No, thanks. This food seems still alive"
// Door Closed Message : 0x592f to 0x5954
const uint16 dsAddr_doorClosedMsg = 0x592f; // "The door is closed. What a surprise."
// Empty Message : 0x5955 to 0x5961
const uint16 dsAddr_emptyMsg = 0x5955; // "It's Empty"
// Geography Class Message : 0x5962 to 0x599c
const uint16 dsAddr_geographyClassMsg = 0x5962; // "I should have paid more attention in geography classes."
// Don't Need Mess Message : 0x599d to 0x59b5
const uint16 dsAddr_dontNeedMessMsg = 0x599d; // "I don't need this mess"
// Seen Softer Rocks Message : 0x59b6 to 0x59da
const uint16 dsAddr_seenSofterRocksMsg = 0x59b6; // "Thanks, but I've seen softer rocks"
// Too Blunt Message : 0x59db to 0x5a00
const uint16 dsAddr_tooBluntMsg = 0x59db; // "They are too blunt to be of any use"
// Useless Models Message : 0x5a01 to 0x5a1f
const uint16 dsAddr_uselessModelsMsg = 0x5a01; // "What's the use of the models?"
// Barman Will Notice Message : 0x5a20 to 0x5a50
const uint16 dsAddr_barmanWillNoticeMsg = 0x5a20; // "The barman will surely notice its disappearing"
// Too Much To Drink Message : 0x5a51 to 0x5a95
const uint16 dsAddr_tooMuchToDrinkMsg = 0x5a51; // "It'd take too much time to drink it..."
// 0x5a96 to 0x5a97 : 2 extra null bytes (padding?)
// Not Thief Message : 0x5a98 to 0x5ac5
const uint16 dsAddr_notThiefMsg = 0x5a98; // "I'm not a thief. And it's empty, by the way."
// Too Many To Search Message : 0x5ac6 to 0x5aec
const uint16 dsAddr_tooManyToSearchMsg = 0x5ac6; // "There are too many of them to search"
// Captain Would Not Fit Message : 0x5aed to 0x5b26
const uint16 dsAddr_captainWouldNotFitMsg = 0x5aed; // "Captain surely wouldn't fit them. I must look elsewhere"
// Chickening Never Message : 0x5b27 to 0x5b3e
const uint16 dsAddr_chickenNeverMsg = 0x5b27; // "Chickening? Me? Never!"
// Can't Open It Message : 0x5b3f to 0x5b50
const uint16 dsAddr_cantOpenItMsg = 0x5b3f; // "I can't open it"
// Don't Need Them Message : 0x5b51 to 0x5b64
const uint16 dsAddr_dontNeedThemMsg = 0x5b51; // "I don't need them"
// Peeping Tom Message : 0x5b65 to 0x5b7f
const uint16 dsAddr_peepingTomMsg = 0x5b65; // "What am I? A Peeping Tom?"
// Big Pockets Message : 0x5b80 to 0x5baa
const uint16 dsAddr_bigPocketsMsg = 0x5b80; // "I have big pockets, but there are limits"
// Trouble With Stairs Message : 0x5bab to 0x5be6
const uint16 dsAddr_troubleWithStairsMsg = 0x5bab; // "If I put it on I might have trouble walking up the stairs"
// 9 Lives To Read Message : 0x5be7 to 0x5c0a
const uint16 dsAddr_9LivesToReadMsg = 0x5be7; // "I'd need 9 lives to read them all"
// Thanks Not Tired Message : 0x5c0b to 0x5c25
const uint16 dsAddr_thanksNotTiredMsg = 0x5c0b; // "Thanks, I'm not so tired"
// No Need To Turn On Message : 0x5c26 to 0x5c45
const uint16 dsAddr_noNeedToTurnOnMsg = 0x5c26; // "There's no need to turn it on"
// Won't Bear Weight Message : 0x5c46 to 0x5c5f
const uint16 dsAddr_wontBearWeightMsg = 0x5c46; // "It won't bear my weight"
// Never Learnt Message : 0x5c60 to 0x5c81
const uint16 dsAddr_neverLearntMsg = 0x5c60; // "I never learnt to how use one"
// So Sharp Message : 0x5c82 to 0x5cab
const uint16 dsAddr_soSharpMsg = 0x5c82; // "They're so sharp they'd rip my trousers!"
// Cognac Message : 0x5cac to 0x5cda
const uint16 dsAddr_cognacMsg = 0x5cac; // "Pfui! The cognac really didn't do any good"
// No Time For Pleasures Message : 0x5cdb to 0x5cfc
const uint16 dsAddr_noTimeForPleasuresMsg = 0x5cdb; // "I don't have time for pleasures"
// Not Socks With Bare Hands Message : 0x5cfd to 0x5d2b
const uint16 dsAddr_notSocksWithBareHandsMsg = 0x5cfd; // "I won't touch these socks with my bare hands!"
// Not Halloween Message : 0x5d2c to 0x5d40
const uint16 dsAddr_notHalloweenMsg = 0x5d2c; // "It's not Halloween"
// Not Manual Message : 0x5d41 to 0x5d6d
const uint16 dsAddr_NotManualMsg = 0x5d41; // "It can't be controlled manually! I hate it!"
// Nothing To Play Message : 0x5d6e to 0x5d86
const uint16 dsAddr_nothingToPlayMsg = 0x5d6e; // "I have nothing to play"
// Not Mine Message : 0x5d87 to 0x5da7
const uint16 dsAddr_notMineMsg = 0x5d87; // "I can't take it. It's not mine."
// Hey What's The Matter Message : 0x5da8 to 0x5dc1
const uint16 dsAddr_HeyWtmQMsg = 0x5da8; // "Hey! What's the matter?!"
// Its Open Message : 0x5dc2 to 0x5dcd
const uint16 dsAddr_ItsOpenMsg = 0x5dc2; // "It's Open!"
// Out Of Order Message : 0x5dce to 0x5de1
const uint16 dsAddr_outOfOrderMsg = 0x5dce; // "It's out of order"
// Captain Watching Message : 0x5de2 to 0x5e0a
const uint16 dsAddr_captainWatchingMsg = 0x5de2; // "with captain watching? Better not"
// Blunt Sickle Message : 0x5e0b to 0x5e24
const uint16 dsAddr_bluntSickleMsg = 0x5e0b; // "The sickle is too blunt"
// First Business Message : 0x5e25 to 0x5e53
const uint16 dsAddr_firstBusinessMsg = 0x5e25; // "First I've got some business to take care of"
// No Digging Knife Message : 0x5e54 to 0x5e8e
const uint16 dsAddr_noDiggingKnifeMsg = 0x5e54; // "Digging it out with the knife could take a hundred years"

// FIXME - Where is this message used?! Unused?
// No Mess On Table Message : 0x5e8f to 0x5ebd
const uint16 dsAddr_noMessOnTableMsg = 0x5e8f; // "I don't want to make more mess on this table"

// Throw Crumbs To Bird Question Message : 0x5ebe to 0x5ee5
const uint16 dsAddr_throwCrumbsToBirdQMsg = 0x5ebe; // "Should I throw the crumbs to the bird?"
// Don't Waste Crumbs Message : 0x5ee6 to 0x5f10
const uint16 dsAddr_dontWasteCrumbs = 0x5ee6; // "I don't want to waste these tasty crumbs"
// Might Slip Fall In Message : 0x5f11 to 0x5f3b
const uint16 dsAddr_mightSlipFallInMsg = 0x5f11; // "Better not... I might slip and fall in..."
// Book Color Message Address Pointers : (6 * 2-byte) = 0x5f3c to 0x5f47
const uint16 dsAddr_bookColorMsgPtr = 0x5f3c;
// Book Color Message #0 : 0x5f48 to 0x5f60
const uint16 dsAddr_bookColorMsg0 = 0x5f48; // ""The history of blues""
// Book Color Message #1 : 0x5f61 to 0x5f8f
const uint16 dsAddr_bookColorMsg1 = 0x5f61; // ""Manchester United, or the Red Devils story""
// Book Color Message #2 : 0x5f90 to 0x5fb5
const uint16 dsAddr_bookColorMsg2 = 0x5f90; // ""Greyhounds and other hunting dogs""
// Book Color Message #3 : 0x5fb6 to 0x5fe6
const uint16 dsAddr_bookColorMsg3 = 0x5fb6; // ""Greenhorn, or my adventures in the Wild West""
// Book Color Message #4 : 0x5fe7 to 0x6008
const uint16 dsAddr_bookColorMsg4 = 0x5fe7; // ""Charlie Brown and his company""
// Book Color Message #5 : 0x6009 to 0x6034
const uint16 dsAddr_bookColorMsg5 = 0x6009; // ""Pink Panther: an unauthorised biography""

// Mansion Intrusion Function Pointers : (5 * 2-byte) = 0x6035 to 0x603e
const uint16 dsAddr_MansionIntrusionFnPtr = 0x6035;

// Save State Region : 0x6478 to 0xdbf1
const uint16 dsAddr_saveState = 0x6478;
const uint16 saveStateSize = 0x777a;

// Save Description String (24 bytes) : 0x6478 to 0x648f

// Ego (Mark) position in scene : 0x64af to 0x64b2
const uint16 dsAddr_egoX = 0x64af; // 2 bytes
const uint16 dsAddr_egoY = 0x64b1; // 2 bytes

// Idle Animation List Table : 0x6540 to 0x????
const uint16 dsAddr_idleAnimationListPtr = 0x6540;

// Palette Effect Data : 0x6609 to 0x????
const uint16 dsAddr_paletteEffectData = 0x6609;

// Scene Fade Table (2 byte address * 42): 0x663e to 0x6691
const uint16 dsAddr_sceneFadeTablePtr = 0x663e;

// Scene Walkbox Table (2 byte LE address * 42) : 0x6746 to 0x6799
const uint16 dsAddr_sceneWalkboxTablePtr = 0x6746;

// Scene Zoom Table (2 byte address * 42) : 0x70f4 to 0x7147
const uint16 dsAddr_sceneZoomTablePtr = 0x70f4;

// Scene Object Table (2 byte address * 42) : 0x7254 to 0x72a7
const uint16 dsAddr_sceneObjectTablePtr = 0x7254;

// Scene Object Name : Sonny or whatever : 0x92e5 to 0x92f6
const uint16 dsAddr_scnObjNameSonny = 0x92e5; // "Sonny or whatever"

// Scene Object Name - Anne : 0x9820 to 0x9824
const uint16 dsAddr_scnObjNameAnne = 0x9820; // "Anne"

// Scene Object Name - Mike : 0xaa94 to 0xaa98
const uint16 dsAddr_scnObjNameMike = 0xaa94; // "Mike"

// Current Scene Id : 0xb4f3
const uint16 dsAddr_currentScene = 0xb4f3; // 1 byte

// Ons Animation Table (2 byte address * ??) : 0xb4f5 to 0x????
const uint16 dsAddr_onsAnimationTablePtr = 0xb4f5;

// Examine Object Callback Table (2 byte LE address * ??) : 0xb5ce to 0x????
const uint16 dsAddr_objExamineCallbackTablePtr = 0xb5ce;

// Use Object Callback Table (2 byte LE address * ??) : 0xb89c to 0x????
const uint16 dsAddr_objUseCallbackTablePtr = 0xb89c;

// Inventory Object Callback Table (3 byte (id, callbackAddr) * 7) : 0xbb72 to 0xbb86
const uint16 dsAddr_objCallbackTablePtr = 0xbb72;
// invItemToolboxFull = csAddr_openFullToolbox
// invItemToolboxHalfEmpty = csAddr_openHalfEmptyToolbox
// invItemDiveEquipment = csAddr_useDivingEquipment
// invItemShovelAct2 = csAddr_digMansionWall
// 0xff = csAddr_tooDarkHere // TODO: No object has id 0xff - Callback Disabled?
// invItemBanknote = csAddr_examineBanknote
// invItemTimePills = csAddr_useTimePills

// Scene Hotspots Table (2 byte LE address * ??) : 0xbb87 to 0x????
const uint16 dsAddr_sceneHotspotsPtr = 0xbb87;

// Inventory Object Combining Table (5 byte (id, id, new object id, msgAddr) * 34) : 0xc335 to 0xc3de
const uint16 dsAddr_objCombiningTablePtr = 0xc335;
// 3 byte null terminator for Combining table 0xc3df to 0xc3e1

// Object Combine Error Message : 0xc3e2 to 0xc41e
const uint16 dsAddr_objCombineErrorMsg = 0xc3e2; // "Using these two objects ..."

// Inventory (item ids held by Ego) (1 byte * 24) : 0xc48d to 0xc4a4
const uint16 dsAddr_inventory = 0xc48d;
// 0xc4a5 is null word alignment byte
// Inventory item data address table (2 bytes * 92) : 0xc4a6 to 0xc55d
const uint16 dsAddr_inventoryItemDataPtrTable = 0xc4a6;

// Lans Animation Table (4 byte * ??) : 0xd89e to 0x????
const uint16 dsAddr_lansAnimationTablePtr = 0xd89e;

// Spoken With Mansion Guard Flag : 0xda96
// FIXME - This is probably unecessary as although this location is set, it
//         doesn't now appear to be read.
const uint16 dsAddr_spokenWithMansionGuardFlag = 0xda96; // 1 byte
// Have Not Spoken With Mansion Guard Flag : 0xda97
// FIXME - This is probably unecessary as although this location is set, it
//         doesn't now appear to be read.
const uint16 dsAddr_haveNotSpokenWithMansionGuardFlag = 0xda97; // 1 byte

// Dialog Stack - Pleading with Mansion Guard : 0xdaa6 to 0xdab1
const uint16 dsAddr_dialogStackPleadingToMansionGuard = 0xdaa6;
// Dialog Stack - Mansion Guard Drinking : 0xdab2 to 0xdab9
// FIXME - Can't find where this is used...
const uint16 dsAddr_dialogStackMansionGuardDrinking = 0xdab2;
// Dialog Stack - Talking To Sonny : 0xdaba to 0xdac3
const uint16 dsAddr_dialogStackSonny = 0xdaba;
// Dialog Stack - Talking To Grandpa : 0xdac4 to 0xdacd
const uint16 dsAddr_dialogStackGrandpa = 0xdac4;
// Cave Thorns Cut Down Flag : 0xdaca
// FIXME - Cave Thorns Flag overlap with dsAddr_dialogStackGrandpa. Bug or typo?
const uint16 dsAddr_caveThornsCutDownFlag = 0xdaca; // 1 byte
// Dialog Stack - Trying To Borrow Shotgun From Grandpa : 0xdace to 0xdad3
const uint16 dsAddr_dialogStackGrandpaShotgun = 0xdace;
// Dialog Stack - Trying To Borrow Fan From Grandpa : 0xdad4 to 0xdad9
const uint16 dsAddr_dialogStackGrandpaFan = 0xdad4;
// Dialog Stack - Ask Old Lady if OK : 0xdada to 0xdaef
const uint16 dsAddr_dialogStackAskOldLadyOK = 0xdada;
// Dialog Stack - Talking To Old Lady : 0xdaf0 to 0xdaf5
const uint16 dsAddr_dialogStackOldLady = 0xdaf0;
// Dialog Stack - Borrow Duster From Old Lady : 0xdaf6 to 0xdafb
const uint16 dsAddr_dialogStackBorrowDusterFromOldLady = 0xdaf6;
// Dialog Stack - Get Old Lady's Apple : 0xdafc to 0xdb01
const uint16 dsAddr_dialogStackGetAppleOldLady = 0xdafc;
// Dialog Stack - Giving Another Flower To Anne : 0xdb02 to 0xdb07
const uint16 dsAddr_dialogStackAnotherFlowerToAnne = 0xdb02;
// Dialog Stack - Talking To Squirrel : 0xdb08 to 0xdb13
const uint16 dsAddr_dialogStackSquirrel = 0xdb08;
// Dialog Stack - Talking To Dog : 0xdb14 to 0xdb1d
const uint16 dsAddr_dialogStackDog = 0xdb14;
// Dialog Stack - Take Axe : 0xdb1e to 0xdb23
const uint16 dsAddr_dialogStackTakeAxe = 0xdb1e;
// Dialog Stack - Talking To Busy Cook : 0xdb24 to 0xdb2d
const uint16 dsAddr_dialogStackBusyCook = 0xdb24;
// Dialog Stack - Talking To Mike the Robot Safe : 0xdb2e to 0xdb35
const uint16 dsAddr_dialogStackRobotSafe = 0xdb2e;
// Dialog Stack - Talking To John Noty At Endgame : 0xdb36 to 0xdb3f
const uint16 dsAddr_dialogStackJohnNotyEndgame = 0xdb36;
// Dialog Stack - Camp Guard Waiting For Documents : 0xdb40 to 0xdb4b
const uint16 dsAddr_dialogStackCampGuardWantsDocuments = 0xdb40;
// Dialog Stack - Camp Guard Reading Soldier News : 0xdb4c to 0xdb55
const uint16 dsAddr_dialogStackCampGuardReadingNews = 0xdb4c;
// Dialog Stack - Camp Guard Show Pass : 0xdb56 to 0xdb5b
const uint16 dsAddr_dialogStackCampGuardShowPass = 0xdb56;
// Dialog Stack - Jail Door Grates : 0xdb5c to 0xdb67
const uint16 dsAddr_dialogStackJailDoorGrates = 0xdb5c;
// Dialog Stack - Talking to Barman : 0xdb68 to 0xdb71
const uint16 dsAddr_dialogStackBarman = 0xdb68;
// Dialog Stack - Fall Into Mudpool : 0xdb72 to 0xdb79
const uint16 dsAddr_dialogStackFallIntoMudpool = 0xdb72;
// Dialog Stack - Talking To Mudpool Bird : 0xdb7a to 0xdb81
const uint16 dsAddr_dialogStackMudpoolBird = 0xdb7a;
// Dialog Stack - Interrogate Captain : 0xdb82 to 0xdb89
const uint16 dsAddr_dialogStackInterrogateCaptain = 0xdb82;
// Dialog Stack - Bar Cellar Door : 0xdb8a to 0xdb8f
const uint16 dsAddr_dialogStackBarCellarDoor = 0xdb8a;
// Current Music Id Playing : 0xdb90
const uint16 dsAddr_currentMusic = 0xdb90; // 1 byte
// Unused Byte : 0xdb91
// Already Adjusted Hoop Pole Flag : 0xdb92
const uint16 dsAddr_alreadyAdjustedHoopPoleFlag = 0xdb92; // 1 byte
// Already Kicked Hen Flag : 0xdb93
const uint16 dsAddr_alreadyKickedHenFlag = 0xdb93; // 1 byte
// Already Pulled Trunk Release Lever Flag : 0xdb94
const uint16 dsAddr_alreadyPulledTrunkReleaseLeverFlag = 0xdb94; // 1 byte
// Car Trunk Empty Flag : 0xdb95
const uint16 dsAddr_carTrunkEmptyFlag = 0xdb95; // 1 byte
// Birds Gone From Scarecrow Flag : 0xdb96
const uint16 dsAddr_birdsGoneFromScarecrowFlag = 0xdb96; // 1 byte
// Already Spoken To Anne Flag : 0xdb97
const uint16 dsAddr_alreadySpokenToAnneFlag = 0xdb97; // 1 byte
// Flower Isle in Lake State (0 = Both Flowers Present, 1 = One Flower Taken, 2+ = Both Flowers Taken): 0xdb98
const uint16 dsAddr_flowerIsleState = 0xdb98; // 1 byte
// Already Got Broken Paddle Flag : 0xdb99
const uint16 dsAddr_alreadyGotBrokenPaddleFlag = 0xdb99; // 1 byte
// Given Flower To OldLady Already Flag : 0xdb9a
const uint16 dsAddr_givenFlowerToOldLadyAlreadyFlag = 0xdb9a; // 1 byte
// Given Flower To Anne Already Flag : 0xdb9b
const uint16 dsAddr_givenFlowerToAnneAlreadyFlag = 0xdb9b; // 1 byte
// Scared Guard Already Flag : 0xdb9c
const uint16 dsAddr_scaredGuardAlreadyFlag = 0xdb9c; // 1 byte
// Got Needle Already Flag : 0xdb9d
const uint16 dsAddr_gotNeedleAlreadyFlag = 0xdb9d; // 1 byte
// Got Potato Already Flag : 0xdb9e
const uint16 dsAddr_gotPotatoAlreadyFlag = 0xdb9e; // 1 byte
// Bees Gone Flag : 0xdb9f
const uint16 dsAddr_beesGoneFlag = 0xdb9f; // 1 byte
// Mansion Already Been Through Tunnel Flag : 0xdba0
const uint16 dsAddr_mansionTunnelDoneFlag = 0xdba0; // 1 byte
// Mansion Tree Hollow Empty Flag : 0xdba1
const uint16 dsAddr_mansionTreeHollowEmptyFlag = 0xdba1; // 1 byte
// Climbed Mansion Tree Already Flag : 0xdba2
const uint16 dsAddr_climbedMansionTreeAlreadyFlag = 0xdba2; // 1 byte
// Cellar Door Open Flag : 0xdba3
const uint16 dsAddr_cellarDoorOpenFlag = 0xdba3; // 1 byte
// Cellar Light On Flag : 0xdba4
const uint16 dsAddr_lightOnFlag = 0xdba4; // 1 byte
// Laundry State (0 = Wet on Line, 1 = Dry on Line, 2 = Not Present): 0xdba5
const uint16 dsAddr_laundryState = 0xdba5; // 1 byte
// Lake Diving Exit Message (0 to 5+) : 0xdba6
const uint16 dsAddr_lakeDivingExitMessage = 0xdba6; // 1 byte
// Searched Grandpa Drawers Flag : 0xdba7
const uint16 dsAddr_SearchedGrandpaDrawersFlag = 0xdba7; // 1 byte
// Hankerchief in Mousehole Flag : 0xdba8
const uint16 dsAddr_HankerchiefInMouseholeFlag = 0xdba8; // 1 byte
// Mouse Hole State : 0xdba9, 0 = Mouse Gone, 1 = Mouse Trapped, 2 = Mouse Success(?)
const uint16 dsAddr_mouseHoleState = 0xdba9; // 1 byte
// Mouse Nerve Message Said Flag : 0xdbaa
const uint16 dsAddr_mouseNerveMsgSaidFlag = 0xdbaa; // 1 byte
// Mouse Already Got Gold Nugget Flag : 0xdbab
const uint16 dsAddr_mouseGotGoldNuggetFlag = 0xdbab; // 1 byte
// Unused Byte : 0xdbac
// Dog Has Bone Flag : 0xdbad
const uint16 dsAddr_dogHasBoneFlag = 0xdbad; // 1 byte
// Ego Already Scared By Spider Flag : 0xdbae
const uint16 dsAddr_egoAlreadyScaredBySpiderFlag = 0xdbae; // 1 byte
// Already Said That Anne is Beautiful Flag : 0xdbaf
const uint16 dsAddr_alreadySaidAnneBeautifulFlag = 0xdbaf; // 1 byte
// Squirrel's Nut State (0 = Nut in Tree, 1 = Nut in Grass, 2 = Nut Found with Rake) : 0xdbb0
const uint16 dsAddr_squirrelNutState = 0xdbb0; // 1 byte
// Nut Swapped For Apple in Fruit Bowl Flag : 0xdbb1
const uint16 dsAddr_nutSwappedForAppleFlag = 0xdbb1; // 1 byte
// Spoken To Man In Well Flag : 0xdbb2
const uint16 dsAddr_spokenToManInWellFlag = 0xdbb2; // 1 byte
// Spoken To Mirror Flag : 0xdbb3
const uint16 dsAddr_spokenToMirrorFlag = 0xdbb3; // 1 byte
// Cellar Shelves Examine Count (0 to 2(clamped))) : 0xdbb4
const uint16 dsAddr_cellarShelfExamineCount = 0xdbb4; // 1 byte
// Examined Bank Note Flag : 0xdbb5
const uint16 dsAddr_examinedBanknoteFlag = 0xdbb5; // 1 byte
// VGA Artist Quip Already Said Flag : 0xdbb6
const uint16 dsAddr_vgaArtistQuipAlreadySaidFlag = 0xdbb6; // 1 byte
// Mansion Desk Blue Drawer Open Flag : 0xdbb7
const uint16 dsAddr_blueDrawerOpenFlag = 0xdbb7; // 1 byte
// Mansion Desk Red Drawer Open Flag : 0xdbb8
const uint16 dsAddr_redDrawerOpenFlag = 0xdbb8; // 1 byte
// Mansion Desk Grey Drawer Open Flag : 0xdbb9
const uint16 dsAddr_greyDrawerOpenFlag = 0xdbb9; // 1 byte
// Mansion Desk Green Drawer Open Flag : 0xdbba
const uint16 dsAddr_greenDrawerOpenFlag = 0xdbba; // 1 byte
// Mansion Desk Brown Drawer Open Flag : 0xdbbb
const uint16 dsAddr_brownDrawerOpenFlag = 0xdbbb; // 1 byte
// Mansion Desk Pink Drawer Open Flag : 0xdbbc
const uint16 dsAddr_pinkDrawerOpenFlag = 0xdbbc; // 1 byte
// Mansion Colored Drawer Puzzle Hint Message Given Flag : 0xdbbd
const uint16 dsAddr_drawerPuzzleHintGivenFlag = 0xdbbd; // 1 byte
// Mansion Colored Drawer Got Dictaphone Flag : 0xdbbe
const uint16 dsAddr_drawerGotDictaphoneFlag = 0xdbbe; // 1 byte
// Mansion Colored Drawer Got Polaroid Flag : 0xdbbf
const uint16 dsAddr_drawerGotPolaroidFlag = 0xdbbf; // 1 byte
// Mansion Colored Drawer Puzzle Book Message Flag : 0xdbc0
const uint16 dsAddr_drawerPuzzleBookMessageFlag = 0xdbc0; // 1 byte
// Mansion Colored Drawer Puzzle - Random Book Color Value (0 = No Book, 1 to 6 = Books) : 0xdbc1
const uint16 dsAddr_drawerPuzzleBookValue = 0xdbc1; // 1 byte
// Mansion Colored Drawer Puzzle Solved Flag : 0xdbc2
const uint16 dsAddr_drawerPuzzleSolvedFlag = 0xdbc2; // 1 byte
// Mansion Trashcan Searched Flag : 0xdbc3
const uint16 dsAddr_mansionTrashcanSearchedFlag = 0xdbc3; // 1 byte
// Mansion Read Newspaper Flag : 0xdbc4
const uint16 dsAddr_mansionReadNewspaperFlag = 0xdbc4; // 1 byte
// Mansion TV On Flag : 0xdbc5
const uint16 dsAddr_mansionTVOnFlag = 0xdbc5; // 1 byte
// Mansion VCR Playing Tape Flag : 0xdbc6
const uint16 dsAddr_mansionVCRPlayingTapeFlag = 0xdbc6; // 1 byte
// Mansion VCR Played Tape Before Flag : 0xdbc7
const uint16 dsAddr_mansionVCRPlayedTapeBeforeFlag = 0xdbc7; // 1 byte
// Mansion VCR Tape Loaded Flag : 0xdbc8
const uint16 dsAddr_mansionVCRTapeLoadedFlag = 0xdbc8; // 1 byte
// Mansion Examined Couch Before Flag : 0xdbc9
const uint16 dsAddr_mansionExaminedCouchBeforeFlag = 0xdbc9; // 1 byte
// Mansion Used Polaroid on TV Flag : 0xdbca
const uint16 dsAddr_usedPolaroidOnTVFlag = 0xdbca; // 1 byte
// Mansion Used Dictaphone on TV Flag : 0xdbcb
const uint16 dsAddr_usedDictaphoneOnTVFlag = 0xdbcb; // 1 byte
// Mansion Cook Gone Flag : 0xdbcc
const uint16 dsAddr_MansionCookGoneFlag = 0xdbcc; // 1 byte
// Mansion Radio Broken Flag : 0xdbcd
const uint16 dsAddr_MansionRadioBrokenFlag = 0xdbcd; // 1 byte
// Mansion Got Radio Batteries Flag : 0xdbce
const uint16 dsAddr_MansionGotRadioBatteriesFlag = 0xdbce; // 1 byte
// Mansion Have Opened Fridge Before Flag : 0xdbcf
const uint16 dsAddr_MansionHaveOpenedFridgeBeforeFlag = 0xdbcf; // 1 byte
// Mansion Put Burning Paper In Fridge Flag : 0xdbd0
const uint16 dsAddr_MansionPutBurningPaperInFridgeFlag = 0xdbd0; // 1 byte
// Mansion Robot Safe Unlocked Flag : 0xdbd1
const uint16 dsAddr_MansionRobotSafeUnlockedFlag = 0xdbd1; // 1 byte
// Mansion Robot Safe Voice Test Passed Flag : 0xdbd2
const uint16 dsAddr_MansionRobotSafeVoiceTestPassedFlag = 0xdbd2; // 1 byte
// Mansion Robot Safe Scent Test Passed Flag : 0xdbd3
const uint16 dsAddr_MansionRobotSafeScentTestPassedFlag = 0xdbd3; // 1 byte
// Mansion Robot Safe View Test Passed Flag : 0xdbd4
const uint16 dsAddr_MansionRobotSafeViewTestPassedFlag = 0xdbd4; // 1 byte
// Mansion John Noty Outside Bathroom Flag : 0xdbd5
const uint16 dsAddr_MansionJohnNotyOutsideBathroomFlag = 0xdbd5; // 1 byte
// Mansion Sink State (0 - No Plug, Sink Empty, 1 - Plug, Sink Empty, 2 - Plug, Sink Full) : 0xdbd6
const uint16 dsAddr_MansionSinkState = 0xdbd6; // 1 byte
// Mansion Through Fan By Time Pill Flag : 0xdbd7
const uint16 dsAddr_MansionThruFanByTimePillFlag = 0xdbd7; // 1 byte
// Mansion Ventilator Fan Stopped Flag : 0xdbd8
const uint16 dsAddr_MansionVentFanStoppedFlag = 0xdbd8; // 1 byte
// Mansion John Noty Escaping Flag : 0xdbd9
const uint16 dsAddr_MansionJohnNotyEscapingFlag = 0xdbd9; // 1 byte
// Shown Pass To Guard Flag : 0xdbda
const uint16 dsAddr_ShownPassToGuardFlag = 0xdbda; // 1 byte
// Graffiti Message Id (0 to 6) : 0xdbdb
const uint16 dsAddr_graffitiMsgId = 0xdbdb; // 1 byte
// Got Food Bowl in Jail Flag : 0xdbdc
const uint16 dsAddr_GotFoodBowlInJailFlag = 0xdbdc; // 1 byte
// Jail Cable and Bowl State (0 = Cable not in Bowl, 1 = Cable in Bowl, 2 = Bowl Electrified 3 = Captain Shocked) : 0xdbdd
const uint16 dsAddr_JailCableAndBowlState = 0xdbdd; // 1 byte
// Got Jail Key Flag : 0xdbde
const uint16 dsAddr_GotJailKeyFlag = 0xdbde; // 1 byte
// First Act Trial State (0 = Before First Trial, 1 to 3 = Trial 1st to 3rd) : 0xdbdf
const uint16 dsAddr_FirstActTrialState = 0xdbdf; // 1 byte
// Already Tickled Captain Flag : 0xdbe0
const uint16 dsAddr_AlreadyTickledCaptainFlag = 0xdbe0; // 1 byte
// Cut Fence Flag : 0xdbe1
const uint16 dsAddr_cutFenceFlag = 0xdbe1; // 1 byte
// Act 1 Guard State (0 = Normal, 1 = With Kaleidoscope & Grenade, 2 = Kaleidoscope & No Grenade) : 0xdbe2
const uint16 dsAddr_act1GuardState = 0xdbe2; // 1 byte
// Spoken to Barman About Third Trial Flag : 0xdbe3
const uint16 dsAddr_spokeToBarmanAboutThirdTrialFlag = 0xdbe3; // 1 byte
// Got Mug Of Mud Flag : 0xdbe4
const uint16 dsAddr_gotMugOfMudFlag = 0xdbe4; // 1 byte
// Got Rope In Act 1 Flag : 0xdbe5
const uint16 dsAddr_gotRopeAct1Flag = 0xdbe5; // 1 byte
// Captain Drawer State : 0xdbe6
const uint16 dsAddr_captainDrawerState = 0xdbe6; // 1 byte
// Bird on Bar Radio Antenna Flag : 0xdbe7
const uint16 dsAddr_birdOnBarRadioAntennaFlag = 0xdbe7; // 1 byte
// Swapped Barman Mug Flag : 0xdbe8
const uint16 dsAddr_swappedBarmanMugFlag = 0xdbe8; // 1 byte
// Barman Passed Out Flag : 0xdbe9
const uint16 dsAddr_barmanPassedOutFlag = 0xdbe9; // 1 byte
// Counter for Mansion Intrusion Attempts : 0xdbea
const uint16 dsAddr_mansionEntryCount = 0xdbea;// 1 byte
// Unused Byte : 0xdbeb
// John Noty Outside Mansion Door Flag : 0xdbec
const uint16 dsAddr_johnNotyOutsideMansionDoorFlag = 0xdbec; // 1 byte
// Unused Byte : 0xdbed
// Lovestruck By Anne Flag : 0xdbee
const uint16 dsAddr_lovestruckByAnneFlag = 0xdbee;// 1 byte
// Mansion Handle in Door Hole Flag : 0xdbef
const uint16 dsAddr_mansionHandleInDoorHoleFlag = 0xdbef;// 1 byte
// Got Password Need to Speak To Barman Flag : 0xdbf0
const uint16 dsAddr_gotPasswordNeedSpeakBarmanFlag = 0xdbf0; // 1 byte
// Mansion Already Used Time Pills Flag : 0xdbf1
const uint16 dsAddr_mansionAlreadyUsedTimePillsFlag = 0xdbf1; // 1 byte

// Intro Credits #1 : 0xe3c2 to 0xe3e5 (Read Only)
const uint16 dsAddr_introCredits1 = 0xe3c2; // "backgrounds ..."
// Intro Credits #2 : 0xe3e6 to 0xe3fe (Read Only)
const uint16 dsAddr_introCredits2 = 0xe3e6; // "music ..."
// Intro Credits #3 : 0xe3ff to 0xe42e (Read Only)
const uint16 dsAddr_introCredits3 = 0xe3ff; // "animation..."
// Intro Credits #4 : 0xe42f to 0xe45b (Read Only)
const uint16 dsAddr_introCredits4 = 0xe42f; // "programming..."
// Credits #5 : 0xe45c to 0xe47b (Read Only)
const uint16 dsAddr_credits5 = 0xe45c; // "after the tiring journey..."
// Final Credits #6 : 0xe47c to 0xe487 (Read Only)
const uint16 dsAddr_finalCredits6 = 0xe47c; // "THE END..."
// Final Credits #7 : 0xe488 to 0xe782 (Read Only)
const uint16 dsAddr_finalCredits7 = 0xe488; // "programming..."
// 0xe783 to 0xe78f: 13 null bytes at end of dseg data - segment alignment padding?

class Resources {
public:
	Resources();
	~Resources();
	bool loadArchives(const ADGameDescription *gd);

	void loadOff(Graphics::Surface &surface, byte *palette, int id);
	Common::SeekableReadStream *loadLan(uint32 id) const;
	Common::SeekableReadStream *loadLan000(uint32 id) const;

	/*
	 * PSP (as the other sony playstation consoles - to be confirmed and 'ifdef'ed here too)
	 * is very sensitive to the number of simultaneously opened files.
	 * This is an attempt to reduce their number to zero.
	 * TransientFilePack does not keep opened file descriptors and reopens it on each request.
	 */
#ifdef __PSP__
	TransientFilePack off, on, ons, lan000, lan500, sam_mmm, sam_sam, mmm, voices;
#else
	FilePack off, on, ons, lan000, lan500, sam_mmm, sam_sam, mmm, voices;
#endif

	Segment dseg;
	Font font7, font8;

	//const byte *getDialog(uint16 dialogNum) { return eseg.ptr(dialogOffsets[dialogNum]); }
	uint16 getDialogAddr(uint16 dialogNum) { return dialogOffsets[dialogNum]; }

	Segment eseg;
private:
	void precomputeDialogOffsets();

	Common::Array<uint16> dialogOffsets;
};

} // End of namespace TeenAgent

#endif
