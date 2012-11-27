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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef STATICDATA_H
#define STATICDATA_H

#define NUM_VARIANTE     6
#define MAZE_SCREEN      39                         // First maze screen
#define DONT_CARE        0xFF                       // Any state allowed in command verb
#define DOORDELAY        12                         // Time for a door to open
#define PENDELAY         25                         // Hold off going upstairs
#define LIPDX            11                         // To position LIPS object on person
#define LIPDY            8
#define DARTTIME         70                         // This many ticks to fall asleep
#define NORMAL_TPS_v1d   8                          // Number of ticks (frames) per second
#define NORMAL_TPS_v2d   9                          // Number of ticks (frames) per second
#define STORYDELAY       (5 * NORMAL_TPS_v2d)

//***************************************************************************
// Hugo 1 Windows
//***************************************************************************
const char *textData_1w[] = {
	"",
	"You don't have it.",
	"You don't have the key.",
	"It is too dark to see\nanything in there.",
	"You don't have anything\nto oil it with.",
//5
	"You don't have anything\nto cut it with.",
// Strings for when object is in wrong state for cmd
	"That wouldn't work.",
	"It is already locked.",
	"It is already unlocked.",
	"You unlock the door with the key.",
//10
	"You grasp the pumpkin\nand try to prise it apart.",
	"It is locked.",
	"It is already open.",
	"It is already closed.",
	"It is already smashed.",
//15
	"There is nothing in there.",
	"You are already wearing it\nyou cross-eyed baboon!",
	"You don't have it on,\nI know it's hard to tell!",
	"You'll need to\ntake it off first.",
	"You already oiled it,\nwhy don't you just\nopen it???",
//20
	"Despite your best efforts the\nbolt refuses to budge.",
	"I believe you already cut it!",
	"It is a rock, of no\nparticular significance.",
// Strings for when cmd successfully carried out (cmnd->donestr)
	"Ok.",
	"You blow the whistle.",
//25
	"The shed seems to have fallen\ninto disuse and is pretty\nmuch full of garbage, either\nbroken or rusted to pieces.\nYou are just on the point of\nleaving when you notice an\noilcan sitting on a shelf.",
	"I said it's an oil-CAN,\nnot an oil-LAMP dummy!",
	"Look what you found - a trapdoor!",
	"You put a few drops of\noil on the rusty bolt.",
	"With a mighty effort you\nmanage to slide the bolt open!",
//30
	"Stupidly, you slide the bolt shut.",
	"You cut the rope with your knife.\nThe boat is now floating free!",
	"The rope is knotted too tightly\nand you are, alas, unable to\nuntie it despite your best efforts!",
	"This rope is a lot stronger\nthan you and you are, alas,\nunable to break it.",
	"The boat is now watertight!",
//35
	"The oldman seems somewhat\nsurprised by your attack and a\nlittle offended.\n\nYou may wish to try a different\nstrategy!",
// Added in data1.cpp by Strangerke, to avoid hardcoded strings in background_t variable initialization*/
	"Nobody answers!",
	"It's not locked!",
// Object descriptions, invoked by the words "look" (at object)
	"It is you, the hero.",
	"Why not try opening it?",
//40
	"Just a regular wooden closet.",
	"They appear to be looking at you!",
	"It's just flapping around up there.",
	"There appears to be\nsomething inside it.",
	"Looks like a front door key to me.",
//45
	"A useful looking candle.",
	"A rather useful looking penknife.",
	"It is a little silver whistle.",
	"It appears to be a halloween mask\nlooking somewhat like a grotesque\nmonkey's head!",
	"I don't think you want to mess with\nthis guy!",
//50
	"A rather yummy looking pork chop.",
	"Frankenstein's monster.  Nice suit and tie!",
	"Count Dracula.  Wonder what he's drinking!",
	"It's Gwendolin, the mad professor's daughter.",
	"It's the Fiendish Friar of Frankfurt!",
//55
	"This monster is called \"Slime\".",
	"This monster has a head\nthe size and color of a pea.",
	"This little oilcan looks like\nit contains some oil.",
	"A little wooden trapdoor.",
	"A rather rusty (but strong)\nmetal bolt.",
//60
	"Believe me, this is no Lassie!",
	"It looks like a serviceable boat.\nI wonder whether it would get\nyou to the other side?",
	"The rope is to stop\nthe boat drifting away!",
	"Wooo!  A small fortune in gold!",
	"He looks at least\n200 years old!!",
//65
	"This is one dude you\ndon't want to argue with!",
	"He looks totally gone!",
	"He has a certain charm,\nI suppose!",
	"Well, it's sort of\nround and rubbery!",
	"Try getting Hugo to\nbreak the pumpkin open!",
//70
	"Use the key on a door\nto unlock it!",
	"You don't need to do\nanything with the candle,\njust holding it is sufficient!",
	"The only thing you can do\nwith the mask is wear it.",
	"You can't use the\nbung on that!",
	"That's not the way\nto use the chop!",
//75
	"Trying to cut that won't help you!",
	"Oiling that is not useful!",
	"Your generous gift is refused!"
};

//***************************************************************************
// Hugo 2
//***************************************************************************
const char *textData_2w[] = {
	"",
// Use following standard strings where applicable
	"You see nothing very\ninteresting about it",
	"I see no purpose\nin doing that!",
	"You find nothing of\nany interest inside!",
// Strings for when required object(s) not carried
	"You don't have it",
//5
	"You don't have\nany matches!",
	"You don't have a weapon!",
// Strings for when object is in wrong state for cmd
	"That wouldn't work",
	"It is already closed",
	"It is already smashed",
//10
	"It is already open",
	"You already drank it!",
	"Cousin Harry is too convulsed\nwith laughter to talk!",
	"You can't!  Great Aunt Hester\nwill see you!",
	"Click!\nOops, I think you\nemptied it buster!",
// Strings for when cmd successfully carried out (cmnd->donestr)
//15
	"Ok!",
	"Now it's gone for ever!",
	"You break off a shoot and\nplace it near the cat.\nThe cat awakes from its\nreverie and sniffs your\ngift excitedly.  After 5\nminutes of friendly play\nit falls asleep again,\nrather contentedly!",
	"Hmm, what an interesting flavor!\n\n(...nothing happens.)",
	"You slide the paper under the door.",
//20
	"There's no point, the\nkey has already fallen.\nI'm afraid you're stuck!",
	"You already pushed the\nkey out the other side.\nI'm afraid you're stuck!",
	"Why don't you just\ntry reading it?",
	"Quietly opening it, you quickly\nscan it.  It appears to be a\nletter from Hester's attorney\nconcerning the life insurance\npolicy on Great Uncle Horace.\nFeeling rather uncomfortable\nabout reading other peoples'\nmail, you return it to its\nenvelope without reading any\nfurther.",
	"It won't open!",
//25
	"Just walk through!\nThis door will\nopen automatically!",
	"The door is firmly locked.",
	"You don't have any keys!",
	"The room is too dark to\nsee anything, now",
	"Opening the desk drawer reveals:\na book of matches!",
//30
	"Opening the cupboard reveals:\na clove of garlic!",
	"You need to say which\ncolor button to press!",
// Added in data1.cpp by Strangerke, to avoid hardcoded strings in background_t variable initialization*/
	"Nobody answers!",
	"It's not locked!",
// Object descriptions, invoked by the words "look" (at object)
	"It is the handsome Hugo!",
//35
	"It is the pretty Penelope!",
	"Why not try opening it?",
	"The door looks solidly built.",
	"You see a saucy looking french maid!",
	"The maid speaks to you:\nThat is ze wrong way, monsieur!\nPlease go upstairs to your room!",
//40
	"What a cute doggy!",
	"He looks at least\n200 years old!!",
	"You can see a rather impressive\nbook collection.\nOne yellow book in particular\nseems to stand out.",
	"One yellow book seems to be\nsticking out slightly.",
	"It is a quaint old four-poster.",
//45
	"It is a brightly colored parrot.",
	"A little book of matches.",
	"Your matches look a little soggy!\n(Presumably because you dropped\nthem in the stream so carelessly.)",
	"It is a brightly colored balloon.",
	"It is a dumb waiter,\nused to carry food\nup and down from\nthe kitchen.",
//50
	"The rope looks climbable.",
	"Ugh! The garlic smells foul!",
	"You can see 4 colored buttons.\nUnderneath the buttons is some\nlettering.  The lettering is very\nfaded and you can only make out\nthe following characters under\nthe green button:\n\"B*g *a*pe*\"",
	"The shed light is on!",
	"The gate lanterns are now on!",
//55
	"It is a strong-scented mint plant containing\na substance attractive to cats.",
	"A rickety old bridge.  Watch your step!",
	"They are small but have\nvery vicious looking\nproboscises.",
	"It is one of the rarer\nvenomous species:\nbeautiful but very deadly!",
	"It looks like a regular phone booth, on\nthe outside.  It seems unexpectedly\nroomy, on the inside.",
//60
	"Examining the gun, you see\nit contains one bullet!",
	"Examining the gun, you see\nit is now empty!",
	"The stick of dynamite has\na fuse at one end.",
	"You peer down into the\nmurky depths of the well\nand see only blackness...\nTaking a small pebble, you\nlean over and drop it down.\nAfter a while you hear a\ndull thud...",
	"It looks like an ancient\noil lamp.  It appears to\nbe totally empty now,\nexcept it feels curiously\nheavy.",
//65
	"It looks pretty moldy, I don't\nthink I'd try eating it!",
	"Just your regular genie type!",
	"It's Hugo's affable cousin Harry!",
	"It's Hugo's lovable aunt Hester!",
	"Yes, there is an envelope\non the table with a letter\ninside it!",
//70
	"Who is this mysterious fellow, I wonder?\nWho he is and where he comes from\nI have simply no idea!",
	"It's the cook!",
	"He's a very authoritative\nlooking police officer!",
	"A sprightly looking old man!",
	"It is an evil mechanized monster\nbent on the destruction of the\nuniverse!",
//75
	"It is a pile of smouldering metal!",
	"It's the kind of bell\none might use to\nsummon aid.",
	"Looking through the\nkeyhole, you can\nsee the key has been\nleft in the lock.\nToo bad it's on the\nother side!",
	"It's just a regular pencil.",
	"It is a magnifying glass\nused (for example) for\nreading small print.",
//80
	"It is a pretty solid\nlooking safe, I don't\nthink there's any way\nto open it unless you\nknow the combination!",
	"It is an extremely intricate\nlooking device, I'm afraid\nit's workings are quite\nbeyond me!",
	"It has some printing\non it, some of which\nis readable.",
	"It contains several old photos\nof Hugo as a child.  One photo\nin particular catches your\nattention.  It shows Hugo\nwith a young girl of about\nthe same age standing in front\nof a huge pointed tower.\nSomething about the girl strikes\nyou as being very familiar...",
	"A rather wistful-looking cat",
//85
	"It is a clear glass bottle.\nA label is attached with the\nword \"SERUM\" printed on it.",
	"It has a U-shaped fluorescent\nlight tube at its center, with\na series of metal rods spaced\nvertically around it.\nSurrounding the rods is a thick\nmesh-like screen which also acts\nas the frame of the device.",
// Strings when objects not used together
	"Setting fire to that would\nnot be very useful!",
	"Try giving the garlic\nto Penelope to eat!",
	"That's not the way\nto use the stick!",
//90
	"The serum is for\ncuring snake bites!",
	"I'd save the dynamite for\nlater, if I were you!",
	"That would not help you!",
	"I've no idea what you're trying to do!",
	"That won't work!",
//95
	"You can't unscrew that!",
	"Ding Dong!\n\n(...nothing happens)",
	"That's not the way\nto use the catnip!",
	"This is no time to\nread a newspaper!",
	"You can't use a\npencil on that!",
//100
	"Your examination reveals\nnothing of interest."
};

//***************************************************************************
// Hugo 3
//***************************************************************************
const char *textData_3w[] = {
	"",
// Use following standard strings where applicable
	"You see nothing very\ninteresting about it",
	"Nothing happens.",
// Strings for when required object(s) not carried
	"You don't have it",
	"You don't have a weapon!",
//5
	"You don't have anything\nto stick in it.",
	"There are three things\nrequired to perform an\nexorcism.  You do not\nhave all of them...",
// Strings for when object is in wrong state for cmd
	"That wouldn't work.",
	"It is already closed.",
	"It is already smashed.",
//10
	"It is already open.",
	"There is nothing in there.",
	"You already used the clay\nto make an effigy of the\nWitch Doctor",
	"It already has some\nwater inside it.",
	"There is nothing more\nto find there.",
//15
	"You can see a very slight\ndepression in the earth\nwhere something spherical\nprobably once rested there.",
	"It is already lit.",
	"It is not lit.",
	"Wheeee!",
// Strings for when cmd successfully carried out (cmnd->donestr)
	"Ok.",
//20
	"          LOST!\n\nNear the mighty boulder,\nMy precious crystal ball,\nI guess I couldn't hold 'er,\nSo I must've let her fall!\n\nIf anyone should find it,\nPlease return it to my keep,\nBe careful if you use it,\nFor its powers runneth deep!\n\nsigned:  The old man.",
	"My my, we are thorough,\naren't we?  Well, just\nfor you:\n\nThere is a crystal ball\nlying behind the rock,\nglinting in the sun.\n\nYou fetch it out...",
	"Ding dong!\n\n(Nothing happens...)",
	"You start to swing...\n\nThe ghost is impressed by\nyour fine acrobatics (but\nstill won't let you pass)!",
	"You grab the vine\nand start swinging...",
//25
	"It looks much like any other water.",
	"Mmm!  Delicious cool water!",
	"Mmm!  The water tastes delicious!\nBut what about poor Penelope?",
	"Swoosh!\n\nOops, missed him!  Your\naim is terrible, you need\na bigger target, ha ha!",
// Make use of object with another (unsuccessfully)
	"You can't use the clay like that!",
//30
	"That's not the way to use the flask!",
	"You can't use the\nbouillon cubes like that!",
	"Doing that with the cage isn't useful!",
	"Get Hugo to rub the crystal ball!",
	"That's not the way to use it!",
//35
	"The ghost will not let you pass!",
	"Use an elephant?  You're funny!",
	"Get Hugo to read it!",
	"The candle cannot be\nused in such a manner.",
	"Ding dong!\n\nNothing happens...",
//40
	"Shooting your blowpipe\nat that won't help!",
// Added in data1.cpp by Strangerke, to avoid hardcoded strings in background_t variable initialization*/
	"Nobody answers!",
	"It's not locked!",
// Object descriptions, invoked by the words "look" (at object)
	"It is the handsome Hugo!",
	"It is the pretty Penelope!",
//45
	"Poor Penelope doesn't\nlook very well!",
	"It doesn't look too badly damaged.",
	"The waterfall is flowing too\nswiftly for you to cross.",
	"The water looks clean\nenough to drink.",
	"This is the magical pool of life!\nTake some water to Penelope to save her.",
//50
	"The vines look pretty sturdy.",
	"You see nothing unusual\nabout the bullrushes.",
	"Some very narrow stepping\nstones cross the stream.",
	"Not the kind of guy you'd\nwant to meet in a dark alley!",
	"It is the kind used to\nmold shapes with.",
//55
	"They look very sharp!",
	"An empty water flask.",
	"A flask containing\nsome regular water.",
	"A flask containing\nsome magic water.",
	"You see nothing out of\nthe ordinary about him",
//60
	"You see nothing out of\nthe ordinary about her",
	"It is gray and furry\nand rather cute!",
	"It is a small cage suitable\nfor holding small animals.",
	"It is a small cage with a\ncute furry mouse inside.",
	"You can see a blowpipe with some\ndarts full of sleeping potion!",
//65
	"It is big, gray and\nlooks extremely heavy!",
	"It is a box of bouillon cubes\nnormally used as seasoning in\ncooking but often carried for\nemergency rations in aircraft.",
	"You are looking at what appears\nto be the half-eaten remains of\na cheese sandwich.",
	"It is an extremely\nvoracious-looking spider!",
	"The scroll appears\nto have some writing\non it.",
//70
	"The rock itself looks\nlike any other mighty\nboulder you might find\nin these parts.",
	"The crystal ball appears cloudy.",
	"The ghost does not appear\nto be very friendly and\nwill not let you pass.",
	"The bell is small and golden.\nIt makes a pleasant tinkling\nring which you find rather\ncomforting for some reason.",
	"The book is old and very worn.\nThe pages although yellow with\nage, still have discernible\nwriting on them.  You feel\nthe book has hidden powers.",
//75
	"The candle stick is golden and\nvery heavy.  The flickering of\nthe candle feels comforting,\neven in the daylight."
};

//***************************************************************************
// Hugo 1 Dos
//***************************************************************************
const char *textData_1d[] = {
	"",
	"You don't have it",
	"You don't have the key",
	"It is too dark to see\nanything in there",
	"You don't have anything\nto oil it with",
	"You don't have anything\nto cut it with",
	"That wouldn't work",
	"It is already locked",
	"It is already unlocked",
	"It is locked",
// 10
	"It is already closed",
	"It is already smashed",
	"It is already open",
	"There is nothing in there",
	"You are already wearing it\nyou cross-eyed baboon!",
	"You don't have it on,\nI know it's hard to tell!",
	"You'll need to\ntake it off first",
	"You already oiled it,\nwhy don't you just\nopen it???",
	"Despite your best efforts the\nbolt refuses to budge",
	"I believe you already did!",
//20
	"Ok.",
	"The shed seems to have fallen\ninto disuse and is pretty\nmuch full of garbage, either\nbroken or rusted to pieces.\nYou are just on the point of\nleaving when you notice an\noilcan sitting on a shelf",
	"I said it's an oil-CAN,\nnot an oil-LAMP dummy!",
	"The boat is now floating free!",
	"The rope is knotted too tightly\nand you are, alas, unable to\nuntie it despite your best efforts!",
	"This rope is a lot stronger\nthan you and you are, alas,\nunable to break it",
	"The oldman seems somewhat\nsurprised by your attack \nand a little offended.\n\nYou may wish to try a\ndifferent strategy!",
	"It is you, the hero.",
	"Why not try opening it?",
	"I can see no more than\nyou at this moment",
//30
	"They appear to be looking at you!",
	"It's just flapping around up there",
	"There appears to be\nsomething inside it",
	"Looks like a front door key to me",
	"A useful looking candle",
	"A rather useful looking penknife",
	"It is a little silver whistle",
	"It appears to be a halloween mask\nlooking somewhat like a grotesque\nmonkey's head!",
	"I don't think you want to mess with\nthis guy!",
	"A rather yummy looking pork chop",
// 40
	"This little oilcan looks like\nit contains some oil",
	"Believe me, this is no Lassie!",
	"It looks like a serviceable\nboat.  I wonder whether it\nwould get you to the other side?",
	"The rope is to stop\nthe boat drifting away!",
	"Wooo!  A small fortune in gold!",
	"He looks at least\n200 years old!!",
	"This is one dude you\ndon't want to argue with!",
	"He looks totally gone!",
	"He has a certain charm,\nI suppose!",
	"Well, it's sort of\nround and rubbery!",
// 50
	"Nobody answers!",
	".",                            // No song
	".O0L5CL1DL3D#CL6..L5CL1DL3D#CL6G#L1GGFEbEbDL5CL1DL3D#CL6..L5CL1DL3D#CL6O1L1C>B<CC#DD#L3ECO2L1F.FE.CD.DC.>A<L3DL6CL1CDEL1F.FE.CD.DC.>A<Eb..L9D>L1GGAA#A#<L6C>L1AAA#<CCL6DL1>GGGbGGAA#A#AGG<DL9CL1CDEF.FE.CD.DC.>A<L3DL6CL1>AAGFFA<CCD#DDC>BbBbA<DDDL9DL1>GGAA#A#<L3CL1DEEL3FL1C>L3AL6<DL1C.C>B.<CC#.DD#.EL3FGG#AL1BbFDBbFDAFCAFCGECGECAFCAFCBbFDBbFDAFCAFC>CB<CC#DEF>..F.",
	"O3L1E.>E<EF.>F<FE.>E<EF.>F<FE.F.G.A.G.A.B.<C.D.DbC>B.<D.Db.C.E.F.L4EFL6E.",
	"O2L4G<C>GEL6CL2AGEL4GL9D."
};

//***************************************************************************
// Hugo 2 DOS
//***************************************************************************
const char *textData_2d[] = {
	"",
	"You see nothing very\ninteresting about it",
	"I see no purpose\nin doing that!",
	"You find nothing of\nany interest inside!",
	"You don't have it",
	"You don't have\nany matches!",
	"You don't have a weapon!",
	"That wouldn't work",
	"It is already closed",
	"It is already smashed",
// 10
	"It is already open",
	"There is nothing in there",
	"Cousin Harry is too convulsed\nwith laughter to talk!",
	"You can't!  Great Aunt Hester\nwill see you!",
	"You don't have any\nmore quarters!",
	"Click!\nOops, I think you\nemptied it buster!",
	"Ok.",
	"Now it's gone for ever!",
	"You break off a shoot and\nplace it near the cat.\nThe cat awakes from its\nreverie and sniffs your\ngift excitedly.  After 5\nminutes of friendly play\nit falls asleep again,\nrather contentedly!",
	"Why don't you just\ntry reading it?",
// 20
	"Quietly opening it, you quickly\nscan it.  It appears to be a\nletter from Hester's attorney\nconcerning the life insurance\npolicy on Great Uncle Horace.\nFeeling rather uncomfortable\nabout reading other peoples'\nmail, you return it to its\nenvelope without reading any\nfurther.",
	"It won't open!",
	"Just walk through!\nThis door will\nopen automatically!",
	"The room is too dark to\nsee anything, now",
	"Opening the desk drawer reveals:\na book of matches!",
	"Opening the cupboard reveals:\na clove of garlic!",
	"It is the handsome Hugo!",
	"It is the pretty Penelope!",
	"Why not try opening it?",
	"You see a saucy looking french maid!",
//30
	"What a cute doggy!",
	"He looks at least\n200 years old!!",
	"You can see a rather impressive\nbook collection.\nOne yellow book in particular\nseems to stand out",
	"One yellow book seems to be\nsticking out slightly",
	"It is a brightly colored parrot.",
	"It is a brightly colored balloon.",
	"It is a dumb waiter,\nused to carry food\nup and down from\nthe kitchen",
	"The rope looks climbable",
	"Ugh! The garlic smells foul!",
	"The gardener returns your stare!",
// 40
	"You can see 4 colored buttons.\nUnderneath the buttons is some\nlettering.  The lettering is\nvery faded and you can only make\nout the following characters\nunder the green button:\nb*g *a*pe*",
	"They are small but have\nvery vicious looking\nproboscises",
	"It is one of the rarer\nvenomous species:\nbeautiful but very deadly!",
	"The stick of dynamite has\na fuse at one end.",
	"You peer down into the\nmurky depths of the well\nand see only blackness...\nTaking a small pebble, you\nlean over and drop it down.\nAfter a while you hear a\ndull thud...",
	"It looks like an ancient\noil lamp.  It appears to\nbe totally empty now,\nexcept it feels curiously\nheavy.",
	"It looks pretty moldy, I don't\nthink I'd try eating it!",
	"Just your regular genie type!",
	"It's Hugo's affable cousin Harry!",
	"It's Hugo's lovable aunt Hester!",
// 50
	"Yes, there is an envelope\non the table with a letter\ninside it!",
	"This mysterious fellow calls himself\n""the Doctor"".\nWho he is and where he comes from\nI have simply no idea!",
	"It's the cook!",
	"He's a very authoritative\nlooking police officer!",
	"A sprightly looking old man!",
	"It is an evil mechanized monster\nbent on the destruction\nof the universe!",
	"It's the kind of bell\none might use to\nsummon aid.",
	"It looks real, I\nwonder if it works?",
	"Looking through the\nkeyhole, you can\nsee the key has been\nleft in the lock.\nToo bad it's on the\nother side!",
	"It is a magnifying glass\nused (for example) for\nreading small print.",
// 60
	"It is a pretty solid\nlooking safe, I don't\nthink there's any way\nto open it unless you\nknow the combination!",
	"It is an extremely intricate\nlooking device, I'm afraid\nit's workings are quite\nbeyond me!",
	"It has some printing\non it, some of which\nis readable.",
	"It contains several old photos\nof Hugo as a child.  One photo\nin particular catches your\nattention.  It shows Hugo\nwith a young girl of about\nthe same age standing in front\nof a huge pointed tower.\nSomething about the girl strikes\nyou as being very familiar...",
	"It is a clear glass bottle.\nA label is attached with the\nword \"SERUM\" printed on it.",
	"It has a U-shaped fluorescent\nlight tube at its center, with\na series of metal rods spaced\nvertically around it.\nSurrounding the rods is a thick\nmesh-like screen which also acts\nas the frame of the device.",
	"Nobody answers!",
	"It's not locked!",
	"You need to say which\ncolor button to press!",
	".",
// 70
	".O2L1C^EGBACEAGAGFEDEFG^GCGvD^GvG^GvEFEDC.^C.vC^EGBACEAGAGFEDEFG^GCGvD^GvG^GvEFEDC.^C.vCDCvABAG.^CDCvABAG.^C.CvBA.^D.vB^CvBAGvG^ABvC^EGBACEAGAGFEDEFG^GCGvD^GvG^GvEFEDC.^C.",
	".O2L1CEG^C.",
	".O3L1CvGEC.",
	".O1L1AbAAbFAbAAbFAbAAbFAbAAbFAbAAbFAbAAbFAbAAbFAbAAbF.",
	".O2L1^CvGEC^CvGECBGECBGECAGECAGECBGECBGEC^CvGEC^CvGECBGECBGECAGECAGECBF#DvB^BF#DvB^GDvBG^GDvBG^GbDvBG^GbDvBG^EDvBG^EDvBG^GbDvBG^GbDvBG^GDvBG^GDvBG^GbDvBG^GbDvBG^EDvBG^EDvBG^Gb.......^CvGEC^CvGECBGECBGECAGECAGECBGECBGEC^CvGEC^CvGECBGECBGECAGECAGECBF#DvB^BF#DvB^GDvBG^GDvBG^GbDvBG^GbDvBG^EDvBG^EDvBG^GbDvBG^GbDvBG^GDvBG^GDvBG^GbDvBG^GbDvBG^EDvBG^EDvBG^GbDvBG^Gb...",
	".O0L1CC#CC#CC#CC#CC#CC#CC#CC#CC#.",
	".O1L3^CvL1G.GL3G#L1G.....L1B..^C.",
	"O3L1E.vE^EF.vF^FE.vE^EF.vF^FE.F.G.A.G.A.B.^C.D.DbCvB.^D.Db.C.E.F.L4EFL6E.",
	"O2L4G<C>GEL6CL2AGEL4GL9D."
};

//***************************************************************************
// Hugo 3 DOS
//***************************************************************************
const char *textData_3d[] = {
	"",
	"You see nothing very\ninteresting about it",
	"I see no purpose\nin doing that!",
	"You find nothing of\nany interest inside!",
	"Nothing happens.",
	"You don't have it",
	"You don't have a weapon!",
	"You don't have anything\nto stick in it.",
	"There are three things\nrequired to perform an\nexorcism.  You do not\nhave all of them...",
	"That wouldn't work.",
// 10
	"It is already closed.",
	"It is already smashed.",
	"It is already open.",
	"There is nothing in there.",
	"You already have!",
	"It already has some\nwater inside it.",
	"You can see a very slight\ndepression in the earth\nwhere something spherical\nprobably once rested there.",
	"It is already lit.",
	"It is not lit.",
	"Ok.",
// 20
	"Now it's gone for ever!",
	"It won't open!",
	"          LOST!\n\nNear the mighty boulder,\nMy precious crystal ball,\nI guess I couldn't hold 'er,\nSo I must've let her fall!\n\nIf anyone should find it,\nPlease return it to my keep,\nBe careful if you use it,\nFor its powers runneth deep!\n\nsigned:  The old man.",
	"My my, we are thorough,\naren't we?  Well, just\nfor you:\n\nThere is a crystal ball\nlying behind the rock,\nglinting in the sun.\n\nYou fetch it out...",
	"Ding dong!\n\n(Nothing happens...)",
	"The ghost is impressed by\nyour fine acrobatics (but\nstill won't let you pass)!",
	"Nobody answers!",
	"It's not locked!",
	"It is the handsome Hugo!",
	"It is the pretty Penelope!",
// 30
	"It doesn't look too badly damaged.",
	"The vines look pretty sturdy.",
	"Not the kind of guy you'd\nwant to meet in a dark alley!",
	"It is the kind used to\nmold shapes with.",
	"They look very sharp!",
	"You see nothing out of\nthe ordinary about him",
	"You see nothing out of\nthe ordinary about her",
	"It is a flask for\nholding drinking water.",
	"It is gray and furry\nand rather cute!",
	"It is a small cage suitable\nfor holding small animals.",
// 40
	"You can see a blowpipe with\nsome darts full of\nsleeping potion!",
	"It's just sitting there,\nminding its own business!",
	"It is a box of bouillon cubes\nnormally used as seasoning in\ncooking but often carried for\nemergency rations in aircraft.",
	"You are looking at what appears\nto be the half-eaten remains of\na cheese sandwich.",
	"It is an extremely\nvoracious-looking spider!",
	"The scroll appears\nto have some writing\non it.",
	"The crystal ball appears cloudy.",
	"The ghost does not appear\nto be very friendly and\nwill not let you pass.",
	"The bell is small and golden.\nIt makes a pleasant tinkling\nring which you find rather\ncomforting for some reason.",
	"The book is old and very worn.\nThe pages although yellow with\nage, still have discernible\nwriting on them.  You feel\nthe book has hidden powers.",
//50
	"The candle stick is golden and\nvery heavy.  The flickering of\nthe candle feels comforting,\neven in the daylight.",
//Added by Strangerke to improve mouse support
	"Poor Penelope doesn't\nlook very well!",
//
	".",
	".O4L1C.C.C.",
	".O1L2ECEG^C.vBL6^CvGAL2G.FL6E.",
	".O2L1CEG^C.",
	".O3L1CvGEC.",
	".O1L1AbAAbFAbAAbFAbAAbFAbAAbFAbAAbFAbAAbFAbAAbFAbAAbF.",
	".O0L1CC#CC#CC#CC#CC#CC#CC#CC#CC#.",
	".O1L3^CvL1G.GL3G#L1G.....L1B..^C.",
// 60
	"O3L1E.vE^EF.vF^FE.vE^EF.vF^FE.F.G.A.G.A.B.^C.D.DbCvB.^D.Db.C.E.F.L4EFL6E.",
	"O2L4G^CvGEL6CL2AGEL4GL9D.",
	".O2L1AAbAA#BAL2G^CvGL1^CvC.",
	".O2L1DGB^D.vB^L5D.",
	".O3L0F.FC.CvA..ABb^F.DvBb.^C.CvA.AF..FG^E.vBbG.F.FA.A^C.vBb.Bb^D.DF.C.CE.EG....CEGBbGE.FCvAF...vFCvAF.",
	".O2L0EFF#GGbFE."
};

// Added by Strangerke
const char *string_t_Data_1w[] = {""};
const char *string_t_Data_2w[] = {""};
const char *string_t_Data_3w[] = {""};
const char *string_t_Data_2d[] = {""};
const char *string_t_Data_3d[] = {""};

//***************************************************************************
// Hugo 1 Dos - Only Hugo 1 DOS doesn't use a DAT file to store those strings
//***************************************************************************
const char *string_t_Data_1d[] = {
	"",
	"There is nothing in there",
	"Ok.",
	"The pumpkin breaks open to reveal:\n\nA key!",
	"There is a little penknife lying\nin the dust on the floor",
	"There is a little whistle lying\nin the dust on the floor",
	"I don't see anything much\nin here... Wait! there\nis a little penknife lying\nin the dust on the floor\nand next to it is a little\nsilver whistle",
	"Now it's gone for ever!",
	"Hmm... that seems to have had\na strange effect on the bats!",
	"Hey, what do you know!!!\nYou appear to have confused\nthe bats' sense of direction.\nIt must be something to do\nwith their sonar-like hearing!",
	"Care for a chop, sir?",          // Prompt
	"yes",                            // Response
	"This door appears to be locked\nwith a combination lock.\nWhat's the combination?",
	"333",   // 333
	"You appear to be having a\nlittle difficulty here...\n\nWould you like some help?",
	"You still appear to be having\na little difficulty here...\n\nWould you like some help?",
	"What was the first name of\nthe hero in 'The Hobbit'?",
	"Where did Aslan live?\n(Hint: Not in a wardrobe!)",
	"Who invented Count Dracula?",
	"What should you do with a\nPan-galactic gargle blaster?\n(a) Ride it\n(b) Fire it\n(c) Drink it\n(d) Run away from it",
	"And now a riddle:\nWhat's the name of the only\nmammal that can't fly\nthat can fly?",
	"Nearly there, just 2 more to go!\n\nWhat was the name of Roy\nRogers's dog?",
	"And lastly:\nAre you sure you want\nto rescue Penelope??",
	"'Z^]c",          // "bilbo"
	"3Rdi]L",         // "narnia"
	"cSh4^]Ya'<",     // "bram stoker"
	"(",              // "c",
	"2R`",            // "man"
	"'f^gY_",         // "bullet"
	"Try walking there!",
	"Nothing happens...",
	"I see no purpose\nin doing that!",
	"I'm afraid I have no idea!",
	"You see nothing very\ninteresting about it",
	"You find nothing of\nany interest inside!",
	"Seeing how ferocious he looks,\nyou may well be better off\nthrowing it!!",
	"I love your sense of humor!",
	"Since you can't lift it\nI don't see how you can\nlook under it!",
	"No jumping allowed in this game!",
	"You try but fail miserably!",
	"You ought to be aware that\ntalking to oneself is\nthe first sign of madness!",
	"You know you really should\ncurb those violent tendencies!",
	"You should try to be\nmore constructive!",
	"All is quiet!",
	"It has an indistinct aroma!",
	"Why, is it falling?",
	"Try pressing the F1 key!",
	"An oak tree, I think",
	"It's a pretty boring fence",
	"It sure is a spooky looking house.\nI think you should go inside!",
	"I can see no more than you can!",
	"You'll go blind!",
	"Yeuch!  Raw pumpkin?  No thanks!",
	"Nice try!\n\nNothing under there, though!",
	"You are in front of the house\nwhere Penelope was last seen.\n\nIf you are to rescue her, you\nmust find a way inside, no\nmatter what lies ahead...!",
	"I'd rather not,\nit's too scary!",
	"An exquisitely bad work of art",
	"It is a small round wooden\ntable, useful for putting\nthings on, like candles",
	"There are some rooms upstairs",
	"It is a candle",
	"You are in the hall.  It sounds as\nif a big feast or something is\ngoing on in the room on the right.\nThere is a flight of stairs to the\nleft.  So far your presence has\ngone undetected!",
	"You can hear the sounds of a\nfeast in the room to the right",
	"You get down on your hands and\nknees and peer under the bed to\nfind... absolutely nothing!",
	"This is not the time\nfor a snooze!!",
	"It certainly looks like a\nwardrobe, wonder what's inside!",
	"It looks more like a wardrobe!",
	"It's pretty dark out there,\nyou can just make out the\noutline of a shed below,\nin amongst some trees",
	"I'd describe it more like a mask",
	"You are in one of the upstairs\nbedrooms.  There doesn't appear\nto be much of any interest at\nall in here.",
	"That's some kind of spread.  I don't\nknow whether I'd want to taste any\nof it, though!",
	"I'd stay out of his way\nif I were you!",
	"It's pretty, but useless!",
	"It is Zelda, wicked witch\nof the west!",
	"Apart from the moon,\nthere's not much to see",
	"It doesn't look very tempting\nbesides, there's very little left",
	"Just your regular old monster type",
	"But what would Penelope say?",
	"A feast is in full swing here in\nthe dining room.  Some of the\nguests look rather ugly!  Something\ntells me this is not a good place\nto be unless, of course, you want\nto end up on the dinner plate!",
	"But you're not invited!",
	"I think you should\nwait for the butler to\noffer you something!",
	"These guys are only\ninterested in eating!",
	"Get lost, dweeb!",
	"The butler is too busy\nto engage in chit-chat",
	"Please control yourself until\nyou rescue Penelope!!",
	"All you can hear are various\nslurping and chomping sounds",
	"Hugo says he's not thirsty!",
	"There appears to be something\ndaubed on the mirror in red.\nIt looks like the number \"333\"",
	"I'd rather not, thanks",
	"You have arrived at the bathroom.",
	"Yeugh! How uncouth!",
	"You don't have\ntime for that!!",
	"Look, are you taking this\ngame seriously, or what?",
	"Peering inside, you find nothing\nto write home about!",
	"It's pretty dark out there,\nyou can just make out the\noutline of a shed, in\namongst some trees",
	"A pretty ordinary\nlooking broom",
	"Gonna do some sweeping\nare we?\nHey! the broom must\nbe gripped by some\nmagical powers! Despite\nyour best efforts you\ncan't budge it!!",
	"Nothing happens...\n(You look pretty silly, too!)",
	"From the looks of it, this\nis the kitchen.  You can hear\nthe sounds of a feast or some-\nthing going on in the room to\nthe right.",
	"They have a housemaid\nto do that!!",
	"It probably warrants further\ninvestigation inside!",
	"You shinny up the tree,\nfind lots of branches\nand leaves and stuff,\nand shinny down again.\nGood excercise, huh?",
	"Everything in the garden\nis as it seems!",
	"You are behind the house, now,\nin what appears to be a small\nfenced in yard.  To the rear\nof the garden is a little shed",
	"Please say 'undo bolt'",
	"You rummage around in the\nhole and feel something soft\nand slightly moist.\nPhew!  A medium sized pile of\nmouse droppings!!",
	"For goodness sakes!!\nGive me a break!",
	"This is a pretty empty looking\nroom adjoining the kitchen.\nThe only thing of any interest\nthat immediately catches your\neye is a rather large and ugly\nlooking dog!",
	"Looking through the door,\nyou can make out your\nbeloved Penelope's tear-\nstreaked face!",
	"Nice try!\n\nToo bad all you end up with\nis an oily door that still\nwon't budge!",
	"Grunt!  Grunt!  Heave!  Phew!!\nNope!  This door won't budge!",
	"You examine every rock\nin minute detail and\ncome up with: Zilch!",
	"This door is impregnable!\nTry as you might, it\nis impervious to\nyour attack!",
	"Are you kidding?  Well,\nI suppose you had to\ntry!  Needless to say,\nthe door is locked!",
	"Well, for a start there's\nno keyhole and secondly\nno bolts either!",
	"Nobody answers... (Surprised?)",
	"Penelope, alas, can't speak\nsince she is gagged!",
	"All you can hear is\nsome muffled sobbing!",
	"You are below the house now.  The\nwalls appear to be partly hewn out\nof rock.  To the right of the\nbasement is a large, extremely\nheavy looking door.  You can hear\nthe muffled sounds of someone\nsobbing!",
	"You are in a cave full\nof bats.",
	"Seen one rock, seen 'em all!",
	"You are in a cave\nwith the mummy's tomb!",
	"The tomb looks very unusual,\nbut then you don't see one\nevery day, do you?!",
	"You are now in a large open cavern.\nThere is a small underground lake\nhere, at the far side of which is\nsome sort of jetty and a very old\nlooking man who appears to be just\nsitting on the jetty, fishing.\n\nTo the far right, at the back of\nthe cavern, you can see a tunnel!",
	"You'd be a lot better off\nin the boat, my friend!",
	"You must be more specific.",
	"You have arrived in a passage\nwith a room at the end.  There\nis a large guard at the end who\nappears to be standing outside\na kind of jail.  Wait!  Behind\nthe guard you can make out a\nfamiliar shape!  Yes!  It's\nPenelope, being held prisoner!",
	"He doesn't understand you!",
	"You have arrived in the\nmad-professor's laboratory!\nThere is a strange looking\nbox connected to some weird\nmachine with flashing lights.\n\nIn the left-hand corner of\nthe room is a little table\nwith an assortment of odd-\nshaped items on it.",
	"Igor is good at taking orders\nbut is not much of a\nconversationalist!",
	"'Look, just do as you're told,\nok ?'",
	"You see lots of buttons\nand dials and flashing\nlights!\n\nPlease don't ask me to\ntry and figure it out!",
	"I think you'd better\nleave that to Igor!",
	"The only thing you recognize\nis a useful looking rubber\nbung!",
	"The door to the professor's\nbox can only be opened\nand closed by the machine!",
	"Lots of flashing lights!",
	"I wonder what the whistle is for?",
	"Nothing seems to happen...\nHey, maybe it's one of those\ndog whistles that only dogs\ncan hear!!",
	"Nothing seems to happen",
	"The high frequency cosmic\nradiation emanating from\nthis room knocks your mask\nonto the floor!\n(You can pick it up again\nwhen you come out!)",
	"Very good sir, <sniff>",
	"'Ere! just a moment! You're\nnot one of us, you're a bloomin'\ninterloper!  Come 'ere you\nlittle blighter, I'm going to\n chop yer 'ead off!!",
	"It appears your game is up, so to\nspeak.  The butler deftly slices\nyour head off with a handy carving\nknife.  So much for rescuing\nPenelope!",
	"Very good sir, <sniff>, enjoy!",
	"The butler hands you a\njuicy looking chop.\nYou take it from him\nwith a polite 'Thank you'",
	"Very good sir, <sniff>,\nperhaps a little later.",
	"Mmmm!  You feel\nsomewhat invigorated!",
	"That seems to have caught\nthe dog's attention!  It\nlooks like he's going to\nbe rather busy for a while!",
	"I'm sorry, that combination\ndoesn't seem to work!",
	"Bingo! The door opens...",
	"Oh dear, it seems the\nnice doggy has eaten\nyou all up!\n\nYou'll never get to rescue\nPenelope this way!",
	"You casually throw the mask away,\nsince you won't be needing\nit any more!",
	"The trapdoor appears to be\nbolted shut!",
	"Well, the answer to this puzzle\nis extremely simple.  In fact,\nyou don't have to type anything!",
	"Oh very well then...\n\nGood luck!",
	"OK, here's the scoop...\nWalk between the rocks!",
	"Gotcher!!",
	"Oh dear, you seem to have\nwandered a bit too close\nto one of the vampire bats!\n\nLooks like it's curtains\nfor you, now you'll never\nbe able to rescue Penelope!",
	"Oh how careless!  You seem\nto have landed yourself in\nthe clutches of the evil mummy!\n\nWell, since it's curtains\nfor you, you'll never be able\nto rescue your sweetheart!",
	"I'm afraid the boat has\na hole in the bottom!\nUnless you can plug it\nwith something, this\nboat will surely sink\nbefore you get to the\nother shore!",
	"The old man is blocking\nyour path!",
	"Shutup and enjoy the ride, buster!",
	"The boat wobbles a bit\nbut doesn't really go\nanywhere due to the fact\nthat it is still tied to\nthe post!",
	"Correct! And the next\nquestion is:",
	"I'm sorry, that is incorrect",
	"Since you have failed to\nanswer my questions I\nhereby doom you to float\nforever on my lake!",
	"Well in that case, I can hardly\nlet you pass can I?  You do\nunderstand don't you?  In all\ngood conscience I feel I must\nnow doom you to float forever\nhere on this lake of mine!!!",
	"Wonderful!  Thou art truly a\nnoble and wise adventurer!!\n\nGo in peace, my friend and\ngood luck in thy mission!",
	"The old man seems about to speak...",
	"Ah!  Welcome to my lake, my fine\nyoung friend.  I have been waiting\nfor you!  I am well aware of your\nquest and I would hasten you on your\nway.  However, before I let you pass\nI must satisfy myself that you have\nthe experience to handle the dangers\nthat lurk through yonder passage...",
	"To this end, you will permit me\nto test your mettle with a few\nquestions the answers to which\nwould come readily to the lips\nof any seasoned adventurer.\n\nBe warned, however that I can\nonly accept your first answer!",
	"The old man clears his throat\nand asks:",
	"The old man has nothing\nmore to say to you!",
	"Congratulations!!",
	"You are so glad to have rescued\nPenelope, you dance for joy!",
	"After cutting her ropes with\nyour penknife, you open the\nbolts of the jail door and find\nyourself back in the basement!",
	"From here you trip hand in hand\nup the basement stairs, past the\nferocious doggy, through the\nkitchen and out the front door\nto freedom!\n\n(And live happily ever after!)",
	"You hand over one coin from\nyour little bag of golden coins!",
	"He makes a gruff noise which\nyou assume was a 'Thank you'\nand steps aside allowing you\nto pass!\n\nNearly there, Hugo!",
	"Your generous gift is refused!",
	"Goodbye!",
	"Too bad!  In your current state\nyou can't grip the door handle\nto open it!",
	"Too bad!  In your current state\nyou can't coordinate your hand\nto turn the door handle!",
	"Too bad!  In your current state\nyou can't reach the door handle!",
	"The mad professor speaks:\n\nAh!  There you are!\nI've been looking all over\nthe house for you!  Look,\nyou're late and we haven't\nmuch time!  Step into the\nbox and we'll begin the\nexperiment!",
	"The professor beckons you\nto step into the large cubicle\nin the center of the room",
	"'Good!', says the professor.\n'Ok, Igor, press the BLUE button!'",
	"Igor grumbles something\nincoherent and deftly\npresses the RED button!",
	"'You idiot, Igor!!', roars the\nprofessor.  'That's all I need, a\ncolor-blind imbecile for an\nassistant!'\n\nOh dear, I've got my headache\ncoming on again!  I've had\nenough hassle for one day, I'm\ngoing to have a lie down!'",
	"The professor storms off, leaving\nyou alone with Igor...",
	"Igor is fed up pressing buttons\nand refuses!!\n\nI think you should go while you\ncan!!",
	"Igor deftly presses\nthe BLUE button!",
	"Igor deftly presses\nthe GREEN button!",
	"Igor deftly presses\nthe YELLOW button!",
	"Igor refuses!",
	"But you're not in\nthe box, master!",
	"Nobody answers!",
	"We are getting desperate,\naren't we!",
	"No digging allowed!",
	"Same to you, loser!",
	"Nobody answers...!"
};

// Nouns and synonyms (N.B. put null string at end of last synonym)
// For ambiguous substrings, e.g.lock, unlock, put longest first.
// For objects, the first name is used as the file name
// The second is used when hero types "look"
// The third is used with the mouse pointer

// Ambiguous ones (multi-version)
const char *nBell_2w[]       = {"bell", "a bell", "bell", ""};
const char *nBell_3w[]       = {"bell", "a golden bell", "bell", ""};
const char *nBlock_3w[]      = {"block", "~"};
const char *nBook_2w[]       = {"book", "book", "book", ""};
const char *nBook_3w[]       = {"book", "a spell book", "book", "spell", ""};
const char *nCage_3w[]       = {"cage", "a little cage", "cage", "door", ""};
const char *nCandle_1w[]     = {"candle", "a candle", "candle", ""};
const char *nCandle_3w[]     = {"candle", "a golden candle", "candle", ""};
const char *nCupb_1w[]       = {"cupboard", "cabinet", "cupboard", "dresser", "hutch", "sideboard", "bureau", ""};
const char *nCupb_2w[]       = {"cupboard", "cabinet", "cupboard", "hutch", "dresser", "sideboard", "bureau", "desk", "drawer", ""};
const char *nDoctor_2w[]     = {"doctor", "man", "Strange man", ""};
const char *nDoctor_3w[]     = {"doctor", "witch", "Witch Doctor", "native", "man", ""};
const char *nDog_1w[]        = {"dog", "~", "dog", ""};
const char *nDog_2w[]        = {"dog", "dog", "dog", ""};
const char *nDoor_1w[]       = {"door", "~", "door", ""};
const char *nDoor_2w[]       = {"door", "gate", "door", ""};
const char *nDoor_3w[]       = {"door", "~", ""};
const char *nHero_1w[]       = {"hero", "self", "Hugo", "hugo", ""};
const char *nHero_2w[]       = {"hero", "self", "Hugo", ""};
const char *nKey_1w[]        = {"xkey", "a door key", " key", ""};
const char *nKey_2w[]        = {"key", "key", "key", ""};
const char *nKnife_1w[]      = {"knife", "a penknife", "penknife", ""};
const char *nKnife_2w[]      = {"knife", "knife", "knife", ""};
const char *nLips_1w[]       = {"lips", "~", ""};
const char *nOldman_1w[]     = {"oldman", "old", "Old man", "man", ""};
const char *nOldman_2w[]     = {"oldman", "old", "man", ""};
const char *nPenny_2w[]      = {"penelope", "woman", "Penelope", "girl", "lady", ""};
const char *nPennylie_2w[]   = {"pennylie", "woman", "Penelope", ""};
const char *nRope_1w[]       = {"rope", "some rope", "rope", "string", ""};
const char *nRope_2w[]       = {"rope", "string", "rope", ""};
const char *nShed_1w[]       = {"shed", "~", "shed", ""};
const char *nShed_2w[]       = {"shed", "~", ""};
const char *nSnake_2w[]      = {"snake", "snake", "snake", ""};
const char *nSnake_3w[]      = {"snake", "~", ""};
const char *nTrap_1w[]       = {"trap", "~", "trapdoor", ""};
const char *nTrap_2w[]       = {"trap", "~", ""};
const char *nWard_1w[]       = {"wardrobe", "cabinet", "closet", "door", ""};
const char *nWard_2w[]       = {"wardrobe", "cabinet", "closet", ""};
const char *nWhero_1w[]      = {"whero", "~", "Hugo", ""};

const char *nBed_1w[]        = {"bed", "~", "bed", ""};
const char *nBed_2w[]        = {"bed", "bed", "bed", ""};
const char *nBlock_2w[]      = {"block", "brick", "blocks", ""};
const char *nBridge_2w[]     = {"bridge", "~", "bridge", ""};
const char *nBridge_3w[]     = {"bridge", "~", ""};
const char *nBroom_1w[]      = {"broom", "~", "broom", ""};
const char *nBroom_2w[]      = {"broom", "brush", ""};
const char *nCage_2w[]       = {"cage", "pen", "playpen", ""};
const char *nDroppings_1w[]  = {"droppings", "~", ""};
const char *nFence_1w[]      = {"fence", "~", "fence", ""};
const char *nFence_2w[]      = {"fence", "hedge", "bush", ""};
const char *nFood_1w[]       = {"food", "plate", "food", ""};
const char *nFood_2w[]       = {"food", "plate", ""};
const char *nFood_3w[]       = {"food", "meat", "hyena", ""};
const char *nGardenbits_1w[] = {"grass", "fence", "Branch", "path", ""};
const char *nGround_1w[]     = {"ground", "floor", ""};
const char *nHouse_1w[]      = {"house", "~", ""};
const char *nLight_1w[]      = {"light", "lamp", ""};
const char *nMan_1w[]        = {"man", "dracula", "monster", "frank", "alien", "guest", "vampire", ""};
const char *nMan_2w[]        = {"man", "~", ""};
const char *nMirror_1w[]     = {"mirror", "~", "mirror", ""};
const char *nMoon_1w[]       = {"moon", "~", ""};
const char *nMouse_2w[]      = {"mouse", "rat", "rodent", "animal", "furry", ""};
const char *nMouse_3w[]      = {"mouse", "rat", "mouse", "rodent", "animal", "furry", ""};
const char *nMousehole_1w[]  = {"hole", "~", "mousehole", ""};
const char *nMousehole_2w[]  = {"hole", "~", "mouse hole", ""};
const char *nPicture_1w[]    = {"picture", "painting", "picture", ""};
const char *nPlant_1w[]      = {"plant", "flower", "pot plant", "vase", ""};
const char *nPlant_2w[]      = {"plant", "flower", "plant", "vase", ""};
const char *nRock_1w[]       = {"rock", "boulder", "rock", "stone", ""};
const char *nRock_2w[]       = {"rock", "boulder", "rockfall", "stone", ""};
const char *nRock_3w[]       = {"rock", "stone", "boulder", ""};
const char *nRoof_1w[]       = {"roof", "chimney", ""};
const char *nSink_1w[]       = {"sink", "~", ""};
const char *nSky_1w[]        = {"sky", "ceiling", ""};
const char *nStairs_1w[]     = {"stairs", "landing", ""};
const char *nStairs_2w[]     = {"stair", "landing", "ladder", ""};
const char *nTable_1w[]      = {"table", "~", "table", ""};
const char *nTable_2w[]      = {"table", "bench", ""};
const char *nTree_1w[]       = {"tree", "~", "tree", ""};
const char *nTree_2w[]       = {"tree", "~", ""};
const char *nUnits_1w[]      = {"Unit", "oven", "cupboard", "sink", "cupb", "drawer", "pantry", "cabinet", "range", "counter", ""};
const char *nUnits_2w[]      = {"Unit", "oven", "sink", "cupb", "drawer", "pantry", "cabinet", "range", "counter", "cooker", ""};
const char *nWall_1w[]       = {"wall", "~", ""};
const char *nWater_2w[]      = {"water", "~", ""};
const char *nWater_3w[]      = {"water", "stream", "river", "pool", "lake", ""};
const char *nWindow_1w[]     = {"window", "outside", "window", "inside", ""};
const char *nWindow_3w[]     = {"window", "~", "window", ""};
const char *nWoman_1w[]      = {"woman", "girl", "lady", ""};

//Unique ones
//***************************************************************************
// Hugo 1 Windows
//***************************************************************************
const char *nDummy[]          = {"",""};
const char *nBat_1w[]         = {"bat", "~", "bat", ""};
const char *nEyes_1w[]        = {"eyes", "~", "eyes", ""};
const char *nBatpic_1w[]      = {"bat", "~", "picture", ""};
const char *nPkin_1w[]        = {"pumpkin", "a pumpkin", "pumpkin", "punkin", ""};
const char *nWhistle_1w[]     = {"whistle", "a whistle", "whistle", ""};
const char *nWdoorl_1w[]      = {"wdoorl", "~", ""};
const char *nWdoorr_1w[]      = {"wdoorr", "~", ""};
const char *nMask_1w[]        = {"mask", "a mask", "mask", ""};
const char *nButler_1w[]      = {"butler", "waiter", "butler", "servant", ""};
const char *nChop_1w[]        = {"chop", "a juicy chop", "chop", "meat", "steak", ""};
const char *nRedeyes_1w[]     = {"redeyes", "~", ""};
const char *nArm_1w[]         = {"arm", "~", ""};
const char *nHdlshero_1w[]    = {"hdlshero", "~", "Hugo", ""};
const char *nMonkey_1w[]      = {"monkee", "~", "Hugo", ""};
const char *nCarpet_1w[]      = {"carpet", "mat", "rug", ""};
const char *nBolt_1w[]        = {"bolt", "~", "bolt", ""};
const char *nHerodead_1w[]    = {"herodead", "~", "Hugo", ""};
const char *nOilcan_1w[]      = {" oil", "an oilcan", "oilcan", ""};
const char *nMummy_1w[]       = {"mummy", "monster", "Mummy", ""};
const char *nMdoor_1w[]       = {"mdoor", "~", "tomb", ""};
const char *nGold_1w[]        = {"gold", "a bag of gold", "gold", "money", "treasure", "bag", "jewels", ""};
const char *nBoat_1w[]        = {"boat", "~", "boat", ""};
const char *nGuard_1w[]       = {"guard", "man", "Guard", ""};
const char *nProf_1w[]        = {"prof", "~", "Professor", ""};
const char *nIgor_1w[]        = {"igor", "man", "Igor", ""};
const char *nBung_1w[]        = {"bung", "a rubber bung", "bung", ""};
const char *nGdoor_1w[]       = {"glasdoor", "~", "door", ""};
const char *nSpachero_1w[]    = {"spachero", "~", "Hugo", ""};
const char *nFuzyhero_1w[]    = {"fuzyhero", "~", "Hugo", ""};
const char *nSpark_1w[]       = {"arc", "~", ""};
const char *nFrank_1w[]       = {"frank", "monster", "Frankie", "man", ""};
const char *nDracula_1w[]     = {"drac", "monster", "Dracula", "man", ""};
const char *nGwen_1w[]        = {"gwen", "monster", "Gwendolin", "lady", ""};
const char *nHood_1w[]        = {"hood", "friar", "Hood", "monster", "man", ""};
const char *nSlime_1w[]       = {"slime", "monster", "Slime", "man", ""};
const char *nPeahead_1w[]     = {"pea", "monster", "Pea Head", "man", ""};
const char *nFace_1w[]        = {"face", "head", ""};
const char *nWitch_1w[]       = {"witch", "~", ""};
const char *nToilet_1w[]      = {"toilet", "~", "toilet", ""};
const char *nBath_1w[]        = {"bath", "~", "tub", ""};
const char *nPenelope_1w[]    = {"penelope", "~", ""};
const char *nTomb_1w[]        = {"tomb", "coffin", "tomb", "box", "sarcop", ""};
const char *nBooth_1w[]       = {"booth", "box", "cubicle", ""};
const char *nMachinebits_1w[] = {"machine", "control", "button", "dial", "knob", "computer", ""};
const char *nMachine_1w[]     = {"machine", "~", "machine", ""};
const char *nHands_1w[]       = {"hands", "~", ""};
const char *nCut_1w[]         = {"cut", "~", ""};
const char *nOil_1w[]         = {"oil ", "~", ""};

const char **arrayNouns_1w[] = {
	nDummy,      nHero_1w,       nTrap_1w,      nWard_1w,        nDoor_1w,
	nBat_1w,     nEyes_1w,       nBatpic_1w,    nPkin_1w,        nCandle_1w,
	nRope_1w,    nCupb_1w,       nKnife_1w,     nWhistle_1w,     nWdoorl_1w,
	nWdoorr_1w,  nMask_1w,       nButler_1w,    nChop_1w,        nRedeyes_1w,
	nLips_1w,    nArm_1w,        nHdlshero_1w,  nMonkey_1w,      nKey_1w,
	nShed_1w,    nDog_1w,        nCarpet_1w,    nBolt_1w,        nHerodead_1w,
	nOilcan_1w,  nMummy_1w,      nMdoor_1w,     nGold_1w,        nBoat_1w,
	nOldman_1w,  nWhero_1w,      nGuard_1w,     nProf_1w,        nIgor_1w,
	nBung_1w,    nGdoor_1w,      nSpachero_1w,  nFuzyhero_1w,    nSpark_1w,
	nFrank_1w,   nDracula_1w,    nGwen_1w,      nHood_1w,        nSlime_1w,
	nPeahead_1w, nSky_1w,        nWall_1w,      nGround_1w,      nTree_1w,
	nFence_1w,   nHouse_1w,      nRoof_1w,      nLight_1w,       nMoon_1w,
	nPicture_1w, nTable_1w,      nStairs_1w,    nBed_1w,         nFace_1w,
	nPlant_1w,   nWitch_1w,      nFood_1w,      nWoman_1w,       nMan_1w,
	nMirror_1w,  nToilet_1w,     nBath_1w,      nSink_1w,        nUnits_1w,
	nBroom_1w,   nGardenbits_1w, nMousehole_1w, nPenelope_1w,    nRock_1w,
	nTomb_1w,    nBooth_1w,      nDroppings_1w, nMachinebits_1w, nMachine_1w,
	nHands_1w,   nWindow_1w,     nCut_1w,       nOil_1w
};

//***************************************************************************
// Hugo 2 Windows
//***************************************************************************
const char *nPenfall_2w[]   = {"penfall", "woman", "Penelope", ""};
const char *nSmoke_2w[]     = {"smoke", "~", "~", ""};
const char *nMaid_2w[]      = {"maid", "waitress", "maid", "woman", "girl", "lady", ""};
const char *nHallgo_2w[]    = {"hallgo", "~", "~", ""};
const char *nBookcase_2w[]  = {"bookcase", "bookcase", "bookcase", ""};
const char *nKeyhole_2w[]   = {"hole", "hole", "keyhole", ""};
const char *nPanel_2w[]     = {"panel", "~", ""};
const char *nMatches_2w[]   = {"matches", "a box of matches", "matches", "match", ""};
const char *nCrate_2w[]     = {"crate", "box", "box", ""};
const char *nDumb_2w[]      = {"dumb", "waiter", "dumb waiter", "box", ""};
const char *nMurder_2w[]    = {"murder", "~", "~", ""};
const char *nGardner_2w[]   = {"gard", "man", "gardener", ""};
const char *nGate_2w[]      = {"gate", "~", "gates", ""};
const char *nRed_2w[]       = {"red", "red", "red button", ""};
const char *nYellow_2w[]    = {"yellow", "yellow", "yellow button", ""};
const char *nGreen_2w[]     = {"green", "green", "green button", ""};
const char *nBlue_2w[]      = {"blue", "blue", "blue button", ""};
const char *nFly_2w[]       = {"fly", "~", "fly", ""};
const char *nLeaf_2w[]      = {"leaf", "~", "venus fly trap", ""};
const char *nGarlic_2w[]    = {"garlic", "some garlic", "garlic", "clove", ""};
const char *nButton_2w[]    = {"button", "switch", "button", "knob", ""};
const char *nShedlight_2w[] = {"slight", "~", "light bulb", ""};
const char *nGatelight_2w[] = {"glight", "~", "lantern", ""};
const char *nZapper_2w[]    = {"zapper", "lamp", "bug zapper", "light", "lantern", ""};
const char *nBug_2w[]       = {"bug", "insect", "bee", "wasp", ""};
const char *nStick_2w[]     = {"stick", "a pile of sticks", "stick", "twig", ""};
const char *nDynamite_2w[]  = {"dynamite", "a stick of dynamite", "dynamite", "fuze", "fuse", ""};
const char *nKennel_2w[]    = {"kennel", "house", "dog house", ""};
const char *nWell_2w[]      = {"well", "rope", "well", ""};
const char *nBanana_2w[]    = {"banana", "a moldy banana", "banana", ""};
const char *nLamp_2w[]      = {"lamp", "an old dusty oil lamp", "oil lamp", ""};
const char *nGenie_2w[]     = {"genie", "man", "Genie", ""};
const char *nTardis_2w[]    = {"tardis", "phone", "phone booth", ""};
const char *nHarry_2w[]     = {"harry", "man", "Harry", ""};
const char *nHester_2w[]    = {"hester", "woman", "Hester", "girl", "lady", ""};
const char *nLetter_2w[]    = {"letter", "envelope", "letter", "card", ""};
const char *nRobot_2w[]     = {"alek", "monster", "robot", ""};
const char *nCook_2w[]      = {"cook", "woman", "cook", "girl", "lady", ""};
const char *nCookb_2w[]     = {"cookb", "~", ""};
const char *nCop_2w[]       = {"cop", "man", "Officer Higgins", ""};
const char *nHorace_2w[]    = {"horace", "man", "Horace", ""};
const char *nCatnip_2w[]    = {"catnip", "some catnip", "catnip", ""};
const char *nCat_2w[]       = {"cat", "puss", "cat", ""};
const char *nGun_2w[]       = {"gun", "a gun", "pistol", "revolver", "weapon", ""};
const char *nPaper_2w[]     = {"paper", "a newspaper", "newspaper", "notepad", " pad", "blotter", ""};
const char *nPencil_2w[]    = {"pencil", "a pencil", "pencil", " pen", ""};
const char *nMagnify_2w[]   = {"magnifier", "a magnifying glass", "magnifying glass", "magnify", "glass", ""};
const char *nSafe_2w[]      = {"safe", "safe", "safe", ""};
const char *nScrew_2w[]     = {"screwdriver", "sonic", "screwdriver", ""};
const char *nWill_2w[]      = {"will", "print", "Horace's will", ""};
const char *nAlbum_2w[]     = {"album", "photo", "album", ""};
const char *nBottle_2w[]    = {"bottle", "a bottle", "serum", ""};
const char *nBalloon_2w[]   = {"balloon", "balloon", "balloon", ""};

const char *nBird_2w[]      = {"bird", "budgie", "parrot", ""};
const char *nPhone_2w[]     = {"phone", "phone", "phone", ""};
const char *nBlotpad_2w[]   = {"blotter", "~", "blotter", ""};
const char *nDrawer_2w[]    = {"drawer", "~", "drawer", ""};
const char *nChair_2w[]     = {"chair", "seat", "couch", "settee", ""};
const char *nTools_2w[]     = {"tools", "hammer", "tools", "pliers", "screw", "knife", "saw", ""};
const char *nBucket_2w[]    = {"bucket", "container", ""};
const char *nWand_2w[]      = {"wand", "~", ""};
const char *nHole_2w[]      = {"hole", "~", "chasm", ""};
const char *nHandle_2w[]    = {"handle", "lever", ""};
const char *nChute_2w[]     = {"chute", "~", "chute", ""};
const char *nOrgan_2w[]     = {"organ", "piano", "organ", "pipe", ""};
const char *nPost_2w[]      = {"post", "pole", "scratching post", ""};
const char *nGraf_2w[]      = {"graffiti", "graf", "graffiti", "writing", ""};
const char *nSwitch_2w[]    = {"switch", "~", "switch", ""};

const char **arrayNouns_2w[] = {
	nDummy,        nHero_2w,     nPenny_2w,     nPennylie_2w,   nPenfall_2w,
	nSmoke_2w,     nLips_1w,     nMaid_2w,      nHallgo_2w,     nBookcase_2w,
	nBook_2w,      nKeyhole_2w,  nPanel_2w,     nMatches_2w,    nCrate_2w,
	nDumb_2w,      nMurder_2w,   nGardner_2w,   nTrap_2w,       nWard_2w,
	nDoor_2w,      nGate_2w,     nRope_2w,      nRed_2w,        nYellow_2w,
	nGreen_2w,     nBlue_2w,     nFly_2w,       nLeaf_2w,       nCupb_2w,
	nGarlic_2w,    nButton_2w,   nShedlight_2w, nGatelight_2w,  nZapper_2w,
	nBug_2w,       nKnife_2w,    nShed_2w,      nOldman_2w,     nSnake_2w,
	nStick_2w,     nDynamite_2w, nKennel_2w,    nDog_2w,        nWell_2w,
	nBanana_2w,    nLamp_2w,     nGenie_2w,     nTardis_2w,     nHarry_2w,
	nHester_2w,    nLetter_2w,   nDoctor_2w,    nRobot_2w,      nCook_2w,
	nCookb_2w,     nCop_2w,      nHorace_2w,    nBell_2w,       nCatnip_2w,
	nCat_2w,       nGun_2w,      nPaper_2w,     nPencil_2w,     nKey_2w,
	nMagnify_2w,   nSafe_2w,     nScrew_2w,     nWill_2w,       nAlbum_2w,
	nBottle_2w,    nBalloon_2w,  nSky_1w,       nWall_1w,       nGround_1w,
	nTree_2w,      nFence_2w,    nHouse_1w,     nRoof_1w,       nLight_1w,
	nMoon_1w,      nPicture_1w,  nTable_2w,     nStairs_2w,     nBed_2w,
	nPlant_2w,     nFood_2w,     nWoman_1w,     nMan_2w,        nMirror_1w,
	nSink_1w,      nUnits_2w,    nBroom_2w,     nGardenbits_1w, nRock_2w,
	nDroppings_1w, nWindow_1w,   nBird_2w,      nCage_2w,       nPhone_2w,
	nBlotpad_2w,   nDrawer_2w,   nChair_2w,     nTools_2w,      nBridge_2w,
	nWater_2w,     nBucket_2w,   nMouse_2w,     nWand_2w,       nHole_2w,
	nMousehole_2w, nHandle_2w,   nChute_2w,     nOrgan_2w,      nPost_2w,
	nGraf_2w,      nSwitch_2w,   nBlock_2w
};

//***************************************************************************
// Hugo 3 Windows
//***************************************************************************
const char *nHero_old_3w[]  = {"hero_old", "~", "Hugo", ""};
const char *nWheroold_3w[]  = {"wheroold", "~", "Hugo", ""};
const char *nPlane_3w[]     = {"plane", "craft", "cabin", "cockpit", ""};
const char *nCdoor_3w[]     = {"door", "cage", "door", "gate", ""};
const char *nVine_3w[]      = {"vine", "rope", "vine", ""};
const char *nSwinger_3w[]   = {"swinger", "~", "Hugo", ""};
const char *nSteps_3w[]     = {"step", "step", "stones", ""};
const char *nClay_3w[]      = {"clay", "some clay", "modelling clay", "plasticine", "model", "effigy", "voodoo", "doll", ""};
const char *nDoorlock_3w[]  = {"lock", "~", ""};
const char *nNeedles_3w[]   = {"needle", "some pins", "needles", "pin", ""};
const char *nNative_3w[]    = {"native", "man", "native", "lady", ""};
const char *nNat1_3w[]      = {"nat1", "man", "native", ""};
const char *nNat2_3w[]      = {"nat2", "man", "native", ""};
const char *nNat3_3w[]      = {"nat3", "man", "native", ""};
const char *nNatb_3w[]      = {"natb", "man", "native", ""};
const char *nNatg_3w[]      = {"natg", "man", "native girl", "girl", ""};
const char *nBottles_3w[]   = {"bottles", "~", ""};
const char *nFlask_3w[]     = {"flask", "a water flask", "flask", "water", "remedy", "antidote", ""};
const char *nPipe_3w[]      = {"blowpipe", "blowpipe and darts", "blowpipe", "dart", ""};
const char *nElephant_3w[]  = {"elephant", "animal", "elephant", ""};
const char *nE_eyes_3w[]    = {"e_eyes", "~", ""};
const char *nBouillon_3w[]  = {"bouillon", "bouillon cubes", "bouillon cubes", "bouil", "boull", "boulion", "cube", "season", "oxo", "knorr", ""};
const char *nMoushole_3w[]  = {"moushole", "~", ""};
const char *nDoclie_3w[]    = {"doclie", ".", "witch doctor", ""};
const char *nCheese_3w[]    = {"cheese", "a sandwich", "sandwich", ""};
const char *nSpider_3w[]    = {"spider", "insect", "spider", "tarantula", ""};
const char *nFire_3w[]      = {"fire", "flame", "pot", ""};
const char *nDocbits_3w[]   = {"skull", "shield", "torch", "spear", "bone", ""};
const char *nFire_1_3w[]    = {"fire_1", "~", ""};
const char *nFire_2_3w[]    = {"fire_2", "~", ""};
const char *nFire_3_3w[]    = {"fire_3", "~", ""};
const char *nFroth_3w[]     = {"froth", "~", ""};
const char *nScroll_3w[]    = {"scroll", "an old scroll", "scroll", "writing", ""};
const char *nCrystal_3w[]   = {"crystal", "a crystal ball", "crystal ball", "ball", ""};
const char *nGhost_3w[]     = {"ghost", "ghoul", "ghost", "spirit", ""};
const char *nRush_3w[]      = {"rushes", "ferns", "bullrushes", ""};

const char *nPool_3w[]      = {"pool", "pool", "pool", ""};
const char *nWaterfall_3w[] = {"waterfall", "waterfall", "waterfall", ""};
const char *nShelfbits_3w[] = {"shelf", "bottle", "jar", "medicine", "label", ""};
const char *nJungle_3w[]    = {"tree", "vine", "flower", "fern", "frond", "plant", "jungle", "undergrowth", ""};
const char *nOrchid_3w[]    = {"orchid", "flower", "plant", ""};
const char *nPole_3w[]      = {"pole", "stick", "wood", ""};
const char *nHut_3w[]       = {"hut", "house", "hut", "shed", ""};
const char *nAircraft_3w[]  = {"aircraft", "~", ""};
const char *nPlant1_3w[]    = {"plant1", "~", ""};
const char *nPlant2_3w[]    = {"plant2", "~", ""};
const char *nPlant3_3w[]    = {"plant3", "~", ""};
const char *nPlant4_3w[]    = {"plant4", "~", ""};
const char *nPlant5_3w[]    = {"plant5", "~", ""};
const char *nWeb_3w[]       = {"web", "~", ""};
const char *nO_eye_3w[]     = {"o_eye", "~", ""};
const char *nFire_4_3w[]    = {"fire_4", "~", ""};
const char *nMouth_3w[]     = {"mouth", "~", ""};
const char *nThem_3w[]      = {"them", "~", ""};

const char **arrayNouns_3w[] = {
	nDummy,        nHero_2w,     nWhero_1w,    nHero_old_3w,  nPenny_2w,
	nPennylie_2w,  nLips_1w,     nPlane_3w,    nDoor_3w,      nBlock_3w,
	nCdoor_3w,     nVine_3w,     nSwinger_3w,  nSteps_3w,     nDoctor_3w,
	nClay_3w,      nDoorlock_3w, nNeedles_3w,  nNative_3w,    nNat1_3w,
	nNat2_3w,      nNat3_3w,     nNatb_3w,     nNatg_3w,      nBottles_3w,
	nFlask_3w,     nPipe_3w,     nElephant_3w, nE_eyes_3w,    nBouillon_3w,
	nMoushole_3w,  nDoclie_3w,   nCheese_3w,   nCage_3w,      nSpider_3w,
	nSnake_3w,     nFire_3w,     nDocbits_3w,  nFire_1_3w,    nFire_2_3w,
	nFire_3_3w,    nScroll_3w,   nCrystal_3w,  nGhost_3w,     nBell_3w,
	nBook_3w,      nCandle_3w,   nRush_3w,     nSky_1w,       nWall_1w,
	nGround_1w,    nFence_2w,    nBridge_3w,   nWater_3w,     nPool_3w,
	nWaterfall_3w, nMouse_3w,    nWindow_3w,   nShelfbits_3w, nOrchid_3w,
	nPole_3w,      nHut_3w,      nRock_3w,     nAircraft_3w,  nPlant1_3w,
	nPlant2_3w,    nPlant3_3w,   nPlant4_3w,   nPlant5_3w,    nJungle_3w,
	nWeb_3w,       nO_eye_3w,    nFire_4_3w,   nMouth_3w,     nFood_3w,
	nThem_3w
};

//***************************************************************************
// Hugo 1 Dos
//***************************************************************************

const char *nArc_1d[]       = {"arc", "~", ""};
const char *nHero_1d[]      = {"hero", "self", "Hugo", ""};
const char *nKey_1d[]       = {"key", "a door key", " key", ""};
const char *nMonkey_1d[]    = {"monkey", "~", "Hugo", ""};
const char *nOldman_1d[]    = {"oldman", "old", "old man", "man",  ""};
const char *nPkin_1d[]      = {"pumpkin", "a pumpkin", "pumpkin", ""};
const char *nWard_1d[]      = {"wardrobe", "cabinet", "closet", ""};

const char *nMan_1d[]       = {"man", "dracula", "monster", "frankenstein", "alien", "guest", "vampire", ""};
const char *nPlant_1d[]     = {"plant", "flower", "plant", "vase", ""};

const char **arrayNouns_1d[] = {
	nDummy,          nHero_1d,     nTrap_1w,     nWard_1d,     nDoor_1w,
	nBat_1w,         nEyes_1w,     nPkin_1d,     nCandle_1w,   nRope_1w,
	nCupb_1w,        nKnife_1w,    nWhistle_1w,  nWdoorl_1w,   nWdoorr_1w,
	nMask_1w,        nButler_1w,   nChop_1w,     nRedeyes_1w,  nLips_1w,
	nArm_1w,         nHdlshero_1w, nMonkey_1d,   nKey_1d,      nShed_2w,
	nDog_1w,         nCarpet_1w,   nBolt_1w,     nHerodead_1w, nOilcan_1w,
	nMummy_1w,       nMdoor_1w,    nGold_1w,     nBoat_1w,     nWhero_1w,
	nOldman_1d,      nGuard_1w,    nProf_1w,     nIgor_1w,     nBung_1w,
	nGdoor_1w,       nSpachero_1w, nFuzyhero_1w, nArc_1d,      nSky_1w,
	nWall_1w,        nGround_1w,   nTree_1w,     nFence_1w,    nHouse_1w,
	nRoof_1w,        nLight_1w,    nMoon_1w,     nPicture_1w,  nTable_1w,
	nStairs_1w,      nBed_1w,      nFace_1w,     nPlant_1d,    nWitch_1w,
	nFood_1w,        nWoman_1w,    nMan_1d,      nMirror_1w,   nToilet_1w,
	nBath_1w,        nSink_1w,     nUnits_1w,    nBroom_1w,    nGardenbits_1w,
	nMousehole_1w,   nPenelope_1w, nRock_1w,     nTomb_1w,     nDroppings_1w,
	nMachinebits_1w, nHands_1w,    nWindow_1w,   nBatpic_1w
};

//***************************************************************************
// Hugo 2 Dos
//***************************************************************************
const char *nBug_2d[]        = {"bug", "bee", "bug zapper", "insect", "wasp", ""};
const char *nCupb_2d[]       = {"cupboard", "cabinet", "dresser", "hutch", "sideboard", "bureau", "desk", "drawer", ""};
const char *nDalek_2d[]      = {"alek", "monster", "robot", ""};

const char **arrayNouns_2d[] = {
	nDummy,        nHero_2w,       nPenny_2w,   nPennylie_2w,  nPenfall_2w,
	nSmoke_2w,     nLips_1w,       nMaid_2w,    nBookcase_2w,  nBook_2w,
	nKeyhole_2w,   nPanel_2w,      nMatches_2w, nCrate_2w,     nDumb_2w,
	nMurder_2w,    nTrap_2w,       nWard_2w,    nDoor_2w,      nRope_2w,
	nCupb_2d,      nGarlic_2w,     nGardner_2w, nButton_2w,    nRed_2w,
	nYellow_2w,    nGreen_2w,      nBlue_2w,    nFly_2w,       nLeaf_2w,
	nShedlight_2w, nGatelight_2w,  nZapper_2w,  nBug_2d,       nKnife_2w,
	nShed_2w,      nOldman_2w,     nSnake_2w,   nStick_2w,     nDynamite_2w,
	nKennel_2w,    nDog_2w,        nWell_2w,    nBanana_2w,    nLamp_2w,
	nGenie_2w,     nTardis_2w,     nHarry_2w,   nHester_2w,    nLetter_2w,
	nDoctor_2w,    nDalek_2d,      nCook_2w,    nCookb_2w,     nCop_2w,
	nHorace_2w,    nBell_2w,       nCatnip_2w,  nCat_2w,       nGun_2w,
	nPaper_2w,     nPencil_2w,     nKey_2w,     nMagnify_2w,   nSafe_2w,
	nScrew_2w,     nWill_2w,       nAlbum_2w,   nBottle_2w,    nBalloon_2w,
	nSky_1w,       nWall_1w,       nGround_1w,  nTree_2w,      nFence_2w,
	nHouse_1w,     nRoof_1w,       nLight_1w,   nMoon_1w,      nPicture_1w,
	nTable_2w,     nStairs_2w,     nBed_2w,     nPlant_2w,     nFood_2w,
	nWoman_1w,     nMan_2w,        nMirror_1w,  nSink_1w,      nUnits_2w,
	nBroom_2w,     nGardenbits_1w, nRock_2w,    nDroppings_1w, nWindow_1w,
	nBird_2w,      nCage_2w,       nPhone_2w,   nChair_2w,     nTools_2w,
	nBridge_2w,    nWater_2w,      nBucket_2w,  nMouse_2w,     nWand_2w,
	nHole_2w,      nHandle_2w,     nChute_2w,   nOrgan_2w,     nPost_2w,
	nGraf_2w
};

const char *nDocbits_3d[]  = {"skull", "shield", "torch", "fire", "flame", "spear", "bone", ""};
const char *nPipe_3d[]     = {"blowpipe", "blowpipe & darts", "blowpipe", "dart", ""};

const char **arrayNouns_3d[] = {
	nDummy,       nHero_2w,      nWhero_1w,    nHero_old_3w, nWheroold_3w,
	nPenny_2w,    nPennylie_2w,  nLips_1w,     nPlane_3w,    nDoor_3w,
	nCdoor_3w,    nBlock_3w,     nVine_3w,     nSwinger_3w,  nDoctor_3w,
	nClay_3w,     nDoorlock_3w,  nNeedles_3w,  nNative_3w,   nNat1_3w,
	nNat2_3w,     nNat3_3w,      nNatb_3w,     nNatg_3w,     nBottles_3w,
	nFlask_3w,    nCage_3w,      nPipe_3d,     nElephant_3w, nE_eyes_3w,
	nBouillon_3w, nMoushole_3w,  nDoclie_3w,   nCheese_3w,   nSpider_3w,
	nSnake_3w,    nFire_3w,      nDocbits_3d,  nFire_1_3w,   nFire_2_3w,
	nFire_3_3w,   nFroth_3w,     nScroll_3w,   nCrystal_3w,  nGhost_3w,
	nBell_3w,     nBook_3w,      nCandle_3w,   nSky_1w,      nWall_1w,
	nGround_1w,   nFence_2w,     nBridge_3w,   nWater_3w,    nMouse_3w,
	nWindow_3w,   nShelfbits_3w, nJungle_3w,   nOrchid_3w,   nPole_3w,
	nHut_3w,      nRock_3w,      nAircraft_3w, nPlant1_3w,   nPlant2_3w,
	nPlant3_3w,   nPlant4_3w,    nPlant5_3w,   nWeb_3w,      nO_eye_3w,
	nFire_4_3w,   nMouth_3w,     nFood_3w,     nThem_3w
};

// Verbs and synonyms (N.B. put null string at end of last synonym)
//***************************************************************************
// Multi-version verbs
//***************************************************************************
const char *vDummy[]        = {"", ""};
const char *vBlow_1w[]      = {"blow", ""};
const char *vClose_1w[]     = {"close ", "shut", ""};
const char *vCut_1w[]       = {"cut", ""};
const char *vDial_2w[]      = {"dial", "ring", "call", ""};
const char *vEat_1w[]       = {"eat", ""};
const char *vBreak_2w[]     = {"break", "smash", "destroy", "kick", "slash", "pop ", "burst", ""};
const char *vAttack_2w[]    = {"attack", "hit", "kill", "fight", "punch", ""};
const char *vGive_1w[]      = {"give", "offer", ""};		// Must come before off
const char *vHerring_2w[]   = {"herring", ""};			// Must come before ring
const char *vLift_1w[]      = {"lift", ""};
const char *vLock_1w[]      = {"lock", ""};
const char *vLook_2w[]      = {"look", "examine", "search", ""};// Must come after under
const char *vMakeUseOf_1w[] = {"makeuseof", ""};
const char *vMove_1w[]      = {"move", ""};
const char *vOff_1w[]       = {"off", "remove", ""};		// e.g. take off, turn off
const char *vOil_1w[]       = {"oil ", ""};				// Note this is a noun too! (hence space)
const char *vOpen_1w[]      = {"open", ""};
const char *vOutof_1w[]     = {"get out", ""};
const char *vPush_2w[]      = {"push", "press", "slide", "slip", ""};// Must come before off
const char *vRead_2w[]      = {"read", ""};
const char *vRest_2w[]      = {"rest", "lie down", "lie on", ""};
const char *vRide_1w[]      = {"ride", "mount", "get on", ""};
const char *vRing_2w[]      = {"ring", ""};
const char *vStrike_2w[]    = {"strike", "light", ""};	// As in matches, fuze
const char *vTake_1w[]      = {"take", "get", "carry", "pick up", "grab", ""};
const char *vTalk_2w[]      = {"talk", "speak", "say", "tell", "ask", ""};
const char *vThrowit_1w[]   = {"throw", "hurl", "fling", ""};
const char *vUnder_1w[]     = {"under", ""};				// e.g. look under
const char *vUnlock_1w[]    = {"unlock", "undo", ""};
const char *vUntie_1w[]     = {"untie", ""};
const char *vWear_1w[]      = {"wear", "put on", ""};

const char *vClimb_1w[]     = {"climb", ""};
const char *vDig_1w[]       = {"dig", "burrow", ""};		// Must come before under
const char *vDrink_1w[]     = {"drink", ""};
const char *vFeed_1w[]      = {"feed", ""};
const char *vGo_1w[]        = {"go", ""};
const char *vHelp_1w[]      = {"help", ""};
const char *vJump_1w[]      = {"jump", ""};
const char *vKiss_1w[]      = {"kiss", ""};
const char *vKnock_1w[]     = {"knock", ""};
const char *vListen_1w[]    = {"listen", ""};
const char *vMagic_2w[]     = {"sesame", "abraca", "xyzzy", ""};// Before open
const char *vPlay_2w[]      = {"play", ""};
const char *vQuery_1w[]     = {"what", "why", "when", "where", "which", "who", "how", "?", ""};
const char *vShout_1w[]     = {"shout", "call", ""};
const char *vSit_1w[]       = {"sit", ""};
const char *vSmell_1w[]     = {"smell", ""};
const char *vStroke_2w[]    = {"stroke", "pet ", ""};
const char *vSweep_1w[]     = {"sweep", ""};
const char *vSwitch_1w[]    = {"switch", ""};
const char *vTie_2w[]       = {"tie ", "attach", ""};
const char *vUndress_2w[]   = {"undress", ""};			// Must come before take
const char *vUnscrew_2w[]   = {"unscrew", ""};			// Must come before rude
const char *vUse_2w[]       = {"use ", ""};
const char *vWash_1w[]      = {"wash", ""};
const char *vWind_2w[]      = {"wind", "turn", "rotate", "winch", ""}; // As in wind, unwind rope
const char *vWish_2w[]      = {"wish", ""};

//***************************************************************************
// Hugo 1 Windows
//***************************************************************************
const char *vPush_1w[]      = {"push", "press", "cast off", ""};// Must come before off
const char *vInto_1w[]      = {"get in", "enter", ""};	// Must come before take (get)
const char *vDrop_1w[]      = {"drop", "put down", ""};
const char *vAttack_1w[]    = {"attack", "hit", "kill", "fight", ""};
const char *vBreak_1w[]     = {"break", "smash", "destroy", "kick", "slash", ""};
const char *vRub_1w[]       = {"rub", ""};				// Must come before Oil
const char *vLook_1w[]      = {"look", "examine", ""};	// Must come after under
const char *vTalk_1w[]      = {"talk", "speak", "say", ""};
const char *vPlug_1w[]      = {"plug", ""};

const char *vEnter_1w[]     = {"enter", ""};
const char *vCrap_1w[]      = {"crap", "shit", "piss", ""};
const char *vUnbolt_1w[]    = {"unbolt", ""};
const char *vLakeverbs_1w[] = {"swim", "paddle", "wade", ""};
const char *vMagic_1w[]     = {"sesame", "abraca", "xyzzy", "zelda", ""};// Before open
const char *vRude_1w[]      = {"fuck", "fart", ""};
const char *vStroke_1w[]    = {"stroke", ""};
const char *vHide_1w[]      = {"hide", "behind", ""};

// Added by Strangerke to ease usage of hugo.dat
const char *vBolt_1w[]      = {"bolt", "~", "bolt", ""};
const char *vHero_1w[]      = {"hero", "self", "Hugo", "hugo", ""};

const char **arrayVerbs_1w[]  = {
	vDummy,     vMakeUseOf_1w, vMagic_1w,     vOpen_1w,   vClose_1w,
	vUnlock_1w, vLock_1w,      vPush_1w,      vGive_1w,   vRude_1w,
	vOff_1w,    vInto_1w,      vOutof_1w,     vCrap_1w,   vRide_1w,
	vTake_1w,   vDrop_1w,      vAttack_1w,    vBreak_1w,  vThrowit_1w,
	vWear_1w,   vRub_1w,       vOil_1w,       vMove_1w,   vLift_1w,
	vDig_1w,    vUnder_1w,     vLook_1w,      vEat_1w,    vBlow_1w,
	vUntie_1w,  vCut_1w,       vTalk_1w,      vPlug_1w,   vShout_1w,
	vQuery_1w,  vJump_1w,      vGo_1w,        vEnter_1w,  vClimb_1w,
	vSwitch_1w, vListen_1w,    vKnock_1w,     vSmell_1w,  vSit_1w,
	vKiss_1w,   vUnbolt_1w,    vLakeverbs_1w, vHelp_1w,   vDrink_1w,
	vSweep_1w,  vFeed_1w,      vWash_1w,      vStroke_1w, vHide_1w,
	vBolt_1w,   vHero_1w
};

//***************************************************************************
// Hugo 2 Windows
//***************************************************************************
// Verbs and synonyms (N.B. put null string at end of last synonym)
const char *vBlock_2w[]     = {"block", "brick", "blocks", ""}; // Must come before lock
const char *vSearch_2w[]    = {"search", "examine", ""};
const char *vInto_2w[]      = {"get in", "enter", "go in", " in ", ""};// Must come before take (get)
const char *vDrop_2w[]      = {"drop", "put down", "place", "put ", ""};
const char *vFire_2w[]      = {"fire", "shoot", ""};

const char *vSwitch_2w[]    = {"switch", "~", "switch", ""};
const char *vHello_2w[]     = {"hello", "hi ", " hi", ""};
const char *vRude_2w[]      = {"fuck", "fart", "screw ", ""};// Must come before off
const char *vScribble_2w[]  = {"scribble", ""};

const char **arrayVerbs_2w[] = {
	vDummy,      vMakeUseOf_1w, vBlock_2w,   vUse_2w,    vPush_2w,
	vUnder_1w,   vSearch_2w,    vLook_2w,    vMagic_2w,  vOpen_1w,
	vClose_1w,   vUnlock_1w,    vLock_1w,    vRide_1w,   vRest_2w,
	vUndress_2w, vGive_1w,      vUnscrew_2w, vRude_2w,   vOff_1w,
	vInto_2w,    vOutof_1w,     vTake_1w,    vDrop_2w,   vAttack_2w,
	vBreak_2w,   vThrowit_1w,   vWear_1w,    vRub_1w,    vOil_1w,
	vMove_1w,    vLift_1w,      vDig_1w,     vEat_1w,    vBlow_1w,
	vUntie_1w,   vCut_1w,       vTalk_2w,    vStrike_2w, vHerring_2w,
	vRing_2w,    vRead_2w,      vDial_2w,    vFire_2w,   vShout_1w,
	vQuery_1w,   vJump_1w,      vGo_1w,      vClimb_1w,  vSwitch_2w,
	vListen_1w,  vKnock_1w,     vSmell_1w,   vSit_1w,    vKiss_1w,
	vHelp_1w,    vDrink_1w,     vSweep_1w,   vFeed_1w,   vWash_1w,
	vHello_2w,   vWind_2w,      vTie_2w,     vStroke_2w, vPlay_2w,
	vWish_2w,    vScribble_2w
};

//***************************************************************************
// Hugo 3 Windows
//***************************************************************************
// Verbs and synonyms (N.B. put null string at end of last synonym)
const char *vSearch_3w[]   = {"search", "examine", "inside", "look in", ""};
const char *vBehind_3w[]   = {"behind", ""};			// As in look behind
const char *vInto_3w[]     = {"get in", "enter", "go in", ""};// Must come before take (get)
const char *vOutof_3w[]    = {"get out", "get down", "go back", "exit", ""};
const char *vTake_3w[]     = {"take", "get", "carry", "pick ", "grab", "catch", ""};
const char *vDrop_3w[]     = {"drop", " down", "place", ""};
const char *vRub_3w[]      = {"rub", "clean", "wipe", "polish", ""};
const char *vFill_3w[]     = {"fill", ""};				// Must come before ask
const char *vEmpty_3w[]    = {"empty", "pour", ""};
const char *vShoot_3w[]    = {"fire ", "shoot", ""};
const char *vSwing_3w[]    = {"swing", ""};
const char *vCross_3w[]    = {"cross", ""};
const char *vMake_3w[]     = {"make", "build", "construct", ""};
const char *vStick_3w[]    = {"stick", "push", "prick", "stab", "put", ""};  // As in pins
const char *vRepair_3w[]   = {"repair", "fix ", "mend ", ""};
const char *vFly_3w[]      = {"fly ", ""};
const char *vDouse_3w[]    = {"douse", "extinguish", "put out", ""};
const char *vExorcise_3w[] = {"exorcise", "banish", "frighten", "scare", ""};

const char *vHello_3w[]    = {"hello", "hi ", " hi", "howd", ""};
const char *vRude_3w[]     = {"fuck", "fart", "screw ", "shit", ""};  // Must come before off
const char *vNaughty_3w[]  = {"fondle", "breasts", "tits", "fanny", "kiss", ""};
const char *vPut_3w[]      = {"put ", ""};
const char *vSwim_3w[]     = {"swim", "wade", ""};
const char *vShow_3w[]     = {"show", ""};				// As in show mouse to elephant

const char **arrayVerbs_3w[] = {
	vDummy,     vMakeUseOf_1w, vPush_2w,    vUnder_1w,   vSearch_3w,
	vBehind_3w, vLook_2w,      vMagic_2w,   vOpen_1w,    vClose_1w,
	vRide_1w,   vRest_2w,      vUndress_2w, vGive_1w,    vUnscrew_2w,
	vRude_3w,   vNaughty_3w,   vOff_1w,     vInto_3w,    vOutof_3w,
	vTake_3w,   vDrop_3w,      vUnlock_1w,  vLock_1w,    vAttack_2w,
	vBreak_2w,  vThrowit_1w,   vWear_1w,    vRub_3w,     vOil_1w,
	vMove_1w,   vLift_1w,      vDig_1w,     vEat_1w,     vBlow_1w,
	vUntie_1w,  vCut_1w,       vFill_3w,    vEmpty_3w,   vDrink_1w,
	vStrike_2w, vHerring_2w,   vRing_2w,    vRead_2w,    vDial_2w,
	vShoot_3w,  vRepair_3w,    vFly_3w,     vDouse_3w,   vExorcise_3w,
	vSwing_3w,  vCross_3w,     vMake_3w,    vStick_3w,   vTalk_2w,
	vShout_1w,  vQuery_1w,     vJump_1w,    vGo_1w,      vClimb_1w,
	vSwitch_1w, vListen_1w,    vKnock_1w,   vSmell_1w,   vSit_1w,
	vKiss_1w,   vHelp_1w,      vSweep_1w,   vFeed_1w,    vWash_1w,
	vHello_3w,  vWind_2w,      vTie_2w,     vStroke_2w,  vPlay_2w,
	vWish_2w,   vPut_3w,       vUse_2w,     vSwim_3w,    vShow_3w
};

//***************************************************************************
// Hugo 1 Dos
//***************************************************************************
// Verbs and synonyms (N.B. put null string at end of last synonym)
const char *vPush_1d[]   = {"push", "press", "cast off", ""}; // Must come before off
const char *vInto_1d[]   = {"get in", "enter", ""};    // Must come before take (get)
const char *vOutof_1d[]  = {"get out", ""};
const char *vTake_1d[]   = {"take", "get", "carry", "pick up", "grab", ""};
const char *vDrop_1d[]   = {"drop", "put down", ""};
const char *vAttack_1d[] = {"attack", "hit", "kill", "fight", ""};
const char *vBreak_1d[]  = {"break", "smash", "destroy", "kick", "slash", ""};
const char *vThrow_1d[]  = {"throw", "hurl", "fling", ""};
const char *vRub_1d[]    = {"rub", ""};   // Must come before Oil
const char *vLook_1d[]   = {"look", "examine", ""};    // Must come after under
const char *vTalk_1d[]   = {"talk", "speak", "say", ""};

const char *vSwitch_1d[] = {"switch", ""};
const char *vMagic_1d[]  = {"sesame", "abraca", "xyzzy", "zelda", ""}; // Before open

const char **arrayVerbs_1d[]  = {
	vDummy,     vMagic_1d,     vOpen_1w,  vClose_1w, vUnlock_1w,
	vLock_1w,   vPush_1d,      vGive_1w,  vRude_1w,  vOff_1w,
	vInto_1d,   vOutof_1d,     vCrap_1w,  vRide_1w,  vTake_1d,
	vDrop_1d,   vAttack_1d,    vBreak_1d, vThrow_1d, vWear_1w,
	vRub_1d,    vOil_1w,       vMove_1w,  vLift_1w,  vDig_1w,
	vUnder_1w,  vLook_1d,      vEat_1w,   vBlow_1w,  vUntie_1w,
	vCut_1w,    vTalk_1d,      vPlug_1w,  vShout_1w, vQuery_1w,
	vJump_1w,   vGo_1w,        vEnter_1w, vClimb_1w, vSwitch_1d,
	vListen_1w, vKnock_1w,     vSmell_1w, vSit_1w,   vKiss_1w,
	vUnbolt_1w, vLakeverbs_1w, vHelp_1w,  vDrink_1w, vSweep_1w,
	vFeed_1w,   vWash_1w
};

//***************************************************************************
// Hugo 2 Dos
//***************************************************************************
const char *vBlock_2d[]    = {"block", "brick", ""}; // Must come before lock
const char *vUse_2d[]      = {"use ", ""};
const char *vPush_2d[]     = {"push", "press", "slide", "slip", ""};// Must come before off
const char *vUnder_2d[]    = {"under", ""}; // e.g. look under.  Must come after push
const char *vSearch_2d[]   = {"search", "examine", ""};
const char *vLook_2d[]     = {"look", "examine", "search", ""};    // Must come after under
const char *vOpen_2d[]     = {"open", ""};
const char *vClose_2d[]    = {"close ", "shut", ""};
const char *vUnlock_2d[]   = {"unlock", "undo", ""};
const char *vLock_2d[]     = {"lock", ""};
const char *vRide_2d[]     = {"ride", "mount", "get on", ""};
const char *vRest_2d[]     = {"rest", "lie down", "lie on", ""};
const char *vOff_2d[]      = {"off", "remove", ""};   // e.g. take off,  turn off
const char *vInto_2d[]     = {"get in", "enter", "go in", " in ", ""};// Must come before take (get)
const char *vOutof_2d[]    = {"get out", ""};
const char *vTake_2d[]     = {"take", "get", "carry", "pick up", "grab", ""};
const char *vDrop_2d[]     = {"drop", "put down", "place", "put ", ""};
const char *vAttack_2d[]   = {"attack", "hit", "kill", "fight", "punch", ""};
const char *vBreak_2d[]    = {"break", "smash", "destroy", "kick", "slash", "pop ", "burst", ""};
const char *vThrow_2d[]    = {"throw", "hurl", "fling", ""};
const char *vWear_2d[]     = {"wear", "put on", ""};
const char *vRub_2d[]      = {"rub", ""};    // Must come before Oil
const char *vOil_2d[]      = {"oil ", ""};   // Note this is a noun too! (hence space)
const char *vMove_2d[]     = {"move", ""};
const char *vLift_2d[]     = {"lift", ""};
const char *vEat_2d[]      = {"eat", ""};
const char *vBlow_2d[]     = {"blow", ""};
const char *vUntie_2d[]    = {"untie", ""};
const char *vCut_2d[]      = {"cut", ""};
const char *vTalk_2d[]     = {"talk", "speak", "say", "tell", "ask", ""};
const char *vGive_2d[]     = {"give", "offer", ""};      // Must come before off
const char *vStrike_2d[]   = {"strike", "light", ""};    // As in matches,  fuze
const char *vHerring_2d[]  = {"herring", ""};           // Must come before ring
const char *vRing_2d[]     = {"ring", ""};
const char *vRead_2d[]     = {"read", ""};
const char *vDial_2d[]     = {"dial", "ring", "call", ""};
const char *vFire_2d[]     = {"fire", "shoot", ""};

const char *vShout_2d[]    = {"shout", "call", ""};
const char *vQuery_2d[]    = {"what", "why", "when", "where", "which", "who", "how", "?", ""};
const char *vJump_2d[]     = {"jump", ""};
const char *vGo_2d[]       = {"go", ""};
const char *vClimb_2d[]    = {"climb", ""};
const char *vSwitch_2d[]   = {"switch", ""};
const char *vListen_2d[]   = {"listen", ""};
const char *vKnock_2d[]    = {"knock", ""};
const char *vSmell_2d[]    = {"smell", ""};
const char *vSit_2d[]      = {"sit", ""};
const char *vKiss_2d[]     = {"kiss", ""};
const char *vHelp_2d[]     = {"help", ""};
const char *vMagic_2d[]    = {"sesame", "abraca", "xyzzy", ""}; // Before open
const char *vDig_2d[]      = {"dig", "burrow", ""}; // Must come before under
const char *vDrink_2d[]    = {"drink", ""};
const char *vSweep_2d[]    = {"sweep", ""};
const char *vFeed_2d[]     = {"feed", ""};
const char *vWash_2d[]     = {"wash", ""};
const char *vHello_2d[]    = {"hello", "hi ", " hi", ""};
const char *vWind_2d[]     = {"wind", "turn", "rotate", "winch", ""};  // As in wind,  unwind rope
const char *vTie_2d[]      = {"tie ", "attach", ""};
const char *vStroke_2d[]   = {"stroke", "pet ", ""};
const char *vUnscrew_2d[]  = {"unscrew", ""};               // Must come before rude
const char *vRude_2d[]     = {"fuck", "fart", "screw ", ""};  // Must come before off
const char *vUndress_2d[]  = {"undress", ""};               // Must come before take
const char *vPlay_2d[]     = {"play", ""};
const char *vWish_2d[]     = {"wish", ""};

const char **arrayVerbs_2d[] = {
	vDummy,      vBlock_2d, vUse_2d,     vPush_2d,    vUnder_2d,
	vSearch_2d,  vLook_2d,  vOpen_2d,    vClose_2d,   vUnlock_2d,
	vLock_2d,    vRide_2d,  vRest_2d,    vOff_2d,     vInto_2d,
	vOutof_2d,   vTake_2d,  vDrop_2d,    vAttack_2d,  vBreak_2d,
	vThrow_2d,   vWear_2d,  vRub_2d,     vOil_2d,     vMove_2d,
	vLift_2d,    vEat_2d,   vBlow_2d,    vUntie_2d,   vCut_2d,
	vTalk_2d,    vGive_2d,  vStrike_2d,  vHerring_2d, vRing_2d,
	vRead_2d,    vDial_2d,  vFire_2d,    vShout_2d,   vQuery_2d,
	vJump_2d,    vGo_2d,    vClimb_2d,   vSwitch_2d,  vListen_2d,
	vKnock_2d,   vSmell_2d, vSit_2d,     vKiss_2d,    vHelp_2d,
	vMagic_2d,   vDig_2d,   vDrink_2d,   vSweep_2d,   vFeed_2d,
	vWash_2d,    vHello_2d, vWind_2d,    vTie_2d,     vStroke_2d,
	vUnscrew_2d, vRude_2d,  vUndress_2d, vPlay_2d,    vWish_2d
};

// Verbs and synonyms (N.B. put null string at end of last synonym)
const char *vPush_3d[]     = {"push", "press", "slide", "slip", ""};
const char *vUnder_3d[]    = {"under", ""};
const char *vSearch_3d[]   = {"search", "examine", "inside", "look in", ""};
const char *vBehind_3d[]   = {"behind", ""};
const char *vLook_3d[]     = {"look", "examine", "search", ""};
const char *vOpen_3d[]     = {"open", ""};
const char *vClose_3d[]    = {"close ", "shut", ""};
const char *vRide_3d[]     = {"ride", "mount", "get on", ""};
const char *vRest_3d[]     = {"rest", "lie down", "lie on", ""};
const char *vOff_3d[]      = {"off", "remove", ""};
const char *vInto_3d[]     = {"get in", "enter", "go in", ""};
const char *vOutof_3d[]    = {"get out", "get down", "go back", "exit", ""};
const char *vTake_3d[]     = {"take", "get", "carry", "pick ", "grab", "catch", ""};
const char *vDrop_3d[]     = {"drop", " down", "place", ""};
const char *vUnlock_3d[]   = {"unlock", "undo", ""};
const char *vLock_3d[]     = {"lock", ""};
const char *vAttack_3d[]   = {"attack", "hit", "kill", "fight", "punch", ""};
const char *vBreak_3d[]    = {"break", "smash", "destroy", "kick", "slash", "pop ", "burst", ""};
const char *vThrow_3d[]    = {"throw", "hurl", "fling", ""};
const char *vWear_3d[]     = {"wear", "put on", ""};
const char *vRub_3d[]      = {"rub", "clean", "wipe", "polish", ""};
const char *vOil_3d[]      = {"oil ", ""};
const char *vMove_3d[]     = {"move", ""};
const char *vLift_3d[]     = {"lift", ""};
const char *vEat_3d[]      = {"eat", ""};
const char *vBlow_3d[]     = {"blow ", ""};
const char *vUntie_3d[]    = {"untie", ""};
const char *vCut_3d[]      = {"cut", ""};
const char *vFill_3d[]     = {"fill", ""};
const char *vEmpty_3d[]    = {"empty", "pour", ""};
const char *vDrink_3d[]    = {"drink", ""};
const char *vGive_3d[]     = {"give", "offer", ""};
const char *vStrike_3d[]   = {"strike", "light", ""};
const char *vHerring_3d[]  = {"herring", ""};
const char *vRing_3d[]     = {"ring", ""};
const char *vRead_3d[]     = {"read", ""};
const char *vDial_3d[]     = {"dial", "ring", "call", ""};
const char *vShoot_3d[]    = {"fire ", "shoot", ""};
const char *vSwing_3d[]    = {"swing", ""};
const char *vCross_3d[]    = {"cross", ""};
const char *vMake_3d[]     = {"make", "build", "construct", ""};
const char *vStick_3d[]    = {"stick", "push", "prick", "stab", "put", ""};
const char *vTalk_3d[]     = {"talk", "speak", "say", "tell", "ask ", ""};
const char *vRepair_3d[]   = {"repair", "fix ", "mend ", ""};
const char *vFly_3d[]      = {"fly ", ""};
const char *vDouse_3d[]    = {"douse", "extinguish", "put out", ""};
const char *vExorcise_3d[] = {"exorcise", "banish", "frighten", "scare", ""};

const char *vShout_3d[]    = {"shout", "call", ""};
const char *vQuery_3d[]    = {"what", "why", "when", "where", "which", "who", "how", "?", ""};
const char *vJump_3d[]     = {"jump", ""};
const char *vGo_3d[]       = {"go", ""};
const char *vClimb_3d[]    = {"climb", ""};
const char *vSwitch_3d[]   = {"switch", ""};
const char *vListen_3d[]   = {"listen", ""};
const char *vKnock_3d[]    = {"knock", ""};
const char *vSmell_3d[]    = {"smell", ""};
const char *vSit_3d[]      = {"sit", ""};
const char *vKiss_3d[]     = {"kiss", ""};
const char *vHelp_3d[]     = {"help", ""};
const char *vMagic_3d[]    = {"sesame", "abraca", "xyzzy", ""}; // Before open
const char *vDig_3d[]      = {"dig", "burrow", ""}; // Must come before under
const char *vSweep_3d[]    = {"sweep", ""};
const char *vFeed_3d[]     = {"feed", ""};
const char *vWash_3d[]     = {"wash", ""};
const char *vHello_3d[]    = {"hello", "hi ", " hi", "howd", ""};
const char *vWind_3d[]     = {"wind", "turn", "rotate", "winch", ""};
const char *vTie_3d[]      = {"tie ", "attach", ""};
const char *vStroke_3d[]   = {"stroke", "pet ", ""};
const char *vUnscrew_3d[]  = {"unscrew", ""};
const char *vRude_3d[]     = {"fuck", "fart", "screw ", "shit", ""};
const char *vNaughty_3d[]  = {"fondle", "breasts", "tits", "fanny", "kiss", ""};
const char *vUndress_3d[]  = {"undress", ""};
const char *vPlay_3d[]     = {"play", ""};
const char *vWish_3d[]     = {"wish", ""};
const char *vPut_3d[]      = {"put ", ""};
const char *vUse_3d[]      = {"use ", ""};
const char *vSwim_3d[]     = {"swim", "wade", ""};
const char *vShow_3d[]     = {"show", ""};

const char **arrayVerbs_3d[] = {
	vDummy,      vPush_3d,    vUnder_3d,  vSearch_3d,   vBehind_3d,
	vLook_3d,    vMagic_3d,   vOpen_3d,   vClose_3d,    vRide_3d,
	vRest_3d,    vUndress_3d, vGive_3d,   vUnscrew_3d,  vRude_3d,
	vNaughty_3d, vOff_3d,     vInto_3d,   vOutof_3d,    vTake_3d,
	vDrop_3d,    vUnlock_3d,  vLock_3d,   vAttack_3d,   vBreak_3d,
	vThrow_3d,   vWear_3d,    vRub_3d,    vOil_3d,      vMove_3d,
	vLift_3d,    vDig_3d,     vEat_3d,    vBlow_3d,     vUntie_3d,
	vCut_3d,     vFill_3d,    vEmpty_3d,  vDrink_3d,    vStrike_3d,
	vHerring_3d, vRing_3d,    vRead_3d,   vDial_3d,     vShoot_3d,
	vRepair_3d,  vFly_3d,     vDouse_3d,  vExorcise_3d, vSwing_3d,
	vCross_3d,   vMake_3d,    vStick_3d,  vTalk_3d,     vShout_3d,
	vQuery_3d,   vJump_3d,    vGo_3d,     vClimb_3d,    vSwitch_3d,
	vListen_3d,  vKnock_3d,   vSmell_3d,  vSit_3d,      vKiss_3d,
	vHelp_3d,    vSweep_3d,   vFeed_3d,   vWash_3d,     vHello_3d,
	vWind_3d,    vTie_3d,     vStroke_3d, vPlay_3d,     vWish_3d,
	vPut_3d,     vUse_3d,     vSwim_3d,   vShow_3d
};

const char *screenNames_1w[] = {
		"House",    "Hall",     "Bed1",    "Diningrm", "Bathroom",
/* 5*/	"Kitchen",  "Garden",   "Storerm", "Basement", "Batcave",
/*10*/	"Mummyrm",  "Lakeroom", "Deadend", "Jail",     "The_end",
/*15*/	"Lab",      "House"
};

const char *screenNames_2w[] = {
		"House",    "hall",     "bed1",     "bed2",     "keyhole",
/* 5*/	"bed3",     "kitchen",  "backdoor", "shed",     "inshed",
/*10*/	"venus",    "gatesopn", "gatescls", "stream",   "zapper",
/*15*/	"mushroom", "well",     "snakepit", "phonebox", "street",
/*20*/	"kennel",   "rockroom", "rockgone", "threeway", "lampcave",
/*25*/	"chasm",    "passage",  "ladder",   "traproom", "hall2",
/*30*/	"lounge",   "parlor",   "catroom",  "boxroom",  "hall3",
/*35*/	"organ",    "hestroom", "retupmoc", "hall1",    "maze12",
/*40*/	"maze3" ,   "maze11",   "maze8" ,   "maze3",    "maze1" ,
/*45*/	"maze3" ,   "maze11",   "maze6" ,   "maze6",    "maze15",
/*50*/	"maze11",   "maze6" ,   "maze10",   "maze6",    "maze13",
/*55*/	"maze15",   "maze2" ,   "maze11",   "maze15",   "maze2" ,
/*60*/	"maze2" ,   "maze4" ,   "maze11",   "maze12",   "maze11",
/*65*/	"maze6" ,   "maze8" ,   "maze1" ,   "maze1",    "maze7" ,
/*70*/	"maze6" ,   "maze6" ,   "maze6" ,   "maze6",    "maze12",
/*75*/	"maze1" ,   "maze11",   "maze15",   "maze14",   "maze6" ,
/*80*/	"maze5" ,   "maze2" ,   "maze2" ,   "maze9",    "maze5" ,
/*85*/	"maze1" ,   "maze9" ,   "maze6" ,   "maze13",   "maze12",
/*90*/	"maze3" ,   "maze1" ,   "maze4" ,   "maze1",    "maze11",
/*95*/	"maze15",   "maze1" ,   "maze14",   "mazex",    "maze8" ,
/*100*/	"maze2" ,   "maze1" ,   "maze14",   "intro"
};

const char *screenNames_3w[] = {
		"CRASH",    "spider",   "BRIDGE",   "BRIDGE2" , "CLIFFTOP",
/* 5*/	"WFALL",    "WFALL_B",  "WBASE",    "STREAM",   "STREAM2",
/*10*/	"PATH_UL",  "VILLAGE",  "HUT_OUT",  "HUT_IN",   "GARDEN",
/*15*/	"OLDMAN",   "CLIFF",    "SLOPE",    "CAMP",     "SUNSET",
/*20*/	"TURN",     "PLANE",    "MAP",      "PATH",     "CAVE",
/*25*/	"titlescr"
};

const char *screenNames_1d[] = {
		"House",    "Hall",     "Bed1",     "Diningrm", "Bathroom",
/* 5*/	"Kitchen",  "Garden",   "Storerm",  "Basement", "Batcave",
/*10*/	"Mummyrm",  "Lakeroom", "Deadend",  "Jail",     "The_end",
/*15*/	"Lab",      "House"
};

const char *screenNames_2d[] = {
	    "House",    "hall",     "bed1",     "bed2",     "keyhole",
/* 5*/  "bed3",     "kitchen",  "backdoor", "shed",     "inshed",
/*10*/  "venus",    "gatesopn", "gatescls", "stream",   "zapper",
/*15*/  "mushroom", "well",     "snakepit", "phonebox", "street",
/*20*/  "kennel",   "rockroom", "rockgone", "threeway", "lampcave",
/*25*/  "chasm",    "passage",  "ladder",   "traproom", "hall2",
/*30*/  "lounge",   "parlor",   "catroom",  "boxroom",  "hall3",
/*35*/  "organ",    "hestroom", "retupmoc", "hall1",    "maze12",
/*40*/  "maze3",    "maze11",   "maze8",    "maze3",    "maze1",
/*45*/  "maze3",    "maze11",   "maze6",    "maze6",    "maze15",
/*50*/  "maze11",   "maze6",    "maze10",   "maze6" ,   "maze13",
/*55*/  "maze15",   "maze2",    "maze11",   "maze15",   "maze2",
/*60*/  "maze2",    "maze4",    "maze11",   "maze12",   "maze11",
/*65*/  "maze6",    "maze8",    "maze1",    "maze1",    "maze7",
/*70*/  "maze6",    "maze6",    "maze6",    "maze6",    "maze12",
/*75*/  "maze1",    "maze11",   "maze15",   "maze14",   "maze6",
/*80*/  "maze5",    "maze2",    "maze2",    "maze9",    "maze5",
/*85*/  "maze1",    "maze9",    "maze6",    "maze13",   "maze12",
/*90*/  "maze3",    "maze1",    "maze4",    "maze1",    "maze11",
/*95*/  "maze15",   "maze1",    "maze14",   "mazex",    "maze8",
/*100*/ "maze2",    "maze1",    "maze14",   "intro"
};

const char *screenNames_3d[] = {
		"CRASH",   "spider",  "BRIDGE",  "BRIDGE2", "CLIFFTOP",
/*10*/  "WFALL",   "WFALL_B", "WBASE",   "STREAM",  "STREAM2",
		"PATH_UL", "VILLAGE", "HUT_OUT", "HUT_IN",  "GARDEN",
/*20*/  "OLDMAN",  "CLIFF",   "SLOPE",   "CAMP",    "SUNSET",
		"TURN",    "PLANE",   "MAP",     "PATH",    "CAVE",
/*30*/  "titlescr"
};

// The required objects lists are lists of ptrs to nouns. Suffix with 0
uint16 rkey_1w[]    = {kNKey_1w,    0};
uint16 rpkin_1w[]   = {kNPkin_1w,   0};
uint16 rcandle_1w[] = {kNCandle_1w, 0};
uint16 rmask_1w[]   = {kNMask_1w,   0};
uint16 roil_1w[]    = {kNOilcan_1w, 0};
uint16 rknife_1w[]  = {kNKnife_1w,  0};
uint16 rbung_1w[]   = {kNBung_1w,   0};
uint16 rDummy[]     = {0,        0};
const uint16 *arrayReqs_1w[] = {rDummy,rkey_1w, rpkin_1w, rcandle_1w, rmask_1w, roil_1w, rknife_1w, rbung_1w, };

uint16 rgarlic_2w[] = {kNGarlic_2w,   0};
uint16 rmatch_2w[]  = {kNMatches_2w,  0};
uint16 rstick_2w[]  = {kNStick_2w,    0};
uint16 rdyn_2w[]    = {kNDynamite_2w, 0};
uint16 rlamp_2w[]   = {kNLamp_2w,     0};
uint16 rbanana_2w[] = {kNBanana_2w,   0};
uint16 rbell_2w[]   = {kNBell_2w,     0};
uint16 rcatnip_2w[] = {kNCatnip_2w,   0};
uint16 rgun_2w[]    = {kNGun_2w,      0};
uint16 rpaper_2w[]  = {kNPaper_2w,    0};
uint16 rpencil_2w[] = {kNPencil_2w,   0};
uint16 rmagnify_2w[]= {kNMagnify_2w,  0};
uint16 rwill_2w[]   = {kNWill_2w,     0};
uint16 rserum_2w[]  = {kNBottle_2w,   0};
const uint16 *arrayReqs_2w[] = {
	rDummy,    rgarlic_2w, rmatch_2w,   rstick_2w,  rdyn_2w,
	rlamp_2w,  rbanana_2w, rbell_2w,    rcatnip_2w, rgun_2w,
	rpaper_2w, rpencil_2w, rmagnify_2w, rwill_2w,   rserum_2w
};

uint16 rpins_3w[]    = {kNNeedles_3w, 0};
uint16 rcheese_3w[]  = {kNCheese_3w,  0};
uint16 rcrystal_3w[] = {kNCrystal_3w, 0};
uint16 rexor_3w[]    = {kNBell_3w,    kNBook_3w, kNCandle_3w, 0};
uint16 rbook_3w[]    = {kNBook_3w,    0};
uint16 rbell_3w[]    = {kNBell_3w,    0};
uint16 rpipe_3w[]    = {kNPipe_3w,    0};
const uint16 *arrayReqs_3w[] = {rDummy,rpins_3w, rcheese_3w, rcrystal_3w, rexor_3w, rbook_3w, rbell_3w, rpipe_3w};

uint16 rkey_1d[]    = {kNKey_1d,    0};
uint16 rpkin_1d[]   = {kNPkin_1d,   0};
uint16 rcandle_1d[] = {kNCandle_1d, 0};
uint16 rmask_1d[]   = {kNMask_1d,   0};
uint16 roil_1d[]    = {kNOilcan_1d, 0};
uint16 rknife_1d[]  = {kNKnife_1d,  0};
uint16 rbung_1d[]   = {kNBung_1d,   0};
const uint16 *arrayReqs_1d[] = {rDummy,rkey_1d, rpkin_1d, rcandle_1d, rmask_1d, roil_1d, rknife_1d, rbung_1d};

uint16 rgarlic_2d[] = {kNGarlic_2d,   0};
uint16 rmatch_2d[]  = {kNMatches_2d,  0};
uint16 rstick_2d[]  = {kNStick_2d,    0};
uint16 rdyn_2d[]    = {kNDynamite_2d, 0};
uint16 rlamp_2d[]   = {kNLamp_2d,     0};
uint16 rbanana_2d[] = {kNBanana_2d,   0};
uint16 rbell_2d[]   = {kNBell_2d,     0};
uint16 rcatnip_2d[] = {kNCatnip_2d,   0};
uint16 rgun_2d[]    = {kNGun_2d,      0};
uint16 rpaper_2d[]  = {kNPaper_2d,    0};
uint16 rpencil_2d[] = {kNPencil_2d,   0};
uint16 rmagnify_2d[]= {kNMagnify_2d,  0};
uint16 rwill_2d[]   = {kNWill_2d,     0};
uint16 rserum_2d[]  = {kNBottle_2d,   0};

const uint16 *arrayReqs_2d[] = {
	rDummy,    rgarlic_2d, rmatch_2d,   rstick_2d,  rdyn_2d,
	rlamp_2d,  rbanana_2d, rbell_2d,    rcatnip_2d, rgun_2d,
	rpaper_2d, rpencil_2d, rmagnify_2d, rwill_2d,   rserum_2d
};

uint16 rpins_3d[]    = {kNNeedles_3d, 0};
uint16 rcheese_3d[]  = {kNCheese_3d,  0};
uint16 rcrystal_3d[] = {kNCrystal_3d, 0};
uint16 rexor_3d[]    = {kNBell_3d,    kNBook_3d, kNCandle_3d, 0};
uint16 rbook_3d[]    = {kNBook_3d,    0};
uint16 rbell_3d[]    = {kNBell_3d,    0};

const uint16 *arrayReqs_3d[] = {rDummy, rpins_3d, rcheese_3d, rcrystal_3d, rexor_3d, rbook_3d, rbell_3d};


// List of 'hotspots' which are areas in which actions are taken if hero
// steps on them.  Note that they are only checked when hero collides with
// a boundary, therefore they must always contain a boundary.
// Structure is: screen of hotspot, coord box x1,y1,x2,y2, action list,
// [+ viewx (or -1), viewy, direction for exits only]

hotspot_t hotspots_1w[] = {
	{0,   30, 132,  50, 168, kALscr1_1w,    33,  175, Common::KEYCODE_UP},     // Front of house to hall
	{1,   99, 178, 136, 186, kALscr10_1w,   110, 181, Common::KEYCODE_DOWN},   // Hall to front of house
	{1,  288, 110, 315, 175, kALscr13_1w,   290, 171, Common::KEYCODE_RIGHT},  // Hall to dining room
	{1,  199, 110, 243, 158, kALscr15_1w,   211, 155, Common::KEYCODE_UP},     // Hall to kitchen
	{1,  268,  53, 302, 101, kALscr115_1w,  273, 98,  Common::KEYCODE_RIGHT},  // Hall to lab
	{2,  158, 132, 193, 140, kALscr21_1w,   168, 132, Common::KEYCODE_DOWN},   // Bed1 to hall
	{3,    3,  73,  14, 168, kALscr31_1w,   12,  161, Common::KEYCODE_LEFT},   // Dining room to hall
	{3,   47,  66,  63, 126, kALscr35_1w,   62,  124, Common::KEYCODE_LEFT},   // Dining room to kitchen
	{4,  126, 165, 160, 173, kALscr41_1w,   135, 166, Common::KEYCODE_DOWN},   // Bathroom to hall
	{5,  215, 169, 272, 177, kALscr51_1w,   242, 172, Common::KEYCODE_DOWN},   // Kitchen to hall
	{5,  262,  79, 280, 146, kALscr53_1w,   270, 140, Common::KEYCODE_RIGHT},  // Kitchen to dining room
	{5,  213,  79, 244, 133, kALscr56_1w,   222, 140, Common::KEYCODE_UP},     // Kitchen to garden
	{5,   25,  87,  43, 157, kALscr57_1w,   44,  150, Common::KEYCODE_LEFT},   // Kitchen to store room
	{6,  232, 180, 312, 192, kALscr65_1w,   250, 180, Common::KEYCODE_DOWN},   // Garden to kitchen
	{7,  263,  90, 285, 156, kALscr75_1w,   265, 150, Common::KEYCODE_RIGHT},  // Store room to kitchen
	{8,  150, 141, 186, 143, kALscr89_1w,   152, 142, Common::KEYCODE_RIGHT},  // Basement to batcave
	{8,   80,  30, 144,  58, kALscr87_1w,   120, 60,  Common::KEYCODE_UP},     // Basement to storeroom
	{9,   95,  74, 147, 109, kALscr910_1w,  108, 108, Common::KEYCODE_UP},     // Batcave to mummy room
	{9,  250, 177, 319, 185, kALscr98_1w,   275, 179, Common::KEYCODE_DOWN},   // Batcave to basement
	{10,  30, 178, 218, 188, kALscr109_1w,  107, 178, Common::KEYCODE_DOWN},   // Mummy room to batcave
	{10, 258,  57, 282, 122, kALscr1011_1w, 265, 125, Common::KEYCODE_UP},     // Mummy room to lake room
	{11,  43, 180,  88, 193, kALscr1110_1w, 55,  182, Common::KEYCODE_DOWN},   // Lake room to mummy room
	{11, 300,  30, 319,  86, kALscr1112_1w, 300, 70,  Common::KEYCODE_RIGHT},  // Lake room to dead-end
	{12,  52, 175, 295, 190, kALscr1211_1w, 142, 176, Common::KEYCODE_DOWN},   // Dead-end to lake room
	{12, 137,  33, 178,  80, kALscr1213_1w, 152, 75,  Common::KEYCODE_UP},     // Dead-end to jail
	{15,  19, 110,  34, 182, kALscr151_1w,  30,  177, Common::KEYCODE_LEFT},   // Laboratory to hall
	{15, 109, 150, 144, 156, kALbox_1w,     -1,  -1,  -1},                     // Professor's box
	{-1,  -1,  -1,  -1,  -1, 0,             -1,  -1,  -1}                      // End of list marker
};

hotspot_t hotspots_2w[] = {
	{2,   81, 177, 124, 190, kALscr0201_2w,  93,  178, Common::KEYCODE_DOWN},  // bed1 to hall
	{3,    1, 100,  32, 168, kALscr0305_2w,  30,  162, Common::KEYCODE_LEFT},  // bed2 to bed3
	{3,  230, 182, 277, 194, kALscr0301_2w,  242, 183, Common::KEYCODE_DOWN},  // bed2 to hall
	{5,  207,  89, 240, 150, kALscr0503_2w,  209, 143, Common::KEYCODE_RIGHT}, // bed3 to bed2
	{5,  167, 155, 214, 165, kALscr0301_2w,  182, 157, Common::KEYCODE_DOWN},  // bed3 to hall (same as bed2 to hall)
	{6,  201,  80, 238, 132, kALscr0607_2w,  217, 130, Common::KEYCODE_UP},    // kitchen to backdoor
	{6,  279,  92, 313, 162, kALscr0631_2w,  289, 156, Common::KEYCODE_RIGHT}, // kitchen to parlor
	{7,  122,  95, 171, 159, kALscr0706_2w,  140, 157, Common::KEYCODE_UP},    // backdoor to kitchen
	{7,    4, 179, 302, 199, kALscr0708_2w,  208, 181, Common::KEYCODE_DOWN},  // backdoor to shed
	{7,    0, 174,  14, 190, kALscr0708_2w,  80,  181, Common::KEYCODE_LEFT},  // backdoor to shed (again)
	{7,  294, 150, 319, 199, kALscr0710_2w,  300, 174, Common::KEYCODE_RIGHT}, // backdoor to venus fly traps
	{8,    0, 121,  20, 171, kALscr0807_2w,  17,  148, Common::KEYCODE_LEFT},  // shed to backdoor
	{8,  305, 121, 319, 171, kALscrgate1_2w, 307, 152, Common::KEYCODE_RIGHT}, // shed to gates
	{8,  160,  87, 191, 134, kALscr0809_2w,  164, 132, Common::KEYCODE_RIGHT}, // shed to inside shed
	{9,   34,  76,  66, 137, kALscr0908_2w,  64,  133, Common::KEYCODE_LEFT},  // inside shed to shed
	{10,   0,  96,  15, 160, kALscr1007_2w,  10,  132, Common::KEYCODE_LEFT},  // venus to backdoor
	{10, 299,  96, 319, 160, kALscrgate1_2w, 301, 124, Common::KEYCODE_RIGHT}, // venus to gates
	{10,  32,  97, 298, 158, kALvenus_2w,    -1,  -1,  -1},                    // venus fly traps
	{11,   0, 155,  12, 185, kALscr1108_2w,  11,  172, Common::KEYCODE_LEFT},  // gates (open) to shed
	{12,   0, 155,  12, 185, kALscr1108_2w,  11,  172, Common::KEYCODE_LEFT},  // gates (close) to shed
	{11, 300, 157, 319, 185, kALscr1113_2w,  301, 172, Common::KEYCODE_RIGHT}, // gates (open) to stream
	{12, 300, 157, 319, 185, kALscr1113_2w,  301, 172, Common::KEYCODE_RIGHT}, // gates (close) to stream
	{11, 145, 103, 195, 145, kAL11maze_2w,   167, 143, Common::KEYCODE_UP},    // gatesopn to maze
	{13,   0, 133,  14, 163, kALscrgate2_2w, 14,  145, Common::KEYCODE_LEFT},  // stream to gates
	{13, 303, 146, 319, 173, kALscr1314_2w,  305, 153, Common::KEYCODE_RIGHT}, // stream to zapper
	{13, 158, 115, 226, 147, kALbridge_2w,   -1,  -1,  -1},                    // bridge over stream
	{14,   0,  96,  14, 160, kALscr1413_2w,  10,  134, Common::KEYCODE_LEFT},  // zapper to stream
	{14, 301,  96, 319, 160, kALscr1415_2w,  302, 134, Common::KEYCODE_RIGHT}, // zapper to mushroom
	{15,   0,  96,  16, 130, kALscr1514_2w,  14,  130, Common::KEYCODE_LEFT},  // mushroom to zapper
	{15, 296,  96, 312, 130, kALscr1517_2w,  298, 127, Common::KEYCODE_RIGHT}, // mushroom to snakepit
	{15,  60, 182, 256, 196, kALscr1516_2w,  157, 183, Common::KEYCODE_DOWN},  // mushroom to well
	{16,  83,  48, 144,  69, kALscr1615_2w,  106, 69,  Common::KEYCODE_UP},    // well to mushroom
	{17,   0, 128,  20, 152, kALscr1715_2w,  19,  143, Common::KEYCODE_LEFT},  // snakepit to mushroom
	{17, 303, 144, 319, 172, kALscr1718_2w,  305, 150, Common::KEYCODE_RIGHT}, // snakepit to phonebox
	{17, 301,  98, 319, 115, kALscr1720_2w,  303, 110, Common::KEYCODE_RIGHT}, // snakepit to kennel
	{18,   0,  54,  15, 120, kALscr1817_2w,  12,  108, Common::KEYCODE_LEFT},  // phonebox to snakepit
	{18,   0, 122,  15, 181, kALscr1819l_2w, 13,  136, Common::KEYCODE_LEFT},  // phonebox to street (left)
	{18,   0, 175, 319, 199, kALscr1819c_2w, 155, 177, Common::KEYCODE_DOWN},  // phonebox to street (center)
	{18, 304,  95, 319, 199, kALscr1819r_2w, 307, 136, Common::KEYCODE_RIGHT}, // phonebox to street (right)
	{18,  15, 122,  56, 130, kALphonebox_2w, -1,  -1,  -1},                    // in the phonebox
	{19,   0, 122,  20, 183, kALscr1918l_2w, 19,  131, Common::KEYCODE_LEFT},  // street to phonebox (left)
	{19,   0, 175, 319, 199, kALscr1918c_2w, 153, 175, Common::KEYCODE_DOWN},  // street to phonebox (center)
	{19, 301, 122, 319, 199, kALscr1918r_2w, 304, 131, Common::KEYCODE_RIGHT}, // street to phonebox (right)
	{20,   5, 103,  22, 186, kALscr2017_2w,  16,  145, Common::KEYCODE_LEFT},  // kennel to snakepit
	{22, 280,  41, 312, 102, kALscr2223_2w,  289, 92,  Common::KEYCODE_RIGHT}, // rockgone to threeway
	{23,  59, 179, 245, 193, kALscr2322_2w,  146, 179, Common::KEYCODE_DOWN},  // threeway to rockgone
	{23,  11,  42,  33, 103, kALscr2324_2w,  18,  90,  Common::KEYCODE_LEFT},  // threeway to lampcave
	{23, 141,  28, 178,  65, kALscr2325_2w,  150, 58,  Common::KEYCODE_UP},    // threeway to chasm
	{23, 273,  40, 296, 100, kALscr2326_2w,  291, 87,  Common::KEYCODE_RIGHT}, // threeway to passage
	{24, 300,  83, 319, 183, kALscr2423_2w,  304, 120, Common::KEYCODE_RIGHT}, // lampcave to threeway
	{25,   0,  98,  15, 171, kALscr2523_2w,  14,  167, Common::KEYCODE_LEFT},  // chasm to threeway
	{25, 172,  95, 221, 172, kALchasm_2w,    -1,  -1,  -1},                    // chasm
	{26,   0,  94,  19, 179, kALscr2623_2w,  17,  157, Common::KEYCODE_LEFT},  // passage to threeway
	{26, 300,  87, 319, 179, kALscr2627_2w,  302, 157, Common::KEYCODE_RIGHT}, // passage to ladder
	{27,   0, 100,  15, 180, kALscr2726_2w,  14,  152, Common::KEYCODE_LEFT},  // ladder to passage
	{27, 200,  23, 243,  55, kALtrap_2w,     210, 100, Common::KEYCODE_UP},    // trapdoor at top of ladder
	{28, 218, 143, 242, 149, kALscr2827_2w,  225, 147, Common::KEYCODE_UP},    // traproom to ladder
	{28,  35,  96,  74, 153, kALscr2829_2w,  71,  150, Common::KEYCODE_LEFT},  // traproom to hall2
	{29, 274,  55, 319, 142, kALscr2928_2w,  283, 134, Common::KEYCODE_RIGHT}, // hall2 to traproom
	{29, 235,  41, 275, 106, kALscr2930_2w,  242, 100, Common::KEYCODE_RIGHT}, // hall2 to lounge
	{29,  81,  28, 108,  74, kALscr2931_2w,  90,  71,  Common::KEYCODE_UP},    // hall2 to parlor
	{29,  15,  71,  54,  95, kALscr2934_2w,  48,  84,  Common::KEYCODE_LEFT},  // hall2 to hall3
	{29,   0,  57,  20, 144, kALscr2938_2w,  19,  135, Common::KEYCODE_LEFT},  // hall2 to hall1
	{30, 222, 183, 258, 199, kALscr3029_2w,  232, 183, Common::KEYCODE_DOWN},  // lounge to hall2
	{31, 263,  90, 287, 146, kALscr3132_2w,  266, 143, Common::KEYCODE_RIGHT}, // parlor to catroom
	{31, 172, 164, 211, 178, kALscr3129_2w,  185, 165, Common::KEYCODE_DOWN},  // parlor to hall2
	{31,   0,  97,  46, 158, kALscr3106_2w,  35,  154, Common::KEYCODE_LEFT},  // parlor to kitchen
	{32,  60,  73,  90, 123, kALscr3231_2w,  83,  120, Common::KEYCODE_LEFT},  // catroom to parlor
	{34,   6, 100,  39, 179, kALscr3438_2w,  38,  172, Common::KEYCODE_LEFT},  // hall3 to hall1
	{34,   7, 180, 311, 199, kALscr3429_2w,  168, 186, Common::KEYCODE_DOWN},  // hall3 to hall2
	{35,  65, 176, 101, 188, kALscr3534_2w,  74,  178, Common::KEYCODE_DOWN},  // organ to hall3
	{36, 238, 164, 272, 177, kALscr3634_2w,  250, 166, Common::KEYCODE_DOWN},  // hestroom to hall3
	{36,  62, 140, 158, 158, kALhtable_2w,   -1,  -1,  -1},                    // hestroom table
	{37, 253,  82, 286,  84, kALscr3718_2w,  254, 83,  Common::KEYCODE_RIGHT}, // retupmoc to phonebox
	{38, 275, 116, 307, 166, kALscr3829_2w,  279, 164, Common::KEYCODE_RIGHT}, // hall1 to hall2
	{38, 142, 108, 172, 150, kALscr3834_2w,  152, 150, Common::KEYCODE_UP},    // hall1 to hall3
	{MAZE_SCREEN + 59, 100, 140, 180, 160, kALexitmaze_2w, 133, 143, Common::KEYCODE_DOWN},  // Exit maze
	{-1, -1,  -1,  -1,  -1,  0,              -1,  -1,  -1}                     // End of list marker
};

hotspot_t hotspots_3w[] = {
	{CRASH_3w,      0, 163,  30, 190, kALcrash_web_3w,       11, 176, Common::KEYCODE_LEFT},
	{CRASH_3w,    164, 102, 205, 144, kALplane_3w,          178, 153, Common::KEYCODE_UP},
	{PLANE_3w,    120, 126, 220, 170, kALexit_3w,             0,   0, Common::KEYCODE_DOWN},  // Hero invisible so exit is instant
	{WEB_3w,      296,  73, 319, 190, kALweb_crash_3w,      307, 149, Common::KEYCODE_RIGHT},
	{WEB_3w,        0, 156,  30, 190, kALweb_path_3w,        15, 182, Common::KEYCODE_LEFT},
	{PATH_UL_3w,  293, 161, 319, 190, kALpath_web_3w,       305, 181, Common::KEYCODE_RIGHT},
	{PATH_UL_3w,    0, 127,  24, 138, kALpath_brg_3w,        12, 137, Common::KEYCODE_LEFT},
	{BRIDGE_3w,   296, 125, 319, 138, kALbrg_path_3w,       309, 135, Common::KEYCODE_RIGHT},
	{BRIDGE_3w,     0, 125,  21, 138, kALbrg_clftop_3w,      10, 135, Common::KEYCODE_LEFT},
	{BRIDGE_3w,   225, 132, 240, 135, kALblk_3w,             -1, -1, -1},
	{BRIDGE2_3w,  296, 125, 319, 138, kALbrg_path_3w,       309, 135, Common::KEYCODE_RIGHT},
	{BRIDGE2_3w,    0, 125,  21, 138, kALbrg_clftop_3w,      10, 135, Common::KEYCODE_LEFT},
	{CLIFFTOP_3w, 293,  69, 319,  79, kALclftop_brg_3w,     303,  77, Common::KEYCODE_RIGHT},
	{CLIFFTOP_3w, 294, 131, 319, 149, kALclftop_slope_3w,   304, 136, Common::KEYCODE_RIGHT},
	{CLIFFTOP_3w, 290, 171, 319, 186, kALclftop_clf_3w,     303, 176, Common::KEYCODE_RIGHT},
	{SLOPE_3w,      0, 147,  25, 165, kALslope_clftop_3w,    13, 164, Common::KEYCODE_LEFT},
	{SLOPE_3w,    291, 127, 319, 143, kALslope_stream_3w,   303, 141, Common::KEYCODE_RIGHT},
	{CLIFF_3w,      0, 152,  26, 190, kALclf_clftop_3w,      15, 183, Common::KEYCODE_LEFT},
	{CLIFF_3w,    293, 152, 319, 190, kALclf_wfall_3w,      303, 183, Common::KEYCODE_RIGHT},
	{STREAM_3w,     0, 160,  26, 190, kALstream_slope_3w,    13, 177, Common::KEYCODE_LEFT},
	{STREAM_3w,   293, 163, 319, 190, kALstream_path_3w,    307, 177, Common::KEYCODE_RIGHT},
	{STREAM_3w,   198, 140, 245, 160, kALfindbook_3w,        -1, -1, -1},
	{STREAM2_3w,    0, 160,  26, 190, kALstream_slope_3w,    13, 177, Common::KEYCODE_LEFT},
	{STREAM2_3w,  293, 163, 319, 190, kALstream_path_3w,    307, 177, Common::KEYCODE_RIGHT},
	{STREAM2_3w,  198, 140, 245, 160, kALfindbook_3w,        -1, -1, -1},
	{PATH_3w,     292, 161, 319, 190, kALpath_village_3w,   304, 184, Common::KEYCODE_RIGHT},
	{PATH_3w,       0, 161,  24, 190, kALpath_stream_3w,     13, 184, Common::KEYCODE_LEFT},
	{VILLAGE_3w,   14, 137,  41, 156, kALvillage_thing_3w,   -1, -1, -1},
	{VILLAGE_3w,    0, 148,  24, 190, kALvillage_path_3w,    14, 175, Common::KEYCODE_LEFT},
	{VILLAGE_3w,  292, 161, 319, 190, kALvillage_turn_3w,   307, 175, Common::KEYCODE_RIGHT},
	{VILLAGE_3w,   14, 126, 111, 155, kALvillage_camp_l_3w,  90, 153, Common::KEYCODE_UP},
	{VILLAGE_3w,  112, 126, 252, 155, kALvillage_camp_r_3w, 162, 153, Common::KEYCODE_UP},
	{HUT_OUT_3w,    0, 147,  30, 190, kALhut_camp_3w,        14, 177, Common::KEYCODE_LEFT},
	{HUT_OUT_3w,  290, 147, 319, 190, kALhut_village_r_3w,  304, 177, Common::KEYCODE_RIGHT}, // Exit right
	{HUT_OUT_3w,   14, 178, 319, 190, kALhut_village_c_3w,  150, 189, Common::KEYCODE_DOWN},  // Exit center
	{HUT_OUT_3w,  132, 126, 177, 145, kALhut_enter_3w,      150, 136, Common::KEYCODE_UP},
	{HUT_IN_3w,   138, 183, 189, 199, kALhut_out_3w,        153, 189, Common::KEYCODE_DOWN},
	{CAMP_3w,     290, 136, 319, 190, kALcamp_hut_3w,       303, 165, Common::KEYCODE_RIGHT},
	{CAMP_3w,       0, 109,  27, 190, kALcamp_village_l_3w,  13, 155, Common::KEYCODE_LEFT},  // exit left
	{CAMP_3w,      13, 180, 303, 199, kALcamp_village_c_3w, 130, 190, Common::KEYCODE_DOWN},  // exit center
	{TURN_3w,       0, 114,  28, 133, kALturn_village_3w,    14, 131, Common::KEYCODE_LEFT},
	{TURN_3w,       0, 165,  28, 190, kALturn_cave_3w,       13, 180, Common::KEYCODE_LEFT},
	{CAVE_3w,     291, 140, 319, 190, kALcave_turn_3w,      307, 184, Common::KEYCODE_RIGHT},
	{CAVE_3w,       0, 146,  27, 190, kALcave_wfall_3w,      14, 184, Common::KEYCODE_LEFT},
	{CAVE_3w,      86, 128, 162, 141, kALcave_oldman_3w,    114, 134, Common::KEYCODE_UP},
	{WFALL_3w,      0, 166,  28, 190, kALwfall_clf_3w,       14, 184, Common::KEYCODE_LEFT},
	{WFALL_3w,    294, 159, 319, 190, kALwfall_cave_3w,     306, 184, Common::KEYCODE_RIGHT},
	{WFALL_3w,    238, 147, 284, 158, kALwfall_wbase_3w,    255, 154, Common::KEYCODE_UP},
	{WFALL_B_3w,    0, 166,  28, 190, kALwfallb_clf_3w,      14, 184, Common::KEYCODE_LEFT},
	{WFALL_B_3w,  294, 159, 319, 190, kALwfallb_cave_3w,    306, 184, Common::KEYCODE_RIGHT},
	{WFALL_B_3w,  238, 147, 284, 158, kALwfallb_wbase_3w,   255, 154, Common::KEYCODE_UP},
	{WBASE_3w,    295, 146, 319, 190, kALwbase_wfall_3w,    306, 154, Common::KEYCODE_RIGHT},
	{WBASE_3w,      0, 149,  26, 190, kALwbase_garden_3w,    14, 154, Common::KEYCODE_LEFT},
	{GARDEN_3w,   294, 165, 319, 190, kALgarden_wbase_3w,   306, 177, Common::KEYCODE_RIGHT},
	{-1,          -1,  -1,  -1,  -1,  0,                    -1,  -1,  -1} // End of list marker
};

hotspot_t hotspots_1d[] = {
	{0,  30,  132, 50,  165, kALscr1_1d,    33,  175, Common::KEYCODE_UP},    // Front of house to hall
	{1,  99,  178, 136, 186, kALscr10_1d,   110, 181, Common::KEYCODE_DOWN},  // Hall to front of house
	{1,  288, 110, 315, 175, kALscr13_1d,   290, 171, Common::KEYCODE_RIGHT}, // Hall to dining room
	{1,  199, 110, 243, 158, kALscr15_1d,   211, 155, Common::KEYCODE_UP},    // Hall to kitchen
	{1,  268, 53,  302, 101, kALscr115_1d,  273, 98,  Common::KEYCODE_RIGHT}, // Hall to lab
	{2,  158, 132, 193, 140, kALscr21_1d,   168, 132, Common::KEYCODE_DOWN},  // Bed1 to hall
	{3,  3,   73,  14,  168, kALscr31_1d,   12,  161, Common::KEYCODE_LEFT},  // Dining room to hall
	{3,  47,  66,  63,  126, kALscr35_1d,   62,  124, Common::KEYCODE_LEFT},  // Dining room to kitchen
	{4,  126, 165, 160, 173, kALscr41_1d,   135, 166, Common::KEYCODE_DOWN},  // Bathroom to hall
	{5,  215, 169, 272, 177, kALscr51_1d,   242, 172, Common::KEYCODE_DOWN},  // Kitchen to hall
	{5,  262, 79,  280, 146, kALscr53_1d,   270, 140, Common::KEYCODE_RIGHT}, // Kitchen to dining room
	{5,  212, 79,  244, 133, kALscr56_1d,   222, 140, Common::KEYCODE_UP},    // Kitchen to garden
	{5,  25,  87,  43,  157, kALscr57_1d,   44,  150, Common::KEYCODE_LEFT},  // Kitchen to store room
	{6,  232, 180, 312, 192, kALscr65_1d,   250, 180, Common::KEYCODE_DOWN},  // Garden to kitchen
	{7,  263, 90,  285, 156, kALscr75_1d,   265, 150, Common::KEYCODE_RIGHT}, // Store room to kitchen
	{8,  150, 141, 186, 143, kALscr89_1d,   152, 142, Common::KEYCODE_RIGHT}, // Basement to batcave
	{8,  80,  30,  144, 58,  kALscr87_1d,   120, 60,  Common::KEYCODE_UP},    // Basement to storeroom
	{9,  95,  74,  147, 109, kALscr910_1d,  108, 108, Common::KEYCODE_UP},    // Batcave to mummy room
	{9,  250, 177, 319, 185, kALscr98_1d,   275, 179, Common::KEYCODE_DOWN},  // Batcave to basement
	{10, 30,  178, 218, 188, kALscr109_1d,  107, 178, Common::KEYCODE_DOWN},  // Mummy room to batcave
	{10, 258, 57,  282, 122, kALscr1011_1d, 265, 125, Common::KEYCODE_UP},    // Mummy room to lake room
	{11, 43,  180, 88,  193, kALscr1110_1d, 55,  182, Common::KEYCODE_DOWN},  // Lake room to mummy room
	{11, 300, 30,  319, 86,  kALscr1112_1d, 300, 70,  Common::KEYCODE_RIGHT}, // Lake room to dead-end
	{12, 52,  175, 295, 190, kALscr1211_1d, 142, 176, Common::KEYCODE_DOWN},  // Dead-end to lake room
	{12, 137, 33,  178, 80,  kALscr1213_1d, 152, 75,  Common::KEYCODE_UP},    // Dead-end to jail
	{15, 19,  110, 34,  182, kALscr151_1d,  30,  177, Common::KEYCODE_LEFT},  // Laboratory to hall
	{15, 109, 150, 144, 156, kALbox_1d,     -1,  -1,  -1},                    // Professor's box
	{-1, -1,  -1,  -1,  -1,  0,             -1,  -1,  -1}                     // End of list marker
};

hotspot_t hotspots_2d[] = {
	{2,  81,  177, 124, 190, kALscr0201_2d,  93,  178, Common::KEYCODE_DOWN},  // bed1 to hall
	{3,  1,   100, 32,  168, kALscr0305_2d,  30,  162, Common::KEYCODE_LEFT},  // bed2 to bed3
	{3,  230, 182, 277, 194, kALscr0301_2d,  242, 183, Common::KEYCODE_DOWN},  // bed2 to hall
	{5,  207, 89,  240, 150, kALscr0503_2d,  209, 143, Common::KEYCODE_RIGHT}, // bed3 to bed2
	{5,  167, 155, 214, 165, kALscr0301_2d,  182, 157, Common::KEYCODE_DOWN},  // bed3 to hall (same as bed2 to hall)
	{6,  201, 80,  238, 132, kALscr0607_2d,  217, 130, Common::KEYCODE_UP},    // kitchen to backdoor
	{6,  279, 92,  313, 162, kALscr0631_2d,  289, 156, Common::KEYCODE_RIGHT}, // kitchen to parlor
	{7,  122, 95,  171, 159, kALscr0706_2d,  140, 157, Common::KEYCODE_UP},    // backdoor to kitchen
	{7,  4,   179, 302, 199, kALscr0708_2d,  208, 181, Common::KEYCODE_DOWN},  // backdoor to shed
	{7,  0,   174, 14,  190, kALscr0708_2d,  80,  181, Common::KEYCODE_LEFT},  // backdoor to shed (again)
	{7,  294, 150, 319, 199, kALscr0710_2d,  300, 174, Common::KEYCODE_RIGHT}, // backdoor to venus fly traps
	{8,  0,   121, 20,  171, kALscr0807_2d,  17,  148, Common::KEYCODE_LEFT},  // shed to backdoor
	{8,  305, 121, 319, 171, kALscrgate1_2d, 307, 152, Common::KEYCODE_RIGHT}, // shed to gates
	{8,  160, 87,  191, 134, kALscr0809_2d,  164, 132, Common::KEYCODE_RIGHT}, // shed to inside shed
	{9,  34,  76,  66,  137, kALscr0908_2d,  64,  133, Common::KEYCODE_LEFT},  // inside shed to shed
	{10, 0,   96,  15,  160, kALscr1007_2d,  10,  132, Common::KEYCODE_LEFT},  // venus to backdoor
	{10, 299, 96,  319, 160, kALscrgate1_2d, 301, 124, Common::KEYCODE_RIGHT}, // venus to gates
	{10, 32,  97,  298, 158, kALvenus_2d,    -1,  -1,  -1},                    // venus fly traps
	{11, 0,   155, 12,  185, kALscr1108_2d,  11,  172, Common::KEYCODE_LEFT},  // gates (open) to shed
	{12, 0,   155, 12,  185, kALscr1108_2d,  11,  172, Common::KEYCODE_LEFT},  // gates (close) to shed
	{11, 300, 157, 319, 185, kALscr1113_2d,  301, 172, Common::KEYCODE_RIGHT}, // gates (open) to stream
	{12, 300, 157, 319, 185, kALscr1113_2d,  301, 172, Common::KEYCODE_RIGHT}, // gates (close) to stream
	{11, 145, 133, 195, 145, kAL11maze_2d,   167, 143, Common::KEYCODE_UP},    // gatesopn to maze
	{13, 0,   133, 14,  163, kALscrgate2_2d, 14,  145, Common::KEYCODE_LEFT},  // stream to gates
	{13, 303, 146, 319, 173, kALscr1314_2d,  305, 153, Common::KEYCODE_RIGHT}, // stream to zapper
	{13, 158, 115, 226, 147, kALbridge_2d,   -1,  -1,  -1},                    // bridge over stream
	{14, 0,   96,  14,  160, kALscr1413_2d,  10,  134, Common::KEYCODE_LEFT},  // zapper to stream
	{14, 301, 96,  319, 160, kALscr1415_2d,  302, 134, Common::KEYCODE_RIGHT}, // zapper to mushroom
	{15, 0,   96,  16,  130, kALscr1514_2d,  14,  130, Common::KEYCODE_LEFT},  // mushroom to zapper
	{15, 296, 96,  312, 130, kALscr1517_2d,  298, 127, Common::KEYCODE_RIGHT}, // mushroom to snakepit
	{15, 60,  182, 256, 196, kALscr1516_2d,  157, 183, Common::KEYCODE_DOWN},  // mushroom to well
	{16, 83,  48,  144, 69,  kALscr1615_2d,  106, 69,  Common::KEYCODE_UP},    // well to mushroom
	{17, 0,   128, 20,  152, kALscr1715_2d,  19,  143, Common::KEYCODE_LEFT},  // snakepit to mushroom
	{17, 303, 144, 319, 172, kALscr1718_2d,  305, 150, Common::KEYCODE_RIGHT}, // snakepit to phonebox
	{17, 301, 98,  319, 115, kALscr1720_2d,  303, 110, Common::KEYCODE_RIGHT}, // snakepit to kennel
	{18, 0,   54,  15,  120, kALscr1817_2d,  12,  108, Common::KEYCODE_LEFT},  // phonebox to snakepit
	{18, 0,   122, 15,  181, kALscr1819l_2d, 13,  136, Common::KEYCODE_LEFT},  // phonebox to street (left)
	{18, 0,   175, 319, 199, kALscr1819c_2d, 155, 177, Common::KEYCODE_DOWN},  // phonebox to street (center)
	{18, 304, 95,  319, 199, kALscr1819r_2d, 307, 136, Common::KEYCODE_RIGHT}, // phonebox to street (right)
	{18, 15,  122, 56,  130, kALphonebox_2d, -1,  -1,  -1},                    // in the phonebox
	{19, 0,   122, 20,  183, kALscr1918l_2d, 19,  131, Common::KEYCODE_LEFT},  // street to phonebox (left)
	{19, 0,   175, 319, 199, kALscr1918c_2d, 153, 175, Common::KEYCODE_DOWN},  // street to phonebox (center)
	{19, 301, 122, 319, 199, kALscr1918r_2d, 304, 131, Common::KEYCODE_RIGHT}, // street to phonebox (right)
	{20, 5,   103, 22,  186, kALscr2017_2d,  16,  145, Common::KEYCODE_LEFT},  // kennel to snakepit
	{22, 280, 41,  312, 102, kALscr2223_2d,  289, 92,  Common::KEYCODE_RIGHT}, // rockgone to threeway
	{23, 59,  179, 245, 193, kALscr2322_2d,  146, 179, Common::KEYCODE_DOWN},  // threeway to rockgone
	{23, 11,  42,  33,  103, kALscr2324_2d,  18,  90,  Common::KEYCODE_LEFT},  // threeway to lampcave
	{23, 141, 28,  178, 65,  kALscr2325_2d,  150, 58,  Common::KEYCODE_UP},    // threeway to chasm
	{23, 273, 40,  296, 100, kALscr2326_2d,  291, 87,  Common::KEYCODE_RIGHT}, // threeway to passage
	{24, 300, 83,  319, 183, kALscr2423_2d,  304, 120, Common::KEYCODE_RIGHT}, // lampcave to threeway
	{25, 0,   98,  15,  171, kALscr2523_2d,  14,  167, Common::KEYCODE_LEFT},  // chasm to threeway
	{25, 172, 95,  221, 172, kALchasm_2d,    -1,  -1,  -1},                    // chasm
	{26, 0,   94,  19,  179, kALscr2623_2d,  17,  157, Common::KEYCODE_LEFT},  // passage to threeway
	{26, 300, 87,  319, 179, kALscr2627_2d,  302, 157, Common::KEYCODE_RIGHT}, // passage to ladder
	{27, 0,   100, 15,  180, kALscr2726_2d,  14,  152, Common::KEYCODE_LEFT},  // ladder to passage
	{27, 200, 43,  243, 55,  kALtrap_2d,     210, 100, Common::KEYCODE_UP},    // trapdoor at top of ladder
	{28, 218, 143, 242, 149, kALscr2827_2d,  225, 147, Common::KEYCODE_UP},    // traproom to ladder
	{28, 35,  96,  74,  153, kALscr2829_2d,  71,  150, Common::KEYCODE_LEFT},  // traproom to hall2
	{29, 274, 55,  319, 142, kALscr2928_2d,  283, 134, Common::KEYCODE_RIGHT}, // hall2 to traproom
	{29, 235, 41,  275, 106, kALscr2930_2d,  242, 100, Common::KEYCODE_RIGHT}, // hall2 to lounge
	{29, 81,  28,  108, 74,  kALscr2931_2d,  90,  71,  Common::KEYCODE_UP},    // hall2 to parlor
	{29, 15,  71,  54,  95,  kALscr2934_2d,  48,  84,  Common::KEYCODE_LEFT},  // hall2 to hall3
	{29, 0,   57,  20,  144, kALscr2938_2d,  19,  135, Common::KEYCODE_LEFT},  // hall2 to hall1
	{30, 222, 183, 258, 199, kALscr3029_2d,  232, 183, Common::KEYCODE_DOWN},  // lounge to hall2
	{31, 263, 90,  287, 146, kALscr3132_2d,  266, 143, Common::KEYCODE_RIGHT}, // parlor to catroom
	{31, 172, 164, 211, 178, kALscr3129_2d,  185, 165, Common::KEYCODE_DOWN},  // parlor to hall2
	{31, 0,   97,  46,  158, kALscr3106_2d,  35,  154, Common::KEYCODE_LEFT},  // parlor to kitchen
	{32, 60,  73,  90,  123, kALscr3231_2d,  83,  120, Common::KEYCODE_LEFT},  // catroom to parlor
	{34, 6,   100, 39,  179, kALscr3438_2d,  38,  172, Common::KEYCODE_LEFT},  // hall3 to hall1
	{34, 7,   180, 311, 199, kALscr3429_2d,  168, 186, Common::KEYCODE_DOWN},  // hall3 to hall2
	{35, 65,  176, 101, 188, kALscr3534_2d,  74,  178, Common::KEYCODE_DOWN},  // organ to hall3
	{36, 238, 164, 272, 177, kALscr3634_2d,  250, 166, Common::KEYCODE_DOWN},  // hestroom to hall3
	{36, 62,  140, 158, 158, kALhtable_2d,   -1,  -1,  -1},                    // hestroom table
	{37, 253, 82,  286, 84,  kALscr3718_2d,  254, 83,  Common::KEYCODE_RIGHT}, // retupmoc to phonebox
	{38, 275, 116, 307, 166, kALscr3829_2d,  279, 164, Common::KEYCODE_RIGHT}, // hall1 to hall2
	{38, 142, 108, 172, 150, kALscr3834_2d,  152, 150, Common::KEYCODE_UP},    // hall1 to hall3
	{MAZE_SCREEN + 59, 100, 140, 180, 160, kALexitmaze_2d, 133, 143, Common::KEYCODE_DOWN},  // Exit maze
	{-1,  -1,  -1,  -1,  -1, 0,              -1, -1, -1}   // End of list marker
};

hotspot_t hotspots_3d[] = {
	{CRASH_3d,    0,   163, 30,  190, kALcrash_web_3d,      11, 176,  Common::KEYCODE_LEFT},
	{CRASH_3d,    164, 102, 205, 144, kALplane_3d,          178, 153, Common::KEYCODE_UP},    // Added by Strangerke to enter plane cockpit
	{PLANE_3d,    120, 126, 220, 170, kALexit_3d,           0,   0,   Common::KEYCODE_DOWN},  // Added by Strangerke to exit plane cockpit
	{WEB_3d,      296, 73,  319, 190, kALweb_crash_3d,      307, 149, Common::KEYCODE_RIGHT},
	{WEB_3d,      0,   156, 30,  190, kALweb_path_3d,       15,  182, Common::KEYCODE_LEFT},
	{PATH_UL_3d,  293, 161, 319, 190, kALpath_web_3d,       305, 181, Common::KEYCODE_RIGHT},
	{PATH_UL_3d,  0,   127, 24,  138, kALpath_brg_3d,       12,  137, Common::KEYCODE_LEFT},
	{BRIDGE_3d,   296, 125, 319, 138, kALbrg_path_3d,       309, 135, Common::KEYCODE_RIGHT},
	{BRIDGE_3d,   0,   125, 21,  138, kALbrg_clftop_3d,     10,  135, Common::KEYCODE_LEFT},
	{BRIDGE_3d,   225, 132, 235, 135, kALblk_3d,            -1,  -1, -1},
	{BRIDGE2_3d,  296, 125, 319, 138, kALbrg_path_3d,       309, 135, Common::KEYCODE_RIGHT},
	{BRIDGE2_3d,  0,   125, 21,  138, kALbrg_clftop_3d,     10,  135, Common::KEYCODE_LEFT},
	{CLIFFTOP_3d, 293, 69,  319, 79,  kALclftop_brg_3d,     303, 77,  Common::KEYCODE_RIGHT},
	{CLIFFTOP_3d, 294, 131, 319, 149, kALclftop_slope_3d,   304, 136, Common::KEYCODE_RIGHT},
	{CLIFFTOP_3d, 290, 171, 319, 186, kALclftop_clf_3d,     303, 176, Common::KEYCODE_RIGHT},
	{SLOPE_3d,    0,   147, 25,  165, kALslope_clftop_3d,   13,  164, Common::KEYCODE_LEFT},
	{SLOPE_3d,    291, 127, 319, 143, kALslope_stream_3d,   303, 141, Common::KEYCODE_RIGHT},
	{CLIFF_3d,    0,   152, 26,  190, kALclf_clftop_3d,     15,  183, Common::KEYCODE_LEFT},
	{CLIFF_3d,    293, 152, 319, 190, kALclf_wfall_3d,      303, 183, Common::KEYCODE_RIGHT},
	{STREAM_3d,   0,   160, 26,  190, kALstream_slope_3d,   13,  177, Common::KEYCODE_LEFT},
	{STREAM_3d,   293, 163, 319, 190, kALstream_path_3d,    307, 177, Common::KEYCODE_RIGHT},
	{STREAM_3d,   198, 140, 245, 160, kALfindbook_3d,       -1,  -1,  -1},
	{STREAM2_3d,  0,   160, 26,  190, kALstream_slope_3d,   13,  177, Common::KEYCODE_LEFT},
	{STREAM2_3d,  293, 163, 319, 190, kALstream_path_3d,    307, 177, Common::KEYCODE_RIGHT},
	{STREAM2_3d,  198, 140, 245, 160, kALfindbook_3d,       -1,  -1,  -1},
	{PATH_3d,     292, 161, 319, 190, kALpath_village_3d,   304, 184, Common::KEYCODE_RIGHT},
	{PATH_3d,     0,   161, 24,  190, kALpath_stream_3d,    13,  184, Common::KEYCODE_LEFT},
	{VILLAGE_3d,  14,  137, 41,  156, kALvillage_thing_3d,  -1,  -1,  -1},
	{VILLAGE_3d,  0,   148, 24,  190, kALvillage_path_3d,   14,  175, Common::KEYCODE_LEFT},
	{VILLAGE_3d,  292, 161, 319, 190, kALvillage_turn_3d,   307, 175, Common::KEYCODE_RIGHT},
	{VILLAGE_3d,  14,  126, 111, 155, kALvillage_camp_l_3d, 90,  153, Common::KEYCODE_UP},
	{VILLAGE_3d,  112, 126, 252, 155, kALvillage_camp_r_3d, 162, 153, Common::KEYCODE_UP},
	{HUT_OUT_3d,  0,   147, 30,  190, kALhut_camp_3d,       14,  177, Common::KEYCODE_LEFT},
	{HUT_OUT_3d,  290, 147, 319, 190, kALhut_village_r_3d,  304, 177, Common::KEYCODE_RIGHT}, // Exit right
	{HUT_OUT_3d,  14,  178, 319, 190, kALhut_village_c_3d,  150, 189, Common::KEYCODE_DOWN},  // Exit center
	{HUT_OUT_3d,  132, 126, 177, 145, kALhut_enter_3d,      150, 136, Common::KEYCODE_UP},
	{HUT_IN_3d,   138, 183, 189, 199, kALhut_out_3d,        153, 189, Common::KEYCODE_DOWN},
	{CAMP_3d,     290, 136, 319, 190, kALcamp_hut_3d,       303, 165, Common::KEYCODE_RIGHT},
	{CAMP_3d,     0,   109, 27,  190, kALcamp_village_l_3d, 13,  155, Common::KEYCODE_LEFT},  // exit left
	{CAMP_3d,     13,  180, 303, 199, kALcamp_village_c_3d, 130, 190, Common::KEYCODE_DOWN},  // exit center
	{TURN_3d,     0,   114, 28,  133, kALturn_village_3d,   14,  131, Common::KEYCODE_LEFT},
	{TURN_3d,     0,   165, 28,  190, kALturn_cave_3d,      13,  180, Common::KEYCODE_LEFT},
	{CAVE_3d,     291, 140, 319, 190, kALcave_turn_3d,      307, 184, Common::KEYCODE_RIGHT},
	{CAVE_3d,     0,   146, 27,  190, kALcave_wfall_3d,     14,  184, Common::KEYCODE_LEFT},
	{CAVE_3d,     86,  128, 162, 141, kALcave_oldman_3d,    114, 134, Common::KEYCODE_UP},
	{WFALL_3d,    0,   166, 28,  190, kALwfall_clf_3d,      14,  184, Common::KEYCODE_LEFT},
	{WFALL_3d,    294, 159, 319, 190, kALwfall_cave_3d,     306, 184, Common::KEYCODE_RIGHT},
	{WFALL_3d,    238, 147, 284, 158, kALwfall_wbase_3d,    255, 154, Common::KEYCODE_UP},
	{WFALL_B_3d,  0,   166, 28,  190, kALwfallb_clf_3d,     14,  184, Common::KEYCODE_LEFT},
	{WFALL_B_3d,  294, 159, 319, 190, kALwfallb_cave_3d,    306, 184, Common::KEYCODE_RIGHT},
	{WFALL_B_3d,  238, 147, 284, 158, kALwfallb_wbase_3d,   255, 154, Common::KEYCODE_UP},
	{WBASE_3d,    295, 146, 319, 190, kALwbase_wfall_3d,    306, 154, Common::KEYCODE_RIGHT},
	{WBASE_3d,    0,   149, 26,  190, kALwbase_garden_3d,   14,  154, Common::KEYCODE_LEFT},
	{GARDEN_3d,   294, 165, 319, 190, kALgarden_wbase_3d,   306, 177, Common::KEYCODE_RIGHT},
	{-1,          -1,  -1,  -1,  -1,  0,                    -1,  -1,  -1} // End of list marker               -1, -1, -1}  // End of list marker
};

// List all objects that can appear in inventory list
int16 invent_1w[] = {
	PKIN_1w,     KEY_1w,     CANDLE_1w,  KNIFE_1w,  WHISTLE_1w,
	MASK_1w,     CHOP_1w,    OILCAN_1w,  GOLD_1w,   BUNG_1w,
	-1
};

int16 invent_2w[] = {
	MATCHES_2w,  GARLIC_2w,  MAGNIFY_2w, CATNIP_2w, STICK_2w,
	DYNAMITE_2w, OILLAMP_2w, BANANA_2w,  WILL_2w,   ALBUM_2w,
	PAPER_2w,    PENCIL_2w,  SCREW_2w,   BELL_2w,   GUN_2w,
	BOTTLE_2w,   -1
};

int16 invent_3w[] = {
	CLAY_3w, NEEDLES_3w,  FLASK_3w,  BOUILLON_3w, CHEESE_3w,
	CAGE_3w, BLOWPIPE_3w, SCROLL_3w, CRYSTAL_3w,  BELL_3w,
	BOOK_3w, CANDLE_3w,   -1
};

// TODO: This inventory list is only used in the Win versions. Should we add them in the DOS versions?
int16 invent_1d[] = {-1};
int16 invent_2d[] = {-1};
int16 invent_3d[] = {-1};

uses_t uses_1w[] = {
	{PKIN_1w,    kDTupkin_1w,   {{kNHero_1w, kVOpen_1w},      {0, 0}}},
	{KEY_1w,     kDTukey_1w,    {{kNDoor_1w, kVUnlock_1w},    {0, 0}}},
	{CANDLE_1w,  kDTucandle_1w, {{0, 0}}},
	{MASK_1w,    kDTumasked_1w, {{kNHero_1w, kVMakeUseOf_1w}, {0, 0}}},
	{BUNG_1w,    kDTubung_1w,   {{kNBoat_1w, kVPlug_1w},      {0, 0}}},
	{CHOP_1w,    kDTuchop_1w,   {{kNHero_1w, kVEat_1w},       {kNDog_1w,   kVThrowit_1w},{kNButler_1w, kVGive_1w},{kNFrank_1w, kVGive_1w}, {kNDracula_1w, kVGive_1w}, {kNGwen_1w, kVGive_1w}, {kNHood_1w, kVGive_1w}, {kNSlime_1w, kVGive_1w}, {kNPeahead_1w, kVGive_1w}, {kNProf_1w, kVGive_1w}, {kNIgor_1w, kVGive_1w}, {0, 0}}},
	{KNIFE_1w,   kDTuknife_1w,  {{kNHero_1w, kVCut_1w},       {kNRope_1w,  kVCut_1w},    {0, 0}}},
	{OILCAN_1w,  kDTuoil_1w,    {{kNHero_1w, kVOil_1w},       {kNTrap_1w,  kVBolt_1w},   {kNBolt_1w,   kVOil_1w}, {kNDoor_1w,  kVOil_1w},  {0, 0}}},
	{GOLD_1w,    kDTugold_1w,   {{kNHero_1w, kVMakeUseOf_1w}, {kNGuard_1w, kVGive_1w},   {0, 0}}},
	{LASTOBJ_1w, 0, {{0, 0}}}
};

uses_t uses_2w[] = {
	{MATCHES_2w,  kDTumatches_2w,  {{kNHero_2w, kVStrike_2w},    {kNDynamite_2w, kVMakeUseOf_2w},{0, 0}}},
	{GARLIC_2w,   kDTugarlic_2w,   {{kNHero_2w, kVEat_2w},       {kNGardner_2w,  kVGive_2w},     {kNDog_2w,     kVGive_2w},      {kNCupb_2w,  kVDrop_2w},   {0, 0}}},
	{MAGNIFY_2w,  kDTumagnify_2w,  {{kNWill_2w, kVRead_2w},      {0, 0}}},
	{STICK_2w,    kDTustick_2w,    {{kNHero_2w, kVThrowit_2w},   {kNDog_2w,      kVGive_2w},     {kNSnake_2w,   kVAttack_2w},    {0, 0}}},
	{BOTTLE_2w,   kDTubottle_2w,   {{kNHero_2w, kVDrink_2w},     {kNDog_2w,      kVGive_2w},     {kNSnake_2w,   kVGive_2w},      {0, 0}}},
	{DYNAMITE_2w, kDTudynamite_2w, {{kNHero_2w, kVStrike_2w},    {kNRock_2w,     kVDrop_2w},     {kNMatches_2w, kVMakeUseOf_2w}, {kNSnake_2w, kVAttack_2w}, {0, 0}}},
	{GUN_2w,      kDTugun_2w,      {{kNHero_2w, kVFire_2w},      {kNSnake_2w,    kVAttack_2w},   {kNRobot_2w,   kVFire_2w},      {0, 0}}},
	{BANANA_2w,   kDTubanana_2w,   {{kNHero_2w, kVEat_2w},       {kNGenie_2w,    kVGive_2w},     {0, 0}}},
	{OILLAMP_2w,  kDTulamp_2w,     {{kNHero_2w, kVRub_2w},       {kNGenie_2w,    kVGive_2w},     {0, 0}}},
	{SCREW_2w,    kDTuscrew_2w,    {{kNHero_2w, kVMakeUseOf_2w}, {kNSafe_2w,     kVOpen_2w},     {0, 0}}},
	{BELL_2w,     kDTubell_2w,     {{kNHero_2w, kVRing_2w},      {kNMaid_2w,     kVGive_2w},     {kNCat_2w,     kVGive_2w},      {kNHarry_2w, kVGive_2w},   {kNHester_2w, kVGive_2w}, {kNCook_2w,   kVGive_2w}, {0, 0}}},
	{CATNIP_2w,   kDTucatnip_2w,   {{kNHero_2w, kVEat_2w},       {kNBell_2w,     kVRub_2w},      {kNMaid_2w,    kVGive_2w},      {kNCat_2w,   kVGive_2w},   {kNHarry_2w,  kVGive_2w}, {kNHester_2w, kVGive_2w}, {kNCook_2w, kVGive_2w}, {0, 0}}},
	{PAPER_2w,    kDTupaper_2w,    {{kNHero_2w, kVRead_2w},      {kNDoor_2w,     kVPush_2w},     {0, 0}}},
	{PENCIL_2w,   kDTupencil_2w,   {{kNHero_2w, kVMakeUseOf_2w}, {kNPaper_2w,    kVScribble_2w}, {kNDoor_2w,    kVMakeUseOf_2w}, {0, 0}}},
	{LASTOBJ_2w,  0, {{0, 0}}}
};

uses_t uses_3w[] = {
	{CLAY_3w,     kDTuclay_3w,     {{kNHero_3w, kVMake_3w},   {0, 0}}},
	{FLASK_3w,    kDTuflask_3w,    {{kNHero_3w, kVDrink_3w},  {kNPennylie_3w, kVGive_3w},     {kNWater_3w, kVFill_3w},     {kNPool_3w, kVFill_3w},   {0, 0}}},
	{BOUILLON_3w, kDTubouillon_3w, {{kNNat1_3w, kVGive_3w},   {kNNat2_3w,     kVGive_3w},     {kNNat3_3w,  kVGive_3w},     {kNNatb_3w, kVGive_3w},   {kNNatg_3w, kVGive_3w}, {kNPennylie_3w, kVGive_3w}, {kNDoctor_3w, kVGive_3w}, {0, 0}}},
	{CRYSTAL_3w,  kDTucrystal_3w,  {{kNHero_3w, kVRub_3w},    {0, 0}}},
	{CHEESE_3w,   kDTucheese_3w,   {{kNHero_3w, kVEat_3w},    {kNCage_3w,     kVPut_3w},      {0, 0}}},
	{SCROLL_3w,   kDTuread_3w,     {{kNHero_3w, kVRead_3w},   {0, 0}}},
	{BOOK_3w,     kDTuread_3w,     {{kNHero_3w, kVRead_3w},   {kNGhost_3w,    kVExorcise_3w}, {0, 0}}},
	{CANDLE_3w,   kDTucandle_3w,   {{kNHero_3w, kVStrike_3w}, {0, 0}}},
	{BELL_3w,     kDTubell_3w,     {{kNHero_3w, kVRing_3w},   {0, 0}}},
	{BLOWPIPE_3w, kDTupipe_3w,     {{kNHero_3w, kVShoot_3w},  {kNDoctor_3w,   kVShoot_3w},    {kNElephant_3w, kVShoot_3w}, {kNGhost_3w, kVShoot_3w}, {0, 0}}},
	{LASTOBJ_3w,  0, {{0, 0}}}
};

// TODO: This use lists are only used in Win versions. Should we add them in the DOS versions?
uses_t uses_1d[] = {
	{LASTOBJ_1d,  0, {{0, 0}}}
};

uses_t uses_2d[] = {
	{LASTOBJ_2d,  0, {{0, 0}}}
};

uses_t uses_3d[] = {
	{LASTOBJ_3d,  0, {{0, 0}}}
};

background_t catchall_1w[] = {					// Generally applicable phrases
	{kVJump_1w,      0,            kSTnojump_1w,      false, 0, 0},
	{kVGo_1w,        0,            kSTtrywalk_1w,     false, 0, 0},
	{kVEnter_1w,     0,            kSTtrywalk_1w,     false, 0, 0},
	{kVClimb_1w,     0,            kSTnoclimb_1w,     false, 0, 0},
	{kVShout_1w,     0,            kSTnothing_1w,     false, 0, 0},
	{kVTalk_1w,      0,            kSTnotalk_1w,      false, 0, 0},
	{kVSwitch_1w,    0,            kSTenopurps_1w,    false, 0, 0},
	{kVThrowit_1w,   0,            kSTenopurps_1w,    false, 0, 0},
	{kVAttack_1w,    0,            kSTnoattack_1w,    false, 0, 0},
	{kVBreak_1w,     0,            kSTnobreak_1w,     false, 0, 0},
	{kVListen_1w,    0,            kSTnonoise_1w,     false, 0, 0},
	{kVSmell_1w,     0,            kSTnosmell_1w,     false, 0, 0},
	{kVQuery_1w,     0,            kSTnoidea_1w,      false, 0, 0},
	{kVLook_1w,      kNSky_1w,     kSTlooksky_1w,     false, 0, 0},
	{kVLook_1w,      kNWall_1w,    kSTedull_1w,       false, 0, 0},
	{kVLook_1w,      kNGround_1w,  kSTedull_1w,       false, 0, 0},
	{kVHelp_1w,      0,            kSTaskhelp_1w,     false, 0, 0},
	{kVMagic_1w,     0,            kSTabracadabra_1w, false, 0, 0},
	{kVDig_1w,       0,            kSTnodig_1w,       false, 0, 0},
	{kVRude_1w,      0,            kSTnorude_1w,      false, 0, 0},
	{kVKnock_1w,     0,            kSTnoanswer_1w,    false, 0, 0},
	{kVTake_1w,      kNPicture_1w, kSTenopurps_1w,    false, 0, 0},
	{kVHero,         kNCut_1w,     kSTCutHero_1w,     false, 0, 0},
	{kVHero,         kNOil_1w,     kSTOilHero_1w,     false, 0, 0},
	{kVMakeUseOf_1w, kNGold_1w,    kSTGoldHero_1w,    false, 0, 0},
	{kVLook_1w,      kNMonkey_1w,  kSTLookMonkey_1w,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen0_desc_1w[] = {						// Outside house
	{kVLook_1w,  kNTree_1w,   kSTlooktree_1w,    false, 0, 0},
	{kVLook_1w,  kNFence_1w,  kSTlookfence_1w,   false, 0, 0},
	{kVLook_1w,  kNHouse_1w,  kSTlookhouse_1w,   false, 0, 0},
	{kVLook_1w,  kNWindow_1w, kSTlookwindow_1w,  false, 0, 0},
	{kVLook_1w,  kNRoof_1w,   kSTedull_1w,       false, 0, 0},
	{kVLook_1w,  kNLight_1w,  kSTedull_1w,       false, 0, 0},
	{kVLook_1w,  kNMoon_1w,   kSTlookmoon_1w,    false, 0, 0},
	{kVClimb_1w, kNTree_1w,   kSTclimbtree1_1w,  false, 0, 0},
	{kVClimb_1w, kNFence_1w,  kSTclimbfence_1w,  false, 0, 0},
	{kVEat_1w,   kNPkin_1w,   kSTeatpumpkin_1w,  false, 0, 0},
	{kVUnder_1w, kNCarpet_1w, kSTundermat_1w,    false, 0, 0},
	{kVLook_1w,  0,           kSTlookscreen0_1w, true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen1_desc_1w[] = {						// Hall
	{kVLook_1w,   kNBat_1w,     kSTlookbat_1w,      false, 0, 0},
	{kVLook_1w,   kNPicture_1w, kSTlookpicture_1w,  false, 0, 0},
	{kVLook_1w,   kNTable_1w,   kSTlooktable_1w,    false, 0, 0},
	{kVLook_1w,   kNCarpet_1w,  kSTedull_1w,        false, 0, 0},
	{kVLook_1w,   kNStairs_1w,  kSTlookupstairs_1w, false, 0, 0},
	{kVLook_1w,   kNLight_1w,   kSTlooklight_1w,    false, 0, 0},
	{kVLook_1w,   0,            kSTlookscreen1_1w,  true,  0, 0},
	{kVListen_1w, 0,            kSTlistenhall_1w,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen2_desc_1w[] = {						// Bedroom 1
	{kVLook_1w,  kNBed_1w,    kSTlookbed_1w,     false, 0, 0},
	{kVUnder_1w, kNBed_1w,    kSTlookbed_1w,     false, 0, 0},
	{kVRide_1w,  kNBed_1w,    kSTnosnooze_1w,    false, 0, 0},
	{kVInto_1w,  kNBed_1w,    kSTnosnooze_1w,    false, 0, 0},
	{kVLook_1w,  kNWard_1w,   kSTlookward_1w,    false, 0, 0},
	{kVLook_1w,  kNCupb_1w,   kSTlookcupbd_1w,   false, 0, 0},
	{kVLook_1w,  kNWindow_1w, kSTlookbed1win_1w, false, 0, 0},
	{kVLook_1w,  kNLight_1w,  kSTedull_1w,       false, 0, 0},
	{kVLook_1w,  kNFace_1w,   kSTlookface_1w,    false, 0, 0},
	{kVLook_1w,  0,           kSTlookscreen2_1w, true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen3_desc_1w[] = {						// Dining room
	{kVLook_1w,   kNTable_1w,   kSTlookspread_1w,      false, 0, 0},
	{kVLook_1w,   kNButler_1w,  kSTlookbutler_1w,      false, 0, 0},
	{kVLook_1w,   kNPlant_1w,   kSTlookplant_1w,       false, 0, 0},
	{kVLook_1w,   kNPicture_1w, kSTlookzelda_1w,       false, 0, 0},
	{kVLook_1w,   kNWitch_1w,   kSTlookzelda_1w,       false, 0, 0},
	{kVLook_1w,   kNWindow_1w,  kSTlookdiningwin_1w,   false, 0, 0},
	{kVLook_1w,   kNFood_1w,    kSTlookfood_1w,        false, 0, 0},
	{kVLook_1w,   kNMan_1w,     kSTlookdiningman_1w,   false, 0, 0},
	{kVLook_1w,   kNCupb_1w,    kSTedull_1w,           false, 0, 0},
	{kVLook_1w,   kNWoman_1w,   kSTlookwoman_1w,       false, 0, 0},
	{kVLook_1w,   0,            kSTlookscreen3_1w,     true,  0, 0},
	{kVSit_1w,    0,            kSTsitdown_1w,         false, 0, 0},
	{kVTake_1w,   kNFood_1w,    kSTtakefood_1w,        false, 0, 0},
	{kVEat_1w,    0,            kSTtakefood_1w,        false, 0, 0},
	{kVTalk_1w,   kNMan_1w,     kSTtalkdiningman_1w,   false, 0, 0},
	{kVTalk_1w,   kNWoman_1w,   kSTtalkdiningwoman_1w, false, 0, 0},
	{kVTalk_1w,   kNButler_1w,  kSTtalkbutler_1w,      false, 0, 0},
	{kVKiss_1w,   0,            kSTskiss_1w,           false, 0, 0},
	{kVListen_1w, 0,            kSTlistendining_1w,    false, 0, 0},
	{kVDrink_1w,  0,            kSTdrinkdining_1w,     false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen4_desc_1w[] = {						// Bathroom
	{kVLook_1w, kNWindow_1w, kSTlookbathwin_1w, false, 0, 0},
	{kVLook_1w, kNLight_1w,  kSTedull_1w,       false, 0, 0},
	{kVLook_1w, kNMirror_1w, kSTlookmirror_1w,  false, 0, 0},
	{kVLook_1w, kNToilet_1w, kSTlooktoilet_1w,  false, 0, 0},
	{kVLook_1w, kNBath_1w,   kSTlooktub_1w,     false, 0, 0},
	{kVLook_1w, kNSink_1w,   kSTedull_1w,       false, 0, 0},
	{kVLook_1w, 0,           kSTlookscreen4_1w, true,  0, 0},
	{kVCrap_1w, 0,           kSTdopoo_1w,       false, 0, 0},
	{kVSit_1w,  kNToilet_1w, kSTdowee_1w,       false, 0, 0},
	{kVRide_1w, kNToilet_1w, kSTdowee_1w,       false, 0, 0},
	{kVInto_1w, kNBath_1w,   kSTusetub_1w,      false, 0, 0},
	{kVTake_1w, kNBath_1w,   kSTusetub_1w,      false, 0, 0},
	{kVSit_1w,  0,           kSTsittoilet_1w,   false, 0, 0},
	{kVWash_1w, kNHands_1w,  kSTwashhands_1w,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen5_desc_1w[] = {						// Kitchen
	{kVLook_1w,   kNLight_1w,  kSTedull_1w,          false, 0, 0},
	{kVLook_1w,   kNUnits_1w,  kSTlookunits_1w,      false, 0, 0},
	{kVOpen_1w,   kNUnits_1w,  kSTeempty_1w,         false, 0, 0},
	{kVLook_1w,   kNWindow_1w, kSTlookkitchenwin_1w, false, 0, 0},
	{kVLook_1w,   kNBroom_1w,  kSTlookbroom_1w,      false, 0, 0},
	{kVTake_1w,   kNBroom_1w,  kSTtakebroom_1w,      false, 0, 0},
	{kVRide_1w,   kNBroom_1w,  kSTridebroom_1w,      false, 0, 0},
	{kVLook_1w,   0,           kSTlookscreen5_1w,    true,  0, 0},
	{kVSweep_1w,  0,           kSTsweepbroom_1w,     false, 0, 0},
	{kVListen_1w, 0,           kSTlistenkitchen_1w,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen6_desc_1w[] = {						// Garden
	{kVLook_1w,  kNShed_1w,       kSTlookShed_1w,    false, 0, 0},
	{kVLook_1w,  kNMoon_1w,       kSTedull_1w,       false, 0, 0},
	{kVLook_1w,  kNTree_1w,       kSTlooktree_1w,    false, 0, 0},
	{kVClimb_1w, kNTree_1w,       kSTclimbtree_1w,   false, 0, 0},
	{kVLook_1w,  kNGardenbits_1w, kSTlookgarden_1w,  false, 0, 0},
	{kVLook_1w,  0,               kSTlookscreen6_1w, true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen7_desc_1w[] = {						// Store room
	{kVLook_1w,   kNCarpet_1w,    kSTLookCarpet_1w,    false, 0, 0},
	{kVLook_1w,   kNLight_1w,     kSTedull_1w,         false, 0, 0},
	{kVUnbolt_1w, kNTrap_1w,      kSTsayunbolt_1w,     false, 0, 0},
	{kVLook_1w,   kNMousehole_1w, kSTlookmousehole_1w, false, 0, 0},
	{kVTake_1w,   kNMousehole_1w, kSTtakedroppings_1w, false, 0, 0},
	{kVTake_1w,   kNDroppings_1w, kSTtakedroppings_1w, false, 0, 0},
	{kVStroke_1w, kNDog_1w,       kSTStrokeDog_1w,     false, 0, 0},
	{kVGive_1w,   kNChop_1w,      kSTchop1_1w,         false, 0, 0},
	{kVFeed_1w,   kNDog_1w,       kSTchop1_1w,         false, 0, 0},
	{kVLook_1w,   0,              kSTlookscreen7_1w,   true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen8_desc_1w[] = {						// Basement
	{kVLook_1w,   kNLight_1w,    kSTedull_1w,          false, 0, 0},
	{kVLook_1w,   kNDoor_1w,     kSTlookbasedoor_1w,   false, 0, 0},
	{kVPush_1w,   kNDoor_1w,     kSTpushbasedoor_1w,   false, 0, 0},
	{kVOil_1w,    kNDoor_1w,     kSToilbasedoor_1w,    false, 0, 0},
	{kVLook_1w,   kNRock_1w,     kSTexaminerock_1w,    false, 0, 0},
	{kVPush_1w,   kNRock_1w,     kSTnowayhose_1w,      false, 0, 0},
	{kVLift_1w,   kNRock_1w,     kSTnowayhose_1w,      false, 0, 0},
	{kVMove_1w,   kNRock_1w,     kSTnowayhose_1w,      false, 0, 0},
	{kVUnder_1w,  kNRock_1w,     kSTnounder_1w,        false, 0, 0},
	{kVAttack_1w, kNDoor_1w,     kSTbreakbasedoor_1w,  false, 0, 0},
	{kVBreak_1w,  kNDoor_1w,     kSTbreakbasedoor_1w,  false, 0, 0},
	{kVOpen_1w,   kNDoor_1w,     kSTopenbasedoor_1w,   false, 0, 0},
	{kVUnlock_1w, kNDoor_1w,     kSTunlockbasedoor_1w, false, 0, 0},
	{kVKnock_1w,  kNDoor_1w,     kSTknockbasedoor_1w,  false, 0, 0},
	{kVTalk_1w,   kNPenelope_1w, kSTtalkpenelope_1w,   false, 0, 0},
	{kVShout_1w,  kNPenelope_1w, kSTtalkpenelope_1w,   false, 0, 0},
	{kVListen_1w, 0,             kSTlistenbase_1w,     false, 0, 0},
	{kVLook_1w,   0,             kSTlookscreen8_1w,    true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen9_desc_1w[] = {						// Bat cave
	{kVLook_1w,  0,         kSTlookscreen9_1w, true,  0, 0},
	{kVLook_1w,  kNRock_1w, kSTlookrock_1w,    false, 0, 0},
	{kVPush_1w,  kNRock_1w, kSTnowayhose_1w,   false, 0, 0},
	{kVLift_1w,  kNRock_1w, kSTnowayhose_1w,   false, 0, 0},
	{kVMove_1w,  kNRock_1w, kSTnowayhose_1w,   false, 0, 0},
	{kVUnder_1w, kNRock_1w, kSTnounder_1w,     false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen10_desc_1w[] = {					// Mummy room
	{kVLook_1w,  0,          kSTlookscreen10_1w, true,  0, 0},
	{kVLook_1w,  kNRock_1w,  kSTlookrock_1w,     false, 0, 0},
	{kVPush_1w,  kNRock_1w,  kSTnowayhose_1w,    false, 0, 0},
	{kVLift_1w,  kNRock_1w,  kSTnowayhose_1w,    false, 0, 0},
	{kVMove_1w,  kNRock_1w,  kSTnowayhose_1w,    false, 0, 0},
	{kVUnder_1w, kNRock_1w,  kSTnounder_1w,      false, 0, 0},
	{kVLook_1w,  kNTomb_1w,  kSTlooktomb_1w,     false, 0, 0},
	{kVLook_1w,  kNMummy_1w, kSTLookMummy_1w,    false, 0, 0},
	{kVTalk_1w,  kNMummy_1w, kSTTalkMummy_1w,    false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen11_desc_1w[] = {					// Lake room
	{kVLook_1w,      kNRock_1w, kSTlookrock_1w,     false, 0, 0},
	{kVPush_1w,      kNRock_1w, kSTnowayhose_1w,    false, 0, 0},
	{kVLift_1w,      kNRock_1w, kSTnowayhose_1w,    false, 0, 0},
	{kVMove_1w,      kNRock_1w, kSTnowayhose_1w,    false, 0, 0},
	{kVUnder_1w,     kNRock_1w, kSTnounder_1w,      false, 0, 0},
	{kVLook_1w,      0,         kSTlookscreen11_1w, true,  0, 0},
	{kVLakeverbs_1w, 0,         kSTsuggestboat_1w,  false, 0, 0},
	{kVDrink_1w,     0,         kSTnotthirsty_1w,   false, 0, 0},
	{kVPlug_1w,      0,         kSTqueryplug_1w,    false, 0, 0},
	{kVMakeUseOf_1w, kNBoat_1w, kSTsShutup_1w,      false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen12_desc_1w[] = {					// Dead end
	{kVLook_1w,  kNRock_1w,  kSTlookrock_1w,     false, 0, 0},
	{kVPush_1w,  kNRock_1w,  kSTnowayhose_1w,    false, 0, 0},
	{kVLift_1w,  kNRock_1w,  kSTnowayhose_1w,    false, 0, 0},
	{kVMove_1w,  kNRock_1w,  kSTnowayhose_1w,    false, 0, 0},
	{kVUnder_1w, kNRock_1w,  kSTnounder_1w,      false, 0, 0},
	{kVLook_1w,  0,          kSTlookscreen12_1w, true,  0, 0},
	{kVTalk_1w,  kNGuard_1w, kSTtalkguard_1w,    false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen13_desc_1w[] = {					// Jail
	{0, 0, 0, false, 0, 0}
};

background_t screen14_desc_1w[] = {					// The end
	{0, 0, 0, false, 0, 0}
};

background_t screen15_desc_1w[] =  {					// Laboratory
	{kVLook_1w,  0,                kSTlookscreen15_1w, true,  0, 0},
	{kVTalk_1w,  kNIgor_1w,        kSTtalkigor_1w,     false, 0, 0},
	{kVTalk_1w,  kNProf_1w,        kSTtalkprof_1w,     false, 0, 0},
	{kVLook_1w,  kNMachinebits_1w, kSTlookmachine_1w,  false, 0, 0},
	{kVPush_1w,  kNMachinebits_1w, kSTusemachine_1w,   false, 0, 0},
	{kVLook_1w,  kNTable_1w,       kSTlooklabtable_1w, false, 0, 0},
	{kVClose_1w, kNDoor_1w,        kSTuseboxdoor_1w,   false, 0, 0},
	{kVOpen_1w,  kNDoor_1w,        kSTuseboxdoor_1w,   false, 0, 0},
	{kVLook_1w,  kNLight_1w,       kSTlooklights_1w,   false, 0, 0},
	{kVLook_1w,  kNBooth_1w,       kSTlookbooth_1w,    false, 0, 0},
	{0, 0, 0, false, 0, 0}
};


// Array of ptrs to object_list_t
objectList_t backgroundList_1w[] = {
	screen0_desc_1w,  screen1_desc_1w,  screen2_desc_1w,  screen3_desc_1w,  screen4_desc_1w,
	screen5_desc_1w,  screen6_desc_1w,  screen7_desc_1w,  screen8_desc_1w,  screen9_desc_1w,
	screen10_desc_1w, screen11_desc_1w, screen12_desc_1w, screen13_desc_1w, screen14_desc_1w,
	screen15_desc_1w
};

background_t catchall_2w[] = {					// Generally applicable phrases
// Make sure verbs with nouns come before same verb with 0
	{kVTake_2w,      kNPicture_2w,    kSTNopurps_2w,      false, DONT_CARE, 0},
	{kVLook_2w,      kNPenny_2w,      kSTLookpen_2w,      false, DONT_CARE, 0},
	{kVKiss_2w,      kNPenny_2w,      kSTMmmm_2w,         false, DONT_CARE, 0},
	{kVRude_2w,      kNPenny_2w,      kSTHeadache_2w,     false, DONT_CARE, 0},
	{kVLook_2w,      kNStairs_2w,     kSTDull_2w,         false, DONT_CARE, 0},
	{kVUnder_2w,     kNStairs_2w,     kSTDull_2w,         false, DONT_CARE, 0},
	{kVLook_2w,      kNPlant_2w,      kSTSplant_2w,       false, DONT_CARE, 0},
	{kVLook_2w,      kNWindow_2w,     kSTS6garden_2w,     false, DONT_CARE, 0},
	{kVLook_2w,      kNPicture_2w,    kSTSpicture_2w,     false, DONT_CARE, 0},
	{kVLook_2w,      kNMirror_2w,     kSTDull_2w,         false, DONT_CARE, 0},
	{kVLook_2w,      kNTable_2w,      kSTNo_on_2w,        false, DONT_CARE, 0},
	{kVJump_2w,      0,               kSTSjump_2w,        false, DONT_CARE, 0},
	{kVGo_2w,        0,               kSTTrywalk_2w,      false, DONT_CARE, 0},
	{kVInto_2w,      0,               kSTTrywalk_2w,      false, DONT_CARE, 0},
	{kVClimb_2w,     0,               kSTSclimb_2w,       false, DONT_CARE, 0},
	{kVShout_2w,     0,               kSTNothing_2w,      false, DONT_CARE, 0},
	{kVTalk_2w,      0,               kSTStalk_2w,        false, DONT_CARE, 0},
	{kVSwitch_2w,    0,               kSTMorespecific_2w, false, DONT_CARE, 0},
	{kVThrowit_2w,   0,               kSTNopurps_2w,      false, DONT_CARE, 0},
	{kVAttack_2w,    0,               kSTSattack_2w,      false, DONT_CARE, 0},
	{kVBreak_2w,     0,               kSTSbreak_2w,       false, DONT_CARE, 0},
	{kVListen_2w,    0,               kSTQuiet_2w,        false, DONT_CARE, 0},
	{kVSmell_2w,     0,               kSTAroma_2w,        false, DONT_CARE, 0},
	{kVQuery_2w,     0,               kSTNoidea_2w,       false, DONT_CARE, 0},
	{kVLook_2w,      kNSky_2w,        kSTFalling_2w,      false, DONT_CARE, 0},
	{kVLook_2w,      kNWall_2w,       kSTDull_2w,         false, DONT_CARE, 0},
	{kVLook_2w,      kNGround_2w,     kSTDull_2w,         false, DONT_CARE, 0},
	{kVHelp_2w,      0,               kSTShelp_2w,        false, DONT_CARE, 0},
	{kVMagic_2w,     0,               kSTSmagic_2w,       false, DONT_CARE, 0},
	{kVWish_2w,      0,               kSTSmagic_2w,       false, DONT_CARE, 0},
	{kVDig_2w,       0,               kSTSdig_2w,         false, DONT_CARE, 0},
	{kVRude_2w,      0,               kSTSrude_2w,        false, DONT_CARE, 0},
	{kVKnock_2w,     0,               kSTNoanswer_2w,     false, DONT_CARE, 0},
	{kVOpen_2w,      kNDoor_2w,       kSTWontopen_2w,     false, DONT_CARE, 0},
	{kVUnlock_2w,    kNDoor_2w,       kSTCantunlock_2w,   false, DONT_CARE, 0},
	{kVLook_2w,      kNDoor_2w,       kSTDull_2w,         false, DONT_CARE, 0},
	{kVLook_2w,      kNLight_2w,      kSTDull_2w,         false, DONT_CARE, 0},
	{kVHello_2w,     0,               kSTHi_2w,           false, DONT_CARE, 0},
	{kVLook_2w,      kNFence_2w,      kSTLookover_2w,     false, DONT_CARE, 0},
	{kVLook_2w,      kNWall_2w,       kSTLookover_2w,     false, DONT_CARE, 0},
	{kVLook_2w,      kNGardenbits_2w, kSTDull_2w,         false, DONT_CARE, 0},
	{kVGive_2w,      0,               kSTNothanks_2w,     false, DONT_CARE, 0},
	{kVLook_2w,      kNTree_2w,       kSTDull2_2w,        false, DONT_CARE, 0},
	{kVFire_2w,      0,               kSTFire2_2w,        false, DONT_CARE, 0},
	{kVShout_2w,     0,               kSTNoanswer_2w,     false, DONT_CARE, 0},
	{kVHerring_2w,   0,               kSTSherring_2w,     false, DONT_CARE, 0},
	{kVUndress_2w,   0,               kSTSundress_2w,     false, DONT_CARE, 0},
	{kVSit_2w,       0,               kSTStired_2w,       false, DONT_CARE, 0},
	{kVFeed_2w,      0,               kSTNothanks_2w,     false, DONT_CARE, 0},
	{kVRub_2w,       kNCatnip_2w,     kSTRubcatnip1_2w,   false, DONT_CARE, 0},
	{kVMakeUseOf_2w, kNScrew_2w,      kSTLookScrew_2w,    false, DONT_CARE, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen0_desc_2w[] = {						// Outside house
	{kVLook_2w, 0, kSTWelcome_2w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen1_desc_2w[] = {						// Hall
	{kVLook_2w,   0,          kSTLookhall_2w,  true,  0, 0},
	{kVLook_2w,   kNMaid_2w,  kSTTmaid_2w,     false, 0, 0},
	{kVTalk_2w,   kNMaid_2w,  kSTChatmaid1_2w, false, 0, 0},
	{kVTalk_2w,   kNPenny_2w, kSTChatmaid2_2w, false, 0, 0},
	{kVKiss_2w,   kNMaid_2w,  kSTChatmaid3_2w, false, 0, 0},
	{kVRude_2w,   kNMaid_2w,  kSTRudemaid_2w,  false, 0, 0},
	{kVOpen_2w,   kNDoor_2w,  kSTTmaiddoor_2w, false, 0, 0},
	{kVUnlock_2w, kNDoor_2w,  kSTTmaiddoor_2w, false, 0, 0},
	{kVLook_2w,   kNDoor_2w,  kSTSdoor_2w,     false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen2_desc_2w[] = { 					// Bed1
	{kVLook_2w,  0,             kSTLookbed1_2w, true,  DONT_CARE, 0},
	{kVLook_2w,  kNBed_2w,      kSTLookbed_2w,  false, DONT_CARE, 0},
	{kVInto_2w,  kNBed_2w,      kSTS2bed_2w,    false, 0,         0},
	{kVRide_2w,  kNBed_2w,      kSTS2bed_2w,    false, 0,         0},
	{kVRest_2w,  kNBed_2w,      kSTS2bed_2w,    false, 0,         0},
	{kVRide_2w,  kNPenny_2w,    kSTS2bed_2w,    false, 0,         0},
	{kVTalk_2w,  kNPenny_2w,    kSTZzzz_2w,     false, 0,         0},
	{kVLook_2w,  kNPennylie_2w, kSTZzzz_2w,     false, 0,         0},
	{kVTalk_2w,  kNPennylie_2w, kSTS2bed_2w,    false, 0,         0},
	{kVInto_2w,  kNBed_2w,      kSTNopurps_2w,  false, 1,         0},
	{kVRide_2w,  kNBed_2w,      kSTNopurps_2w,  false, 1,         0},
	{kVRest_2w,  kNBed_2w,      kSTNopurps_2w,  false, 1,         0},
	{kVUnder_2w, kNBed_2w,      kSTSsearch_2w,  false, DONT_CARE, 0},
	{kVRead_2w,  kNBook_2w,     kSTNocarry_2w,  false, DONT_CARE, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen3_desc_2w[] = {						// Bed2
	{kVLook_2w,  0,            kSTLookbed2_2w,   true , 0, 0},
	{kVLook_2w,  kNBird_2w,    kSTParrot_2w,     false, 0, 0},
	{kVLook_2w,  kNPenny_2w,   kSTLookpen_2w,    false, 0, 0}, // since following cage has synonym "pen"
	{kVLook_2w,  kNCage_2w,    kSTDull_2w,       false, 0, 0},
	{kVLook_2w,  kNPencil_2w,  kSTDull_2w,       false, 0, 0},
	{kVLook_2w,  kNPhone_2w,   kSTDull_2w,       false, 0, 0},
	{kVLook_2w,  kNPaper_2w,   kSTBlotter_2w,    false, 0, 0},
	{kVRead_2w,  kNPaper_2w,   kSTBlotter_2w,    false, 0, 0},
	{kVTake_2w,  kNBlotpad_2w, kSTNouse_2w,      false, 0, 0},
	{kVLook_2w,  kNChair_2w,   kSTDull_2w,       false, 0, 0},
	{kVTake_2w,  kNPencil_2w,  kSTNouse_2w,      false, 0, 0},
	{kVTake_2w,  kNPaper_2w,   kSTNouse_2w,      false, 0, 0},
	{kVLook_2w,  kNKeyhole_2w, kSTKeyhole2_2w,   false, 0, 0},
	{kVOpen_2w,  kNKeyhole_2w, kSTKeyhole3_2w,   false, 0, 0},
	{kVLook_2w,  kNWall_2w,    kSTDumbwaiter_2w, false, 0, 0},
	{kVLook_2w,  kNButton_2w,  kSTSbutton_2w,    false, 0, 0},
	{kVPush_2w,  kNButton_2w,  kSTS3dumb_2w,     false, 0, 0},
	{kVPush_2w,  kNSwitch_2w,  kSTS3dumb_2w,     false, 0, 0},
	{kVDial_2w,  0,            kSTS3phone_2w,    false, 0, 4},
	{kVTake_2w,  kNPhone_2w,   kSTS3phone_2w,    false, 0, 4},
	{kVUse_2w,   kNPhone_2w,   kSTS3phone_2w,    false, 0, 4},
	{kVLift_2w,  kNPhone_2w,   kSTS3phone_2w,    false, 0, 4},
	{kVTalk_2w,  kNBird_2w,    kSTS3bird_2w,     false, 0, 5},
	{kVInto_2w,  0,            kSTSinto_2w,      true,  0, 0},
	{kVUnder_2w, kNCupb_2w,    kSTSsearch_2w,    false, 0, 0},
	{kVFeed_2w,  kNBird_2w,    kSTBirdfull_2w,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen4_desc_2w[] = {						// Keyhole
	{kVLook_2w, 0, kSTLooklook_2w, true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen5_desc_2w[] = {						// Bed3
	{kVLook_2w,  0,           kSTLookbed3_2w,     true,  0, 0},
	{kVLook_2w,  kNBlock_2w,  kSTLookblocks_2w,   false, 0, 0},
	{kVPlay_2w,  kNBlock_2w,  kSTSblock_2w,       false, 0, 0},
	{kVBlock_2w, 0,           kSTSblock_2w,       false, 0, 0},
	{kVLook_2w,  kNCage_2w,   kSTLookplaypen_2w,  false, 0, 0},
	{kVInto_2w,  kNCage_2w,   kSTStoobigtofit_2w, false, 0, 0},
	{kVLook_2w,  kNWindow_2w, kSTS6garden_2w,     false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen6_desc_2w[] = {						// Kitchen
	{kVLook_2w,   0,           kSTLookkitchen_2w,   true,  DONT_CARE, 0},
	{kVLook_2w,   kNButton_2w, kSTSbutton_2w,       false, DONT_CARE, 0},
	{kVPush_2w,   kNButton_2w, kSTS3dumb_2w,        false, DONT_CARE, 0},
	{kVLook_2w,   kNWindow_2w, kSTS6garden_2w,      false, DONT_CARE, 0},
	{kVLook_2w,   kNUnits_2w,  kSTS6dull_2w,        false, DONT_CARE, 0},
	{kVOpen_2w,   kNUnits_2w,  kSTS6dull_2w,        false, DONT_CARE, 0},
	{kVInto_2w,   0,           kSTSinto_2w,         true,  DONT_CARE, 0},
	{kVOpen_2w,   kNDoor_2w,   kSTMorespecific_2w,  false, DONT_CARE, 0},
	{kVLook_2w,   kNDoor_2w,   kSTMorespecific_2w,  false, DONT_CARE, 0},
	{kVTalk_2w,   kNCook_2w,   kSTTalkcook_2w,      false, 1,         0},
	{kVLook_2w,   kNCook_2w,   kSTLookcook_2w,      false, 1,         0},
	{kVLook_2w,   kNKnife_2w,  kSTLookknife_2w,     false, 1,         0},
	{kVTake_2w,   kNKnife_2w,  kSTTakeknife_2w,     false, 1,         0},
	{kVListen_2w, 0,           kSTListenkitchen_2w, false, 1,         0},
	{0, 0, 0, false, 0, 0}
};

background_t screen7_desc_2w[] = {						// Backdoor
	{kVLook_2w, 0,           kSTLookback_2w, true,  0, 0},
	{kVLook_2w, kNWindow_2w, kSTLookwin_2w,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen8_desc_2w[] = {						// Shed
	{kVLook_2w, 0,           kSTLookshed_2w,   true,  0, 0},
	{kVLook_2w, kNWindow_2w, kSTLookwin_2w,    false, 0, 0},
	{kVLook_2w, kNShed_2w,   kSTLookatshed_2w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen9_desc_2w[] = {						// In shed
	{kVLook_2w,  0,            kSTLookinshed_2w,  true,  0, 0},
	{kVLook_2w,  kNWall_2w,    kSTS9tools1_2w,    false, 0, 0},
	{kVLook_2w,  kNTools_2w,   kSTS9tools1_2w,    false, 0, 0},
	{kVTake_2w,  kNTools_2w,   kSTS9tools2_2w,    false, 0, 0},
	{kVLook_2w,  kNBroom_2w,   kSTDull_2w,        false, 0, 0},
	{kVTake_2w,  kNBroom_2w,   kSTNouse_2w,       false, 0, 0},
	{kVLook_2w,  kNTable_2w,   kSTSomebuttons_2w, false, 0, 0},
	{kVKiss_2w,  kNGardner_2w, kSTMore_2w,        false, 0, 0},
	{kVRude_2w,  kNGardner_2w, kSTRudeshed_2w,    false, 0, 0},
	{kVTalk_2w,  kNGardner_2w, kSTIgnore_2w,      false, 0, 0},
	{kVClose_2w, kNDoor_2w,    kSTShedclose_2w,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen10_desc_2w[] = {					// Venus fly traps
	{kVLook_2w, 0,           kSTLookvenus_2w, true,  0, 0},
	{kVTake_2w, kNMirror_2w, kSTNotmirror_2w, false, 0, 0},
	{kVLook_2w, kNFly_2w,    kSTLookFly_2w,   false, 0, 0},
	{kVLook_2w, kNLeaf_2w,   kSTLookTrap_2w,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen11_desc_2w[] = {					// Gates open
	{kVLook_2w,  0,         kSTS11look_2w, true,  0, 0},
	{kVClose_2w, kNDoor_2w, kSTNopurps_2w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen12_desc_2w[] = {					// Gates closed
	{kVLook_2w,   0,         kSTS12look_2w, true,  0, 0},
	{kVLook_2w,   kNDoor_2w, kSTGates1_2w,  false, 0, 0},
	{kVOpen_2w,   kNDoor_2w, kSTGates1_2w,  false, 0, 0},
	{kVBreak_2w,  kNDoor_2w, kSTGates2_2w,  false, 0, 0},
	{kVAttack_2w, kNDoor_2w, kSTGates2_2w,  false, 0, 0},
	{kVUnlock_2w, kNDoor_2w, kSTGates3_2w,  false, 0, 0},
	{kVUnlock_2w, kNGate_2w, kSTGates3_2w,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen13_desc_2w[] = {					// Stream
	{kVLook_2w,    0,            kSTS13look_2w,    true,  0, 0},
	{kVLook_2w,    kNBridge_2w,  kSTNospecial_2w,  false, 0, 0},
	{kVUnder_2w,   kNBridge_2w,  kSTSsearch_2w,    false, 0, 0},
	{kVLook_2w,    kNWater_2w,   kSTDull_2w,       false, 0, 0},
	{kVThrowit_2w, kNMatches_2w, kSTThrowmatch_2w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen14_desc_2w[] = {					// Zapper
	{kVLook_2w,   0,           kSTS14look_2w,  true,  0, 0},
	{kVSwitch_2w, kNZapper_2w, kSTNoswitch_2w, false, 0, 0},
	{kVWind_2w,   kNZapper_2w, kSTNoswitch_2w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen15_desc_2w[] = {					// Mushroom
	{kVLook_2w,   0,           kSTS15look_2w,  true,  0, 0},
	{kVUse_2w,    kNOldman_2w, kSTS15wand2_2w, false, 0, 0},
	{kVTalk_2w,   kNOldman_2w, kSTNoreply_2w,  false, 0, 0},
	{kVLook_2w,   kNWand_2w,   kSTS15wand1_2w, false, 0, 0},
	{kVTake_2w,   kNWand_2w,   kSTS15wand2_2w, false, 0, 0},
	{kVSearch_2w, kNOldman_2w, kSTSsearch_2w,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen16_desc_2w[] = {					// Well
	{kVLook_2w,  0,           kSTS16look_2w,     true,  0, 0},
	{kVClimb_2w, 0,           kSTTryrope_2w,     false, 0, 0},
	{kVGo_2w,    kNWell_2w,   kSTTryrope_2w,     false, 0, 0},
	{kVWind_2w,  kNRope_2w,   kSTWindwell_2w,    false, 0, 0},
	{kVTie_2w,   kNRope_2w,   kSTNopurps_2w,     false, 0, 0},
	{kVTake_2w,  kNBucket_2w, kSTNosee_2w,       false, 0, 0},
	{kVLook_2w,  kNBucket_2w, kSTNosee_2w,       false, 0, 0},
	{kVWind_2w,  kNBucket_2w, kSTWindwell_2w,    false, 0, 0},
	{kVTake_2w,  kNWater_2w,  kSTNosee_2w,       false, 0, 0},
	{kVWind_2w,  kNHandle_2w, kSTWindwell_2w,    false, 0, 0},
	{kVInto_2w,  kNBucket_2w, kSTNosee_2w,       false, 0, 0},
	{kVInto_2w,  kNWell_2w,   kSTIntowell_2w,    false, 0, 0},
	{kVWish_2w,  0,           kSTGetonwithit_2w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen17_desc_2w[] = {					// Snakepit
	{kVLook_2w,   0,          kSTS17look_2w, true,  0, 0},
	{kVAttack_2w, kNSnake_2w, kSTS17kill_2w, false, 0, 0},
	{kVBreak_2w,  kNSnake_2w, kSTS17kill_2w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen18_desc_2w[] = {					// Phonebox
	{kVLook_2w, 0,          kSTS18look_2w, true,  0, 0},
	{kVLook_2w, kNPhone_2w, kSTS18look_2w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen19_desc_2w[] = {					// Street
	{kVLook_2w, 0,         kSTS19look_2w, true,  0, 0},
	{kVLook_2w, kNWall_2w, kSTSgraf_2w,   false, 0, 0},
	{kVRead_2w, kNWall_2w, kSTSgraf_2w,   false, 0, 0},
	{kVLook_2w, kNGraf_2w, kSTSgraf_2w,   false, 0, 0},
	{kVRead_2w, kNGraf_2w, kSTSgraf_2w,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen20_desc_2w[] = {					// Kennel
	{kVLook_2w,    0,           kSTS20look_2w,      true,  0, 0},
	{kVLook_2w,    kNWindow_2w, kSTMorespecific_2w, false, 0, 0},
	{kVThrowit_2w, kNStick_2w,  kSTDonthaveone_2w,  false, 0, 0},
	{kVStroke_2w,  kNDog_2w,    kSTStrokedog_2w,    false, 0, 0},
	{kVTalk_2w,    kNDog_2w,    kSTStrokedog_2w,    false, 0, 0},
	{kVInto_2w,    0,           kSTStoobigtofit_2w, false, 0, 0},
	{kVTake_2w,    kNStick_2w,  kSTThrown_2w,       false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen21_desc_2w[] = {					// Rockroom
	{kVLook_2w,  0,         kSTS21look_2w,   true,  0, 0},
	{kVClimb_2w, kNRope_2w, kSTNotclose_2w,  false, 0, 0},
	{kVMove_2w,  kNRock_2w, kSTTooheavy_2w,  false, 0, 0},
	{kVUnder_2w, kNRock_2w, kSTNounder_2w,   false, 0, 0},
	{kVLift_2w,  kNRock_2w, kSTNowayhose_2w, false, 0, 0},
	{kVLook_2w,  kNRock_2w, kSTDull_2w,      false, 0, 0},
	{kVTake_2w,  kNRock_2w, kSTNowayhose_2w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen22_desc_2w[] = {					// Rockgone
	{kVLook_2w,  0,         kSTS22look_2w,  true,  0, 0},
	{kVClimb_2w, kNRope_2w, kSTNotclose_2w, false, 0, 0},
	{kVLift_2w,  kNRock_2w, kSTNopurps_2w,  false, 0, 0},
	{kVLook_2w,  kNRock_2w, kSTDull_2w,     false, 0, 0},
	{kVTake_2w,  kNRock_2w, kSTNopurps_2w,  false, 0, 0},
	{kVUnder_2w, kNRock_2w, kSTSsearch_2w,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen23_desc_2w[] = {					// Threeway
	{kVLook_2w,  0,         kSTS23look_2w, true,  0, 0},
	{kVLift_2w,  kNRock_2w, kSTNopurps_2w, false, 0, 0},
	{kVLook_2w,  kNRock_2w, kSTDull_2w,    false, 0, 0},
	{kVUnder_2w, kNRock_2w, kSTDull2_2w,   false, 0, 0},
	{kVTake_2w,  kNRock_2w, kSTNopurps_2w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen24_desc_2w[] = {					// Lampcave
	{kVLook_2w,  0,         kSTS24look_2w,   true,  0, 0},
	{kVLift_2w,  kNRock_2w, kSTSsearch_2w,   false, 0, 0},
	{kVLook_2w,  kNRock_2w, kSTNospecial_2w, false, 0, 0},
	{kVUnder_2w, kNRock_2w, kSTDull2_2w,     false, 0, 0},
	{kVTake_2w,  kNRock_2w, kSTNopurps_2w,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen25_desc_2w[] = {					// Chasm
	{kVLook_2w,  0,         kSTS25look_2w, true,  0, 0},
	{kVUnder_2w, kNRock_2w, kSTSsearch_2w, false, 0, 0},
	{kVLift_2w,  kNRock_2w, kSTSsearch_2w, false, 0, 0},
	{kVLook_2w,  kNRock_2w, kSTDull_2w,    false, 0, 0},
	{kVTake_2w,  kNRock_2w, kSTNopurps_2w, false, 0, 0},
	{kVJump_2w,  0,         kSTSnojump_2w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen26_desc_2w[] = {					// Passage
	{kVLook_2w,  0,         kSTS26look_2w, true,  0, 0},
	{kVUnder_2w, kNRock_2w, kSTSsearch_2w, false, 0, 0},
	{kVLift_2w,  kNRock_2w, kSTSsearch_2w, false, 0, 0},
	{kVLook_2w,  kNRock_2w, kSTDull_2w,    false, 0, 0},
	{kVTake_2w,  kNRock_2w, kSTNopurps_2w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen27_desc_2w[] = {					// genie
	{kVLook_2w,    0,           kSTS27look_2w,   true,  0, 0},
	{kVUnder_2w,   kNRock_2w,   kSTSsearch_2w,   false, 0, 0},
	{kVLift_2w,    kNRock_2w,   kSTSsearch_2w,   false, 0, 0},
	{kVLook_2w,    kNRock_2w,   kSTDull_2w,      false, 0, 0},
	{kVLook_2w,    kNTrap_2w,   kSTBudge_2w,     false, 0, 0},
	{kVOpen_2w,    kNTrap_2w,   kSTBudge_2w,     false, 0, 0},
	{kVUnscrew_2w, kNTrap_2w,   kSTBudge_2w,     false, 0, 0},
	{kVUnlock_2w,  kNTrap_2w,   kSTBudge_2w,     false, 0, 0},
	{kVPush_2w,    kNTrap_2w,   kSTBudge_2w,     false, 0, 0},
	{kVTalk_2w,    kNGenie_2w,  kSTTalkgenie_2w, false, 0, 0},
	{kVRude_2w,    kNGenie_2w,  kSTRudeshed_2w,  false, 0, 0},
	{kVKiss_2w,    kNGenie_2w,  kSTRudeshed_2w,  false, 0, 0},
	{kVGive_2w,    kNBanana_2w, kSTNobanana_2w,  false, 0, 0},
	{kVClimb_2w,   kNStairs_2w, kSTTrywalk_2w,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen28_desc_2w[] = {					// traproom
	{kVLook_2w,    0,              kSTS28look_2w,      true,  0, 0},
	{kVBreak_2w,   kNSafe_2w,      kSTNowayhose_2w,    false, 0, 0},
	{kVLook_2w,    kNHole_2w,      kSTS28hole_2w,      false, 0, 28},
	{kVTake_2w,    kNMouse_2w,     kSTS28mouse_2w,     false, 0, 0},
	{kVTake_2w,    kNMousehole_2w, kSTS28mouse_2w,     false, 0, 0},
	{kVTake_2w,    kNDroppings_2w, kSTSdroppings_2w,   false, 0, 0},
	{kVUnscrew_2w, kNSafe_2w,      kSTMorespecific_2w, false, 0, 0},
	{kVOpen_2w,    0,              kSTUnlocksafe_2w,   false, 0, 0},
	{kVUnlock_2w,  kNSafe_2w,      kSTUnlocksafe_2w,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen29_desc_2w[] = {					// Hall 2
	{kVLook_2w, 0, kSTLookhall_2w, true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen30_desc_2w[] = {					// Lounge
	{kVLook_2w, 0,           kSTS30look_2w,  true,  0, 0},
	{kVLook_2w, kNWindow_2w, kSTS6garden_2w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen31_desc_2w[] = {					// parlor
	{kVLook_2w,   0,         kSTS31look_2w,      true,  0, 0},
	{kVListen_2w, 0,         kSTBlah_2w,         false, 0, 0},
	{kVOpen_2w,   kNDoor_2w, kSTMorespecific_2w, false, 0, 0},
	{kVLook_2w,   kNDoor_2w, kSTMorespecific_2w, false, 0, 0},
	{kVLook_2w,   kNMaid_2w, kSTLookPMaid_2w,    false, 0, 0},
	{kVTalk_2w,   kNMaid_2w, kSTTalkPMaid_2w,    false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen32_desc_2w[] = {					// catroom
	{kVLook_2w,   0,           kSTS32look_2w,    true,  0, 0},
	{kVStroke_2w, kNCat_2w,    kSTSstrokecat_2w, false, 0, 0},
	{kVPlay_2w,   kNCat_2w,    kSTSplaycat_2w,   false, 0, 0},
	{kVTalk_2w,   kNCat_2w,    kSTStalkcat_2w,   false, 0, 0},
	{kVLook_2w,   kNPost_2w,   kSTSlookpost_2w,  false, 0, 0},
	{kVGive_2w,   0,           kSTSgivecat_2w,   false, 0, 0},
	{kVLook_2w,   kNWindow_2w, kSTS6garden_2w,   false, 0, 0},
	{kVRub_2w,    kNCatnip_2w, kSTRubcatnip2_2w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen33_desc_2w[] = {					// Boxroom
	{kVLook_2w,      0,           kSTS33look_2w,      true,  0, 0},
	{kVLook_2w,      kNDoor_2w,   kSTLookboxdoor_2w,  false, 0, 0},
	{kVOpen_2w,      kNDoor_2w,   kSTWontopen_2w,     false, 0, 0},
	{kVRead_2w,      kNPaper_2w,  kSTReadpaper_2w,    false, 0, 29},
	{kVLook_2w,      kNPaper_2w,  kSTReadpaper_2w,    false, 0, 29},
	{kVLook_2w,      kNCrate_2w,  kSTSlookbox_2w,     false, 0, 0},
	{kVInto_2w,      kNCrate_2w,  kSTSgetinbox_2w,    false, 0, 0},
	{kVLook_2w,      kNChute_2w,  kSTLookchute_2w,    false, 0, 0},
	{kVLook_2w,      kNHole_2w,   kSTLookchute_2w,    false, 0, 0},
	{kVClimb_2w,     kNChute_2w,  kSTUpchute_2w,      false, 0, 0},
	{kVLook_2w,      kNWall_2w,   kSTLookchute_2w,    false, 0, 0},
	{kVInto_2w,      kNDoor_2w,   kSTMorespecific_2w, false, 0, 0},// Kludge for "put pencil in door"
	{kVPush_2w,      kNKey_2w,    kSTMorespecific_2w, false, 0, 0},
	{kVMakeUseOf_2w, kNPencil_2w, kSTUsePencil_2w,    false, 0, 0},
	{kVScribble_2w,  kNPencil_2w, kSTDoCrossword_2w,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen34_desc_2w[] = {					// hall3
	{kVLook_2w, 0,           kSTLookhall_2w,   true,  0, 0},
	{kVLook_2w, kNMirror_2w, kSTLookMirror_2w, false, 0, 0},
	{kVTake_2w, kNMirror_2w, kSTNouse_2w,      false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen35_desc_2w[] = {					// Organ
	{kVLook_2w,   0,          kSTS35look_2w,   true,  DONT_CARE, 0},
	{kVLook_2w,   kNOrgan_2w, kSTLookOrgan_2w, false, DONT_CARE, 0},
	{kVPlay_2w,   kNOrgan_2w, kSTPlayorgan_2w, false, DONT_CARE, 0},
	{kVListen_2w, 0,          kSTHearorgan_2w, false, 0,         0},
	{kVListen_2w, 0,          kSTHearlaugh_2w, false, 1,         0},
	{0, 0, 0, false, 0, 0}
};

background_t screen36_desc_2w[] = {					// Hestroom
	{kVLook_2w, 0,             kSTLookhest_2w,    true,  0, 0},
	{kVTake_2w, kNBook_2w,     kSTS36book_2w,     false, 0, 0},
	{kVTake_2w, kNBookcase_2w, kSTS36book_2w,     false, 0, 0},
	{kVRead_2w, kNBook_2w,     kSTS36book_2w,     false, 0, 0},
	{kVLook_2w, kNBook_2w,     kSTS36lookbook_2w, false, 0, 0},
	{kVLook_2w, kNTable_2w,    kSTS36table_2w,    false, 0, 0},
	{kVTalk_2w, kNHester_2w,   kSTTalkhester_2w,  false, 0, 0},
	{kVLook_2w, kNWindow_2w,   kSTS6garden_2w,    false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen37_desc_2w[] = {					// Retupmoc
// Screen states: 0: default, 1: Got screwdriver
	{kVLook_2w,  0,           kSTS37look_2w,   true,  DONT_CARE, 0},
	{kVLift_2w,  kNRock_2w,   kSTNopurps_2w,   false, 0,         0},
	{kVLook_2w,  kNRock_2w,   kSTNospecial_2w, false, 0,         0},
	{kVUnder_2w, kNRock_2w,   kSTDull2_2w,     false, 0,         0},
	{kVTalk_2w,  kNDoctor_2w, kSTComeHere_2w,  false, 0,         0},
	{kVTalk_2w,  kNDoctor_2w, kSTPleasego_2w,  false, 1,         0},
	{0, 0, 0, false, 0, 0}
};

background_t screen38_desc_2w[] = {					// hall1
	{kVLook_2w, 0, kSTS38look_2w, true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

// Array of ptrs to object_list_t
objectList_t backgroundList_2w[] = {
	screen0_desc_2w,  screen1_desc_2w,  screen2_desc_2w,  screen3_desc_2w,  screen4_desc_2w,
	screen5_desc_2w,  screen6_desc_2w,  screen7_desc_2w,  screen8_desc_2w,  screen9_desc_2w,
	screen10_desc_2w, screen11_desc_2w, screen12_desc_2w, screen13_desc_2w, screen14_desc_2w,
	screen15_desc_2w, screen16_desc_2w, screen17_desc_2w, screen18_desc_2w, screen19_desc_2w,
	screen20_desc_2w, screen21_desc_2w, screen22_desc_2w, screen23_desc_2w, screen24_desc_2w,
	screen25_desc_2w, screen26_desc_2w, screen27_desc_2w, screen28_desc_2w, screen29_desc_2w,
	screen30_desc_2w, screen31_desc_2w, screen32_desc_2w, screen33_desc_2w, screen34_desc_2w,
	screen35_desc_2w, screen36_desc_2w, screen37_desc_2w, screen38_desc_2w
};

background_t catchall_3w[] = {					// Generally applicable phrases
// Make sure verbs with nouns come before same verb with 0
	{kVLook_3w,    kNPenny_3w,  kSTLookpen_3w,      false, DONT_CARE, 0},
	{kVKiss_3w,    kNPenny_3w,  kSTMmmm_3w,         false, DONT_CARE, 0},
	{kVRude_3w,    kNPenny_3w,  kSTHeadache_3w,     false, DONT_CARE, 0},
	{kVNaughty_3w, kNPenny_3w,  kSTHeadache_3w,     false, DONT_CARE, 0},
	{kVInto_3w,    kNWindow_3w, kSTThruwindow_3w,   false, DONT_CARE, 0},
	{kVOutof_3w,   kNWindow_3w, kSTThruwindow_3w,   false, DONT_CARE, 0},
	{kVJump_3w,    0,           kSTSjump_3w,        false, DONT_CARE, 0},
	{kVGo_3w,      0,           kSTTrywalk_3w,      false, DONT_CARE, 0},
	{kVInto_3w,    0,           kSTTrywalk_3w,      false, DONT_CARE, 0},
	{kVClimb_3w,   0,           kSTSclimb_3w,       false, DONT_CARE, 0},
	{kVShout_3w,   0,           kSTNothing_3w,      false, DONT_CARE, 0},
	{kVTalk_3w,    0,           kSTStalk_3w,        false, DONT_CARE, 0},
	{kVSwitch_3w,  0,           kSTMorespecific_3w, false, DONT_CARE, 0},
	{kVUse_3w,     0,           kSTMorespecific_3w, false, DONT_CARE, 0},
	{kVThrowit_3w, 0,           kSTNopurps_3w,      false, DONT_CARE, 0},
	{kVRude_3w,    0,           kSTSrude_3w,        false, DONT_CARE, 0},
	{kVAttack_3w,  0,           kSTSattack_3w,      false, DONT_CARE, 0},
	{kVBreak_3w,   0,           kSTSbreak_3w,       false, DONT_CARE, 0},
	{kVListen_3w,  0,           kSTQuiet_3w,        false, DONT_CARE, 0},
	{kVSmell_3w,   0,           kSTAroma_3w,        false, DONT_CARE, 0},
	{kVQuery_3w,   0,           kSTNoidea_3w,       false, DONT_CARE, 0},
	{kVLook_3w,    kNSky_3w,    kSTFalling_3w,      false, DONT_CARE, 0},
	{kVLook_3w,    kNWall_3w,   kSTDull_3w,         false, DONT_CARE, 0},
	{kVLook_3w,    kNGround_3w, kSTDull_3w,         false, DONT_CARE, 0},
	{kVHelp_3w,    0,           kSTShelp_3w,        false, DONT_CARE, 0},
	{kVMagic_3w,   0,           kSTSmagic_3w,       false, DONT_CARE, 0},
	{kVWish_3w,    0,           kSTSmagic_3w,       false, DONT_CARE, 0},
	{kVDig_3w,     0,           kSTSdig_3w,         false, DONT_CARE, 0},
	{kVNaughty_3w, 0,           kSTSnaughty_3w,     false, DONT_CARE, 0},
	{kVKnock_3w,   0,           kSTNoanswer_3w,     false, DONT_CARE, 0},
	{kVOpen_3w,    kNDoor_3w,   kSTWontopen_3w,     false, DONT_CARE, 0},
	{kVUnlock_3w,  kNDoor_3w,   kSTCantunlock_3w,   false, DONT_CARE, 0},
	{kVLook_3w,    kNDoor_3w,   kSTDull_3w,         false, DONT_CARE, 0},
	{kVHello_3w,   0,           kSTHi_3w,           false, DONT_CARE, 0},
	{kVGive_3w,    0,           kSTNothanks_3w,     false, DONT_CARE, 0},
	{kVShout_3w,   0,           kSTNoanswer_3w,     false, DONT_CARE, 0},
	{kVUndress_3w, 0,           kSTSundress_3w,     false, DONT_CARE, 0},
	{kVSit_3w,     0,           kSTStired_3w,       false, DONT_CARE, 0},
	{kVFeed_3w,    0,           kSTNothanks_3w,     false, DONT_CARE, 0},
	{kVTake_3w,    kNVine_3w,   kSTNopurps_3w,      false, DONT_CARE, 0},
	{kVClimb_3w,   kNJungle_3w, kSTNopurps_3w,      false, DONT_CARE, 0},
	{kVLook_3w,    kNJungle_3w, kSTLookjungle_3w,   false, DONT_CARE, 0},
	{kVPut_3w,     kNFire_3w,   kSTNopurps_3w,      false, DONT_CARE, 0},
	{kVSwim_3w,    0,           kSTCantswim_3w,     false, DONT_CARE, 0},
	{kVTake_3w,    kNMouse_3w,  kSTCantcatch_3w,    false, DONT_CARE, 0},
	{kVLook_3w,    kNMouse_3w,  kSTNospecial_3w,    false, DONT_CARE, 0},
	{0, 0, 0, false, 0, 0}
};

background_t crash_desc_3w[] = {						// At the crash site
	{kVLook_3w,   0,          kSTLookcrash_3w,     false, 0, 0},
	{kVRepair_3w, kNPlane_3w, kSTNopurps_3w,       false, 0, 0},
	{kVFly_3w,    kNPlane_3w, kSTNopurps_3w,       false, 0, 0},
	{kVInto_3w,   0,          kSTMorespecific_3w,  true , 0, 0},
	{kVOpen_3w,   kNDoor_3w,  kSTOpenplanedoor_3w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t plane_desc_3w[] = {						// Inside the plane
	{kVLook_3w,   0, kSTLookplane_3w, true , 0, 0},
	{kVSearch_3w, 0, kSTSaylook_3w,   false, 0, 0},
	{kVInto_3w,   0, kSTYouarein_3w,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t web_desc_3w[] = { 						// At the spider's web
	{kVLook_3w, 0,           kSTLookweb_3w,   true,  0, 0},
	{kVTake_3w, kNNative_3w, kSTTakegirl_3w,  false, 0, 0},
	{kVLook_3w, kNWeb_3w,    kSTLookatweb_3w, false, 0, 0},
	{kVTake_3w, kNPenny_3w,  kSTTakepenny_3w, false, 0, 0},
	{kVTalk_3w, kNPenny_3w,  kSTTalkpenny_3w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t wfall_desc_3w[] = {						// Waterfall and stream
	{kVLook_3w,   0,          kSTLookwfall_3w,  true,  0, 0},
	{kVLook_3w,   kNWater_3w, kSTLookwfall_3w,  false, 0, 0},
	{kVCross_3w,  kNWater_3w, kSTCantcross_3w,  false, 0, 0},
	{kVListen_3w, 0,          kSTListenfall_3w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t wfall_b_desc_3w[] = {						// Same as above but no water
	{kVLook_3w,  0,          kSTLookwfall_b_3w, true,  0, 0},
	{kVLook_3w,  kNWater_3w, kSTLookwfall_b_3w, false, 0, 0},
	{kVCross_3w, kNWater_3w, kSTToomuddy_3w,    false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t wbase_desc_3w[] = {						// Base of waterfall
	{kVLook_3w,  0,          kSTLookwbase_3w, true , 0, 0},
	{kVLook_3w,  kNWater_3w, kSTLookwbase_3w, false, 0, 0},
	{kVCross_3w, kNWater_3w, kSTToomuddy_3w,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t path_ul_desc_3w[] = {						// Path at left of spider's web
	{kVLook_3w, 0, kSTLookpath_ul_3w, true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t bridge_desc_3w[] = {						// At the bridge
	{kVLook_3w,  0,         kSTLookbridge1_3w,    true,  0, 0},
	{kVSwing_3w, 0,         kSTSwingbridge_3w,    false, 0, 0},
	{kVTake_3w,  kNVine_3w, kSTGetbridgevines_3w, false, 0, 0},
	{kVTie_3w,   kNThem_3w, kSTMorespecific_3w,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t bridge2_desc_3w[] = {						// At the bridge
	{kVLook_3w,  0, kSTLookbridge2_3w, true,  0, 0},
	{kVSwing_3w, 0, kSTSwingbridge_3w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t stream_desc_3w[] = {						// stream with vines crossing
	{kVLook_3w,  0, kSTLookstream1_3w,  true,  0, 0},
	{kVSwing_3w, 0, kSTMorespecific_3w, false, 0, 0},
	{kVCross_3w, 0, kSTStep1_3w,        false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t stream2_desc_3w[] = {						// Stream with elephant sitting in it
	{kVLook_3w,  0,             kSTLookstream2_3w, true,  0, 0},
	{kVSwing_3w, 0,             kSTCantswing_3w,   false, 0, 0},
	{kVLook_3w,  kNElephant_3w, kSTLookele2_3w,    false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t village_desc_3w[] = {						// Long shot of village
	{kVLook_3w, 0,         kSTLookvillage_3w, true,  0, 0},
	{kVTake_3w, kNFood_3w, kSTTakething_3w,   false, 0, 0},
	{kVLook_3w, kNFood_3w, kSTTakething_3w,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t hut_out_desc_3w[] = {						// Outside witch doctor's hut
	{kVLook_3w,   0,            kSTLookhut_out_3w, false, 0, 0},
	{kVLook_3w,   kNDocbits_3w, kSTLookdocbits_3w, false, 0, 0},
	{kVTake_3w,   kNDocbits_3w, kSTTakedocbits_3w, false, 0, 0},
	{kVLook_3w,   kNFire_3w,    kSTLookdocbits_3w, false, 0, 0},
	{kVTake_3w,   kNFire_3w,    kSTTakedocbits_3w, false, 0, 0},
	{kVLook_3w,   kNSpider_3w,  kSTLookspider_3w,  false, 0, 0},
	{kVTake_3w,   kNSpider_3w,  kSTTakespider_3w,  false, 0, 0},
	{kVLook_3w,   kNSnake_3w,   kSTLooksnake_3w,   false, 0, 0},
	{kVTake_3w,   kNSnake_3w,   kSTTakesnake_3w,   false, 0, 0},
	{kVSearch_3w, kNWindow_3w,  kSTLookinhut_3w,   false, 0, 0},
	{kVLook_3w,   kNWindow_3w,  kSTLookinhut_3w,   false, 0, 0},
	{kVSearch_3w, kNHut_3w,     kSTLookinhut_3w,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t hut_in_desc_3w[] = {						// Inside hut
// States: 0 imprisoned, 1 doctor incapacitated
	{kVLook_3w,    0,              kSTLookhut_in_3w,     true,  0,         0},
	{kVLook_3w,    0,              kSTLookhut_in2_3w,    true,  1,         0},
	{kVLook_3w,    kNDoctor_3w,    kSTLookhut_in2_3w,    false, 1,         0},
	{kVTake_3w,    0,              kSTTakeincage_3w,     false, 0,         0},
	{kVTake_3w,    kNDoctor_3w,    kSTTakedoctor0_3w,    false, 0,         0},
	{kVTake_3w,    kNDoctor_3w,    kSTTakedoctor1_3w,    false, 1,         0},
	{kVLook_3w,    kNDocbits_3w,   kSTLookdocbits_3w,    false, DONT_CARE, 0},
	{kVTake_3w,    kNDocbits_3w,   kSTTakedocbits_3w,    false, DONT_CARE, 0},
	{kVLook_3w,    kNSpider_3w,    kSTLookspider_3w,     false, DONT_CARE, 0},
	{kVTake_3w,    kNSpider_3w,    kSTTakespider_3w,     false, DONT_CARE, 0},
	{kVLook_3w,    kNSnake_3w,     kSTLooksnake_3w,      false, DONT_CARE, 0},
	{kVTake_3w,    kNSnake_3w,     kSTTakesnake_3w,      false, DONT_CARE, 0},
	{kVLook_3w,    kNWindow_3w,    kSTLookouthut_3w,     false, DONT_CARE, 0},
	{kVLook_3w,    kNShelfbits_3w, kSTLookshelfbits_3w,  false, 0,         0},
	{kVLook_3w,    kNShelfbits_3w, kSTLookshelfbits2_3w, false, 1,         0},
	{kVTake_3w,    kNShelfbits_3w, kSTTakeshelfbits_3w,  false, 1,         0},
	{kVDrink_3w,   kNShelfbits_3w, kSTTakeshelfbits_3w,  false, 1,         0},
	{kVLook_3w,    kNBottles_3w,   kSTLookshelfbits_3w,  false, 0,         0},
	{kVLook_3w,    kNBottles_3w,   kSTLookshelfbits2_3w, false, 1,         0},
	{kVTake_3w,    kNBottles_3w,   kSTTakeshelfbits_3w,  false, 1,         0},
	{kVDrink_3w,   kNBottles_3w,   kSTTakeshelfbits_3w,  false, 1,         0},
	{kVSearch_3w,  0,              kSTCantlookin_3w,     false, 0,         0},
	{kVInto_3w,    kNFire_3w,      kSTGetinpot_3w,       false, 1,         0},
	{kVSearch_3w,  kNFire_3w,      kSTLookinfire_3w,     false, 1,         0},
	{kVLook_3w,    kNFire_3w,      kSTLookfire_3w,       false, DONT_CARE, 0},
	{kVTalk_3w,    kNDoctor_3w,    kSTTalkdoc_3w,        false, 0,         0},
	{kVTalk_3w,    kNDoctor_3w,    kSTTalkdoc2_3w,       false, 1,         0},
	{kVSearch_3w,  kNMouse_3w,     kSTLookinhole_3w,     false, DONT_CARE, 0},
	{kVTalk_3w,    kNMouse_3w,     kSTTalkmouse_3w,      false, DONT_CARE, 0},
	{kVUnlock_3w,  kNCdoor_3w,     kSTPicklock_3w,       false, 0,         0},
	{kVTake_3w,    kNDoorlock_3w,  kSTPicklock_3w,       false, 0,         0},
	{kVGive_3w,    kNMouse_3w,     kSTGivemouse_3w,      false, DONT_CARE, 0},
	{kVThrowit_3w, kNCheese_3w,    kSTGivemouse_3w,      false, DONT_CARE, 0},
	{kVTake_3w,    kNNative_3w,    kSTTakenative_3w,     false, 0,         0},
	{kVShoot_3w,   kNDoctor_3w,    kSTMissed_3w,         false, DONT_CARE, 0},
	{kVBlow_3w,    kNDoctor_3w,    kSTMissed_3w,         false, DONT_CARE, 0},
	{kVUse_3w,     kNPipe_3w,      kSTMissed_3w,         false, DONT_CARE, 0},
	{kVClose_3w,   kNCdoor_3w,     kSTNotclose_3w,       false, DONT_CARE, 0},
	{0, 0, 0, false, 0, 0}
};

background_t garden_desc_3w[] = {						// The secret garden
	{kVLook_3w,  0,           kSTLookgarden_3w,  true,  0, 0},
	{kVLook_3w,  kNOrchid_3w, kSTLookorchid_3w,  false, 0, 0},
	{kVTake_3w,  kNOrchid_3w, kSTTakeorchid_3w,  false, 0, 0},
	{kVCross_3w, 0,           kSTCrossgarden_3w, false, 0, 0},
	{kVLook_3w,  kNWater_3w,  kSTLookgarden_3w,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t clifftop_desc_3w[] = {					// Top of cliff path
	{kVLook_3w, 0, kSTLookclifftop_3w, true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t oldman_desc_3w[] = {						// Old man inside cave
	{kVLook_3w, 0, kSTLookoldman_3w, true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t cliff_desc_3w[] = {						// Lower cliff path
	{kVLook_3w, 0, kSTLookcliff_3w, true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t camp_desc_3w[] = {						// Camp scene in village
	{kVLook_3w,   0,             kSTLookcamp_3w,    true,  0, 0},
	{kVLook_3w,   kNFire_3w,     kSTLookhyena_3w,   false, 0, 0},
	{kVLook_3w,   kNPole_3w,     kSTLookpole_3w,    false, 0, 0},
	{kVBehind_3w, kNHut_3w,      kSTBehindhut_3w,   false, 0, 0},// Must come before look hut
	{kVSearch_3w, kNWindow_3w,   kSTLookintohut_3w, false, 0, 0},
	{kVSearch_3w, kNHut_3w,      kSTLookintohut_3w, false, 0, 0},
	{kVLook_3w,   kNHut_3w,      kSTLookhut_3w,     false, 0, 0},
	{kVLook_3w,   kNWindow_3w,   kSTLookintohut_3w, false, 0, 0},
	{kVEat_3w,    0,             kSTEatroast_3w,    false, 0, 0},
	{kVTake_3w,   kNFood_3w,     kSTEatroast_3w,    false, 0, 0},
	{kVInto_3w,   kNFire_3w,     kSTIntofire_3w,    false, 0, 0},
	{kVTake_3w,   kNFire_3w,     kSTIntofire_3w,    false, 0, 0},
	{kVTake_3w,   kNNative_3w,   kSTTakenative_3w,  false, 0, 0},
	{kVTake_3w,   kNPipe_3w,     kSTMakeoffer_3w,   false, 0, 0},
	{kVGive_3w,   kNBouillon_3w, kSTNonecarried_3w, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t turn_desc_3w[] = {						// Turnaround path
	{kVLook_3w,  0,         kSTLookturn_3w,  true,  DONT_CARE, 0},
	{kVUnder_3w, kNRock_3w, kSTUnderrock_3w, false, 0,         0},
	{kVLook_3w,  kNRock_3w, kSTLookrock_3w,  false, 0,         0},
	{kVRide_3w,  kNRock_3w, kSTOntorock_3w,  false, 0,         0},
	{kVClimb_3w, kNRock_3w, kSTOntorock_3w,  false, 0,         0},
	{0, 0, 0, false, 0, 0}
};

background_t slope_desc_3w[] = {						// Slope between cliff and stream
	{kVLook_3w, 0, kSTLookslope_3w, true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t path_desc_3w[] = {						// Path containing elephant
// States: 0 Elephant present, 1 - Elephant not present
	{kVLook_3w, 0,             kSTLookpath2_1_3w,  true,  0, 0},
	{kVLook_3w, 0,             kSTLookpath2_2_3w,  true,  1, 0},
	{kVRide_3w, kNElephant_3w, kSTRideelephant_3w, false, 0, 0},
	{kVTake_3w, kNElephant_3w, kSTGetelephant_3w,  false, 0, 0},
	{kVShow_3w, kNMouse_3w,    kSTShowmouse_3w,    false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t cave_desc_3w[] = {						// Cave mouth
	{kVLook_3w,   0,          kSTLookcave1_3w,   true,  0, 0},
	{kVLook_3w,   0,          kSTLookcave2_3w,   true,  1, 0},
	{kVAttack_3w, kNGhost_3w, kSTAttackghost_3w, false, 0, 0},
	{kVBreak_3w,  kNGhost_3w, kSTAttackghost_3w, false, 0, 0},
	{kVShoot_3w,  kNGhost_3w, kSTAttackghost_3w, false, 0, 0},
	{kVTake_3w,   kNGhost_3w, kSTTakeghost_3w,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t bgDummy[] = {
	{0, 0, 0, false, 0, 0}
};

// Array of ptrs to background_t
// Strangerke : replaced 0 by bgDummy. TODO: Suppress this comment if OK
objectList_t backgroundList_3w[] =  {
	crash_desc_3w,   web_desc_3w,     bridge_desc_3w,  bridge2_desc_3w, clifftop_desc_3w,
	wfall_desc_3w,   wfall_b_desc_3w, wbase_desc_3w,   stream_desc_3w,  stream2_desc_3w,
	path_ul_desc_3w, village_desc_3w, hut_out_desc_3w, hut_in_desc_3w,  garden_desc_3w,
	oldman_desc_3w,  cliff_desc_3w,   slope_desc_3w,   camp_desc_3w,    bgDummy,
	turn_desc_3w,    plane_desc_3w,   bgDummy,         path_desc_3w,    cave_desc_3w
};

background_t catchall_1d[] = { // Generally applicable phrases
	{kVJump_1d,   0,            kSTnojump_1d,   false, 0, 0},
	{kVGo_1d,     0,            kSTtrywalk_1d,  false, 0, 0},
	{kVEnter_1d,  0,            kSTtrywalk_1d,  false, 0, 0},
	{kVClimb_1d,  0,            kSTnoclimb_1d,  false, 0, 0},
	{kVShout_1d,  0,            kSTnothing_1d,  false, 0, 0},
	{kVTalk_1d,   0,            kSTnotalk_1d,   false, 0, 0},
	{kVSwitch_1d, 0,            kSTnopurps_1d,  false, 0, 0},
	{kVThrow_1d,  0,            kSTnopurps_1d,  false, 0, 0},
	{kVAttack_1d, 0,            kSTnoattack_1d, false, 0, 0},
	{kVBreak_1d,  0,            kSTnobreak_1d,  false, 0, 0},
	{kVListen_1d, 0,            kSTnolisten_1d, false, 0, 0},
	{kVSmell_1d,  0,            kSTnosmell_1d,  false, 0, 0},
	{kVQuery_1d,  0,            kSTnoidea_1d,   false, 0, 0},
	{kVLook_1d,   kNSky_1d,     kSTnolook_1d,   false, 0, 0},
	{kVLook_1d,   kNWall_1d,    kSTdull_1d,     false, 0, 0},
	{kVLook_1d,   kNGround_1d,  kSTdull_1d,     false, 0, 0},
	{kVHelp_1d,   0,            kSTnohelp_1d,   false, 0, 0},
	{kVMagic_1d,  0,            kSTnomagic_1d,  false, 0, 0},
	{kVDig_1d,    0,            kSTnodig_1d,    false, 0, 0},
	{kVRude_1d,   0,            kSTnorude_1d,   false, 0, 0},
	{kVKnock_1d,  0,            kSTnoknock_1d,  false, 0, 0},
	{kVTake_1d,   kNPicture_1d, kSTnopurps_1d,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen0_desc_1d[] = {      // Outside house
	{kVLook_1d,  kNTree_1d,   kSTlooks0tree_1d,    false, 0, 0},
	{kVLook_1d,  kNFence_1d,  kSTlooks0fence_1d,   false, 0, 0},
	{kVLook_1d,  kNHouse_1d,  kSTlooks0house_1d,   false, 0, 0},
	{kVLook_1d,  kNWindow_1d, kSTlooks0window_1d,  false, 0, 0},
	{kVLook_1d,  kNRoof_1d,   kSTdull_1d,          false, 0, 0},
	{kVLook_1d,  kNLight_1d,  kSTdull_1d,          false, 0, 0},
	{kVLook_1d,  kNMoon_1d,   kSTlooks0moon_1d,    false, 0, 0},
	{kVEat_1d,   kNPkin_1d,   kSTeats0pkin_1d,     false, 0, 0},
	{kVUnder_1d, kNCarpet_1d, kSTunders0carpet_1d, false, 0, 0},
	{kVLook_1d,  0,           kSTlooks0_1d,        true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen1_desc_1d[] = {      // Hall
	{kVLook_1d,   kNBat_1d,     kSTlooks1bat_1d,     false, 0, 0},
	{kVLook_1d,   kNPicture_1d, kSTlooks1picture_1d, false, 0, 0},
	{kVLook_1d,   kNTable_1d,   kSTlooks1table_1d,   false, 0, 0},
	{kVLook_1d,   kNCarpet_1d,  kSTdull_1d,          false, 0, 0},
	{kVLook_1d,   kNStairs_1d,  kSTlooks1stairs_1d,  false, 0, 0},
	{kVLook_1d,   kNLight_1d,   kSTlooks1light_1d,   false, 0, 0},
	{kVLook_1d,   0,            kSTlooks1_1d,        true,  0, 0},
	{kVListen_1d, 0,            kSTlistens1_1d,      false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen2_desc_1d[] = {      // Bedroom 1
	{kVLook_1d,  kNBed_1d,    kSTlooks2bed_1d,    false, 0, 0},
	{kVUnder_1d, kNBed_1d,    kSTlooks2bed_1d,    false, 0, 0},
	{kVRide_1d,  kNBed_1d,    kSTrides2bed_1d,    false, 0, 0},
	{kVInto_1d,  kNBed_1d,    kSTrides2bed_1d,    false, 0, 0},
	{kVLook_1d,  kNWard_1d,   kSTlooks2ward_1d,   false, 0, 0},
	{kVLook_1d,  kNCupb_1d,   kSTlooks2cupb_1d,   false, 0, 0},
	{kVLook_1d,  kNWindow_1d, kSTlooks2window_1d, false, 0, 0},
	{kVLook_1d,  kNLight_1d,  kSTdull_1d,         false, 0, 0},
	{kVLook_1d,  kNFace_1d,   kSTlooks2face_1d,   false, 0, 0},
	{kVLook_1d,  0,           kSTlooks2_1d,       true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen3_desc_1d[] = {      // Dining room
	{kVLook_1d,   kNTable_1d,   kSTlooks3table_1d,  false, 0, 0},
	{kVLook_1d,   kNButler_1d,  kSTlooks3butler_1d, false, 0, 0},
	{kVLook_1d,   kNPlant_1d,   kSTlooks3plant_1d,  false, 0, 0},
	{kVLook_1d,   kNPicture_1d, kSTlooks3witch_1d,  false, 0, 0},
	{kVLook_1d,   kNWitch_1d,   kSTlooks3witch_1d,  false, 0, 0},
	{kVLook_1d,   kNWindow_1d,  kSTlooks3window_1d, false, 0, 0},
	{kVLook_1d,   kNFood_1d,    kSTlooks3food_1d,   false, 0, 0},
	{kVLook_1d,   kNMan_1d,     kSTlooks3man_1d,    false, 0, 0},
	{kVLook_1d,   kNCupb_1d,    kSTdull_1d,         false, 0, 0},
	{kVLook_1d,   kNWoman_1d,   kSTlooks3woman_1d,  false, 0, 0},
	{kVLook_1d,   0,            kSTlooks3_1d,       true,  0, 0},
	{kVSit_1d,    0,            kSTsits3_1d,        false, 0, 0},
	{kVTake_1d,   kNFood_1d,    kSTtakes3food_1d,   false, 0, 0},
	{kVEat_1d,    0,            kSTtakes3food_1d,   false, 0, 0},
	{kVTalk_1d,   kNMan_1d,     kSTtalks3man_1d,    false, 0, 0},
	{kVTalk_1d,   kNWoman_1d,   kSTtalks3woman_1d,  false, 0, 0},
	{kVTalk_1d,   kNButler_1d,  kSTtalkS3butler_1d, false, 0, 0},
	{kVKiss_1d,   0,            kSTkisss3_1d,       false, 0, 0},
	{kVListen_1d, 0,            kSTlistens3_1d,     false, 0, 0},
	{kVDrink_1d,  0,            kSTdrinks3_1d,      false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen4_desc_1d[] = {      // Bathroom
	{kVLook_1d, kNWindow_1d, kSTlooks2window_1d, false, 0, 0},
	{kVLook_1d, kNLight_1d,  kSTdull_1d,         false, 0, 0},
	{kVLook_1d, kNMirror_1d, kSTlooks4mirror_1d, false, 0, 0},
	{kVLook_1d, kNToilet_1d, kSTlooks4toilet_1d, false, 0, 0},
	{kVLook_1d, kNBath_1d,   kSTdull_1d,         false, 0, 0},
	{kVLook_1d, kNSink_1d,   kSTdull_1d,         false, 0, 0},
	{kVLook_1d, 0,           kSTlooks4_1d,       true,  0, 0},
	{kVCrap_1d, 0,           kSTcraps4_1d,       false, 0, 0},
	{kVSit_1d,  0,           kSTsits4_1d,        false, 0, 0},
	{kVWash_1d, kNHands_1d,  kSTwashs4hands_1d,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen5_desc_1d[] = {      // Kitchen
	{kVLook_1d,   kNLight_1d,  kSTdull_1d,         false, 0, 0},
	{kVLook_1d,   kNUnits_1d,  kSTlooks5units_1d,  false, 0, 0},
	{kVOpen_1d,   kNUnits_1d,  kSTempty_1d,        false, 0, 0},
	{kVLook_1d,   kNWindow_1d, kSTlooks5window_1d, false, 0, 0},
	{kVLook_1d,   kNBroom_1d,  kSTlooks5broom_1d,  false, 0, 0},
	{kVTake_1d,   kNBroom_1d,  kSTtakes5broom_1d,  false, 0, 0},
	{kVRide_1d,   kNBroom_1d,  kSTrides5broom_1d,  false, 0, 0},
	{kVLook_1d,   0,           kSTlooks5_1d,       true,  0, 0},
	{kVSweep_1d,  0,           kSTsweeps5_1d,      false, 0, 0},
	{kVListen_1d, 0,           kSTlistens1_1d,     false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen6_desc_1d[] = {      // Garden
	{kVLook_1d,  kNShed_1d,       kSTlooks6sched_1d,      false, 0, 0},
	{kVLook_1d,  kNMoon_1d,       kSTdull_1d,             false, 0, 0},
	{kVLook_1d,  kNTree_1d,       kSTlooks0tree_1d,       false, 0, 0},
	{kVClimb_1d, kNTree_1d,       kSTclimbs6tree_1d,      false, 0, 0},
	{kVLook_1d,  kNGardenbits_1d, kSTlooks6gardenbits_1d, false, 0, 0},
	{kVLook_1d,  0,               kSTlooks6_1d,           true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen7_desc_1d[] = {      // Store room
	{kVLook_1d,   kNCarpet_1d,    kSTdull_1d,            false, 0, 0},
	{kVLook_1d,   kNLight_1d,     kSTdull_1d,            false, 0, 0},
	{kVUnbolt_1d, kNTrap_1d,      kSTunbolts7trap_1d,    false, 0, 0},
	{kVLook_1d,   kNMousehole_1d, kSTlooks7mousehole_1d, false, 0, 0},
	{kVTake_1d,   kNDroppings_1d, kSTtakes7droppings_1d, false, 0, 0},
	{kVGive_1d,   kNChop_1d,      kSTchop1_1d,           false, 0, 0},
	{kVFeed_1d,   kNDog_1d,       kSTchop1_1d,           false, 0, 0},
	{kVLook_1d,   0,              kSTlooks7_1d,          true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen8_desc_1d[] = {      // Basement
	{kVLook_1d,   kNLight_1d,    kSTdull_1d,           false, 0, 0},
	{kVLook_1d,   kNDoor_1d,     kSTlooks8door_1d,     false, 0, 0},
	{kVOil_1d,    kNDoor_1d,     kSToils8door_1d,      false, 0, 0},
	{kVPush_1d,   kNDoor_1d,     kSTpushs8door_1d,     false, 0, 0},
	{kVLook_1d,   kNRock_1d,     kSTlooks8rock_1d,     false, 0, 0},
	{kVPush_1d,   kNRock_1d,     kSTnowayhose_1d,      false, 0, 0},
	{kVLift_1d,   kNRock_1d,     kSTnowayhose_1d,      false, 0, 0},
	{kVMove_1d,   kNRock_1d,     kSTnowayhose_1d,      false, 0, 0},
	{kVUnder_1d,  kNRock_1d,     kSTnounder_1d,        false, 0, 0},
	{kVAttack_1d, kNDoor_1d,     kSTbreaks8door_1d,    false, 0, 0},
	{kVBreak_1d,  kNDoor_1d,     kSTbreaks8door_1d,    false, 0, 0},
	{kVOpen_1d,   kNDoor_1d,     kSTopens8door_1d,     false, 0, 0},
	{kVUnlock_1d, kNDoor_1d,     kSTunlocks8door_1d,   false, 0, 0},
	{kVKnock_1d,  kNDoor_1d,     kSTknocks8door_1d,    false, 0, 0},
	{kVTalk_1d,   kNPenelope_1d, kSTtalks8penelope_1d, false, 0, 0},
	{kVShout_1d,  kNPenelope_1d, kSTtalks8penelope_1d, false, 0, 0},
	{kVListen_1d, 0,             kSTlistens8_1d,       false, 0, 0},
	{kVLook_1d,   0,             kSTlooks8_1d,         true,  0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen9_desc_1d[] = {      // Bat cave
	{kVLook_1d,  0,         kSTlooks9_1d,     true,  0, 0},
	{kVLook_1d,  kNRock_1d, kSTlooks9rock_1d, false, 0, 0},
	{kVPush_1d,  kNRock_1d, kSTnowayhose_1d,  false, 0, 0},
	{kVLift_1d,  kNRock_1d, kSTnowayhose_1d,  false, 0, 0},
	{kVMove_1d,  kNRock_1d, kSTnowayhose_1d,  false, 0, 0},
	{kVUnder_1d, kNRock_1d, kSTnounder_1d,    false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen10_desc_1d[] = {     // Mummy room
	{kVLook_1d,  0,         kSTlooks10_1d,     true,  0, 0},
	{kVLook_1d,  kNRock_1d, kSTlooks9rock_1d,  false, 0, 0},
	{kVPush_1d,  kNRock_1d, kSTnowayhose_1d,   false, 0, 0},
	{kVLift_1d,  kNRock_1d, kSTnowayhose_1d,   false, 0, 0},
	{kVMove_1d,  kNRock_1d, kSTnowayhose_1d,   false, 0, 0},
	{kVUnder_1d, kNRock_1d, kSTnounder_1d,     false, 0, 0},
	{kVLook_1d,  kNTomb_1d, kSTlooks10tomb_1d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen11_desc_1d[] = {     // Lake room
	{kVLook_1d,      kNRock_1d, kSTlooks9rock_1d,   false, 0, 0},
	{kVPush_1d,      kNRock_1d, kSTnowayhose_1d,    false, 0, 0},
	{kVLift_1d,      kNRock_1d, kSTnowayhose_1d,    false, 0, 0},
	{kVMove_1d,      kNRock_1d, kSTnowayhose_1d,    false, 0, 0},
	{kVUnder_1d,     kNRock_1d, kSTnounder_1d,      false, 0, 0},
	{kVLook_1d,      0,         kSTlooks11_1d,      true,  0, 0},
	{kVLakeverbs_1d, 0,         kSTlakeverbss11_1d, false, 0, 0},
	{kVDrink_1d,     0,         kSTdrinks3_1d,      false, 0, 0},
	{kVPlug_1d,      0,         kSTplugs11_1d,      false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen12_desc_1d[] = {     // Dead end
	{kVLook_1d,  kNRock_1d,  kSTlooks9rock_1d,   false, 0, 0},
	{kVPush_1d,  kNRock_1d,  kSTnowayhose_1d,    false, 0, 0},
	{kVLift_1d,  kNRock_1d,  kSTnowayhose_1d,    false, 0, 0},
	{kVMove_1d,  kNRock_1d,  kSTnowayhose_1d,    false, 0, 0},
	{kVUnder_1d, kNRock_1d,  kSTnounder_1d,      false, 0, 0},
	{kVLook_1d,  0,          kSTlooks12_1d,      true,  0, 0},
	{kVTalk_1d,  kNGuard_1d, kSTtalks12guard_1d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen13_desc_1d[] = {     // Jail
	{0, 0, 0, false, 0, 0}
};

background_t screen14_desc_1d[] = {     // The end
	{0, 0, 0, false, 0, 0}
};

background_t screen15_desc_1d[] = {     // Laboratory
	{kVLook_1d,  0,                kSTlooks15_1d,            true,  0, 0},
	{kVTalk_1d,  kNIgor_1d,        kSTtalks15igor_1d,        false, 0, 0},
	{kVTalk_1d,  kNProf_1d,        kSTtalks15prof_1d,        false, 0, 0},
	{kVLook_1d,  kNMachinebits_1d, kSTlooks15machinebits_1d, false, 0, 0},
	{kVPush_1d,  kNMachinebits_1d, kSTpushs15machinebits_1d, false, 0, 0},
	{kVLook_1d,  kNTable_1d,       kSTlooks15table_1d,       false, 0, 0},
	{kVClose_1d, kNDoor_1d,        kSTopens15door_1d,        false, 0, 0},
	{kVOpen_1d,  kNDoor_1d,        kSTopens15door_1d,        false, 0, 0},
	{kVLook_1d,  kNLight_1d,       kSTlooks15light_1d,       false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

// Array of ptrs to object_list_t
objectList_t backgroundList_1d[] =  {
	screen0_desc_1d,  screen1_desc_1d,  screen2_desc_1d,  screen3_desc_1d,  screen4_desc_1d,
	screen5_desc_1d,  screen6_desc_1d,  screen7_desc_1d,  screen8_desc_1d,  screen9_desc_1d,
	screen10_desc_1d, screen11_desc_1d, screen12_desc_1d, screen13_desc_1d, screen14_desc_1d,
	screen15_desc_1d
};

background_t catchall_2d[] = { // Generally applicable phrases
	{kVTake_2d,    kNPicture_2d,    kSTNopurps_2d,      false, DONT_CARE, 0},
	{kVLook_2d,    kNPenny_2d,      kSTLookpen_2d,      false, DONT_CARE, 0},
	{kVKiss_2d,    kNPenny_2d,      kSTMmmm_2d,         false, DONT_CARE, 0},
	{kVRude_2d,    kNPenny_2d,      kSTHeadache_2d,     false, DONT_CARE, 0},
	{kVLook_2d,    kNStairs_2d,     kSTDull_2d,         false, DONT_CARE, 0},
	{kVUnder_2d,   kNStairs_2d,     kSTDull_2d,         false, DONT_CARE, 0},
	{kVLook_2d,    kNPlant_2d,      kSTSplant_2d,       false, DONT_CARE, 0},
	{kVLook_2d,    kNPicture_2d,    kSTSpicture_2d,     false, DONT_CARE, 0},
	{kVLook_2d,    kNMirror_2d,     kSTDull_2d,         false, DONT_CARE, 0},
	{kVLook_2d,    kNTable_2d,      kSTNo_on_2d,        false, DONT_CARE, 0},
	{kVJump_2d,    0,               kSTSjump_2d,        false, DONT_CARE, 0},
	{kVGo_2d,      0,               kSTTrywalk_2d,      false, DONT_CARE, 0},
	{kVInto_2d,    0,               kSTTrywalk_2d,      false, DONT_CARE, 0},
	{kVClimb_2d,   0,               kSTSclimb_2d,       false, DONT_CARE, 0},
	{kVShout_2d,   0,               kSTNothing_2d,      false, DONT_CARE, 0},
	{kVTalk_2d,    0,               kSTStalk_2d,        false, DONT_CARE, 0},
	{kVSwitch_2d,  0,               kSTMorespecific_2d, false, DONT_CARE, 0},
	{kVThrow_2d,   0,               kSTNopurps_2d,      false, DONT_CARE, 0},
	{kVAttack_2d,  0,               kSTSattack_2d,      false, DONT_CARE, 0},
	{kVBreak_2d,   0,               kSTSbreak_2d,       false, DONT_CARE, 0},
	{kVListen_2d,  0,               kSTQuiet_2d,        false, DONT_CARE, 0},
	{kVSmell_2d,   0,               kSTAroma_2d,        false, DONT_CARE, 0},
	{kVQuery_2d,   0,               kSTNoidea_2d,       false, DONT_CARE, 0},
	{kVLook_2d,    kNSky_2d,        kSTFalling_2d,      false, DONT_CARE, 0},
	{kVLook_2d,    kNWall_2d,       kSTDull_2d,         false, DONT_CARE, 0},
	{kVLook_2d,    kNGround_2d,     kSTDull_2d,         false, DONT_CARE, 0},
	{kVHelp_2d,    0,               kSTShelp_2d,        false, DONT_CARE, 0},
	{kVMagic_2d,   0,               kSTSmagic_2d,       false, DONT_CARE, 0},
	{kVWish_2d,    0,               kSTSmagic_2d,       false, DONT_CARE, 0},
	{kVDig_2d,     0,               kSTSdig_2d,         false, DONT_CARE, 0},
	{kVRude_2d,    0,               kSTSrude_2d,        false, DONT_CARE, 0},
	{kVKnock_2d,   0,               kSTNoanswer_2d,     false, DONT_CARE, 0},
	{kVOpen_2d,    kNDoor_2d,       kSTWontopen_2d,     false, DONT_CARE, 0},
	{kVUnlock_2d,  kNDoor_2d,       kSTCantunlock_2d,   false, DONT_CARE, 0},
	{kVLook_2d,    kNDoor_2d,       kSTDull_2d,         false, DONT_CARE, 0},
	{kVLook_2d,    kNLight_2d,      kSTDull_2d,         false, DONT_CARE, 0},
	{kVHello_2d,   0,               kSTHi_2d,           false, DONT_CARE, 0},
	{kVLook_2d,    kNFence_2d,      kSTLookover_2d,     false, DONT_CARE, 0},
	{kVLook_2d,    kNWall_2d,       kSTLookover_2d,     false, DONT_CARE, 0},
	{kVLook_2d,    kNGardenbits_2d, kSTDull_2d,         false, DONT_CARE, 0},
	{kVGive_2d,    0,               kSTNothanks_2d,     false, DONT_CARE, 0},
	{kVLook_2d,    kNTree_2d,       kSTDull2_2d,        false, DONT_CARE, 0},
	{kVFire_2d,    0,               kSTFire2_2d,        false, DONT_CARE, 0},
	{kVShout_2d,   0,               kSTNoanswer_2d,     false, DONT_CARE, 0},
	{kVHerring_2d, 0,               kSTSherring_2d,     false, DONT_CARE, 0},
	{kVUndress_2d, 0,               kSTSundress_2d,     false, DONT_CARE, 0},
	{kVSit_2d,     0,               kSTStired_2d,       false, DONT_CARE, 0},
	{kVFeed_2d,    0,               kSTNothanks_2d,     false, DONT_CARE, 0},
	{kVRub_2d,     kNCatnip_2d,     kSTRubcatnip1_2d,   false, DONT_CARE, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen0_desc_2d[] = {       // Outside house
	{kVLook_2d, 0, kSTWelcome_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen1_desc_2d[] = {       // Hall
	{kVLook_2d,   0,          kSTLookhall_2d,  true,  0, 0},
	{kVLook_2d,   kNMaid_2d,  kSTTmaid_2d,     false, 0, 0},
	{kVTalk_2d,   kNMaid_2d,  kSTChatmaid1_2d, false, 0, 0},
	{kVTalk_2d,   kNPenny_2d, kSTChatmaid2_2d, false, 0, 0},
	{kVKiss_2d,   kNMaid_2d,  kSTChatmaid3_2d, false, 0, 0},
	{kVRude_2d,   kNMaid_2d,  kSTRudemaid_2d,  false, 0, 0},
	{kVOpen_2d,   kNDoor_2d,  kSTTmaiddoor_2d, false, 0, 0},
	{kVUnlock_2d, kNDoor_2d,  kSTTmaiddoor_2d, false, 0, 0},
	{kVLook_2d,   kNDoor_2d,  kSTSdoor_2d,     false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen2_desc_2d[] = {       // Bed1
	{kVLook_2d,  0,          kSTLookbed1_2d, true,  DONT_CARE, 0},
	{kVLook_2d,  kNBed_2d,   kSTLookbed_2d,  false, DONT_CARE, 0},
	{kVInto_2d,  kNBed_2d,   kSTS2bed_2d,    false, 0,         0},
	{kVRide_2d,  kNBed_2d,   kSTS2bed_2d,    false, 0,         0},
	{kVRest_2d,  kNBed_2d,   kSTS2bed_2d,    false, 0,         0},
	{kVRide_2d,  kNPenny_2d, kSTS2bed_2d,    false, 0,         0},
	{kVTalk_2d,  kNPenny_2d, kSTZzzz_2d,     false, 0,         0},
	{kVInto_2d,  kNBed_2d,   kSTNopurps_2d,  false, 1,         0},
	{kVRide_2d,  kNBed_2d,   kSTNopurps_2d,  false, 1,         0},
	{kVRest_2d,  kNBed_2d,   kSTNopurps_2d,  false, 1,         0},
	{kVUnder_2d, kNBed_2d,   kSTSsearch_2d,  false, DONT_CARE, 0},
	{kVRead_2d,  kNBook_2d,  kSTNocarry_2d,  false, DONT_CARE, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen3_desc_2d[] = {       // Bed2
	{kVLook_2d,  0,           kSTLookbed2_2d,   true,  0, 0},
	{kVLook_2d,  kNBird_2d,   kSTParrot_2d,     false, 0, 0},
	{kVLook_2d,  kNCage_2d,   kSTDull_2d,       false, 0, 0},
	{kVLook_2d,  kNPencil_2d, kSTDull_2d,       false, 0, 0},
	{kVLook_2d,  kNPhone_2d,  kSTDull_2d,       false, 0, 0},
	{kVLook_2d,  kNPaper_2d,  kSTBlotter_2d,    false, 0, 0},
	{kVRead_2d,  kNPaper_2d,  kSTBlotter_2d,    false, 0, 0},
	{kVLook_2d,  kNChair_2d,  kSTDull_2d,       false, 0, 0},
	{kVTake_2d,  kNPencil_2d, kSTNouse_2d,      false, 0, 0},
	{kVTake_2d,  kNPaper_2d,  kSTNouse_2d,      false, 0, 0},
	{kVLook_2d,  kNHole_2d,   kSTDull_2d,       false, 0, 0},
	{kVLook_2d,  kNWall_2d,   kSTDumbwaiter_2d, false, 0, 0},
	{kVLook_2d,  kNButton_2d, kSTSbutton_2d,    false, 0, 0},
	{kVPush_2d,  kNButton_2d, kSTS3dumb_2d,     false, 0, 0},
	{kVDial_2d,  0,           kSTS3phone_2d,    false, 0, 4},
	{kVTake_2d,  kNPhone_2d,  kSTS3phone_2d,    false, 0, 4},
	{kVUse_2d,   kNPhone_2d,  kSTS3phone_2d,    false, 0, 4},
	{kVLift_2d,  kNPhone_2d,  kSTS3phone_2d,    false, 0, 4},
	{kVTalk_2d,  kNBird_2d,   kSTS3bird_2d,     false, 0, 5},
	{kVInto_2d,  0,           kSTSinto_2d,      true,  0, 0},
	{kVUnder_2d, kNCupb_2d,   kSTSsearch_2d,    false, 0, 0},
	{kVFeed_2d,  kNBird_2d,   kSTBirdfull_2d,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};


background_t screen4_desc_2d[] = {       // Keyhole
	{kVLook_2d, 0, kSTLooklook_2d, true, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen5_desc_2d[] = {       // Bed3
	{kVLook_2d,  0,           kSTLookbed3_2d,     true,  0, 0},
	{kVBlock_2d, 0,           kSTSblock_2d,       false, 0, 0},
	{kVInto_2d,  kNCage_2d,   kSTStoobigtofit_2d, false, 0, 0},
	{kVLook_2d,  kNWindow_2d, kSTS6garden_2d,     false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen6_desc_2d[] = {       // Kitchen
	{kVLook_2d,   0,           kSTLookkitchen_2d,   true,  DONT_CARE, 0},
	{kVLook_2d,   kNButton_2d, kSTSbutton_2d,       false, DONT_CARE, 0},
	{kVPush_2d,   kNButton_2d, kSTS3dumb_2d,        false, DONT_CARE, 0},
	{kVLook_2d,   kNWindow_2d, kSTS6garden_2d,      false, DONT_CARE, 0},
	{kVLook_2d,   kNUnits_2d,  kSTS6dull_2d,        false, DONT_CARE, 0},
	{kVOpen_2d,   kNUnits_2d,  kSTS6dull_2d,        false, DONT_CARE, 0},
	{kVInto_2d,   0,           kSTSinto_2d,         true,  DONT_CARE, 0},
	{kVOpen_2d,   kNDoor_2d,   kSTMorespecific_2d,  false, DONT_CARE, 0},
	{kVLook_2d,   kNDoor_2d,   kSTMorespecific_2d,  false, DONT_CARE, 0},
	{kVTalk_2d,   kNCook_2d,   kSTTalkcook_2d,      false, 1,         0},
	{kVLook_2d,   kNCook_2d,   kSTLookcook_2d,      false, 1,         0},
	{kVLook_2d,   kNKnife_2d,  kSTLookknife_2d,     false, 1,         0},
	{kVTake_2d,   kNKnife_2d,  kSTTakeknife_2d,     false, 1,         0},
	{kVListen_2d, 0,           kSTListenkitchen_2d, false, 1,         0},
	{0, 0, 0, false, 0, 0}
};

background_t screen7_desc_2d[] = {       // Backdoor
	{kVLook_2d, 0,           kSTLookback_2d, true,  0, 0},
	{kVLook_2d, kNWindow_2d, kSTLookwin_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen8_desc_2d[] = {       // Shed
	{kVLook_2d, 0,           kSTLookshed_2d,   true,  0, 0},
	{kVLook_2d, kNWindow_2d, kSTLookwin_2d,    false, 0, 0},
	{kVLook_2d, kNShed_2d,   kSTLookatshed_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen9_desc_2d[] = {       // In shed
	{kVLook_2d,  0,            kSTLookinshed_2d,  true,  0, 0},
	{kVLook_2d,  kNWall_2d,    kSTS9tools1_2d,    false, 0, 0},
	{kVLook_2d,  kNTools_2d,   kSTS9tools1_2d,    false, 0, 0},
	{kVTake_2d,  kNTools_2d,   kSTS9tools2_2d,    false, 0, 0},
	{kVLook_2d,  kNBroom_2d,   kSTDull_2d,        false, 0, 0},
	{kVTake_2d,  kNBroom_2d,   kSTNouse_2d,       false, 0, 0},
	{kVLook_2d,  kNTable_2d,   kSTSomebuttons_2d, false, 0, 0},
	{kVKiss_2d,  kNGardner_2d, kSTMore_2d,        false, 0, 0},
	{kVRude_2d,  kNGardner_2d, kSTRudeshed_2d,    false, 0, 0},
	{kVTalk_2d,  kNGardner_2d, kSTIgnore_2d,      false, 0, 0},
	{kVClose_2d, kNDoor_2d,    kSTShedclose_2d,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen10_desc_2d[] = {      // Venus fly traps
	{kVLook_2d, 0,           kSTLookvenus_2d, true,  0, 0},
	{kVTake_2d, kNMirror_2d, kSTNotmirror_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen11_desc_2d[] = {      // Gates kVOpen_2d,
	{kVLook_2d,  0,         kSTS11look_2d, true,  0, 0},
	{kVClose_2d, kNDoor_2d, kSTNopurps_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen12_desc_2d[] = {      // Gates closed
	{kVLook_2d,   0,         kSTS12look_2d, true,  0, 0},
	{kVOpen_2d,   kNDoor_2d, kSTGates1_2d,  false, 0, 0},
	{kVBreak_2d,  kNDoor_2d, kSTGates2_2d,  false, 0, 0},
	{kVAttack_2d, kNDoor_2d, kSTGates2_2d,  false, 0, 0},
	{kVUnlock_2d, kNDoor_2d, kSTGates3_2d,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen13_desc_2d[] = {      // Stream
	{kVLook_2d,  0,            kSTS13look_2d,    true,  0, 0},
	{kVLook_2d,  kNBridge_2d,  kSTNospecial_2d,  false, 0, 0},
	{kVUnder_2d, kNBridge_2d,  kSTSsearch_2d,    false, 0, 0},
	{kVLook_2d,  kNWater_2d,   kSTDull_2d,       false, 0, 0},
	{kVThrow_2d, kNMatches_2d, kSTThrowmatch_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen14_desc_2d[] = {      // Zapper
	{kVLook_2d,   0,           kSTS14look_2d,  true,  0, 0},
	{kVSwitch_2d, kNZapper_2d, kSTNoswitch_2d, false, 0, 0},
	{kVWind_2d,   kNZapper_2d, kSTNoswitch_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen15_desc_2d[] = {      // Mushroom
	{kVLook_2d,   0,         kSTS15look_2d,  true,  0, 0},
	{kVTalk_2d,   kNMan_2d,  kSTNoreply_2d,  false, 0, 0},
	{kVLook_2d,   kNWand_2d, kSTS15wand1_2d, false, 0, 0},
	{kVTake_2d,   kNWand_2d, kSTS15wand2_2d, false, 0, 0},
	{kVSearch_2d, kNMan_2d,  kSTSsearch_2d,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen16_desc_2d[] = {      // Well
	{kVLook_2d,  0,           kSTS16look_2d,     true,  0, 0},
	{kVClimb_2d, 0,           kSTTryrope_2d,     false, 0, 0},
	{kVGo_2d,    kNWell_2d,   kSTTryrope_2d,     false, 0, 0},
	{kVWind_2d,  kNRope_2d,   kSTWindwell_2d,    false, 0, 0},
	{kVTie_2d,   kNRope_2d,   kSTNopurps_2d,     false, 0, 0},
	{kVTake_2d,  kNBucket_2d, kSTNosee_2d,       false, 0, 0},
	{kVLook_2d,  kNBucket_2d, kSTNosee_2d,       false, 0, 0},
	{kVWind_2d,  kNBucket_2d, kSTWindwell_2d,    false, 0, 0},
	{kVTake_2d,  kNWater_2d,  kSTNosee_2d,       false, 0, 0},
	{kVWind_2d,  kNHandle_2d, kSTWindwell_2d,    false, 0, 0},
	{kVInto_2d,  kNBucket_2d, kSTNosee_2d,       false, 0, 0},
	{kVInto_2d,  kNWell_2d,   kSTIntowell_2d,    false, 0, 0},
	{kVWish_2d,  0,           kSTGetonwithit_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen17_desc_2d[] = {      // Snakepit
	{kVLook_2d,   0,          kSTS17look_2d, true,  0, 0},
	{kVAttack_2d, kNSnake_2d, kSTS17kill_2d, false, 0, 0},
	{kVBreak_2d,  kNSnake_2d, kSTS17kill_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen18_desc_2d[] = {      // Phonebox
	{kVLook_2d, 0,          kSTS18look_2d, true,  0, 0},
	{kVLook_2d, kNPhone_2d, kSTS18look_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};


background_t screen19_desc_2d[] = {      // Street
	{kVLook_2d, 0,         kSTS19look_2d, true,  0, 0},
	{kVLook_2d, kNWall_2d, kSTSgraf_2d,   false, 0, 0},
	{kVRead_2d, kNWall_2d, kSTSgraf_2d,   false, 0, 0},
	{kVLook_2d, kNGraf_2d, kSTSgraf_2d,   false, 0, 0},
	{kVRead_2d, kNGraf_2d, kSTSgraf_2d,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen20_desc_2d[] = {      // Kennel
	{kVLook_2d,   0,           kSTS20look_2d,      true,  0, 0},
	{kVLook_2d,   kNWindow_2d, kSTMorespecific_2d, false, 0, 0},
	{kVThrow_2d,  kNStick_2d,  kSTDonthaveone_2d,  false, 0, 0},
	{kVStroke_2d, kNDog_2d,    kSTStrokedog_2d,    false, 0, 0},
	{kVTalk_2d,   kNDog_2d,    kSTStrokedog_2d,    false, 0, 0},
	{kVInto_2d,   0,           kSTStoobigtofit_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen21_desc_2d[] = {      // Rockroom
	{kVLook_2d,  0,         kSTS21look_2d,   true,  0, 0},
	{kVClimb_2d, kNRope_2d, kSTNotclose_2d,  false, 0, 0},
	{kVUnder_2d, kNRock_2d, kSTNounder_2d,   false, 0, 0},
	{kVMove_2d,  kNRock_2d, kSTTooheavy_2d,  false, 0, 0},
	{kVLift_2d,  kNRock_2d, kSTNowayhose_2d, false, 0, 0},
	{kVLook_2d,  kNRock_2d, kSTDull_2d,      false, 0, 0},
	{kVTake_2d,  kNRock_2d, kSTNowayhose_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen22_desc_2d[] = {      // Rockgone
	{kVLook_2d,  0,         kSTS22look_2d,  true,  0, 0},
	{kVClimb_2d, kNRope_2d, kSTNotclose_2d, false, 0, 0},
	{kVLift_2d,  kNRock_2d, kSTNopurps_2d,  false, 0, 0},
	{kVLook_2d,  kNRock_2d, kSTDull_2d,     false, 0, 0},
	{kVTake_2d,  kNRock_2d, kSTNopurps_2d,  false, 0, 0},
	{kVUnder_2d, kNRock_2d, kSTSsearch_2d,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen23_desc_2d[] = {      // Threeway
	{kVLook_2d,  0,         kSTS23look_2d, true,  0, 0},
	{kVLift_2d,  kNRock_2d, kSTNopurps_2d, false, 0, 0},
	{kVLook_2d,  kNRock_2d, kSTDull_2d,    false, 0, 0},
	{kVUnder_2d, kNRock_2d, kSTDull2_2d,   false, 0, 0},
	{kVTake_2d,  kNRock_2d, kSTNopurps_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};


background_t screen24_desc_2d[] = {      // Lampcave
	{kVLook_2d,  0,         kSTS24look_2d,   true,  0, 0},
	{kVLift_2d,  kNRock_2d, kSTSsearch_2d,   false, 0, 0},
	{kVLook_2d,  kNRock_2d, kSTNospecial_2d, false, 0, 0},
	{kVUnder_2d, kNRock_2d, kSTDull2_2d,     false, 0, 0},
	{kVTake_2d,  kNRock_2d, kSTNopurps_2d,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen25_desc_2d[] = {      // Chasm
	{kVLook_2d,  0,         kSTS25look_2d, true,  0, 0},
	{kVUnder_2d, kNRock_2d, kSTSsearch_2d, false, 0, 0},
	{kVLift_2d,  kNRock_2d, kSTSsearch_2d, false, 0, 0},
	{kVLook_2d,  kNRock_2d, kSTDull_2d,    false, 0, 0},
	{kVTake_2d,  kNRock_2d, kSTNopurps_2d, false, 0, 0},
	{kVJump_2d,  0,         kSTSnojump_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen26_desc_2d[] = {      // Passage
	{kVLook_2d,  0,         kSTS26look_2d, true,  0, 0},
	{kVUnder_2d, kNRock_2d, kSTSsearch_2d, false, 0, 0},
	{kVLift_2d,  kNRock_2d, kSTSsearch_2d, false, 0, 0},
	{kVLook_2d,  kNRock_2d, kSTDull_2d,    false, 0, 0},
	{kVTake_2d,  kNRock_2d, kSTNopurps_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen27_desc_2d[] = {      // genie
	{kVLook_2d,    0,           kSTS27look_2d,   true,  0, 0},
	{kVUnder_2d,   kNRock_2d,   kSTSsearch_2d,   false, 0, 0},
	{kVLift_2d,    kNRock_2d,   kSTSsearch_2d,   false, 0, 0},
	{kVLook_2d,    kNRock_2d,   kSTDull_2d,      false, 0, 0},
	{kVLook_2d,    kNTrap_2d,   kSTBudge_2d,     false, 0, 0},
	{kVOpen_2d,    kNTrap_2d,   kSTBudge_2d,     false, 0, 0},
	{kVUnscrew_2d, kNTrap_2d,   kSTBudge_2d,     false, 0, 0},
	{kVUnlock_2d,  kNTrap_2d,   kSTBudge_2d,     false, 0, 0},
	{kVPush_2d,    kNTrap_2d,   kSTBudge_2d,     false, 0, 0},
	{kVTalk_2d,    kNGenie_2d,  kSTTalkgenie_2d, false, 0, 0},
	{kVRude_2d,    kNGenie_2d,  kSTRudeshed_2d,  false, 0, 0},
	{kVKiss_2d,    kNGenie_2d,  kSTRudeshed_2d,  false, 0, 0},
	{kVGive_2d,    kNBanana_2d, kSTNobanana_2d,  false, 0, 0},
	{kVClimb_2d,   kNStairs_2d, kSTTrywalk_2d,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen28_desc_2d[] = {      // traproom
	{kVLook_2d,    0,              kSTS28look_2d,      true,  0, 0},
	{kVBreak_2d,   kNSafe_2d,      kSTNowayhose_2d,    false, 0, 0},
	{kVLook_2d,    kNHole_2d,      kSTS28hole_2d,      false, 0, 28},
	{kVTake_2d,    kNMouse_2d,     kSTS28mouse_2d,     false, 0, 0},
	{kVTake_2d,    kNDroppings_2d, kSTSdroppings_2d,   false, 0, 0},
	{kVUnscrew_2d, kNSafe_2d,      kSTMorespecific_2d, false, 0, 0},
	{kVOpen_2d,    0,              kSTUnlocksafe_2d,   false, 0, 0},
	{kVUnlock_2d,  kNSafe_2d,      kSTUnlocksafe_2d,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen29_desc_2d[] = {      // Hall 2
	{kVLook_2d, 0, kSTLookhall_2d, true, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen30_desc_2d[] = {      // Lounge
	{kVLook_2d, 0,           kSTS30look_2d,  true,  0, 0},
	{kVLook_2d, kNWindow_2d, kSTS6garden_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen31_desc_2d[] = {      // parlor
	{kVLook_2d,   0,         kSTS31look_2d,      true,  0, 0},
	{kVListen_2d, 0,         kSTBlah_2d,         false, 0, 0},
	{kVOpen_2d,   kNDoor_2d, kSTMorespecific_2d, false, 0, 0},
	{kVLook_2d,   kNDoor_2d, kSTMorespecific_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen32_desc_2d[] = {      // catroom
	{kVLook_2d,   0,           kSTS32look_2d,    true,  0, 0},
	{kVStroke_2d, kNCat_2d,    kSTSstrokecat_2d, false, 0, 0},
	{kVPlay_2d,   kNCat_2d,    kSTSplaycat_2d,   false, 0, 0},
	{kVTalk_2d,   kNCat_2d,    kSTStalkcat_2d,   false, 0, 0},
	{kVLook_2d,   kNPost_2d,   kSTSlookpost_2d,  false, 0, 0},
	{kVGive_2d,   0,           kSTSgivecat_2d,   false, 0, 0},
	{kVLook_2d,   kNWindow_2d, kSTS6garden_2d,   false, 0, 0},
	{kVRub_2d,    kNCatnip_2d, kSTRubcatnip2_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen33_desc_2d[] = {      // Boxroom
	{kVLook_2d,  0,          kSTS33look_2d,      true,  0, 0},
	{kVLook_2d,  kNDoor_2d,  kSTLookboxdoor_2d,  false, 0, 0},
	{kVRead_2d,  kNPaper_2d, kSTReadpaper_2d,    false, 0, 29},
	{kVLook_2d,  kNPaper_2d, kSTReadpaper_2d,    false, 0, 29},
	{kVLook_2d,  kNCrate_2d, kSTSlookbox_2d,     false, 0, 0},
	{kVInto_2d,  kNCrate_2d, kSTSgetinbox_2d,    false, 0, 0},
	{kVLook_2d,  kNChute_2d, kSTLookchute_2d,    false, 0, 0},
	{kVLook_2d,  kNHole_2d,  kSTLookchute_2d,    false, 0, 0},
	{kVClimb_2d, kNChute_2d, kSTUpchute_2d,      false, 0, 0},
	{kVLook_2d,  kNWall_2d,  kSTLookchute_2d,    false, 0, 0},
	{kVInto_2d,  kNDoor_2d,  kSTMorespecific_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen34_desc_2d[] = {      // hall3
	{kVLook_2d, 0,           kSTLookhall_2d,  true,  0, 0},
	{kVLook_2d, kNMirror_2d, kSTNospecial_2d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen35_desc_2d[] = {      // Organ
	{kVLook_2d,   0,          kSTS35look_2d,   true,  DONT_CARE, 0},
	{kVLook_2d,   kNOrgan_2d, kSTNospecial_2d, false, DONT_CARE, 0},
	{kVPlay_2d,   kNOrgan_2d, kSTPlayorgan_2d, false, DONT_CARE, 0},
	{kVListen_2d, 0,          kSTHearorgan_2d, false, 0,         0},
	{kVListen_2d, 0,          kSTHearlaugh_2d, false, 1,         0},
	{0, 0, 0, false, 0, 0}
};

background_t screen36_desc_2d[] = {      // Hestroom
	{kVLook_2d, 0,           kSTLookhest_2d,   true,  0, 0},
	{kVTake_2d, kNBook_2d,   kSTS36book_2d,    false, 0, 0},
	{kVRead_2d, kNBook_2d,   kSTS36book_2d,    false, 0, 0},
	{kVLook_2d, kNBook_2d,   kSTS36book_2d,    false, 0, 0},
	{kVLook_2d, kNTable_2d,  kSTS36table_2d,   false, 0, 0},
	{kVTalk_2d, kNHester_2d, kSTTalkhester_2d, false, 0, 0},
	{kVLook_2d, kNWindow_2d, kSTS6garden_2d,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen37_desc_2d[] = {      // Retupmoc
	{kVLook_2d,  0,           kSTS37look_2d,   true,  0, 0},
	{kVLift_2d,  kNRock_2d,   kSTNopurps_2d,   false, 0, 0},
	{kVLook_2d,  kNRock_2d,   kSTNospecial_2d, false, 0, 0},
	{kVUnder_2d, kNRock_2d,   kSTDull2_2d,     false, 0, 0},
	{kVTalk_2d,  kNDoctor_2d, kSTPleasego_2d,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t screen38_desc_2d[] = {      // hall1
	{kVLook_2d, 0, kSTS38look_2d, true, 0, 0},
	{0, 0, 0, false, 0, 0}
};

objectList_t backgroundList_2d[] =  {
	screen0_desc_2d,  screen1_desc_2d,  screen2_desc_2d,  screen3_desc_2d,  screen4_desc_2d,
	screen5_desc_2d,  screen6_desc_2d,  screen7_desc_2d,  screen8_desc_2d,  screen9_desc_2d,
	screen10_desc_2d, screen11_desc_2d, screen12_desc_2d, screen13_desc_2d, screen14_desc_2d,
	screen15_desc_2d, screen16_desc_2d, screen17_desc_2d, screen18_desc_2d, screen19_desc_2d,
	screen20_desc_2d, screen21_desc_2d, screen22_desc_2d, screen23_desc_2d, screen24_desc_2d,
	screen25_desc_2d, screen26_desc_2d, screen27_desc_2d, screen28_desc_2d, screen29_desc_2d,
	screen30_desc_2d, screen31_desc_2d, screen32_desc_2d, screen33_desc_2d, screen34_desc_2d,
	screen35_desc_2d, screen36_desc_2d, screen37_desc_2d, screen38_desc_2d
};

background_t catchall_3d[] = {  // Generally applicable phrases
	{kVLook_3d,    kNPenny_3d,  kSTLookpen_3d,      false, DONT_CARE, 0},
	{kVKiss_3d,    kNPenny_3d,  kSTMmmm_3d,         false, DONT_CARE, 0},
	{kVRude_3d,    kNPenny_3d,  kSTHeadache_3d,     false, DONT_CARE, 0},
	{kVNaughty_3d, kNPenny_3d,  kSTHeadache_3d,     false, DONT_CARE, 0},
	{kVInto_3d,    kNWindow_3d, kSTThruwindow_3d,   false, DONT_CARE, 0},
	{kVOutof_3d,   kNWindow_3d, kSTThruwindow_3d,   false, DONT_CARE, 0},
	{kVJump_3d,    0,           kSTSjump_3d,        false, DONT_CARE, 0},
	{kVGo_3d,      0,           kSTTrywalk_3d,      false, DONT_CARE, 0},
	{kVInto_3d,    0,           kSTTrywalk_3d,      false, DONT_CARE, 0},
	{kVClimb_3d,   0,           kSTSclimb_3d,       false, DONT_CARE, 0},
	{kVShout_3d,   0,           kSTNothing_3d,      false, DONT_CARE, 0},
	{kVTalk_3d,    0,           kSTStalk_3d,        false, DONT_CARE, 0},
	{kVSwitch_3d,  0,           kSTMorespecific_3d, false, DONT_CARE, 0},
	{kVUse_3d,     0,           kSTMorespecific_3d, false, DONT_CARE, 0},
	{kVThrow_3d,   0,           kSTNopurps_3d,      false, DONT_CARE, 0},
	{kVRude_3d,    0,           kSTSrude_3d,        false, DONT_CARE, 0},
	{kVAttack_3d,  0,           kSTSattack_3d,      false, DONT_CARE, 0},
	{kVBreak_3d,   0,           kSTSbreak_3d,       false, DONT_CARE, 0},
	{kVListen_3d,  0,           kSTQuiet_3d,        false, DONT_CARE, 0},
	{kVSmell_3d,   0,           kSTAroma_3d,        false, DONT_CARE, 0},
	{kVQuery_3d,   0,           kSTNoidea_3d,       false, DONT_CARE, 0},
	{kVLook_3d,    kNSky_3d,    kSTFalling_3d,      false, DONT_CARE, 0},
	{kVLook_3d,    kNWall_3d,   kSTDull_3d,         false, DONT_CARE, 0},
	{kVLook_3d,    kNGround_3d, kSTDull_3d,         false, DONT_CARE, 0},
	{kVHelp_3d,    0,           kSTShelp_3d,        false, DONT_CARE, 0},
	{kVMagic_3d,   0,           kSTSmagic_3d,       false, DONT_CARE, 0},
	{kVWish_3d,    0,           kSTSmagic_3d,       false, DONT_CARE, 0},
	{kVDig_3d,     0,           kSTSdig_3d,         false, DONT_CARE, 0},
	{kVNaughty_3d, 0,           kSTSnaughty_3d,     false, DONT_CARE, 0},
	{kVKnock_3d,   0,           kSTNoanswer_3d,     false, DONT_CARE, 0},
	{kVOpen_3d,    kNDoor_3d,   kSTWontopen_3d,     false, DONT_CARE, 0},
	{kVUnlock_3d,  kNDoor_3d,   kSTCantunlock_3d,   false, DONT_CARE, 0},
	{kVLook_3d,    kNDoor_3d,   kSTDull_3d,         false, DONT_CARE, 0},
	{kVHello_3d,   0,           kSTHi_3d,           false, DONT_CARE, 0},
	{kVGive_3d,    0,           kSTNothanks_3d,     false, DONT_CARE, 0},
	{kVShout_3d,   0,           kSTNoanswer_3d,     false, DONT_CARE, 0},
	{kVUndress_3d, 0,           kSTSundress_3d,     false, DONT_CARE, 0},
	{kVSit_3d,     0,           kSTStired_3d,       false, DONT_CARE, 0},
	{kVFeed_3d,    0,           kSTNothanks_3d,     false, DONT_CARE, 0},
	{kVTake_3d,    kNVine_3d,   kSTNopurps_3d,      false, DONT_CARE, 0},
	{kVClimb_3d,   kNJungle_3d, kSTNopurps_3d,      false, DONT_CARE, 0},
	{kVLook_3d,    kNJungle_3d, kSTLookjungle_3d,   false, DONT_CARE, 0},
	{kVPut_3d,     kNFire_3d,   kSTNopurps_3d,      false, DONT_CARE, 0},
	{kVSwim_3d,    0,           kSTCantswim_3d,     false, DONT_CARE, 0},
	{kVTake_3d,    kNMouse_3d,  kSTCantcatch_3d,    false, DONT_CARE, 0},
	{kVLook_3d,    kNMouse_3d,  kSTNospecial_3d,    false, DONT_CARE, 0},
	{0, 0, 0, false, 0, 0}
};

background_t crash_desc_3d[] = {      // At the crash site
	{kVLook_3d,   0,          kSTLookcrash_3d,     false, 0, 0},
	{kVRepair_3d, kNPlane_3d, kSTNopurps_3d,       false, 0, 0},
	{kVFly_3d,    kNPlane_3d, kSTNopurps_3d,       false, 0, 0},
	{kVInto_3d,   0,          kSTMorespecific_3d,  true,  0, 0},
	{kVOpen_3d,   kNDoor_3d,  kSTOpenplanedoor_3d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t plane_desc_3d[] = {      // Inside the plane
	{kVLook_3d,   0, kSTLookplane_3d, true,  0, 0},
	{kVSearch_3d, 0, kSTSaylook_3d,   false, 0, 0},
	{kVInto_3d,   0, kSTYouarein_3d,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t web_desc_3d[] = {      // At the spider's web
	{kVLook_3d, 0,           kSTLookweb_3d,   true,  0, 0},
	{kVTake_3d, kNNative_3d, kSTTakegirl_3d,  false, 0, 0},
	{kVLook_3d, kNWeb_3d,    kSTLookatweb_3d, false, 0, 0},
	{kVTake_3d, kNPenny_3d,  kSTTakepenny_3d, false, 0, 0},
	{kVTalk_3d, kNPenny_3d,  kSTTalkpenny_3d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t wfall_desc_3d[] = {      // Waterfall and stream
	{kVLook_3d,   0,          kSTLookwfall_3d,  true,  0, 0},
	{kVLook_3d,   kNWater_3d, kSTLookwfall_3d,  false, 0, 0},
	{kVCross_3d,  kNWater_3d, kSTCantcross_3d,  false, 0, 0},
	{kVListen_3d, 0,          kSTListenfall_3d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t wfall_b_desc_3d[] = {      // Same as above but no water
	{kVLook_3d,  0,          kSTLookwfall_b_3d, true,  0, 0},
	{kVLook_3d,  kNWater_3d, kSTLookwfall_b_3d, false, 0, 0},
	{kVCross_3d, kNWater_3d, kSTToomuddy_3d,    false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t wbase_desc_3d[] = {      // Base of waterfall
	{kVLook_3d,  0,          kSTLookwbase_3d, true,  0, 0},
	{kVLook_3d,  kNWater_3d, kSTLookwbase_3d, false, 0, 0},
	{kVCross_3d, kNWater_3d, kSTToomuddy_3d,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t path_ul_desc_3d[] = {      // Path at left of spider's web
	{kVLook_3d, 0, kSTLookpath_ul_3d, true, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t bridge_desc_3d[] = {      // At the bridge
	{kVLook_3d,  0,         kSTLookbridge1_3d,    true,  0, 0},
	{kVSwing_3d, 0,         kSTSwingbridge_3d,    false, 0, 0},
	{kVTake_3d,  kNVine_3d, kSTGetbridgevines_3d, false, 0, 0},
	{kVTie_3d,   kNThem_3d, kSTMorespecific_3d,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t bridge2_desc_3d[] = {      // At the bridge
	{kVLook_3d,  0, kSTLookbridge2_3d, true,  0, 0},
	{kVSwing_3d, 0, kSTSwingbridge_3d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t stream_desc_3d[] = {      // stream with vines crossing
	{kVLook_3d,  0, kSTLookstream1_3d,  true,  0, 0},
	{kVSwing_3d, 0, kSTMorespecific_3d, false, 0, 0},
	{kVCross_3d, 0, kSTStep1_3d,        false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t stream2_desc_3d[] = {      // Stream with elephant sitting in it
	{kVLook_3d,  0,             kSTLookstream2_3d, true,  0, 0},
	{kVSwing_3d, 0,             kSTCantswing_3d,   false, 0, 0},
	{kVLook_3d,  kNElephant_3d, kSTLookele2_3d,    false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t village_desc_3d[] = {      // Long shot of village
	{kVLook_3d, 0,         kSTLookvillage_3d, true,  0, 0},
	{kVTake_3d, kNFood_3d, kSTTakething_3d,   false, 0, 0},
	{kVLook_3d, kNFood_3d, kSTTakething_3d,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t hut_out_desc_3d[] = {      // Outside witch doctor's hut
	{kVLook_3d,   0,            kSTLookhut_out_3d, false, 0, 0},
	{kVLook_3d,   kNDocbits_3d, kSTLookdocbits_3d, false, 0, 0},
	{kVTake_3d,   kNDocbits_3d, kSTTakedocbits_3d, false, 0, 0},
	{kVLook_3d,   kNSpider_3d,  kSTLookspider_3d,  false, 0, 0},
	{kVTake_3d,   kNSpider_3d,  kSTTakespider_3d,  false, 0, 0},
	{kVLook_3d,   kNSnake_3d,   kSTLooksnake_3d,   false, 0, 0},
	{kVTake_3d,   kNSnake_3d,   kSTTakesnake_3d,   false, 0, 0},
	{kVSearch_3d, kNWindow_3d,  kSTLookinhut_3d,   false, 0, 0},
	{kVLook_3d,   kNWindow_3d,  kSTLookinhut_3d,   false, 0, 0},
	{kVSearch_3d, kNHut_3d,     kSTLookinhut_3d,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t hut_in_desc_3d[] = {      // Inside hut
// States: 0 imprisoned_3d, 1 doctor incapacitated
	{kVLook_3d,   0,              kSTLookhut_in_3d,     true,  0,         0},
	{kVLook_3d,   0,              kSTLookhut_in2_3d,    true,  1,         0},
	{kVLook_3d,   kNDoctor_3d,    kSTLookhut_in2_3d,    false, 1,         0},
	{kVTake_3d,   0,              kSTTakeincage_3d,     false, 0,         0},
	{kVTake_3d,   kNDoctor_3d,    kSTTakedoctor0_3d,    false, 0,         0},
	{kVTake_3d,   kNDoctor_3d,    kSTTakedoctor1_3d,    false, 1,         0},
	{kVLook_3d,   kNDocbits_3d,   kSTLookdocbits_3d,    false, DONT_CARE, 0},
	{kVTake_3d,   kNDocbits_3d,   kSTTakedocbits_3d,    false, DONT_CARE, 0},
	{kVLook_3d,   kNSpider_3d,    kSTLookspider_3d,     false, DONT_CARE, 0},
	{kVTake_3d,   kNSpider_3d,    kSTTakespider_3d,     false, DONT_CARE, 0},
	{kVLook_3d,   kNSnake_3d,     kSTLooksnake_3d,      false, DONT_CARE, 0},
	{kVTake_3d,   kNSnake_3d,     kSTTakesnake_3d,      false, DONT_CARE, 0},
	{kVLook_3d,   kNWindow_3d,    kSTLookouthut_3d,     false, DONT_CARE, 0},
	{kVLook_3d,   kNShelfbits_3d, kSTLookshelfbits_3d,  false, 0,         0},
	{kVLook_3d,   kNShelfbits_3d, kSTLookshelfbits2_3d, false, 1,         0},
	{kVTake_3d,   kNShelfbits_3d, kSTTakeshelfbits_3d,  false, 1,         0},
	{kVDrink_3d,  kNShelfbits_3d, kSTTakeshelfbits_3d,  false, 1,         0},
	{kVLook_3d,   kNBottles_3d,   kSTLookshelfbits_3d,  false, 0,         0},
	{kVLook_3d,   kNBottles_3d,   kSTLookshelfbits2_3d, false, 1,         0},
	{kVTake_3d,   kNBottles_3d,   kSTTakeshelfbits_3d,  false, 1,         0},
	{kVDrink_3d,  kNBottles_3d,   kSTTakeshelfbits_3d,  false, 1,         0},
	{kVSearch_3d, 0,              kSTCantlookin_3d,     false, 0,         0},
	{kVSearch_3d, kNFire_3d,      kSTLookinfire_3d,     false, 1,         0},
	{kVLook_3d,   kNFire_3d,      kSTLookfire_3d,       false, DONT_CARE, 0},
	{kVTalk_3d,   kNDoctor_3d,    kSTTalkdoc_3d,        false, 0,         0},
	{kVTalk_3d,   kNDoctor_3d,    kSTTalkdoc2_3d,       false, 1,         0},
	{kVSearch_3d, kNMouse_3d,     kSTLookinhole_3d,     false, DONT_CARE, 0},
	{kVTalk_3d,   kNMouse_3d,     kSTTalkmouse_3d,      false, DONT_CARE, 0},
	{kVUnlock_3d, kNCdoor_3d,     kSTPicklock_3d,       false, 0,         0},
	{kVTake_3d,   kNDoorlock_3d,  kSTPicklock_3d,       false, 0,         0},
	{kVGive_3d,   kNMouse_3d,     kSTGivemouse_3d,      false, DONT_CARE, 0},
	{kVThrow_3d,  kNCheese_3d,    kSTGivemouse_3d,      false, DONT_CARE, 0},
	{kVInto_3d,   kNFire_3d,      kSTGetinpot_3d,       false, 1,         0},
	{kVTake_3d,   kNNative_3d,    kSTTakenative_3d,     false, 0,         0},
	{kVShoot_3d,  kNDoctor_3d,    kSTMissed_3d,         false, DONT_CARE, 0},
	{kVBlow_3d,   kNDoctor_3d,    kSTMissed_3d,         false, DONT_CARE, 0},
	{kVUse_3d,    kNPipe_3d,      kSTMissed_3d,         false, DONT_CARE, 0},
	{kVClose_3d,  kNCdoor_3d,     kSTNotclose_3d,       false, DONT_CARE, 0},
	{0, 0, 0, false, 0, 0}
};

background_t garden_desc_3d[] = {      // The secret garden
	{kVLook_3d,  0,           kSTLookgarden_3d,  true,  0, 0},
	{kVLook_3d,  kNOrchid_3d, kSTLookorchid_3d,  false, 0, 0},
	{kVTake_3d,  kNOrchid_3d, kSTTakeorchid_3d,  false, 0, 0},
	{kVCross_3d, 0,           kSTCrossgarden_3d, false, 0, 0},
	{kVLook_3d,  kNWater_3d,  kSTLookgarden_3d,  false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t clifftop_desc_3d[] = {      // Top of cliff path
	{kVLook_3d, 0, kSTLookclifftop_3d, true, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t oldman_desc_3d[] = {      // Old man inside cave
	{kVLook_3d, 0, kSTLookoldman_3d, true, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t cliff_desc_3d[] = {      // Lower cliff path
	{kVLook_3d, 0, kSTLookcliff_3d, true, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t camp_desc_3d[] = {      // Camp scene in village
	{kVLook_3d,   0,             kSTLookcamp_3d,    true,  0, 0},
	{kVLook_3d,   kNFire_3d,     kSTLookhyena_3d,   false, 0, 0},
	{kVLook_3d,   kNPole_3d,     kSTLookpole_3d,    false, 0, 0},
	{kVBehind_3d, kNHut_3d,      kSTBehindhut_3d,   false, 0, 0},
	{kVSearch_3d, kNWindow_3d,   kSTLookintohut_3d, false, 0, 0},
	{kVSearch_3d, kNHut_3d,      kSTLookintohut_3d, false, 0, 0},
	{kVLook_3d,   kNHut_3d,      kSTLookhut_3d,     false, 0, 0},
	{kVLook_3d,   kNWindow_3d,   kSTLookintohut_3d, false, 0, 0},
	{kVEat_3d,    0,             kSTEatroast_3d,    false, 0, 0},
	{kVTake_3d,   kNFood_3d,     kSTEatroast_3d,    false, 0, 0},
	{kVInto_3d,   kNFire_3d,     kSTIntofire_3d,    false, 0, 0},
	{kVTake_3d,   kNFire_3d,     kSTIntofire_3d,    false, 0, 0},
	{kVTake_3d,   kNNative_3d,   kSTTakenative_3d,  false, 0, 0},
	{kVTake_3d,   kNPipe_3d,     kSTMakeoffer_3d,   false, 0, 0},
	{kVGive_3d,   kNBouillon_3d, kSTNonecarried_3d, false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t turn_desc_3d[] = {      // Turnaround path
	{kVLook_3d,  0,         kSTLookturn_3d,  true,  DONT_CARE, 0},
	{kVUnder_3d, kNRock_3d, kSTUnderrock_3d, false, 0,         0},
	{kVLook_3d,  kNRock_3d, kSTLookrock_3d,  false, 0,         0},
	{kVRide_3d,  kNRock_3d, kSTOntorock_3d,  false, 0,         0},
	{kVClimb_3d, kNRock_3d, kSTOntorock_3d,  false, 0,         0},
	{0, 0, 0, false, 0, 0}
};

background_t slope_desc_3d[] = {      // Slope between cliff and stream
	{kVLook_3d, 0, kSTLookslope_3d, true, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t path_desc_3d[] = {       // Path containing elephant
// States: 0 Elephant present_3d, 1 - Elephant not present
	{kVLook_3d, 0,             kSTLookpath2_1_3d,  true,  0, 0},
	{kVLook_3d, 0,             kSTLookpath2_2_3d,  true,  1, 0},
	{kVRide_3d, kNElephant_3d, kSTRideelephant_3d, false, 0, 0},
	{kVTake_3d, kNElephant_3d, kSTGetelephant_3d,  false, 0, 0},
	{kVShow_3d, kNMouse_3d,    kSTShowmouse_3d,    false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t cave_desc_3d[] = {      // Cave mouth
	{kVLook_3d,   0,          kSTLookcave1_3d,   true,  0, 0},
	{kVLook_3d,   0,          kSTLookcave2_3d,   true,  1, 0},
	{kVAttack_3d, kNGhost_3d, kSTAttackghost_3d, false, 0, 0},
	{kVBreak_3d,  kNGhost_3d, kSTAttackghost_3d, false, 0, 0},
	{kVShoot_3d,  kNGhost_3d, kSTAttackghost_3d, false, 0, 0},
	{kVTake_3d,   kNGhost_3d, kSTTakeghost_3d,   false, 0, 0},
	{0, 0, 0, false, 0, 0}
};

background_t *backgroundList_3d[] =  {
	crash_desc_3d,   web_desc_3d,     bridge_desc_3d,  bridge2_desc_3d, clifftop_desc_3d,
	wfall_desc_3d,   wfall_b_desc_3d, wbase_desc_3d,   stream_desc_3d,  stream2_desc_3d,
	path_ul_desc_3d, village_desc_3d, hut_out_desc_3d, hut_in_desc_3d,  garden_desc_3d,
	oldman_desc_3d,  cliff_desc_3d,   slope_desc_3d,   camp_desc_3d,    bgDummy,
	turn_desc_3d,    plane_desc_3d,   bgDummy,         path_desc_3d,    cave_desc_3d
};

byte points_1w[] = {
	0,  11, 8,  9,  17,
	12, 7,  33, 21, 8,
	8,  10, 5
};

byte points_2w[] = {
	0,  3,  5,  3,  5,
	1,  10, 15, 10, 10,
	15, 5,  5,  5,  15,
	10, 5,  20, 7,  12,
	8,  10, 5,  5,  15,
	1,  7,  12, 1,  1,
	9
};

byte points_3w[] = {
	0,  2,  3, 4, 7,
	3,  5,  9, 2, 4,
	8,  10, 1, 0, 3,
	12, 2
};

byte points_1d[] = {
	5,  11, 8,  9,  17,
	12, 7,  33, 21, 8
};

byte points_2d[] = {
	0,  3,  5,  3,  5,
	1,  10, 15, 10, 10,
	15, 5,  5,  5,  15,
	10, 5,  20, 7,  12,
	8,  10, 5,  5,  15,
	1,  7,  12, 1,  1,
	9
};

byte points_3d[] = {
	0,  2,  3, 4, 7,
	3,  5,  9, 2, 4,
	8,  10, 1, 0, 3,
	12, 2
};

#define blowdw_1w       {kVBlow_1w,      0,           kDTnull,         0,         0, kDTnull,         kDTokblow_1w,     kALblowdw_1w}
#define breakpkin_1w    {kVBreak_1w,     0,           kDTnull,         0,         1, kDTnull,         kDTokgen_1w,      kALpkin_1w}
#define brkrope_1w      {kVBreak_1w,     0,           kDTnull,         DONT_CARE, 0, kDTnull,         kDTsrbreak_1w,    0}
#define closebolt_1w    {kVClose_1w,     0,           kDTnull,         2,         1, kDTsclosed_1w,   kDTsclosebolt_1w, 0}
#define closedoor1_1w   {kVClose_1w,     0,           kDTnull,         1,         0, kDTsclosed_1w,   kDTnull,          kALclosedoor1_1w}
#define closedoor2_1w   {kVClose_1w,     0,           kDTnull,         1,         0, kDTsclosed_1w,   kDTnull,          0}
#define closedoor3_1w   {kVClose_1w,     0,           kDTnull,         1,         0, kDTsclosed_1w,   kDTnull,          0}
#define closedoor4_1w   {kVClose_1w,     0,           kDTnull,         1,         0, kDTsclosed_1w,   kDTnull,          kALclosedoor4_1w}
#define closetrap_1w    {kVClose_1w,     0,           kDTnull,         1,         0, kDTsclosed_1w,   kDTnull,          kALclosetrap_1w}
#define closewdoors_1w  {kVClose_1w,     0,           kDTnull,         1,         0, kDTsclosed_1w,   kDTnull,          kALclosewdoors_1w}
#define cutrope_1w      {kVCut_1w,       kRknife_1w,  kDTrnoknife_1w,  0,         1, kDTsnocut_1w,    kDTscut_1w,       kALcutrope_1w}
#define dropmask_1w     {kVDrop_1w,      0,           kDTnull,         0,         0, kDTsworn3_1w,    kDTnull,          kALdropmask_1w}
#define droppkin_1w     {kVDrop_1w,      kRpkin_1w,   kDTnocgen_1w,    0,         1, kDTnull,         kDTnull,          kALpkin_1w}
#define eatchop_1w      {kVEat_1w,       0,           kDTnull,         0,         0, kDTnull,         kDTnull,          kALeatchop2_1w}
#define getchop_1w      {kVTake_1w,      0,           kDTnull,         0,         0, kDTnull,         kDTnull,          kALgetchop_1w}
#define getdw_1w        {kVTake_1w,      0,           kDTnull,         0,         0, kDTnull,         kDTnull,          kALcupbdw_1w}
#define getinboat_1w    {kVInto_1w,      0,           kDTnull,         DONT_CARE, 0, kDTnull,         kDTnull,          kALgetinboat_1w}
#define getknife_1w     {kVTake_1w,      0,           kDTnull,         0,         0, kDTnull,         kDTnull,          kALcupbpk_1w}
#define getoilcan_1w    {kVTake_1w,      0,           kDTnull,         0,         0, kDTnull,         kDTnull,          kALshedoil_1w}
#define getoutboat_1w   {kVOutof_1w,     0,           kDTnull,         DONT_CARE, 0, kDTnull,         kDTnull,          kALgetoutboat_1w}
#define givegold_1w     {kVGive_1w,      0,           kDTnull,         0,         0, kDTnull,         kDTnull,          kALgold_1w}
#define hiderock_1w     {kVHide_1w,      0,           kDTnull,         0,         1, kDTsrock_1w,     kDTnull,          kALrock_1w}
#define kickpkin_1w     {kVAttack_1w,    0,           kDTnull,         0,         1, kDTnull,         kDTokgen_1w,      kALpkin_1w}
#define knock_1w        {kVKnock_1w,     0,           kDTnull,         DONT_CARE, 0, kDTnull,         kDTsNobody_1w,    0}
#define lockbolt_1w     {kVLock_1w,      0,           kDTnull,         2,         1, kDTsclosed_1w,   kDTsclosebolt_1w, 0}
#define lockdoor_1w     {kVLock_1w,      kRkey_1w,    kDTnockey_1w,    1,         0, kDTslock_1w,     kDTokgen_1w,      kALclosedoor1_1w}
#define lookcupb_1w     {kVLook_1w,      kRcandle_1w, kDTnoccandle_1w, DONT_CARE, 0, kDTnull,         kDTnull,          kALlookcupb_1w}
#define lookshed_1w     {kVLook_1w,      kRcandle_1w, kDTnoccandle_1w, 0,         0, kDTsnoseeoil_1w, kDTsseeoil_1w,    kALshedoil_1w}
#define movecarp1_1w    {kVMove_1w,      0,           kDTnull,         0,         0, kDTnull,         kDTsrollrug_1w,   kALmovecarp_1w}
#define movecarp2_1w    {kVLift_1w,      0,           kDTnull,         0,         0, kDTnull,         kDTsrollrug_1w,   kALmovecarp_1w}
#define movecarp3_1w    {kVUnder_1w,     0,           kDTnull,         0,         0, kDTnull,         kDTsrollrug_1w,   kALmovecarp_1w}
#define offmask_1w      {kVOff_1w,       0,           kDTnull,         1,         0, kDTsworn2_1w,    kDTokgen_1w,      kALswapmask_1w}
#define oilbolt_1w      {kVOil_1w,       kRoil_1w,    kDTrnooil_1w,    0,         1, kDTsoiled_1w,    kDTsoilbolt_1w,   0}
#define omattack_1w     {kVAttack_1w,    0,           kDTnull,         0,         0, kDTnull,         kDTsomattack_1w,  0}
#define ombreak_1w      {kVBreak_1w,     0,           kDTnull,         0,         0, kDTnull,         kDTsomattack_1w,  0}
#define omtalk_1w       {kVTalk_1w,      0,           kDTnull,         DONT_CARE, 0, kDTnull,         kDTnull,          kALoldman_1w}
#define openbolt_1w     {kVOpen_1w,      0,           kDTnull,         1,         2, kDTsstuck_1w,    kDTsopenbolt_1w,  0}
#define opendoor1_1w    {kVOpen_1w,      kRkey_1w,    kDTslocked_1w,   0,         1, kDTsopen_1w,     kDTsunlock_1w,    kALopendoor1_1w}
#define opendoor2_1w    {kVOpen_1w,      0,           kDTnull,         0,         1, kDTsopen_1w,     kDTnull,          kALopendoor2_1w}
#define opendoor3_1w    {kVOpen_1w,      0,           kDTnull,         0,         1, kDTsopen_1w,     kDTnull,          kALopendoor3_1w}
#define opendoor4_1w    {kVOpen_1w,      0,           kDTnull,         0,         0, kDTsopen_1w,     kDTnull,          kALopendoor4_1w}
#define openpkin_1w     {kVOpen_1w,      0,           kDTnull,         0,         1, kDTnull,         kDTsopenpkin_1w,  kALpkin_1w}
#define opentrap_1w     {kVOpen_1w,      0,           kDTnull,         0,         0, kDTnull,         kDTnull,          kALopentrap_1w}
#define openwdoors_1w   {kVOpen_1w,      0,           kDTnull,         0,         1, kDTsopen_1w,     kDTnull,          kALopenwdoors_1w}
#define plugbung_1w     {kVPlug_1w,      kRbung_1w,   kDTnocgen_1w,    0,         1, kDTnull,         kDTsplug_1w,      kALplugbung_1w}
#define pushboat_1w     {kVPush_1w,      0,           kDTnull,         DONT_CARE, 0, kDTnull,         kDTnull,          kALpushboat_1w}
#define pushigor_1w     {kVPush_1w,      0,           kDTnull,         0,         0, kDTnull,         kDTnull,          kALigor_1w}
#define ruboilcan_1w    {kVRub_1w,       0,           kDTnull,         0,         0, kDTnull,         kDTsruboil_1w,    0}
#define talkdrac_1w     {kVTalk_1w,      0,           kDTnull,         0,         0, kDTnull,         kDTnull,          kALtalkdrac_1w}
#define talkfrank_1w    {kVTalk_1w,      0,           kDTnull,         0,         0, kDTnull,         kDTnull,          kALtalkfrank_1w}
#define talkgwen_1w     {kVTalk_1w,      0,           kDTnull,         0,         0, kDTnull,         kDTnull,          kALtalkgwen_1w}
#define talkhood_1w     {kVTalk_1w,      0,           kDTnull,         0,         0, kDTnull,         kDTnull,          kALtalkhood_1w}
#define talkpeahd_1w    {kVTalk_1w,      0,           kDTnull,         0,         0, kDTnull,         kDTnull,          kALtalkpeahd_1w}
#define talkslime_1w    {kVTalk_1w,      0,           kDTnull,         0,         0, kDTnull,         kDTnull,          kALtalkslime_1w}
#define throwchop_1w    {kVThrowit_1w,   0,           kDTnull,         0,         0, kDTnull,         kDTnull,          kALthrowchop_1w}
#define unlkdoor_1w     {kVUnlock_1w,    kRkey_1w,    kDTnockey_1w,    0,         1, kDTsunlocked_1w, kDTsunlock_1w,    kALopendoor1_1w}
#define unlock_1w       {kVUnlock_1w,    0,           kDTnull,         DONT_CARE, 0, kDTnull,         kDTsUnlocked_1w,  0}
#define unlockbolt_1w   {kVUnlock_1w,    0,           kDTnull,         1,         2, kDTsstuck_1w,    kDTsopenbolt_1w,  0}
#define untierope_1w    {kVUntie_1w,     0,           kDTnull,         DONT_CARE, 0, kDTnull,         kDTsuntie_1w,     0}
#define useboat_1w      {kVMakeUseOf_1w, 0,           kDTnull,         DONT_CARE, 0, kDTnull,         kDTnull,          kALuseboat_1w}
#define usemask_1w      {kVMakeUseOf_1w, kRmask_1w,   kDTnocgen_1w,    DONT_CARE, 0, kDTnull,         kDTnull,          kALusemask_1w}
#define wearmask_1w     {kVWear_1w,      kRmask_1w,   kDTnocgen_1w,    0,         1, kDTsworn1_1w,    kDTokgen_1w,      kALswapmask_1w}

#define emptyCmd     {0,           0,        kDTnull,         0,         0, kDTnull,         kDTnull,          0}

cmd cmdDummy[]    = {emptyCmd};

cmd boat_1w[]    = {useboat_1w,    getinboat_1w,   getoutboat_1w, pushboat_1w,  emptyCmd};
cmd bolt_1w[]    = {openbolt_1w,   oilbolt_1w,     unlockbolt_1w, closebolt_1w, lockbolt_1w, emptyCmd};
cmd bung_1w[]    = {plugbung_1w,   emptyCmd};
cmd carpet_1w[]  = {movecarp1_1w,  movecarp2_1w,   movecarp3_1w,  emptyCmd};
cmd chop_1w[]    = {eatchop_1w,    throwchop_1w,   getchop_1w,    emptyCmd};
cmd cupb_1w[]    = {lookcupb_1w,   emptyCmd};
cmd door1_1w[]   = {opendoor1_1w,  closedoor1_1w,  unlkdoor_1w,   lockdoor_1w,  knock_1w,    emptyCmd};
cmd door2_1w[]   = {opendoor2_1w,  closedoor2_1w,  knock_1w,      unlock_1w,    emptyCmd};
cmd door3_1w[]   = {opendoor3_1w,  closedoor3_1w,  knock_1w,      unlock_1w,    emptyCmd};
cmd door4_1w[]   = {opendoor4_1w,  closedoor4_1w,  knock_1w,      emptyCmd};
cmd drac_1w[]    = {talkdrac_1w,   emptyCmd};
cmd frank_1w[]   = {talkfrank_1w,  emptyCmd};
cmd gold_1w[]    = {givegold_1w,   emptyCmd};
cmd gwen_1w[]    = {talkgwen_1w,   emptyCmd};
cmd hood_1w[]    = {talkhood_1w,   emptyCmd};
cmd igor_1w[]    = {pushigor_1w,   emptyCmd};
cmd knife_1w[]   = {getknife_1w,   emptyCmd};
cmd mask_1w[]    = {usemask_1w,    wearmask_1w,    offmask_1w,    dropmask_1w,  emptyCmd};
cmd oilcan_1w[]  = {getoilcan_1w,  ruboilcan_1w,   emptyCmd};
cmd oldman_1w[]  = {omtalk_1w,     omattack_1w,    ombreak_1w,    emptyCmd};
cmd peahd_1w[]   = {talkpeahd_1w,  emptyCmd};
cmd pkin_1w[]    = {openpkin_1w,   kickpkin_1w,    breakpkin_1w,  droppkin_1w,  emptyCmd};
cmd rock_1w[]    = {hiderock_1w,   emptyCmd};
cmd rope_1w[]    = {cutrope_1w,    untierope_1w,   brkrope_1w,    emptyCmd};
cmd shed_1w[]    = {lookshed_1w,   emptyCmd};
cmd slime_1w[]   = {talkslime_1w,  emptyCmd};
cmd trap_1w[]    = {opentrap_1w,   closetrap_1w,   knock_1w,      emptyCmd};
cmd ward_1w[]    = {openwdoors_1w, closewdoors_1w, knock_1w,      emptyCmd};
cmd whistle_1w[] = {blowdw_1w,     getdw_1w,       emptyCmd};

const cmd *cmdList_1w[] = {
	cmdDummy,  boat_1w,  bolt_1w,  bung_1w,  carpet_1w,
	chop_1w,   cupb_1w,  door1_1w, door2_1w, door3_1w,
	door4_1w,  drac_1w,  frank_1w, gold_1w,  gwen_1w,
	hood_1w,   igor_1w,  knife_1w, mask_1w,  oilcan_1w,
	oldman_1w, peahd_1w, pkin_1w,  rock_1w,  rope_1w,
	shed_1w,   slime_1w, trap_1w,  ward_1w,  whistle_1w
};

#define climbdumb_2w      {kVClimb_2w,     0,           kDTnull,       0,         0, kDTnull,         kDTnull,           kALdumb_2w}
#define climbrope_2w      {kVClimb_2w,     0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTnull,           kALclimbrope_2w}
#define climbwell_2w      {kVClimb_2w,     0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTnull,           kALclimbwell_2w}
#define closedoor1_2w     {kVClose_2w,     0,           kDTnull,       1,         0, kDTsclose_2w,    kDTnull,           0}
#define closedoor2_2w     {kVClose_2w,     0,           kDTnull,       1,         0, kDTsclose_2w,    kDTnull,           0}
#define closedoor3_2w     {kVClose_2w,     0,           kDTnull,       1,         0, kDTsclose_2w,    kDTnull,           0}
#define closesafe_2w      {kVClose_2w,     0,           kDTnull,       1,         0, kDTsclose_2w,    kDTokgen_2w,       0}
#define dialphone_2w      {kVDial_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTnull,           kALphone_2w}
#define doorpencil_2w     {kVMakeUseOf_2w, kRpencil_2w, kDTnocgen_2w,  0,         1, kDTspencil_2w,   kDTnull,           kALchkpap1_2w}
#define dropdynamite_2w   {kVDrop_2w,      kRdyn_2w,    kDTnocgen_2w,  DONT_CARE, 0, kDTnull,         kDTnull,           kALdropdynamite_2w}
#define eatbanana_2w      {kVEat_2w,       kRbanana_2w, kDTnocgen_2w,  0,         0, kDTnull,         kDTnull,           kALeatbanana_2w}
#define eatcatnip_2w      {kVEat_2w,       kRcatnip_2w, kDTnocgen_2w,  0,         0, kDTnopurps_2w,   kDTseatnip_2w,     0}
#define eatgarlic_2w      {kVEat_2w,       kRgarlic_2w, kDTnocgen_2w,  DONT_CARE, 0, kDTnull,         kDTnull,           kALgarlic_2w}
#define firegun_2w        {kVFire_2w,      kRgun_2w,    kDTnogun_2w,   0,         1, kDTsempty_2w,    kDTnull,           kALgun_2w}
#define gard1_2w          {kVTalk_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTnull,           kALtalkgard_2w}
#define gard2_2w          {kVLook_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTnull,           kALlookgard_2w}
#define getballoon_2w     {kVTake_2w,      0,           kDTnull,       0,         0, kDTnull,         kDTnull,           kALballoon_2w}
#define getbook_2w        {kVTake_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTnull,           kALgetbook_2w}
#define getdynamite_2w    {kVTake_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTnull,           kALgetdynamite_2w}
#define getletter_2w      {kVTake_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTsgetlet_2w,     0}
#define givebanana_2w     {kVGive_2w,      kRbanana_2w, kDTnocgen_2w,  0,         0, kDTnull,         kDTnull,           kALbanana_2w}
#define givebell_2w       {kVGive_2w,      kRbell_2w,   kDTnocgen_2w,  DONT_CARE, 0, kDTnull,         kDTnull,           kALgivebel_2w}
#define givecatnip_2w     {kVGive_2w,      kRcatnip_2w, kDTnocgen_2w,  0,         0, kDTnopurps_2w,   kDTscatnip_2w,     0}
#define intodumb_2w       {kVInto_2w,      0,           kDTnull,       0,         0, kDTnull,         kDTnull,           kALdumb_2w}
#define knock_2w          {kVKnock_2w,     0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTsNobody_2w,     0}
#define lightdynamite_2w  {kVStrike_2w,    kRmatch_2w,  kDTnomatch_2w, DONT_CARE, 0, kDTnull,         kDTnull,           kALlightdynamite_2w}
#define lookcubp_2w       {kVLook_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTnull,           kALphoto_2w}
#define lookgarlic_2w     {kVLook_2w,      0,           kDTnull,       0,         1, kDTempty_2w,     kDTsFindClove_2w,  kALgetgarlic_2w}
#define lookhole_2w       {kVLook_2w,      0,           kDTnull,       0,         0, kDTsDarkHole_2w, kDTnull,           kALkeyhole_2w}
#define lookkennel_2w     {kVLook_2w,      0,           kDTnull,       0,         0, kDTnull,         kDTnull,           kALlookkennel_2w}
#define lookmat_2w        {kVLook_2w,      0,           kDTnull,       0,         1, kDTempty_2w,     kDTsFindMatch_2w,  kALgetmatch_2w}
#define opencubp_2w       {kVOpen_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTnull,           kALphoto_2w}
#define opendoor1_2w      {kVOpen_2w,      0,           kDTnull,       0,         1, kDTsopen1_2w,    kDTnull,           kALopendoor1_2w}
#define opendoor2_2w      {kVOpen_2w,      0,           kDTnull,       0,         1, kDTsopen1_2w,    kDTnull,           kALopendoor2_2w}
#define opendoor3_2w      {kVOpen_2w,      0,           kDTnull,       0,         1, kDTsopen1_2w,    kDTnull,           kALopendoor3_2w}
#define opendum_2w        {kVOpen_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTsopendum_2w,    0}
#define opengarlic_2w     {kVOpen_2w,      0,           kDTnull,       0,         1, kDTempty_2w,     kDTsFindClove_2w,  kALgetgarlic_2w}
#define openkdoor_2w      {kVOpen_2w,      0,           kDTnull,       0,         0, kDTnull,         kDTwontopen_2w,    0}
#define openlamp_2w       {kVOpen_2w,      kRlamp_2w,   kDTnocgen_2w,  DONT_CARE, 0, kDTnull,         kDTempty_2w,       0}
#define openmat_2w        {kVOpen_2w,      0,           kDTnull,       0,         1, kDTempty_2w,     kDTsFindMatch_2w,  kALgetmatch_2w}
#define openpdoor_2w      {kVOpen_2w,      0,           kDTnull,       0,         0, kDTnull,         kDTspdoor_2w,      0}
#define opensafe_2w       {kVOpen_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTnull,           kALsafe_2w}
#define popballoon_2w     {kVBreak_2w,     0,           kDTnull,       0,         0, kDTnull,         kDTnull,           kALballoon_2w}
#define pushblue_2w       {kVPush_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTnull,           kALworkgates_2w}
#define pushbutton_2w     {kVPush_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTsWhichColor_2w, 0}
#define pushgreen_2w      {kVPush_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTnull,           kALbugzapper_2w}
#define pushpaper_2w      {kVPush_2w,      kRpaper_2w,  kDTnocgen_2w,  0,         1, kDTsnopaper_2w,  kDTspaper_2w,      kALpushpaper_2w}
#define pushpencil_2w     {kVPush_2w,      kRpencil_2w, kDTnocgen_2w,  0,         0, kDTspencil_2w,   kDTnull,           kALpushpencil_2w}
#define pushred_2w        {kVPush_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTnull,           kALshedlight_2w}
#define pushyellow_2w     {kVPush_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTnull,           kALgatelight_2w}
#define readalbum_2w      {kVRead_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTtalbum_2w,      0}
#define readletter_2w     {kVRead_2w,      0,           kDTnull,       3,         3, kDTsnoread_2w,   kDTsread_2w,       kALreadlet_2w}
#define readwill_2w       {kVRead_2w,      kRwill_2w,   kDTnocgen_2w,  1,         1, kDTnull,         kDTnull,           kALwill_2w}
#define ringbell_2w       {kVRing_2w,      kRbell_2w,   kDTnocgen_2w,  DONT_CARE, 0, kDTnull,         kDTnull,           kALbell_2w}
#define rubcatnip_2w      {kVRub_2w,       kRcatnip_2w, kDTnocgen_2w,  0,         0, kDTnopurps_2w,   kDTnull,           kALcatnip_2w}
#define rublamp_2w        {kVRub_2w,       kRlamp_2w,   kDTnocgen_2w,  0,         0, kDTnopurps_2w,   kDTnull,           kALlamp_2w}
#define serum_2w          {kVDrink_2w,     kRserum_2w,  kDTnocgen_2w,  0,         1, kDTsnoserum_2w,  kDTnull,           kALbottle_2w}
#define strikematch_2w    {kVStrike_2w,    kRmatch_2w,  kDTnocgen_2w,  DONT_CARE, 0, kDTnull,         kDTnull,           kALstrikematch_2w}
#define takepaper_2w      {kVTake_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTnull,           kALtakepaper_2w}
#define takephone_2w      {kVTake_2w,      0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTnull,           kALphone_2w}
#define talkharry_2w      {kVTalk_2w,      0,           kDTnull,       0,         1, kDTsharry_2w,    kDTnull,           kALharry_2w}
#define throwstick_2w     {kVThrowit_2w,   kRstick_2w,  kDTnocgen_2w,  0,         1, kDTnull,         kDTnull,           kALthrowstick_2w}
#define unlock_2w         {kVUnlock_2w,    0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTsUnlocked_2w,   0}
#define unlockdum_2w      {kVUnlock_2w,    0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTsunlockdum_2w,  0}
#define usedynamite_2w    {kVMakeUseOf_2w, kRmatch_2w,  kDTnomatch_2w, DONT_CARE, 0, kDTnull,         kDTnull,           kALlightdynamite_2w}
#define userobot_2w       {kVMakeUseOf_2w, 0,           kDTnull,       DONT_CARE, 0, kDTnull,         kDTnull,           0}

cmd album_2w[]    = {readalbum_2w,   emptyCmd};
cmd balloon_2w[]  = {popballoon_2w,  getballoon_2w,   emptyCmd};
cmd banana_2w[]   = {givebanana_2w,  eatbanana_2w,    emptyCmd};
cmd bell_2w[]     = {ringbell_2w,    givebell_2w,     emptyCmd};
cmd blue_2w[]     = {pushblue_2w,    emptyCmd};
cmd book_2w[]     = {getbook_2w,     emptyCmd};
cmd bottle_2w[]   = {serum_2w,       emptyCmd};
cmd button_2w[]   = {pushbutton_2w,  emptyCmd};
cmd catnip_2w[]   = {rubcatnip_2w,   givecatnip_2w,   eatcatnip_2w,     emptyCmd};
cmd cupbp_2w[]    = {opencubp_2w,    lookcubp_2w,     emptyCmd};
cmd door1_2w[]    = {opendoor1_2w,   closedoor1_2w,   knock_2w,         unlock_2w, emptyCmd};
cmd door2_2w[]    = {opendoor2_2w,   closedoor2_2w,   knock_2w,         unlock_2w, emptyCmd};
cmd door3_2w[]    = {opendoor3_2w,   closedoor3_2w,   knock_2w,         unlock_2w, emptyCmd};
cmd doordum_2w[]  = {opendum_2w,     unlockdum_2w,    knock_2w,         emptyCmd};
cmd dumb_2w[]     = {intodumb_2w,    climbdumb_2w,    emptyCmd};
cmd dynamite_2w[] = {getdynamite_2w, dropdynamite_2w, lightdynamite_2w, emptyCmd};
cmd garlic_2w[]   = {eatgarlic_2w,   emptyCmd};
cmd green_2w[]    = {pushgreen_2w,   emptyCmd};
cmd gun_2w[]      = {firegun_2w,     emptyCmd};
cmd harry_2w[]    = {talkharry_2w,   emptyCmd};
cmd kdoor_2w[]    = {openkdoor_2w,   emptyCmd};
cmd kennel_2w[]   = {lookkennel_2w,  emptyCmd};
cmd keyhole_2w[]  = {lookhole_2w,    emptyCmd};
cmd lamp_2w[]     = {rublamp_2w,     openlamp_2w,     emptyCmd};
cmd letter_2w[]   = {readletter_2w,  getletter_2w,    emptyCmd};
cmd lookcupb_2w[] = {opengarlic_2w,  lookgarlic_2w,   emptyCmd};
cmd lookdesk_2w[] = {openmat_2w,     lookmat_2w,      emptyCmd};
cmd lookgard_2w[] = {gard1_2w,       gard2_2w,        emptyCmd};
cmd matches_2w[]  = {strikematch_2w, usedynamite_2w,  emptyCmd};
cmd paper_2w[]    = {pushpaper_2w,   takepaper_2w,    emptyCmd};
cmd pdoor_2w[]    = {openpdoor_2w,   emptyCmd};
cmd pencil_2w[]   = {doorpencil_2w,  pushpencil_2w,   emptyCmd};
cmd red_2w[]      = {pushred_2w,     emptyCmd};
cmd robot_2w[]    = {userobot_2w,    firegun_2w,      emptyCmd};
cmd rope_2w[]     = {climbrope_2w,   emptyCmd};
cmd safe_2w[]     = {opensafe_2w,    closesafe_2w,    emptyCmd};
cmd stick_2w[]    = {throwstick_2w,  emptyCmd};
cmd tardis_2w[]   = {dialphone_2w,   takephone_2w,    emptyCmd};
cmd well_2w[]     = {climbwell_2w,   emptyCmd};
cmd will_2w[]     = {readwill_2w,    emptyCmd};
cmd yellow_2w[]   = {pushyellow_2w,  emptyCmd};

const cmd *cmdList_2w[] = {
	cmdDummy,  album_2w,    balloon_2w,  banana_2w,   bell_2w,
	blue_2w,   book_2w,     bottle_2w,   button_2w,   catnip_2w,
	cupbp_2w,  door1_2w,    door2_2w,    door3_2w,    doordum_2w,
	dumb_2w,   dynamite_2w, garlic_2w,   green_2w,    gun_2w,
	harry_2w,  kdoor_2w,    kennel_2w,   keyhole_2w,  lamp_2w,
	letter_2w, lookcupb_2w, lookdesk_2w, lookgard_2w, matches_2w,
	paper_2w,  pdoor_2w,    pencil_2w,   red_2w,      robot_2w,
	rope_2w,   safe_2w,     stick_2w,    tardis_2w,   well_2w,
	will_2w,   yellow_2w
};

#define bell_3w          {kVRing_3w,      kRbell_3w,    kDTnocgen_3w, DONT_CARE, 0, kDTnull,        kDTokbell_3w,    0}
#define blow_3w          {kVBlow_3w,      kRpipe_3w,    kDTnogun_3w,  DONT_CARE, 0, kDTnull,        kDTnull,         kALdart_3w}
#define blowdoc_3w       {kVShoot_3w,     kRpipe_3w,    kDTnogun_3w,  DONT_CARE, 0, kDTnull,        kDTsblowdoc_3w,  0}
#define book_3w          {kVRead_3w,      kRbook_3w,    kDTnocgen_3w, DONT_CARE, 0, kDTnull,        kDTnull,         kALreadbook_3w}
#define cage1_3w         {kVOpen_3w,      0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTnull,         kALopencage_3w}
#define cage2_3w         {kVClose_3w,     0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTokgen_3w,     0}
#define cage3_3w         {kVTake_3w,      0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTnull,         kALtakecage_3w}
#define cageuse_3w       {kVMakeUseOf_3w, 0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTnull,         kALusecage_3w}
#define candle1_3w       {kVStrike_3w,    0,            kDTnull,      0,         1, kDTslit_3w,     kDTokgen_3w,     0}
#define candle2_3w       {kVDouse_3w,     0,            kDTnull,      1,         0, kDTsunlit_3w,   kDTokgen_3w,     0}
#define cdoor1_3w        {kVOpen_3w,      0,            kDTnull,      0,         0, kDTsopen1_3w,   kDTnull,         kALopendoor_3w}
#define cdoor2_3w        {kVClose_3w,     0,            kDTnull,      1,         0, kDTsclose_3w,   kDTokgen_3w,     kALclosedoor_3w}
#define cdrinkpool_3w    {kVMakeUseOf_3w, 0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTspool_3w,     0}
#define cdrinkstream_3w  {kVMakeUseOf_3w, 0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTsstream_3w,   0}
#define cexit1_3w        {kVOutof_3w,     0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTokgen_3w,     kALexit_3w}
#define cexit2_3w        {kVClimb_3w,     0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTokgen_3w,     kALexit_3w}
#define cflask1_3w       {kVFill_3w,      0,            kDTnull,      0,         1, kDTsfull_3w,    kDTnull,         kALfill_3w}
#define cflask2_3w       {kVPut_3w,       0,            kDTnull,      0,         1, kDTsfull_3w,    kDTnull,         kALfill_3w}
#define cflask3_3w       {kVEmpty_3w,     0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTnull,         kALempty2_3w}
#define cflask4_3w       {kVDrink_3w,     0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTnull,         kALdrink_3w}
#define cflask5_3w       {kVGive_3w,      0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTnull,         kALflask_3w}
#define cheese1_3w       {kVEat_3w,       kRcheese_3w,  kDTnocgen_3w, 0,         0, kDTnull,        kDTnull,         kALeatcheese_3w}
#define cheese2_3w       {kVDrop_3w,      0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTnull,         kALdropcheese_3w}
#define cheese3_3w       {kVPut_3w,       0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTnull,         kALdropcheese_3w}
#define cheese4_3w       {kVTake_3w,      0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTnull,         kALtakecheese_3w}
#define cmake1_3w        {kVMake_3w,      0,            kDTnull,      0,         0, kDTsmade_3w,    kDTnull,         kALmakeclay_3w}
#define cmake2_3w        {kVStick_3w,     kRpins_3w,    kDTnopins_3w, DONT_CARE, 0, kDTnull,        kDTnull,         kALstick_3w}
#define cplane1_3w       {kVClimb_3w,     0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTokgen_3w,     kALplane_3w}
#define cplane2_3w       {kVInto_3w,      0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTokgen_3w,     kALplane_3w}
#define cplane3_3w       {kVSearch_3w,    0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTokgen_3w,     kALplane_3w}
#define crystal_3w       {kVRub_3w,       kRcrystal_3w, kDTnocgen_3w, DONT_CARE, 0, kDTnull,        kDTnull,         kALcrystal_3w}
#define csteps1_3w       {kVMakeUseOf_3w, 0,            kDTnull,      0,         0, kDTnull,        kDTnull,         kALsteps_3w}
#define cstick1_3w       {kVStick_3w,     0,            kDTnull,      0,         0, kDTnull,        kDTnull,         kALstick_3w}
#define cswing1_3w       {kVSwing_3w,     0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTswinger_3w,   kALswing_3w}
#define ctalknat_3w      {kVTalk_3w,      0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTnull,         kALtalknat_3w}
#define cube1_3w         {kVGive_3w,      0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTnull,         kALgiveb_3w}
#define cvine1_3w        {kVMakeUseOf_3w, 0,            kDTnull,      0,         0, kDTnull,        kDTnull,         kALvine_3w}
#define cvine2_3w        {kVUntie_3w,     0,            kDTnull,      0,         0, kDTnull,        kDTnull,         kALuntie_vine_3w}
#define cvine3_3w        {kVTie_3w,       0,            kDTnull,      0,         0, kDTnull,        kDTnull,         kALvine_3w}
#define cwaterfall_3w    {kVLook_3w,      0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTnull,         kALlookwfall_3w}
#define cwaterpool_3w    {kVLook_3w,      0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTswater_3w,    0}
#define cwaterstream_3w  {kVLook_3w,      0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTswater_3w,    0}
#define dart_3w          {kVShoot_3w,     kRpipe_3w,    kDTnogun_3w,  DONT_CARE, 0, kDTnull,        kDTnull,         kALdart_3w}
#define elephant_3w      {kVMakeUseOf_3w, 0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTuelephant_3w, 0}
#define ghost1_3w        {kVMakeUseOf_3w, 0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTughost_3w,    0}
#define ghost2_3w        {kVExorcise_3w,  kRexor_3w,    kDTnocex_3w,  DONT_CARE, 0, kDTnull,        kDTnull,         kALexorcise_3w}
#define knock_3w         {kVKnock_3w,     0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTsNobody_3w,   0}
#define lookrush_3w      {kVBehind_3w,    0,            kDTnull,      0,         1, kDTsfoundb_3w,  kDTnull,         kALfindbook_3w}
#define readit_3w        {kVRead_3w,      0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTsread_3w,     0}
#define rock1_3w         {kVBehind_3w,    0,            kDTnull,      0,         1, kDTsfoundc_3w,  kDTsfindc_3w,    kALfindcrystal_3w}
#define swingc_3w        {kVSwing_3w,     0,            kDTnull,      0,         1, kDTsnoswing_3w, kDTswingcave_3w, 0}
#define unlock_3w        {kVUnlock_3w,    0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTsUnlocked_3w, 0}
#define usedoc_3w        {kVMakeUseOf_3w, 0,            kDTnull,      DONT_CARE, 0, kDTnull,        kDTnull,         kALtalkdoc_3w}

cmd cbell_3w[]     = {bell_3w,         emptyCmd};
cmd cbook_3w[]     = {book_3w,         emptyCmd};
cmd cbouillon_3w[] = {cube1_3w,        emptyCmd};
cmd ccage_3w[]     = {cageuse_3w,      cage1_3w,        cage2_3w,   cage3_3w,   emptyCmd};
cmd ccandle_3w[]   = {candle1_3w,      candle2_3w,      emptyCmd};
cmd ccheese_3w[]   = {cheese1_3w,      cheese2_3w,      cheese3_3w, cheese4_3w, emptyCmd};
cmd cclay_3w[]     = {cmake1_3w,       cmake2_3w,       emptyCmd};
cmd ccrystal_3w[]  = {crystal_3w,      emptyCmd};
cmd cdart_3w[]     = {dart_3w,         blow_3w,         emptyCmd};
cmd cdoctor_3w[]   = {usedoc_3w,       blowdoc_3w,      emptyCmd};
cmd cdoor_3w[]     = {cdoor1_3w,       cdoor2_3w,       emptyCmd};
cmd celephant_3w[] = {elephant_3w,     dart_3w,         blow_3w,    emptyCmd};
cmd cexit_3w[]     = {cexit1_3w,       cexit2_3w,       emptyCmd};
cmd cflask_3w[]    = {cflask1_3w,      cflask2_3w,      cflask3_3w, cflask4_3w, cflask5_3w, emptyCmd};
cmd cghost_3w[]    = {ghost1_3w,       ghost2_3w,       emptyCmd};
cmd cnative_3w[]   = {ctalknat_3w,     emptyCmd};
cmd cpins_3w[]     = {cstick1_3w,      emptyCmd};
cmd cplane_3w[]    = {cplane1_3w,      cplane2_3w,      cplane3_3w, emptyCmd};
cmd crock_3w[]     = {rock1_3w,        emptyCmd};
cmd crush_3w[]     = {lookrush_3w,     emptyCmd};
cmd cscroll_3w[]   = {readit_3w,       emptyCmd};
cmd csteps_3w[]    = {csteps1_3w,      emptyCmd};
cmd cswing_3w[]    = {cswing1_3w,      emptyCmd};
cmd cswingc_3w[]   = {swingc_3w,       emptyCmd};
cmd cvine_3w[]     = {cvine1_3w,       cvine2_3w,       cvine3_3w,  emptyCmd};
cmd cwfall_3w[]    = {cwaterfall_3w,   emptyCmd};
cmd cwpool_3w[]    = {cdrinkpool_3w,   cwaterpool_3w,   emptyCmd};
cmd cwstream_3w[]  = {cdrinkstream_3w, cwaterstream_3w, emptyCmd};

const cmd *cmdList_3w[] = {
	cmdDummy,   cbell_3w,   cbook_3w,     cbouillon_3w, ccage_3w,
	ccandle_3w, ccheese_3w, cclay_3w,     ccrystal_3w,  cdart_3w,
	cdoctor_3w, cdoor_3w,   celephant_3w, cexit_3w,     cflask_3w,
	cghost_3w,  cnative_3w, cpins_3w,     cplane_3w,    crock_3w,
	crush_3w,   cscroll_3w, csteps_3w,    cswing_3w,    cswingc_3w,
	cvine_3w,   cwfall_3w,  cwpool_3w,    cwstream_3w
};

#define blowdw_1d        {kVBlow_1d,   0,           0,               0,         0, 0,             kDTokgen_1d,     kALblowdw_1d}
#define breakpkin_1d     {kVBreak_1d,  0,           0,               0,         1, 0,             kDTokgen_1d,     kALpkin_1d}
#define brkrope_1d       {kVBreak_1d,  0,           0,               DONT_CARE, 0, 0,             kDTsrbreak_1d,   0}
#define closebolt_1d     {kVClose_1d,  0,           0,               2,         1, kDTsclose_1d,  kDTokgen_1d,     0}
#define closedoor1_1d    {kVClose_1d,  0,           0,               2,         0, kDTsclose_1d,  0,               kALclosedoor1_1d}
#define closedoor2_1d    {kVClose_1d,  0,           0,               1,         0, kDTsclose_1d,  0,               0}
#define closedoor3_1d    {kVClose_1d,  0,           0,               1,         0, kDTsclose_1d,  0,               0}
#define closedoor4_1d    {kVClose_1d,  0,           0,               1,         0, kDTsclose_1d,  0,               kALclosedoor4_1d}
#define closetrap_1d     {kVClose_1d,  0,           0,               1,         0, kDTsclose_1d,  kDTokgen_1d,     kALclosetrap_1d}
#define closewdoors_1d   {kVClose_1d,  0,           0,               1,         0, kDTsclose_1d,  0,               kALclosewdoors_1d}
#define cutrope_1d       {kVCut_1d,    kRknife_1d,  kDTrnoknife_1d,  0,         1, kDTsnocut_1d,  kDTscut_1d,      kALcutrope_1d}
#define dropmask_1d      {kVDrop_1d,   0,           0,               0,         0, kDTsworn3_1d,  0,               kALdropmask_1d}
#define droppkin_1d      {kVDrop_1d,   kRpkin_1d,   kDTnocgen_1d,    0,         1, 0,             0,               kALpkin_1d}
#define eatchop_1d       {kVEat_1d,    0,           0,               0,         0, 0,             0,               kALeatchop_1d}
#define getdw_1d         {kVTake_1d,   0,           0,               0,         0, 0,             0,               kALcupbdw_1d}
#define getinboat_1d     {kVInto_1d,   0,           0,               DONT_CARE, 0, 0,             0,               kALgetinboat_1d}
#define getknife_1d      {kVTake_1d,   0,           0,               0,         0, 0,             0,               kALcupbpk_1d}
#define getoilcan_1d     {kVTake_1d,   0,           0,               0,         0, 0,             0,               kALshedoil_1d}
#define getoutboat_1d    {kVOutof_1d,  0,           0,               DONT_CARE, 0, 0,             0,               kALgetoutboat_1d}
#define givegold_1d      {kVGive_1d,   0,           0,               0,         0, 0,             0,               kALgold_1d}
#define kickpkin_1d      {kVAttack_1d, 0,           0,               0,         1, 0,             kDTokgen_1d,     kALpkin_1d}
#define knock_1d         {kVKnock_1d,  0,           0,               DONT_CARE, 0, 0,             kDTsknock_1d,    0}
#define lockbolt_1d      {kVLock_1d,   0,           0,               2,         1, kDTsclose_1d,  kDTokgen_1d,     0}
#define lockdoor_1d      {kVLock_1d,   kRkey_1d,    kDTnockey_1d,    2,         0, kDTslock_1d,   kDTokgen_1d,     0}
#define lookcupb_1d      {kVLook_1d,   kRcandle_1d, kDTnoccandle_1d, DONT_CARE, 0, 0,             0,               kALlookcupb_1d}
#define lookshed_1d      {kVLook_1d,   kRcandle_1d, kDTnoccandle_1d, 0,         0, kDTsnosee_1d,  kDTsseeoil_1d,   0}
#define movecarp1_1d     {kVMove_1d,   0,           0,               0,         0, 0,             kDTokgen_1d,     kALmovecarp_1d}
#define movecarp2_1d     {kVLift_1d,   0,           0,               0,         0, 0,             kDTokgen_1d,     kALmovecarp_1d}
#define movecarp3_1d     {kVUnder_1d,  0,           0,               0,         0, 0,             kDTokgen_1d,     kALmovecarp_1d}
#define offmask_1d       {kVOff_1d,    0,           0,               1,         0, kDTsworn2_1d,  kDTokgen_1d,     kALswapmask_1d}
#define oilbolt_1d       {kVOil_1d,    kRoil_1d,    kDTrnooil_1d,    0,         1, kDTsoiled_1d,  kDTokgen_1d,     0}
#define omattack_1d      {kVAttack_1d, 0,           0,               0,         0, 0,             kDTsomattack_1d, 0}
#define ombreak_1d       {kVBreak_1d,  0,           0,               0,         0, 0,             kDTsomattack_1d, 0}
#define omtalk_1d        {kVTalk_1d,   0,           0,               DONT_CARE, 0, 0,             0,               kALoldman_1d}
#define openbolt_1d      {kVOpen_1d,   0,           0,               1,         2, kDTsstuck_1d,  kDTokgen_1d,     0}
#define opendoor1_1d     {kVOpen_1d,   0,           0,               1,         2, kDTsopen_1d,   0,               kALopendoor1_1d}
#define opendoor2_1d     {kVOpen_1d,   0,           0,               0,         1, kDTsopen2_1d,  0,               kALopendoor2_1d}
#define opendoor3_1d     {kVOpen_1d,   0,           0,               0,         1, kDTsopen2_1d,  0,               kALopendoor3_1d}
#define opendoor4_1d     {kVOpen_1d,   0,           0,               0,         0, kDTsopen2_1d,  0,               kALopendoor4_1d}
#define openpkin_1d      {kVOpen_1d,   0,           0,               0,         1, 0,             kDTokgen_1d,     kALpkin_1d}
#define opentrap_1d      {kVOpen_1d,   0,           0,               0,         0, 0,             0,               kALopentrap_1d}
#define openwdoors_1d    {kVOpen_1d,   0,           0,               0,         1, kDTsopen2_1d,  0,               kALopenwdoors_1d}
#define plugbung_1d      {kVPlug_1d,   kRbung_1d,   kDTnocgen_1d,    0,         1, 0,             kDTokgen_1d,     kALplugbung_1d}
#define pushboat_1d      {kVPush_1d,   0,           0,               DONT_CARE, 0, 0,             0,               kALpushboat_1d}
#define pushigor_1d      {kVPush_1d,   0,           0,               0,         0, 0,             0,               kALigor_1d}
#define ruboilcan_1d     {kVRub_1d,    0,           0,               0,         0, 0,             kDTsruboil_1d,   0}
#define throwchop_1d     {kVThrow_1d,  0,           0,               0,         0, 0,             0,               kALthrowchop_1d}
#define unlkdoor_1d      {kVUnlock_1d, kRkey_1d,    kDTnockey_1d,    0,         1, kDTsunlock_1d, kDTokgen_1d,     0}
#define unlock_1d        {kVUnlock_1d, 0,           0,               DONT_CARE, 0, 0,             kDTsunlock_1d,   0}
#define unlockbolt_1d    {kVUnlock_1d, 0,           0,               1,         2, kDTsstuck_1d,  kDTokgen_1d,     0}
#define untierope_1d     {kVUntie_1d,  0,           0,               DONT_CARE, 0, 0,             kDTsuntie_1d,    0}
#define wearmask_1d      {kVWear_1d,   kRmask_1d,   kDTnocgen_1d,    0,         1, kDTsworn1_1d,  kDTokgen_1d,     kALswapmask_1d}

cmd boat_1d[] =    {getinboat_1d,  getoutboat_1d,  pushboat_1d,   emptyCmd};
cmd bolt_1d[] =    {oilbolt_1d,    openbolt_1d,    unlockbolt_1d, closebolt_1d,  lockbolt_1d, emptyCmd};
cmd bung_1d[] =    {plugbung_1d,   emptyCmd};
cmd carpet_1d[] =  {movecarp1_1d,  movecarp2_1d,   movecarp3_1d,  emptyCmd};
cmd chop_1d[] =    {eatchop_1d,    throwchop_1d,   emptyCmd};
cmd cupb_1d[] =    {lookcupb_1d,   emptyCmd};
cmd door1_1d[] =   {opendoor1_1d,  closedoor1_1d,  unlkdoor_1d,   lockdoor_1d,   knock_1d,    emptyCmd};
cmd door2_1d[] =   {opendoor2_1d,  closedoor2_1d,  knock_1d,      unlock_1d,     emptyCmd};
cmd door3_1d[] =   {opendoor3_1d,  closedoor3_1d,  knock_1d,      unlock_1d,     emptyCmd};
cmd door4_1d[] =   {opendoor4_1d,  closedoor4_1d,  knock_1d,      emptyCmd};
cmd gold_1d[] =    {givegold_1d,   emptyCmd};
cmd igor_1d[] =    {pushigor_1d,   emptyCmd};
cmd knife_1d[] =   {getknife_1d,   emptyCmd};
cmd mask_1d[] =    {wearmask_1d,   offmask_1d,     dropmask_1d,   emptyCmd};
cmd oilcan_1d[] =  {getoilcan_1d,  ruboilcan_1d,   emptyCmd};
cmd oldman_1d[] =  {omattack_1d,   ombreak_1d,     omtalk_1d,     emptyCmd};
cmd pkin_1d[] =    {kickpkin_1d,   breakpkin_1d,   openpkin_1d,   droppkin_1d,   emptyCmd};
cmd rope_1d[] =    {cutrope_1d,    untierope_1d,   brkrope_1d,    emptyCmd};
cmd shed_1d[] =    {lookshed_1d,   emptyCmd};
cmd trap_1d[] =    {opentrap_1d,   closetrap_1d,   knock_1d,      emptyCmd};
cmd ward_1d[] =    {openwdoors_1d, closewdoors_1d, knock_1d,      emptyCmd};
cmd whistle_1d[] = {getdw_1d,      blowdw_1d,      emptyCmd};

const cmd *cmdList_1d[] = {
	cmdDummy,
	boat_1d,   bolt_1d,    bung_1d,  carpet_1d, chop_1d,
	cupb_1d,   door1_1d,   door2_1d, door3_1d,  door4_1d,
	gold_1d,   igor_1d,    knife_1d, mask_1d,   oilcan_1d,
	oldman_1d, pkin_1d,    rope_1d,  shed_1d,   trap_1d,
	ward_1d,   whistle_1d
};

#define climbdumb_2d      {kVClimb_2d,  0,           0,             0,         0, 0,              0,                   kALdumb_2d}
#define climbrope_2d      {kVClimb_2d,  0,           0,             DONT_CARE, 0, 0,              0,                   kALclimbrope_2d}
#define climbwell_2d      {kVClimb_2d,  0,           0,             DONT_CARE, 0, 0,              0,                   kALclimbwell_2d}
#define closedoor1_2d     {kVClose_2d,  0,           0,             1,         0, kDTsclose_2d,   0,                   0}
#define closedoor2_2d     {kVClose_2d,  0,           0,             1,         0, kDTsclose_2d,   0,                   0}
#define closedoor3_2d     {kVClose_2d,  0,           0,             1,         0, kDTsclose_2d,   0,                   0}
#define closesafe_2d      {kVClose_2d,  0,           0,             1,         0, kDTsclose_2d,   kDTokgen_2d,         0}
#define dialphone_2d      {kVDial_2d,   0,           0,             1,         1, kDTsdialed_2d,  0,                   kALphone_2d}
#define dropdynamite_2d   {kVDrop_2d,   kRdyn_2d,    kDTnocgen_2d,  DONT_CARE, 0, 0,              0,                   kALdropdynamite_2d}
#define eatbanana_2d      {kVEat_2d,    kRbanana_2d, kDTnocgen_2d,  0,         0, 0,              0,                   kALeatbanana_2d}
#define eatgarlic_2d      {kVEat_2d,    kRgarlic_2d, kDTnocgen_2d,  DONT_CARE, 0, 0,              0,                   kALgarlic_2d}
#define firegun_2d        {kVFire_2d,   kRgun_2d,    kDTnogun_2d,   0,         1, kDTsempty_2d,   0,                   kALgun_2d}
#define getballoon_2d     {kVTake_2d,   0,           0,             0,         0, 0,              0,                   kALballoon_2d}
#define getbook_2d        {kVTake_2d,   0,           0,             DONT_CARE, 0, 0,              0,                   kALgetbook_2d}
#define getdynamite_2d    {kVTake_2d,   0,           0,             DONT_CARE, 0, 0,              0,                   kALgetdynamite_2d}
#define getletter_2d      {kVTake_2d,   0,           0,             DONT_CARE, 0, kDTsgetlet_2d,  0,                   0}
#define givebanana_2d     {kVGive_2d,   kRbanana_2d, kDTnocgen_2d,  0,         0, 0,              0,                   kALbanana_2d}
#define givebell_2d       {kVGive_2d,   kRbell_2d,   kDTnocgen_2d,  DONT_CARE, 0, 0,              0,                   kALgivebel_2d}
#define givecatnip_2d     {kVGive_2d,   kRcatnip_2d, kDTnocgen_2d,  0,         0, kDTnopurps_2d,  kDTscatnip_2d,       0}
#define intodumb_2d       {kVInto_2d,   0,           0,             0,         0, 0,              0,                   kALdumb_2d}
#define knock_2d          {kVKnock_2d,  0,           0,             DONT_CARE, 0, 0,              kDTtnoknock_2d,      0}
#define lightdynamite_2d  {kVStrike_2d, kRmatch_2d,  kDTnomatch_2d, DONT_CARE, 0, 0,              0,                   kALlightdynamite_2d}
#define lookcubp_2d       {kVLook_2d,   0,           0,             DONT_CARE, 0, 0,              0,                   kALphoto_2d}
#define lookgarlic_2d     {kVLook_2d,   0,           0,             0,         1, kDTempty_2d,    kDTfindclove_2d,     kALgetgarlic_2d}
#define lookhole_2d       {kVLook_2d,   0,           0,             0,         0, kDTdarkhole_2d, 0,                   kALkeyhole_2d}
#define lookkennel_2d     {kVLook_2d,   0,           0,             0,         0, 0,              0,                   kALlookkennel_2d}
#define lookmat_2d        {kVLook_2d,   0,           0,             0,         1, kDTempty_2d,    kDTfindmatch_2d,     kALgetmatch_2d}
#define lookmatch_2d      {kVLook_2d,   0,           0,             DONT_CARE, 0, 0,              0,                   kALlookmatch_2d}
#define opencubp_2d       {kVOpen_2d,   0,           0,             DONT_CARE, 0, 0,              0,                   kALphoto_2d}
#define opendoor1_2d      {kVOpen_2d,   0,           0,             0,         1, kDTsopen1_2d,   0,                   kALopendoor1_2d}
#define opendoor2_2d      {kVOpen_2d,   0,           0,             0,         1, kDTsopen1_2d,   0,                   kALopendoor2_2d}
#define opendoor3_2d      {kVOpen_2d,   0,           0,             0,         1, kDTsopen1_2d,   0,                   kALopendoor3_2d}
#define opengarlic_2d     {kVOpen_2d,   0,           0,             0,         1, kDTempty_2d,    kDTfindclove_2d,     kALgetgarlic_2d}
#define openkdoor_2d      {kVOpen_2d,   0,           0,             0,         0, 0,              kDTwontopen_2d,      0}
#define openlamp_2d       {kVOpen_2d,   kRlamp_2d,   kDTnocgen_2d,  DONT_CARE, 0, 0,              kDTempty_2d,         0}
#define openmat_2d        {kVOpen_2d,   0,           0,             0,         1, kDTempty_2d,    kDTfindmatch_2d,     kALgetmatch_2d}
#define openpdoor_2d      {kVOpen_2d,   0,           0,             0,         0, 0,              kDTspdoor_2d,        0}
#define opensafe_2d       {kVOpen_2d,   0,           0,             DONT_CARE, 0, 0,              0,                   kALsafe_2d}
#define popballoon_2d     {kVBreak_2d,  0,           0,             0,         0, 0,              0,                   kALballoon_2d}
#define pushblue_2d       {kVPush_2d,   0,           0,             DONT_CARE, 0, 0,              0,                   kALworkgates_2d}
#define pushbutton_2d     {kVPush_2d,   0,           0,             DONT_CARE, 0, 0,              kDTtnopushbutton_2d, 0}
#define pushgreen_2d      {kVPush_2d,   0,           0,             DONT_CARE, 0, 0,              0,                   kALbugzapper_2d}
#define pushpaper_2d      {kVPush_2d,   kRpaper_2d,  kDTnocgen_2d,  0,         1, 0,              kDTokgen_2d,         kALpushpaper_2d}
#define pushpencil_2d     {kVPush_2d,   kRpencil_2d, kDTnocgen_2d,  0,         0, 0,              0,                   kALpushpencil_2d}
#define pushred_2d        {kVPush_2d,   0,           0,             DONT_CARE, 0, 0,              0,                   kALshedlight_2d}
#define pushyellow_2d     {kVPush_2d,   0,           0,             DONT_CARE, 0, 0,              0,                   kALgatelight_2d}
#define readletter_2d     {kVRead_2d,   0,           0,             3,         3, kDTsnoread_2d,  kDTsread_2d,         kALreadlet_2d}
#define readwill_2d       {kVRead_2d,   kRwill_2d,   kDTnocgen_2d,  1,         1, 0,              0,                   kALwill_2d}
#define ringbell_2d       {kVRing_2d,   kRbell_2d,   kDTnocgen_2d,  DONT_CARE, 0, 0,              0,                   kALbell_2d}
#define rubcatnip_2d      {kVRub_2d,    kRcatnip_2d, kDTnocgen_2d,  0,         0, kDTnopurps_2d,  0,                   kALcatnip_2d}
#define rublamp_2d        {kVRub_2d,    kRlamp_2d,   kDTnocgen_2d,  0,         0, kDTnopurps_2d,  0,                   kALlamp_2d}
#define serum_2d          {kVDrink_2d,  kRserum_2d,  kDTnocgen_2d,  0,         1, kDTsnosee_2d,   0,                   kALbottle_2d}
#define strikematch_2d    {kVStrike_2d, kRmatch_2d,  kDTnocgen_2d,  DONT_CARE, 0, 0,              0,                   kALstrikematch_2d}
#define talkharry_2d      {kVTalk_2d,   0,           0,             0,         1, kDTsharry_2d,   0,                   kALharry_2d}
#define throwstick_2d     {kVThrow_2d,  kRstick_2d,  kDTnocgen_2d,  0,         1, 0,              0,                   kALthrowstick_2d}
#define unlock_2d         {kVUnlock_2d, 0,           0,             DONT_CARE, 0, 0,              kDTtnounlock_2d,     0}

cmd balloon_2d[] =  {popballoon_2d,  getballoon_2d,   emptyCmd};
cmd banana_2d[] =   {givebanana_2d,  eatbanana_2d,    emptyCmd};
cmd bell_2d[] =     {ringbell_2d,    givebell_2d,     emptyCmd};
cmd blue_2d[] =     {pushblue_2d,    emptyCmd};
cmd book_2d[] =     {getbook_2d,     emptyCmd};
cmd bottle_2d[] =   {serum_2d,       emptyCmd};
cmd button_2d[] =   {pushbutton_2d,  emptyCmd};
cmd catnip_2d[] =   {rubcatnip_2d,   givecatnip_2d,   emptyCmd};
cmd cupbp_2d[] =    {opencubp_2d,    lookcubp_2d,     emptyCmd};
cmd door1_2d[] =    {opendoor1_2d,   closedoor1_2d,   knock_2d,         unlock_2d, emptyCmd};
cmd door2_2d[] =    {opendoor2_2d,   closedoor2_2d,   knock_2d,         unlock_2d, emptyCmd};
cmd door3_2d[] =    {opendoor3_2d,   closedoor3_2d,   knock_2d,         unlock_2d, emptyCmd};
cmd dumb_2d[] =     {intodumb_2d,    climbdumb_2d,    emptyCmd};
cmd dynamite_2d[] = {getdynamite_2d, dropdynamite_2d, lightdynamite_2d, emptyCmd};
cmd garlic_2d[] =   {eatgarlic_2d,   emptyCmd};
cmd green_2d[] =    {pushgreen_2d,   emptyCmd};
cmd gun_2d[] =      {firegun_2d,     emptyCmd};
cmd harry_2d[] =    {talkharry_2d,   emptyCmd};
cmd kdoor_2d[] =    {openkdoor_2d,   emptyCmd};
cmd kennel_2d[] =   {lookkennel_2d,  emptyCmd};
cmd keyhole_2d[] =  {lookhole_2d,    emptyCmd};
cmd lamp_2d[] =     {rublamp_2d,     openlamp_2d,     emptyCmd};
cmd letter_2d[] =   {getletter_2d,   readletter_2d,   emptyCmd};
cmd lookcupb_2d[] = {opengarlic_2d,  lookgarlic_2d,   emptyCmd};
cmd lookdesk_2d[] = {openmat_2d,     lookmat_2d,      emptyCmd};
cmd matches_2d[] =  {lookmatch_2d,   strikematch_2d,  emptyCmd};
cmd paper_2d[] =    {pushpaper_2d,   emptyCmd};
cmd pdoor_2d[] =    {openpdoor_2d,   emptyCmd};
cmd pencil_2d[] =   {pushpencil_2d,  emptyCmd};
cmd red_2d[] =      {pushred_2d,     emptyCmd};
cmd rope_2d[] =     {climbrope_2d,   emptyCmd};
cmd safe_2d[] =     {opensafe_2d,    closesafe_2d,    emptyCmd};
cmd stick_2d[] =    {throwstick_2d,  emptyCmd};
cmd tardis_2d[] =   {dialphone_2d,   emptyCmd};
cmd well_2d[] =     {climbwell_2d,   emptyCmd};
cmd will_2d[] =     {readwill_2d,    emptyCmd};
cmd yellow_2d[] =   {pushyellow_2d,  emptyCmd};

const cmd *cmdList_2d[] = {
	cmdDummy,    balloon_2d, banana_2d, bell_2d,   blue_2d,
	book_2d,     bottle_2d,  button_2d, catnip_2d, cupbp_2d,
	door1_2d,    door2_2d,   door3_2d,  dumb_2d,   dynamite_2d,
	garlic_2d,   green_2d,   gun_2d,    harry_2d,  kdoor_2d,
	kennel_2d,   keyhole_2d, lamp_2d,   letter_2d, lookcupb_2d,
	lookdesk_2d, matches_2d, paper_2d,  pdoor_2d,  pencil_2d,
	red_2d,      rope_2d,    safe_2d,   stick_2d,  tardis_2d,
	well_2d,     will_2d,    yellow_2d
};

#define bell_3d          {kVRing_3d,     kRbell_3d,    kDTnocgen_3d, DONT_CARE, 0, 0,             kDTokbell_3d,    0}
#define blow_3d          {kVBlow_3d,     0,            0,            DONT_CARE, 0, 0,             0,               kALdart_3d}
#define book_3d          {kVRead_3d,     kRbook_3d,    kDTnocgen_3d, DONT_CARE, 0, 0,             0,               kALreadbook_3d}
#define cage1_3d         {kVOpen_3d,     0,            0,            DONT_CARE, 0, 0,             0,               kALopencage_3d}
#define cage2_3d         {kVClose_3d,    0,            0,            DONT_CARE, 0, 0,             kDTokgen_3d,     0}
#define cage3_3d         {kVTake_3d,     0,            0,            DONT_CARE, 0, 0,             0,               kALtakecage_3d}
#define candle1_3d       {kVDouse_3d,    0,            0,            1,         0, kDTsunlit_3d,  kDTokgen_3d,     0}
#define candle2_3d       {kVStrike_3d,   0,            0,            0,         1, kDTslit_3d,    kDTokgen_3d,     0}
#define cdoor1_3d        {kVOpen_3d,     0,            0,            0,         0, kDTsopen1_3d,  0,               kALopendoor_3d}
#define cdoor2_3d        {kVClose_3d,    0,            0,            1,         0, kDTsclose_3d,  kDTokgen_3d,     kALclosedoor_3d}
#define cexit1_3d        {kVOutof_3d,    0,            0,            DONT_CARE, 0, 0,             kDTokgen_3d,     kALexit_3d}
#define cexit2_3d        {kVClimb_3d,    0,            0,            DONT_CARE, 0, 0,             kDTokgen_3d,     kALexit_3d}
#define cflask1_3d       {kVFill_3d,     0,            0,            0,         1, kDTsfull_3d,   0,               kALfill_3d}
#define cflask2_3d       {kVPut_3d,      0,            0,            0,         1, kDTsfull_3d,   0,               kALfill_3d}
#define cflask3_3d       {kVEmpty_3d,    0,            0,            DONT_CARE, 0, 0,             0,               kALempty_3d}
#define cflask4_3d       {kVDrink_3d,    0,            0,            DONT_CARE, 0, 0,             0,               kALdrink_3d}
#define cflask5_3d       {kVGive_3d,     0,            0,            DONT_CARE, 0, 0,             0,               kALflask_3d}
#define cheese1_3d       {kVEat_3d,      kRcheese_3d,  kDTnocgen_3d, 0,         0, 0,             0,               kALeatcheese_3d}
#define cheese2_3d       {kVDrop_3d,     0,            0,            DONT_CARE, 0, 0,             0,               kALdropcheese_3d}
#define cheese3_3d       {kVPut_3d,      0,            0,            DONT_CARE, 0, 0,             0,               kALdropcheese_3d}
#define cheese4_3d       {kVTake_3d,     0,            0,            DONT_CARE, 0, 0,             0,               kALtakecheese_3d}
#define cmake1_3d        {kVMake_3d,     0,            0,            0,         0, kDTsmade_3d,   0,               kALmakeclay_3d}
#define cmake2_3d        {kVStick_3d,    kRpins_3d,    kDTnopins_3d, DONT_CARE, 0, 0,             0,               kALstick_3d}
#define cplane1_3d       {kVClimb_3d,    0,            0,            DONT_CARE, 0, 0,             kDTokgen_3d,     kALplane_3d}
#define cplane2_3d       {kVInto_3d,     0,            0,            DONT_CARE, 0, 0,             kDTokgen_3d,     kALplane_3d}
#define cplane3_3d       {kVSearch_3d,   0,            0,            DONT_CARE, 0, 0,             kDTokgen_3d,     kALplane_3d}
#define crystal_3d       {kVRub_3d,      kRcrystal_3d, kDTnocgen_3d, DONT_CARE, 0, 0,             0,               kALcrystal_3d}
#define cstick1_3d       {kVStick_3d,    0,            0,            0,         0, 0,             0,               kALstick_3d}
#define cswing1_3d       {kVSwing_3d,    0,            0,            DONT_CARE, 0, 0,             kDTokgen_3d,     kALswing_3d}
#define ctalknat_3d      {kVTalk_3d,     0,            0,            DONT_CARE, 0, 0,             0,               kALtalknat_3d}
#define cube1_3d         {kVGive_3d,     0,            0,            DONT_CARE, 0, 0,             0,               kALgiveb_3d}
#define cvine1_3d        {kVUntie_3d,    0,            0,            0,         0, 0,             0,               kALuntie_vine_3d}
#define cvine2_3d        {kVTie_3d,      0,            0,            0,         0, 0,             0,               kALvine_3d}
#define cwaterfall_3d    {kVLook_3d,     0,            0,            DONT_CARE, 0, 0,             0,               kALlookwfall_3d}
#define cwaterpool_3d    {kVLook_3d,     0,            0,            DONT_CARE, 0, 0,             kDTdull_3d,      0}
#define cwaterstream_3d  {kVLook_3d,     0,            0,            DONT_CARE, 0, 0,             kDTdull_3d,      0}
#define dart_3d          {kVShoot_3d,    0,            0,            DONT_CARE, 0, 0,             0,               kALdart_3d}
#define ghost_3d         {kVExorcise_3d, kRexor_3d,    kDTnocex_3d,  DONT_CARE, 0, 0,             0,               kALexorcise_3d}
#define knock_3d         {kVKnock_3d,    0,            0,            DONT_CARE, 0, 0,             kDTsNobody_3d,   0}
#define readit_3d        {kVRead_3d,     0,            0,            DONT_CARE, 0, 0,             kDTsread_3d,     0}
#define rock1_3d         {kVBehind_3d,   0,            0,            0,         1, kDTsfoundc_3d, kDTsfindc_3d,    kALfindcrystal_3d}
#define swingc_3d        {kVSwing_3d,    0,            0,            DONT_CARE, 0, 0,             kDTswingcave_3d, 0}
#define unlock_3d        {kVUnlock_3d,   0,            0,            DONT_CARE, 0, 0,             kDTsUnlocked_3d, 0}

cmd cbell_3d[] =     {bell_3d,         emptyCmd};
cmd cbook_3d[] =     {book_3d,         emptyCmd};
cmd cbouillon_3d[] = {cube1_3d,        emptyCmd};
cmd ccage_3d[] =     {cage1_3d,        cage2_3d,   cage3_3d,   emptyCmd};
cmd ccandle_3d[] =   {candle1_3d,      candle2_3d, emptyCmd};
cmd ccheese_3d[] =   {cheese1_3d,      cheese2_3d, cheese3_3d, cheese4_3d, emptyCmd};
cmd cclay_3d[] =     {cmake1_3d,       cmake2_3d,  emptyCmd};
cmd ccrystal_3d[] =  {crystal_3d,      emptyCmd};
cmd cdart_3d[] =     {dart_3d,         blow_3d,    emptyCmd};
cmd cdoor_3d[] =     {cdoor1_3d,       cdoor2_3d,  emptyCmd};
cmd cexit_3d[] =     {cexit1_3d,       cexit2_3d,  emptyCmd};
cmd cflask_3d[] =    {cflask1_3d,      cflask2_3d, cflask3_3d, cflask4_3d, cflask5_3d, emptyCmd};
cmd cghost_3d[] =    {ghost_3d,        emptyCmd};
cmd cnative_3d[] =   {ctalknat_3d,     emptyCmd};
cmd cpins_3d[] =     {cstick1_3d,      emptyCmd};
cmd cplane_3d[] =    {cplane1_3d,      cplane2_3d, cplane3_3d, emptyCmd};
cmd crock_3d[] =     {rock1_3d,        emptyCmd};
cmd cscroll_3d[] =   {readit_3d,       emptyCmd};
cmd cswing_3d[] =    {cswing1_3d,      emptyCmd};
cmd cswingc_3d[] =   {swingc_3d,       emptyCmd};
cmd cvine_3d[] =     {cvine1_3d,       cvine2_3d,  emptyCmd};
cmd cwfall_3d[] =    {cwaterfall_3d,   emptyCmd};
cmd cwpool_3d[] =    {cwaterpool_3d,   emptyCmd};
cmd cwstream_3d[] =  {cwaterstream_3d, emptyCmd};

const cmd *cmdList_3d[] = {
	cmdDummy,   cbell_3d,   cbook_3d,  cbouillon_3d, ccage_3d,
	ccandle_3d, ccheese_3d, cclay_3d,  ccrystal_3d,  cdart_3d,
	cdoor_3d,   cexit_3d,   cflask_3d, cghost_3d,    cnative_3d,
	cpins_3d,   cplane_3d,  crock_3d,  cscroll_3d,   cswing_3d,
	cswingc_3d, cvine_3d,   cwfall_3d, cwpool_3d,    cwstream_3d
};

// The following are lists of actions invoked when entering a screen
// They consist of actions which occur no matter which door is entered
// See the list of 'hotspots' and 'Open' cmds for door specific actions
uint16 s0acts_1w[]  = {kALclosedoor1_1w, kALblinkeyes1_1w, kALightning_1w, kALbat_1w,    0}; // House
uint16 s1acts_1w[]  = {kALblinkeyes2_1w, kALridprof_1w,    0};                               // Hall
uint16 s2acts_1w[]  = {0};                                                                   // Bed1
uint16 s3acts_1w[]  = {kALbut_1w,        kALrepredeye_1w,  kALreplips_1w,  kALreparm_1w, 0}; // dining room
uint16 s4acts_1w[]  = {0};                                                                   // Bathroom
uint16 s5acts_1w[]  = {0};                                                                   // Kitchen
uint16 s6acts_1w[]  = {0};                                                                   // Garden
uint16 s7acts_1w[]  = {kALdog_1w,        0};                                                 // Store room
uint16 s8acts_1w[]  = {kALhelp_1w,       0};                                                 // Basement
uint16 s9acts_1w[]  = {kALbatattack_1w,  0};                                                 // Batcave
uint16 s10acts_1w[] = {kALmum_1w,        0};                                                 // Mummy room
uint16 s11acts_1w[] = {0};                                                                   // Lake room
uint16 s12acts_1w[] = {0};                                                                   // Dead end
uint16 s13acts_1w[] = {kALjail_1w,       0};                                                 // Jail
uint16 s14acts_1w[] = {kALgoodbye_1w,    0};                                                 // The end
uint16 s15acts_1w[] = {kALlab_1w,        kALbox_1w,        0};                               // Laboratory

const uint16 *screenActs_1w[] = {
	s0acts_1w,  s1acts_1w,  s2acts_1w,  s3acts_1w,  s4acts_1w,
	s5acts_1w,  s6acts_1w,  s7acts_1w,  s8acts_1w,  s9acts_1w,
	s10acts_1w, s11acts_1w, s12acts_1w, s13acts_1w, s14acts_1w,
	s15acts_1w
};

uint16 s0acts_2w[]  = {kALscr01Story_2w, 0};                             // House
uint16 s1acts_2w[]  = {kALpenny1_2w,     kALmaid_2w, kALheroxy01_2w, 0}; // Hall
uint16 s2acts_2w[]  = {kALscr02_2w,      0};                             // Bed1
uint16 s3acts_2w[]  = {kALscr03_2w,      0};                             // Bed2
uint16 s4acts_2w[]  = {kALscr04_2w,      0};                             // Murder
uint16 s6acts_2w[]  = {kALscr06_2w,      0};                             // Kitchen
uint16 s9acts_2w[]  = {kALscr09_2w,      0};                             // In shed
uint16 s10acts_2w[] = {kALscr10_2w,      0};                             // Venus
uint16 s14acts_2w[] = {kALscr14_2w,      0};                             // Bug attack
uint16 s15acts_2w[] = {kALscr15_2w,      0};                             // Old man
uint16 s17acts_2w[] = {kALmap0_2w,       0};                             // Snakepit
uint16 s18acts_2w[] = {kALmap1_2w,       0};                             // Phonebox
uint16 s25acts_2w[] = {kALscr25_2w,      0};                             // Chasm
uint16 s29acts_2w[] = {kALscr29_2w,      0};                             // Hall2
uint16 s30acts_2w[] = {kALscr30_2w,      0};                             // Lounge
uint16 s31acts_2w[] = {kALmaidp_2w,      0};                             // Parlor
uint16 s33acts_2w[] = {kALscr33_2w,      0};                             // Boxroom
uint16 s34acts_2w[] = {kALscr34_2w,      0};                             // Hall3
uint16 s35acts_2w[] = {kALscr35_2w,      0};                             // Organ
uint16 s36acts_2w[] = {kALscr36_2w,      0};                             // Hestroom
uint16 s37acts_2w[] = {kALsong3_2w,      0};                             // Retupmoc
const uint16 *screenActs_2w[] = {
	/*  0 */ s0acts_2w,  s1acts_2w,  s2acts_2w,  s3acts_2w,  s4acts_2w,
	         0,          s6acts_2w,  0,          0,          s9acts_2w,
	/* 10 */ s10acts_2w, 0,          0,          0,          s14acts_2w,
	         s15acts_2w, 0,          s17acts_2w, s18acts_2w, 0,
	/* 20 */ 0,          0,          0,          0,          0,
	         s25acts_2w, 0,          0,          0,          s29acts_2w,
	/* 30 */ s30acts_2w, s31acts_2w, 0,          s33acts_2w, s34acts_2w,
	         s35acts_2w, s36acts_2w, s37acts_2w, 0,          0,
	/* 40 */ 0,          0,          0,          0,          0,
	         0,          0,          0,          0,          0,
	/* 50 */ 0,          0,          0,          0,          0,
	         0,          0,          0,          0,          0,
	/* 60 */ 0,          0,          0,          0,          0,
	         0,          0,          0,          0,          0,
	/* 70 */ 0,          0,          0,          0,          0,
	         0,          0,          0,          0,          0,
	/* 80 */ 0,          0,          0,          0,          0,
	         0,          0,          0,          0,          0,
	/* 90 */ 0,          0,          0,          0,          0,
	         0,          0,          0,          0,          0,
	/*100 */ 0,          0,          0
};

uint16 s0acts_3w[]  = {kALcrashStory_3w, kALhorizon_3w,   0}; // Crash site
uint16 s1acts_3w[]  = {kALweb_3w,        0};                  // Spider's web
uint16 s2acts_3w[]  = {kALbridgetest_3w, kALbridgetip_3w, 0}; // Bridge1
uint16 s13acts_3w[] = {kALhut_in_3w,     kALmouse_3w,     0}; // In hut
uint16 s15acts_3w[] = {kALoldman_3w,     kALflash_3w,     0}; // Oldman
uint16 s18acts_3w[] = {kALcamp_3w,       0};                  // At camp
uint16 s19acts_3w[] = {kALsunset_3w,     0};                  // Bye bye!
uint16 s23acts_3w[] = {kALpath_3w,       0};                  // Elephant
const uint16 *screenActs_3w[] = {
	/*  0 */ s0acts_3w,  s1acts_3w, s2acts_3w, 0,          0,
	         0,          0,         0,         0,          0,
	/* 10 */ 0,          0,         0,         s13acts_3w, 0,
	         s15acts_3w, 0,         0,         s18acts_3w, s19acts_3w,
	/* 20 */ 0,          0,         0,         s23acts_3w, 0,
	         0,          0,         0,         0,          0,
	/* 30 */ 0
};

uint16 s0acts_1d[] =  {kALsong1_1d,      kALclosedoor1_1d, kALblinkeyes1_1d, kALightning_1d, kALbat_1d, 0};
uint16 s1acts_1d[] =  {kALblinkeyes2_1d, kALridprof_1d,    0};
uint16 s2acts_1d[] =  {0};
uint16 s3acts_1d[] =  {kALbut_1d,        kALrepredeye_1d,  kALreplips_1d,    kALreparm_1d,   0};
uint16 s4acts_1d[] =  {0};
uint16 s5acts_1d[] =  {0};
uint16 s6acts_1d[] =  {0};
uint16 s7acts_1d[] =  {kALdog_1d,        0};
uint16 s8acts_1d[] =  {kALhelp_1d,       0};
uint16 s9acts_1d[] =  {kALbatattack_1d,  0};
uint16 s10acts_1d[] = {kALmum_1d,        0};
uint16 s11acts_1d[] = {0};
uint16 s12acts_1d[] = {0};
uint16 s13acts_1d[] = {kALjail_1d,       0};
uint16 s14acts_1d[] = {kALgoodbye_1d,    0};
uint16 s15acts_1d[] = {kALlab_1d,        0};

const uint16 *screenActs_1d[] = {
	s0acts_1d,  s1acts_1d,  s2acts_1d,  s3acts_1d,  s4acts_1d,
	s5acts_1d,  s6acts_1d,  s7acts_1d,  s8acts_1d,  s9acts_1d,
	s10acts_1d, s11acts_1d, s12acts_1d, s13acts_1d, s14acts_1d,
	s15acts_1d
};


uint16 s0acts_2d[]  = {kALscr01_2d,  kALsong1_2d, 0};                  // House
uint16 s1acts_2d[]  = {kALpenny1_2d, kALmaid_2d,  kALheroxy01_2d, 0};  // Hall
uint16 s2acts_2d[]  = {kALscr02_2d,  0};                               // Bed1
uint16 s3acts_2d[]  = {kALscr03_2d,  0};                               // Bed2
uint16 s4acts_2d[]  = {kALscr04_2d,  0};                               // Murder
uint16 s6acts_2d[]  = {kALscr06_2d,  0};                               // Kitchen
uint16 s9acts_2d[]  = {kALscr09_2d,  0};                               // In shed
uint16 s10acts_2d[] = {kALscr10_2d,  0};                               // Venus
uint16 s14acts_2d[] = {kALscr14_2d,  0};                               // Bug attack
uint16 s15acts_2d[] = {kALscr15_2d,  0};                               // Old man
uint16 s17acts_2d[] = {kALmap0_2d,   0};                               // Snakepit
uint16 s18acts_2d[] = {kALmap1_2d,   0};                               // Phonebox
uint16 s29acts_2d[] = {kALscr29_2d,  0};                               // Hall2
uint16 s30acts_2d[] = {kALscr30_2d,  0};                               // Lounge
uint16 s31acts_2d[] = {kALmaidp_2d,  0};                               // Parlor
uint16 s33acts_2d[] = {kALscr33_2d,  0};                               // Boxroom
uint16 s34acts_2d[] = {kALscr34_2d,  0};                               // Hall3
uint16 s35acts_2d[] = {kALscr35_2d,  0};                               // Organ
uint16 s36acts_2d[] = {kALscr36_2d,  0};                               // Hestroom
uint16 s37acts_2d[] = {kALsong3_2d,  0};                               // Retupmoc

const uint16 *screenActs_2d[] = {
	s0acts_2d,  s1acts_2d,  s2acts_2d,  s3acts_2d,  s4acts_2d,
	0,          s6acts_2d,  0,          0,          s9acts_2d,
	s10acts_2d, 0,          0,          0,          s14acts_2d,
	s15acts_2d, 0,          s17acts_2d, s18acts_2d, 0,
	0,          0,          0,          0,          0,
	0,          0,          0,          0,          s29acts_2d,
	s30acts_2d, s31acts_2d, 0,          s33acts_2d, s34acts_2d,
	s35acts_2d, s36acts_2d, s37acts_2d, 0,          0,
	0,          0,          0,          0,          0,
	0,          0,          0,          0,          0,
	0,          0,          0,          0,          0,
	0,          0,          0,          0,          0,
	0,          0,          0,          0,          0,
	0,          0,          0,          0,          0,
	0,          0,          0,          0,          0,
	0,          0,          0,          0,          0,
	0,          0,          0,          0,          0,
	0,          0,          0,          0,          0,
	0,          0,          0,          0,          0,
	0,          0,          0,          0,          0,
	0,          0,          0
};

uint16 s0acts_3d[]  = {kALcrashStory_3d, kALhorizon_3d, 0}; // Crash site
uint16 s1acts_3d[]  = {kALweb_3d,        0};                // Spider's web
uint16 s2acts_3d[]  = {kALbridgetip_3d,  0};                // Bridge1
uint16 s13acts_3d[] = {kALhut_in_3d,     kALmouse_3d,   0}; // In hut
uint16 s15acts_3d[] = {kALoldman_3d,     kALflash_3d,   0}; // Oldman
uint16 s18acts_3d[] = {kALcamp_3d,       0};                // At camp
uint16 s19acts_3d[] = {kALsunset_3d,     0};                // Bye bye!
uint16 s23acts_3d[] = {kALpath_3d,       0};                // Elephant
const uint16 *screenActs_3d[] = {
	/*  0 */ s0acts_3d,  s1acts_3d, s2acts_3d, 0,          0,
	         0,          0,         0,         0,          0,
	/* 10 */ 0,          0,         0,         s13acts_3d, 0,
	         s15acts_3d, 0,         0,         s18acts_3d, s19acts_3d,
	/* 20 */ 0,          0,         0,         s23acts_3d, 0,
	         0,          0,         0,         0,          0,
	/* 30 */ 0
};

object_t objects_1w[] = {
//name,       description, description_s,path, dx,  dy,  aptr,         seq,seqp,     cyc,             n,frm,rad,scr,x,y  ,        oldxy,vxy,val,g,cmnd,         c, s,ctx,fgb
{kNHero_1w,        kDTthero_1w,    0, USER, 0,   0,   0,            PERSON, CYCLE_FORWARD,  0, 0, 0,  0, 0,  229, 144, 90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 0: (Outside house)
{kNDoor_1w,        kDTtdoor_1w,    0, AUTO, 0,   0,   0,            THING4, INVISIBLE,      1, 4, 4, 16, 0,  26,  131, 90,  90,  0,  0, 0,  1, kCMDdoor1_1w,   0, 0, 0, FLOATING,   29,  175, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNEyes_1w,        kDTteyes_1w,    0, AUTO, 0,   0,   0,            THING1, NOT_CYCLING,    0, 0, 0, -1, 0,  80,  148, 90,  90,  0,  0, 0,  1, 0,              0, 0, 0, BACKGROUND, 72,  175, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNEyes_1w,        kDTteyes_1w,    0, AUTO, 0,   0,   0,            THING1, NOT_CYCLING,    0, 0, 0, -1, 0,  59,  78,  90,  90,  0,  0, 0,  1, 0,              0, 0, 0, BACKGROUND, 72,  175, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNBat_1w,         kDTtbat_1w,     0, AUTO, 0,   0,   0,            THING1, NOT_CYCLING,    0, 0, 0, -1, 0,  95,  55,  90,  90,  0,  0, 0,  1, 0,              0, 0, 0, BACKGROUND, 0,   0,   0,                     0, 0, 0, 0},
{kNPkin_1w,        kDTtpkin_1w,    0, AUTO, 0,   0,   0,            THING1, NOT_CYCLING,    0, 0, 0, 10, 0,  20,  168, 90,  90,  0,  0, 2,  7, kCMDpkin_1w,    0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNKey_1w,         kDTtkey_1w,     0, AUTO, 0,   0,   0,            THING1, INVISIBLE,      0, 0, 0, 10, 0,  24,  177, 90,  90,  0,  0, 5,  7, 0,              0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNFence_1w,       0,              0, AUTO, 193, 37,  0,            THING0, INVISIBLE,      0, 0, 0, 30, 0,  69,  161, 127, 145, 0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   225, 183, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNTree_1w,        0,              0, AUTO, 25,  52,  0,            THING0, INVISIBLE,      0, 0, 0, 30, 0,  69,  161, 186, 93,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   190, 183, Common::KEYCODE_UP,    0, 0, 0, 0},
// Screen 1: (Hall)
{kNDoor_1w,        kDTtdoor_1w,    0, AUTO, 0,   0,   0,            THING4, NOT_CYCLING,    1, 4, 4, 16, 1,  125, 56,  90,  90,  0,  0, 0,  1, kCMDdoor2_1w,   0, 0, 0, FLOATING,   127, 98,  Common::KEYCODE_UP,    0, 0, 0, 0},
{kNDoor_1w,        kDTtdoor_1w,    0, AUTO, 0,   0,   0,            THING4, NOT_CYCLING,    1, 4, 4, 16, 1,  208, 56,  90,  90,  0,  0, 0,  1, kCMDdoor3_1w,   0, 0, 0, FLOATING,   210, 98,  Common::KEYCODE_UP,    0, 0, 0, 0},
{kNEyes_1w,        kDTteyes_1w,    0, AUTO, 0,   0,   0,            THING1, NOT_CYCLING,    0, 0, 0, -1, 1,  23,  48,  90,  90,  0,  0, 0,  1, 0,              0, 0, 0, BACKGROUND, 48,  98,  Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNEyes_1w,        kDTteyes_1w,    0, AUTO, 0,   0,   0,            THING1, NOT_CYCLING,    0, 0, 0, -1, 1,  7,   93,  90,  90,  0,  0, 0,  1, 0,              0, 0, 0, BACKGROUND, 32,  140, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNBatpic_1w,      0,              0, AUTO, 46,  30,  0,            THING0, INVISIBLE,      0, 0, 0, 30, 1,  69,  161, 113, 108, 0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   110, 160, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNCandle_1w,      kDTtcandle_1w,  0, AUTO, 0,   0,   0,            THING2, CYCLE_FORWARD,  0, 0, 0, 30, 1,  176, 131, 90,  90,  0,  0, 3,  7, 0,              0, 0, 0, FLOATING,   164, 165, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNCupb_1w,        0,              0, AUTO, 9,   25,  0,            THING0, INVISIBLE,      0, 0, 0, 20, 1,  69,  161, 67,  140, 0,  0, 0,  1, kCMDcupb_1w,    0, 0, 1, FLOATING,   81,  168, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNKnife_1w,       kDTtknife_1w,   0, AUTO, 0,   0,   0,            THING1, INVISIBLE,      0, 0, 0, 30, 1,  69,  169, 90,  90,  0,  0, 6,  7, kCMDknife_1w,   0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNWhistle_1w,     kDTtwhistle_1w, 0, AUTO, 0,   0,   0,            THING1, INVISIBLE,      0, 0, 0, 30, 1,  79,  171, 90,  90,  0,  0, 6,  7, kCMDwhistle_1w, 0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
// Screen 2: (Bedroom 1)
{kNWard_1w,        kDTtward_1w,    0, AUTO, 46,  51,  0,            THING0, INVISIBLE,      1, 4, 4, 30, 2,  172, 113, 150, 58,  0,  0, 0,  1, kCMDward_1w,    0, 0, 0, FLOATING,   168, 117, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNWdoorl_1w,      0,              0, AUTO, 0,   0,   0,            THING4, NOT_CYCLING,    1, 4, 4, 0,  2,  150, 56,  90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNWdoorr_1w,      0,              0, AUTO, 0,   0,   0,            THING4, NOT_CYCLING,    1, 4, 4, 0,  2,  174, 56,  90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNMask_1w,        kDTtmask_1w,    0, AUTO, 0,   0,   0,            THING1, INVISIBLE,      0, 0, 0, 10, 2,  155, 100, 90,  90,  0,  0, 4,  7, kCMDmask_1w,    0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNMonkey_1w,      0,              0, AUTO, 0,   0,   0,            PERSON, INVISIBLE,      0, 0, 0, 0,  2,  229, 144, 90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNWindow_1w,      0,              0, AUTO, 29,  22,  0,            THING0, INVISIBLE,      1, 4, 4, 30, 2,  172, 113, 117, 57,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   136, 116, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNBed_1w,         0,              0, AUTO, 60,  36,  0,            THING0, INVISIBLE,      1, 4, 4, 30, 2,  172, 113, 78,  92,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   127, 130, Common::KEYCODE_LEFT,  0, 0, 0, 0},
// Screen 3: (Dining room)
{kNButler_1w,      kDTtbutler_1w,  0, AUTO, 0,   0,   kALbutler_1w, PERSON,  NOT_CYCLING,   0, 0, 0, 20, 3,  70,  78,  90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNChop_1w,        kDTtchop_1w,    0, AUTO, 0,   0,   0,            THING1,  INVISIBLE,     0, 0, 0, 20, 3,  69,  161, 90,  90,  0,  0, 0,  7, kCMDchop_1w,    0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNRedeyes_1w,     0,              0, AUTO, 0,   0,   0,            THING1,  NOT_CYCLING,   0, 0, 0, 0,  3,  212, 108, 90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FOREGROUND, 0,   0,   0,                     0, 0, 0, 0},
{kNLips_1w,        0,              0, AUTO, 0,   0,   0,            THING2,  NOT_CYCLING,   0, 1, 1, 0,  3,  113, 105, 90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FOREGROUND, 0,   0,   0,                     0, 0, 0, 0},
{kNArm_1w,         0,              0, AUTO, 0,   0,   0,            THING2,  NOT_CYCLING,   0, 5, 5, 0,  3,  166, 122, 90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNHdlshero_1w,    0,              0, AUTO, 0,   0,   0,            THING1,  INVISIBLE,     0, 0, 0, 10, 0,  24,  177, 90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNPicture_1w,     0,              0, AUTO, 74,  41,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 3,  172, 113, 122, 52,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   169, 120, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNWindow_1w,      0,              0, AUTO, 41,  50,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 3,  172, 113, 265, 64,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   269, 139, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNCupb_1w,        0,              0, AUTO, 44,  29,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 3,  172, 113, 65,  69,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   78,  124, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNFrank_1w,       kDTtfrank_1w,   0, AUTO, 23,  28,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 3,  90,  110, 104, 97,  0,  0, 0,  1, kCMDfrank_1w,   0, 0, 0, FLOATING,   90,  134, Common::KEYCODE_DOWN,  0, 0, 0, 0},
{kNDracula_1w,     kDTtdrac_1w,    0, AUTO, 28,  18,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 3,  130, 110, 135, 107, 0,  0, 0,  1, kCMDdrac_1w,    0, 0, 0, FLOATING,   130, 134, Common::KEYCODE_DOWN,  0, 0, 0, 0},
{kNGwen_1w,        kDTtlady_1w,    0, AUTO, 17,  23,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 3,  160, 110, 174, 97,  0,  0, 0,  1, kCMDgwen_1w,    0, 0, 0, FLOATING,   162, 134, Common::KEYCODE_DOWN,  0, 0, 0, 0},
{kNHood_1w,        kDTthood_1w,    0, AUTO, 35,  26,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 3,  195, 110, 205, 101, 0,  0, 0,  1, kCMDhood_1w,    0, 0, 0, FLOATING,   195, 134, Common::KEYCODE_DOWN,  0, 0, 0, 0},
{kNSlime_1w,       kDTtslime_1w,   0, AUTO, 21,  24,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 3,  75,  140, 88,  113, 0,  0, 0,  1, kCMDslime_1w,   0, 0, 0, FLOATING,   74,  160, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNPeahead_1w,     kDTtpeahd_1w,   0, AUTO, 20,  15,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 3,  164, 140, 172, 121, 0,  0, 0,  1, kCMDpeahd_1w,   0, 0, 0, FLOATING,   152, 162, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNFood_1w,        0,              0, AUTO, 19,  10,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 3,  130, 140, 149, 127, 0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   138, 162, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNPlant_1w,       0,              0, AUTO, 11,  21,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 3,  172, 113, 239, 74,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   230, 125, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
// Screen 4: (bathroom)
{kNWindow_1w,      0,              0, AUTO, 31,  27,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 4,  172, 113, 153, 61,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   157, 114, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNToilet_1w,      0,              0, AUTO, 32,  17,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 4,  172, 113, 101, 113, 0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   122, 122, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNBath_1w,        0,              0, AUTO, 47,  36,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 4,  172, 113, 73,  125, 0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   119, 156, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNMirror_1w,      0,              0, AUTO, 12,  24,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 4,  172, 113, 207, 71,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   180, 127, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
// Screen 5: (kitchen)
{kNWindow_1w,      0,              0, AUTO, 66,  35,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 5,  172, 113, 107, 76,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   124, 143, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNBroom_1w,       0,              0, AUTO, 19,  51,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 5,  172, 113, 280, 114, 0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   276, 166, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNCupb_1w,        0,              0, AUTO, 159, 29,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 5,  172, 113, 50,  112, 0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 6: (Garden)
{kNDoor_1w,        kDTtdoor_1w,    0, AUTO, 0,   0,   0,            THING4,  NOT_CYCLING,   1, 4, 4, 16, 6,  226, 58,  90,  90,  0,  0, 0,  1, kCMDdoor4_1w,   0, 0, 0, FLOATING,   224, 104, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNShed_1w,        0,              0, AUTO, 106, 60,  0,            THING0,  INVISIBLE,     0, 0, 0, 60, 6,  277, 39,  214, 37,  0,  0, 0,  1, kCMDshed_1w,    0, 0, 1, FLOATING,   213, 103, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNOilcan_1w,      kDTtoilcan_1w,  0, AUTO, 0,   0,   0,            THING0,  INVISIBLE,     0, 0, 0, 40, 6,  240, 65,  90,  90,  0,  0, 4,  1, kCMDoilcan_1w,  0, 0, 0, FOREGROUND, 0,   0,   0,                     0, 0, 0, 0},
{kNTree_1w,        0,              0, AUTO, 30,  120, 0,            THING0,  INVISIBLE,     1, 4, 4, 30, 6,  172, 113, 161, 17,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   163, 148, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNShed_1w,        0,              0, AUTO, 25,  40,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 6,  172, 113, 226, 58,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   285, 99,  Common::KEYCODE_RIGHT, 0, 0, 0, 0},
// Screen 7: (Store room)
{kNDog_1w,         kDTtdog_1w,     0, AUTO, 0,   0,   kALdoggy_1w,  ANIMAL,  NOT_CYCLING,   0, 0, 0, -1, 7,  105, 119, 90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNCarpet_1w,      0,              0, AUTO, 0,   0,   0,            THING1,  NOT_CYCLING,   0, 0, 0, 50, 7,  191, 142, 90,  90,  0,  0, 0,  0, kCMDcarpet_1w,  0, 0, 0, BACKGROUND, 234, 153, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNTrap_1w,        kDTttrap_1w,    0, AUTO, 0,   0,   0,            THING4,  INVISIBLE,     0, 2, 2, 20, 7,  216, 140, 90,  90,  0,  0, 0,  1, kCMDtrap_1w,    0, 0, 0, BACKGROUND, 240, 152, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNBolt_1w,        kDTtbolt_1w,    0, AUTO, 7,   5,   0,            THING0,  INVISIBLE,     0, 0, 0, 20, 7,  220, 145, 237, 151, 0,  0, 0,  1, kCMDbolt_1w,    0, 0, 0, BACKGROUND, 240, 152, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNHerodead_1w,    0,              0, AUTO, 0,   0,   0,            THING1,  INVISIBLE,     0, 0, 0, 10, 0,  24,  177, 90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNMousehole_1w,   0,              0, AUTO, 7,   8,   0,            THING0,  INVISIBLE,     1, 4, 4, 30, 7,  172, 113, 52,  148, 0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   65,  158, Common::KEYCODE_LEFT,  0, 0, 0, 0},
// Screen 8: (Basement)
{kNRock_1w,        0,              0, AUTO, 66,  53,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 8,  172, 113, 132, 88,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   123, 149, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNRock_1w,        0,              0, AUTO, 56,  60,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 8,  172, 113, 257, 125, 0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   270, 187, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNRock_1w,        0,              0, AUTO, 25,  91,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 8,  172, 113, 0,   102, 0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   20,  187, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNDoor_1w,        0,              0, AUTO, 30,  57,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 8,  172, 113, 207, 84,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   210, 142, Common::KEYCODE_UP,    0, 0, 0, 0},
// Screen 9: (Batcave)
{kNBat_1w,         kDTtbat_1w,     0, AUTO, 0,   0,   kALbats_1w,   THING1,  NOT_CYCLING,   0, 0, 0, 16, 9,  65,  25,  90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBat_1w,         kDTtbat_1w,     0, AUTO, 0,   0,   kALbats_1w,   THING1,  NOT_CYCLING,   0, 0, 0, 16, 9,  55,  65,  90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBat_1w,         kDTtbat_1w,     0, AUTO, 0,   0,   kALbats_1w,   THING1,  NOT_CYCLING,   0, 0, 0, 16, 9,  55,  120, 90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBat_1w,         kDTtbat_1w,     0, AUTO, 0,   0,   kALbats_1w,   THING1,  NOT_CYCLING,   0, 0, 0, 16, 9,  55,  130, 90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 10: (Mummy room)
{kNMummy_1w,       0,              0, AUTO, 0,   0,   kALmummy_1w,  PERSON,  NOT_CYCLING,   0, 0, 0, DX, 10, 256, 77,  90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNMdoor,          0,              0, AUTO, 0,   0,   0,            THING4,  NOT_CYCLING,   1, 4, 4, 0,  10, 258, 55,  90,  90,  0,  0, 0,  1, kCMDdoor4_1w,   0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNGold_1w,        kDTtgold_1w,    0, AUTO, 0,   0,   0,            THING1,  NOT_CYCLING,   0, 0, 0, 20, 10, 208, 152, 90,  90,  0,  0, 10, 7, kCMDgold_1w,    0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNRock_1w,        0,              0, AUTO, 54,  34,  0,            THING0,  INVISIBLE,     1, 4, 4, -1, 10, 172, 113, 127, 109, 0,  0, 0,  1, kCMDrock_1w,    0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 11: (Lakeroom)
{kNBoat_1w,        kDTtboat_1w,    0, AUTO, 0,   0,   0,            THING2c, NOT_CYCLING,   0, 0, 0, 30, 11, 230, 118, 90,  90,  0,  0, 0,  1, kCMDboat_1w,    0, 0, 1, FLOATING,   250, 150, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNRope_1w,        kDTtrope_1w,    0, AUTO, 0,   0,   0,            THING2c, NOT_CYCLING,   0, 0, 0, 30, 11, 220, 132, 90,  90,  0,  0, 0,  1, kCMDrope_1w,    0, 0, 0, FLOATING,   209, 153, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNOldman_1w,      kDTtoldman_1w,  0, AUTO, 0,   0,   0,            THING1,  NOT_CYCLING,   0, 0, 0, 30, 11, 160, 38,  90,  90,  0,  0, 0,  1, kCMDoldman_1w,  0, 0, 0, FLOATING,   150, 54,  Common::KEYCODE_DOWN,  0, 0, 0, 0},
{kNWhero_1w,       0,              0, AUTO, 0,   0,   0,            PERSON,  INVISIBLE,     0, 0, 0, 0,  11, 100, 100, 90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 12: (Dead end)
{kNGuard_1w,       kDTtguard_1w,   0, AUTO, 0,   0,   0,            THING2d, NOT_CYCLING,   0, 0, 0, -1, 12, 147, 38,  90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   133, 91,  Common::KEYCODE_UP,    0, 0, 0, 0},
// Screen 15: (Laboratory)
{kNProf_1w,        kDTtprof_1w,    0, AUTO, 0,   0,   0,            PERSON2, CYCLE_FORWARD, 0, 0, 0, -1, 1,  150, 55,  90,  90,  DX, 0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNIgor_1w,        kDTtigor_1w,    0, AUTO, 0,   0,   0,            PERSON2, CYCLE_FORWARD, 0, 0, 0, -1, 15, 180, 122, 90,  90,  DX, 0, 0,  1, kCMDigor_1w,    0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBung_1w,        kDTtbung_1w,    0, AUTO, 0,   0,   0,            THING1,  NOT_CYCLING,   0, 0, 0, 14, 15, 75,  145, 90,  90,  0,  0, 11, 7, kCMDbung_1w,    0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNGdoor_1w,       0,              0, AUTO, 0,   0,   0,            THING1,  NOT_CYCLING,   0, 0, 0, 30, 15, 59,  100, 90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNSpachero_1w,    0,              0, AUTO, 0,   0,   0,            PERSON,  INVISIBLE,     0, 0, 0, 0,  15, 100, 100, 90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNFuzyhero_1w,    0,              0, AUTO, 0,   0,   0,            PERSON,  INVISIBLE,     0, 0, 0, 0,  15, 100, 100, 90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNSpark_1w,       0,              0, AUTO, 0,   0,   0,            THING2,  CYCLE_FORWARD, 0, 0, 0, 0,  15, 106, 74,  90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBooth_1w,       0,              0, AUTO, 38,  64,  0,            THING0,  INVISIBLE,     1, 4, 4, -1, 15, 172, 113, 108, 99,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   124, 154, Common::KEYCODE_DOWN,  0, 0, 0, 0},
{kNMachinebits_1w, 0,              0, AUTO, 41,  7,   0,            THING0,  INVISIBLE,     1, 4, 4, 0,  15, 172, 113, 184, 118, 0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   191, 168, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNMachine_1w,     0,              0, AUTO, 103, 34,  0,            THING0,  INVISIBLE,     1, 4, 4, 0,  15, 172, 113, 177, 130, 0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   240, 168, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNTable_1w,       0,              0, AUTO, 44,  11,  0,            THING0,  INVISIBLE,     1, 4, 4, 0,  15, 172, 113, 49,  137, 0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   64,  162, Common::KEYCODE_UP,    0, 0, 0, 0}
};

uint16 tmatch_2w[]  = {kDTtmatch1_2w, kDTtmatch2_2w, 0};
uint16 tguns_2w[]   = {kDTtgun0_2w,   kDTtgun1_2w,   0};
uint16 trobots_2w[] = {kDTtrobot0_2w, kDTtrobot1_2w, 0};

object_t objects_2w[]  = {
// Note new use of non-image objects:  Use old_x, old_y, dx, dy to mark rectangle
// and use dx <> 0 to allow point & click interface to access them.
// viewx: -1 Walk to object, 0 immediate use, else walk to viewx,viewy.
// name,description,description_s,path,dx,dy,aptr,seq,seqp,                  cyc,n,frm,rad,scr,x,y  ,oldxy,vxy,val,g,cmnd,c,s,ctx,fgb
{kNHero_2w,      kDTthero_2w,     0,          AUTO,    0,    0,    0,            PERSON,  INVISIBLE,     0, 0, 0,  0,  0,                319, 199, 90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNPenny_2w,     kDTtpenny_2w,    0,          AUTO,    0,    0,    0,            PERSON,  NOT_CYCLING,   0, 0, 0,  -1, 1,                109, 140, 90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 0: (Outside house)
{kNSmoke_2w,     0,               0,          AUTO,    0,    0,    0,            THING3,  CYCLE_FORWARD, 0, 2, 0,  0,  0,                233, 20,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 1: (Hall)
{kNDoor_2w,      kDTtdoor_2w,     0,          AUTO,    0,    0,    0,            THING4,  NOT_CYCLING,   1, 4, 4,  50, 1,                238, 40,  90,  90,  0, 0, 0,  1,  kCMDdoor1_2w,    0, 0, 0, FLOATING,   239, 82,  Common::KEYCODE_UP,    0, 0, 0, 0},
{kNDoor_2w,      kDTtdoordum_2w,  0,          AUTO,    26,   42,   0,            THING0,  INVISIBLE,     0, 0, 0,  50, 1,                80,  40,  80,  40,  0, 0, 0,  1,  kCMDdoordum_2w,  0, 0, 0, FLOATING,   82,  82,  Common::KEYCODE_UP,    0, 0, 0, 0},
{kNDoor_2w,      kDTtdoordum_2w,  0,          AUTO,    26,   42,   0,            THING0,  INVISIBLE,     0, 0, 0,  50, 1,                160, 40,  160, 40,  0, 0, 0,  1,  kCMDdoordum_2w,  0, 0, 0, FLOATING,   162, 82,  Common::KEYCODE_UP,    0, 0, 0, 0},
{kNLips_2w,      0,               0,          AUTO,    0,    0,    0,            THING2,  INVISIBLE,     0, 0, 0,  0,  1,                186, 100, 90,  90,  0, 0, 0,  0,  0,               0, 0, 0, FOREGROUND, 0,   0,   0,                     0, 0, 0, 0},
{kNMaid_2w,      kDTtmaid_2w,     0,          AUTO,    0,    0,    0,            PERSON4, NOT_CYCLING,   0, 0, 0,  8,  1,                149, 135, 90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNHallgo_2w,    kDTthallgo_2w,   0,          AUTO,    189,  72,   0,            THING0,  INVISIBLE,     0, 0, 0,  -1, 1,                0,   0,   116, 106, 0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   218, 181, Common::KEYCODE_UP,    0, 0, 0, 0},
// Screen 2: (Bedroom 1)
{kNPennylie_2w,  0,               0,          AUTO,    0,    0,    0,            THING1,  INVISIBLE,     0, 0, 0,  10, 2,                24,  177, 90,  90,  0, 0, 0,  1,  0,               0, 0, 0, OVEROVL,    0,   0,   0,                     0, 0, 0, 0},
{kNPenfall_2w,   0,               0,          AUTO,    0,    0,    0,            THING2,  INVISIBLE,     0, 0, 0,  10, 2,                24,  177, 90,  90,  0, 0, 0,  0,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBookcase_2w,  kDTtbookcase_2w, 0,          AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,   1, 0, 0,  -1, 2,                70,  81,  90,  90,  0, 0, 0,  1,  kCMDbook_2w,     0, 0, 0, FLOATING,   90,  138, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNBook_2w,      kDTtbook_2w,     0,          AUTO,    0,    0,    0,            THING0,  INVISIBLE,     0, 0, 0,  10, 2,                90,  140, 90,  90,  0, 0, 0,  1,  kCMDbook_2w,     0, 0, 1, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNKeyhole_2w,   0,               0,          AUTO,    7,    9,    0,            THING0,  INVISIBLE,     0, 0, 0,  10, 2,                28,  166, 26,  129, 0, 0, 0,  0,  kCMDkeyhole_2w,  0, 0, 0, FLOATING,   35,  166, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNBed_2w,       kDTtbed_2w,      0,          AUTO,    109,  27,   0,            THING0,  INVISIBLE,     0, 0, 0,  0,  2,                160, 40,  174, 135, 0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   214, 136, Common::KEYCODE_DOWN,  0, 0, 0, 0},
// Screen 3: (Bedroom 2)
{kNPanel_2w,     0,               0,          AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,   1, 0, 0,  -1, 3,                189, 91,  90,  90,  0, 0, 0,  0,  0,               0, 0, 0, FOREGROUND, 0,   0,   0,                     0, 0, 0, 0},
{kNCupb_2w,      0,               0,          AUTO,    0,    0,    0,            THING0,  INVISIBLE,     0, 0, 0,  55, 3,                135, 142, 90,  90,  0, 0, 0,  0,  kCMDlookdesk_2w, 0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBird_2w,      kDTtbird_2w,     0,          AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD, 0, 0, 0,  -1, 3,                186, 100, 90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FOREGROUND, 192, 157, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNMatches_2w,   0,               tmatch_2w,  AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,   1, 0, 0,  30, 255,              78,  30,  90,  90,  0, 0, 5,  15, kCMDmatches_2w,  0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNDumb_2w,      kDTtdumb_2w,     0,          AUTO,    32,   26,   0,            THING0,  INVISIBLE,     0, 0, 0,  30, 3,                72,  138, 55,  91,  0, 0, 0,  1,  kCMDdumb_2w,     0, 0, 0, FLOATING,   66,  140, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNPhone_2w,     0,               0,          AUTO,    16,   7,    0,            THING0,  INVISIBLE,     0, 0, 0,  0,  3,                0,   0,   102, 120, 0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   78,  148, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNBlotpad_2w,   0,               0,          AUTO,    41,   10,   0,            THING0,  INVISIBLE,     0, 0, 0,  0,  3,                0,   0,   116, 122, 0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   132, 160, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNDrawer_2w,    0,               0,          AUTO,    38,   9,    0,            THING0,  INVISIBLE,     0, 0, 0,  0,  3,                0,   0,   127, 133, 0, 0, 0,  1,  kCMDlookdesk_2w, 0, 0, 0, FLOATING,   139, 158, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNSwitch_2w,    0,               0,          AUTO,    5,    5,    0,            THING0,  INVISIBLE,     0, 0, 0,  0,  3,                0,   0,   89,  104, 0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   74,  141, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNKeyhole_2w,   0,               0,          AUTO,    7,    11,   0,            THING0,  INVISIBLE,     0, 0, 0,  0,  3,                0,   0,   282, 134, 0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   260, 170, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
// Screen 4: (Keyhole)
{kNMurder_2w,    0,               0,          AUTO,    0,    0,    0,            THING2e, CYCLE_FORWARD, 1, 2, 16, -1, 4,                141, 76,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 5: (Bed3)
{kNBalloon_2w,   kDTtballoon_2w,  0,          WANDER,  DX/2, DY/2, 0,            THING1,  NOT_CYCLING,   0, 0, 0,  50, 5,                180, 40,  90,  90,  0, 0, 0,  1,  kCMDballoon_2w,  0, 0, 0, FLOATING,   146, 130, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNBlock_2w,     0,               0,          AUTO,    18,   10,   0,            THING0,  INVISIBLE,     0, 0, 0,  0,  5,                0,   0,   175, 131, 0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   176, 135, Common::KEYCODE_DOWN,  0, 0, 0, 0},
{kNCage_2w,      0,               0,          AUTO,    46,   23,   0,            THING0,  INVISIBLE,     0, 0, 0,  0,  5,                0,   0,   93,  123, 0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   96,  150, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNWindow_2w,    0,               0,          AUTO,    15,   26,   0,            THING0,  INVISIBLE,     0, 0, 0,  0,  5,                0,   0,   83,  88,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   106, 133, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNWindow_2w,    0,               0,          AUTO,    31,   21,   0,            THING0,  INVISIBLE,     0, 0, 0,  0,  5,                0,   0,   133, 85,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   149, 133, Common::KEYCODE_UP,    0, 0, 0, 0},
// Screen 6: (Kitchen)
{kNDumb_2w,      kDTtdumb_2w,     0,          AUTO,    21,   24,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 6,                35,  152, 26,  99,  0, 0, 0,  1,  kCMDdumb_2w,     0, 0, 0, FLOATING,   40,  153, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNCupb_2w,      0,               0,          AUTO,    61,   23,   0,            THING0,  INVISIBLE,     0, 0, 0,  55, 6,                135, 142, 90,  118, 0, 0, 0,  0,  kCMDlookcupb_2w, 0, 0, 0, FLOATING,   112, 143, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNGarlic_2w,    kDTtgarlic_2w,   0,          AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,   0, 0, 0,  20, 255,              78,  30,  90,  90,  0, 0, 5,  7,  kCMDgarlic_2w,   0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNDoor_2w,      kDTdull_2w,      0,          AUTO,    0,    0,    0,            THING0,  INVISIBLE,     0, 0, 0,  55, 6,                290, 196, 90,  90,  0, 0, 0,  0,  kCMDkdoor_2w,    0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNWindow_2w,    0,               0,          AUTO,    67,   29,   0,            THING0,  INVISIBLE,     0, 0, 0,  0,  6,                0,   0,   106, 75,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   139, 143, Common::KEYCODE_UP,    0, 0, 0, 0},
// Screen 8: (Shed)
{kNGardner_2w,   0,               0,          WANDER,  DX,   DY,   0,            PERSON2, CYCLE_FORWARD, 0, 0, 0,  -1, 8,                250, 90,  90,  90,  0, 0, 0,  1,  kCMDlookgard_2w, 0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 9: In shed
{kNRed_2w,       kDTtbutton_2w,   0,          AUTO,    5,    5,    0,            THING0,  INVISIBLE,     0, 0, 0,  30, 9,                190, 137, 177, 99,  0, 0, 0,  1,  kCMDred_2w,      0, 0, 0, FLOATING,   188, 137, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNYellow_2w,    kDTtbutton_2w,   0,          AUTO,    5,    5,    0,            THING0,  INVISIBLE,     0, 0, 0,  30, 9,                190, 137, 189, 99,  0, 0, 0,  1,  kCMDyellow_2w,   0, 0, 0, FLOATING,   172, 137, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNGreen_2w,     kDTtbutton_2w,   0,          AUTO,    5,    5,    0,            THING0,  INVISIBLE,     0, 0, 0,  30, 9,                190, 137, 201, 99,  0, 0, 0,  1,  kCMDgreen_2w,    0, 0, 0, FLOATING,   184, 137, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNBlue_2w,      kDTtbutton_2w,   0,          AUTO,    5,    5,    0,            THING0,  INVISIBLE,     0, 0, 0,  30, 9,                190, 137, 215, 99,  0, 0, 0,  1,  kCMDblue_2w,     0, 0, 0, FLOATING,   198, 137, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNButton_2w,    kDTtbutton_2w,   0,          AUTO,    0,    0,    0,            THING0,  INVISIBLE,     0, 0, 0,  30, 9,                190, 137, 90,  90,  0, 0, 0,  1,  kCMDbutton_2w,   0, 1, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNShedlight_2w, kDTtslight_2w,   0,          AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,   1, 0, 0,  -1, 9,                161, 48,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNTools_2w,     0,               0,          AUTO,    33,   21,   0,            THING0,  INVISIBLE,     0, 0, 0,  30, 9,                190, 137, 127, 79,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   136, 127, Common::KEYCODE_UP,    0, 0, 0, 0},
// Screen 10: Venus fly traps
{kNMagnify_2w,   kDTtmagnify_2w,  0,          AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,   0, 0, 0,  20, 10,               95,  96,  90,  90,  0, 0, 15, 7,  0,               0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNFly_2w,       0,               0,          WANDER2, DX,   DY,   0,            THING2,  NOT_CYCLING,   0, 1, 0,  20, 10,               48,  60,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNFly_2w,       0,               0,          WANDER2, DX,   DY,   0,            THING2,  NOT_CYCLING,   0, 1, 0,  20, 10,               58,  70,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNFly_2w,       0,               0,          WANDER2, DX,   DY,   0,            THING2,  NOT_CYCLING,   0, 1, 0,  20, 10,               268, 90,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNLeaf_2w,      0,               0,          AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD, 0, 1, 0,  40, 10,               48,  86,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   41,  103, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNLeaf_2w,      0,               0,          AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD, 0, 2, 0,  40, 10,               79,  104, 90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   69,  121, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNLeaf_2w,      0,               0,          AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD, 0, 1, 0,  40, 10,               71,  141, 90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   68,  153, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNLeaf_2w,      0,               0,          AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD, 0, 3, 0,  40, 10,               116, 113, 90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   99,  132, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNLeaf_2w,      0,               0,          AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD, 0, 1, 1,  40, 10,               164, 120, 90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   184, 136, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNLeaf_2w,      0,               0,          AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD, 0, 2, 0,  40, 10,               185, 83,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   167, 101, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNLeaf_2w,      0,               0,          AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD, 0, 1, 0,  40, 10,               232, 96,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   223, 116, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNLeaf_2w,      0,               0,          AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD, 0, 1, 0,  40, 10,               273, 141, 90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   251, 156, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
// Screen 11/12: Gates
{kNGatelight_2w, kDTtglight_2w,   0,          AUTO,    0,    0,    0,            THING1,  INVISIBLE,     1, 0, 0,  -1, 11,               90,  72,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNGate_2w,      0,               0,          AUTO,    83,   56,   0,            THING0,  INVISIBLE,     0, 0, 0,  -1, 12,               190, 137, 118, 99,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   152, 157, Common::KEYCODE_UP,    0, 0, 0, 0},
// Screen 13: Stream
{kNCatnip_2w,    kDTtcatnip_2w,   0,          AUTO,    54,   8,    0,            THING0,  INVISIBLE,     0, 0, 0,  -1, 13,               211, 136, 29,  114, 0, 0, 5,  3,  kCMDcatnip_2w,   0, 0, 0, FLOATING,   49,  130, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNBridge_2w,    kDTtbridge_2w,   0,          AUTO,    61,   25,   0,            THING0,  INVISIBLE,     0, 0, 0,  -1, 13,               211, 136, 162, 116, 0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   218, 124, Common::KEYCODE_DOWN,  0, 0, 0, 0},
// Screen 14: Zapper
{kNZapper_2w,    kDTtzapper_2w,   0,          AUTO,    0,    0,    0,            THING1,  INVISIBLE,     1, 0, 0,  -1, 14,               134, 46,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBug_2w,       kDTtbug_2w,      0,          AUTO,    0,    0,    kALbugs_2w,   THING2,  CYCLE_FORWARD, 0, 0, 0,  16, 14,               65,  25,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBug_2w,       kDTtbug_2w,      0,          AUTO,    0,    0,    kALbugs_2w,   THING2,  CYCLE_FORWARD, 0, 0, 0,  16, 14,               65,  25,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBug_2w,       kDTtbug_2w,      0,          AUTO,    0,    0,    kALbugs_2w,   THING2,  CYCLE_FORWARD, 0, 0, 0,  16, 14,               65,  25,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBug_2w,       kDTtbug_2w,      0,          AUTO,    0,    0,    kALbugs_2w,   THING2,  CYCLE_FORWARD, 0, 0, 0,  16, 14,               65,  25,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBug_2w,       kDTtbug_2w,      0,          AUTO,    0,    0,    kALbugs_2w,   THING2,  CYCLE_FORWARD, 0, 0, 0,  16, 14,               65,  25,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 15: Mushroom
{kNOldman_2w,    kDTtoldman_2w,   0,          AUTO,    0,    0,    0,            THING2c, NOT_CYCLING,   1, 0, 0,  -1, 15,               126, 77,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 16: Well
{kNWell_2w,      kDTtwell_2w,     0,          AUTO,    53,   55,   0,            THING0,  INVISIBLE,     0, 0, 0,  30, 16,               211, 136, 184, 85,  0, 0, 0,  1,  kCMDwell_2w,     0, 0, 0, FLOATING,   242, 131, Common::KEYCODE_LEFT,  0, 0, 0, 0},
// Screen 17: Snakepit
{kNSnake_2w,     kDTtsnake_2w,    0,          CHASE2,  DX,   DY,   kALsnake_2w,  PERSON2, CYCLE_FORWARD, 0, 0, 0,  25, 17,               165, 95,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
// Screen 18: Phonebox
{kNTardis_2w,    kDTttardis_2w,   0,          AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,   0, 0, 0,  50, 18,               21,  74,  90,  90,  0, 0, 0,  1,  kCMDtardis_2w,   0, 0, 1, FLOATING,   50,  126, Common::KEYCODE_LEFT,  0, 0, 0, 0},
// Screen 19: Street
{kNGraf_2w,      0,               0,          AUTO,    34,   14,   0,            THING0,  INVISIBLE,     0, 0, 0,  50, 19,               21,  74,  134, 102, 0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   143, 134, Common::KEYCODE_UP,    0, 0, 0, 0},
// Screen 20: Kennel
{kNStick_2w,     kDTdull_2w,      0,          AUTO,    29,   11,   0,            THING0,  INVISIBLE,     0, 0, 0,  30, 20,               89,  120, 83,  116, 0, 0, 5,  3,  kCMDstick_2w,    0, 0, 0, FLOATING,   88,  132, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNDynamite_2w,  kDTtdynamite_2w, 0,          AUTO,    0,    0,    0,            THING2a, INVISIBLE,     0, 0, 0,  40, 20,               195, 111, 90,  90,  0, 0, 0,  7,  kCMDdynamite_2w, 0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNKennel_2w,    0,               0,          AUTO,    97,   40,   0,            THING0,  INVISIBLE,     0, 0, 0,  50, 20,               195, 114, 173, 73,  0, 0, 0,  0,  kCMDkennel_2w,   0, 0, 0, FLOATING,   160, 113, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNDog_2w,       kDTtdog_2w,      0,          AUTO,    0,    0,    0,            THING2f, CYCLE_FORWARD, 0, 2, 0,  30, 20,               184, 80,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   200, 117, Common::KEYCODE_LEFT,  0, 0, 0, 0},
// Screen 21: (Rockroom)
{kNRock_2w,      0,               0,          AUTO,    60,   36,   0,            THING0,  INVISIBLE,     0, 0, 0,  30, 21,               89,  120, 216, 63,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   254, 102, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNRope_2w,      kDTtrope_2w,     0,          AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,   0, 0, 0,  60, 21,               78,  30,  90,  90,  0, 0, 0,  1,  kCMDrope_2w,     0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
// Screen 22: (Rockgone)
{kNRope_2w,      kDTtrope_2w,     0,          AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,   0, 0, 0,  60, 22,               78,  30,  90,  90,  0, 0, 0,  1,  kCMDrope_2w,     0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
// Screen 24: (Lampcave)
{kNLamp_2w,      kDTtlamp_2w,     0,          AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,   0, 0, 0,  30, 24,               78,  115, 90,  90,  0, 0, 10, 7,  kCMDlamp_2w,     0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
// Screen 25: (Chasm)
{kNBanana_2w,    kDTtbanana_2w,   0,          AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,   0, 0, 0,  20, 25,               254, 107, 90,  90,  0, 0, 5,  7,  kCMDbanana_2w,   0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNHole_2w,      0,               0,          AUTO,    40,   80,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 25,               254, 107, 183, 92,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   187, 123, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
// Screen 27: (Ladder)
{kNGenie_2w,     kDTtgenie_2w,    0,          AUTO,    0,    0,    0,            PERSON3, INVISIBLE,     0, 0, 0,  40, 27,               138, 70,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
// Screen 28: (Traproom)
{kNSafe_2w,      kDTtsafe_2w,     0,          AUTO,    37,   34,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 28,               122, 144, 104, 110, 0, 0, 0,  1,  kCMDsafe_2w,     0, 0, 0, FLOATING,   106, 145, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNWill_2w,      kDTtwill_2w,     0,          AUTO,    0,    0,    0,            THING0,  INVISIBLE,     0, 0, 0,  20, 28,               122, 144, 90,  90,  0, 0, 5,  7,  kCMDwill_2w,     0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNMousehole_2w, 0,               0,          AUTO,    5,    5,    0,            THING0,  INVISIBLE,     0, 0, 0,  20, 28,               122, 144, 164, 132, 0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   153, 146, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNWindow_2w,    0,               0,          AUTO,    15,   30,   0,            THING0,  INVISIBLE,     0, 0, 0,  -1, 28,               122, 144, 250, 89,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   222, 141, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
// Screen 30: (Lounge)
{kNPicture_2w,   0,               0,          AUTO,    35,   31,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 30,               122, 144, 262, 90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNWindow_2w,    0,               0,          AUTO,    55,   55,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 30,               122, 144, 163, 86,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   186, 144, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNWindow_2w,    0,               0,          AUTO,    52,   24,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 30,               122, 144, 89,  86,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   93,  142, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNWindow_2w,    0,               0,          AUTO,    55,   24,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 30,               122, 144, 23,  89,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   60,  150, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNPlant_2w,     0,               0,          AUTO,    19,   51,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 30,               122, 144, 57,  93,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   70,  146, Common::KEYCODE_UP,    0, 0, 0, 0},
// Screen 31: (Parlor)
{kNCupb_2w,      0,               0,          AUTO,    64,   28,   0,            THING0,  INVISIBLE,     0, 0, 0,  55, 31,               212, 142, 182, 113, 0, 0, 0,  0,  kCMDcupbp_2w,    0, 0, 0, FLOATING,   200, 150, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNDoor_2w,      kDTdull_2w,      0,          AUTO,    0,    0,    0,            THING0,  INVISIBLE,     0, 0, 0,  55, 31,               28,  154, 90,  90,  0, 0, 0,  0,  kCMDpdoor_2w,    0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNAlbum_2w,     kDTtalbum_2w,    0,          AUTO,    0,    0,    0,            THING0,  INVISIBLE,     0, 0, 0,  55, 255,              212, 142, 90,  90,  0, 0, 5,  1,  kCMDalbum_2w,    0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 32: (Catroom)
{kNCat_2w,       kDTtcat_2w,      0,          AUTO,    0,    0,    0,            THING2b, CYCLE_FORWARD, 0, 2, 0,  40, 32,               189, 69,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   171, 117, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNWindow_2w,    0,               0,          AUTO,    27,   24,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 32,               122, 144, 106, 68,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   113, 118, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNWindow_2w,    0,               0,          AUTO,    26,   24,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 32,               122, 144, 179, 68,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   189, 118, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNPost_2w,      0,               0,          AUTO,    29,   32,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 32,               122, 144, 195, 100, 0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   195, 128, Common::KEYCODE_DOWN,  0, 0, 0, 0},
// Screen 33: (Boxroom)
{kNDoor_2w,      kDTtbdoor_2w,    0,          AUTO,    0,    0,    0,            THING4,  NOT_CYCLING,   1, 4, 4,  16, 33,               137, 97,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   140, 142, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNPaper_2w,     0,               0,          AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,   1, 0, 0,  80, 33,               205, 147, 90,  90,  0, 0, 5,  7,  kCMDpaper_2w,    0, 0, 0, BACKGROUND, 188, 159, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNPencil_2w,    kDTtpencil_2w,   0,          AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,   1, 0, 0,  40, 33,               205, 163, 90,  90,  0, 0, 5,  7,  kCMDpencil_2w,   0, 0, 0, FLOATING,   188, 168, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNChute_2w,     0,               0,          AUTO,    12,   21,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 33,               122, 144, 73,  107, 0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   114, 149, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNCrate_2w,     0,               0,          AUTO,    48,   28,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 33,               122, 144, 68,  133, 0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   114, 149, Common::KEYCODE_LEFT,  0, 0, 0, 0},
// Screen 34: (Hall3)
{kNDoor_2w,      kDTtdoor_2w,     0,          AUTO,    0,    0,    0,            THING4,  NOT_CYCLING,   1, 4, 4,  16, 34,               234, 73,  90,  90,  0, 0, 0,  1,  kCMDdoor2_2w,    0, 0, 0, FLOATING,   240, 121, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNDoor_2w,      kDTtdoor_2w,     0,          AUTO,    0,    0,    0,            THING4,  NOT_CYCLING,   1, 4, 4,  16, 34,               103, 73,  90,  90,  0, 0, 0,  1,  kCMDdoor3_2w,    0, 0, 0, FLOATING,   107, 121, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNDoor_2w,      kDTtdoordum_2w,  0,          AUTO,    16,   56,   0,            THING0,  INVISIBLE,     0, 0, 0,  50, 34,               80,  40,  73,  92,  0, 0, 0,  1,  kCMDdoordum_2w,  0, 0, 0, FLOATING,   90,  148, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNMirror_2w,    0,               0,          AUTO,    20,   33,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 34,               122, 144, 172, 75,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   175, 125, Common::KEYCODE_UP,    0, 0, 0, 0},
// Screen 35: (Organ)
{kNHarry_2w,     kDTtharry_2w,    0,          AUTO,    DX,   DY,   0,            THING2f, CYCLE_FORWARD, 0, 2, 0,  -1, 35,               188, 84,  90,  90,  0, 0, 0,  1,  kCMDharry_2w,    0, 0, 0, FLOATING,   216, 132, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNWindow_2w,    0,               0,          AUTO,    33,   33,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 35,               122, 144, 248, 72,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   251, 134, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNPicture_2w,   0,               0,          AUTO,    33,   21,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 35,               122, 144, 95,  78,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNPlant_2w,     0,               0,          AUTO,    23,   57,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 35,               122, 144, 52,  83,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   83,  138, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNOrgan_2w,     0,               0,          AUTO,    65,   66,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 35,               122, 144, 162, 62,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 36: (Hestroom)
{kNHester_2w,    kDTthester_2w,   0,          AUTO,    0,    0,    0,            PERSON,  CYCLE_FORWARD, 0, 0, 0,  30, 36,               78,  114, 90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   70,  150, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNLetter_2w,    kDTtletter_2w,   0,          AUTO,    19,   6,    0,            THING0,  INVISIBLE,     0, 0, 0,  30, 36,               110, 150, 106, 132, 0, 0, 0,  1,  kCMDletter_2w,   0, 0, 0, FLOATING,   98,  151, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNBookcase_2w,  0,               0,          AUTO,    69,   62,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 36,               122, 144, 179, 81,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   201, 146, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNPicture_2w,   0,               0,          AUTO,    23,   24,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 36,               122, 144, 271, 91,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   237, 145, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNPlant_2w,     0,               0,          AUTO,    12,   21,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 36,               122, 144, 263, 111, 0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   237, 145, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNWindow_2w,    0,               0,          AUTO,    24,   23,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 36,               122, 144, 25,  94,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   40,  154, Common::KEYCODE_LEFT,  0, 0, 0, 0},
// Screen 37: (Retupmoc)
{kNDoctor_2w,    kDTtdoctor_2w,   0,          WANDER,  DX,   DY,   kALdoctor_2w, PERSON,  CYCLE_FORWARD, 0, 0, 0,  -1, 37,               78,  114, 90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNRobot_2w,     0,               trobots_2w, CHASE2,  DX,   DY,   kALrobot_2w,  PERSON5, NOT_CYCLING,   0, 0, 0,  -1, 37,               78,  114, 90,  90,  0, 0, 0,  9,  kCMDrobot_2w,    0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNScrew_2w,     kDTtscrew_2w,    0,          AUTO,    0,    0,    0,            THING0,  INVISIBLE,     0, 0, 0,  20, 37,               100, 123, 90,  90,  0, 0, 15, 3,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 38: (Hall1 revisited)
{kNDoor_2w,      kDTtdoordum_2w,  0,          AUTO,    0,    0,    0,            THING4,  NOT_CYCLING,   1, 4, 4,  50, 38,               238, 40,  90,  90,  0, 0, 0,  1,  kCMDdoordum_2w,  0, 0, 0, FLOATING,   239, 82,  Common::KEYCODE_UP,    0, 0, 0, 0},
{kNDoor_2w,      kDTtdoordum_2w,  0,          AUTO,    26,   42,   0,            THING0,  INVISIBLE,     0, 0, 0,  50, 38,               80,  40,  80,  40,  0, 0, 0,  1,  kCMDdoordum_2w,  0, 0, 0, FLOATING,   82,  82,  Common::KEYCODE_UP,    0, 0, 0, 0},
{kNDoor_2w,      kDTtdoordum_2w,  0,          AUTO,    26,   42,   0,            THING0,  INVISIBLE,     0, 0, 0,  50, 38,               160, 40,  160, 40,  0, 0, 0,  1,  kCMDdoordum_2w,  0, 0, 0, FLOATING,   162, 82,  Common::KEYCODE_UP,    0, 0, 0, 0},
{kNPicture_2w,   0,               0,          AUTO,    32,   18,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 38,               122, 144, 187, 113, 0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   200, 158, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNPlant_2w,     0,               0,          AUTO,    25,   43,   0,            THING0,  INVISIBLE,     0, 0, 0,  20, 38,               122, 144, 239, 111, 0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   219, 157, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
// Misc:
{kNCook_2w,      kDTtcook_2w,     0,          AUTO,    DX,   DY,   0,            PERSON,  INVISIBLE,     0, 0, 0,  30, 06,               98,  98,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNCookb_2w,     kDTtcook_2w,     0,          AUTO,    0,    0,    0,            THING4,  CYCLE_FORWARD, 0, 0, 0,  30, 255,              98,  98,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNCop_2w,       kDTtcop_2w,      0,          AUTO,    0,    0,    0,            PERSON2, INVISIBLE,     0, 0, 0,  30, 29,               180, 47,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNHorace_2w,    kDTthorace_2w,   0,          AUTO,    0,    0,    0,            PERSON,  INVISIBLE,     0, 0, 0,  30, 34,               215, 76,  90,  90,  0, 0, 0,  1,  0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBell_2w,      kDTtbell_2w,     0,          AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,   0, 0, 0,  30, MAZE_SCREEN + 15, 149, 109, 90,  90,  0, 0, 5,  7,  kCMDbell_2w,     0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNGun_2w,       0,               tguns_2w,   AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,   0, 0, 0,  30, MAZE_SCREEN + 26, 149, 109, 90,  90,  0, 0, 10, 15, kCMDgun_2w,      0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNBottle_2w,    kDTtbottle_2w,   0,          AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,   0, 0, 0,  30, MAZE_SCREEN + 27, 149, 109, 90,  90,  0, 0, 15, 7,  kCMDbottle_2w,   0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
};

uint16 tflask_3w[] = {kDTtflask1_3w, kDTtflask2_3w, kDTtflask3_3w, 0};// Descriptions depend on flask state
uint16 tcage_3w[]  = {kDTtcage1_3w,  kDTtcage2_3w,  0};

// The objects data base
// Note new use of non-image objects:  Use old_x, old_y, dx, dy to mark rectangle
// and use dx <> 0 to allow point & click interface to access them.
object_t objects_3w[] = {
//name,description,description_s,path,dx,dy,aptr,SPRITE, cyc, n,frm,rad,      scr,x,y  ,oldxy,vxy,val,g,cmnd,c,s,ctx,fgb,   viewx,viewy,dir
// Common objects  // Set Penny state to 3 to avoid story
{kNHero_3w,      kDTthero_3w,     0,         USER,   0,   0,   0,            PERSON,  CYCLE_FORWARD, 0, 0,  0, 0,  0,           161, 110, 90,  90,  0,  0,  0,  1,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNWhero_3w,     kDTthero_3w,     0,         USER,   0,   0,   0,            PERSON,  INVISIBLE,     0, 0,  0, 0,  0,           219, 133, 90,  90,  0,  0,  0,  1,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
//#if STORY
{kNPenny_3w,     kDTtpenny_3w,    0,         AUTO,   0,   0,   0,            PERSON,  CYCLE_FORWARD, 0, 0,  0, -1, 0,           109, 110, 90,  90,  0,  0,  0,  1,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
//#else
//{kNPenny_3w,     kDTtpenny_3w,    0,         AUTO,   0,   0,   0,            PERSON,  CYCLE_FORWARD, 0, 0,  0, -1, 0,           109, 110, 90,  90,  0,  0,  0,  1,  0,                0, 3, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
//#endif
{kNPennylie_3w,  kDTtplie_3w,     0,         AUTO,   0,   0,   0,            THING1,  INVISIBLE,     0, 0,  0, -1, WEB_3w,      75,  156, 90,  90,  0,  0,  0,  1,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNLips_3w,      0,               0,         AUTO,   0,   0,   0,            THING2,  INVISIBLE,     0, 0,  0, 0,  0,           186, 100, 90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FOREGROUND, 0,      0,   0,                     0, 0, 0, 0},
// CRASH site
{kNPlane_3w,     kDTtplane_3w,    0,         AUTO,   0,   0,   0,            THING0,  INVISIBLE,     0, 0,  0, -1, CRASH_3w,    184, 136, 90,  90,  0,  0,  0,  1,  kCMDcplane_3w,    0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNDoor_3w,      kDTdull_3w,      0,         AUTO,   0,   0,   0,            THING0,  INVISIBLE,     0, 0,  0, 80, CRASH_3w,    184, 136, 90,  90,  0,  0,  0,  1,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNPlant1_3w,    0,               0,         AUTO,   0,   0,   0,            THING1,  INVISIBLE,     0, 0,  0, 0,  CRASH_3w,    132, 165, 90,  90,  0,  0,  0,  0,  0,                0, 0, 0, OVEROVL,    0,      0,   0,                     0, 0, 0, 0},
// INPLANE
{kNPlane_3w,     0,               0,         AUTO,   0,   0,   0,            THING0,  INVISIBLE,     0, 0,  0, -1, PLANE_3w,    184, 136, 90,  90,  0,  0,  0,  0,  kCMDcexit_3w,     0, 0, 1, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNWater_3w,     kDTtwwater_3w,   0,         AUTO,   195, 10,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, WFALL_3w,    184, 136, 0,   155, 0,  0,  0,  0,  kCMDcwfall_3w,    0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNWaterfall_3w, kDTtwwater_3w,   0,         AUTO,   57,  131, 0,            THING0,  INVISIBLE,     0, 0,  0, -1, WFALL_3w,    184, 136, 239, 22,  0,  0,  0,  0,  kCMDcwfall_3w,    0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNWater_3w,     0,               0,         AUTO,   0,   0,   0,            THING0,  INVISIBLE,     0, 0,  0, -1, WFALL_B_3w,  184, 136, 90,  90,  0,  0,  0,  0,  kCMDcwfall_3w,    0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNWater_3w,     kDTtswater_3w,   0,         AUTO,   106, 19,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, STREAM_3w,   184, 136, 102, 150, 0,  0,  0,  0,  kCMDcwstream_3w,  0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNWater_3w,     kDTtswater_3w,   0,         AUTO,   195, 9,   0,            THING0,  INVISIBLE,     0, 0,  0, -1, STREAM_3w,   184, 136, 55,  183, 0,  0,  0,  0,  kCMDcwstream_3w,  0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNPool_3w,      kDTtmwater_3w,   0,         AUTO,   87,  34,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, GARDEN_3w,   184, 136, 112, 129, 0,  0,  0,  0,  kCMDcwpool_3w,    0, 0, 0, FLOATING,   155,    170, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNClay_3w,      kDTtclay_3w,     0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, PLANE_3w,    162, 96,  90,  90,  0,  0,  5,  7,  kCMDcclay_3w,     0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNNeedles_3w,   kDTtneedles_3w,  0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, PLANE_3w,    172, 90,  90,  90,  0,  0,  5,  7,  kCMDcpins_3w,     0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNFlask_3w,     0,               tflask_3w, AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, PLANE_3w,    190, 90,  90,  90,  0,  0,  5,  11, kCMDcflask_3w,    0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNBouillon_3w,  kDTtbouillon_3w, 0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, PLANE_3w,    185, 94,  90,  90,  0,  0,  5,  7,  kCMDcbouillon_3w, 0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNCheese_3w,    kDTtcheese_3w,   0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, PLANE_3w,    185, 100, 90,  90,  0,  0,  5,  1,  kCMDccheese_3w,   0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
// WEB
{kNSpider_3w,    kDTtspider_3w,   0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, -1, WEB_3w,      77,  50,  90,  90,  0,  0,  0,  1,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNPlant3_3w,    0,               0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, 0,  WEB_3w,      245, 117, 90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNPlant4_3w,    0,               0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, 0,  WEB_3w,      285, 90,  91,  90,  0,  0,  0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
// BRIDGE
{kNBlock_3w,     0,               0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, BRIDGE_3w,   225, 132, 90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNBlock_3w,     0,               0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, BRIDGE_3w,   225, 133, 90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNBlock_3w,     0,               0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, BRIDGE_3w,   225, 134, 90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNBlock_3w,     0,               0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, BRIDGE_3w,   225, 135, 90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNVine_3w,      kDTtvine_3w,     0,         AUTO,   8,   76,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, BRIDGE_3w,   184, 136, 228, 24,  0,  0,  0,  1,  kCMDcvine_3w,     0, 0, 0, FLOATING,   237,    131, Common::KEYCODE_UP,    0, 0, 0, 0},
// STREAM
{kNVine_3w,      kDTtvine_3w,     0,         AUTO,   6,   50,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, STREAM_3w,   184, 136, 101, 96,  0,  0,  0,  1,  kCMDcswing_3w,    0, 0, 0, FLOATING,   50,     172, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNVine_3w,      kDTtvine_3w,     0,         AUTO,   6,   50,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, STREAM_3w,   184, 136, 203, 96,  0,  0,  0,  1,  kCMDcswing_3w,    0, 0, 0, FLOATING,   233,    170, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNSwinger_3w,   0,               0,         AUTO,   0,   0,   0,            PERSON2, INVISIBLE,     0, 0,  0, 0,  STREAM_3w,   219, 133, 90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNSteps_3w,     kDTtsteps_3w,    0,         AUTO,   153, 15,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, STREAM_3w,   184, 136, 79,  168, 0,  0,  0,  1,  kCMDcsteps_3w,    0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
// HUT_IN
{kNDoctor_3w,    kDTtdoctor_3w,   0,         WANDER, DX,  DY,  kALdocgot_3w, PERSON3, CYCLE_FORWARD, 0, 0,  0, -1, CAMP_3w,     273, 83,  90,  90,  0,  0,  0,  1,  kCMDcdoctor_3w,   0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNDoclie_3w,    0,               0,         AUTO,   0,   0,   0,            THING2,  INVISIBLE,     0, 0,  0, 30, HUT_IN_3w,   239, 103, 90,  90,  0,  0,  0,  1,  kCMDcdoctor_3w,   0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNCdoor_3w,     kDTdull_3w,      0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, 40, HUT_IN_3w,   239, 103, 90,  90,  0,  0,  0,  1,  kCMDcdoor_3w,     0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNMouse_3w,     kDTtmouse_3w,    0,         AUTO,   0,   0,   0,            THING2b, INVISIBLE,     0, 0,  0, 30, HUT_IN_3w,   186, 170, 90,  90,  0,  0,  0,  1,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNMoushole_3w,  0,               0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, HUT_IN_3w,   203, 122, 90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNCage_3w,      0,               tcage_3w,  AUTO,   0,   0,   0,            THING2a, NOT_CYCLING,   0, 0,  0, 30, HUT_IN_3w,   156, 121, 90,  90,  0,  0,  1,  13, kCMDccage_3w,     0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNFire_1_3w,    0,               0,         AUTO,   0,   0,   0,            THING2,  CYCLE_FORWARD, 0, 0,  0, 0,  HUT_OUT_3w,  127, 100, 90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNFire_1_3w,    0,               0,         AUTO,   0,   0,   0,            THING2,  CYCLE_FORWARD, 0, 0,  0, 0,  HUT_OUT_3w,  172, 100, 90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNFire_2_3w,    0,               0,         AUTO,   0,   0,   0,            THING2,  CYCLE_FORWARD, 0, 0,  0, 0,  HUT_IN_3w,   30,  145, 90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNFire_3_3w,    0,               0,         AUTO,   0,   0,   0,            THING2,  CYCLE_FORWARD, 0, 0,  0, 0,  CAMP_3w,     120, 135, 90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNFire_3w,      0,               0,         AUTO,   42,  54,  0,            THING0,  INVISIBLE,     0, 0,  0, 50, HUT_IN_3w,   50,  136, 30,  121, 0,  0,  0,  1,  0,                0, 0, 0, FLOATING,   44,     153, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNWindow_3w,    0,               0,         AUTO,   39,  32,  0,            THING0,  INVISIBLE,     0, 0,  0, 50, HUT_IN_3w,   184, 136, 101, 72,  0,  0,  0,  1,  0,                0, 0, 0, FLOATING,   111,    148, Common::KEYCODE_UP,    0, 0, 0, 0},
// CAMP
{kNNat1_3w,      kDTtnative_3w,   0,         WANDER, DX,  0,   0,            PERSON,  NOT_CYCLING,   0, 0,  0, -1, CAMP_3w,     130, 105, 90,  90,  0,  0,  0,  1,  kCMDcnative_3w,   0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNNat2_3w,      kDTtnative_3w,   0,         AUTO,   DX,  DY,  0,            PERSON,  CYCLE_FORWARD, 0, 0,  0, -1, CAMP_3w,     17,  97,  90,  90,  4,  0,  0,  1,  kCMDcnative_3w,   0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNNat3_3w,      kDTtnative_3w,   0,         AUTO,   DX,  DY,  0,            THING2,  CYCLE_FORWARD, 0, 16, 0, -1, CAMP_3w,     96,  40,  90,  90,  0,  0,  0,  1,  kCMDcnative_3w,   0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNNatb_3w,      kDTtnative_3w,   0,         AUTO,   DX,  DY,  0,            THING2,  CYCLE_FORWARD, 0, 20, 0, -1, CAMP_3w,     72,  51,  90,  90,  0,  0,  0,  1,  kCMDcnative_3w,   0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNNatg_3w,      kDTtnatgirl_3w,  0,         AUTO,   DX,  DY,  kALnative_3w, PERSON,  CYCLE_FORWARD, 0, 0,  0, -1, CAMP_3w,     28,  101, 90,  90,  3,  0,  0,  1,  kCMDcnative_3w,   0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNPipe_3w,      kDTtpipe_3w,     0,         AUTO,   0,   0,   0,            THING1,  INVISIBLE,     0, 0,  0, 30, CAMP_3w,     225, 135, 90,  90,  0,  0,  0,  7,  kCMDcdart_3w,     0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNHut_3w,       0,               0,         AUTO,   47,  64,  0,            THING0,  INVISIBLE,     0, 0,  0, 50, CAMP_3w,     184, 136, 0,   42,  0,  0,  0,  1,  0,                0, 0, 0, FLOATING,   42,     92,  Common::KEYCODE_LEFT,  0, 0, 0, 0},
// PATH_2
{kNElephant_3w,  kDTtelephant_3w, 0,         AUTO,   0,   0,   0,            THING2a, NOT_CYCLING,   0, 4,  0, -1, PATH_3w,     163, 85,  90,  90,  0,  0,  0,  1,  kCMDcelephant_3w, 0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNE_eyes_3w,    0,               0,         AUTO,   0,   0,   0,            THING2c, NOT_CYCLING,   0, 0,  0, -1, PATH_3w,     194, 102, 90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FOREGROUND, 0,      0,   0,                     0, 0, 0, 0},
// Misc
{kNHero_old_3w,  0,               0,         USER,   0,   0,   0,            PERSON,  INVISIBLE,     0, 0,  0,  0, 0,           161, 120, 90,  90,  0,  0,  0,  1,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNAircraft_3w,  0,               0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, -1, SUNSET_3w,   250, 116, 90,  90,  -2, -1, 0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNScroll_3w,    kDTtscroll_3w,   0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, CLIFFTOP_3w, 75,  116, 90,  90,  0,  0,  3,  7,  kCMDcscroll_3w,   0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNCrystal_3w,   kDTtcrystal_3w,  0,         AUTO,   0,   0,   0,            THING1,  INVISIBLE,     0, 0,  0, 30, TURN_3w,     275, 116, 90,  90,  0,  0,  9,  7,  kCMDccrystal_3w,  0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNRock_3w,      kDTtrock_3w,     0,         AUTO,   93,  55,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, TURN_3w,     100, 100, 220, 120, 0,  0,  0,  0,  kCMDcrock_3w,     0, 0, 0, FLOATING,   220,    169, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
// CAVE etc.
{kNPlant2_3w,    0,               0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0,  0, CAVE_3w,     27,  160, 90,  90,  0,  0,  0,  0,  0,                0, 0, 0, OVEROVL,    0,      0,   0,                     0, 0, 0, 0},
{kNGhost_3w,     kDTtghost_3w,    0,         CHASE,  DX,  DY,  0,            THING2c, NOT_CYCLING,   0, 0,  0, -1, CAVE_3w,     121, 86,  90,  90,  0,  0,  0,  1,  kCMDcghost_3w,    0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNBell_3w,      kDTtbell_3w,     0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, CLIFF_3w,    202, 152, 90,  90,  0,  0,  2,  7,  kCMDcbell_3w,     0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNBook_3w,      kDTtbook_3w,     0,         AUTO,   0,   0,   0,            THING1,  INVISIBLE,     0, 0,  0, 30, STREAM_3w,   275, 116, 90,  90,  0,  0,  10, 7,  kCMDcbook_3w,     0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNCandle_3w,    kDTtcandle_3w,   0,         AUTO,   0,   0,   0,            THING2,  CYCLE_FORWARD, 0, 20, 0, 30, HUT_IN_3w,   77,  134, 90,  90,  0,  0,  3,  7,  kCMDccandle_3w,   0, 1, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNVine_3w,      kDTtvine_3w,     0,         AUTO,   5,   53,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, CAVE_3w,     184, 136, 116, 80,  0,  0,  0,  1,  kCMDcswingc_3w,   0, 0, 0, FLOATING,   114,    139, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNRush_3w,      kDTtrush_3w,     0,         AUTO,   40,  18,  0,            THING0,  INVISIBLE,     0, 0,  0, 60, STREAM_3w,   240, 160, 231, 144, 0,  0,  0,  1,  kCMDcrush_3w,     0, 0, 0, FLOATING,   240,    160, Common::KEYCODE_DOWN,  0, 0, 0, 0},
{kNRush_3w,      kDTtrush_3w,     0,         AUTO,   40,  18,  0,            THING0,  INVISIBLE,     0, 0,  0, 60, STREAM2_3w,  240, 160, 231, 144, 0,  0,  0,  1,  kCMDcrush_3w,     0, 0, 0, FLOATING,   240,    160, Common::KEYCODE_DOWN,  0, 0, 0, 0},
// OLDMAN (inside cave)
{kNO_eye_3w,     0,               0,         AUTO,   0,   0,   0,            THING3,  INVISIBLE,     0, 0,  0, 0,  OLDMAN_3w,   237, 77,  90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNFire_4_3w,    0,               0,         AUTO,   0,   0,   0,            THING2,  CYCLE_FORWARD, 0, 0,  0, 0,  OLDMAN_3w,   65,  56,  90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNMouth_3w,     0,               0,         AUTO,   0,   0,   0,            THING2,  NOT_CYCLING,   0, 0,  0, 0,  OLDMAN_3w,   191, 128, 90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNPole_3w,      0,               0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, 0,  CAMP_3w,     126, 35,  90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNPlant5_3w,    0,               0,         AUTO,   0,   0,   0,            THING1,  NOT_CYCLING,   0, 0,  0, 0,  TURN_3w,     65,  139, 90,  90,  0,  0,  0,  0,  0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
};

object_t objects_1d[] = {
//name, description, path, dx, dy, aptr, seq, seqp,           cyc, n, frm, rad, scr, x, y  , oldxy, vxy, val, g, cmnd, c, s, ctx, fgb
{kNHero_1d,     kDTthero_1d,    0, USER, 0,  0,  0,            PERSON,  CYCLE_FORWARD, 0, 0, 0, 0,  0,  229, 144, 90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 0: (Outside house)
{kNDoor_1d,     kDTtdoor_1d,    0, AUTO, 0,  0,  0,            THING4,  NOT_CYCLING,   1, 4, 4, 16, 0,  26,  131, 90,  90,  0,  0, 0,  1, kCMDdoor1_1d,   0, 0, 0, FLOATING,   29,  175, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNEyes_1d,     kDTteyes_1d,    0, AUTO, 0,  0,  0,            THING1,  NOT_CYCLING,   0, 0, 0, -1, 0,  80,  148, 90,  90,  0,  0, 0,  1, 0,              0, 0, 0, BACKGROUND, 72,  175, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNEyes_1d,     kDTteyes_1d,    0, AUTO, 0,  0,  0,            THING1,  NOT_CYCLING,   0, 0, 0, -1, 0,  59,  78,  90,  90,  0,  0, 0,  1, 0,              0, 0, 0, BACKGROUND, 72,  175, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNBat_1d,      kDTtbat_1d,     0, AUTO, 0,  0,  0,            THING1,  NOT_CYCLING,   0, 0, 0, -1, 0,  95,  55,  90,  90,  0,  0, 0,  1, 0,              0, 0, 0, BACKGROUND, 0,   0,   0,                     0, 0, 0, 0},
{kNPkin_1d,     kDTtpkin_1d,    0, AUTO, 0,  0,  0,            THING1,  NOT_CYCLING,   0, 0, 0, 10, 0,  20,  168, 90,  90,  0,  0, 2,  7, kCMDpkin_1d,    0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNKey_1d,      kDTtkey_1d,     0, AUTO, 0,  0,  0,            THING1,  INVISIBLE,     0, 0, 0, 10, 0,  24,  177, 90,  90,  0,  0, 5,  7, 0,              0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
// Screen 1: (Hall)
{kNDoor_1d,     kDTtdoor_1d,    0, AUTO, 0,  0,  0,            THING4,  NOT_CYCLING,   1, 4, 4, 16, 1,  125, 56,  90,  90,  0,  0, 0,  1, kCMDdoor2_1d,   0, 0, 0, FLOATING,   127, 98,  Common::KEYCODE_UP,    0, 0, 0, 0},
{kNDoor_1d,     kDTtdoor_1d,    0, AUTO, 0,  0,  0,            THING4,  NOT_CYCLING,   1, 4, 4, 16, 1,  208, 56,  90,  90,  0,  0, 0,  1, kCMDdoor3_1d,   0, 0, 0, FLOATING,   210, 98,  Common::KEYCODE_UP,    0, 0, 0, 0},
{kNEyes_1d,     kDTteyes_1d,    0, AUTO, 0,  0,  0,            THING1,  NOT_CYCLING,   0, 0, 0, -1, 1,  23,  48,  90,  90,  0,  0, 0,  1, 0,              0, 0, 0, BACKGROUND, 48,  98,  Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNEyes_1d,     kDTteyes_1d,    0, AUTO, 0,  0,  0,            THING1,  NOT_CYCLING,   0, 0, 0, -1, 1,  7,   93,  90,  90,  0,  0, 0,  1, 0,              0, 0, 0, BACKGROUND, 32,  140, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNCandle_1d,   kDTtcandle_1d,  0, AUTO, 0,  0,  0,            THING2,  CYCLE_FORWARD, 0, 0, 0, 20, 1,  176, 131, 90,  90,  0,  0, 3,  7, 0,              0, 0, 0, FLOATING,   169, 164, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNCupb_1d,     0,              0, AUTO, 9, 25,  0,            THING0,  INVISIBLE,     0, 0, 0, 20, 1,  69,  161, 67,  140, 0,  0, 0,  1, kCMDcupb_1d,    0, 0, 1, FOREGROUND, 81,  168, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNKnife_1d,    kDTtknife_1d,   0, AUTO, 0,  0,  0,            THING0,  INVISIBLE,     0, 0, 0, 20, 1,  69,  161, 90,  90,  0,  0, 6,  3, kCMDknife_1d,   0, 0, 0, FOREGROUND, -1,  0,   0,                     0, 0, 0, 0},
{kNWhistle_1d,  kDTtwhistle_1d, 0, AUTO, 0,  0,  0,            THING0,  INVISIBLE,     0, 0, 0, 20, 1,  69,  161, 90,  90,  0,  0, 6,  3, kCMDwhistle_1d, 0, 0, 0, FOREGROUND, -1,  0,   0,                     0, 0, 0, 0},
// Screen 2: (Bedroom 1)
// Note how wardrobe doors are catered for: The wardrobe has a name but no images,
// the right and left doors have no reference but are cycled by the wardrobe action list
{kNWard_1d,     kDTtward_1d,    0, AUTO, 0,   0,  0,            THING0,  INVISIBLE,     1, 4, 4, 30, 2,  172, 113, 90,  90,  0,  0, 0,  1, kCMDward_1d,    0, 0, 1, FOREGROUND, 168, 117, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNWdoorl_1d,   0,              0, AUTO, 0,   0,  0,            THING4,  NOT_CYCLING,   1, 4, 4, 0,  2,  150, 56,  90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNWdoorr_1d,   0,              0, AUTO, 0,   0,  0,            THING4,  NOT_CYCLING,   1, 4, 4, 0,  2,  174, 56,  90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNMask_1d,     kDTtmask_1d,    0, AUTO, 0,   0,  0,            THING1,  NOT_CYCLING,   0, 0, 0, 10, 2,  155, 100, 90,  90,  0,  0, 4,  7, kCMDmask_1d,    0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNMonkey_1d,   0,              0, AUTO, 0,   0,  0,            PERSON,  INVISIBLE,     0, 0, 0, 0,  2,  229, 144, 90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 3: (Dining room)
{kNButler_1d,   kDTtbutler_1d,  0, AUTO, 0,   0,  kALbutler_1d, PERSON,  NOT_CYCLING,   0, 0, 0, DX, 3,  70,  78,  90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNChop_1d,     kDTtchop_1d,    0, AUTO, 0,   0,  0,            THING1,  INVISIBLE,     0, 0, 0, 20, 3,  69,  161, 90,  90,  0,  0, 8,  7, kCMDchop_1d,    0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNRedeyes_1d,  0,              0, AUTO, 0,   0,  0,            THING1,  NOT_CYCLING,   0, 0, 0, 0,  3,  212, 108, 90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FOREGROUND, 0,   0,   0,                     0, 0, 0, 0},
{kNLips_1d,     0,              0, AUTO, 0,   0,  0,            THING2,  NOT_CYCLING,   0, 1, 1, 0,  3,  113, 105, 90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FOREGROUND, 0,   0,   0,                     0, 0, 0, 0},
{kNArm_1d,      0,              0, AUTO, 0,   0,  0,            THING2,  NOT_CYCLING,   0, 5, 5, 0,  3,  166, 122, 90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNHdlshero_1d, 0,              0, AUTO, 0,   0,  0,            THING1,  INVISIBLE,     0, 0, 0, 10, 0,  24,  177, 90,  90,  0,  0, 0,  7, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 6: (Garden)
{kNDoor_1d,     kDTtdoor_1d,    0, AUTO, 0,   0,  0,            THING4,  NOT_CYCLING,   1, 4, 4, 16, 6,  226, 58,  90,  90,  0,  0, 0,  1, kCMDdoor4_1d,   0, 0, 0, FLOATING,   224, 104, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNShed_1d,     0,              0, AUTO, 0,   0,  0,            THING0,  INVISIBLE,     0, 0, 0, 60, 6,  277, 39,  214, 37,  0,  0, 0,  1, kCMDshed_1d,    0, 0, 1, FOREGROUND, 213, 103, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNOilcan_1d,   kDTtoilcan_1d,  0, AUTO, 0,   0,  0,            THING0,  INVISIBLE,     0, 0, 0, 40, 6,  240, 65,  90,  90,  0,  0, 4,  3, kCMDoilcan_1d,  0, 0, 0, FOREGROUND, 0,   0,   0,                     0, 0, 0, 0},
// Screen 7: (Store room)
{kNDog_1d,      kDTtdog_1d,     0, AUTO, 0,   0,  kALdoggy_1d,  ANIMAL,  NOT_CYCLING,   0, 0, 0, 20, 7,  105, 119, 90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNCarpet_1d,   0,              0, AUTO, 0,   0,  0,            THING1,  NOT_CYCLING,   0, 0, 0, 50, 7,  191, 142, 90,  90,  0,  0, 0,  0, kCMDcarpet_1d,  0, 0, 0, BACKGROUND, 234, 153, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNTrap_1d,     0,              0, AUTO, 0,   0,  0,            THING4,  INVISIBLE,     0, 2, 2, 20, 7,  216, 140, 90,  90,  0,  0, 0,  1, kCMDtrap_1d,    0, 0, 0, BACKGROUND, 240, 152, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNBolt_1d,     0,              0, AUTO, 0,   0,  0,            THING0,  INVISIBLE,     0, 0, 0, 20, 7,  220, 145, 237, 151, 0,  0, 0,  1, kCMDbolt_1d,    0, 0, 0, FOREGROUND, 240, 152, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNHerodead_1d, 0,              0, AUTO, 0,   0,  0,            THING1,  INVISIBLE,     0, 0, 0, 10, 0,  24,  177, 90,  90,  0,  0, 0,  7, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 9: (Batcave)
{kNBat_1d,      kDTtbat_1d,     0, AUTO, 0,   0,  kALbats_1d,   THING1,  NOT_CYCLING,   0, 0, 0, 16, 9,  65,  25,  90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBat_1d,      kDTtbat_1d,     0, AUTO, 0,   0,  kALbats_1d,   THING1,  NOT_CYCLING,   0, 0, 0, 16, 9,  55,  65,  90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBat_1d,      kDTtbat_1d,     0, AUTO, 0,   0,  kALbats_1d,   THING1,  NOT_CYCLING,   0, 0, 0, 16, 9,  55,  120, 90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBat_1d,      kDTtbat_1d,     0, AUTO, 0,   0,  kALbats_1d,   THING1,  NOT_CYCLING,   0, 0, 0, 16, 9,  55,  130, 90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 10: (Mummy room)
{kNMummy_1d,    0,              0, AUTO, 0,   0,  kALmummy_1d,  PERSON,  NOT_CYCLING,   0, 0, 0, DX, 10, 256, 77,  90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNMdoor_1d,    0,              0, AUTO, 0,   0,  0,            THING4,  NOT_CYCLING,   1, 4, 4, 0,  10, 258, 55,  90,  90,  0,  0, 0,  1, kCMDdoor4_1d,   0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNGold_1d,     kDTtgold_1d,    0, AUTO, 0,   0,  0,            THING1,  NOT_CYCLING,   0, 0, 0, 20, 10, 208, 152, 90,  90,  0,  0, 10, 7, kCMDgold_1d,    0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
// Screen 11: (Lakeroom)
{kNBoat_1d,     kDTtboat_1d,    0, AUTO, 0,   0,  0,            THING2c, NOT_CYCLING,   0, 0, 0, 30, 11, 230, 118, 90,  90,  0,  0, 0,  1, kCMDboat_1d,    0, 0, 1, FLOATING,   250, 150, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNRope_1d,     kDTtrope_1d,    0, AUTO, 0,   0,  0,            THING2c, NOT_CYCLING,   0, 0, 0, 30, 11, 220, 132, 90,  90,  0,  0, 0,  1, kCMDrope_1d,    0, 0, 0, FLOATING,   209, 153, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNOldman_1d,   kDTtoldman_1d,  0, AUTO, 0,   0,  0,            THING1,  NOT_CYCLING,   0, 0, 0, 30, 11, 160, 38,  90,  90,  0,  0, 0,  1, kCMDoldman_1d,  0, 0, 0, FLOATING,   150, 54,  Common::KEYCODE_DOWN,  0, 0, 0, 0},
{kNWhero_1d,    0,              0, AUTO, 0,   0,  0,            PERSON,  INVISIBLE,     0, 0, 0, 0,  11, 100, 100, 90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 12: (Dead end)
{kNGuard_1d,    kDTtguard_1d,   0, AUTO, 0,   0,  0,            THING2d, NOT_CYCLING,   0, 0, 0, -1, 12, 147, 39,  90,  90,  0,  0, 0,  1, 0,              0, 0, 0, FLOATING,   133, 91,  Common::KEYCODE_UP,    0, 0, 0, 0},
// Screen 15: (Laboratory)
{kNProf_1d,     kDTtprof_1d,    0, AUTO, 0,   0,  0,            PERSON2, CYCLE_FORWARD, 0, 0, 0, -1, 1,  150, 55,  90,  90,  DX, 0, 0,  1, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNIgor_1d,     kDTtigor_1d,    0, AUTO, 0,   0,  0,            PERSON2, CYCLE_FORWARD, 0, 0, 0, -1, 15, 180, 122, 90,  90,  DX, 0, 0,  1, kCMDigor_1d,    0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBung_1d,     kDTtbung_1d,    0, AUTO, 0,   0,  0,            THING1,  NOT_CYCLING,   0, 0, 0, 14, 15, 75,  145, 90,  90,  0,  0, 11, 7, kCMDbung_1d,    0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNGdoor_1d,    0,              0, AUTO, 0,   0,  0,            THING1,  NOT_CYCLING,   0, 0, 0, 30, 15, 59,  100, 90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNSpachero_1d, 0,              0, AUTO, 0,   0,  0,            PERSON,  INVISIBLE,     0, 0, 0, 0,  15, 100, 100, 90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNFuzyhero_1d, 0,              0, AUTO, 0,   0,  0,            PERSON,  INVISIBLE,     0, 0, 0, 0,  15, 100, 100, 90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNArc_1d,      0,              0, AUTO, 0,   0,  0,            THING2,  CYCLE_FORWARD, 0, 0, 0, 0,  15, 106, 74,  90,  90,  0,  0, 0,  0, 0,              0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0}
};

object_t objects_2d[] = {
//name,description, path,dx,dy,aptr,seq,seqp,                 cyc,n,frm,rad,scr,x,y  ,oldxy,vxy,val,g,cmnd,c,s,ctx,fgb
{kNHero_2d,      kDTthero_2d,     0, AUTO,    0,    0,    0,            PERSON,  INVISIBLE,        0, 0, 0,  0,  0,                319, 199, 90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNPenny_2d,     kDTtpenny_2d,    0, AUTO,    0,    0,    0,            PERSON,  NOT_CYCLING,      0, 0, 0,  -1, 1,                109, 140, 90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 0: (Outside house)
{kNSmoke_2d,     0,               0, AUTO,    0,    0,    0,            THING3,  CYCLE_FORWARD,    0, 2, 0,  0,  0,                233, 20,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 1: (Hall)
{kNDoor_2d,      kDTtdoor_2d,     0, AUTO,    0,    0,    0,            THING4,  NOT_CYCLING,      1, 4, 4,  16, 1,                238, 40,  90,  90,  0, 0, 0,  1, kCMDdoor1_2d,    0, 0, 0, FLOATING,   239, 82,  Common::KEYCODE_UP,    0, 0, 0, 0},
{kNLips_2d,      0,               0, AUTO,    0,    0,    0,            THING2,  INVISIBLE,        0, 0, 0,  0,  1,                186, 100, 90,  90,  0, 0, 0,  0, 0,               0, 0, 0, FOREGROUND, 0,   0,   0,                     0, 0, 0, 0},
{kNMaid_2d,      kDTtmaid_2d,     0, AUTO,    0,    0,    0,            PERSON4, NOT_CYCLING,      0, 0, 0,  8,  1,                149, 135, 90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 2: (Bedroom 1)
{kNPennylie_2d,  0,               0, AUTO,    0,    0,    0,            THING1,  INVISIBLE,        0, 0, 0,  10, 2,                24,  177, 90,  90,  0, 0, 0,  1, 0,               0, 0, 0, OVEROVL,    0,   0,   0,                     0, 0, 0, 0},
{kNPenfall_2d,   0,               0, AUTO,    0,    0,    0,            THING2,  INVISIBLE,        0, 0, 0,  10, 2,                24,  177, 90,  90,  0, 0, 0,  0, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBookcase_2d,  kDTtbookcase_2d, 0, AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,      1, 0, 0,  -1, 2,                70,  81,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   90,  138, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNBook_2d,      kDTtbook_2d,     0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  10, 2,                90,  140, 90,  90,  0, 0, 0,  1, kCMDbook_2d,     0, 0, 1, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNKeyhole_2d,   0,               0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  10, 2,                28,  166, 26,  129, 0, 0, 0,  0, kCMDkeyhole_2d,  0, 0, 0, FLOATING,   35,  166, Common::KEYCODE_LEFT,  0, 0, 0, 0},
// Screen 3: (Bedroom 2)
{kNPanel_2d,     0,               0, AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,      1, 0, 0,  -1, 3,                189, 91,  90,  90,  0, 0, 0,  0, 0,               0, 0, 0, FOREGROUND, 0,   0,   0,                     0, 0, 0, 0},
{kNCupb_2d,      0,               0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  55, 3,                135, 142, 90,  90,  0, 0, 0,  0, kCMDlookdesk_2d, 0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBird_2d,      kDTtbird_2d,     0, AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD,    0, 0, 0,  -1, 3,                186, 100, 90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FOREGROUND, 192, 157, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNMatches_2d,   0,               0, AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,      1, 0, 0,  20, 255,              78,  30,  90,  90,  0, 0, 5,  7, kCMDmatches_2d,  0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNDumb_2d,      kDTtdumb_2d,     0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  20, 3,                72,  138, 55,  91,  0, 0, 0,  1, kCMDdumb_2d,     0, 0, 0, FLOATING,   66,  140, Common::KEYCODE_UP,    0, 0, 0, 0},
// Screen 4: (Keyhole)
{kNMurder_2d,    0,               0, AUTO,    0,    0,    0,            THING2e, CYCLE_FORWARD,    1, 2, 16, -1, 4,                141, 76,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 5: (Bed3)
{kNBalloon_2d,   kDTtballoon_2d,  0, WANDER,  DX/2, DY/2, 0,            THING1,  NOT_CYCLING,      0, 0, 0,  50, 5,                180, 40,  90,  90,  0, 0, 0,  1, kCMDballoon_2d,  0, 0, 0, FLOATING,   146, 130, Common::KEYCODE_UP,    0, 0, 0, 0},
// Screen 6: (Kitchen)
{kNDumb_2d,      kDTtdumb_2d,     0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  20, 6,                35,  152, 26,  99,  0, 0, 0,  1, kCMDdumb_2d,     0, 0, 0, FLOATING,   40,  153, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNCupb_2d,      0,               0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  55, 6,                135, 142, 90,  118, 0, 0, 0,  0, kCMDlookcupb_2d, 0, 0, 0, FLOATING,   112, 143, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNGarlic_2d,    kDTtgarlic_2d,   0, AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,      0, 0, 0,  20, 255,              78,  30,  90,  90,  0, 0, 5,  7, kCMDgarlic_2d,   0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNDoor_2d,      kDTdull_2d,      0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  55, 6,                290, 196, 90,  90,  0, 0, 0,  0, kCMDkdoor_2d,    0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 8: (Shed)
{kNGardner_2d,   kDTtgardner_2d,  0, WANDER,  DX,   DY,   0,            PERSON2, CYCLE_FORWARD,    0, 0, 0,  -1, 8,                250, 90,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 9: In shed
{kNButton_2d,    kDTtbutton_2d,   0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  30, 9,                190, 137, 90,  90,  0, 0, 0,  1, kCMDbutton_2d,   0, 1, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNRed_2d,       0,               0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  30, 9,                190, 137, 177, 99,  0, 0, 0,  1, kCMDred_2d,      0, 0, 0, FLOATING,   188, 137, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNYellow_2d,    0,               0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  30, 9,                190, 137, 189, 99,  0, 0, 0,  1, kCMDyellow_2d,   0, 0, 0, FLOATING,   172, 137, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNGreen_2d,     0,               0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  30, 9,                190, 137, 201, 99,  0, 0, 0,  1, kCMDgreen_2d,    0, 0, 0, FLOATING,   184, 137, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNBlue_2d,      0,               0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  30, 9,                190, 137, 215, 99,  0, 0, 0,  1, kCMDblue_2d,     0, 0, 0, FLOATING,   198, 137, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNShedlight_2d, 0,               0, AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,      1, 0, 0,  -1, 9,                161, 48,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 10: Venus fly traps
{kNMagnify_2d,   kDTtmagnify_2d,  0, AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,      0, 0, 0,  20, 10,               95,  96,  90,  90,  0, 0, 15, 7, 0,               0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNFly_2d,       0,               0, WANDER2, DX,   DY,   0,            THING2,  NOT_CYCLING,      0, 1, 0,  20, 10,               48,  60,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNFly_2d,       0,               0, WANDER2, DX,   DY,   0,            THING2,  NOT_CYCLING,      0, 1, 0,  20, 10,               58,  70,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNFly_2d,       0,               0, WANDER2, DX,   DY,   0,            THING2,  NOT_CYCLING,      0, 1, 0,  20, 10,               268, 90,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNLeaf_2d,      0,               0, AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD,    0, 1, 0,  20, 10,               48,  86,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   41,  103, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNLeaf_2d,      0,               0, AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD,    0, 2, 0,  20, 10,               79,  104, 90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   69,  121, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNLeaf_2d,      0,               0, AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD,    0, 1, 0,  20, 10,               71,  141, 90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   68,  153, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNLeaf_2d,      0,               0, AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD,    0, 3, 0,  20, 10,               116, 113, 90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   99,  132, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNLeaf_2d,      0,               0, AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD,    0, 1, 1,  20, 10,               164, 120, 90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   184, 136, Common::KEYCODE_LEFT,  0, 0, 0, 0},
{kNLeaf_2d,      0,               0, AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD,    0, 2, 0,  20, 10,               185, 83,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   167, 101, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNLeaf_2d,      0,               0, AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD,    0, 1, 0,  20, 10,               232, 96,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   223, 116, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNLeaf_2d,      0,               0, AUTO,    0,    0,    0,            THING2,  CYCLE_FORWARD,    0, 1, 0,  20, 10,               273, 141, 90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   251, 156, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
// Screen 11: Gates
{kNGatelight_2d, 0,               0, AUTO,    0,    0,    0,            THING1,  ALMOST_INVISIBLE, 1, 0, 0,  -1, 11,               90,  72,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 13: Stream
{kNCatnip_2d,    kDTdull_2d,      0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  -1, 13,               211, 136, 29,  114, 0, 0, 5,  3, kCMDcatnip_2d,   0, 0, 0, FLOATING,   49,  130, Common::KEYCODE_UP,    0, 0, 0, 0},
// Screen 14: Zapper
{kNZapper_2d,    kDTtzapper_2d,   0, AUTO,    0,    0,    0,            THING1,  ALMOST_INVISIBLE, 1, 0, 0,  -1, 14,               134, 46,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBug_2d,       kDTtbug_2d,      0, AUTO,    0,    0,    kALbugs_2d,   THING2,  CYCLE_FORWARD,    0, 0, 0,  16, 14,               65,  25,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBug_2d,       kDTtbug_2d,      0, AUTO,    0,    0,    kALbugs_2d,   THING2,  CYCLE_FORWARD,    0, 0, 0,  16, 14,               65,  25,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBug_2d,       kDTtbug_2d,      0, AUTO,    0,    0,    kALbugs_2d,   THING2,  CYCLE_FORWARD,    0, 0, 0,  16, 14,               65,  25,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBug_2d,       kDTtbug_2d,      0, AUTO,    0,    0,    kALbugs_2d,   THING2,  CYCLE_FORWARD,    0, 0, 0,  16, 14,               65,  25,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBug_2d,       kDTtbug_2d,      0, AUTO,    0,    0,    kALbugs_2d,   THING2,  CYCLE_FORWARD,    0, 0, 0,  16, 14,               65,  25,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 15: Mushroom
{kNOldman_2d,    kDTtoldman_2d,   0, AUTO,    0,    0,    0,            THING2c, NOT_CYCLING,      1, 0, 0,  -1, 15,               126, 77,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 16: Well
{kNWell_2d,      kDTtwell_2d,     0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  30, 16,               211, 136, 184, 85,  0, 0, 0,  1, kCMDwell_2d,     0, 0, 0, FLOATING,   242, 131, Common::KEYCODE_LEFT,  0, 0, 0, 0},
// Screen 17: Snakepit
{kNSnake_2d,     kDTtsnake_2d,    0, CHASE2,  DX,   DY,   kALsnake_2d,  PERSON2, CYCLE_FORWARD,    0, 0, 0,  16, 17,               165, 95,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
// Screen 18: Phonebox
{kNTardis_2d,    0,               0, AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,      0, 0, 0,  50, 18,               21,  74,  90,  90,  0, 0, 0,  0, kCMDtardis_2d,   0, 0, 1, FLOATING,   50,  126, Common::KEYCODE_LEFT,  0, 0, 0, 0},
// Screen 20: Kennel
{kNStick_2d,     kDTdull_2d,      0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  30, 20,               89,  120, 83,  116, 0, 0, 5,  3, kCMDstick_2d,    0, 0, 0, FLOATING,   88,  132, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNDynamite_2d,  kDTtdynamite_2d, 0, AUTO,    0,    0,    0,            THING2a, INVISIBLE,        0, 0, 0,  30, 20,               200, 100, 90,  90,  0, 0, 0,  7, kCMDdynamite_2d, 0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNKennel_2d,    0,               0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  20, 20,               195, 114, 173, 73,  0, 0, 0,  0, kCMDkennel_2d,   0, 0, 0, FLOATING,   160, 113, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNDog_2d,       kDTtdog_2d,      0, AUTO,    0,    0,    0,            THING2f, CYCLE_FORWARD,    0, 2, 0,  30, 20,               184, 80,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   200, 117, Common::KEYCODE_LEFT,  0, 0, 0, 0},
// Screen 21: (Rockroom)
{kNRope_2d,      kDTtrope_2d,     0, AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,      0, 0, 0,  60, 21,               78,  30,  90,  90,  0, 0, 0,  1, kCMDrope_2d,     0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
// Screen 22: (Rockgone)
{kNRope_2d,      kDTtrope_2d,     0, AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,      0, 0, 0,  60, 22,               78,  30,  90,  90,  0, 0, 0,  1, kCMDrope_2d,     0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
// Screen 24: (Lampcave)
{kNLamp_2d,      kDTtlamp_2d,     0, AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,      0, 0, 0,  30, 24,               78,  114, 90,  90,  0, 0, 10, 7, kCMDlamp_2d,     0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
// Screen 25: (Chasm)
{kNBanana_2d,    kDTtbanana_2d,   0, AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,      0, 0, 0,  20, 25,               254, 107, 90,  90,  0, 0, 5,  7, kCMDbanana_2d,   0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
// Screen 27: (Ladder)
{kNGenie_2d,     kDTtgenie_2d,    0, AUTO,    0,    0,    0,            PERSON3, INVISIBLE,        0, 0, 0,  30, 27,               138, 70,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
// Screen 28: (Traproom)
{kNSafe_2d,      kDTtsafe_2d,     0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  20, 28,               122, 144, 104, 110, 0, 0, 0,  1, kCMDsafe_2d,     0, 0, 0, FLOATING,   106, 145, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNWill_2d,      kDTtwill_2d,     0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  20, 28,               122, 144, 90,  90,  0, 0, 5,  7, kCMDwill_2d,     0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
// Screen 31: (Parlor)
{kNCupb_2d,      0,               0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  55, 31,               212, 142, 182, 113, 0, 0, 0,  0, kCMDcupbp_2d,    0, 0, 0, FLOATING,   200, 150, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNDoor_2d,      kDTdull_2d,      0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  55, 31,               28,  154, 90,  90,  0, 0, 0,  0, kCMDpdoor_2d,    0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNAlbum_2d,     kDTtalbum_2d,    0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  55, 255,              212, 142, 90,  90,  0, 0, 5,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Screen 32: (Catroom)
{kNCat_2d,       kDTdull_2d,      0, AUTO,    0,    0,    0,            THING2b, CYCLE_FORWARD,    0, 2, 0,  40, 32,               189, 69,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   171, 117, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
// Screen 33: (Boxroom)
{kNDoor_2d,      kDTtbdoor_2d,    0, AUTO,    0,    0,    0,            THING4,  NOT_CYCLING,      1, 4, 4,  16, 33,               137, 97,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   140, 142, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNPaper_2d,     0,               0, AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,      1, 0, 0,  20, 33,               205, 147, 90,  90,  0, 0, 5,  7, kCMDpaper_2d,    0, 0, 0, BACKGROUND, 193, 159, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNPencil_2d,    kDTdull_2d,      0, AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,      1, 0, 0,  20, 33,               205, 163, 90,  90,  0, 0, 5,  7, kCMDpencil_2d,   0, 0, 0, FLOATING,   188, 168, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
// Screen 34: (Hall3)
{kNDoor_2d,      kDTtdoor_2d,     0, AUTO,    0,    0,    0,            THING4,  NOT_CYCLING,      1, 4, 4,  16, 34,               234, 73,  90,  90,  0, 0, 0,  1, kCMDdoor2_2d,    0, 0, 0, FLOATING,   240, 121, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNDoor_2d,      kDTtdoor_2d,     0, AUTO,    0,    0,    0,            THING4,  NOT_CYCLING,      1, 4, 4,  16, 34,               103, 73,  90,  90,  0, 0, 0,  1, kCMDdoor3_2d,    0, 0, 0, FLOATING,   107, 121, Common::KEYCODE_UP,    0, 0, 0, 0},
// Screen 35: (Organ)
{kNHarry_2d,     kDTtharry_2d,    0, AUTO,    DX,   DY,   0,            THING2g, CYCLE_FORWARD,    0, 2, 0,  -1, 35,               188, 84,  90,  90,  0, 0, 0,  1, kCMDharry_2d,    0, 0, 0, FLOATING,   216, 132, Common::KEYCODE_LEFT,  0, 0, 0, 0},
// Screen 36: (Hestroom)
{kNHester_2d,    kDTthester_2d,   0, AUTO,    0,    0,    0,            PERSON,  CYCLE_FORWARD,    0, 0, 0,  30, 36,               78,  114, 90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   70,  150, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNLetter_2d,    kDTtletter_2d,   0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  30, 36,               110, 150, 106, 132, 0, 0, 0,  1, kCMDletter_2d,   0, 0, 0, FLOATING,   98,  151, Common::KEYCODE_UP,    0, 0, 0, 0},
// Screen 37: (Retupmoc)
{kNDoctor_2d,    kDTtdoctor_2d,   0, WANDER,  DX,   DY,   kALdoctor_2d, PERSON,  CYCLE_FORWARD,    0, 0, 0,  -1, 37,               78,  114, 90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNDalek_2d,     kDTtdalek_2d,    0, CHASE2,  DX,   DY,   kALdalek_2d,  PERSON5, NOT_CYCLING,      0, 0, 0,  -1, 37,               78,  114, 90,  90,  0, 0, 0,  1, kCMDgun_2d,      0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNScrew_2d,     kDTtscrew_2d,    0, AUTO,    0,    0,    0,            THING0,  INVISIBLE,        0, 0, 0,  20, 37,               100, 123, 90,  90,  0, 0, 15, 3, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
// Misc:
{kNCook_2d,      kDTtcook_2d,     0, AUTO,    DX,   DY,   0,            PERSON,  INVISIBLE,        0, 0, 0,  30, 06,               98,  98,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNCookb_2d,     kDTtcook_2d,     0, AUTO,    0,    0,    0,            THING4,  CYCLE_FORWARD,    0, 0, 0,  30, 255,              98,  98,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNCop_2d,       kDTtcop_2d,      0, AUTO,    0,    0,    0,            PERSON2, INVISIBLE,        0, 0, 0,  30, 29,               180, 47,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNHorace_2d,    kDTthorace_2d,   0, AUTO,    0,    0,    0,            PERSON,  INVISIBLE,        0, 0, 0,  30, 34,               215, 76,  90,  90,  0, 0, 0,  1, 0,               0, 0, 0, FLOATING,   0,   0,   0,                     0, 0, 0, 0},
{kNBell_2d,      kDTtbell_2d,     0, AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,      0, 0, 0,  30, MAZE_SCREEN + 15, 149, 109, 90,  90,  0, 0, 5,  7, kCMDbell_2d,     0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNGun_2d,       kDTtgun_2d,      0, AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,      0, 0, 0,  30, MAZE_SCREEN + 26, 149, 109, 90,  90,  0, 0, 10, 7, kCMDgun_2d,      0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0},
{kNBottle_2d,    kDTtbottle_2d,   0, AUTO,    0,    0,    0,            THING1,  NOT_CYCLING,      0, 0, 0,  30, MAZE_SCREEN + 27, 149, 109, 90,  90,  0, 0, 15, 7, kCMDbottle_2d,   0, 0, 0, FLOATING,   -1,  0,   0,                     0, 0, 0, 0}
};

object_t objects_3d[] = {
//name,description,path,dx,dy,aptr,SPRITE, cyc, n,frm,rad,      scr,x,y  ,oldxy,vxy,val,g,cmnd,c,s,ctx,fgb
// Common objects  // Set Penny state to 3 to avoid story
{kNHero_3d,     kDTthero_3d,     0, USER,   0,  0,  0,            PERSON,  CYCLE_FORWARD, 0, 0,  0, 0,  0,           161, 110, 90,  90,  0,  0,  0,  1, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNWhero_3d,    kDTthero_3d,     0, USER,   0,  0,  0,            PERSON,  INVISIBLE,     0, 0,  0, 0,  0,           219, 133, 90,  90,  0,  0,  0,  1, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNPenny_3d,    kDTtpenny_3d,    0, AUTO,   0,  0,  0,            PERSON,  CYCLE_FORWARD, 0, 0,  0, -1, 0,           109, 110, 90,  90,  0,  0,  0,  1, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNPennylie_3d, kDTtplie_3d,     0, AUTO,   0,  0,  0,            THING1,  INVISIBLE,     0, 0,  0, -1, WEB_3d,      75,  156, 90,  90,  0,  0,  0,  1, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNLips_3d,     0,               0, AUTO,   0,  0,  0,            THING2,  INVISIBLE,     0, 0,  0, 0,  0,           186, 100, 90,  90,  0,  0,  0,  0, 0,                0, 0, 0, FOREGROUND, 0,      0,   0,                     0, 0, 0, 0},
// CRASH site
{kNPlane_3d,    kDTtplane_3d,    0, AUTO,   0,  0,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, CRASH_3d,    184, 136, 90,  90,  0,  0,  0,  1, kCMDcplane_3d,    0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNDoor_3d,     kDTdull_3d,      0, AUTO,   0,  0,  0,            THING0,  INVISIBLE,     0, 0,  0, 80, CRASH_3d,    184, 136, 90,  90,  0,  0,  0,  1, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNPlant1_3d,   0,               0, AUTO,   0,  0,  0,            THING1,  INVISIBLE,     0, 0,  0, 0,  CRASH_3d,    132, 165, 90,  90,  0,  0,  0,  0, 0,                0, 0, 0, OVEROVL,    0,      0,   0,                     0, 0, 0, 0},
// INPLANE
{kNPlane_3d,    0,               0, AUTO,   0,  0,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, PLANE_3d,    184, 136, 90,  90,  0,  0,  0,  0, kCMDcexit_3d,     0, 0, 1, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNWater_3d,    0,               0, AUTO,   0,  0,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, WFALL_3d,    184, 136, 239, 22,  0,  0,  0,  0, kCMDcwfall_3d,    0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNWater_3d,    0,               0, AUTO,   0,  0,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, WFALL_B_3d,  184, 136, 90,  90,  0,  0,  0,  0, kCMDcwfall_3d,    0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNWater_3d,    0,               0, AUTO,   0,  0,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, STREAM_3d,   184, 136, 55,  183, 0,  0,  0,  0, kCMDcwstream_3d,  0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNWater_3d,    0,               0, AUTO,   0,  0,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, GARDEN_3d,   184, 136, 112, 129, 0,  0,  0,  0, kCMDcwpool_3d,    0, 0, 0, FLOATING,   155,    170, Common::KEYCODE_UP,    0, 0, 0, 0},
{kNClay_3d,     kDTtclay_3d,     0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, PLANE_3d,    162, 96,  90,  90,  0,  0,  5,  7, kCMDcclay_3d,     0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNNeedles_3d,  kDTtneedles_3d,  0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, PLANE_3d,    172, 90,  90,  90,  0,  0,  5,  7, kCMDcpins_3d,     0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNFlask_3d,    kDTtflask_3d,    0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, PLANE_3d,    190, 90,  90,  90,  0,  0,  5,  7, kCMDcflask_3d,    0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNBouillon_3d, kDTtbouillon_3d, 0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, PLANE_3d,    185, 94,  90,  90,  0,  0,  5,  7, kCMDcbouillon_3d, 0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNCheese_3d,   kDTtcheese_3d,   0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, PLANE_3d,    185, 100, 90,  90,  0,  0,  5,  1, kCMDccheese_3d,   0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
// WEB
{kNSpider_3d,   kDTtspider_3d,   0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, -1, WEB_3d,      77,  50,  90,  90,  0,  0,  0,  1, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNPlant3_3d,   0,               0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, 0,  WEB_3d,      245, 117, 90,  90,  0,  0,  0,  0, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNPlant4_3d,   0,               0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, 0,  WEB_3d,      285, 90,  91,  90,  0,  0,  0,  0, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
// BRIDGE
{kNBlock_3d,    0,               0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, BRIDGE_3d,   225, 133, 90,  90,  0,  0,  0,  0, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNBlock_3d,    0,               0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, BRIDGE_3d,   225, 134, 90,  90,  0,  0,  0,  0, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNBlock_3d,    0,               0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, BRIDGE_3d,   225, 135, 90,  90,  0,  0,  0,  0, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNVine_3d,     kDTtvine_3d,     0, AUTO,   0,  0,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, BRIDGE_3d,   184, 136, 228, 24,  0,  0,  0,  1, kCMDcvine_3d,     0, 0, 0, FLOATING,   237,    131, Common::KEYCODE_UP,    0, 0, 0, 0},
// STREAM
{kNVine_3d,     kDTtvine_3d,     0, AUTO,   0,  0,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, STREAM_3d,   184, 136, 101, 96,  0,  0,  0,  1, kCMDcswing_3d,    0, 0, 0, FLOATING,   50,     172, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
{kNSwinger_3d,  0,               0, AUTO,   0,  0,  0,            PERSON2, INVISIBLE,     0, 0,  0, 0,  STREAM_3d,   219, 133, 90,  90,  0,  0,  0,  0, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
// HUT_IN
{kNDoctor_3d,   kDTtdoctor_3d,   0, WANDER, DX, DY, kALdocgot_3d, PERSON3, CYCLE_FORWARD, 0, 0,  0, -1, CAMP_3d,     273, 83,  90,  90,  0,  0,  0,  1, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNDoclie_3d,   0,               0, AUTO,   0,  0,  0,            THING2,  INVISIBLE,     0, 0,  0, 30, HUT_IN_3d,   239, 103, 90,  90,  0,  0,  0,  1, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNCdoor_3d,    kDTdull_3d,      0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, 40, HUT_IN_3d,   239, 103, 90,  90,  0,  0,  0,  1, kCMDcdoor_3d,     0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNMouse_3d,    kDTtmouse_3d,    0, AUTO,   0,  0,  0,            THING2b, INVISIBLE,     0, 0,  0, 30, HUT_IN_3d,   186, 170, 90,  90,  0,  0,  0,  1, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNMoushole_3d, 0,               0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, HUT_IN_3d,   203, 122, 90,  90,  0,  0,  0,  1, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNCage_3d,     kDTtcage_3d,     0, AUTO,   0,  0,  0,            THING2a, NOT_CYCLING,   0, 0,  0, 30, HUT_IN_3d,   156, 121, 90,  90,  0,  0,  1,  5, kCMDccage_3d,     0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNFire_1_3d,   0,               0, AUTO,   0,  0,  0,            THING2,  CYCLE_FORWARD, 0, 0,  0, 0,  HUT_OUT_3d,  127, 100, 90,  90,  0,  0,  0,  0, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNFire_1_3d,   0,               0, AUTO,   0,  0,  0,            THING2,  CYCLE_FORWARD, 0, 0,  0, 0,  HUT_OUT_3d,  172, 100, 90,  90,  0,  0,  0,  0, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNFire_2_3d,   0,               0, AUTO,   0,  0,  0,            THING2,  CYCLE_FORWARD, 0, 0,  0, 0,  HUT_IN_3d,   30, 145,  90,  90,  0,  0,  0,  0, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNFire_3_3d,   0,               0, AUTO,   0,  0,  0,            THING2,  CYCLE_FORWARD, 0, 0,  0, 0,  CAMP_3d,     120, 135, 90,  90,  0,  0,  0,  0, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
// CAMP
{kNNat1_3d,     kDTtnative_3d,   0, WANDER, DX, 0,  0,            PERSON,  NOT_CYCLING,   0, 0,  0, -1, CAMP_3d,     130, 105, 90,  90,  0,  0,  0,  1, kCMDcnative_3d,   0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNNat2_3d,     kDTtnative_3d,   0, AUTO,   DX, DY, 0,            PERSON,  CYCLE_FORWARD, 0, 0,  0, -1, CAMP_3d,     17,  97,  90,  90,  4,  0,  0,  1, kCMDcnative_3d,   0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNNat3_3d,     kDTtnative_3d,   0, AUTO,   DX, DY, 0,            THING2,  CYCLE_FORWARD, 0, 16, 0, -1, CAMP_3d,     96,  40,  90,  90,  0,  0,  0,  1, kCMDcnative_3d,   0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNNatb_3d,     kDTtnative_3d,   0, AUTO,   DX, DY, 0,            THING2,  CYCLE_FORWARD, 0, 20, 0, -1, CAMP_3d,     72,  51,  90,  90,  0,  0,  0,  1, kCMDcnative_3d,   0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNNatg_3d,     kDTtnatgirl_3d,  0, AUTO,   DX, DY, kALnative_3d, PERSON,  CYCLE_FORWARD, 0, 0,  0, -1, CAMP_3d,     28,  101, 90,  90,  3,  0,  0,  1, kCMDcnative_3d,   0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNPipe_3d,     kDTtpipe_3d,     0, AUTO,   0,  0,  0,            THING1,  INVISIBLE,     0, 0,  0, 30, CAMP_3d,     225, 135, 90,  90,  0,  0,  0,  7, kCMDcdart_3d,     0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
// PATH_2
{kNElephant_3d, kDTtelephant_3d, 0, AUTO,   0,  0,  0,            THING2a, NOT_CYCLING,   0, 4,  0, -1, PATH_3d,     163, 85,  90,  90,  0,  0,  0,  1, kCMDcdart_3d,     0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNE_eyes_3d,   0,               0, AUTO,   0,  0,  0,            THING2c, NOT_CYCLING,   0, 0,  0, -1, PATH_3d,     194, 102, 90,  90,  0,  0,  0,  0, 0,                0, 0, 0, FOREGROUND, 0,      0,   0,                     0, 0, 0, 0},
// Misc
{kNHero_old_3d, 0,               0, USER,   0,  0,  0,            PERSON,  INVISIBLE,     0, 0,  0, 0,  0,           161, 120, 90,  90,  0,  0,  0,  1, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNAircraft_3d, 0,               0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, -1, SUNSET_3d,   275, 116, 90,  90,  -2, -1, 0,  1, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNScroll_3d,   kDTtscroll_3d,   0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, CLIFFTOP_3d, 75,  116, 90,  90,  0,  0,  3,  7, kCMDcscroll_3d,   0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNCrystal_3d,  kDTtcrystal_3d,  0, AUTO,   0,  0,  0,            THING1,  INVISIBLE,     0, 0,  0, 30, TURN_3d,     275, 116, 90,  90,  0,  0,  9,  7, kCMDccrystal_3d,  0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNRock_3d,     0,               0, AUTO,   0,  0,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, TURN_3d,     100, 100, 220, 120, 0,  0,  0,  0, kCMDcrock_3d,     0, 0, 0, FLOATING,   220,    169, Common::KEYCODE_RIGHT, 0, 0, 0, 0},
// CAVE etc.
{kNPlant2_3d,   0,               0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, 0,  CAVE_3d,     27,  160, 90,  90,  0,  0,  0,  0, 0,                0, 0, 0, OVEROVL,    0,      0,   0,                     0, 0, 0, 0},
{kNGhost_3d,    kDTtghost_3d,    0, CHASE,  DX, DY, 0,            THING2c, NOT_CYCLING,   0, 0,  0, -1, CAVE_3d,     121, 86,  90,  90,  0,  0,  0,  1, kCMDcghost_3d,    0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNBell_3d,     kDTtbell_3d,     0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, 30, CLIFF_3d,    202, 152, 90,  90,  0,  0,  2,  7, kCMDcbell_3d,     0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNBook_3d,     kDTtbook_3d,     0, AUTO,   0,  0,  0,            THING1,  INVISIBLE,     0, 0,  0, 30, STREAM_3d,   275, 116, 90,  90,  0,  0,  10, 7, kCMDcbook_3d,     0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNCandle_3d,   kDTtcandle_3d,   0, AUTO,   0,  0,  0,            THING2,  CYCLE_FORWARD, 0, 20, 0, 30, HUT_IN_3d,   77,  134, 90,  90,  0,  0,  3,  7, kCMDccandle_3d,   0, 0, 0, FLOATING,   GO_OBJ, 0,   0,                     0, 0, 0, 0},
{kNVine_3d,     kDTtvine_3d,     0, AUTO,   0,  0,  0,            THING0,  INVISIBLE,     0, 0,  0, -1, CAVE_3d,     184, 136, 116, 80,  0,  0,  0,  1, kCMDcswingc_3d,   0, 0, 0, FLOATING,   114,    139, Common::KEYCODE_UP,    0, 0, 0, 0},
// OLDMAN (inside cave)
{kNO_eye_3d,    0,               0, AUTO,   0,  0,  0,            THING3,  INVISIBLE,     0, 0,  0, 0,  OLDMAN_3d,   237, 77,  90,  90,  0,  0,  0,  0, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNFire_4_3d,   0,               0, AUTO,   0,  0,  0,            THING2,  CYCLE_FORWARD, 0, 0,  0, 0,  OLDMAN_3d,   65,  56,  90,  90,  0,  0,  0,  0, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNMouth_3d,    0,               0, AUTO,   0,  0,  0,            THING2,  NOT_CYCLING,   0, 0,  0, 0,  OLDMAN_3d,   191, 128, 90,  90,  0,  0,  0,  0, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNPole_3d,     0,               0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, 0,  CAMP_3d,     126, 35,  90,  90,  0,  0,  0,  0, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0},
{kNPlant5_3d,   0,               0, AUTO,   0,  0,  0,            THING1,  NOT_CYCLING,   0, 0,  0, 0,  TURN_3d,     65,  139, 90,  90,  0,  0,  0,  0, 0,                0, 0, 0, FLOATING,   0,      0,   0,                     0, 0, 0, 0}
};

// Hugo 1 Win
int rep1_1w[]    = {kSTsrep1_1w,  -1};
int rep2_1w[]    = {kSTsrep2_1w,  -1};
int rep3_1w[]    = {kSTsrep3_1w,  -1};
int rep4_1w[]    = {kSTsrep4_1w,  -1};
int rep5_1w[]    = {kSTsrep5_1w,  -1};
int rep6_1w[]    = {kSTsrep6_1w,  -1};
int rep7_1w[]    = {kSTsrep7_1w,  -1};
int openrep_1w[] = {kSTsopenr_1w, -1};

// Action lists (suffix with 0)
// These are the various actions, referred to in the command lists and/or
// referenced directly as part of screen actions
act0 areplight_1w  = {ASCHEDULE, 11 * NORMAL_TPS_v2d, kALightning_1w};
act0 arepeye_1w    = {ASCHEDULE, 8 * NORMAL_TPS_v2d,  kALblinkeyes1_1w};
act0 arepbat_1w    = {ASCHEDULE, 12 * NORMAL_TPS_v2d, kALbat_1w};
act0 arepeye2_1w   = {ASCHEDULE, 8 * NORMAL_TPS_v2d,  kALblinkeyes2_1w};
act0 arepredeye_1w = {ASCHEDULE, 6 * NORMAL_TPS_v2d,  kALrepredeye_1w};
act0 areplips_1w   = {ASCHEDULE, 4 * NORMAL_TPS_v2d,  kALreplips_1w};
act0 areparm_1w    = {ASCHEDULE, 5 * NORMAL_TPS_v2d,  kALreparm_1w};
act0 adead_1w      = {ASCHEDULE, 0,                   kALdead_1w};
act0 arepbata_1w   = {ASCHEDULE, 3 * NORMAL_TPS_v2d,  kALbatrep_1w};
act0 ajailrep_1w   = {ASCHEDULE, 4,                   kALjailrep_1w};
act0 aend_1w       = {ASCHEDULE, 4 * NORMAL_TPS_v2d,  kALend_1w};
act0 arepbox_1w    = {ASCHEDULE, NORMAL_TPS_v2d,      kALbox_1w};
act0 aweird_1w     = {ASCHEDULE, 16,                  kALweird_1w};
act0 acycle_1w     = {ASCHEDULE, 0,                   kALcycle_1w};

act1 aopendoor1a_1w  = {START_OBJ, NORMAL_TPS_v2d,         DOOR1_1w,    1, CYCLE_FORWARD};
act1 aclosedoor1_1w  = {START_OBJ, NORMAL_TPS_v2d,         DOOR1_1w,    1, CYCLE_BACKWARD};
act1 ablink1a_1w     = {START_OBJ, 0,                      EYES1_1w,    0, INVISIBLE};
act1 ablink1b_1w     = {START_OBJ, 1,                      EYES1_1w,    0, NOT_CYCLING};
act1 ablink1c_1w     = {START_OBJ, 2,                      EYES1_1w,    0, INVISIBLE};
act1 ablink1d_1w     = {START_OBJ, 3,                      EYES1_1w,    0, NOT_CYCLING};
act1 ablink2a_1w     = {START_OBJ, 3 * NORMAL_TPS_v2d,     EYES2_1w,    0, INVISIBLE};
act1 ablink2b_1w     = {START_OBJ, 3 * NORMAL_TPS_v2d + 1, EYES2_1w,    0, NOT_CYCLING};
act1 ablink2c_1w     = {START_OBJ, 3 * NORMAL_TPS_v2d + 2, EYES2_1w,    0, INVISIBLE};
act1 ablink2d_1w     = {START_OBJ, 3 * NORMAL_TPS_v2d + 3, EYES2_1w,    0, NOT_CYCLING};
act1 aridpkin_1w     = {START_OBJ, 0,                      PKIN_1w,     0, INVISIBLE};
act1 ashowkey_1w     = {START_OBJ, 0,                      KEY_1w,      0, NOT_CYCLING};
act1 aridprof_1w     = {START_OBJ, 130 / DX,               PROF_1w,     0, INVISIBLE};
act1 aopendoor2_1w   = {START_OBJ, 0,                      DOOR2_1w,    1, CYCLE_FORWARD};
act1 aopendoor3_1w   = {START_OBJ, 0,                      DOOR3_1w,    1, CYCLE_FORWARD};
act1 ablink3a_1w     = {START_OBJ, 0,                      EYES3_1w,    0, INVISIBLE};
act1 ablink3b_1w     = {START_OBJ, 1,                      EYES3_1w,    0, NOT_CYCLING};
act1 ablink3c_1w     = {START_OBJ, 2,                      EYES3_1w,    0, INVISIBLE};
act1 ablink3d_1w     = {START_OBJ, 3,                      EYES3_1w,    0, NOT_CYCLING};
act1 ablink4a_1w     = {START_OBJ, 3 * NORMAL_TPS_v2d,     EYES4_1w,    0, INVISIBLE};
act1 ablink4b_1w     = {START_OBJ, 3 * NORMAL_TPS_v2d + 1, EYES4_1w,    0, NOT_CYCLING};
act1 ablink4c_1w     = {START_OBJ, 3 * NORMAL_TPS_v2d + 2, EYES4_1w,    0, INVISIBLE};
act1 ablink4d_1w     = {START_OBJ, 3 * NORMAL_TPS_v2d + 3, EYES4_1w,    0, NOT_CYCLING};
act1 a115b_1w        = {START_OBJ, 0,                      MASK_1w,     0, NOT_CYCLING};
act1 acupknife_1w    = {START_OBJ, 0,                      KNIFE_1w,    0, NOT_CYCLING};
act1 acupwhist_1w    = {START_OBJ, 0,                      WHISTLE_1w,  0, NOT_CYCLING};
act1 aopenwdoorl_1w  = {START_OBJ, NORMAL_TPS_v2d,         WDOORL_1w,   1, CYCLE_FORWARD};
act1 aopenwdoorr_1w  = {START_OBJ, NORMAL_TPS_v2d,         WDOORR_1w,   1, CYCLE_FORWARD};
act1 aopenwd1_1w     = {START_OBJ, NORMAL_TPS_v2d,         MASK_1w,     1, NOT_CYCLING};
act1 aclosewdoorl_1w = {START_OBJ, NORMAL_TPS_v2d,         WDOORL_1w,   1, CYCLE_BACKWARD};
act1 aclosewdoorr_1w = {START_OBJ, NORMAL_TPS_v2d,         WDOORR_1w,   1, CYCLE_BACKWARD};
act1 abut1_1w        = {START_OBJ, 4 * NORMAL_TPS_v2d,     BUTLER_1w,   0, CYCLE_FORWARD};
act1 abut6d_1w       = {START_OBJ, 7,                      HERO,        0, INVISIBLE};
act1 abut6f_1w       = {START_OBJ, 7,                      HDLSHERO_1w, 0, NOT_CYCLING};
act1 ashowchop_1w    = {START_OBJ, 0,                      CHOP_1w,     0, NOT_CYCLING};
act1 aridchop_1w     = {START_OBJ, 0,                      CHOP_1w,     0, INVISIBLE};
act1 adogcyc_1w      = {START_OBJ, 0,                      DOG_1w,      0, CYCLE_FORWARD};
act1 ablink5a_1w     = {START_OBJ, 0,                      REDEYES_1w,  0, INVISIBLE};
act1 ablink5b_1w     = {START_OBJ, 1,                      REDEYES_1w,  0, NOT_CYCLING};
act1 ablink5c_1w     = {START_OBJ, 2,                      REDEYES_1w,  0, INVISIBLE};
act1 ablink5d_1w     = {START_OBJ, 3,                      REDEYES_1w,  0, NOT_CYCLING};
act1 alips_1w        = {START_OBJ, 0,                      LIPS_1w,     6, CYCLE_FORWARD};
act1 aarm_1w         = {START_OBJ, 0,                      ARM_1w,      3, CYCLE_BACKWARD};
act1 aopendoor4_1w   = {START_OBJ, 0,                      DOOR4_1w,    1, CYCLE_FORWARD};
act1 aclosedoor4_1w  = {START_OBJ, 0,                      DOOR4_1w,    1, CYCLE_BACKWARD};
act1 adog1_1w        = {START_OBJ, 0,                      DOG_1w,      0, CYCLE_FORWARD};
act1 adead1_1w       = {START_OBJ, 0,                      HERO,        0, INVISIBLE};
act1 adead3_1w       = {START_OBJ, 0,                      HERODEAD_1w, 0, NOT_CYCLING};
act1 amovecarp1_1w   = {START_OBJ, 0,                      CARPET_1w,   0, INVISIBLE};
act1 amovecarp2_1w   = {START_OBJ, 0,                      TRAP_1w,     0, NOT_CYCLING};
act1 aopentrap_1w    = {START_OBJ, NORMAL_TPS_v2d,         TRAP_1w,     1, CYCLE_FORWARD};
act1 aclosetrap_1w   = {START_OBJ, NORMAL_TPS_v2d,         TRAP_1w,     1, CYCLE_BACKWARD};
act1 amdoor1_1w      = {START_OBJ, NORMAL_TPS_v2d,         MDOOR_1w,    1, CYCLE_FORWARD};
act1 amdoor2_1w      = {START_OBJ, 3 * NORMAL_TPS_v2d,     MDOOR_1w,    0, INVISIBLE};
act1 amum1_1w        = {START_OBJ, 2 * NORMAL_TPS_v2d,     MUMMY_1w,    0, CYCLE_FORWARD};
act1 arock2_1w       = {START_OBJ, 0,                      HERO,        0, CYCLE_FORWARD};
act1 arock7_1w       = {START_OBJ, 20,                     HERO,        0, NOT_CYCLING};
act1 arock9_1w       = {START_OBJ, 40,                     HERO,        0, CYCLE_FORWARD};
act1 arock14_1w      = {START_OBJ, 55,                     HERO,        0, NOT_CYCLING};
act1 abin2_1w        = {START_OBJ, 0,                      HERO,        0, INVISIBLE};
act1 aridbung_1w     = {START_OBJ, 0,                      BUNG_1w,     0, INVISIBLE};
act1 about2_1w       = {START_OBJ, 0,                      HERO,        0, NOT_CYCLING};
act1 ajail2_1w       = {START_OBJ, 0,                      HERO,        0, CYCLE_FORWARD};
act1 atheend1_1w     = {START_OBJ, 5 * NORMAL_TPS_v2d,     HERO,        0, INVISIBLE};
act1 aguardgo2_1w    = {START_OBJ, 0,                      GUARD_1w,    0, CYCLE_FORWARD};
act1 alab5_1w        = {START_OBJ, 0,                      PROF_1w,     0, CYCLE_FORWARD};
act1 alab8_1w        = {START_OBJ, 12,                     PROF_1w,     0, NOT_CYCLING};
act1 alab9_1w        = {START_OBJ, 16,                     IGOR_1w,     0, NOT_CYCLING};
act1 abox10_1w       = {START_OBJ, 38,                     PROF_1w,     0, CYCLE_FORWARD};
act1 abox11_1w       = {START_OBJ, 73,                     PROF_1w,     0, INVISIBLE};

act2 abatxy_1w      = {INIT_OBJXY, 0,                   BAT_1w,   95,  55};
act2 aheroxy01_1w   = {INIT_OBJXY, 0,                   HERO,     106, 130};
act2 aheroxy12_1w   = {INIT_OBJXY, 12,                  HERO,     169, 87};
act2 aheroxy14_1w   = {INIT_OBJXY, 12,                  HERO,     135, 115};
act2 aheroxy10_1w   = {INIT_OBJXY, 0,                   HERO,     33,  134};
act2 aheroxy13_1w   = {INIT_OBJXY, 0,                   HERO,     40,  127};
act2 aheroxy15_1w   = {INIT_OBJXY, 0,                   HERO,     250, 120};
act2 ahchase2_1w    = {INIT_OBJXY, 5 * NORMAL_TPS_v2d,  DOG_1w,   280, 137};
act2 akchase2_1w    = {INIT_OBJXY, 5 * NORMAL_TPS_v2d,  DOG_1w,   30,  120};
act2 a115d_1w       = {INIT_OBJXY, 0,                   MASK_1w,  236, 91};
act2 aheroxy115_1w  = {INIT_OBJXY, 1,                   HERO,     27,  130};
act2 aheroxy21_1w   = {INIT_OBJXY, 0,                   HERO,     130, 56};
act2 achopxy_1w     = {INIT_OBJXY, 0,                   CHOP_1w,  51,  155};
act2 aheroxy31_1w   = {INIT_OBJXY, 0,                   HERO,     263, 126};
act2 aheroxy35_1w   = {INIT_OBJXY, 0,                   HERO,     253, 96};
act2 aheroxy41_1w   = {INIT_OBJXY, 0,                   HERO,     200, 56};
act2 aheroxy51_1w   = {INIT_OBJXY, 0,                   HERO,     200, 110};
act2 aheroxy53_1w   = {INIT_OBJXY, 0,                   HERO,     50,  90};
act2 aheroxy56_1w   = {INIT_OBJXY, 0,                   HERO,     260, 140};
act2 aheroxy57_1w   = {INIT_OBJXY, 0,                   HERO,     245, 107};
act2 aheroxy65_1w   = {INIT_OBJXY, 0,                   HERO,     215, 96};
act2 aheroxy75_1w   = {INIT_OBJXY, 0,                   HERO,     25,  105};
act2 adog4_1w       = {INIT_OBJXY, 0,                   DOG_1w,   105, 119};
act2 aheroxy78_1w   = {INIT_OBJXY, NORMAL_TPS_v2d + 12, HERO,     80,  42};
act2 aheroxy89_1w   = {INIT_OBJXY, 0,                   HERO,     276, 135};
act2 aheroxy87_1w   = {INIT_OBJXY, 0,                   HERO,     235, 108};
act2 aheroxy910_1w  = {INIT_OBJXY, 0,                   HERO,     50,  132};
act2 aheroxy98_1w   = {INIT_OBJXY, 0,                   HERO,     130, 120};
act2 abata1c_1w     = {INIT_OBJXY, 0,                   BAT2_1w,  65,  25};
act2 abata2c_1w     = {INIT_OBJXY, 0,                   BAT3_1w,  55,  65};
act2 abata3c_1w     = {INIT_OBJXY, 0,                   BAT4_1w,  50,  120};
act2 abata4c_1w     = {INIT_OBJXY, 0,                   BAT5_1w,  55,  130};
act2 aheroxy109_1w  = {INIT_OBJXY, 0,                   HERO,     96,  105};
act2 aheroxy1011_1w = {INIT_OBJXY, 0,                   HERO,     76,  130};
act2 aheroxy1110_1w = {INIT_OBJXY, 0,                   HERO,     261, 77};
act2 aheroxy1112_1w = {INIT_OBJXY, 0,                   HERO,     216, 134};
act2 aherofar_1w    = {INIT_OBJXY, 0,                   HERO,     142, 25};
act2 aheronear_1w   = {INIT_OBJXY, 0,                   HERO,     230, 132};
act2 aheroxy1213_1w = {INIT_OBJXY, 0,                   HERO,     131, 110};
act2 aguardgo1_1w   = {INIT_OBJXY, 0,                   GUARD_1w, 137, 39};
act2 aheroxy1211_1w = {INIT_OBJXY, 0,                   HERO,     291, 42};
act2 aheroxy151_1w  = {INIT_OBJXY, 0,                   HERO,     245, 55};
act2 alab2_1w       = {INIT_OBJXY, 0,                   PROF_1w,  100, 130};
act2 abox4a_1w      = {INIT_OBJXY, 20,                  HERO,     124, 122};
act2 aigor12_1w     = {INIT_OBJXY, 30,                  HERO,     116, 112};

act3 aopenp_1w = {PROMPT, 0, kSTsopenp_1w, openrep_1w, kALopenyes_1w, kALopenno_1w, false};
act3 amanq1_1w = {PROMPT, 0, kSTsq1_1w,    rep1_1w,    kALrepyes1_1w, kALrepno1_1w, false};
act3 amanq2_1w = {PROMPT, 0, kSTsq2_1w,    rep2_1w,    kALrepyes2_1w, kALrepno1_1w, false};
act3 amanq3_1w = {PROMPT, 0, kSTsq3_1w,    rep3_1w,    kALrepyes3_1w, kALrepno1_1w, false};
act3 amanq4_1w = {PROMPT, 0, kSTsq4_1w,    rep4_1w,    kALrepyes4_1w, kALrepno1_1w, false};
act3 amanq5_1w = {PROMPT, 0, kSTsq5_1w,    rep5_1w,    kALrepyes5_1w, kALrepno1_1w, false};
act3 amanq6_1w = {PROMPT, 0, kSTsq6_1w,    rep6_1w,    kALrepyes6_1w, kALrepno1_1w, false};
act3 amanq7_1w = {PROMPT, 0, kSTsq7_1w,    rep7_1w,    kALrepyes7_1w, kALrepno3_1w, false};

act4 abk1_1w     = {BKGD_COLOR, 0,  _TBLUE};
act4 abknorm1_1w = {BKGD_COLOR, 0,  _TBLACK};
act4 abk2_1w     = {BKGD_COLOR, 2,  _TBLUE};
act4 abknorm2_1w = {BKGD_COLOR, 3,  _TBLACK};
act4 abg1_1w     = {BKGD_COLOR, 0,  _TLIGHTYELLOW};
act4 abg2_1w     = {BKGD_COLOR, 2,  _TLIGHTMAGENTA};
act4 abg3_1w     = {BKGD_COLOR, 4,  _TLIGHTRED};
act4 abg4_1w     = {BKGD_COLOR, 6,  _TBLACK};
act4 abg5_1w     = {BKGD_COLOR, 8,  _TLIGHTYELLOW};
act4 abg6_1w     = {BKGD_COLOR, 10, _TLIGHTMAGENTA};
act4 abg7_1w     = {BKGD_COLOR, 12, _TLIGHTRED};
act4 abg8_1w     = {BKGD_COLOR, 14, _TBLACK};

act5 aopendoor1b_1w = {INIT_OBJVXY, 0,                  HERO,      DX,     -DY};
act5 abatvxy1_1w    = {INIT_OBJVXY, 0,                  BAT_1w,    2,      -2};
act5 abatvxy2_1w    = {INIT_OBJVXY, 20,                 BAT_1w,    -1,     1};
act5 abatvxy3_1w    = {INIT_OBJVXY, 40,                 BAT_1w,    -1,     -1};
act5 abatvxy4_1w    = {INIT_OBJVXY, 50,                 BAT_1w,    -5,     0};
act5 abatvxy5_1w    = {INIT_OBJVXY, 60,                 BAT_1w,    0,      0};
act5 astophero_1w   = {INIT_OBJVXY, 0,                  HERO,      0,      0};
act5 abutvxy1_1w    = {INIT_OBJVXY, 4 * NORMAL_TPS_v2d, BUTLER_1w, DX - 2, 0};
act5 adead4_1w      = {INIT_OBJVXY, 0,                  HERO,      0,      0};
act5 arock4_1w      = {INIT_OBJVXY, 0,                  HERO,      DX,     0};
act5 arock6_1w      = {INIT_OBJVXY, 11,                 HERO,      0,      -DY};
act5 arock8_1w      = {INIT_OBJVXY, 20,                 HERO,      0,      0};
act5 arock11_1w     = {INIT_OBJVXY, 40,                 HERO,      0,      -DY};
act5 arock13_1w     = {INIT_OBJVXY, 44,                 HERO,      0,      DY};
act5 arock15_1w     = {INIT_OBJVXY, 55,                 HERO,      0,      0};
act5 ahin2_1w       = {INIT_OBJVXY, 0,                  HERO,      0,      0};
act5 aboatvxy1_1w   = {INIT_OBJVXY, 0,                  BOAT_1w,   0,      -2};
act5 aboatvxy2_1w   = {INIT_OBJVXY, 10,                 BOAT_1w,   -5,     0};
act5 aboatvxy3_1w   = {INIT_OBJVXY, 20,                 BOAT_1w,   2,      0};
act5 aboatvxy4_1w   = {INIT_OBJVXY, 50,                 BOAT_1w,   1,      -1};
act5 aboatvxy5_1w   = {INIT_OBJVXY, 60,                 BOAT_1w,   -2,     -3};
act5 aboatvxy6_1w   = {INIT_OBJVXY, 70,                 BOAT_1w,   -3,     0};
act5 aboatvxy7_1w   = {INIT_OBJVXY, 100,                BOAT_1w,   0,      -3};
act5 aboatvxy8_1w   = {INIT_OBJVXY, 104,                BOAT_1w,   -1,     -1};
act5 aboatvxy9_1w   = {INIT_OBJVXY, 107,                BOAT_1w,   0,      0};
act5 aboatvxy10_1w  = {INIT_OBJVXY, 0,                  BOAT_1w,   9,      7};
act5 aboatvxy11_1w  = {INIT_OBJVXY, 3,                  BOAT_1w,   0,      0};
act5 aboatvxy12_1w  = {INIT_OBJVXY, 10,                 BOAT_1w,   1,      1};
act5 aguardgo4_1w   = {INIT_OBJVXY, 0,                  GUARD_1w,  -DX,    0};
act5 alab3_1w       = {INIT_OBJVXY, 0,                  PROF_1w,   DX,     0};
act5 alab6_1w       = {INIT_OBJVXY, 12,                 PROF_1w,   0,      0};
act5 alab7_1w       = {INIT_OBJVXY, 16,                 IGOR_1w,   0,      0};
act5 abox0_1w       = {INIT_OBJVXY, 0,                  GDOOR_1w,  6,      0};
act5 abox1_1w       = {INIT_OBJVXY, 9,                  GDOOR_1w,  0,      0};
act5 abox7_1w       = {INIT_OBJVXY, 50,                 GDOOR_1w,  -6,     0};
act5 abox8_1w       = {INIT_OBJVXY, 57,                 GDOOR_1w,  0,      0};
act5 abox8a_1w      = {INIT_OBJVXY, 56,                 GDOOR_1w,  0,      0};
act5 abox9_1w       = {INIT_OBJVXY, 38,                 PROF_1w,   -DX,    0};

act6 adroppkin_1w = {INIT_CARRY, 0, PKIN_1w,   false};
act6 a115c_1w     = {INIT_CARRY, 0, MASK_1w,   false};
act6 adropmask_1w = {INIT_CARRY, 0, MASK_1w,   false};
act6 abut7_1w     = {INIT_CARRY, 0, CHOP_1w,   true};
act6 adropchop_1w = {INIT_CARRY, 0, CHOP_1w,   false};
act6 ashedoil3_1w = {INIT_CARRY, 0, OILCAN_1w, true};
act6 adropbung_1w = {INIT_CARRY, 0, BUNG_1w,   false};

act7 amovekey_1w = {INIT_HF_COORD, 0,                  KEY_1w};
act7 ahchase1_1w = {INIT_HF_COORD, 5 * NORMAL_TPS_v2d, DOG_1w};
act7 akchase1_1w = {INIT_HF_COORD, 5 * NORMAL_TPS_v2d, DOG_1w};
// Those two actions were defined as act11 with a type set to INIT_HF_COORD
act7 adog3_1w    = {INIT_HF_COORD, 0,                  DOG_1w};
act7 alab1_1w    = {INIT_HF_COORD, 0,                  PROF_1w};

act8 ascr01_1w   = {NEW_SCREEN, 0,                   1};
act8 ascr12_1w   = {NEW_SCREEN, 12,                  2};
act8 ascr14_1w   = {NEW_SCREEN, 12,                  4};
act8 ascr10_1w   = {NEW_SCREEN, 0,                   0};
act8 ascr13_1w   = {NEW_SCREEN, 0,                   3};
act8 ascr15_1w   = {NEW_SCREEN, 0,                   5};
act8 ascr115_1w  = {NEW_SCREEN, 1,                   15};
act8 ascr21_1w   = {NEW_SCREEN, 0,                   1};
act8 ascr31_1w   = {NEW_SCREEN, 0,                   1};
act8 ascr35_1w   = {NEW_SCREEN, 0,                   5};
act8 ascr41_1w   = {NEW_SCREEN, 0,                   1};
act8 ascr51_1w   = {NEW_SCREEN, 0,                   1};
act8 ascr53_1w   = {NEW_SCREEN, 0,                   3};
act8 ascr56_1w   = {NEW_SCREEN, 0,                   6};
act8 ascr57_1w   = {NEW_SCREEN, 0,                   7};
act8 ascr65_1w   = {NEW_SCREEN, 0,                   5};
act8 ascr75_1w   = {NEW_SCREEN, 0,                   5};
act8 aopen78_1w  = {NEW_SCREEN, NORMAL_TPS_v2d + 12, 8};
act8 ascr89_1w   = {NEW_SCREEN, 0,                   9};
act8 ascr87_1w   = {NEW_SCREEN, 0,                   7};
act8 ascr910_1w  = {NEW_SCREEN, 0,                   10};
act8 ascr98_1w   = {NEW_SCREEN, 0,                   8};
act8 ascr109_1w  = {NEW_SCREEN, 0,                   9};
act8 ascr1011_1w = {NEW_SCREEN, 0,                   11};
act8 ascr1110_1w = {NEW_SCREEN, 0,                   10};
act8 ascr1112_1w = {NEW_SCREEN, 0,                   12};
act8 ascr1213_1w = {NEW_SCREEN, 0,                   13};
act8 atheend2_1w = {NEW_SCREEN, 5 * NORMAL_TPS_v2d,  14};
act8 ascr1211_1w = {NEW_SCREEN, 0,                   11};
act8 ascr151_1w  = {NEW_SCREEN, 0,                   1};

act9 ast12_1w       = {INIT_OBJSTATE, 12,                  DOOR2_1w,   0};
act9 ast14_1w       = {INIT_OBJSTATE, 12,                  DOOR3_1w,   0};
act9 ast01_1w       = {INIT_OBJSTATE, 0,                   DOOR1_1w,   0};
act9 adef8_1w       = {INIT_OBJSTATE, 0,                   WHISTLE_1w, 1};
act9 a115g_1w       = {INIT_OBJSTATE, 0,                   MASK_1w,    0};
act9 aopenwd2_1w    = {INIT_OBJSTATE, 0,                   WDOORL_1w,  1};
act9 aworn_1w       = {INIT_OBJSTATE, 0,                   MASK_1w,    1};
act9 aremoved_1w    = {INIT_OBJSTATE, 0,                   MASK_1w,    0};
act9 abut4a_1w      = {INIT_OBJSTATE, 0,                   BUTLER_1w,  1};
act9 abut7a_1w      = {INIT_OBJSTATE, 0,                   BUTLER_1w,  1};
act9 abut10_1w      = {INIT_OBJSTATE, 10 * NORMAL_TPS_v2d, BUTLER_1w,  0};
act9 astatedoor4_1w = {INIT_OBJSTATE, 0,                   DOOR4_1w,   1};
act9 ashedoil2_1w   = {INIT_OBJSTATE, 0,                   SHED_1w,    1};
act9 ast78_1w       = {INIT_OBJSTATE, NORMAL_TPS_v2d + 12, TRAP_1w,    0};
act9 ahin1_1w       = {INIT_OBJSTATE, 0,                   HERO,       1};
act9 ahout_1w       = {INIT_OBJSTATE, 0,                   HERO,       0};
act9 aboatmov_1w    = {INIT_OBJSTATE, 0,                   BOAT_1w,    2};
act9 aboatfar_1w    = {INIT_OBJSTATE, 107,                 BOAT_1w,    1};
act9 aboatnear_1w   = {INIT_OBJSTATE, 13,                  BOAT_1w,    0};
act9 aompass_1w     = {INIT_OBJSTATE, 0,                   OLDMAN_1w,  1};
act9 abox12_1w      = {INIT_OBJSTATE, 0,                   GDOOR_1w,   1};
act9 aigor33_1w     = {INIT_OBJSTATE, 0,                   GDOOR_1w,   4};
act9 aigor23_1w     = {INIT_OBJSTATE, 0,                   GDOOR_1w,   3};
act9 aigor14_1w     = {INIT_OBJSTATE, 0,                   GDOOR_1w,   2};

act10 ahchase3_1w = {INIT_PATH, 5 * NORMAL_TPS_v2d, DOG_1w,    CHASE,  DX * 2, DY * 2};
act10 akchase3_1w = {INIT_PATH, 5 * NORMAL_TPS_v2d, DOG_1w,    CHASE,  DX * 2, DY * 2};
act10 adef2_1w    = {INIT_PATH, NORMAL_TPS_v2d,     BAT2_1w,   WANDER, DX,     DY};
act10 adef3_1w    = {INIT_PATH, NORMAL_TPS_v2d,     BAT3_1w,   WANDER, DX,     DY};
act10 adef4_1w    = {INIT_PATH, NORMAL_TPS_v2d,     BAT4_1w,   WANDER, DX,     DY};
act10 adef5_1w    = {INIT_PATH, NORMAL_TPS_v2d,     BAT5_1w,   WANDER, DX,     DY};
act10 abut2_1w    = {INIT_PATH, 8 * NORMAL_TPS_v2d, BUTLER_1w, CHASE,  DX - 2, DY - 2};
act10 abut3_1w    = {INIT_PATH, 0,                  HERO,      AUTO,   0,      0};
act10 abut8_1w    = {INIT_PATH, 0,                  BUTLER_1w, WANDER, DX - 2, DY - 2};
act10 abut9_1w    = {INIT_PATH, 0,                  HERO,      USER,   0,      0};
act10 adog2_1w    = {INIT_PATH, 0,                  DOG_1w,    CHASE,  DX * 2, DY * 2};
act10 abata1a_1w  = {INIT_PATH, 0,                  BAT2_1w,   CHASE,  DX * 2, DY * 2};
act10 abata1b_1w  = {INIT_PATH, 7,                  BAT2_1w,   WANDER, DX,     DY};
act10 abata2a_1w  = {INIT_PATH, 0,                  BAT3_1w,   CHASE,  DX * 2, DY * 2};
act10 abata2b_1w  = {INIT_PATH, 6,                  BAT3_1w,   WANDER, DX,     DY};
act10 abata3a_1w  = {INIT_PATH, 0,                  BAT4_1w,   CHASE,  DX * 2, DY * 2};
act10 abata3b_1w  = {INIT_PATH, 5,                  BAT4_1w,   WANDER, DX,     DY};
act10 abata4a_1w  = {INIT_PATH, 0,                  BAT5_1w,   CHASE,  DX * 2, DY * 2};
act10 abata4b_1w  = {INIT_PATH, 4,                  BAT5_1w,   WANDER, DX,     DY};
act10 amum2_1w    = {INIT_PATH, 3 * NORMAL_TPS_v2d, MUMMY_1w,  CHASE,  DX * 2, DY * 2};
act10 arock1_1w   = {INIT_PATH, 0,                  HERO,      AUTO,   0,      0};
act10 arock16_1w  = {INIT_PATH, 55,                 HERO,      USER,   0,      0};
act10 abin3_1w    = {INIT_PATH, 0,                  HERO,      AUTO,   0,      0};
act10 about3_1w   = {INIT_PATH, 0,                  HERO,      USER,   0,      0};
act10 ajail1_1w   = {INIT_PATH, 0,                  HERO,      AUTO,   0,      0};
act10 alab14_1w   = {INIT_PATH, 40,                 IGOR_1w,   WANDER, DX,     0};
act10 acyc1_1w    = {INIT_PATH, 0,                  HERO,      QUIET,  0,      0};
act10 acyc2_1w    = {INIT_PATH, 57,                 HERO,      USER,   0,      0};

act11 atcup2_1w        = {COND_R,        0,   CUPBOARD_1w, 2, kALcuptxt2_1w,    kALcuptxt3_1w};
act11 atcup1_1w        = {COND_R,        0,   CUPBOARD_1w, 1, kALcuptxt1_1w,    kALlookcupb2_1w};
act11 atcup0_1w        = {COND_R,        0,   CUPBOARD_1w, 0, kALcuptxt0_1w,    kALlookcupb1_1w};
act11 a115a_1w         = {COND_R,        0,   MASK_1w,     1, kALswapmask_1w,   0};
act11 achkmaskcarry_1w = {COND_R,        0,   WDOORL_1w,   0, kALopenwdoorm_1w, 0};
act11 aswapcheck_1w    = {COND_R,        0,   MASK_1w,     0, kALwearmask_1w,   kALremovemask_1w};
act11 abut3a_1w        = {COND_R,        0,   BUTLER_1w,   0, kALbutp_1w,       0};
act11 abut5_1w         = {COND_R,        0,   MASK_1w,     0, kALbutchopped_1w, kALbutchop_1w};
act11 abut6_1w         = {COND_R,        0,   MASK_1w,     0, kALbutchopped_1w, kALbutroam_1w};
act11 at78b_1w         = {COND_R,        0,   MASK_1w,     1, kALswapmask_1w,   0};
act11 atrap_1w         = {COND_R,        0,   BOLT_1w,     2, kALopenpass_1w,   kALopenfail_1w};
act11 abatcheck_1w     = {COND_R,        0,   WHISTLE_1w,  0, kALbatty_1w,      0};
act11 abin_1w          = {COND_R,        0,   BUNG_1w,     0, kALnobung_1w,     kALembark_1w};
act11 achkout_1w       = {COND_R,        0,   OLDMAN_1w,   1, kALdeboat_1w,     kALnodeboat_1w};
act11 agetout_1w       = {COND_R,        0,   BOAT_1w,     0, kALdeboat_1w,     kALchkout_1w};
act11 aherochk_1w      = {COND_R,        107, HERO,        1, kALherofar_1w,    0};
act11 apush_1w         = {COND_R,        0,   BOAT_1w,     0, kALgofar_1w,      kALcomenear_1w};
act11 achkmove_1w      = {COND_R,        0,   BOAT_1w,     2, kALmoving_1w,     kALmoveboat_1w};
act11 achkrope_1w      = {COND_R,        0,   ROPE_1w,     1, kALchkmove_1w,    kALnotcut_1w};
act11 achkboat1_1w     = {COND_R,        0,   HERO,        0, kALgetinboat_1w,  kALchkboat2_1w};
act11 achkpass_1w      = {COND_R,        0,   OLDMAN_1w,   1, kALomasked_1w,    kALomask_1w};
act11 achkd3_1w        = {COND_R,        0,   GDOOR_1w,    3, kALdmsg3_1w,      kALok151_1w};
act11 achkd2_1w        = {COND_R,        0,   GDOOR_1w,    2, kALdmsg2_1w,      kALchkd3_1w};
act11 achkd1_1w        = {COND_R,        0,   GDOOR_1w,    1, kALdmsg1_1w,      kALchkd2_1w};
act11 achkd0_1w        = {COND_R,        0,   GDOOR_1w,    0, kALok151_1w,      kALchkd1_1w};
act11 achklab_1w       = {COND_R,        0,   GDOOR_1w,    0, kALprof_1w,       0};
act11 abbox_1w         = {COND_R,        0,   GDOOR_1w,    0, kALbox0_1w,       0};
act11 aichk3_1w        = {COND_R,        0,   GDOOR_1w,    3, kALigor3_1w,      kALinorm_1w};
act11 aichk2_1w        = {COND_R,        0,   GDOOR_1w,    2, kALigor2_1w,      kALichk3_1w};
act11 aichk1_1w        = {COND_R,        0,   GDOOR_1w,    1, kALigor1_1w,      kALichk2_1w};
act11 aichk0_1w        = {COND_R,        0,   GDOOR_1w,    0, kALigor0_1w,      kALichk1_1w};

act12 apbreak_1w     = {TEXT, 0,                      kSTspbreak_1w};
act12 acuptxt0_1w    = {TEXT, 0,                      kSTsseepkdw_1w};
act12 acuptxt1_1w    = {TEXT, 0,                      kSTsseedw_1w};
act12 acuptxt2_1w    = {TEXT, 0,                      kSTsseepk_1w};
act12 acuptxt3_1w    = {TEXT, 0,                      kSTesnosee_1w};
act12 adwwhy_1w      = {TEXT, 0,                      kSTsWonder_1w};
act12 ablowt_1w      = {TEXT, 3 * NORMAL_TPS_v2d,     kSTsBlowWhistle_1w};
act12 adef6_1w       = {TEXT, NORMAL_TPS_v2d,         kSTsdefbat1_1w};
act12 adef7_1w       = {TEXT, 3 * NORMAL_TPS_v2d,     kSTsdefbat2_1w};
act12 anought_1w     = {TEXT, NORMAL_TPS_v2d,         kSTsNothing_1w};
act12 a115e_1w       = {TEXT, 0,                      kSTsDropMask_1w};
act12 aweartext_1w   = {TEXT, 0,                      kSTWearMask_1w};
act12 aremovetext_1w = {TEXT, 0,                      kSTRemoveMask_1w};
act12 abut6a_1w      = {TEXT, 0,                      kSTsButSniff_1w};
act12 abut6b_1w      = {TEXT, 3,                      kSTsButChop_1w};
act12 abut6c_1w      = {TEXT, 6,                      kSTsButHead_1w};
act12 abut9a_1w      = {TEXT, NORMAL_TPS_v2d / 3,     kSTsButEnjoy_1w};
act12 abut9b_1w      = {TEXT, NORMAL_TPS_v2d / 3 + 1, kSTsButTake_1w};
act12 abut11_1w      = {TEXT, NORMAL_TPS_v2d / 3,     kSTsButLater_1w};
act12 aeatchop_1w    = {TEXT, 0,                      kSTsEatChop_1w};
act12 achopfail_1w   = {TEXT, 0,                      kSTesthrown_1w};
act12 achopthrown_1w = {TEXT, 5 * NORMAL_TPS_v2d,     kSTsDogEat_1w};
act12 atalk1a_1w     = {TEXT, 0,                      kSTAskFrank_1w};
act12 atalk1b_1w     = {TEXT, 0,                      kSTRepFrank_1w};
act12 atalk2a_1w     = {TEXT, 0,                      kSTAskDrac_1w};
act12 atalk2b_1w     = {TEXT, 0,                      kSTRepDrac_1w};
act12 atalk3a_1w     = {TEXT, 0,                      kSTAskGwen_1w};
act12 atalk3b_1w     = {TEXT, 0,                      kSTRepGwen_1w};
act12 atalk4a_1w     = {TEXT, 0,                      kSTAskFriar_1w};
act12 atalk4b_1w     = {TEXT, 0,                      kSTRepFriar_1w};
act12 atalk5a_1w     = {TEXT, 0,                      kSTAskSlime_1w};
act12 atalk5b_1w     = {TEXT, 0,                      kSTRepSlime_1w};
act12 atalk6a_1w     = {TEXT, 0,                      kSTAskPea_1w};
act12 atalk6b_1w     = {TEXT, 0,                      kSTRepPea_1w};
act12 anoopen_1w     = {TEXT, 0,                      kSTsWrongCombo_1w};
act12 aopen4_1w      = {TEXT, 0,                      kSTsRightCombo_1w};
act12 ashedoil1_1w   = {TEXT, 0,                      kSTsTakeOil_1w};
act12 adoggy_1w      = {TEXT, 0,                      kSTsDogEatHero_1w};
act12 at78a_1w       = {TEXT, 0,                      kSTsTossMask_1w};
act12 aopenfail_1w   = {TEXT, 0,                      kSTsTrapBolted_1w};
act12 ahelps1_1w     = {TEXT, 0,                      kSTsBaseHelp1_1w};
act12 anohelp_1w     = {TEXT, 0,                      kSTsNoHelp_1w};
act12 ahelps2_1w     = {TEXT, 0,                      kSTsBaseHelp2_1w};
act12 abat5a_1w      = {TEXT, 0,                      kSTsGotcher_1w};
act12 abat5b_1w      = {TEXT, 0,                      kSTsBatGot_1w};
act12 amum3_1w       = {TEXT, 0,                      kSTsGotcher_1w};
act12 amum4_1w       = {TEXT, 0,                      kSTsMummyGot_1w};
act12 abin0_1w       = {TEXT, 0,                      kSTsEnterBoat_1w};
act12 abung1_1w      = {TEXT, 0,                      kSTsBoatHole_1w};
act12 ahout1_1w      = {TEXT, 0,                      kSTsExitBoat_1w};
act12 anodeboat_1w   = {TEXT, 0,                      kSTsManBlock_1w};
act12 amoving_1w     = {TEXT, 0,                      kSTsShutup_1w};
act12 anotcut_1w     = {TEXT, 0,                      kSTsBoatTied_1w};
act12 arepyep_1w     = {TEXT, 0,                      kSTsCorrect_1w};
act12 arepnop_1w     = {TEXT, 0,                      kSTsIncorrect_1w};
act12 amans1_1w      = {TEXT, 0,                      kSTsDoomed1_1w};
act12 arepno5_1w     = {TEXT, 0,                      kSTsDoomed2_1w};
act12 arepyep2_1w    = {TEXT, 0,                      kSTsContinue_1w};
act12 amans3_1w      = {TEXT, 0,                      kSTsOldMan1_1w};
act12 amans4_1w      = {TEXT, 0,                      kSTsOldMan2_1w};
act12 amans5_1w      = {TEXT, 0,                      kSTsOldMan3_1w};
act12 amans6_1w      = {TEXT, 0,                      kSTsOldMan4_1w};
act12 amans7_1w      = {TEXT, 0,                      kSTsOldMan5_1w};
act12 ajails1_1w     = {TEXT, 0,                      kSTsCongrats_1w};
act12 ajails2_1w     = {TEXT, 0,                      kSTsRescued1_1w};
act12 ajails3_1w     = {TEXT, 0,                      kSTsRescued2_1w};
act12 ajails4_1w     = {TEXT, 0,                      kSTsRescued3_1w};
act12 agive1_1w      = {TEXT, 0,                      kSTsGuard1_1w};
act12 agive2_1w      = {TEXT, 0,                      kSTsGuard2_1w};
act12 anogive_1w     = {TEXT, 0,                      kSTsNoGive_1w};
act12 bye1_1w        = {TEXT, 2 * NORMAL_TPS_v2d,     kSTsKissy_1w};
act12 bye2_1w        = {TEXT, 3 * NORMAL_TPS_v2d,     kSTsGoodbye_1w};
act12 admsg3_1w      = {TEXT, 0,                      kSTsGrip_1w};
act12 admsg2_1w      = {TEXT, 0,                      kSTsCoordinate_1w};
act12 admsg1_1w      = {TEXT, 0,                      kSTsReach_1w};
act12 alab12_1w      = {TEXT, 24,                     kSTsProf1_1w};
act12 alab13_1w      = {TEXT, 24,                     kSTsProf2_1w};
act12 abox2_1w       = {TEXT, 16,                     kSTsProf3_1w};
act12 abox3_1w       = {TEXT, 16,                     kSTsIgorRed_1w};
act12 abox5_1w       = {TEXT, 38,                     kSTsProfUpset_1w};
act12 abox6_1w       = {TEXT, 44,                     kSTsProfRetires_1w};
act12 ainorm_1w      = {TEXT, 0,                      kSTsIgorRefuses_1w};
act12 aigor22_1w     = {TEXT, 0,                      kSTsIgorGreen_1w};
act12 aigor13_1w     = {TEXT, 0,                      kSTsIgorYellow_1w};
act12 aigor32_1w     = {TEXT, 0,                      kSTsIgorBlue_1w};
act12 aigor0_1w      = {TEXT, 0,                      kSTsIgorNo_1w};
act12 agobox_1w      = {TEXT, 0,                      kSTsIgorBox_1w};

act13 aswapmask_1w = {SWAP_IMAGES, 0,  HERO, MONKEY_1w};
act13 aswaphero_1w = {SWAP_IMAGES, 0,  HERO, WHERO_1w};
act13 abox4_1w     = {SWAP_IMAGES, 20, HERO, WHERO_1w};
act13 aigor30_1w   = {SWAP_IMAGES, 30, HERO, FUZYHERO_1w};
act13 aigor31_1w   = {SWAP_IMAGES, 30, HERO, HERO};
act13 aigor20_1w   = {SWAP_IMAGES, 30, HERO, SPACHERO_1w};
act13 aigor21_1w   = {SWAP_IMAGES, 30, HERO, FUZYHERO_1w};
act13 aigor10_1w   = {SWAP_IMAGES, 30, HERO, WHERO_1w};
act13 aigor11_1w   = {SWAP_IMAGES, 30, HERO, SPACHERO_1w};

act14 acond9_1w     = {COND_SCR, 0, HERO, 9,  kALdefbats_1w,   kALnought_1w};
act14 acond5_1w     = {COND_SCR, 0, HERO, 5,  kALchasekit_1w,  kALcond9_1w};
act14 ablow1_1w     = {COND_SCR, 0, HERO, 1,  kALchasehall_1w, kALcond5_1w};
act14 athrowchop_1w = {COND_SCR, 0, HERO, 7,  kALchoppass_1w,  kALchopfail_1w};
act14 atakechop_1w  = {COND_SCR, 0, HERO, 7,  kALdoggy_1w,     0};
act14 achkscr_1w    = {COND_SCR, 0, HERO, 12, kALgive_1w,      kALnogive_1w};

act15 adogchop_1w = {AUTOPILOT, 0, DOG_1w, CHOP_1w, DX + 2, DY * 2};

act16 aclosedoor2_1w = {INIT_OBJ_SEQ, 12,                  DOOR2_1w, 0};
act16 aclosedoor3_1w = {INIT_OBJ_SEQ, 12,                  DOOR3_1w, 0};
act16 adogseq_1w     = {INIT_OBJ_SEQ, 0,                   DOG_1w,   1};
act16 adogseq2_1w    = {INIT_OBJ_SEQ, 4 * NORMAL_TPS_v2d,  DOG_1w,   2};
act16 adog5_1w       = {INIT_OBJ_SEQ, 0,                   DOG_1w,   0};
act16 at78c_1w       = {INIT_OBJ_SEQ, NORMAL_TPS_v2d + 12, TRAP_1w,  0};
act16 arock3_1w      = {INIT_OBJ_SEQ, 0,                   HERO,     RIGHT};
act16 arock5_1w      = {INIT_OBJ_SEQ, 11,                  HERO,     __UP};
act16 arock10_1w     = {INIT_OBJ_SEQ, 40,                  HERO,     __UP};
act16 arock12_1w     = {INIT_OBJ_SEQ, 44,                  HERO,     DOWN};
act16 acutrope_1w    = {INIT_OBJ_SEQ, 0,                   ROPE_1w,  1};
act16 abin1_1w       = {INIT_OBJ_SEQ, 0,                   BOAT_1w,  1};
act16 about1_1w      = {INIT_OBJ_SEQ, 0,                   BOAT_1w,  0};
act16 ajail3_1w      = {INIT_OBJ_SEQ, 0,                   HERO,     0};
act16 ajail4_1w      = {INIT_OBJ_SEQ, 2,                   HERO,     1};
act16 aguardgo3_1w   = {INIT_OBJ_SEQ, 0,                   GUARD_1w, 1};
act16 aturnguard_1w  = {INIT_OBJ_SEQ, 2 * NORMAL_TPS_v2d,  GUARD_1w, 0};
act16 alab4_1w       = {INIT_OBJ_SEQ, 0,                   PROF_1w,  0};
act16 alab10_1w      = {INIT_OBJ_SEQ, 14,                  PROF_1w,  1};
act16 alab11_1w      = {INIT_OBJ_SEQ, 18,                  IGOR_1w,  1};

act17 acupbpk_1w = {SET_STATE_BITS, 0, CUPBOARD_1w, 1};
act17 acupbdw_1w = {SET_STATE_BITS, 0, CUPBOARD_1w, 2};

act20 adef1_1w = {DEL_EVENTS, 1 * NORMAL_TPS_v2d, ASCHEDULE};

act21 abut6g_1w = {GAMEOVER, 7};
act21 adead5_1w = {GAMEOVER, 0};

// Those two were act7, with a type set to INIT_HH_COORD
act22 abut6e_1w   = {INIT_HH_COORD, 7, HDLSHERO_1w};
act22 adead2_1w   = {INIT_HH_COORD, 0, HERODEAD_1w};

act23 bye3_1w = {EXIT, 4 * NORMAL_TPS_v2d};

act24 abonus12_1w   = {BONUS, 0, 12};
act24 abonus1_1w    = {BONUS, 0, 1};
act24 abonus2_1w    = {BONUS, 0, 2};
act24 abonus6_1w    = {BONUS, 0, 6};
act24 achopbonus_1w = {BONUS, 0, 11};
act24 abonus3_1w    = {BONUS, 0, 3};
act24 abonus9_1w    = {BONUS, 0, 9};
act24 abonus4_1w    = {BONUS, 0, 4};
act24 abonus5_1w    = {BONUS, 0, 5};
act24 abonus7_1w    = {BONUS, 0, 7};
act24 abonus8_1w    = {BONUS, 0, 8};
// The following was defined as a act27 with a type set to BONUS
act24 abut7b_1w    = {BONUS,  0, 10};

act25 achkdoor_1w = {COND_BOX, 2 * NORMAL_TPS_v2d, HERO, 25,  173, 40,  175, kALgoinside_1w, 0};
act25 achkbbox_1w = {COND_BOX, 0,                  HERO, 100, 153, 144, 160, kALbbox_1w,     0};
act25 aichkbox_1w = {COND_BOX, 0,                  HERO, 100, 153, 144, 163, kALichk0_1w,    kALgobox_1w};

act26 acreak_1w    = {SOUND, 0,                  DOOR_CREAK_1w};
act26 abatsnd_1w   = {SOUND, 50,                 BAT_FLUTTER_1w};
act26 adooropen_1w = {SOUND, 0,                  DOOR_OPEN_1w};
act26 ahchase4_1w  = {SOUND, 5 * NORMAL_TPS_v2d, DOG_BARK_1w};
act26 abutsnd_1w   = {SOUND, 3,                  BUTLER_GOTCHER_1w};
act26 abut6h_1w    = {SOUND, 7,                  T_TRACK10};
act26 amunch_1w    = {SOUND, 0,                  MUNCH_1w};
act26 afork_1w     = {SOUND, 1 * NORMAL_TPS_v2d, FORK_BANG_1w};
act26 asong3_1w    = {SOUND, 0,                  DOG_BARK_1w};
act26 afuneral_1w  = {SOUND, 0,                  T_TRACK10};
act26 abatgot_1w   = {SOUND, 0,                  BAT_FLUTTER_1w};
act26 asong2_1w    = {SOUND, 3 * NORMAL_TPS_v2d, MUMMY_CHASE_1w};
act26 amumgot_1w   = {SOUND, 0,                  MUMMY_GOTCHER_1w};
act26 asplash_1w   = {SOUND, 0,                  SPLASH_1w};
act26 afinale_1w   = {SOUND, 0,                  T_TRACK1};
act26 abgsnd_1w    = {SOUND, 0,                  MACHINE_NOISE_1w};

act27 ashedoil4_1w = {ADD_SCORE, 0, OILCAN_1w};

act28 a115f_1w = {SUB_SCORE, 0, MASK_1w};

act29 achkmask_1w  = {COND_CARRY, 0, MASK_1w, kALputmask_1w, 0};
act29 achkmask2_1w = {COND_CARRY, 0, MASK_1w, kALridmask_1w, 0};

act43 abut4_1w       = {YESNO, 0,                       kSTsbut1_1w,       kALbutyes_1w,   kALbutno_1w};
act43 achopprompt_1w = {YESNO, 0,                       kSTsChopPrompt_1w, kALeatchop_1w,  0};
act43 ahelp1_1w      = {YESNO, 4 * 60 * NORMAL_TPS_v2d, kSTshelpp1_1w,     kALhelpy_1w,    kALhelpn_1w};
act43 ahelp2_1w      = {YESNO, 5 * 60 * NORMAL_TPS_v2d, kSTshelpp2_1w,     kALhelpy2_1w,   kALhelpn_1w};
act43 achkboat2_1w   = {YESNO, 0,                       kSTsBoatAsk_1w,    kALpushboat_1w, kALgetoutboat_1w};

act47 aviewfar_1w  = {INIT_VIEW, 107, BOAT_1w, 140, 55,  Common::KEYCODE_DOWN};
act47 aviewnear_1w = {INIT_VIEW, 13,  BOAT_1w, 250, 150, Common::KEYCODE_UP};

act48 ainitdoor_1w = {INIT_OBJ_FRAME, 0, DOOR1_1w, 0, 3};

actListPtr ALDummy[]           = {0};
actListPtr ALgoinside_1w[]    = {&aopendoor1b_1w, 0};
actListPtr ALopendoor1_1w[]   = {&acreak_1w, &aopendoor1a_1w, &achkdoor_1w, 0};
actListPtr ALclosedoor1_1w[]  = {&acreak_1w, &ainitdoor_1w, &aclosedoor1_1w, 0};
actListPtr ALightning_1w[]    = {&abk1_1w, &abknorm1_1w, &abk2_1w, &abknorm2_1w, &areplight_1w, 0};
actListPtr ALblinkeyes1_1w[]  = {&ablink1a_1w, &ablink1b_1w, &ablink1c_1w, &ablink1d_1w, &ablink2a_1w, &ablink2b_1w, &ablink2c_1w, &ablink2d_1w, &arepeye_1w, 0};
actListPtr ALbat_1w[]         = {&abatxy_1w, &abatvxy1_1w, &abatvxy2_1w, &abatvxy3_1w, &abatvxy4_1w, &abatvxy5_1w, &abatsnd_1w, &arepbat_1w, 0};
actListPtr ALpkin_1w[]        = {&abonus12_1w, &apbreak_1w, &ashowkey_1w, &aridpkin_1w, &adroppkin_1w, &amovekey_1w, 0};
actListPtr ALscr1_1w[]        = {&abonus1_1w, &aheroxy01_1w, &ascr01_1w, 0};
actListPtr ALridprof_1w[]     = {&aridprof_1w, 0};
actListPtr ALopendoor2_1w[]   = {&astophero_1w, &adooropen_1w, &aopendoor2_1w, &ast12_1w, &aheroxy12_1w, &aclosedoor2_1w, &ascr12_1w, 0};
actListPtr ALopendoor3_1w[]   = {&astophero_1w, &adooropen_1w, &aopendoor3_1w, &ast14_1w, &aheroxy14_1w, &aclosedoor3_1w, &ascr14_1w, 0};
actListPtr ALblinkeyes2_1w[]  = {&ablink3a_1w, &ablink3b_1w, &ablink3c_1w, &ablink3d_1w, &ablink4a_1w, &ablink4b_1w, &ablink4c_1w, &ablink4d_1w, &arepeye2_1w, 0};
actListPtr ALscr10_1w[]       = {&aheroxy10_1w, &ast01_1w, &ascr10_1w, 0};
actListPtr ALscr13_1w[]       = {&aheroxy13_1w, &ascr13_1w, 0};
actListPtr ALscr15_1w[]       = {&aheroxy15_1w, &ascr15_1w, 0};
actListPtr ALcuptxt0_1w[]     = {&acuptxt0_1w, &acupknife_1w, &acupwhist_1w, 0};
actListPtr ALcuptxt1_1w[]     = {&acuptxt1_1w, 0};
actListPtr ALcuptxt2_1w[]     = {&acuptxt2_1w, 0};
actListPtr ALcuptxt3_1w[]     = {&acuptxt3_1w, 0};
actListPtr ALlookcupb2_1w[]   = {&atcup2_1w, 0};
actListPtr ALlookcupb1_1w[]   = {&atcup1_1w, 0};
actListPtr ALlookcupb_1w[]    = {&abonus2_1w, &atcup0_1w, 0};
actListPtr ALcupbpk_1w[]      = {&acupbpk_1w, 0};
actListPtr ALcupbdw_1w[]      = {&acupbdw_1w, &adwwhy_1w, 0};
actListPtr ALchasehall_1w[]   = {&ablowt_1w, &ahchase1_1w, &ahchase2_1w, &ahchase3_1w, &ahchase4_1w, 0};
actListPtr ALchasekit_1w[]    = {&ablowt_1w, &akchase1_1w, &akchase2_1w, &akchase3_1w, &ahchase4_1w, 0};
actListPtr ALdefbats_1w[]     = {&abonus6_1w, &adef1_1w, &adef2_1w, &adef3_1w, &adef4_1w, &adef5_1w, &adef6_1w, &adef7_1w, &adef8_1w, 0};
actListPtr ALnought_1w[]      = {&anought_1w, 0};
actListPtr ALcond9_1w[]       = {&acond9_1w, 0};
actListPtr ALcond5_1w[]       = {&acond5_1w, 0};
actListPtr ALblowdw_1w[]      = {&ablow1_1w, 0};
actListPtr ALputmask_1w[]     = {&a115a_1w, &a115b_1w, &a115c_1w, &a115d_1w, &a115e_1w, &a115f_1w, &a115g_1w, 0};
actListPtr ALscr115_1w[]      = {&achkmask_1w, &aheroxy115_1w, &ascr115_1w, 0};
actListPtr ALopenwdoorm_1w[]  = {&aopenwd1_1w, &aopenwd2_1w, 0};
actListPtr ALopenwdoors_1w[]  = {&aopenwdoorl_1w, &aopenwdoorr_1w, &achkmaskcarry_1w, 0};
actListPtr ALclosewdoors_1w[] = {&aclosewdoorl_1w, &aclosewdoorr_1w, 0};
actListPtr ALswapmask_1w[]    = {&aswapmask_1w, 0};
actListPtr ALdropmask_1w[]    = {&adropmask_1w, 0};
actListPtr ALwearmask_1w[]    = {&aweartext_1w, &aswapmask_1w, &aworn_1w, 0};
actListPtr ALremovemask_1w[]  = {&aremovetext_1w, &aswapmask_1w, &aremoved_1w, 0};
actListPtr ALusemask_1w[]     = {&aswapcheck_1w, 0};
actListPtr ALscr21_1w[]       = {&aheroxy21_1w, &ascr21_1w, 0};
actListPtr ALbut_1w[]         = {&abut1_1w, &abutvxy1_1w, &abut2_1w, 0};
actListPtr ALbutler_1w[]      = {&abut3a_1w, 0};
actListPtr ALbutp_1w[]        = {/*&abut3_1w, */&abut4_1w, &abut4a_1w, 0};
actListPtr ALbutyes_1w[]      = {&abut5_1w, 0};
actListPtr ALbutno_1w[]       = {&abut6_1w, 0};
actListPtr ALbutchopped_1w[]  = {&abutsnd_1w, &abut6a_1w, &abut6b_1w, &abut6c_1w, &abut6d_1w, &abut6e_1w, &abut6f_1w, &abut6g_1w, &abut6h_1w, 0};
actListPtr ALbutchop_1w[]     = {&abut7_1w, &abut7a_1w, &abut7b_1w, &abut8_1w, &abut9_1w, &abut9a_1w, &abut9b_1w, 0};
actListPtr ALbutroam_1w[]     = {&abut8_1w, &abut9_1w, &abut10_1w, &abut11_1w, 0};
actListPtr AL_eatchop_1w[]    = {&adropchop_1w, &aeatchop_1w, &aridchop_1w, 0};
actListPtr ALeatchop_1w[]     = {&achopprompt_1w, 0};
actListPtr ALthrowchop_1w[]   = {&athrowchop_1w, 0};
actListPtr ALchopfail_1w[]    = {&adropchop_1w, &aridchop_1w, &achopfail_1w, 0};
actListPtr ALchoppass_1w[]    = {&ashowchop_1w, &achopthrown_1w, &adropchop_1w, &achopxy_1w, &adogchop_1w, &adogseq_1w, &adogcyc_1w, &adogseq2_1w, &achopbonus_1w, 0};
actListPtr ALrepredeye_1w[]   = {&ablink5a_1w, &ablink5b_1w, &ablink5c_1w, &ablink5d_1w, &arepredeye_1w, 0};
actListPtr ALreplips_1w[]     = {&amunch_1w, &alips_1w, &areplips_1w, 0};
actListPtr ALreparm_1w[]      = {&afork_1w, &aarm_1w, &areparm_1w, 0};
actListPtr ALtalkfrank_1w[]   = {&atalk1a_1w, &atalk1b_1w, 0};
actListPtr ALtalkdrac_1w[]    = {&atalk2a_1w, &atalk2b_1w, 0};
actListPtr ALtalkgwen_1w[]    = {&atalk3a_1w, &atalk3b_1w, 0};
actListPtr ALtalkhood_1w[]    = {&atalk4a_1w, &atalk4b_1w, 0};
actListPtr ALtalkslime_1w[]   = {&atalk5a_1w, &atalk5b_1w, 0};
actListPtr ALtalkpeahd_1w[]   = {&atalk6a_1w, &atalk6b_1w, 0};
actListPtr ALscr31_1w[]       = {&aheroxy31_1w, &ascr31_1w, 0};
actListPtr ALscr35_1w[]       = {&aheroxy35_1w, &ascr35_1w, 0};
actListPtr ALscr41_1w[]       = {&aheroxy41_1w, &ascr41_1w, 0};
actListPtr ALscr51_1w[]       = {&aheroxy51_1w, &ascr51_1w, 0};
actListPtr ALscr53_1w[]       = {&aheroxy53_1w, &ascr53_1w, 0};
actListPtr ALscr56_1w[]       = {&aheroxy56_1w, &ascr56_1w, 0};
actListPtr ALscr57_1w[]       = {&aheroxy57_1w, &ascr57_1w, 0};
actListPtr ALscr65_1w[]       = {&aheroxy65_1w, &ascr65_1w, 0};
actListPtr ALopenyes_1w[]     = {&abonus3_1w, &aopen4_1w, &adooropen_1w, &aopendoor4_1w, &astatedoor4_1w, 0};
actListPtr ALopenno_1w[]      = {&anoopen_1w, 0};
actListPtr ALopendoor4_1w[]   = {&aopenp_1w, 0};
actListPtr ALclosedoor4_1w[]  = {&aclosedoor4_1w, 0};
actListPtr ALshedoil_1w[]     = {&ashedoil1_1w, &ashedoil2_1w, &ashedoil3_1w, &ashedoil4_1w, 0};
actListPtr ALscr75_1w[]       = {&aheroxy75_1w, &ascr75_1w, 0};
actListPtr ALdog_1w[]         = {&asong3_1w, &adog1_1w, &adog2_1w, &adog3_1w, &adog4_1w, &adog5_1w, 0};
actListPtr ALdead_1w[]        = {&afuneral_1w, &adead1_1w, &adead2_1w, &adead3_1w, &adead4_1w, &adead5_1w, 0};
actListPtr ALdoggy_1w[]       = {&adead_1w, &adoggy_1w, 0};
actListPtr ALgetchop_1w[]     = {&atakechop_1w, 0};
actListPtr ALmovecarp_1w[]    = {&amovecarp1_1w, &amovecarp2_1w, &abonus9_1w, 0};
actListPtr ALridmask_1w[]     = {&at78a_1w, &at78b_1w, &adropmask_1w, 0};
actListPtr ALopenpass_1w[]    = {&abonus4_1w, &aopentrap_1w, &aheroxy78_1w, &ast78_1w, &at78c_1w, &aopen78_1w, &achkmask2_1w, 0};
actListPtr ALopenfail_1w[]    = {&aopenfail_1w, 0};
actListPtr ALopentrap_1w[]    = {&atrap_1w, 0};
actListPtr ALclosetrap_1w[]   = {&aclosetrap_1w, 0};
actListPtr ALscr89_1w[]       = {&abonus5_1w, &aheroxy89_1w, &ascr89_1w, 0};
actListPtr ALscr87_1w[]       = {&aheroxy87_1w, &ascr87_1w, 0};
actListPtr ALhelpy_1w[]       = {&ahelps1_1w, 0};
actListPtr ALhelpn_1w[]       = {&anohelp_1w, 0};
actListPtr ALhelpy2_1w[]      = {&ahelps2_1w, 0};
actListPtr ALhelp_1w[]        = {&ahelp1_1w, &ahelp2_1w, 0};
actListPtr ALscr910_1w[]      = {&aheroxy910_1w, &ascr910_1w, 0};
actListPtr ALscr98_1w[]       = {&aheroxy98_1w, &ascr98_1w, 0};
actListPtr ALbatrep_1w[]      = {&abata1a_1w, &abata1b_1w, &abata2a_1w, &abata2b_1w, &abata3a_1w, &abata3b_1w, &abata4a_1w, &abata4b_1w, &arepbata_1w, 0};
actListPtr ALbatattack_1w[]   = {&abata1a_1w, &abata1b_1w, &abata1c_1w, &abata2a_1w, &abata2b_1w, &abata2c_1w, &abata3a_1w, &abata3b_1w, &abata3c_1w, &abata4a_1w, &abata4b_1w, &abata4c_1w, &arepbata_1w, 0};
actListPtr ALbatty_1w[]       = {&abatgot_1w, &adead_1w, &abat5a_1w, &abat5b_1w, 0};
actListPtr ALbats_1w[]        = {&abatcheck_1w, 0};
actListPtr ALmum_1w[]         = {&acreak_1w, &asong2_1w, &amdoor1_1w, &amdoor2_1w, &amum1_1w, &amum2_1w, 0};
actListPtr ALmummy_1w[]       = {&adead_1w, &amumgot_1w, &amum3_1w, &amum4_1w, 0};
actListPtr ALrock_1w[]        = {&arock1_1w, &arock2_1w, &arock3_1w, &arock4_1w, &arock5_1w, &arock6_1w, &arock7_1w, &arock8_1w, &arock9_1w, &arock10_1w, &arock11_1w, &arock12_1w, &arock13_1w, &arock14_1w, &arock15_1w, &arock16_1w, 0};
actListPtr ALscr109_1w[]      = {&aheroxy109_1w, &ascr109_1w, 0};
actListPtr ALscr1011_1w[]     = {&aswaphero_1w, &aheroxy1011_1w, &ascr1011_1w, 0};
actListPtr ALscr1110_1w[]     = {&aswaphero_1w, &aheroxy1110_1w, &ascr1110_1w, 0};
actListPtr ALscr1112_1w[]     = {&aswaphero_1w, &aheroxy1112_1w, &ascr1112_1w, 0};
actListPtr ALcutrope_1w[]     = {&acutrope_1w, 0};
actListPtr ALherofar_1w[]     = {&aherofar_1w, 0};
actListPtr ALembark_1w[]      = {&abin0_1w, &ahin1_1w, &ahin2_1w, &abin1_1w, &abin2_1w, &abin3_1w, &asplash_1w, 0};
actListPtr ALnobung_1w[]      = {&abung1_1w, 0};
actListPtr ALgetinboat_1w[]   = {&abin_1w, 0};
actListPtr ALplugbung_1w[]    = {&aridbung_1w, &adropbung_1w, 0};
actListPtr ALdeboat_1w[]      = {&ahout_1w, &ahout1_1w, &about1_1w, &about2_1w, &about3_1w, &asplash_1w, 0};
actListPtr ALnodeboat_1w[]    = {&anodeboat_1w, 0};
actListPtr ALchkout_1w[]      = {&achkout_1w, 0};
actListPtr ALgetoutboat_1w[]  = {&agetout_1w, 0};
actListPtr ALgofar_1w[]       = {&aboatmov_1w,  &aboatvxy1_1w, &aboatvxy2_1w, &aboatvxy3_1w, &aboatvxy4_1w, &aboatvxy5_1w, &aboatvxy6_1w, &aboatvxy7_1w, &aboatvxy8_1w, &aboatvxy9_1w, &aherochk_1w, &aboatfar_1w,   &aviewfar_1w,  0};
actListPtr ALcomenear_1w[]    = {&aboatmov_1w,  &aboatvxy10_1w, &aboatvxy11_1w, &aboatvxy12_1w, &aheronear_1w, &aboatnear_1w, &aviewnear_1w, 0};
actListPtr ALmoveboat_1w[]    = {&apush_1w, 0};
actListPtr ALmoving_1w[]      = {&amoving_1w, 0};
actListPtr ALchkmove_1w[]     = {&achkmove_1w, 0};
actListPtr ALnotcut_1w[]      = {&anotcut_1w, 0};
actListPtr ALpushboat_1w[]    = {&achkrope_1w, 0};
actListPtr ALchkboat2_1w[]    = {&achkboat2_1w, 0};
actListPtr ALuseboat_1w[]     = {&achkboat1_1w, 0};
actListPtr ALrepno1_1w[]      = {&arepnop_1w, &amans1_1w, &aboatvxy9_1w, 0};
actListPtr ALrepno3_1w[]      = {&arepno5_1w, &aboatvxy9_1w, 0};
actListPtr ALrepyes1_1w[]     = {&arepyep_1w, &amanq2_1w, 0};
actListPtr ALrepyes2_1w[]     = {&arepyep_1w, &amanq3_1w, 0};
actListPtr ALrepyes3_1w[]     = {&arepyep_1w, &amanq4_1w, 0};
actListPtr ALrepyes4_1w[]     = {&amanq5_1w, 0};
actListPtr ALrepyes5_1w[]     = {&amanq6_1w, 0};
actListPtr ALrepyes6_1w[]     = {&amanq7_1w, 0};
actListPtr ALrepyes7_1w[]     = {&abonus7_1w, &arepyep2_1w, &aompass_1w, 0};
actListPtr ALomask_1w[]       = {&amans3_1w, &amans4_1w, &amans5_1w, &amans6_1w, &amanq1_1w, 0};
actListPtr ALomasked_1w[]     = {&amans7_1w, 0};
actListPtr ALoldman_1w[]      = {&achkpass_1w, 0};
actListPtr ALscr1213_1w[]     = {&aheroxy1213_1w, &ascr1213_1w, 0};
actListPtr ALjailrep_1w[]     = {&ajail3_1w, &ajail4_1w, &ajailrep_1w, 0};
actListPtr ALend_1w[]         = {&ajails1_1w, &ajails2_1w, &ajails3_1w, &ajails4_1w, 0};
actListPtr ALjail_1w[]        = {&afinale_1w, &ajail1_1w, &ajail2_1w, &ajailrep_1w, &aend_1w, &atheend1_1w, &atheend2_1w, 0};
actListPtr ALgive_1w[]        = {&abonus8_1w, &agive1_1w, &agive2_1w, &aguardgo1_1w, &aguardgo2_1w, &aguardgo3_1w, &aguardgo4_1w, &aturnguard_1w, 0};
actListPtr ALnogive_1w[]      = {&anogive_1w, 0};
actListPtr ALgold_1w[]        = {&achkscr_1w, 0};
actListPtr ALscr1211_1w[]     = {&aswaphero_1w, &aheroxy1211_1w, &ascr1211_1w, 0};
actListPtr ALgoodbye_1w[]     = {&bye1_1w, &bye2_1w, &bye3_1w, 0};
actListPtr ALok151_1w[]       = {&aheroxy151_1w, &ascr151_1w, 0};
actListPtr ALdmsg3_1w[]       = {&admsg3_1w, 0};
actListPtr ALdmsg2_1w[]       = {&admsg2_1w, 0};
actListPtr ALdmsg1_1w[]       = {&admsg1_1w, 0};
actListPtr ALchkd3_1w[]       = {&achkd3_1w, 0};
actListPtr ALchkd2_1w[]       = {&achkd2_1w, 0};
actListPtr ALchkd1_1w[]       = {&achkd1_1w, 0};
actListPtr ALscr151_1w[]      = {&achkd0_1w, 0};
actListPtr ALprof_1w[]        = {&alab1_1w, &alab2_1w, &alab3_1w, &alab4_1w, &alab5_1w, &alab6_1w, &alab7_1w, &alab8_1w, &alab9_1w, &alab10_1w, &alab11_1w, &alab12_1w, &alab13_1w, &alab14_1w, 0};
actListPtr ALlab_1w[]         = {&achklab_1w, 0};
actListPtr ALbox0_1w[]        = {&acycle_1w, &abox2_1w, &abox3_1w, &abox4_1w, &abox4a_1w, &abox5_1w, &abox6_1w, &abox8a_1w, &abox9_1w, &abox10_1w, &abox11_1w, &abox12_1w, 0};
actListPtr ALbbox_1w[]        = {&abbox_1w, 0};
actListPtr ALbox_1w[]         = {&achkbbox_1w, &arepbox_1w, 0};
actListPtr ALweird_1w[]       = {&abgsnd_1w, &abg1_1w, &abg2_1w, &abg3_1w, &abg4_1w, &abg5_1w, &abg6_1w, &abg7_1w, &abg8_1w, 0};
actListPtr ALcycle_1w[]       = {&abox0_1w, &abox1_1w, &acyc1_1w, &acyc2_1w, &aweird_1w, &abox7_1w, &abox8_1w, 0};
actListPtr ALinorm_1w[]       = {&ainorm_1w, 0};
actListPtr ALigor3_1w[]       = {&acycle_1w, &aigor30_1w, &aigor31_1w, &aigor32_1w, &aigor33_1w, 0};
actListPtr ALigor2_1w[]       = {&acycle_1w, &aigor20_1w, &aigor21_1w, &aigor22_1w, &aigor23_1w, 0};
actListPtr ALigor1_1w[]       = {&acycle_1w,  &aigor10_1w, &aigor11_1w, &aigor12_1w, &aigor13_1w, &aigor14_1w, 0};
actListPtr ALigor0_1w[]       = {&aigor0_1w, 0};
actListPtr ALgobox_1w[]       = {&agobox_1w, 0};
actListPtr ALichk3_1w[]       = {&aichk3_1w, 0};
actListPtr ALichk2_1w[]       = {&aichk2_1w, 0};
actListPtr ALichk1_1w[]       = {&aichk1_1w, 0};
actListPtr ALichk0_1w[]       = {&aichk0_1w, 0};
actListPtr ALigor_1w[]        = {&aichkbox_1w, 0};

actList actListArr_1w[] = {
	ALDummy,         ALgoinside_1w,    ALopendoor1_1w,  ALclosedoor1_1w, ALightning_1w,
	ALblinkeyes1_1w, ALbat_1w,         ALpkin_1w,       ALscr1_1w,       ALridprof_1w,
	ALopendoor2_1w,  ALopendoor3_1w,   ALblinkeyes2_1w, ALscr10_1w,      ALscr13_1w,
	ALscr15_1w,      ALcuptxt0_1w,     ALcuptxt1_1w,    ALcuptxt2_1w,    ALcuptxt3_1w,
	ALlookcupb2_1w,  ALlookcupb1_1w,   ALlookcupb_1w,   ALcupbpk_1w,     ALcupbdw_1w,
	ALchasehall_1w,  ALchasekit_1w,    ALdefbats_1w,    ALnought_1w,     ALcond9_1w,
	ALcond5_1w,      ALblowdw_1w,      ALputmask_1w,    ALscr115_1w,     ALopenwdoorm_1w,
	ALopenwdoors_1w, ALclosewdoors_1w, ALswapmask_1w,   ALdropmask_1w,   ALwearmask_1w,
	ALremovemask_1w, ALusemask_1w,     ALscr21_1w,      ALbut_1w,        ALbutler_1w,
	ALbutp_1w,       ALbutyes_1w,      ALbutno_1w,      ALbutchopped_1w, ALbutchop_1w,
	ALbutroam_1w,    AL_eatchop_1w,    ALeatchop_1w,    ALthrowchop_1w,  ALchopfail_1w,
	ALchoppass_1w,   ALrepredeye_1w,   ALreplips_1w,    ALreparm_1w,     ALtalkfrank_1w,
	ALtalkdrac_1w,   ALtalkgwen_1w,    ALtalkhood_1w,   ALtalkslime_1w,  ALtalkpeahd_1w,
	ALscr31_1w,      ALscr35_1w,       ALscr41_1w,      ALscr51_1w,      ALscr53_1w,
	ALscr56_1w,      ALscr57_1w,       ALscr65_1w,      ALopenyes_1w,    ALopenno_1w,
	ALopendoor4_1w,  ALclosedoor4_1w,  ALshedoil_1w,    ALscr75_1w,      ALdog_1w,
	ALdead_1w,       ALdoggy_1w,       ALgetchop_1w,    ALmovecarp_1w,   ALridmask_1w,
	ALopenpass_1w,   ALopenfail_1w,    ALopentrap_1w,   ALclosetrap_1w,  ALscr89_1w,
	ALscr87_1w,      ALhelpy_1w,       ALhelpn_1w,      ALhelpy2_1w,     ALhelp_1w,
	ALscr910_1w,     ALscr98_1w,       ALbatrep_1w,     ALbatattack_1w,  ALbatty_1w,
	ALbats_1w,       ALmum_1w,         ALmummy_1w,      ALrock_1w,       ALscr109_1w,
	ALscr1011_1w,    ALscr1110_1w,     ALscr1112_1w,    ALcutrope_1w,    ALherofar_1w,
	ALembark_1w,     ALnobung_1w,      ALgetinboat_1w,  ALplugbung_1w,   ALdeboat_1w,
	ALnodeboat_1w,   ALchkout_1w,      ALgetoutboat_1w, ALgofar_1w,      ALcomenear_1w,
	ALmoveboat_1w,   ALmoving_1w,      ALchkmove_1w,    ALnotcut_1w,     ALpushboat_1w,
	ALchkboat2_1w,   ALuseboat_1w,     ALrepno1_1w,     ALrepno3_1w,     ALrepyes1_1w,
	ALrepyes2_1w,    ALrepyes3_1w,     ALrepyes4_1w,    ALrepyes5_1w,    ALrepyes6_1w,
	ALrepyes7_1w,    ALomask_1w,       ALomasked_1w,    ALoldman_1w,     ALscr1213_1w,
	ALjailrep_1w,    ALend_1w,         ALjail_1w,       ALgive_1w,       ALnogive_1w,
	ALgold_1w,       ALscr1211_1w,     ALgoodbye_1w,    ALok151_1w,      ALdmsg3_1w,
	ALdmsg2_1w,      ALdmsg1_1w,       ALchkd3_1w,      ALchkd2_1w,      ALchkd1_1w,
	ALscr151_1w,     ALprof_1w,        ALlab_1w,        ALbox0_1w,       ALbbox_1w,
	ALbox_1w,        ALweird_1w,       ALcycle_1w,      ALinorm_1w,      ALigor3_1w,
	ALigor2_1w,      ALigor1_1w,       ALigor0_1w,      ALgobox_1w,      ALichk3_1w,
	ALichk2_1w,      ALichk1_1w,       ALichk0_1w,      ALigor_1w
};

// Hugo 2 Win
int dialrsp_2w[] = {kSTSdial2_2w, -1};
int whorsp_2w[] =  {kSTNobody_2w, kSTNo_one1_2w, kSTNo_one2_2w, kSTSharry_2w, -1};

act0 arepchk_2w   = {ASCHEDULE, 4,                    kALscr25_2w};
act0 aclimax_2w   = {ASCHEDULE, 20,                   kALclimax_2w};
act0 aclue09_2w   = {ASCHEDULE, 200 * NORMAL_TPS_v2d, kALchkc09_2w};
act0 ahdrink10_2w = {ASCHEDULE, 72,                   kALhfaint_2w};
act0 aschedbut_2w = {ASCHEDULE, 30,                   kALschedbut_2w};
act0 arepbuga_2w  = {ASCHEDULE, 3 * NORMAL_TPS_v2d,   kALbugrep1_2w};
act0 arepbugf_2w  = {ASCHEDULE, 2 * NORMAL_TPS_v2d,   kALbugrep2_2w};
act0 arepblah_2w  = {ASCHEDULE, 12 * NORMAL_TPS_v2d,  kALblah_2w};
act0 arepmsg1_2w  = {ASCHEDULE, 120 * NORMAL_TPS_v2d, kALrepmsg1_2w};

act1 aback1_2w       = {START_OBJ, 0,             CAT_2w,       0, INVISIBLE};
act1 aback2_2w       = {START_OBJ, 2,             CAT_2w,       0, NOT_CYCLING};
act1 aball3_2w       = {START_OBJ, 0,             BALLOON_2w,   0, INVISIBLE};
act1 abel1_2w        = {START_OBJ, 0,             BELL_2w,      0, NOT_CYCLING};
act1 acook7_2w       = {START_OBJ, 60,            COOKB_2w,     0, INVISIBLE};
act1 acook8_2w       = {START_OBJ, 60,            COOK_2w,      0, NOT_CYCLING};
act1 acop1_2w        = {START_OBJ, 0,             COP_2w,       0, CYCLE_FORWARD};
act1 acop3_2w        = {START_OBJ, 12,            COP_2w,       0, INVISIBLE};
act1 adone12_2w      = {START_OBJ, 10,            HERO,         0, CYCLE_FORWARD};
act1 adone5_2w       = {START_OBJ, 0,             HORACE_2w,    0, CYCLE_FORWARD};
act1 adyn2_2w        = {START_OBJ, 0,             DYNAMITE_2w,  0, NOT_CYCLING};
act1 afall2_2w       = {START_OBJ, 0,             PENFALL_2w,   0, CYCLE_FORWARD};
act1 afuze2_2w       = {START_OBJ, 0,             DYNAMITE_2w,  0, CYCLE_FORWARD};
act1 agenie2_2w      = {START_OBJ, 0,             GENIE_2w,     0, CYCLE_FORWARD};
act1 agiveb4_2w      = {START_OBJ, 2,             CAT_2w,       0, CYCLE_FORWARD};
act1 agiveb7_2w      = {START_OBJ, 0,             CAT_2w,       0, INVISIBLE};
act1 aglightoff1_2w  = {START_OBJ, 0,             GATELIGHT_2w, 0, INVISIBLE};
act1 aglighton1_2w   = {START_OBJ, 0,             GATELIGHT_2w, 0, NOT_CYCLING};
act1 ahdrink3_2w     = {START_OBJ, 0,             HESTER_2w,    0, CYCLE_FORWARD};
act1 ahdrink6_2w     = {START_OBJ, 70,            HESTER_2w,    0, NOT_CYCLING};
act1 ahest11_2w      = {START_OBJ, 47,            HESTER_2w,    0, NOT_CYCLING};
act1 ahest2_2w       = {START_OBJ, 0,             HESTER_2w,    0, CYCLE_FORWARD};
act1 ahfaint1_2w     = {START_OBJ, 4,             HERO,         0, INVISIBLE};
act1 ahfaint3_2w     = {START_OBJ, 4,             PENNYLIE_2w,  0, NOT_CYCLING};
act1 ahfaint4_2w     = {START_OBJ, 8,             PENNYLIE_2w,  0, INVISIBLE};
act1 ahfaint5_2w     = {START_OBJ, 8,             HERO,         0, NOT_CYCLING};
act1 akeyhole5_2w    = {START_OBJ, 0,             HERO,         0, INVISIBLE};
act1 amaidb2_2w      = {START_OBJ, 0,             MAID_2w,      0, CYCLE_FORWARD};
act1 amaidb5_2w      = {START_OBJ, 10,            MAID_2w,      0, INVISIBLE};
act1 amaidbk3_2w     = {START_OBJ, 0,             MAID_2w,      0, CYCLE_FORWARD};
act1 amaidbk7_2w     = {START_OBJ, 10,            MAID_2w,      0, NOT_CYCLING};
act1 amat2_2w        = {START_OBJ, 0,             MATCHES_2w,   0, NOT_CYCLING};
act1 amurd3_2w       = {START_OBJ, 30,            HERO,         0, NOT_CYCLING};
act1 aom15_2w        = {START_OBJ, 59,            OLDMAN_2w,    0, INVISIBLE};
act1 aom18_2w        = {START_OBJ, 60,            HERO,         0, NOT_CYCLING};
act1 aom19_2w        = {START_OBJ, 60,            OLDMAN_2w,    0, NOT_CYCLING};
act1 aomridlip_2w    = {START_OBJ, 49,            LIPS_2w,      0, INVISIBLE};
act1 aopendoor1_2w   = {START_OBJ, 0,             DOOR1_2w,     1, CYCLE_FORWARD};
act1 apaper2_2w      = {START_OBJ, 0,             PAPER_2w,     0, NOT_CYCLING};
act1 aridban_2w      = {START_OBJ, 0,             BANANA_2w,    0, INVISIBLE};
act1 aridcooklips_2w = {START_OBJ, 22,            LIPS_2w,      0, INVISIBLE};
act1 aridgard2_2w    = {START_OBJ, 0,             GARDENER_2w,  0, CYCLE_FORWARD};
act1 aridgard6_2w    = {START_OBJ, 20,            GARDENER_2w,  0, INVISIBLE};
act1 aridgarl_2w     = {START_OBJ, 0,             GARLIC_2w,    0, INVISIBLE};
act1 arobot5_2w      = {START_OBJ, 4,             ROBOT_2w,     0, CYCLE_FORWARD};
act1 aslightoff1_2w  = {START_OBJ, 0,             SHEDLIGHT_2w, 0, INVISIBLE};
act1 aslighton1_2w   = {START_OBJ, 0,             SHEDLIGHT_2w, 0, NOT_CYCLING};
act1 astick5_2w      = {START_OBJ, 0,             DOG_2w,       0, CYCLE_FORWARD};
act1 astick7_2w      = {START_OBJ, 16,            DOG_2w,       0, INVISIBLE};
act1 azapperoff1_2w  = {START_OBJ, 0,             ZAPPER_2w,    0, INVISIBLE};
act1 azapperon1_2w   = {START_OBJ, 0,             ZAPPER_2w,    0, NOT_CYCLING};
act1 adead1_2w       = {START_OBJ, 0,             HERO,         0, INVISIBLE};
act1 adead3_2w       = {START_OBJ, 0,             PENNYLIE_2w,  0, NOT_CYCLING};
act1 afaint2_2w      = {START_OBJ, 10,            HERO,         0, INVISIBLE};
act1 afaint4_2w      = {START_OBJ, 10,            PENNYLIE_2w,  0, NOT_CYCLING};
act1 afaint6_2w      = {START_OBJ, 30,            PENNYLIE_2w,  0, INVISIBLE};
act1 afaint7_2w      = {START_OBJ, 30,            HERO,         0, NOT_CYCLING};
act1 agone1_2w       = {START_OBJ, 23,            HERO,         0, INVISIBLE};
act1 agone14_2w      = {START_OBJ, 129,           HERO,         0, CYCLE_FORWARD};
act1 agone2_2w       = {START_OBJ, 120,           PENNYLIE_2w,  0, INVISIBLE};
act1 agone4_2w       = {START_OBJ, 120,           HERO,         0, NOT_CYCLING};
act1 aharry1_2w      = {START_OBJ, 0,             HARRY_2w,     0, NOT_CYCLING};
act1 aharry6_2w      = {START_OBJ, 4,             HARRY_2w,     0, CYCLE_FORWARD};
act1 amaidc10_2w     = {START_OBJ, 26,            MAID_2w,      0, INVISIBLE};
act1 amaidc14_2w     = {START_OBJ, 30,            MAID_2w,      0, NOT_CYCLING};
act1 amaidc5_2w      = {START_OBJ, 8,             MAID_2w,      0, CYCLE_FORWARD};
act1 aopendoor2_2w   = {START_OBJ, 0,             DOOR2_2w,     1, CYCLE_FORWARD};
act1 aopendoor3_2w   = {START_OBJ, 0,             DOOR3_2w,     1, CYCLE_FORWARD};
act1 apenblie1_2w    = {START_OBJ, 30,            PENNY_2w,     0, INVISIBLE};
act1 apenblie2_2w    = {START_OBJ, 30,            PENNYLIE_2w,  0, NOT_CYCLING};
act1 apenbstart_2w   = {START_OBJ, 0,             PENNY_2w,     0, CYCLE_FORWARD};
act1 apenbstop_2w    = {START_OBJ, 20,            PENNY_2w,     0, NOT_CYCLING};
act1 apeng1_2w       = {START_OBJ, 23,            HERO,         0, INVISIBLE};
act1 apeng3_2w       = {START_OBJ, 40,            HERO,         0, NOT_CYCLING};
act1 aridmaidlips_2w = {START_OBJ, 15,            LIPS_2w,      0, INVISIBLE};
act1 aherostart_2w   = {START_OBJ, 0,             HERO,         0, NOT_CYCLING};
act1 apenstart_2w    = {START_OBJ, PENDELAY + 10, PENNY_2w,     0, CYCLE_FORWARD};
act1 apenstop_2w     = {START_OBJ, PENDELAY + 70, PENNY_2w,     0, NOT_CYCLING};
act1 askip3_2w       = {START_OBJ, 0,             HERO,         0, NOT_CYCLING};

act2 aback3_2w      = {INIT_OBJXY, 2,         CAT_2w,      189, 69};
act2 abanana10_2w   = {INIT_OBJXY, 16,        GENIE_2w,    212, 10};
act2 abanana11_2w   = {INIT_OBJXY, 18,        GENIE_2w,    209, 20};
act2 abanana12_2w   = {INIT_OBJXY, 20,        GENIE_2w,    206, 40};
act2 abanana13_2w   = {INIT_OBJXY, 22,        GENIE_2w,    200, 65};
act2 abanana5_2w    = {INIT_OBJXY, 00,        GENIE_2w,    203, 60};
act2 abanana6_2w    = {INIT_OBJXY, 02,        GENIE_2w,    206, 40};
act2 abanana7_2w    = {INIT_OBJXY, 04,        GENIE_2w,    209, 20};
act2 abanana8_2w    = {INIT_OBJXY, 06,        GENIE_2w,    212, 10};
act2 abd1_2w        = {INIT_OBJXY, 0,         HERO,        214, 92};
act2 abd10_2w       = {INIT_OBJXY, 0,         HERO,        20,  92};
act2 abd20_2w       = {INIT_OBJXY, 0,         HERO,        20,  92};
act2 abed2_1_2w     = {INIT_OBJXY, 0,         HERO,        200, 95};
act2 abed3_1_2w     = {INIT_OBJXY, 0,         HERO,        60,  120};
act2 abel3_2w       = {INIT_OBJXY, 0,         BELL_2w,     185, 110};
act2 aboomxy_2w     = {INIT_OBJXY, 0,         PENNYLIE_2w, 120, 110};
act2 acatroom1_2w   = {INIT_OBJXY, 0,         HERO,        258, 98};
act2 achasm25_2w    = {INIT_OBJXY, 0,         HERO,        155, 20};
act2 adumb1_2w      = {INIT_OBJXY, 0,         HERO,        40,  107};
act2 adumb11_2w     = {INIT_OBJXY, 0,         HERO,        64,  103};
act2 afall1_2w      = {INIT_OBJXY, 0,         PENFALL_2w,  188, 95};
act2 agard3_2w      = {INIT_OBJXY, 0,         GARDENER_2w, 190, 90};
act2 agates1_2w     = {INIT_OBJXY, 0,         HERO,        279, 108};
act2 agates11_2w    = {INIT_OBJXY, 0,         HERO,        18,  107};
act2 agiveb6_2w     = {INIT_OBJXY, 2,         CAT_2w,      157, 96};
act2 ahall1_1_2w    = {INIT_OBJXY, 0,         HERO,        14,  92};
act2 ahall1_2_2w    = {INIT_OBJXY, 0,         HERO,        35,  130};
act2 ahall2_1_2w    = {INIT_OBJXY, 0,         HERO,        61,  105};
act2 ahall2_2_2w    = {INIT_OBJXY, 0,         HERO,        231, 120};
act2 ahall2_3_2w    = {INIT_OBJXY, 0,         HERO,        182, 122};
act2 ahall2_4_2w    = {INIT_OBJXY, 0,         HERO,        160, 140};
act2 ahall2_5_2w    = {INIT_OBJXY, 0,         HERO,        264, 117};
act2 ahall3_1_2w    = {INIT_OBJXY, 0,         HERO,        147, 112};
act2 ahall3_2_2w    = {INIT_OBJXY, 0,         HERO,        42,  41};
act2 aheroxy12_2w   = {INIT_OBJXY, DOORDELAY, HERO,        100, 125};
act2 ahest1_2w      = {INIT_OBJXY, 0,         HESTER_2w,   78,  114};
act2 ahestroom1_2w  = {INIT_OBJXY, 0,         HERO,        108, 76};
act2 ainshed1_2w    = {INIT_OBJXY, 0,         HERO,        140, 88};
act2 akennel1_2w    = {INIT_OBJXY, 0,         HERO,        283, 63};
act2 akit1_2w       = {INIT_OBJXY, 0,         HERO,        145, 120};
act2 akit11_2w      = {INIT_OBJXY, 0,         HERO,        40,  108};
act2 aladder1_2w    = {INIT_OBJXY, 0,         HERO,        282, 107};
act2 alamp1_2w      = {INIT_OBJXY, 0,         HERO,        26,  46};
act2 alignpen_2w    = {INIT_OBJXY, 0,         HERO,        52,  107};
act2 alounge1_2w    = {INIT_OBJXY, 0,         HERO,        232, 55};
act2 amaidbk8_2w    = {INIT_OBJXY, 12,        MAID_2w,     207, 99};
act2 amat4_2w       = {INIT_OBJXY, 0,         MATCHES_2w,  216, 146};
act2 amurd2_2w      = {INIT_OBJXY, 26,        MURDER_2w,   120, 86};
act2 amush1_2w      = {INIT_OBJXY, 0,         HERO,        272, 107};
act2 amush11_2w     = {INIT_OBJXY, 0,         HERO,        18,  107};
act2 amush21_2w     = {INIT_OBJXY, 0,         HERO,        95,  26};
act2 aom17_2w       = {INIT_OBJXY, 60,        OLDMAN_2w,   155, 95};
act2 aorgan1_2w     = {INIT_OBJXY, 0,         HERO,        241, 76};
act2 apanel2_2w     = {INIT_OBJXY, 0,         PANEL_2w,    189, 91};
act2 apanel4_2w     = {INIT_OBJXY, 20,        HERO,        192, 95};
act2 apaper4_2w     = {INIT_OBJXY, 0,         PAPER_2w,    138, 130};
act2 aparlor1_2w    = {INIT_OBJXY, 0,         HERO,        75,  75};
act2 aparlor2_2w    = {INIT_OBJXY, 0,         HERO,        86,  30};
act2 aparlor3_2w    = {INIT_OBJXY, 2,         HERO,        263, 114};
act2 aparty1b_2w    = {INIT_OBJXY, 0,         COP_2w,      80,  115};
act2 aparty2b_2w    = {INIT_OBJXY, 0,         MAID_2w,     100, 130};
act2 aparty3b_2w    = {INIT_OBJXY, 0,         HESTER_2w,   120, 120};
act2 aparty4b_2w    = {INIT_OBJXY, 0,         GARDENER_2w, 140, 120};
act2 aparty5b_2w    = {INIT_OBJXY, 0,         SNAKE_2w,    80,  150};
act2 aparty6b_2w    = {INIT_OBJXY, 0,         GENIE_2w,    70,  120};
act2 aparty7b_2w    = {INIT_OBJXY, 0,         HARRY_2w,    150, 130};
act2 aparty8b_2w    = {INIT_OBJXY, 0,         DOCTOR_2w,   170, 100};
act2 aparty9b_2w    = {INIT_OBJXY, 0,         COOK_2w,     200, 110};
act2 apass1_2w      = {INIT_OBJXY, 0,         HERO,        262, 45};
act2 apass11_2w     = {INIT_OBJXY, 0,         HERO,        18,  107};
act2 apen4_2w       = {INIT_OBJXY, 0,         HERO,        147, 112};
act2 aphone1_2w     = {INIT_OBJXY, 0,         HERO,        286, 108};
act2 aphone11c_2w   = {INIT_OBJXY, 0,         HERO,        146, 127};
act2 aphone11l_2w   = {INIT_OBJXY, 0,         HERO,        285, 87};
act2 aphone11r_2w   = {INIT_OBJXY, 0,         HERO,        18,  93};
act2 aretupxy_2w    = {INIT_OBJXY, 0,         HERO,        61,  81};
act2 aridgard3_2w   = {INIT_OBJXY, 4,         GARDENER_2w, 190, 88};
act2 arockg1_2w     = {INIT_OBJXY, 0,         HERO,        146, 122};
act2 arxy_2w        = {INIT_OBJXY, 0,         HERO,        75,  73};
act2 arxy21_2w      = {INIT_OBJXY, 0,         HERO,        205, 82};
act2 ascr33c_2w     = {INIT_OBJXY, 4,         HERO,        160, 105};
act2 ashed1_2w      = {INIT_OBJXY, 0,         HERO,        139, 139};
act2 ashed11_2w     = {INIT_OBJXY, 0,         HERO,        54,  89};
act2 ashed21_2w     = {INIT_OBJXY, 0,         HERO,        13,  124};
act2 asnake1_2w     = {INIT_OBJXY, 0,         HERO,        264, 79};
act2 asnake11_2w    = {INIT_OBJXY, 0,         HERO,        14,  66};
act2 asnake21_2w    = {INIT_OBJXY, 0,         HERO,        18,  107};
act2 astick3_2w     = {INIT_OBJXY, 0,         DOG_2w,      165, 80};
act2 astream1_2w    = {INIT_OBJXY, 0,         HERO,        283, 124};
act2 astream11_2w   = {INIT_OBJXY, 0,         HERO,        18,  107};
act2 athree1_2w     = {INIT_OBJXY, 0,         HERO,        268, 50};
act2 athree11_2w    = {INIT_OBJXY, 0,         HERO,        272, 69};
act2 athree21_2w    = {INIT_OBJXY, 0,         HERO,        19,  81};
act2 athree31_2w    = {INIT_OBJXY, 0,         HERO,        18,  107};
act2 atrap1_2w      = {INIT_OBJXY, 0,         HERO,        209, 35};
act2 atrap2_2w      = {INIT_OBJXY, 0,         HERO,        270, 87};
act2 auptrap1_2w    = {INIT_OBJXY, 0,         HERO,        193, 101};
act2 avenus1_2w     = {INIT_OBJXY, 0,         HERO,        275, 130};
act2 awell1_2w      = {INIT_OBJXY, 0,         HERO,        146, 131};
act2 awho3_2w       = {INIT_OBJXY, 0,         HERO,        255, 39};
act2 azap1_2w       = {INIT_OBJXY, 0,         HERO,        284, 110};
act2 azap11_2w      = {INIT_OBJXY, 0,         HERO,        25,  64};
act2 abuga1c_2w     = {INIT_OBJXY, 0,         BUG1_2w,     165, 25};
act2 abuga2c_2w     = {INIT_OBJXY, 0,         BUG2_2w,     265, 95};
act2 abuga3c_2w     = {INIT_OBJXY, 0,         BUG3_2w,     255, 110};
act2 abuga4c_2w     = {INIT_OBJXY, 0,         BUG4_2w,     300, 120};
act2 abuga5c_2w     = {INIT_OBJXY, 0,         BUG5_2w,     175, 130};
act2 abugf1c_2w     = {INIT_OBJXY, 0,         BUG1_2w,     65,  25};
act2 abugf2c_2w     = {INIT_OBJXY, 0,         BUG2_2w,     245, 85};
act2 abugf3c_2w     = {INIT_OBJXY, 0,         BUG3_2w,     155, 60};
act2 abugf4c_2w     = {INIT_OBJXY, 0,         BUG4_2w,     270, 25};
act2 abugf5c_2w     = {INIT_OBJXY, 0,         BUG5_2w,     175, 30};
act2 agone3_2w      = {INIT_OBJXY, 23,        HERO,        229, 119};
act2 aheroxy11_2w   = {INIT_OBJXY, 0,         HERO,        160, 100};
act2 aheroxy3435_2w = {INIT_OBJXY, DOORDELAY, HERO,        76,  133};
act2 aheroxy3436_2w = {INIT_OBJXY, DOORDELAY, HERO,        246, 120};
act2 amaidc12_2w    = {INIT_OBJXY, 30,        MAID_2w,     207, 99};
act2 amaidc3_2w     = {INIT_OBJXY, 8,         MAID_2w,     74,  78};
act2 amaidp2_2w     = {INIT_OBJXY, 0,         MAID_2w,     207, 99};
act2 amazexy_2w     = {INIT_OBJXY, 0,         HERO,        134, 92};
act2 apenbxy1_2w    = {INIT_OBJXY, 0,         PENNY_2w,    129, 119};
act2 apenbxy2_2w    = {INIT_OBJXY, 30,        PENNYLIE_2w, 215, 125};
act2 apeng2_2w      = {INIT_OBJXY, 23,        HERO,        192, 93};
act2 aheroxy01_2w   = {INIT_OBJXY, 0,         HERO,        169, 141};
act2 apenxy_2w      = {INIT_OBJXY, 0,         PENNY_2w,    109, 140};
act2 askip2_2w      = {INIT_OBJXY, 0,         HERO,        100, 100};

act3 aclimax7_2w = {PROMPT, 100, kSTSclimax7_2w, whorsp_2w,  kALcheat_2w, kALdidnt_2w,   false};
act3 adial_2w    = {PROMPT, 0,   kSTSdial1_2w,   dialrsp_2w, kALwho_2w,   kALnoreply_2w, false};
act3 asafepr_2w  = {PROMPT, 0,   kSTSafepr_2w,   dialrsp_2w, kALcomb1_2w, kALcomb2_2w,   false};

act5 aball2_2w     = {INIT_OBJVXY, 0,             BALLOON_2w,  0,       0};
act5 acop2_2w      = {INIT_OBJVXY, 0,             COP_2w,      DX,      0};
act5 adone11_2w    = {INIT_OBJVXY, 10,            HERO,        0,       -1};
act5 adone7_2w     = {INIT_OBJVXY, 0,             HORACE_2w,   -DX,     DY};
act5 adone8_2w     = {INIT_OBJVXY, 10,            HORACE_2w,   0,       -1};
act5 afall4_2w     = {INIT_OBJVXY, 0,             PENFALL_2w,  0,       4};
act5 ahest10_2w    = {INIT_OBJVXY, 46,            HESTER_2w,   0,       0};
act5 ahest4_2w     = {INIT_OBJVXY, 0,             HESTER_2w,   DX,      0};
act5 ahest6_2w     = {INIT_OBJVXY, 22,            HESTER_2w,   0,       -DY};
act5 ahest8_2w     = {INIT_OBJVXY, 24,            HESTER_2w,   -DX,     -1};
act5 amaidb3_2w    = {INIT_OBJVXY, 0,             MAID_2w,     DX,      0};
act5 amaidb4_2w    = {INIT_OBJVXY, 10,            MAID_2w,     0,       0};
act5 amaidbk4_2w   = {INIT_OBJVXY, 0,             MAID_2w,     -DX,     0};
act5 amaidbk5_2w   = {INIT_OBJVXY, 10,            MAID_2w,     0,       0};
act5 amaidstop2_2w = {INIT_OBJVXY, 0,             MAID_2w,     0,       0};
act5 aom3_2w       = {INIT_OBJVXY, 12,            HERO,        0,       0};
act5 aom8_2w       = {INIT_OBJVXY, 40,            HERO,        0,       0};
act5 apanel3_2w    = {INIT_OBJVXY, 5,             PANEL_2w,    0,       -4};
act5 apanel8_2w    = {INIT_OBJVXY, 25,            PANEL_2w,    0,       4};
act5 aridgard5_2w  = {INIT_OBJVXY, 4,             GARDENER_2w, -2 * DX, 0};
act5 arobot4_2w    = {INIT_OBJVXY, 4,             ROBOT_2w,    0,       0};
act5 astick6_2w    = {INIT_OBJVXY, 0,             DOG_2w,      -DX * 2, 0};
act5 abkstart1_2w  = {INIT_OBJVXY, 0,             BOOKCASE_2w, -2,      0};
act5 abkstart2_2w  = {INIT_OBJVXY, 23,            BOOKCASE_2w, +2,      0};
act5 abkstop_2w    = {INIT_OBJVXY, 46,            BOOKCASE_2w, 0,       0};
act5 adead4_2w     = {INIT_OBJVXY, 0,             HERO,        0,       0};
act5 agone15_2w    = {INIT_OBJVXY, 133,           HERO,        -DX,     0};
act5 amaidc1_2w    = {INIT_OBJVXY, 0,             HERO,        0,       0};
act5 amaidc6_2w    = {INIT_OBJVXY, 8,             MAID_2w,     DX,      0};
act5 amaidc8_2w    = {INIT_OBJVXY, 16,            MAID_2w,     -DX,     0};
act5 amaidc9_2w    = {INIT_OBJVXY, 26,            MAID_2w,     0,       0};
act5 apenbvxy1_2w  = {INIT_OBJVXY, 0,             PENNY_2w,    DX,      0};
act5 apenbvxy2_2w  = {INIT_OBJVXY, 20,            PENNY_2w,    0,       0};
act5 astophero_2w  = {INIT_OBJVXY, 0,             HERO,        0,       0};
act5 apenvxy1_2w   = {INIT_OBJVXY, PENDELAY + 10, PENNY_2w,    -DX,     0};
act5 apenvxy2_2w   = {INIT_OBJVXY, PENDELAY + 17, PENNY_2w,    0,       -DY};
act5 apenvxy3_2w   = {INIT_OBJVXY, PENDELAY + 42, PENNY_2w,    DX,      0};
act5 apenvxy4_2w   = {INIT_OBJVXY, PENDELAY + 70, PENNY_2w,    0,       0};

act6 abanana2_2w  = {INIT_CARRY, 0, BANANA_2w,  false};
act6 abel2_2w     = {INIT_CARRY, 0, BELL_2w,    false};
act6 adrop33a_2w  = {INIT_CARRY, 0, BOTTLE_2w,  false};
act6 adrop33b_2w  = {INIT_CARRY, 0, GUN_2w,     false};
act6 adrop33c_2w  = {INIT_CARRY, 0, BELL_2w,    false};
act6 adrop33d_2w  = {INIT_CARRY, 0, SCREW_2w,   false};
act6 adrop33e_2w  = {INIT_CARRY, 0, ALBUM_2w,   false};
act6 adrop33f_2w  = {INIT_CARRY, 0, WILL_2w,    false};
act6 adrop33g_2w  = {INIT_CARRY, 0, OILLAMP_2w, false};
act6 adrop33h_2w  = {INIT_CARRY, 0, CATNIP_2w,  false};
act6 adrop33i_2w  = {INIT_CARRY, 0, MAGNIFY_2w, false};
act6 adrop33j_2w  = {INIT_CARRY, 0, MATCHES_2w, false};
act6 adropban_2w  = {INIT_CARRY, 0, BANANA_2w,  false};
act6 adropgarl_2w = {INIT_CARRY, 0, GARLIC_2w,  false};
act6 amat3_2w     = {INIT_CARRY, 0, MATCHES_2w, false};
act6 apaper3_2w   = {INIT_CARRY, 0, PAPER_2w,   false};
act6 aphoto1_2w   = {INIT_CARRY, 0, ALBUM_2w,   true};
act6 asafe4_2w    = {INIT_CARRY, 0, WILL_2w,    true};
act6 asonic5_2w   = {INIT_CARRY, 0, SCREW_2w,   true};
act6 astick9_2w   = {INIT_CARRY, 0, STICK_2w,   false};

act7 agard2_2w     = {INIT_HF_COORD, 0,  GARDENER_2w};
act7 agetgarlic_2w = {INIT_HF_COORD, 0,  GARLIC_2w};
act7 agetmatch_2w  = {INIT_HF_COORD, 0,  MATCHES_2w};
act7 ahfaint2_2w   = {INIT_HF_COORD, 4,  PENNYLIE_2w};
act7 adead2_2w     = {INIT_HF_COORD, 0,  PENNYLIE_2w};
act7 afaint3_2w    = {INIT_HF_COORD, 10, PENNYLIE_2w};
act7 apenscr_2w    = {INIT_HF_COORD, 0,  PENNY_2w};

act8 akaboom3_2w   = {NEW_SCREEN, 0,               22};
act8 arg_2w        = {NEW_SCREEN, 0,               22};
act8 arr_2w        = {NEW_SCREEN, 0,               21};
act8 arr21_2w      = {NEW_SCREEN, 0,               16};
act8 ascr0204_2w   = {NEW_SCREEN, 0,               4};
act8 ascr0305_2w   = {NEW_SCREEN, 0,               5};
act8 ascr0306_2w   = {NEW_SCREEN, 0,               6};
act8 ascr0402_2w   = {NEW_SCREEN, 30,              2};
act8 ascr0503_2w   = {NEW_SCREEN, 0,               3};
act8 ascr0603_2w   = {NEW_SCREEN, 0,               3};
act8 ascr0607_2w   = {NEW_SCREEN, 0,               7};
act8 ascr0631_2w   = {NEW_SCREEN, 0,               31};
act8 ascr0706_2w   = {NEW_SCREEN, 0,               6};
act8 ascr0708_2w   = {NEW_SCREEN, 0,               8};
act8 ascr0710_2w   = {NEW_SCREEN, 0,               10};
act8 ascr0807_2w   = {NEW_SCREEN, 0,               7};
act8 ascr0809_2w   = {NEW_SCREEN, 0,               9};
act8 ascr0811_2w   = {NEW_SCREEN, 0,               11};
act8 ascr0812_2w   = {NEW_SCREEN, 0,               12};
act8 ascr0908_2w   = {NEW_SCREEN, 0,               8};
act8 ascr1007_2w   = {NEW_SCREEN, 0,               7};
act8 ascr1108_2w   = {NEW_SCREEN, 0,               8};
act8 ascr1113_2w   = {NEW_SCREEN, 0,               13};
act8 ascr12_2w     = {NEW_SCREEN, DOORDELAY,       2};
act8 ascr1314_2w   = {NEW_SCREEN, 0,               14};
act8 ascr1413_2w   = {NEW_SCREEN, 0,               13};
act8 ascr1415_2w   = {NEW_SCREEN, 0,               15};
act8 ascr1514_2w   = {NEW_SCREEN, 0,               14};
act8 ascr1516_2w   = {NEW_SCREEN, 0,               16};
act8 ascr1517_2w   = {NEW_SCREEN, 0,               17};
act8 ascr1615_2w   = {NEW_SCREEN, 0,               15};
act8 ascr1715_2w   = {NEW_SCREEN, 0,               15};
act8 ascr1718_2w   = {NEW_SCREEN, 0,               18};
act8 ascr1720_2w   = {NEW_SCREEN, 0,               20};
act8 ascr1817_2w   = {NEW_SCREEN, 0,               17};
act8 ascr1819_2w   = {NEW_SCREEN, 0,               19};
act8 ascr1837_2w   = {NEW_SCREEN, 0,               37};
act8 ascr1918_2w   = {NEW_SCREEN, 0,               18};
act8 ascr2017_2w   = {NEW_SCREEN, 0,               17};
act8 ascr2223_2w   = {NEW_SCREEN, 0,               23};
act8 ascr2322_2w   = {NEW_SCREEN, 0,               22};
act8 ascr2324_2w   = {NEW_SCREEN, 0,               24};
act8 ascr2325_2w   = {NEW_SCREEN, 0,               25};
act8 ascr2326_2w   = {NEW_SCREEN, 0,               26};
act8 ascr2423_2w   = {NEW_SCREEN, 0,               23};
act8 ascr2523_2w   = {NEW_SCREEN, 0,               23};
act8 ascr2623_2w   = {NEW_SCREEN, 0,               23};
act8 ascr2627_2w   = {NEW_SCREEN, 0,               27};
act8 ascr2726_2w   = {NEW_SCREEN, 0,               26};
act8 ascr2827_2w   = {NEW_SCREEN, 0,               27};
act8 ascr2829_2w   = {NEW_SCREEN, 0,               29};
act8 ascr2928_2w   = {NEW_SCREEN, 0,               28};
act8 ascr2930_2w   = {NEW_SCREEN, 0,               30};
act8 ascr2931_2w   = {NEW_SCREEN, 0,               31};
act8 ascr2934_2w   = {NEW_SCREEN, 0,               34};
act8 ascr2938_2w   = {NEW_SCREEN, 0,               38};
act8 ascr3029_2w   = {NEW_SCREEN, 0,               29};
act8 ascr3106_2w   = {NEW_SCREEN, 2,               6};
act8 ascr3129_2w   = {NEW_SCREEN, 0,               29};
act8 ascr3132_2w   = {NEW_SCREEN, 0,               32};
act8 ascr3231_2w   = {NEW_SCREEN, 0,               31};
act8 ascr3334_2w   = {NEW_SCREEN, 0,               34};
act8 ascr33d_2w    = {NEW_SCREEN, 4,               33};
act8 ascr3429_2w   = {NEW_SCREEN, 0,               29};
act8 ascr3438_2w   = {NEW_SCREEN, 0,               38};
act8 ascr3534_2w   = {NEW_SCREEN, 0,               34};
act8 ascr3634_2w   = {NEW_SCREEN, 0,               34};
act8 ascr3718_2w   = {NEW_SCREEN, 0,               18};
act8 ascr3829_2w   = {NEW_SCREEN, 0,               29};
act8 ascr3834_2w   = {NEW_SCREEN, 0,               34};
act8 auptrap2_2w   = {NEW_SCREEN, 0,               28};
act8 ascr0203_2w   = {NEW_SCREEN, 40,              3};
act8 ascr11maze_2w = {NEW_SCREEN, 0,               MAZE_SCREEN + 59};// Enter maze
act8 ascr3435_2w   = {NEW_SCREEN, DOORDELAY,       35}; // Goto to organ
act8 ascr3436_2w   = {NEW_SCREEN, DOORDELAY,       36}; // Goto to hestroom
act8 ascrmaze_2w   = {NEW_SCREEN, 0,               11}; // Maze exit to gatesopn
act8 ascr01_2w     = {NEW_SCREEN, STORYDELAY + 10, 1};
act8 askip5_2w     = {NEW_SCREEN, 0,               3};

act9 abanana9_2w   = {INIT_OBJSTATE, 16,        GENIE_2w,    1};
act9 abite1_2w     = {INIT_OBJSTATE, 0,         SNAKE_2w,    1};
act9 acallp1_2w    = {INIT_OBJSTATE, 0,         TARDIS_2w,   1};
act9 acat2_2w      = {INIT_OBJSTATE, 0,         CATNIP_2w,   1};
act9 acook6_2w     = {INIT_OBJSTATE, 0,         COOK_2w,     2};
act9 acookp1_2w    = {INIT_OBJSTATE, 0,         COOK_2w,     1};
act9 adynamite1_2w = {INIT_OBJSTATE, 0,         DYNAMITE_2w, 1};
act9 afinito_2w    = {INIT_OBJSTATE, 0,         HERO,        1};
act9 afuze3_2w     = {INIT_OBJSTATE, 0,         WELL_2w,     1};
act9 agarl1_2w     = {INIT_OBJSTATE, 0,         GARLIC_2w,   1};
act9 agenie4_2w    = {INIT_OBJSTATE, 0,         OILLAMP_2w,  1};
act9 agotalbum_2w  = {INIT_OBJSTATE, 0,         CATNIP_2w,   2};
act9 ahdrink1_2w   = {INIT_OBJSTATE, 0,         LETTER_2w,   3};
act9 ahdrink7_2w   = {INIT_OBJSTATE, 70,        LETTER_2w,   2};
act9 ahest16_2w    = {INIT_OBJSTATE, 50,        LETTER_2w,   1};
act9 ahestd1_2w    = {INIT_OBJSTATE, 0,         LETTER_2w,   2};
act9 akeyhole3_2w  = {INIT_OBJSTATE, 0,         KEYHOLE_2w,  1};
act9 akeyhole4_2w  = {INIT_OBJSTATE, 0,         BOOK_2w,     2};
act9 amat1_2w      = {INIT_OBJSTATE, 0,         MATCHES_2w,  1};
act9 amissed1_2w   = {INIT_OBJSTATE, 0,         GUN_2w,      1};
act9 aom4_2w       = {INIT_OBJSTATE, 12,        OLDMAN_2w,   1};
act9 apanel1_2w    = {INIT_OBJSTATE, 0,         PANEL_2w,    1};
act9 apaper1_2w    = {INIT_OBJSTATE, 0,         PAPER_2w,    1};
act9 apaper5_2w    = {INIT_OBJSTATE, 0,         PAPER_2w,    0};
act9 aphoto5_2w    = {INIT_OBJSTATE, 0,         ALBUM_2w,    1};
act9 apushkey_2w   = {INIT_OBJSTATE, 0,         PENCIL_2w,   1};
act9 aridkey1_2w   = {INIT_OBJSTATE, 0,         PAPER_2w,    2};
act9 arobot6_2w    = {INIT_OBJSTATE, 4,         ROBOT_2w,    1};
act9 asafe2_2w     = {INIT_OBJSTATE, 0,         SAFE_2w,     1};
act9 asafe3_2w     = {INIT_OBJSTATE, 0,         WILL_2w,     1};
act9 asonic7_2w    = {INIT_OBJSTATE, 0,         DOCTOR_2w,   1};
act9 ast12_2w      = {INIT_OBJSTATE, DOORDELAY, DOOR1_2w,    0};
act9 astick2_2w    = {INIT_OBJSTATE, 0,         STICK_2w,    1};
act9 awho4_2w      = {INIT_OBJSTATE, 0,         TARDIS_2w,   2};
act9 amaidp9_2w    = {INIT_OBJSTATE, 12,        MAID_2w,     1};
act9 asetbk1_2w    = {INIT_OBJSTATE, 0,         BOOK_2w,     1};
act9 asetbk2_2w    = {INIT_OBJSTATE, 0,         BOOK_2w,     2};
act9 asetbk3_2w    = {INIT_OBJSTATE, 0,         BOOK_2w,     3};
act9 ast3435_2w    = {INIT_OBJSTATE, DOORDELAY, DOOR2_2w,    0};
act9 ast3436_2w    = {INIT_OBJSTATE, DOORDELAY, DOOR3_2w,    0};

act10 aball1_2w     = {INIT_PATH, 0,   BALLOON_2w,   AUTO,    0,      0};
act10 abanana14_2w  = {INIT_PATH, 24,  GENIE_2w,     WANDER,  DX,     DY};
act10 abanana4_2w   = {INIT_PATH, 0,   GENIE_2w,     AUTO,    0,      0};
act10 acook9_2w     = {INIT_PATH, 60,  COOK_2w,      WANDER,  DX,     DY};
act10 agard11_2w    = {INIT_PATH, 300, GARDENER_2w,  WANDER,  DX,     DY};
act10 agard4_2w     = {INIT_PATH, 16,  GARDENER_2w,  CHASE,   DX,     DY};
act10 agenie3_2w    = {INIT_PATH, 4,   GENIE_2w,     CHASE,   DX,     DY};
act10 ahfaint6_2w   = {INIT_PATH, 8,   HERO,         USER,    0,      0};
act10 akeyhole6_2w  = {INIT_PATH, 0,   HERO,         AUTO,    0,      0};
act10 amaidstop1_2w = {INIT_PATH, 0,   MAID_2w,      AUTO,    0,      0};// For phase 3
act10 aom2_2w       = {INIT_PATH, 12,  HERO,         AUTO,    0,      0};
act10 aom20_2w      = {INIT_PATH, 60,  HERO,         USER,    0,      0};
act10 apanel5_2w    = {INIT_PATH, 20,  HERO,         USER,    0,      0};
act10 aparty1c_2w   = {INIT_PATH, 0,   COP_2w,       CHASE,   DX,     DY};
act10 aparty2c_2w   = {INIT_PATH, 0,   MAID_2w,      WANDER,  DX,     DY};
act10 aparty3c_2w   = {INIT_PATH, 0,   HESTER_2w,    WANDER,  DX,     DY};
act10 aparty4c_2w   = {INIT_PATH, 0,   GARDENER_2w,  WANDER,  DX,     DY};
act10 aparty5c_2w   = {INIT_PATH, 0,   SNAKE_2w,     WANDER,  DX,     DY};
act10 aparty6c_2w   = {INIT_PATH, 0,   GENIE_2w,     WANDER,  DX,     DY};
act10 aparty7c_2w   = {INIT_PATH, 0,   HARRY_2w,     AUTO,    0,      0};
act10 aparty8c_2w   = {INIT_PATH, 0,   DOCTOR_2w,    WANDER,  DX,     DY};
act10 aparty9c_2w   = {INIT_PATH, 0,   COOK_2w,      WANDER,  DX,     DY};
act10 aridgard1_2w  = {INIT_PATH, 0,   GARDENER_2w,  AUTO,    0,      0};
act10 arobot3_2w    = {INIT_PATH, 0,   ROBOT_2w,     AUTO,    0,      0};
act10 arobot7_2w    = {INIT_PATH, 4,   DOCTOR_2w,    CHASE,   DX / 2, DY / 2};
act10 ascr33a_2w    = {INIT_PATH, 4,   HERO,         USER,    0,      0};
act10 asonic9_2w    = {INIT_PATH, 16,  DOCTOR_2w,    WANDER,  DX,     DY};
act10 astick8_2w    = {INIT_PATH, 16,  HERO,         USER,    0,      0};
act10 abuga1a_2w    = {INIT_PATH, 0,   BUG1_2w,      CHASE,   DX * 2, DY * 2};
act10 abuga1b_2w    = {INIT_PATH, 7,   BUG1_2w,      WANDER2, DX,     DY};
act10 abuga2a_2w    = {INIT_PATH, 0,   BUG2_2w,      CHASE,   DX,     DY};
act10 abuga2b_2w    = {INIT_PATH, 9,   BUG2_2w,      WANDER2, DX,     DY};
act10 abuga3a_2w    = {INIT_PATH, 0,   BUG3_2w,      CHASE,   DX * 2, DY * 2};
act10 abuga3b_2w    = {INIT_PATH, 6,   BUG3_2w,      WANDER2, DX,     DY};
act10 abuga4a_2w    = {INIT_PATH, 0,   BUG4_2w,      CHASE,   DX,     DY};
act10 abuga4b_2w    = {INIT_PATH, 10,  BUG4_2w,      WANDER2, DX * 2, DY * 2};
act10 abuga5a_2w    = {INIT_PATH, 0,   BUG5_2w,      CHASE,   DX,     DY};
act10 abuga5b_2w    = {INIT_PATH, 4,   BUG5_2w,      WANDER2, DX,     DY};
act10 abugf1b_2w    = {INIT_PATH, 10,  BUG1_2w,      WANDER2, DX,     DY};
act10 abugf2b_2w    = {INIT_PATH, 12,  BUG2_2w,      WANDER2, DX,     DY};
act10 abugf3b_2w    = {INIT_PATH, 5,   BUG3_2w,      WANDER2, DX,     DY};
act10 abugf4b_2w    = {INIT_PATH, 10,  BUG4_2w,      WANDER2, DX * 2, DY * 2};
act10 abugf5b_2w    = {INIT_PATH, 5,   BUG5_2w,      WANDER2, DX,     DY};
act10 acyc2_2w      = {INIT_PATH, 120, HERO,         USER,    0,      0};
act10 adisable_2w   = {INIT_PATH, 0,   HERO,         AUTO,    0,      0};
act10 afaint8_2w    = {INIT_PATH, 30,  HERO,         USER,    0,      0};
act10 amaid2_2w     = {INIT_PATH, 18,  MAID_2w,      CHASE,   DX,     0};
act10 amaidc15_2w   = {INIT_PATH, 30,  HERO,         USER,    0,      0};
act10 amaidgo_2w    = {INIT_PATH, 25,  HERO,         USER,    0,      0};
act10 apeng4_2w     = {INIT_PATH, 40,  HERO,         USER,    0,      0};
act10 askip4_2w     = {INIT_PATH, 0,   HERO,         USER,    0,      0};

act11 abutchk_2w    = {COND_R, 0, GARLIC_2w,   0,    kALcantpush_2w, 0};
act11 achkbite_2w   = {COND_R, 0, SNAKE_2w,    0,    kALnocure_2w,   kALcure_2w};
act11 achkc09_2w    = {COND_R, 0, GARLIC_2w,   0,    kALclue09_2w,   0};
act11 achkcook_2w   = {COND_R, 0, COOK_2w,     1,    kALcook_2w,     0};
act11 achkcookp_2w  = {COND_R, 0, COOK_2w,     0,    kALcookp_2w,    0};
act11 achkcop_2w    = {COND_R, 0, COP_2w,      0x1f, kALcop_2w,      0};
act11 achkcop2_2w   = {COND_R, 0, COP_2w,      0x1f, kALparty_2w,    0};
act11 achkdoc_2w    = {COND_R, 0, DOCTOR_2w,   0,    kALsonic_2w,    0};
act11 achkgive_2w   = {COND_R, 0, CATNIP_2w,   1,    kALgiveb3_2w,   kALgiveb4_2w};
act11 achkhero_2w   = {COND_R, 0, HERO,        1,    kALdone_2w,     0};
act11 achkhr1_2w    = {COND_R, 0, LETTER_2w,   0,    kALhrgreet_2w,  0};
act11 achkhr2_2w    = {COND_R, 0, LETTER_2w,   3,    0,           kALhprompt_2w};
act11 achkhr3_2w    = {COND_R, 0, LETTER_2w,   1,    kALrephest_2w,  0};
act11 achkken1_2w   = {COND_R, 0, STICK_2w,    1,    kALthrown_2w,   kALdog1_2w};
act11 achkken2_2w   = {COND_R, 0, DYNAMITE_2w, 0,    kALdyn1_2w,     kALempty_2w};
act11 achkkit_2w    = {COND_R, 0, GENIE_2w,    1,    kALscrok_2w,    kALstuck_2w};
act11 achkld1_2w    = {COND_R, 0, MATCHES_2w,  1,    kALsoggy_2w,    kALmatok_2w};
act11 achkmat2_2w   = {COND_R, 0, MATCHES_2w,  0,    kALdropmat_2w,  0};
act11 achknasty_2w  = {COND_R, 0, BOTTLE_2w,   0,    kALnasty_2w,    0};
act11 achkom_2w     = {COND_R, 0, OLDMAN_2w,   0,    kALomgag_2w,    0};
act11 achkpanel_2w  = {COND_R, 0, PANEL_2w,    0,    kALpanel_2w,    0};
act11 achkpaper1_2w = {COND_R, 0, PAPER_2w,    1,    kALchkpap2_2w,  kALridkey_2w};
act11 achkpb1_2w    = {COND_R, 0, TARDIS_2w,   0,    kALcallp_2w,    0};
act11 achkpb2_2w    = {COND_R, 0, TARDIS_2w,   0,    kALphonebox_2w, 0};
act11 achkpb3_2w    = {COND_R, 0, TARDIS_2w,   1,    kALdial_2w,     0};
act11 achkpb4_2w    = {COND_R, 0, TARDIS_2w,   2,    kALdialed_2w,   0};
act11 achkphoto_2w  = {COND_R, 0, ALBUM_2w,    0,    kALphoto1_2w,   kALempty_2w};
act11 achkrobot_2w  = {COND_R, 0, ROBOT_2w,    1,    kALchkdoc_2w,   0};
act11 achkrr1_2w    = {COND_R, 0, WELL_2w,     1,    kALchkrr2_2w,   kALclimbup_2w};
act11 achksnake_2w  = {COND_R, 0, SNAKE_2w,    0,    kALbite_2w,     0};
act11 achktrap_2w   = {COND_R, 0, GENIE_2w,    1,    kALuptrap_2w,   kALnotrap_2w};
act11 achkwell_2w   = {COND_R, 0, WELL_2w,     0,    kALrr_2w,       kALrg_2w};
act11 achkwill_2w   = {COND_R, 0, WILL_2w,     0,    kALchksafe_2w,  kALgotwill_2w};
act11 agard1_2w     = {COND_R, 0, GARLIC_2w,   0,    kALgard1_2w,    0};
act11 akeyhole_2w   = {COND_R, 0, BOOK_2w,     0,    kALkeyhole1_2w, kALkeyhole2_2w};
act11 amatchk2_2w   = {COND_R, 0, MATCHES_2w,  0,    kALstrike_2w,   kALsoggy_2w};
act11 achkbed1_2w   = {COND_R, 0, BOOK_2w,     0,    kALbed1_2w,     kALfaint_2w};
act11 achkmaid_2w   = {COND_R, 0, MAID_2w,     0,    kALmaidx_2w,    kALblah_2w};
act11 achkstate0_2w = {COND_R, 0, BOOK_2w,     0,    kALhugone_2w,   kALchkstate1_2w};
act11 achkstate1_2w = {COND_R, 0, BOOK_2w,     1,    kALhole_2w,     kALpengone_2w};

act12 alookbrg_2w  = {TEXT, 0,                       kSTLookBridge_2w};
act12 astuck1_2w   = {TEXT, 0,                       kSTDoorStuck_2w};
act12 atalkg_2w    = {TEXT, 0,                       kSTIgnore_2w};
act12 aball4_2w    = {TEXT, 2,                       kSTBalloon1_2w};
act12 aball5_2w    = {TEXT, 2,                       kSTBalloon2_2w};
act12 aball6_2w    = {TEXT, 2,                       kSTBalloon3_2w};
act12 abanana1_2w  = {TEXT, 0,                       kSTBanana1_2w};
act12 abanana3_2w  = {TEXT, 0,                       kSTBanana2_2w};
act12 abell_2w     = {TEXT, 0,                       kSTBell1_2w};
act12 abell1_2w    = {TEXT, 8,                       kSTMaid7_2w};
act12 abite2_2w    = {TEXT, 0,                       kSTSnake5_2w};
act12 ablah_2w     = {TEXT, 8,                       kSTBlah_2w};
act12 aboom_2w     = {TEXT, 0,                       kSTDyn4_2w};
act12 acallp2_2w   = {TEXT, 0,                       kSTCall1_2w};
act12 acallp3_2w   = {TEXT, 0,                       kSTCall2_2w};
act12 acallp4_2w   = {TEXT, 0,                       kSTCall3_2w};
act12 acallp5_2w   = {TEXT, 0,                       kSTCall4_2w};
act12 acallp6_2w   = {TEXT, 0,                       kSTCall5_2w};
act12 acallp7_2w   = {TEXT, 0,                       kSTCall6_2w};
act12 acantpush_2w = {TEXT, 0,                       kSTPush1_2w};
act12 acat1_2w     = {TEXT, 0,                       kSTCat3_2w};
act12 acat4_2w     = {TEXT, 0,                       kSTMaid8_2w};
act12 achasm1_2w   = {TEXT, 12,                      kSTChasm1_2w};
act12 acheat1_2w   = {TEXT, 0,                       kSTScheat1_2w};
act12 acheat2_2w   = {TEXT, 0,                       kSTScheat2_2w};
act12 aclimax1_2w  = {TEXT, 0,                       kSTSclimax1_2w};
act12 aclimax2_2w  = {TEXT, 8,                       kSTSclimax2_2w};
act12 aclimax3_2w  = {TEXT, 80,                      kSTSclimax3_2w};
act12 aclimax4_2w  = {TEXT, 80,                      kSTSclimax4_2w};
act12 aclimax5_2w  = {TEXT, 80,                      kSTSclimax5_2w};
act12 aclimax6_2w  = {TEXT, 80,                      kSTSclimax6_2w};
act12 aclue09a_2w  = {TEXT, 0,                       kSTSclue09a_2w};
act12 aclue09b_2w  = {TEXT, 0,                       kSTSclue09b_2w};
act12 aclue09c_2w  = {TEXT, 0,                       kSTSclue09c_2w};
act12 acomb1_2w    = {TEXT, 0,                       kSTScomb1_2w};
act12 acomb2_2w    = {TEXT, 0,                       kSTScomb2_2w};
act12 acook1_2w    = {TEXT, 8,                       kSTCook1_2w};
act12 acook2_2w    = {TEXT, 8,                       kSTCook2_2w};
act12 acook3_2w    = {TEXT, 8,                       kSTCook3_2w};
act12 acook4_2w    = {TEXT, 24,                      kSTCook4_2w};
act12 acook5_2w    = {TEXT, 24,                      kSTCook5_2w};
act12 adialed_2w   = {TEXT, 0,                       kSTSdialed_2w};
act12 adidnt1_2w   = {TEXT, 0,                       kSTSdidnt1_2w};
act12 adidnt2_2w   = {TEXT, 0,                       kSTSdidnt2_2w};
act12 adog1_2w     = {TEXT, 0,                       kSTDyn2_2w};
act12 adone1_2w    = {TEXT, 10,                      kSTSdone1_2w};
act12 adone13_2w   = {TEXT, 50,                      kSTSdone5_2w};
act12 adone14_2w   = {TEXT, 50,                      kSTSdone6_2w};
act12 adone15_2w   = {TEXT, 50,                      kSTSdone7_2w};
act12 adone2_2w    = {TEXT, 10,                      kSTSdone2_2w};
act12 adone3_2w    = {TEXT, 10,                      kSTSdone3_2w};
act12 adone4_2w    = {TEXT, 10,                      kSTSdone4_2w};
act12 adraught_2w  = {TEXT, 5 * 60 * NORMAL_TPS_v2d, kSTDraught_2w};
act12 adropdyn2_2w = {TEXT, 0,                       kSTDyn3_2w};
act12 adumb12_2w   = {TEXT, 0,                       kSTDumb2_2w};
act12 adumb2_2w    = {TEXT, 0,                       kSTDumb1_2w};
act12 adyn1_2w     = {TEXT, 0,                       kSTDyn1_2w};
act12 aeatban_2w   = {TEXT, 0,                       kSTSeatbanana_2w};
act12 aeatgarl1_2w = {TEXT, 0,                       kSTSgarl1_2w};
act12 aeatgarl2_2w = {TEXT, 0,                       kSTSgarl2_2w};
act12 aexplainb_2w = {TEXT, 0,                       kSTSexplainb_2w};
act12 aext1_2w     = {TEXT, 0,                       kSTSrobot1_2w};
act12 aext2_2w     = {TEXT, 0,                       kSTSrobot2_2w};
act12 aext3_2w     = {TEXT, 0,                       kSTSrobot3_2w};
act12 agard10_2w   = {TEXT, 300,                     kSTSgard6_2w};
act12 agard5_2w    = {TEXT, 20,                      kSTSgard1_2w};
act12 agard6_2w    = {TEXT, 30,                      kSTSgard2_2w};
act12 agard7_2w    = {TEXT, 40,                      kSTSgard3_2w};
act12 agard8_2w    = {TEXT, 90,                      kSTSgard4_2w};
act12 agard9_2w    = {TEXT, 100,                     kSTSgard5_2w};
act12 agenie1_2w   = {TEXT, 0,                       kSTRub2_2w};
act12 agiveb3_2w   = {TEXT, 0,                       kSTCat2_2w};
act12 aglook1_2w   = {TEXT, 0,                       kSTGardInShed_2w};
act12 aglook2_2w   = {TEXT, 0,                       kSTGardShed_2w};
act12 agotwill_2w  = {TEXT, 0,                       kSTGotWill_2w};
act12 ahdrink11_2w = {TEXT, 82,                      kSTHest4_2w};
act12 ahdrink12_2w = {TEXT, 84,                      kSTSay1_2w};
act12 ahdrink13_2w = {TEXT, 84,                      kSTSay2_2w};
act12 ahdrink2_2w  = {TEXT, 0,                       kSTHest1_2w};
act12 ahdrink8_2w  = {TEXT, 72,                      kSTHest2_2w};
act12 ahdrink9_2w  = {TEXT, 72,                      kSTHest3_2w};
act12 ahest12_2w   = {TEXT, 8,                       kSTHest6_2w};
act12 ahest13_2w   = {TEXT, 12,                      kSTHest7_2w};
act12 ahest14_2w   = {TEXT, 16,                      kSTHest8_2w};
act12 ahest15_2w   = {TEXT, 50,                      kSTHest9_2w};
act12 ahestd2_2w   = {TEXT, 0,                       kSTHest5_2w};
act12 ahnod1_2w    = {TEXT, 0,                       kSTNod1_2w};
act12 ahnod2_2w    = {TEXT, 0,                       kSTSay1_2w};
act12 ahnod3_2w    = {TEXT, 0,                       kSTSay2_2w};
act12 akaboom1_2w  = {TEXT, 0,                       kSTDyn5_2w};
act12 amat5_2w     = {TEXT, 0,                       kSTMatch4_2w};
act12 amissed2_2w  = {TEXT, 0,                       kSTFire1_2w};
act12 amissed3_2w  = {TEXT, 0,                       kSTFire2_2w};
act12 amurd4_2w    = {TEXT, 10,                      kSTArgue1_2w};
act12 anobang_2w   = {TEXT, 0,                       kSTDyn7_2w};
act12 anobang2_2w  = {TEXT, 0,                       kSTDyn6_2w};
act12 anobell_2w   = {TEXT, 8,                       kSTBell2_2w};
act12 anogenie_2w  = {TEXT, 0,                       kSTRub1_2w};
act12 anoreply_2w  = {TEXT, 0,                       kSTBrrr_2w};
act12 anotrap_2w   = {TEXT, 0,                       kSTTrap1_2w};
act12 aom1_2w      = {TEXT, 8,                       kSTSom1_2w};
act12 aom10_2w     = {TEXT, 50,                      kSTSom4_2w};
act12 aom11_2w     = {TEXT, 50,                      kSTSom5_2w};
act12 aom12_2w     = {TEXT, 50,                      kSTSom6_2w};
act12 aom13_2w     = {TEXT, 54,                      kSTSom7_2w};
act12 aom14_2w     = {TEXT, 54,                      kSTSom8_2w};
act12 aom5_2w      = {TEXT, 16,                      kSTSom2_2w};
act12 aom6_2w      = {TEXT, 20,                      kSTSom3_2w};
act12 aom9_2w      = {TEXT, 40,                      kSTSom3a_2w};
act12 apen1_2w     = {TEXT, 0,                       kSTSpen1_2w};
act12 apen2_2w     = {TEXT, 0,                       kSTSpen2_2w};
act12 apen3_2w     = {TEXT, 0,                       kSTSpen3_2w};
act12 aphoto4_2w   = {TEXT, 0,                       kSTSphoto_2w};
act12 aphoto6_2w   = {TEXT, 0,                       kSTSphoto1_2w};
act12 apois1_2w    = {TEXT, 0,                       kSTSnake1_2w};
act12 apois2_2w    = {TEXT, 0,                       kSTSnake2_2w};
act12 apois3_2w    = {TEXT, 0,                       kSTSnake3_2w};
act12 apois4_2w    = {TEXT, 0,                       kSTSnake4_2w};
act12 aridkey2_2w  = {TEXT, 0,                       kSTSridkey_2w};
act12 arobot1_2w   = {TEXT, 0,                       kSTFire3_2w};
act12 arok_2w      = {TEXT, 0,                       kSTWell1_2w};
act12 arumbling_2w = {TEXT, 0,                       kSTRumble_2w};
act12 arup_2w      = {TEXT, 0,                       kSTDyn8_2w};
act12 asafe1_2w    = {TEXT, 0,                       kSTSsafe1_2w};
act12 ascr31_2w    = {TEXT, 0,                       kSTLock1_2w};
act12 aserum1_2w   = {TEXT, 0,                       kSTSserum1_2w};
act12 aserum2_2w   = {TEXT, 0,                       kSTSserum2_2w};
act12 asilly_2w    = {TEXT, 0,                       kSTDyn9_2w};
act12 asniff_2w    = {TEXT, 0,                       kSTCat1_2w};
act12 asoggy_2w    = {TEXT, 0,                       kSTMatch1_2w};
act12 asonic1_2w   = {TEXT, 0,                       kSTSsonic1_2w};
act12 asonic2_2w   = {TEXT, 0,                       kSTSsonic2_2w};
act12 asonic3_2w   = {TEXT, 0,                       kSTSsonic3_2w};
act12 asonic4_2w   = {TEXT, 0,                       kSTSsonic4_2w};
act12 astick1_2w   = {TEXT, 0,                       kSTWeee_2w};
act12 astrike_2w   = {TEXT, 0,                       kSTMatch2_2w};
act12 astung_2w    = {TEXT, 0,                       kSTStung_2w};
act12 awarn_2w     = {TEXT, 8,                       kSTSwarn_2w};
act12 awarnz_2w    = {TEXT, 8,                       kSTSwarnz_2w};
act12 awho1_2w     = {TEXT, 0,                       kSTTard1_2w};
act12 awho2_2w     = {TEXT, 0,                       kSTTard2_2w};
act12 awill1_2w    = {TEXT, 0,                       kSTSwill1_2w};
act12 awill2_2w    = {TEXT, 0,                       kSTSwill2_2w};
act12 awill3_2w    = {TEXT, 0,                       kSTSwill3_2w};
act12 awill4_2w    = {TEXT, 0,                       kSTSwill4_2w};
act12 abell2_2w    = {TEXT, 16,                      kSTMaid6_2w};
act12 abug5a_2w    = {TEXT, 0,                       kSTStingeroo_2w};
act12 abug5b_2w    = {TEXT, 0,                       kSTSbug5b_2w};
act12 aclick_2w    = {TEXT, 0,                       kSTClick_2w};
act12 aempty_2w    = {TEXT, 0,                       kSTEmpty_2w};
act12 afaint1_2w   = {TEXT, 5,                       kSTSfaint1_2w};
act12 afaint10_2w  = {TEXT, 35,                      kSTSfaint4_2w};
act12 afaint5_2w   = {TEXT, 20,                      kSTSfaint2_2w};
act12 afaint9_2w   = {TEXT, 35,                      kSTSfaint3_2w};
act12 agone10_2w   = {TEXT, 115,                     kSTSgone6_2w};
act12 agone11_2w   = {TEXT, 115,                     kSTSgone7_2w};
act12 agone5_2w    = {TEXT, 0,                       kSTSgone1_2w};
act12 agone6_2w    = {TEXT, 34,                      kSTSgone2_2w};
act12 agone7_2w    = {TEXT, 70,                      kSTSgone3_2w};
act12 agone8_2w    = {TEXT, 90,                      kSTSgone4_2w};
act12 agone9_2w    = {TEXT, 115,                     kSTSgone5_2w};
act12 aharry3_2w   = {TEXT, 4,                       kSTOrgan1_2w};
act12 aharry4_2w   = {TEXT, 4,                       kSTOrgan2_2w};
act12 aharry5_2w   = {TEXT, 4,                       kSTOrgan3_2w};
act12 aharry7_2w   = {TEXT, 8,                       kSTOrgan4_2w};
act12 ahole_2w     = {TEXT, 0,                       kSTFirst2_2w};
act12 akeyhole1_2w = {TEXT, 0,                       kSTHole1_2w};
act12 alie1_2w     = {TEXT, 13,                      kSTTired_2w};
act12 alie2_2w     = {TEXT, 18,                      kSTTired2_2w};
act12 amaid10_2w   = {TEXT, 90 * NORMAL_TPS_v2d,     kSTSmaid1_8_2w};
act12 amaid11_2w   = {TEXT, 99 * NORMAL_TPS_v2d,     kSTSmaid1_9_2w};
act12 amaid12_2w   = {TEXT, 0,                       kSTSmaid1_10_2w};
act12 amaid3_2w    = {TEXT, 4,                       kSTSmaid1_1_2w};
act12 amaid4_2w    = {TEXT, 17,                      kSTSmaid1_2_2w};
act12 amaid5_2w    = {TEXT, 17,                      kSTSmaid1_3_2w};
act12 amaid6_2w    = {TEXT, 17,                      kSTSmaid1_4_2w};
act12 amaid7_2w    = {TEXT, 30,                      kSTSmaid1_5_2w};
act12 amaid8_2w    = {TEXT, 30 * NORMAL_TPS_v2d,     kSTSmaid1_6_2w};
act12 amaid9_2w    = {TEXT, 60 * NORMAL_TPS_v2d,     kSTSmaid1_7_2w};
act12 amaidp3_2w   = {TEXT, 8,                       kSTMaid1_2w};
act12 amaidp4_2w   = {TEXT, 8,                       kSTMaid2_2w};
act12 amaidp5_2w   = {TEXT, 8,                       kSTMaid3_2w};
act12 amaidp7_2w   = {TEXT, 12,                      kSTMaid4_2w};
act12 amaidp8_2w   = {TEXT, 12,                      kSTMaid5_2w};
act12 anocarry_2w  = {TEXT, 0,                       kSTNocarry_2w};
act12 anopurps_2w  = {TEXT, 0,                       kSTNopurps_2w};
act12 aok_2w       = {TEXT, 0,                       kSTOkgen_2w};
act12 ascr21_2w    = {TEXT, 0,                       kSTSfirst_2w};
act12 astory_2w    = {TEXT, STORYDELAY,              kSTStory_2w};

act13 ascr33b_2w   = {SWAP_IMAGES, 4,   HERO, PENNY_2w};
act13 aswaphero_2w = {SWAP_IMAGES, 120, HERO, PENNY_2w};
act13 askip1_2w    = {SWAP_IMAGES, 0,   HERO, PENNY_2w};

act14 achkbell1_2w = {COND_SCR, 0, HERO,        31,        kALbell1_2w,    kALchkbell2_2w};
act14 achkbell2_2w = {COND_SCR, 0, HERO,        32,        kALbell2_2w,    kALnobell_2w};
act14 achkcat2_2w  = {COND_SCR, 0, HERO,        32,        kALbell2_2w,    kALcat5_2w};
act14 achkcat3_2w  = {COND_SCR, 0, HERO,        31,        kALcat6_2w,     0};
act14 achkgun_2w   = {COND_SCR, 0, HERO,        37,        kALshot_2w,     kALmissed_2w};
act14 achklamp_2w  = {COND_SCR, 0, HERO,        27,        kALgenie_2w,    kALnogenie_2w};
act14 achkld2_2w   = {COND_SCR, 0, HERO,        21,        kALchkld3_2w,   kALnobang_2w};
act14 achkrr_2w    = {COND_SCR, 0, HERO,        21,        kALkaboom_2w,   kALboom_2w};
act14 adropdyn1_2w = {COND_SCR, 0, HERO,        21,        kALdropdyn1_2w, 0};
act14 adumbchk_2w  = {COND_SCR, 0, HERO,        3,         kALscr0306_2w,  kALscr0603_2w};
act14 agarchk_2w   = {COND_SCR, 0, HERO,        9,         kALridgard_2w,  kALridgarl_2w};
act14 agiveb1_2w   = {COND_SCR, 0, HERO,        32,        kALgiveb2_2w,   kALnopurps_2w};
act14 aglchk_2w    = {COND_SCR, 0, GARDENER_2w, INSHED_2w, kALglook1_2w,   kALglchk2_2w};
act14 aglchk2_2w   = {COND_SCR, 0, GARDENER_2w, SHED_2w,   kALglook2_2w,   0};

act15 aom7_2w    = {AUTOPILOT, 20, HERO,    OLDMAN_2w, DX, DY};
act15 abugf1a_2w = {AUTOPILOT, 4,  BUG1_2w, ZAPPER_2w, DX, DY};
act15 abugf2a_2w = {AUTOPILOT, 6,  BUG2_2w, ZAPPER_2w, DX, DY};
act15 abugf3a_2w = {AUTOPILOT, 13, BUG3_2w, ZAPPER_2w, DX, DY};
act15 abugf4a_2w = {AUTOPILOT, 5,  BUG4_2w, ZAPPER_2w, DX, DY};
act15 abugf5a_2w = {AUTOPILOT, 11, BUG5_2w, ZAPPER_2w, DX, DY};

act16 aback4_2w      = {INIT_OBJ_SEQ, 2,             CAT_2w,      0};
act16 abd11_2w       = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 abd2_2w        = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 abd21_2w       = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 aclosedoor1_2w = {INIT_OBJ_SEQ, DOORDELAY,     DOOR1_2w,    0};
act16 adone10_2w     = {INIT_OBJ_SEQ, 10,            HERO,        __UP};
act16 adone6_2w      = {INIT_OBJ_SEQ, 0,             HORACE_2w,   LEFT};
act16 adone9_2w      = {INIT_OBJ_SEQ, 10,            HORACE_2w,   __UP};
act16 adumb13_2w     = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 adumb3_2w      = {INIT_OBJ_SEQ, 0,             HERO,        RIGHT};
act16 afuze1_2w      = {INIT_OBJ_SEQ, 0,             DYNAMITE_2w, 1};
act16 agiveb5_2w     = {INIT_OBJ_SEQ, 2,             CAT_2w,      1};
act16 ahall1_3_2w    = {INIT_OBJ_SEQ, 0,             HERO,        RIGHT};
act16 ahall2_2a_2w   = {INIT_OBJ_SEQ, 0,             HERO,        LEFT};
act16 ahall3_1a_2w   = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 ahdrink4_2w    = {INIT_OBJ_SEQ, 3,             HESTER_2w,   __UP};
act16 ahdrink5_2w    = {INIT_OBJ_SEQ, 70,            HESTER_2w,   DOWN};
act16 ahest3_2w      = {INIT_OBJ_SEQ, 0,             HESTER_2w,   RIGHT};
act16 ahest5_2w      = {INIT_OBJ_SEQ, 22,            HESTER_2w,   __UP};
act16 ahest7_2w      = {INIT_OBJ_SEQ, 24,            HESTER_2w,   LEFT};
act16 ahest9_2w      = {INIT_OBJ_SEQ, 45,            HESTER_2w,   DOWN};
act16 ainshed2_2w    = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 akit2_2w       = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 amaidb1_2w     = {INIT_OBJ_SEQ, 0,             MAID_2w,     RIGHT};
act16 amaidbk2_2w    = {INIT_OBJ_SEQ, 0,             MAID_2w,     LEFT};
act16 amaidbk6_2w    = {INIT_OBJ_SEQ, 10,            MAID_2w,     DOWN};
act16 amurd1_2w      = {INIT_OBJ_SEQ, 26,            MURDER_2w,   1};
act16 aom16_2w       = {INIT_OBJ_SEQ, 60,            OLDMAN_2w,   1};
act16 apanel7_2w     = {INIT_OBJ_SEQ, 5,             HERO,        DOWN};
act16 apen5_2w       = {INIT_OBJ_SEQ, 0,             HERO,        RIGHT};
act16 aridgard4_2w   = {INIT_OBJ_SEQ, 4,             GARDENER_2w, LEFT};
act16 arobot2_2w     = {INIT_OBJ_SEQ, 0,             ROBOT_2w,    2};
act16 ashed12_2w     = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 ashed2_2w      = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 ashed22_2w     = {INIT_OBJ_SEQ, 0,             HERO,        RIGHT};
act16 astick4_2w     = {INIT_OBJ_SEQ, 0,             DOG_2w,      1};
act16 avenus2_2w     = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 aclosedoor2_2w = {INIT_OBJ_SEQ, DOORDELAY,     DOOR2_2w,    0};
act16 aclosedoor3_2w = {INIT_OBJ_SEQ, DOORDELAY,     DOOR3_2w,    0};
act16 agone12_2w     = {INIT_OBJ_SEQ, 125,           HERO,        DOWN};
act16 agone13_2w     = {INIT_OBJ_SEQ, 127,           HERO,        LEFT};
act16 aharry2_2w     = {INIT_OBJ_SEQ, 0,             HARRY_2w,    1};
act16 aheroseq1_2w   = {INIT_OBJ_SEQ, 1,             HERO,        LEFT};
act16 amaid1_2w      = {INIT_OBJ_SEQ, 1,             MAID_2w,     DOWN};
act16 amaidc13_2w    = {INIT_OBJ_SEQ, 30,            MAID_2w,     DOWN};
act16 amaidc4_2w     = {INIT_OBJ_SEQ, 8,             MAID_2w,     RIGHT};
act16 amaidc7_2w     = {INIT_OBJ_SEQ, 16,            MAID_2w,     LEFT};
act16 amaidp6_2w     = {INIT_OBJ_SEQ, 10,            MAID_2w,     DOWN};
act16 apenbseq1_2w   = {INIT_OBJ_SEQ, 0,             PENNY_2w,    RIGHT};
act16 apenbseq2_2w   = {INIT_OBJ_SEQ, 25,            PENNY_2w,    __UP};
act16 apenseq1_2w    = {INIT_OBJ_SEQ, 0,             PENNY_2w,    RIGHT};
act16 apenseq2_2w    = {INIT_OBJ_SEQ, PENDELAY + 7,  PENNY_2w,    DOWN};
act16 apenseq3_2w    = {INIT_OBJ_SEQ, PENDELAY + 10, PENNY_2w,    LEFT};
act16 apenseq4_2w    = {INIT_OBJ_SEQ, PENDELAY + 17, PENNY_2w,    __UP};
act16 apenseq5_2w    = {INIT_OBJ_SEQ, PENDELAY + 42, PENNY_2w,    RIGHT};
act16 apenseq6_2w    = {INIT_OBJ_SEQ, PENDELAY + 74, PENNY_2w,    2};

act17 acopbit1_2w    = {SET_STATE_BITS,   0, COP_2w,    1};
act17 acopbit16_2w   = {SET_STATE_BITS,   0, COP_2w,    16};
act17 acopbit4_2w    = {SET_STATE_BITS,   0, COP_2w,    4};
act17 agatesopn_2w   = {SET_STATE_BITS,   0, BUTTON_2w, 32};
act17 aglighton2_2w  = {SET_STATE_BITS,   0, BUTTON_2w, 2};
act17 aphoto3_2w     = {SET_STATE_BITS,   0, COP_2w,    2};
act17 ashed23_2w     = {SET_STATE_BITS,   0, BUTTON_2w, 16};
act17 aslighton2_2w  = {SET_STATE_BITS,   0, BUTTON_2w, 1};
act17 astream12_2w   = {SET_STATE_BITS,   0, BUTTON_2w, 4};
act17 azapperon2_2w  = {SET_STATE_BITS,   0, BUTTON_2w, 8};
act17 acopbit8_2w    = {SET_STATE_BITS,   0, COP_2w,    8};

//all the act18 were defined as act17 with a type set to CLEAR_STATE_BITS
act18 agatescls_2w   = {CLEAR_STATE_BITS, 0, BUTTON_2w, 32};
act18 aglightoff2_2w = {CLEAR_STATE_BITS, 0, BUTTON_2w, 2};
act18 aslightoff2_2w = {CLEAR_STATE_BITS, 0, BUTTON_2w, 1};
act18 azapperoff2_2w = {CLEAR_STATE_BITS, 0, BUTTON_2w, 8};

act19 achkbugs_2w   = {TEST_STATE_BITS, 0, BUTTON_2w, 8,  kALbugflit_2w,   kALbugattack_2w};
act19 achkgates_2w  = {TEST_STATE_BITS, 2, BUTTON_2w, 16, kALswgates_2w,   kALrumbling_2w};
act19 achkglight_2w = {TEST_STATE_BITS, 2, BUTTON_2w, 2,  kALglightoff_2w, kALglighton_2w};
act19 achkgo_2w     = {TEST_STATE_BITS, 0, BUTTON_2w, 32, kALgoopen_2w,    kALgoclosed_2w};
act19 achkslight_2w = {TEST_STATE_BITS, 2, BUTTON_2w, 1,  kALslightoff_2w, kALslighton_2w};
act19 achksong_2w   = {TEST_STATE_BITS, 0, COP_2w,    8,  0,               kALsong4_2w};
act19 achkzapper_2w = {TEST_STATE_BITS, 2, BUTTON_2w, 4,  kALswzapper_2w,  kALclick_2w};
act19 aswgates_2w   = {TEST_STATE_BITS, 2, BUTTON_2w, 32, kALgatescls_2w,  kALgatesopn_2w};
act19 aswzapper_2w  = {TEST_STATE_BITS, 2, BUTTON_2w, 8,  kALzapperoff_2w, kALzapperon_2w};

act20 acure_2w     = {DEL_EVENTS, 0, AGSCHEDULE};
act20 aridpath_2w  = {DEL_EVENTS, 5, INIT_PATH};
act20 aridsched_2w = {DEL_EVENTS, 0, ASCHEDULE};
act20 aridtests_2w = {DEL_EVENTS, 0, TEST_STATE_BITS};
act20 aridtext_2w  = {DEL_EVENTS, 5, TEXT};

act21 afall5_2w = {GAMEOVER, 12};
act21 adead5_2w = {GAMEOVER, 0};

act23 adone16_2w = {EXIT,  54};

act24 abonus10_2w = {BONUS, 0,  10};
act24 abonus11_2w = {BONUS, 57, 11};
act24 abonus12_2w = {BONUS, 0,  12};
act24 abonus13_2w = {BONUS, 0,  13};
act24 abonus14_2w = {BONUS, 0,  14};
act24 abonus15_2w = {BONUS, 0,  15};
act24 abonus16_2w = {BONUS, 0,  16};
act24 abonus17_2w = {BONUS, 0,  17};
act24 abonus18_2w = {BONUS, 0,  18};
act24 abonus19_2w = {BONUS, 0,  19};
act24 abonus2_2w  = {BONUS, 0,  2};
act24 abonus20_2w = {BONUS, 0,  20};
act24 abonus21_2w = {BONUS, 0,  21};
act24 abonus22_2w = {BONUS, 0,  22};
act24 abonus24_2w = {BONUS, 0,  24};
act24 abonus25_2w = {BONUS, 0,  25};
act24 abonus26_2w = {BONUS, 0,  26};
act24 abonus27_2w = {BONUS, 0,  27};
act24 abonus30_2w = {BONUS, 0,  30};
act24 abonus6_2w  = {BONUS, 0,  6};
act24 abonus7_2w  = {BONUS, 0,  7};
act24 abonus8_2w  = {BONUS, 0,  8};
act24 abonus9_2w  = {BONUS, 0,  9};
act24 abonus1_2w  = {BONUS, 0,  1};
act24 abonus23_2w = {BONUS, 8,  23};
act24 abonus3_2w  = {BONUS, 0,  3};

act25 achkdrop_2w = {COND_BOX, 0, HERO,        196, 78,  283, 115, 0,         kALdropdyn2_2w};
act25 achkld3_2w  = {COND_BOX, 0, DYNAMITE_2w, 196, 78,  283, 125, kALchkld4_2w, kALnobang2_2w};
act25 achkph2_2w  = {COND_BOX, 0, MAID_2w,     192, 134, 251, 149, kALphoto2_2w, kALphoto3_2w};

act26 abark_2w     = {SOUND, 4,  BARK_2w};
act26 afallsnd_2w  = {SOUND, 0,  CHASM_SCREAM_2w};
act26 agatesnd_2w  = {SOUND, 0,  GATES_RUMBLE_2w};
act26 agunshot_2w  = {SOUND, 0,  GUNSHOT_2w};
act26 ahiss_2w     = {SOUND, 0,  HISS_2w};
act26 amurdsnd_2w  = {SOUND, 20, HORACE_SCREAM_2w};
act26 asong_l_2w   = {SOUND, 0,  T_TRACK6};
act26 asplash_2w   = {SOUND, 0,  DROP_MATCHES_2w};
act26 aurgh_2w     = {SOUND, 0,  URGH_2w};
act26 ading_2w     = {SOUND, 0,  DING_2w};
act26 afinale_2w   = {SOUND, 0,  T_TRACK1};
act26 ameow_2w     = {SOUND, 0,  MEOW_2w};
act26 asong2a_2w   = {SOUND, 0,  GENIE_APPEAR_2w};
act26 asong2dn_2w  = {SOUND, 30, PANEL_DN_2w};
act26 asong2up_2w  = {SOUND, 10, PANEL_UP_2w};
act26 asong5_2w    = {SOUND, 0,  BOOM_2w};
act26 asong6_2w    = {SOUND, 0,  BALLOON_POP_2w};
act26 abugsnd_2w   = {SOUND, 0,  STING_2w};
act26 aclicksnd_2w = {SOUND, 0,  CLICK_2w};
act26 adoorsnd_2w  = {SOUND, 0,  DOOR_CREAK_2w};
act26 ascream_2w   = {SOUND, 0,  SCREAM_2w};
act26 asong2_2w    = {SOUND, 0,  GET_BOOK_2w};
act26 asong4_2w    = {SOUND, 0,  T_HARRY_ORGAN_2w};
act26 asong_r_2w   = {SOUND, 0,  PLANET_2w};

act27 aphoto2_2w = {ADD_SCORE, 0, ALBUM_2w};
act27 asafe5_2w  = {ADD_SCORE, 0, WILL_2w};
act27 asonic6_2w = {ADD_SCORE, 0, SCREW_2w};

act28 abel4_2w     = {SUB_SCORE, 0, BELL_2w};
act28 amat6_2w     = {SUB_SCORE, 0, MATCHES_2w};
act28 asubban_2w   = {SUB_SCORE, 0, BANANA_2w};
act28 subgarlic_2w = {SUB_SCORE, 0, GARLIC_2w};

act29 achkbel_2w    = {COND_CARRY, 0, BELL_2w,     kALcat2_2w,     kALnocarry_2w};
act29 achkcarry_2w  = {COND_CARRY, 0, BELL_2w,     kALcat3_2w,     kALcat4_2w};
act29 achkld4_2w    = {COND_CARRY, 0, DYNAMITE_2w, kALnobang2_2w,  kALbang1_2w};
act29 achkmag_2w    = {COND_CARRY, 0, MAGNIFY_2w,  kALwill1_2w,    kALwill2_2w};
act29 achkmat1_2w   = {COND_CARRY, 0, MATCHES_2w,  kALchkmat2_2w,  kALchkroute_2w};
act29 achkpaper2_2w = {COND_CARRY, 0, PAPER_2w,    kALridkey_2w,   kALpencil_2w};
act29 achkrr3_2w    = {COND_CARRY, 0, DYNAMITE_2w, kALsilly_2w,    kALclimbup_2w};
act29 achksafe_2w   = {COND_CARRY, 0, SCREW_2w,    kALchkscrew_2w, kALsafepr_2w};

act30 ainitmaze_2w = {INIT_MAZE, 0, 8, 50, 37, 260, 154, 140, 175, MAZE_SCREEN};

act31 aexitmaze_2w = {EXIT_MAZE, 0};

act32 apanel6_2w = {INIT_PRIORITY, 20, PANEL_2w, BACKGROUND};

act33 acookp3_2w  = {INIT_SCREEN, 0,  COOKB_2w,     6};
act33 afall3_2w   = {INIT_SCREEN, 0,  PENFALL_2w,   25};
act33 amaidbk1_2w = {INIT_SCREEN, 0,  MAID_2w,      31};
act33 aparty1a_2w = {INIT_SCREEN, 0,  COP_2w,       30};
act33 aparty2a_2w = {INIT_SCREEN, 0,  MAID_2w,      30};
act33 aparty3a_2w = {INIT_SCREEN, 0,  HESTER_2w,    30};
act33 aparty4a_2w = {INIT_SCREEN, 0,  GARDENER_2w,  30};
act33 aparty5a_2w = {INIT_SCREEN, 0,  SNAKE_2w,     30};
act33 aparty6a_2w = {INIT_SCREEN, 0,  GENIE_2w,     30};
act33 aparty7a_2w = {INIT_SCREEN, 0,  HARRY_2w,     30};
act33 aparty8a_2w = {INIT_SCREEN, 0,  DOCTOR_2w,    30};
act33 aparty9a_2w = {INIT_SCREEN, 0,  COOK_2w,      30};
act33 ashed24_2w  = {INIT_SCREEN, 0,  GATELIGHT_2w, 11};
act33 ashed25_2w  = {INIT_SCREEN, 0,  GATELIGHT_2w, 12};
act33 astick10_2w = {INIT_SCREEN, 0,  STICK_2w,     -1};
act33 amaidc11_2w = {INIT_SCREEN, 30, MAID_2w,      31};
act33 amaidc2_2w  = {INIT_SCREEN, 8,  MAID_2w,      32};
act33 amaidp1_2w  = {INIT_SCREEN, 0,  MAID_2w,      31};

act34 abang1_2w   = {AGSCHEDULE, 6 * NORMAL_TPS_v2d,   kALbang2_2w};
act34 abite3_2w   = {AGSCHEDULE, 60 * NORMAL_TPS_v2d,  kALpois1_2w};
act34 abite4_2w   = {AGSCHEDULE, 200 * NORMAL_TPS_v2d, kALpois2_2w};
act34 abite5_2w   = {AGSCHEDULE, 290 * NORMAL_TPS_v2d, kALpois3_2w};
act34 abite6_2w   = {AGSCHEDULE, 300 * NORMAL_TPS_v2d, kALpois4_2w};
act34 acat3_2w    = {AGSCHEDULE, 8 * NORMAL_TPS_v2d,   kALchkcarry_2w};
act34 akaboom2_2w = {AGSCHEDULE, 1,                    kALkaboom3_2w};
act34 amaidb6_2w  = {AGSCHEDULE, 8 * NORMAL_TPS_v2d,   kALmaidbk_2w};

act35 amap0_2w = {REMAPPAL, 0, _TLIGHTMAGENTA, _TLIGHTMAGENTA};
act35 amap1_2w = {REMAPPAL, 0, _TLIGHTMAGENTA, _TBLACK};

act36 achkcat1_2w   = {COND_NOUN, 0, kNBell_2w,  kALcat1_2w,    kALnopurps_2w};
act36 achkpencil_2w = {COND_NOUN, 0, kNKey_2w,   kALchkpap1_2w, kALnopurps_2w};
act36 achkscrew_2w  = {COND_NOUN, 0, kNScrew_2w, kALsafe1_2w,   kALsafepr_2w};

act37 acookp2_2w   = {SCREEN_STATE, 0, 6,           1};
act37 asonic8_2w   = {SCREEN_STATE, 0, RETUPMOC_2w, 1};
act37 ascreen2_2w  = {SCREEN_STATE, 0, 2,           1};
act37 ascreen35_2w = {SCREEN_STATE, 0, 35,          1};

act38 acooklips_2w = {INIT_LIPS, 10, LIPS_2w, COOKB_2w,  LIPDX - 1, LIPDY + 1};
act38 aomlips_2w   = {INIT_LIPS, 41, LIPS_2w, OLDMAN_2w, 24,        LIPDY};
act38 alips_2w     = {INIT_LIPS, 0,  LIPS_2w, MAID_2w,   LIPDX,     LIPDY};
act38 amaidlips_2w = {INIT_LIPS, 5,  LIPS_2w, MAID_2w,   LIPDX,     LIPDY};

act43 abprompt_2w   = {YESNO, 0, kSTBananaPrompt_2w, kALeatban_2w,  0};
act43 agarprompt_2w = {YESNO, 0, kSTGarlicPrompt_2w, kAL_eatgar_2w, 0};
act43 ahestd3_2w    = {YESNO, 0, kSTShest1_2w,       kALhdrink_2w,  kALnodrink_2w};

act44 astopr_2w = {STOP_ROUTE, 0};

act45 acheatchk_2w = {COND_ROUTE, 0, 3, kALstopr_2w,   0};
act45 achkhr4_2w   = {COND_ROUTE, 0, 0, kALlignpen_2w, 0};
act45 achkroute_2w = {COND_ROUTE, 0, 0, kALlookbrg_2w, 0};
act45 achkrx_2w    = {COND_ROUTE, 0, 3, 0,             kALchkmat1_2w};

actListPtr AL11maze_2w[]        = {&ainitmaze_2w, &amazexy_2w, &ascr11maze_2w, 0};
actListPtr AL_eatgar_2w[]       = {&agarchk_2w, 0};
actListPtr ALballoon_2w[]       = {&aball1_2w, &aball2_2w, &aball3_2w, &aball4_2w, &aball5_2w, &aball6_2w, &asong6_2w, 0};
actListPtr ALbanana_2w[]        = {&abonus18_2w, &abanana1_2w, &abanana2_2w, &abanana3_2w, &abanana4_2w, &abanana5_2w, &abanana6_2w, &abanana7_2w, &abanana8_2w, &abanana9_2w, &abanana10_2w, &abanana11_2w, &abanana12_2w, &abanana13_2w, &abanana14_2w, 0};
actListPtr ALbang1_2w[]         = {&abang1_2w, &afuze1_2w, &afuze2_2w, &afuze3_2w, 0};
actListPtr ALbang2_2w[]         = {&achkrr_2w, 0};
actListPtr ALbed1_2w[]          = {&apenscr_2w, &apenbxy1_2w, &apenbseq1_2w, &apenbstart_2w, &apenbvxy1_2w, &apenbvxy2_2w, &apenbseq2_2w, &apenbstop_2w, &apenbxy2_2w, &apenblie1_2w, &apenblie2_2w, &alie1_2w, &alie2_2w, 0};
actListPtr ALbell1_2w[]         = {&ading_2w, &abell_2w, &abell1_2w, 0};
actListPtr ALbell2_2w[]         = {&ading_2w, &abell_2w, &adisable_2w, &amaidc1_2w, &amaidc2_2w, &amaidc3_2w, &amaidc4_2w, &amaidc5_2w, &amaidc6_2w, &abell2_2w, &amaidc7_2w, &amaidc8_2w, &amaidc9_2w, &amaidc10_2w, &amaidc11_2w, &amaidc12_2w, &amaidc13_2w, &amaidc14_2w, &amaidc15_2w, 0};
actListPtr ALbell_2w[]          = {&achkbell1_2w, 0};
actListPtr ALbite_2w[]          = {&ahiss_2w, &abite1_2w, &abite2_2w, &achknasty_2w, 0};
actListPtr ALblah_2w[]          = {&ablah_2w, &arepblah_2w, 0};
actListPtr ALboom_2w[]          = {&asong5_2w, &aboom_2w, &abonus17_2w, 0};
actListPtr ALbottle_2w[]        = {&abonus10_2w, &achkbite_2w, 0};
actListPtr ALbridge_2w[]        = {&achkrx_2w,    0};
actListPtr ALbugattack_2w[]     = {&abuga1a_2w, &abuga1b_2w, &abuga1c_2w, &abuga2a_2w, &abuga2b_2w, &abuga2c_2w, &abuga3a_2w, &abuga3b_2w, &abuga3c_2w, &abuga4a_2w, &abuga4b_2w, &abuga4c_2w, &abuga5a_2w, &abuga5b_2w, &abuga5c_2w, &arepbuga_2w, 0};
actListPtr ALbugflit_2w[]       = {&abugf1a_2w, &abugf1b_2w, &abugf1c_2w, &abugf2a_2w, &abugf2b_2w, &abugf2c_2w, &abugf3a_2w, &abugf3b_2w, &abugf3c_2w, &abugf4a_2w, &abugf4b_2w, &abugf4c_2w, &abugf5a_2w, &abugf5b_2w, &abugf5c_2w, &arepbugf_2w, 0};
actListPtr ALbugrep1_2w[]       = {&abuga1a_2w, &abuga1b_2w, &abuga2a_2w, &abuga2b_2w, &abuga3a_2w, &abuga3b_2w, &abuga4a_2w, &abuga4b_2w, &abuga5a_2w, &abuga5b_2w, &arepbuga_2w, 0};
actListPtr ALbugrep2_2w[]       = {&abugf1a_2w, &abugf1b_2w, &abugf2a_2w, &abugf2b_2w, &abugf3a_2w, &abugf3b_2w, &abugf4a_2w, &abugf4b_2w, &abugf5a_2w, &abugf5b_2w, &arepbugf_2w, 0};
actListPtr ALbugs_2w[]          = {&adead1_2w, &adead2_2w, &adead3_2w, &abugsnd_2w, &abug5a_2w, &abug5b_2w, &adead4_2w, &adead5_2w, 0};
actListPtr ALbugzapper_2w[]     = {&abutchk_2w, &achkzapper_2w, 0};
actListPtr ALcallp_2w[]         = {&acallp1_2w, &acallp2_2w, &acallp3_2w, &acallp4_2w, &acallp5_2w, &acallp6_2w, &acallp7_2w, &abonus13_2w, 0};
actListPtr ALcantpush_2w[]      = {&acantpush_2w, &aridtests_2w,   0};
actListPtr ALcat1_2w[]          = {&achkbel_2w, 0};
actListPtr ALcat2_2w[]          = {&acat1_2w, &acat2_2w, 0};
actListPtr ALcat3_2w[]          = {&aback1_2w, &aback2_2w, &aback3_2w, &aback4_2w, 0};
actListPtr ALcat4_2w[]          = {&aback1_2w, &aback2_2w, &aback3_2w, &aback4_2w, &achkcat2_2w, 0};
actListPtr ALcat5_2w[]          = {&achkcat3_2w, 0};
actListPtr ALcat6_2w[]          = {&aridsched_2w, &ading_2w, &abell_2w, &acat4_2w, &amaidb1_2w, &amaidb2_2w, &amaidb3_2w, &amaidb4_2w, &amaidb5_2w, &amaidb6_2w, 0};
actListPtr ALcatnip_2w[]        = {&achkcat1_2w, 0};
actListPtr ALchasm_2w[]         = {&afallsnd_2w, &achasm1_2w, &astopr_2w, &adead1_2w, &afall1_2w, &afall2_2w, &afall3_2w, &afall4_2w, &adead4_2w, &afall5_2w, 0};
actListPtr ALcheat_2w[]         = {&acheat1_2w, &acheat2_2w, &abonus25_2w, &ascr33a_2w, &ascr33b_2w, &ascr33c_2w, &ascr33d_2w, 0};
actListPtr ALchkbell2_2w[]      = {&achkbell2_2w, 0};
actListPtr ALchkc09_2w[]        = {&achkc09_2w, 0};
actListPtr ALchkcarry_2w[]      = {&achkcarry_2w, 0};		// If hero picked up bell_2w,  tough!
actListPtr ALchkdoc_2w[]        = {&achkdoc_2w,   0};
actListPtr ALchkld3_2w[]        = {&achkld3_2w, 0};
actListPtr ALchkld4_2w[]        = {&achkld4_2w, 0};
actListPtr ALchkmat1_2w[]       = {&achkmat1_2w,  0};
actListPtr ALchkmat2_2w[]       = {&achkmat2_2w,  &astopr_2w, 0};
actListPtr ALchkpap1_2w[]       = {&achkpaper1_2w, &apushkey_2w, 0};
actListPtr ALchkpap2_2w[]       = {&achkpaper2_2w, 0};
actListPtr ALchkroute_2w[]      = {&achkroute_2w, 0};
actListPtr ALchkrr2_2w[]        = {&achkrr3_2w, 0};
actListPtr ALchksafe_2w[]       = {&achksafe_2w, 0};
actListPtr ALchkscrew_2w[]      = {&achkscrew_2w, 0};
actListPtr ALchkstate1_2w[]     = {&achkstate1_2w, 0};
actListPtr ALclick_2w[]         = {&aclicksnd_2w, &aclick_2w, 0};
actListPtr ALclimax_2w[]        = {&aclimax1_2w, &aclimax2_2w, &aclimax3_2w, &aclimax4_2w, &aclimax5_2w, &aclimax6_2w, &aclimax7_2w, 0};
actListPtr ALclimbrope_2w[]     = {&achkrr1_2w, 0};
actListPtr ALclimbup_2w[]       = {&arxy21_2w, &arup_2w, &arr21_2w, 0};
actListPtr ALclimbwell_2w[]     = {&abonus16_2w, &achkwell_2w, 0};
actListPtr ALclue09_2w[]        = {&aclue09a_2w, &aclue09b_2w, &aclue09c_2w, 0};
actListPtr ALcomb1_2w[]         = {&acomb1_2w, 0};
actListPtr ALcomb2_2w[]         = {&acomb2_2w, 0};
actListPtr ALcook_2w[]          = {&acooklips_2w, &aridcooklips_2w, &acook1_2w, &acook2_2w, &acook3_2w, &acook4_2w, &acook5_2w, &acook6_2w, &acopbit4_2w, &acook7_2w, &acook8_2w, &acook9_2w, 0};
actListPtr ALcookp_2w[]         = {&abonus22_2w, &acookp1_2w, &acookp2_2w, &acookp3_2w, 0};
actListPtr ALcop_2w[]           = {&acop1_2w, &acop2_2w, &acop3_2w, 0};
actListPtr ALcure_2w[]          = {&aserum1_2w, &aserum2_2w, &acure_2w, 0};
actListPtr ALdial_2w[]          = {&adial_2w, 0};
actListPtr ALdialed_2w[]        = {&adialed_2w, 0};
actListPtr ALdidnt_2w[]         = {&adidnt1_2w, &adidnt2_2w, &ascr33a_2w,  &ascr33b_2w, &ascr33c_2w, &ascr33d_2w, 0};
actListPtr ALdoctor_2w[]        = {&achkrobot_2w, 0};
actListPtr ALdog1_2w[]          = {&adog1_2w, 0};
actListPtr ALdone_2w[]          = {&afinale_2w, &adisable_2w, &adone1_2w, &adone2_2w, &adone3_2w, &adone4_2w, &adone5_2w, &adone6_2w, &adone7_2w, &adone8_2w, &adone9_2w, &adone10_2w, &adone11_2w, &adone12_2w, &adone13_2w, &adone14_2w, &adone15_2w, &adone16_2w, 0};
actListPtr ALdropdyn1_2w[]      = {&achkdrop_2w, 0};
actListPtr ALdropdyn2_2w[]      = {&adropdyn2_2w, 0};
actListPtr ALdropdynamite_2w[]  = {&adropdyn1_2w, 0};
actListPtr ALdropmat_2w[]       = {&asplash_2w,   &amat1_2w, &amat2_2w, &amat3_2w, &amat4_2w, &amat5_2w, &amat6_2w, 0};
actListPtr ALdumb_2w[]          = {&abonus6_2w, &adumbchk_2w, 0};
actListPtr ALdyn1_2w[]          = {&adyn1_2w, &adyn2_2w, 0};
actListPtr ALeatban_2w[]        = {&aridban_2w, &aeatban_2w, &asubban_2w, &adropban_2w, 0};
actListPtr ALeatbanana_2w[]     = {&abprompt_2w, 0};
actListPtr ALempty_2w[]         = {&aempty_2w,0};
actListPtr ALexitmaze_2w[]      = {&aexitmaze_2w, &aheroxy11_2w, &ascrmaze_2w, 0};
actListPtr ALfaint_2w[]         = {&afaint1_2w, &ascream_2w,  &adisable_2w, &afaint2_2w, &afaint3_2w, &afaint4_2w, &afaint5_2w, &afaint6_2w, &afaint7_2w, &afaint8_2w, &afaint9_2w, &afaint10_2w, 0};
actListPtr ALgard1_2w[]         = {&agard2_2w, &agard3_2w, &agard4_2w, &agard5_2w, &agard6_2w, &agard7_2w, &agard8_2w, &agard9_2w, &agard10_2w, &agard11_2w, 0};
actListPtr ALgarlic_2w[]        = {&agarprompt_2w, 0};
actListPtr ALgatelight_2w[]     = {&abutchk_2w, &achkglight_2w, 0};
actListPtr ALgatescls_2w[]      = {&agatescls_2w, 0};
actListPtr ALgatesopn_2w[]      = {&abonus8_2w, &agatesopn_2w, 0};
actListPtr ALgenie_2w[]         = {&asong2a_2w, &abonus30_2w, &agenie1_2w, &agenie2_2w, &agenie3_2w, &agenie4_2w, 0};
actListPtr ALgetbook_2w[]       = {&achkstate0_2w, 0};
actListPtr ALgetdynamite_2w[]   = {&adynamite1_2w, 0};
actListPtr ALgetgarlic_2w[]     = {&agetgarlic_2w, 0};
actListPtr ALgetmatch_2w[]      = {&agetmatch_2w, 0};
actListPtr ALgiveb2_2w[]        = {&achkgive_2w, 0};
actListPtr ALgiveb3_2w[]        = {&ameow_2w, &agiveb3_2w, &agiveb4_2w, &agiveb5_2w, &agiveb6_2w, &agiveb7_2w, &abel1_2w, &abel2_2w, &abel3_2w, &abel4_2w, &acat3_2w, &abonus21_2w, 0};
actListPtr ALgiveb4_2w[]        = {&asniff_2w, 0};
actListPtr ALgivebel_2w[]       = {&agiveb1_2w, 0};
actListPtr ALglchk2_2w[]        = {&aglchk2_2w, 0};
actListPtr ALglightoff_2w[]     = {&aglightoff1_2w, &aglightoff2_2w, &aclicksnd_2w, &aclick_2w, 0};
actListPtr ALglighton_2w[]      = {&aglighton1_2w,  &aglighton2_2w,  &aclicksnd_2w, &aclick_2w, 0};
actListPtr ALglook1_2w[]        = {&aglook1_2w, 0};
actListPtr ALglook2_2w[]        = {&aglook2_2w, 0};
actListPtr ALgoclosed_2w[]      = {&ashed25_2w, &ascr0812_2w, 0};
actListPtr ALgoopen_2w[]        = {&ashed24_2w, &ascr0811_2w, 0};
actListPtr ALgotwill_2w[]       = {&agotwill_2w, 0};
actListPtr ALgun_2w[]           = {&achkgun_2w, 0};
actListPtr ALharry_2w[]         = {&aharry1_2w,   &aharry2_2w, &aharry3_2w, &aharry4_2w, &aharry5_2w, &aharry6_2w, &aharry7_2w, &acopbit8_2w, &abonus23_2w, &asong_l_2w, &ascreen35_2w, 0};
actListPtr ALhdrink_2w[]        = {&ahdrink1_2w, &ahdrink2_2w, &ahdrink3_2w, &ahdrink4_2w, &ahdrink5_2w, &ahdrink6_2w, &ahdrink7_2w, &ahdrink8_2w, &ahdrink9_2w, &ahdrink10_2w, &ahdrink11_2w, &ahdrink12_2w, &ahdrink13_2w, 0};
actListPtr ALheroxy01_2w[]      = {&aheroxy01_2w, &aherostart_2w, &aheroseq1_2w, 0};
actListPtr ALhfaint_2w[]        = {&adisable_2w, &ahfaint1_2w, &ahfaint2_2w, &ahfaint3_2w, &ahfaint4_2w, &ahfaint5_2w, &ahfaint6_2w, 0};
actListPtr ALhole_2w[]          = {&ahole_2w, 0};
actListPtr ALhprompt_2w[]       = {&ahestd1_2w, &ahestd2_2w, &ahestd3_2w, 0};
actListPtr ALhrgreet_2w[]       = {&ahest1_2w, &ahest2_2w, &ahest3_2w, &ahest4_2w, &ahest5_2w, &ahest6_2w, &ahest7_2w, &ahest8_2w, &ahest9_2w, &ahest10_2w, &ahest11_2w, &ahest12_2w, &ahest13_2w, &ahest14_2w, &ahest15_2w, &ahest16_2w, 0};
actListPtr ALhtable_2w[]        = {&achkhr2_2w, &achkhr3_2w, &achkhr4_2w, &astopr_2w, 0};
actListPtr ALhugone_2w[]        = {&asetbk1_2w, &abkstart1_2w, &abkstart2_2w, &abkstop_2w, &aswaphero_2w, &ascreen2_2w, &abonus1_2w, &adisable_2w, &agone1_2w, &agone2_2w, &agone3_2w, &agone4_2w, &acyc2_2w, &agone5_2w, &asong2_2w, &agone6_2w, &agone7_2w, &agone8_2w, &agone9_2w, &agone10_2w, &agone11_2w, &agone12_2w, &agone13_2w, &agone14_2w, &agone15_2w, 0};
actListPtr ALkaboom3_2w[]       = {&adead1_2w, &adead2_2w, &aboomxy_2w, &adead3_2w, &adead4_2w, &adead5_2w, 0};
actListPtr ALkaboom_2w[]        = {&asong5_2w, &akaboom1_2w, &akaboom2_2w, &akaboom3_2w, 0};
actListPtr ALkeyhole1_2w[]      = {&akeyhole1_2w, 0};
actListPtr ALkeyhole2_2w[]      = {&aok_2w, &akeyhole3_2w, &akeyhole4_2w, &akeyhole5_2w, &akeyhole6_2w, &ascr0204_2w, 0};
actListPtr ALkeyhole_2w[]       = {&akeyhole_2w, 0};
actListPtr ALlamp_2w[]          = {&achklamp_2w, 0};
actListPtr ALlightdynamite_2w[] = {&achkld1_2w, 0};
actListPtr ALlignpen_2w[]       = {&alignpen_2w, 0};
actListPtr ALlookbrg_2w[]       = {&alookbrg_2w,  &astopr_2w, 0};
actListPtr ALlookgard_2w[]      = {&aglchk_2w,  0};
actListPtr ALlookkennel_2w[]    = {&achkken1_2w, 0};
actListPtr ALmaid_2w[]          = {&amaidlips_2w, &aridmaidlips_2w, &amaid1_2w, &amaid2_2w, &amaid3_2w, &amaid4_2w, &amaid5_2w, &amaid6_2w,  &amaidgo_2w, &amaid7_2w, &amaid8_2w, &amaid9_2w, &amaid10_2w, &amaid11_2w, &arepmsg1_2w, 0};
actListPtr ALmaidbk_2w[]        = {&amaidbk1_2w, &amaidbk2_2w, &amaidbk3_2w, &amaidbk4_2w, &amaidbk5_2w, &amaidbk6_2w, &amaidbk7_2w, &amaidbk8_2w, 0};
actListPtr ALmaidp_2w[]         = {&achkmaid_2w, 0};
actListPtr ALmaidx_2w[]         = {&amaidp1_2w, &amaidp2_2w, &amaidp3_2w, &amaidp4_2w, &amaidp5_2w, &amaidp6_2w, &amaidp7_2w, &amaidp8_2w, &amaidp9_2w, &arepblah_2w, 0};
actListPtr ALmap0_2w[]          = {&amap0_2w, 0};
actListPtr ALmap1_2w[]          = {&amap1_2w, 0};
actListPtr ALmatok_2w[]         = {&achkld2_2w, 0};
actListPtr ALmissed_2w[]        = {&agunshot_2w, &amissed1_2w, &amissed2_2w, &amissed3_2w, 0};
actListPtr ALnasty_2w[]         = {&abite3_2w, &abite4_2w, &abite5_2w, &abite6_2w, 0};
actListPtr ALnobang2_2w[]       = {&anobang2_2w, 0};
actListPtr ALnobang_2w[]        = {&anobang_2w, 0};
actListPtr ALnobell_2w[]        = {&ading_2w, &abell_2w, &anobell_2w, 0};
actListPtr ALnocarry_2w[]       = {&anocarry_2w, 0};
actListPtr ALnocure_2w[]        = {&aserum1_2w, 0};
actListPtr ALnodrink_2w[]       = {&ahnod1_2w, &ahnod2_2w, &ahnod3_2w, 0};
actListPtr ALnogenie_2w[]       = {&anogenie_2w, 0};
actListPtr ALnopurps_2w[]       = {&anopurps_2w,0};
actListPtr ALnoreply_2w[]       = {&anoreply_2w, 0};
actListPtr ALnotrap_2w[]        = {&anotrap_2w, 0};
actListPtr ALomgag_2w[]         = {&aomlips_2w, &aomridlip_2w, &aom1_2w, &aom2_2w, &aom3_2w, &aom4_2w, &aom5_2w, &aom6_2w, &aom7_2w, &aom8_2w, &aom9_2w, &aom10_2w, &aom11_2w, &aom12_2w, &aom13_2w, &aom14_2w, &aom15_2w, &aom16_2w, &aom17_2w, &aom18_2w, &aom19_2w, &aom20_2w, &abonus11_2w, 0};
actListPtr ALopendoor1_2w[]     = {&astophero_2w, &adoorsnd_2w, &amaidstop1_2w, &amaidstop2_2w, &aopendoor1_2w, &ast12_2w, &aheroxy12_2w, &aclosedoor1_2w, &ascr12_2w, 0};
actListPtr ALopendoor2_2w[]     = {&astophero_2w, &adoorsnd_2w, &aopendoor2_2w, &ast3435_2w, &aheroxy3435_2w, &aclosedoor2_2w, &ascr3435_2w, 0};
actListPtr ALopendoor3_2w[]     = {&astophero_2w, &adoorsnd_2w, &aopendoor3_2w, &ast3436_2w, &aheroxy3436_2w, &aclosedoor3_2w, &ascr3436_2w, 0};
actListPtr ALpanel_2w[]         = {&asong2up_2w, &asong2dn_2w, &adisable_2w, &apanel1_2w, &apanel2_2w, &apanel3_2w, &apanel4_2w, &apanel5_2w, &apanel6_2w, &apanel7_2w, &apanel8_2w, 0};
actListPtr ALparty_2w[]         = {&adisable_2w, &aparty1a_2w, &aparty1b_2w, &aparty1c_2w, &aparty2a_2w, &aparty2b_2w, &aparty2c_2w, &aparty3a_2w, &aparty3b_2w, &aparty3c_2w, &aparty4a_2w, &aparty4b_2w, &aparty4c_2w, &aparty5a_2w, &aparty5b_2w, &aparty5c_2w, &aparty6a_2w, &aparty6b_2w, &aparty6c_2w, &aparty7a_2w, &aparty7b_2w, &aparty7c_2w, &aparty8a_2w, &aparty8b_2w, &aparty8c_2w, &aparty9a_2w, &aparty9b_2w, &aparty9c_2w, &aclimax_2w, 0};
actListPtr ALpencil_2w[]        = {&apen1_2w, &apen2_2w, &apen3_2w, &apen4_2w, &apen5_2w, &abonus27_2w, &afinito_2w, &ascr3334_2w, 0};
actListPtr ALpengone_2w[]       = {&abonus3_2w,  &abkstart1_2w, &abkstart2_2w, &abkstop_2w, &asong2_2w, &adisable_2w, &apeng1_2w, &apeng2_2w, &apeng3_2w, &apeng4_2w, &ascr0203_2w, 0};
actListPtr ALpenny1_2w[]        = {&apenxy_2w, &apenseq1_2w, &apenseq2_2w, &apenseq3_2w, &apenseq4_2w, &apenstart_2w, &apenvxy1_2w, &apenvxy2_2w, &apenvxy3_2w, &apenvxy4_2w, &apenseq5_2w,  &apenseq6_2w, &apenstop_2w, 0};
actListPtr ALphone_2w[]         = {&achkpb2_2w, &achkpb3_2w, &achkpb4_2w, 0};
actListPtr ALphonebox_2w[]      = {&achkpb1_2w, 0};
actListPtr ALphoto1_2w[]        = {&achkph2_2w, 0};
actListPtr ALphoto2_2w[]        = {&aphoto6_2w, 0};
actListPtr ALphoto3_2w[]        = {&aphoto1_2w, &aphoto2_2w, &aphoto3_2w, &aphoto4_2w, &aphoto5_2w, &agotalbum_2w, 0};
actListPtr ALphoto_2w[]         = {&achkphoto_2w, 0};
actListPtr ALpois1_2w[]         = {&apois1_2w, 0};
actListPtr ALpois2_2w[]         = {&apois2_2w, 0};
actListPtr ALpois3_2w[]         = {&apois3_2w, 0};
actListPtr ALpois4_2w[]         = {&apois4_2w, &aurgh_2w, &adead1_2w, &adead2_2w, &adead3_2w, &adead4_2w, &adead5_2w, 0};
actListPtr ALpushpaper_2w[]     = {&abonus26_2w, &apaper1_2w, &apaper2_2w, &apaper3_2w, &apaper4_2w, 0};
actListPtr ALpushpencil_2w[]    = {&achkpencil_2w, 0};
actListPtr ALreadlet_2w[]       = {&abonus24_2w,  &acopbit16_2w,  0};
actListPtr ALrephest_2w[]       = {&arepblah_2w, 0};
actListPtr ALrepmsg1_2w[]       = {&amaid12_2w, &arepmsg1_2w, 0};
actListPtr ALrg_2w[]            = {&arxy_2w, &arok_2w, &arg_2w, 0};
actListPtr ALridgard_2w[]       = {&adropgarl_2w, &aridgarl_2w,  &aeatgarl2_2w, &aridgard1_2w, &aridgard2_2w, &aridgard3_2w, &aridgard4_2w, &aridgard5_2w, &aridpath_2w, &aridtext_2w, &aridgard6_2w, &abonus7_2w, &agarl1_2w, &aschedbut_2w, 0};
actListPtr ALridgarl_2w[]       = {&adropgarl_2w, &aridgarl_2w, &aeatgarl1_2w, &subgarlic_2w, 0};
actListPtr ALridkey_2w[]        = {&aridkey1_2w, &aridkey2_2w, 0};
actListPtr ALrobot_2w[]         = {&adead1_2w, &ascream_2w, &adead2_2w, &adead3_2w, &adead4_2w, &aext1_2w, &aext2_2w, &aext3_2w, &adead5_2w, 0};
actListPtr ALrr_2w[]            = {&arxy_2w, &arok_2w, &arr_2w, 0};
actListPtr ALrumbling_2w[]      = {&agatesnd_2w, &arumbling_2w, 0};
actListPtr ALsafe1_2w[]         = {&abonus19_2w, &asafe1_2w, &asafe2_2w, &asafe3_2w, &asafe4_2w, &asafe5_2w, 0};
actListPtr ALsafe_2w[]          = {&achkwill_2w, 0};
actListPtr ALsafepr_2w[]        = {&asafepr_2w, 0};
actListPtr ALschedbut_2w[]      = {&aexplainb_2w, 0};
actListPtr ALscr0201_2w[]       = {&ascr21_2w, 0};
actListPtr ALscr02_2w[]         = {&achkbed1_2w, 0};
actListPtr ALscr0301_2w[]       = {&ascr31_2w, 0};
actListPtr ALscr0305_2w[]       = {&abed2_1_2w, &ascr0305_2w, 0};
actListPtr ALscr0306_2w[]       = {&adumb1_2w, &adumb2_2w, &adumb3_2w, &ascr0306_2w, 0};
actListPtr ALscr03_2w[]         = {&achkpanel_2w, 0};
actListPtr ALscr04_2w[]         = {&abonus2_2w, &amurdsnd_2w, &amurd1_2w, &amurd2_2w, &amurd3_2w, &amurd4_2w, &ascr0402_2w, 0};
actListPtr ALscr0503_2w[]       = {&abed3_1_2w, &ascr0503_2w, 0};
actListPtr ALscr0603_2w[]       = {&adumb11_2w, &adumb12_2w, &adumb13_2w, &ascr0603_2w, 0};
actListPtr ALscr0607_2w[]       = {&akit1_2w, &akit2_2w, &ascr0607_2w, 0};
actListPtr ALscr0631_2w[]       = {&achkkit_2w, 0};
actListPtr ALscr06_2w[]         = {&achkcook_2w, 0};
actListPtr ALscr0706_2w[]       = {&abd1_2w, &abd2_2w, &ascr0706_2w, 0};
actListPtr ALscr0708_2w[]       = {&abd20_2w, &abd21_2w, &ascr0708_2w, 0};
actListPtr ALscr0710_2w[]       = {&abd10_2w, &abd11_2w, &ascr0710_2w, 0};
actListPtr ALscr0807_2w[]       = {&ashed1_2w, &ashed2_2w, &ascr0807_2w, 0};
actListPtr ALscr0809_2w[]       = {&ashed11_2w, &ashed12_2w, &ascr0809_2w, 0};
actListPtr ALscr0908_2w[]       = {&ainshed1_2w, &ainshed2_2w, &ascr0908_2w, 0};
actListPtr ALscr09_2w[]         = {&agard1_2w, &aclue09_2w, 0};
actListPtr ALscr1007_2w[]       = {&avenus1_2w, &avenus2_2w, &ascr1007_2w, 0};
actListPtr ALscr10_2w[]         = {&awarn_2w, 0};
actListPtr ALscr1108_2w[]       = {&agates1_2w, &ascr1108_2w, 0};
actListPtr ALscr1113_2w[]       = {&agates11_2w, &ascr1113_2w, 0};
actListPtr ALscr1314_2w[]       = {&astream11_2w, &astream12_2w, &ascr1314_2w, 0};
actListPtr ALscr1413_2w[]       = {&azap1_2w, &ascr1413_2w, 0};
actListPtr ALscr1415_2w[]       = {&azap11_2w, &ascr1415_2w, 0};
actListPtr ALscr14_2w[]         = {&achkbugs_2w, &awarnz_2w, 0};
actListPtr ALscr1514_2w[]       = {&amush1_2w, &ascr1514_2w, 0};
actListPtr ALscr1516_2w[]       = {&amush21_2w, &ascr1516_2w, 0};
actListPtr ALscr1517_2w[]       = {&amush11_2w, &ascr1517_2w, 0};
actListPtr ALscr15_2w[]         = {&achkom_2w, 0};
actListPtr ALscr1615_2w[]       = {&awell1_2w, &ascr1615_2w, 0};
actListPtr ALscr1715_2w[]       = {&asnake1_2w, &ascr1715_2w, 0};
actListPtr ALscr1718_2w[]       = {&asnake11_2w, &ascr1718_2w, 0};
actListPtr ALscr1720_2w[]       = {&asnake21_2w, &ascr1720_2w, 0};
actListPtr ALscr1817_2w[]       = {&aphone1_2w, &ascr1817_2w, 0};
actListPtr ALscr1819c_2w[]      = {&aphone11c_2w, &ascr1819_2w, 0};
actListPtr ALscr1819l_2w[]      = {&aphone11l_2w, &ascr1819_2w, 0};
actListPtr ALscr1819r_2w[]      = {&aphone11r_2w, &ascr1819_2w, 0};
actListPtr ALscr1918c_2w[]      = {&aphone11c_2w, &ascr1918_2w, 0};
actListPtr ALscr1918l_2w[]      = {&aphone11l_2w, &ascr1918_2w, 0};
actListPtr ALscr1918r_2w[]      = {&aphone11r_2w, &ascr1918_2w, 0};
actListPtr ALscr2017_2w[]       = {&akennel1_2w, &ascr2017_2w, 0};
actListPtr ALscr2223_2w[]       = {&arockg1_2w, &ascr2223_2w, 0};
actListPtr ALscr2322_2w[]       = {&athree1_2w, &ascr2322_2w, 0};
actListPtr ALscr2324_2w[]       = {&athree11_2w, &ascr2324_2w, 0};
actListPtr ALscr2325_2w[]       = {&athree21_2w, &ascr2325_2w, 0};
actListPtr ALscr2326_2w[]       = {&athree31_2w, &ascr2326_2w, 0};
actListPtr ALscr2423_2w[]       = {&alamp1_2w, &ascr2423_2w, 0};
actListPtr ALscr2523_2w[]       = {&achasm25_2w, &ascr2523_2w, 0};
actListPtr ALscr25_2w[]         = {&acheatchk_2w, &arepchk_2w, 0};
actListPtr ALscr2623_2w[]       = {&apass1_2w, &ascr2623_2w, 0};
actListPtr ALscr2627_2w[]       = {&apass11_2w, &ascr2627_2w, 0};
actListPtr ALscr2726_2w[]       = {&aladder1_2w, &ascr2726_2w, 0};
actListPtr ALscr2827_2w[]       = {&atrap1_2w, &ascr2827_2w, 0};
actListPtr ALscr2829_2w[]       = {&atrap2_2w, &ascr2829_2w, 0};
actListPtr ALscr2928_2w[]       = {&ahall2_1_2w, &ascr2928_2w, 0};
actListPtr ALscr2930_2w[]       = {&ahall2_2_2w, &ahall2_2a_2w, &ascr2930_2w, 0};
actListPtr ALscr2931_2w[]       = {&ahall2_3_2w, &ascr2931_2w, 0};
actListPtr ALscr2934_2w[]       = {&ahall2_4_2w, &ascr2934_2w, 0};
actListPtr ALscr2938_2w[]       = {&ahall2_5_2w, &ascr2938_2w, 0};
actListPtr ALscr29_2w[]         = {&achkcop_2w, 0};
actListPtr ALscr3029_2w[]       = {&alounge1_2w, &ascr3029_2w, 0};
actListPtr ALscr30_2w[]         = {&achkcop2_2w, 0};
actListPtr ALscr3106_2w[]       = {&adoorsnd_2w, &achkcookp_2w, &aparlor3_2w, &ascr3106_2w, 0};
actListPtr ALscr3129_2w[]       = {&aparlor2_2w, &ascr3129_2w, 0};
actListPtr ALscr3132_2w[]       = {&aparlor1_2w, &ascr3132_2w, 0};
actListPtr ALscr3231_2w[]       = {&acatroom1_2w, &ascr3231_2w, 0};
actListPtr ALscr33_2w[]         = {&adraught_2w, &adrop33a_2w, &adrop33b_2w, &adrop33c_2w, &adrop33d_2w, &adrop33e_2w, &adrop33f_2w, &adrop33g_2w, &adrop33h_2w, &adrop33i_2w, &adrop33j_2w, 0};
actListPtr ALscr3429_2w[]       = {&ahall3_2_2w, &ascr3429_2w, 0};
actListPtr ALscr3438_2w[]       = {&ahall3_1_2w, &ahall3_1a_2w, &ascr3438_2w, 0};
actListPtr ALscr34_2w[]         = {&achkhero_2w, 0};
actListPtr ALscr3534_2w[]       = {&asong_l_2w, &aorgan1_2w, &ascr3534_2w, 0};
actListPtr ALscr35_2w[]         = {&achksong_2w, 0};
actListPtr ALscr3634_2w[]       = {&ahestroom1_2w, &ascr3634_2w, 0};
actListPtr ALscr36_2w[]         = {&achkhr1_2w, 0};
actListPtr ALscr3718_2w[]       = {&aretupxy_2w, &ascr3718_2w, 0};
actListPtr ALscr3829_2w[]       = {&ahall1_1_2w, &ascr3829_2w, 0};
actListPtr ALscr3834_2w[]       = {&ahall1_2_2w, &ahall1_3_2w, &ascr3834_2w, 0};
actListPtr ALscrgate1_2w[]      = {&ashed21_2w, &ashed22_2w, &ashed23_2w, &achkgo_2w, 0};
actListPtr ALscrgate2_2w[]      = {&astream1_2w, &achkgo_2w, 0};
actListPtr ALscrok_2w[]         = {&adoorsnd_2w, &akit11_2w, &ascr0631_2w, 0};
actListPtr ALshedlight_2w[]     = {&abutchk_2w, &achkslight_2w, 0};
actListPtr ALshot_2w[]          = {&agunshot_2w, &aridsched_2w, &amissed1_2w, &amissed2_2w, &amissed3_2w, &arobot1_2w, &arobot2_2w, &arobot3_2w, &arobot4_2w, &arobot5_2w, &arobot6_2w, &arobot7_2w, &abonus15_2w, 0};
actListPtr ALsilly_2w[]         = {&asilly_2w, 0};
actListPtr ALslightoff_2w[]     = {&aclicksnd_2w, &aslightoff1_2w, &aslightoff2_2w, 0};
actListPtr ALslighton_2w[]      = {&aclicksnd_2w, &aslighton1_2w,  &aslighton2_2w, 0};
actListPtr ALsnake_2w[]         = {&achksnake_2w, 0};
actListPtr ALsoggy_2w[]         = {&asoggy_2w, 0};
actListPtr ALsong3_2w[]         = {&asong_r_2w, 0};
actListPtr ALsong4_2w[]         = {&asong4_2w, 0};
actListPtr ALsonic_2w[]         = {&asonic1_2w, &asonic2_2w, &asonic3_2w, &asonic4_2w, &asonic5_2w, &asonic6_2w, &asonic7_2w, &asonic8_2w, &asonic9_2w, 0};
actListPtr ALstopr_2w[]         = {&astopr_2w,0};
actListPtr ALstrike_2w[]        = {&astrike_2w, 0};
actListPtr ALstrikematch_2w[]   = {&amatchk2_2w, 0};
actListPtr ALstuck_2w[]         = {&astuck1_2w, 0};
actListPtr ALswgates_2w[]       = {&aswgates_2w, &agatesnd_2w, &arumbling_2w, 0};
actListPtr ALswzapper_2w[]      = {&aswzapper_2w, &aclicksnd_2w, &aclick_2w, 0};
actListPtr ALtakepaper_2w[]     = {&apaper5_2w, 0};
actListPtr ALtalkgard_2w[]      = {&atalkg_2w,  0};
actListPtr ALthrown_2w[]        = {&achkken2_2w, 0};
actListPtr ALthrowstick_2w[]    = {&abark_2w, &astick1_2w, &adisable_2w, &astick2_2w, &astick3_2w, &astick4_2w, &astick5_2w, &astick6_2w, &astick7_2w, &astick8_2w, &astick9_2w, &astick10_2w, &abonus12_2w, 0};
actListPtr ALtrap_2w[]          = {&achktrap_2w, 0};
actListPtr ALuptrap_2w[]        = {&auptrap1_2w, &auptrap2_2w, 0};
actListPtr ALvenus_2w[]         = {&ascream_2w, &astung_2w, &adead1_2w, &adead2_2w, &adead3_2w, &adead4_2w, &adead5_2w, 0};
actListPtr ALwho_2w[]           = {&abonus14_2w, &awho1_2w, &awho2_2w, &awho3_2w, &awho4_2w, &ascr1837_2w, 0};
actListPtr ALwill1_2w[]         = {&awill1_2w, &awill2_2w, &acopbit1_2w, &abonus20_2w, 0};
actListPtr ALwill2_2w[]         = {&awill3_2w, &awill4_2w, 0};
actListPtr ALwill_2w[]          = {&achkmag_2w, 0};
actListPtr ALworkgates_2w[]     = {&abutchk_2w, &achkgates_2w,  0};
actListPtr ALzapperoff_2w[]     = {&azapperoff1_2w, &azapperoff2_2w, 0};
actListPtr ALzapperon_2w[]      = {&abonus9_2w, &azapperon1_2w, &azapperon2_2w, 0};
actListPtr ALscr01Story_2w[]    = {&astory_2w, &ascr01_2w, 0};
actListPtr ALscr01NoStory_2w[]  = {&askip1_2w, &askip2_2w, &askip3_2w, &askip4_2w, &askip5_2w, 0};

// Special action list for maze
act2  aheroxy_2w   = {INIT_OBJXY,  0, HERO, 0, 0};
//act5  astophero_2w = {INIT_OBJVXY, 0, HERO, 0, 0};
act1  aherostop_2w = {START_OBJ,   0, HERO, 0, NOT_CYCLING};
act8  anewscr_2w   = {NEW_SCREEN,  0, 0};
actListPtr ALnewscr_2w[] = {&aheroxy_2w, &astophero_2w, &aherostop_2w, &anewscr_2w, 0};

actList actListArr_2w[] = {
	ALDummy,            AL11maze_2w,     AL_eatgar_2w,   ALballoon_2w,      ALbanana_2w,
	ALbang1_2w,         ALbang2_2w,      ALbed1_2w,      ALbell1_2w,        ALbell2_2w,
	ALbell_2w,          ALbite_2w,       ALblah_2w,      ALboom_2w,         ALbottle_2w,
	ALbridge_2w,        ALbugattack_2w,  ALbugflit_2w,   ALbugrep1_2w,      ALbugrep2_2w,
	ALbugs_2w,          ALbugzapper_2w,  ALcallp_2w,     ALcantpush_2w,     ALcat1_2w,
	ALcat2_2w,          ALcat3_2w,       ALcat4_2w,      ALcat5_2w,         ALcat6_2w,
	ALcatnip_2w,        ALchasm_2w,      ALcheat_2w,     ALchkbell2_2w,     ALchkc09_2w,
	ALchkcarry_2w,      ALchkdoc_2w,     ALchkld3_2w,    ALchkld4_2w,       ALchkmat1_2w,
	ALchkmat2_2w,       ALchkpap1_2w,    ALchkpap2_2w,   ALchkroute_2w,     ALchkrr2_2w,
	ALchksafe_2w,       ALchkscrew_2w,   ALchkstate1_2w, ALclick_2w,        ALclimax_2w,
	ALclimbrope_2w,     ALclimbup_2w,    ALclimbwell_2w, ALclue09_2w,       ALcomb1_2w,
	ALcomb2_2w,         ALcook_2w,       ALcookp_2w,     ALcop_2w,          ALcure_2w,
	ALdial_2w,          ALdialed_2w,     ALdidnt_2w,     ALdoctor_2w,       ALdog1_2w,
	ALdone_2w,          ALdropdyn1_2w,   ALdropdyn2_2w,  ALdropdynamite_2w, ALdropmat_2w,
	ALdumb_2w,          ALdyn1_2w,       ALeatban_2w,    ALeatbanana_2w,    ALempty_2w,
	ALexitmaze_2w,      ALfaint_2w,      ALgard1_2w,     ALgarlic_2w,       ALgatelight_2w,
	ALgatescls_2w,      ALgatesopn_2w,   ALgenie_2w,     ALgetbook_2w,      ALgetdynamite_2w,
	ALgetgarlic_2w,     ALgetmatch_2w,   ALgiveb2_2w,    ALgiveb3_2w,       ALgiveb4_2w,
	ALgivebel_2w,       ALglchk2_2w,     ALglightoff_2w, ALglighton_2w,     ALglook1_2w,
	ALglook2_2w,        ALgoclosed_2w,   ALgoopen_2w,    ALgotwill_2w,      ALgun_2w,
	ALharry_2w,         ALhdrink_2w,     ALheroxy01_2w,  ALhfaint_2w,       ALhole_2w,
	ALhprompt_2w,       ALhrgreet_2w,    ALhtable_2w,    ALhugone_2w,       ALkaboom3_2w,
	ALkaboom_2w,        ALkeyhole1_2w,   ALkeyhole2_2w,  ALkeyhole_2w,      ALlamp_2w,
	ALlightdynamite_2w, ALlignpen_2w,    ALlookbrg_2w,   ALlookgard_2w,     ALlookkennel_2w,
	ALmaid_2w,          ALmaidbk_2w,     ALmaidp_2w,     ALmaidx_2w,        ALmap0_2w,
	ALmap1_2w,          ALmatok_2w,      ALmissed_2w,    ALnasty_2w,        ALnobang2_2w,
	ALnobang_2w,        ALnobell_2w,     ALnocarry_2w,   ALnocure_2w,       ALnodrink_2w,
	ALnogenie_2w,       ALnopurps_2w,    ALnoreply_2w,   ALnotrap_2w,       ALomgag_2w,
	ALopendoor1_2w,     ALopendoor2_2w,  ALopendoor3_2w, ALpanel_2w,        ALparty_2w,
	ALpencil_2w,        ALpengone_2w,    ALpenny1_2w,    ALphone_2w,        ALphonebox_2w,
	ALphoto1_2w,        ALphoto2_2w,     ALphoto3_2w,    ALphoto_2w,        ALpois1_2w,
	ALpois2_2w,         ALpois3_2w,      ALpois4_2w,     ALpushpaper_2w,    ALpushpencil_2w,
	ALreadlet_2w,       ALrephest_2w,    ALrepmsg1_2w,   ALrg_2w,           ALridgard_2w,
	ALridgarl_2w,       ALridkey_2w,     ALrobot_2w,     ALrr_2w,           ALrumbling_2w,
	ALsafe1_2w,         ALsafe_2w,       ALsafepr_2w,    ALschedbut_2w,     ALscr0201_2w,
	ALscr02_2w,         ALscr0301_2w,    ALscr0305_2w,   ALscr0306_2w,      ALscr03_2w,
	ALscr04_2w,         ALscr0503_2w,    ALscr0603_2w,   ALscr0607_2w,      ALscr0631_2w,
	ALscr06_2w,         ALscr0706_2w,    ALscr0708_2w,   ALscr0710_2w,      ALscr0807_2w,
	ALscr0809_2w,       ALscr0908_2w,    ALscr09_2w,     ALscr1007_2w,      ALscr10_2w,
	ALscr1108_2w,       ALscr1113_2w,    ALscr1314_2w,   ALscr1413_2w,      ALscr1415_2w,
	ALscr14_2w,         ALscr1514_2w,    ALscr1516_2w,   ALscr1517_2w,      ALscr15_2w,
	ALscr1615_2w,       ALscr1715_2w,    ALscr1718_2w,   ALscr1720_2w,      ALscr1817_2w,
	ALscr1819c_2w,      ALscr1819l_2w,   ALscr1819r_2w,  ALscr1918c_2w,     ALscr1918l_2w,
	ALscr1918r_2w,      ALscr2017_2w,    ALscr2223_2w,   ALscr2322_2w,      ALscr2324_2w,
	ALscr2325_2w,       ALscr2326_2w,    ALscr2423_2w,   ALscr2523_2w,      ALscr25_2w,
	ALscr2623_2w,       ALscr2627_2w,    ALscr2726_2w,   ALscr2827_2w,      ALscr2829_2w,
	ALscr2928_2w,       ALscr2930_2w,    ALscr2931_2w,   ALscr2934_2w,      ALscr2938_2w,
	ALscr29_2w,         ALscr3029_2w,    ALscr30_2w,     ALscr3106_2w,      ALscr3129_2w,
	ALscr3132_2w,       ALscr3231_2w,    ALscr33_2w,     ALscr3429_2w,      ALscr3438_2w,
	ALscr34_2w,         ALscr3534_2w,    ALscr35_2w,     ALscr3634_2w,      ALscr36_2w,
	ALscr3718_2w,       ALscr3829_2w,    ALscr3834_2w,   ALscrgate1_2w,     ALscrgate2_2w,
	ALscrok_2w,         ALshedlight_2w,  ALshot_2w,      ALsilly_2w,        ALslightoff_2w,
	ALslighton_2w,      ALsnake_2w,      ALsoggy_2w,     ALsong3_2w,        ALsong4_2w,
	ALsonic_2w,         ALstopr_2w,      ALstrike_2w,    ALstrikematch_2w,  ALstuck_2w,
	ALswgates_2w,       ALswzapper_2w,   ALtakepaper_2w, ALtalkgard_2w,     ALthrown_2w,
	ALthrowstick_2w,    ALtrap_2w,       ALuptrap_2w,    ALvenus_2w,        ALwho_2w,
	ALwill1_2w,         ALwill2_2w,      ALwill_2w,      ALworkgates_2w,    ALzapperoff_2w,
	ALzapperon_2w,      ALnewscr_2w,     ALscr01Story_2w, ALscr01NoStory_2w
};

// Hugo 3 Win
act0 adarttest_3w      = {ASCHEDULE, 0,  kALdartsched_3w};
act0 arepblink_3w      = {ASCHEDULE, 60, kALeleblink_3w};
act0 arepeathorizon_3w = {ASCHEDULE, 2,  kALhorizon_3w};
act0 arepeatmouse_3w   = {ASCHEDULE, 4,  kALmouse_3w};
act0 arepflash_3w      = {ASCHEDULE, 10, kALflash_3w};

act1 aappear1_3w     = {START_OBJ, 1,   HERO,        0, NOT_CYCLING};
act1 abridge3_3w     = {START_OBJ, 0,   HERO,        0, CYCLE_FORWARD};
act1 acamp2a_3w      = {START_OBJ, 0,   NAT2_3w,     0, CYCLE_FORWARD};
act1 acamp2b_3w      = {START_OBJ, 0,   NATG_3w,     0, CYCLE_FORWARD};
act1 acamp8a_3w      = {START_OBJ, 34,  NAT2_3w,     0, NOT_CYCLING};
act1 acamp8b_3w      = {START_OBJ, 34,  NATG_3w,     0, NOT_CYCLING};
act1 acrash14_3w     = {START_OBJ, 20,  PENNY_3w,    0, CYCLE_FORWARD};
act1 acrash19_3w     = {START_OBJ, 42,  HERO,        0, CYCLE_FORWARD};
act1 acrash22_3w     = {START_OBJ, 50,  HERO,        0, INVISIBLE};
act1 acrash9_3w      = {START_OBJ, 5,   HERO,        0, NOT_CYCLING};
act1 adead1_3w       = {START_OBJ, 0,   HERO,        0, INVISIBLE};
act1 adead3_3w       = {START_OBJ, 0,   PENNYLIE_3w, 0, NOT_CYCLING};
act1 adropord2_3w    = {START_OBJ, 0,   CHEESE_3w,   0, NOT_CYCLING};
act1 aenter5_3w      = {START_OBJ, 4,   MOUSE_3w,    0, INVISIBLE};
act1 aenter6_3w      = {START_OBJ, 4,   CHEESE_3w,   0, INVISIBLE};
act1 aenter8_3w      = {START_OBJ, 4,   CAGE_3w,     0, CYCLE_FORWARD};
act1 aex4_3w         = {START_OBJ, 0,   GHOST_3w,    0, INVISIBLE};
act1 afind2_3w       = {START_OBJ, 0,   CRYSTAL_3w,  0, NOT_CYCLING};
act1 afindb3_3w      = {START_OBJ, 0,   BOOK_3w,     0, NOT_CYCLING};
act1 aflask5_3w      = {START_OBJ, 0,   HERO,        0, INVISIBLE};
act1 agot10_3w       = {START_OBJ, 60,  DOCTOR_3w,   0, CYCLE_FORWARD};
act1 agot1a_3w       = {START_OBJ, 0,   HERO,        0, CYCLE_FORWARD};
act1 agot1b_3w       = {START_OBJ, 0,   DOCTOR_3w,   0, CYCLE_FORWARD};
act1 ahelp3_3w       = {START_OBJ, 0,   HERO,        0, CYCLE_FORWARD};
act1 amission12_3w   = {START_OBJ, 10,  NATG_3w,     0, NOT_CYCLING};
act1 amission15_3w   = {START_OBJ, 29,  LIPS_3w,     0, INVISIBLE};
act1 amission18_3w   = {START_OBJ, 34,  HERO,        0, NOT_CYCLING};
act1 amission25_3w   = {START_OBJ, 60,  SPIDER_3w,   0, INVISIBLE};
act1 amission4_3w    = {START_OBJ, 0,   NATG_3w,     0, CYCLE_FORWARD};
act1 amission5_3w    = {START_OBJ, 0,   HERO,        0, CYCLE_FORWARD};
act1 aold1a_3w       = {START_OBJ, 0,   MOUTH_3w,    0, NOT_CYCLING};
act1 aold1b_3w       = {START_OBJ, 20,  MOUTH_3w,    0, CYCLE_FORWARD};
act1 aold6a_3w       = {START_OBJ, 0,   MOUTH_3w,    0, NOT_CYCLING};
act1 aold6g_3w       = {START_OBJ, 22,  HERO,        0, NOT_CYCLING};
act1 aplantfix_3w    = {START_OBJ, 0,   PLANT1_3w,   0, NOT_CYCLING};
act1 aprod1_3w       = {START_OBJ, 0,   DOCTOR_3w,   0, CYCLE_FORWARD};
act1 aprod6_3w       = {START_OBJ, 24,  DOCTOR_3w,   0, CYCLE_FORWARD};
act1 areturn2_3w     = {START_OBJ, 0,   NATG_3w,     0, NOT_CYCLING};
act1 ascare15_3w     = {START_OBJ, 0,   ELEPHANT_3w, 0, CYCLE_FORWARD};
act1 ascare16_3w     = {START_OBJ, 64,  ELEPHANT_3w, 0, INVISIBLE};
act1 ascare4_3w      = {START_OBJ, 0,   E_EYES_3w,   0, INVISIBLE};
act1 ascare6_3w      = {START_OBJ, 0,   MOUSE_3w,    0, CYCLE_FORWARD};
act1 astick4_3w      = {START_OBJ, 16,  DOCTOR_3w,   0, INVISIBLE};
act1 astick6_3w      = {START_OBJ, 17,  DOCLIE_3w,   0, CYCLE_FORWARD};
act1 aswing1_3w      = {START_OBJ, 0,   HERO,        0, INVISIBLE};
act1 aswing3_3w      = {START_OBJ, 1,   HERO,        0, CYCLE_FORWARD};
act1 aswing8_3w      = {START_OBJ, 15,  HERO,        0, NOT_CYCLING};
act1 atakecage1_3w   = {START_OBJ, 0,   CAGE_3w,     0, INVISIBLE};
act1 atakecheese2_3w = {START_OBJ, 0,   CHEESE_3w,   0, INVISIBLE};
act1 avine2_3w       = {START_OBJ, 0,   BLOCK1_3w,   0, INVISIBLE};
act1 avine3_3w       = {START_OBJ, 0,   BLOCK2_3w,   0, INVISIBLE};
act1 avine4_3w       = {START_OBJ, 0,   BLOCK3_3w,   0, INVISIBLE};
act1 avine5_3w       = {START_OBJ, 0,   BLOCK4_3w,   0, INVISIBLE};
act1 aweb19_3w       = {START_OBJ, 110, LIPS_3w,     0, INVISIBLE};
act1 aweb21_3w       = {START_OBJ, 115, PENNY_3w,    0, INVISIBLE};
act1 aweb22_3w       = {START_OBJ, 116, PENNYLIE_3w, 0, NOT_CYCLING};
act1 aweb4_3w        = {START_OBJ, 0,   PENNY_3w,    0, CYCLE_FORWARD};
act1 aweb7_3w        = {START_OBJ, 38,  PENNY_3w,    0, NOT_CYCLING};
act1 awink1_3w       = {START_OBJ, 8,   O_EYE_3w,    1, CYCLE_FORWARD};
act1 awink2_3w       = {START_OBJ, 16,  O_EYE_3w,    1, CYCLE_BACKWARD};
act1 awink3_3w       = {START_OBJ, 19,  O_EYE_3w,    0, INVISIBLE};
act1 aappear_3w      = {START_OBJ, 0,   HERO,        0, NOT_CYCLING};
act1 adisappear_3w   = {START_OBJ, 0,   HERO,        0, INVISIBLE};

act2 acamp0b_3w            = {INIT_OBJXY, 0,  NATG_3w,   128, 101};
act2 acamp3a_3w            = {INIT_OBJXY, 0,  NAT2_3w,   17,  97};
act2 acamp3b_3w            = {INIT_OBJXY, 0,  NATG_3w,   28,  101};
act2 adoc4_3w              = {INIT_OBJXY, 0,  DOCTOR_3w, 70,  110};
act2 adropcheese2_3w       = {INIT_OBJXY, 0,  CHEESE_3w, 158, 142};
act2 aexit1_3w             = {INIT_OBJXY, 0,  HERO,      170, 110};
act2 agot5a_3w             = {INIT_OBJXY, 40, HERO,      239, 104};
act2 ahole4a_3w            = {INIT_OBJXY, 10, MOUSE_3w,  280, 135};
act2 ahole4b_3w            = {INIT_OBJXY, 10, MOUSE_3w,  200, 135};
act2 aleft2_3w             = {INIT_OBJXY, 0,  HERO,      218, 106};
act2 aleft4_3w             = {INIT_OBJXY, 15, HERO,      53,  133};
act2 amission3_3w          = {INIT_OBJXY, 0,  NATG_3w,   30,  120};
act2 aold6f_3w             = {INIT_OBJXY, 0,  HERO,      289, 91};
act2 aplane1_3w            = {INIT_OBJXY, 0,  HERO,      170, 50};
act2 areturn4_3w           = {INIT_OBJXY, 0,  NATG_3w,   85,  115};
act2 aright2_3w            = {INIT_OBJXY, 0,  HERO,      77,  106};
act2 aright4_3w            = {INIT_OBJXY, 15, HERO,      243, 133};
act2 astick5_3w            = {INIT_OBJXY, 17, DOCLIE_3w, 238, 133};
act2 aweb2_3w              = {INIT_OBJXY, 0,  PENNY_3w,  270, 133};
act2 aweb26_3w             = {INIT_OBJXY, 0,  HERO,      174, 107};
act2 axy_brg_clftop_3w     = {INIT_OBJXY, 0,  HERO,      280, 30};
act2 axy_brg_path_3w       = {INIT_OBJXY, 0,  HERO,      16,  91};
act2 axy_camp_hut_3w       = {INIT_OBJXY, 0,  HERO,      27,  133};
act2 axy_camp_village_c_3w = {INIT_OBJXY, 0,  HERO,      100, 143};
act2 axy_camp_village_l_3w = {INIT_OBJXY, 0,  HERO,      45,  145};
act2 axy_cave_turn_3w      = {INIT_OBJXY, 0,  HERO,      22,  136};
act2 axy_cave_wfall_3w     = {INIT_OBJXY, 0,  HERO,      287, 140};
act2 axy_clf_clftop_3w     = {INIT_OBJXY, 0,  HERO,      269, 130};
act2 axy_clf_wfall_3w      = {INIT_OBJXY, 0,  HERO,      28,  140};
act2 axy_clftop_brg_3w     = {INIT_OBJXY, 0,  HERO,      28,  91};
act2 axy_clftop_clf_3w     = {INIT_OBJXY, 0,  HERO,      28,  140};
act2 axy_clftop_slope_3w   = {INIT_OBJXY, 0,  HERO,      28,  120};
act2 axy_crash_web_3w      = {INIT_OBJXY, 0,  HERO,      280, 133};
act2 axy_garden_wbase_3w   = {INIT_OBJXY, 0,  HERO,      24,  114};
act2 axy_hut_camp_3w       = {INIT_OBJXY, 0,  HERO,      280, 121};
act2 axy_hut_in_3w         = {INIT_OBJXY, 0,  HERO,      155, 141};
act2 axy_hut_out_3w        = {INIT_OBJXY, 0,  HERO,      148, 108};
act2 axy_hut_village_c_3w  = {INIT_OBJXY, 0,  HERO,      207, 143};
act2 axy_hut_village_r_3w  = {INIT_OBJXY, 0,  HERO,      207, 143};
act2 axy_path_brg_3w       = {INIT_OBJXY, 0,  HERO,      289, 91};
act2 axy_path_stream_3w    = {INIT_OBJXY, 0,  HERO,      289, 133};
act2 axy_path_village_3w   = {INIT_OBJXY, 0,  HERO,      25,  143};
act2 axy_path_web_3w       = {INIT_OBJXY, 0,  HERO,      25,  140};
act2 axy_slope_clftop_3w   = {INIT_OBJXY, 0,  HERO,      280, 92};
act2 axy_slope_stream_3w   = {INIT_OBJXY, 0,  HERO,      28,  133};
act2 axy_stream_path_3w    = {INIT_OBJXY, 0,  HERO,      27,  140};
act2 axy_stream_slope_3w   = {INIT_OBJXY, 0,  HERO,      275, 90};
act2 axy_turn_cave_3w      = {INIT_OBJXY, 0,  HERO,      272, 140};
act2 axy_turn_village_3w   = {INIT_OBJXY, 0,  HERO,      283, 143};
act2 axy_village_camp_l_3w = {INIT_OBJXY, 0,  HERO,      64,  130};
act2 axy_village_camp_r_3w = {INIT_OBJXY, 0,  HERO,      280, 130};
act2 axy_village_path_3w   = {INIT_OBJXY, 0,  HERO,      280, 140};
act2 axy_village_turn_3w   = {INIT_OBJXY, 0,  HERO,      27,  87};
act2 axy_wbase_garden_3w   = {INIT_OBJXY, 0,  HERO,      272, 133};
act2 axy_wbase_wfall_3w    = {INIT_OBJXY, 0,  HERO,      254, 114};
act2 axy_web_crash_3w      = {INIT_OBJXY, 0,  HERO,      28,  140};
act2 axy_web_path_3w       = {INIT_OBJXY, 0,  HERO,      280, 140};
act2 axy_wfall_cave_3w     = {INIT_OBJXY, 0,  HERO,      27,  140};
act2 axy_wfall_clf_3w      = {INIT_OBJXY, 0,  HERO,      280, 140};
act2 axy_wfallb_wbase_3w   = {INIT_OBJXY, 0,  HERO,      273, 114};

act5 abridge2_3w   = {INIT_OBJVXY, 0,   HERO,        -DX,     0};
act5 acamp4a_3w    = {INIT_OBJVXY, 0,   NAT2_3w,     4,       0};
act5 acamp4b_3w    = {INIT_OBJVXY, 0,   NATG_3w,     3,       0};
act5 acamp6a_3w    = {INIT_OBJVXY, 33,  NAT2_3w,     0,       0};
act5 acamp7a_3w    = {INIT_OBJVXY, 34,  NATG_3w,     0,       0};
act5 aclose1_3w    = {INIT_OBJVXY, 0,   CDOOR_3w,    DX,      0};
act5 aclose2_3w    = {INIT_OBJVXY, 6,   CDOOR_3w,    0,       0};
act5 acrash17_3w   = {INIT_OBJVXY, 23,  PENNY_3w,    -DX,     0};
act5 acrash20_3w   = {INIT_OBJVXY, 42,  HERO,        0,       -1};
act5 acrash21_3w   = {INIT_OBJVXY, 50,  HERO,        0,       0};
act5 acrash4_3w    = {INIT_OBJVXY, 1,   PENNY_3w,    0,       DY};
act5 acrash5_3w    = {INIT_OBJVXY, 1,   HERO,        0,       DY};
act5 acrash6_3w    = {INIT_OBJVXY, 4,   PENNY_3w,    DX,      0};
act5 acrash7_3w    = {INIT_OBJVXY, 4,   HERO,        0,       0};
act5 adead4_3w     = {INIT_OBJVXY, 0,   HERO,        0,       0};
act5 aenter4_3w    = {INIT_OBJVXY, 0,   MOUSE_3w,    -DX,     0};
act5 aex3_3w       = {INIT_OBJVXY, 0,   GHOST_3w,    0,       0};
act5 agot12_3w     = {INIT_OBJVXY, 60,  DOCTOR_3w,   -DX,     0};
act5 agot2b_3w     = {INIT_OBJVXY, 26,  HERO,        0,       0};
act5 agot3b_3w     = {INIT_OBJVXY, 28,  DOCTOR_3w,   0,       -DX};
act5 agot4_3w      = {INIT_OBJVXY, 22,  CDOOR_3w,    -DX,     0};
act5 agot5_3w      = {INIT_OBJVXY, 36,  HERO,        0,       -DX};
act5 agot7_3w      = {INIT_OBJVXY, 36,  CDOOR_3w,    DX,      0};
act5 agot8_3w      = {INIT_OBJVXY, 42,  CDOOR_3w,    0,       0};
act5 ahelp4_3w     = {INIT_OBJVXY, 0,   HERO,        0,       DY};
act5 ahelp6_3w     = {INIT_OBJVXY, 8,   HERO,        -DX * 2, 0};
act5 ahole3a_3w    = {INIT_OBJVXY, 0,   MOUSE_3w,    DX,      0};
act5 ahole3b_3w    = {INIT_OBJVXY, 0,   MOUSE_3w,    -DX,     0};
act5 aleft3_3w     = {INIT_OBJVXY, 0,   HERO,        -DX * 2, 0};
act5 amission11_3w = {INIT_OBJVXY, 10,  NATG_3w,     0,       0};
act5 amission17_3w = {INIT_OBJVXY, 36,  HERO,        0,       0};
act5 amission6_3w  = {INIT_OBJVXY, 0,   NATG_3w,     DX,      -1};
act5 amission7_3w  = {INIT_OBJVXY, 0,   SPIDER_3w,   0,       -1};
act5 amission8_3w  = {INIT_OBJVXY, 0,   HERO,        -DX,     0};
act5 amission9_3w  = {INIT_OBJVXY, 6,   HERO,        -DX,     -1};
act5 aopen1_3w     = {INIT_OBJVXY, 0,   CDOOR_3w,    -DX,     0};
act5 aprod3_3w     = {INIT_OBJVXY, 2,   DOCTOR_3w,   DX * 4,  0};
act5 aprod8_3w     = {INIT_OBJVXY, 26,  DOCTOR_3w,   -DX,     0};
act5 areturn3_3w   = {INIT_OBJVXY, 0,   NATG_3w,     0,       0};
act5 aright3_3w    = {INIT_OBJVXY, 0,   HERO,        DX * 2,  0};
act5 ascare11_3w   = {INIT_OBJVXY, 18,  MOUSE_3w,    DX * 4,  -DY};
act5 ascare14_3w   = {INIT_OBJVXY, 0,   ELEPHANT_3w, -3,      0};
act5 ascare9_3w    = {INIT_OBJVXY, 0,   MOUSE_3w,    -DX * 4, 0};
act5 aswing6_3w    = {INIT_OBJVXY, 15,  HERO,        0,       0};
act5 aweb11_3w     = {INIT_OBJVXY, 50,  SPIDER_3w,   0,       1};
act5 aweb12_3w     = {INIT_OBJVXY, 75,  SPIDER_3w,   0,       -4};
act5 aweb13_3w     = {INIT_OBJVXY, 80,  SPIDER_3w,   0,       2};
act5 aweb14_3w     = {INIT_OBJVXY, 105, SPIDER_3w,   0,       0};
act5 aweb5_3w      = {INIT_OBJVXY, 0,   PENNY_3w,    -DX,     -1};
act5 aweb9_3w      = {INIT_OBJVXY, 40,  PENNY_3w,    0,       0};
act5 astophero_3w  = {INIT_OBJVXY, 0,   HERO,        0,       0};

act6 acheese1_3w     = {INIT_CARRY, 0, CHEESE_3w,   false};
act6 adropord3_3w    = {INIT_CARRY, 0, CHEESE_3w,   false};
act6 agive6_3w       = {INIT_CARRY, 0, BLOWPIPE_3w, true};
act6 agive7_3w       = {INIT_CARRY, 0, BOUILLON_3w, false};
act6 atakecage2_3w   = {INIT_CARRY, 0, CAGE_3w,     true};
act6 atakecb3_3w     = {INIT_CARRY, 0, CRYSTAL_3w,  false};
act6 atakecheese3_3w = {INIT_CARRY, 0, CHEESE_3w,   true};

act7 adead2_3w    = {INIT_HF_COORD, 0, PENNYLIE_3w};
act7 adropord1_3w = {INIT_HF_COORD, 0, CHEESE_3w};
act7 afind1_3w    = {INIT_HF_COORD, 0, CRYSTAL_3w};
act7 afindb2_3w   = {INIT_HF_COORD, 0, BOOK_3w};
act7 ascare5_3w   = {INIT_HF_COORD, 0, MOUSE_3w};

act8 acrash23_3w            = {NEW_SCREEN, 50,  WEB_3w};
act8 aexit3_3w              = {NEW_SCREEN, 0,   CRASH_3w};
act8 aflask6_3w             = {NEW_SCREEN, 0,   SUNSET_3w};
act8 aold6i_3w              = {NEW_SCREEN, 22,  BRIDGE2_3w};
act8 aplane3_3w             = {NEW_SCREEN, 0,   PLANE_3w};
act8 areturn_3w             = {NEW_SCREEN, 1,   CAVE_3w};
act8 ascr_brg_clftop_3w     = {NEW_SCREEN, 0,   CLIFFTOP_3w};
act8 ascr_brg_path_3w       = {NEW_SCREEN, 0,   PATH_UL_3w};
act8 ascr_camp_hut_3w       = {NEW_SCREEN, 0,   HUT_OUT_3w};
act8 ascr_camp_village_c_3w = {NEW_SCREEN, 0,   VILLAGE_3w};
act8 ascr_camp_village_l_3w = {NEW_SCREEN, 0,   VILLAGE_3w};
act8 ascr_cave_man_3w       = {NEW_SCREEN, 0,   OLDMAN_3w};
act8 ascr_cave_turn_3w      = {NEW_SCREEN, 0,   TURN_3w};
act8 ascr_clf_clftop_3w     = {NEW_SCREEN, 0,   CLIFFTOP_3w};
act8 ascr_clf_wfall_3w      = {NEW_SCREEN, 0,   WFALL_3w};
act8 ascr_clf_wnofall_3w    = {NEW_SCREEN, 0,   WFALL_B_3w};
act8 ascr_clftop_brg_3w     = {NEW_SCREEN, 0,   BRIDGE2_3w};
act8 ascr_clftop_clf_3w     = {NEW_SCREEN, 0,   CLIFF_3w};
act8 ascr_clftop_slope_3w   = {NEW_SCREEN, 0,   SLOPE_3w};
act8 ascr_crash_web_3w      = {NEW_SCREEN, 0,   WEB_3w};
act8 ascr_garden_wbase_3w   = {NEW_SCREEN, 0,   WBASE_3w};
act8 ascr_hut_camp_3w       = {NEW_SCREEN, 0,   CAMP_3w};
act8 ascr_hut_in_3w         = {NEW_SCREEN, 0,   HUT_IN_3w};
act8 ascr_hut_out_3w        = {NEW_SCREEN, 0,   HUT_OUT_3w};
act8 ascr_hut_village_c_3w  = {NEW_SCREEN, 0,   VILLAGE_3w};
act8 ascr_hut_village_r_3w  = {NEW_SCREEN, 0,   VILLAGE_3w};
act8 ascr_path_brg1_3w      = {NEW_SCREEN, 0,   BRIDGE_3w};
act8 ascr_path_brg2_3w      = {NEW_SCREEN, 0,   BRIDGE2_3w};
act8 ascr_path_village_3w   = {NEW_SCREEN, 0,   VILLAGE_3w};
act8 ascr_path_web_3w       = {NEW_SCREEN, 0,   WEB_3w};
act8 ascr_slope_clftop_3w   = {NEW_SCREEN, 0,   CLIFFTOP_3w};
act8 ascr_slope_stream1_3w  = {NEW_SCREEN, 0,   STREAM_3w};
act8 ascr_slope_stream2_3w  = {NEW_SCREEN, 0,   STREAM2_3w};
act8 ascr_stream_path_3w    = {NEW_SCREEN, 0,   PATH_3w};
act8 ascr_stream_slope_3w   = {NEW_SCREEN, 0,   SLOPE_3w};
act8 ascr_turn_cave_3w      = {NEW_SCREEN, 0,   CAVE_3w};
act8 ascr_turn_village_3w   = {NEW_SCREEN, 0,   VILLAGE_3w};
act8 ascr_village_camp_l_3w = {NEW_SCREEN, 0,   CAMP_3w};
act8 ascr_village_camp_r_3w = {NEW_SCREEN, 0,   CAMP_3w};
act8 ascr_village_path_3w   = {NEW_SCREEN, 0,   PATH_3w};
act8 ascr_village_turn_3w   = {NEW_SCREEN, 0,   TURN_3w};
act8 ascr_wbase_garden_3w   = {NEW_SCREEN, 0,   GARDEN_3w};
act8 ascr_wbase_wfall_3w    = {NEW_SCREEN, 0,   WFALL_3w};
act8 ascr_web_crash_3w      = {NEW_SCREEN, 0,   CRASH_3w};
act8 ascr_web_path_3w       = {NEW_SCREEN, 0,   PATH_UL_3w};
act8 ascr_wfall_cave_3w     = {NEW_SCREEN, 0,   CAVE_3w};
act8 ascr_wfall_clf_3w      = {NEW_SCREEN, 0,   CLIFF_3w};
act8 ascr_wfallb_wbase_3w   = {NEW_SCREEN, 0,   WBASE_3w};
act8 aweb23_3w              = {NEW_SCREEN, 117, CRASH_3w};

act9 abrg_msg2_3w    = {INIT_OBJSTATE, 0,                  VINE_3w,     1};
act9 acageempty_3w   = {INIT_OBJSTATE, 0,                  CAGE_3w,     0};
act9 acamp0a_3w      = {INIT_OBJSTATE, 4,                  NAT2_3w,     0};
act9 acom0b_3w       = {INIT_OBJSTATE, 0,                  NAT1_3w,     1};
act9 acom1b_3w       = {INIT_OBJSTATE, 0,                  NAT1_3w,     2};
act9 acom2b_3w       = {INIT_OBJSTATE, 0,                  NAT1_3w,     3};
act9 acom3b_3w       = {INIT_OBJSTATE, 0,                  NAT1_3w,     4};
act9 acom4b_3w       = {INIT_OBJSTATE, 0,                  NAT1_3w,     5};
act9 acom5b_3w       = {INIT_OBJSTATE, 0,                  NAT1_3w,     6};
act9 acom6b_3w       = {INIT_OBJSTATE, 0,                  NAT1_3w,     7};
act9 acom7b_3w       = {INIT_OBJSTATE, 0,                  NAT1_3w,     8};
act9 acom8b_3w       = {INIT_OBJSTATE, 0,                  NAT1_3w,     9};
act9 adart1_3w       = {INIT_OBJSTATE, 0,                  BLOWPIPE_3w, 1};
act9 adn_3w          = {INIT_OBJSTATE, 0,                  HERO,        0};
act9 adrink_3w       = {INIT_OBJSTATE, 0,                  FLASK_3w,    0};
act9 adropcheese3_3w = {INIT_OBJSTATE, 0,                  CHEESE_3w,   1};
act9 aelewoken_3w    = {INIT_OBJSTATE, 0,                  ELEPHANT_3w, 0};
act9 aemptyflask_3w  = {INIT_OBJSTATE, 0,                  FLASK_3w,    0};
act9 aendaction_3w   = {INIT_OBJSTATE, DARTTIME + 30,      E_EYES_3w,   0};
act9 aenter1_3w      = {INIT_OBJSTATE, 0,                  MOUSE_3w,    2};
act9 aenter9_3w      = {INIT_OBJSTATE, 0,                  CAGE_3w,     1};
act9 aex7_3w         = {INIT_OBJSTATE, 0,                  GHOST_3w,    1};
act9 aex9_3w         = {INIT_OBJSTATE, 0,                  VINE3_3w,    1};
act9 afillmagic3_3w  = {INIT_OBJSTATE, 0,                  FLASK_3w,    2};
act9 afillord2_3w    = {INIT_OBJSTATE, 0,                  FLASK_3w,    1};
act9 afindb1_3w      = {INIT_OBJSTATE, 0,                  BOOK_3w,     1};
act9 agive3_3w       = {INIT_OBJSTATE, 0,                  NAT1_3w,     10};
act9 agot1_3w        = {INIT_OBJSTATE, 0,                  DOCTOR_3w,   1};
act9 ahelp2_3w       = {INIT_OBJSTATE, 0,                  HERO,        1};
act9 ahole5a_3w      = {INIT_OBJSTATE, 0,                  MOUSE_3w,    1};
act9 ahole5b_3w      = {INIT_OBJSTATE, 0,                  MOUSE_3w,    0};
act9 amakeclay2_3w   = {INIT_OBJSTATE, 0,                  CLAY_3w,     1};
act9 amission1_3w    = {INIT_OBJSTATE, 0,                  PENNY_3w,    2};
act9 amousefree_3w   = {INIT_OBJSTATE, 0,                  MOUSE_3w,    3};
act9 aoldstate_3w    = {INIT_OBJSTATE, 0,                  MOUTH_3w,    1};
act9 aopen2_3w       = {INIT_OBJSTATE, 0,                  CDOOR_3w,    1};
act9 apause0_3w      = {INIT_OBJSTATE, 3 * NORMAL_TPS_v2d, NAT2_3w,     0};
act9 apause1_3w      = {INIT_OBJSTATE, 0,                  NAT2_3w,     1};
act9 astartaction_3w = {INIT_OBJSTATE, 0,                  E_EYES_3w,   1};
act9 astick3_3w      = {INIT_OBJSTATE, 0,                  DOCTOR_3w,   2};
act9 atakecheese1_3w = {INIT_OBJSTATE, 0,                  CHEESE_3w,   0};
act9 aup_3w          = {INIT_OBJSTATE, 0,                  HERO,        1};
act9 avine7_3w       = {INIT_OBJSTATE, 0,                  BLOCK1_3w,   1};
act9 aweb1_3w        = {INIT_OBJSTATE, 0,                  PENNY_3w,    1};

act10 acamp1a_3w  = {INIT_PATH, 0,  NAT2_3w,   AUTO,    0,      0};
act10 acamp1b_3w  = {INIT_PATH, 0,  NATG_3w,   AUTO,    0,      0};
act10 acamp9a_3w  = {INIT_PATH, 60, NATG_3w,   CHASE,   DX / 2, DY / 2};
act10 acamp9b_3w  = {INIT_PATH, 55, NAT2_3w,   WANDER,  DX,     2};
act10 achase1_3w  = {INIT_PATH, 0,  NATG_3w,   AUTO,    0,      0};
act10 achase2_3w  = {INIT_PATH, 8,  NATG_3w,   CHASE,   DX / 2, DY / 2};
act10 adoc2_3w    = {INIT_PATH, 0,  DOCTOR_3w, CHASE,   DX,     DY};
act10 aenter3_3w  = {INIT_PATH, 0,  MOUSE_3w,  AUTO,    0,      0};
act10 aex2_3w     = {INIT_PATH, 0,  GHOST_3w,  AUTO,    0,      0};
act10 agot6_3w    = {INIT_PATH, 40, HERO,      USER,    0,      0};
act10 ahole1_3w   = {INIT_PATH, 0,  MOUSE_3w,  AUTO,    0,      0};
act10 ahole6_3w   = {INIT_PATH, 30, MOUSE_3w,  WANDER2, DX,     0};
act10 aold6h_3w   = {INIT_PATH, 22, HERO,      USER,    0,      0};
act10 areturn1_3w = {INIT_PATH, 0,  NATG_3w,   AUTO,    0,      0};
act10 ascare12_3w = {INIT_PATH, 34, MOUSE_3w,  WANDER2, DX * 4, DY};
act10 ascare7_3w  = {INIT_PATH, 0,  MOUSE_3w,  AUTO,    0,      0};
act10 aswing5_3w  = {INIT_PATH, 20, HERO,      USER,    0,      0};
act10 adisable_3w = {INIT_PATH, 0,  HERO,      AUTO,    0,      0};
act10 aenable_3w  = {INIT_PATH, 0,  HERO,      USER,    0,      0};
act10 aquiet_3w   = {INIT_PATH, 0,  HERO,      QUIET,   0,      0};

act11 aactiontest1_3w = {COND_R, 0,                   E_EYES_3w,   1,  0,                    kALoktoleave1_3w};
act11 aactiontest2_3w = {COND_R, 0,                   E_EYES_3w,   1,  0,                    kALoktoleave2_3w};
act11 ablktest_3w     = {COND_R, 0,                   BLOCK1_3w,   0,  kALblk1_3w,           0};
act11 abrgmsgtest_3w  = {COND_R, 0,                   VINE_3w,     0,  kALbrg_clftop_msg_3w, kALbrg_clftop1_3w};
act11 abrgtest_3w     = {COND_R, 0,                   VINE_3w,     0,  kALbrg_ok_3w,         kALbrg_down_3w};
act11 abridgetest_3w  = {COND_R, 0,                   BLOCK1_3w,   0,  kALbridge_3w,         0};
act11 acagetest_3w    = {COND_R, 0,                   MOUSE_3w,    2,  0,                    kALpostest_3w};
act11 acagetest1_3w   = {COND_R, 0,                   MOUSE_3w,    2,  kALcagetest2_3w,      kALok_3w};
act11 acagetest5_3w   = {COND_R, 0,                   MOUSE_3w,    2,  kALcageprompt_3w,     kALmousegone_3w};
act11 acamptest_3w    = {COND_R, 0,                   NAT1_3w,     0,  kALcampers_3w,        kALchase_3w};
act11 acavetest_3w    = {COND_R, 0,                   GHOST_3w,    0,  kALspirit_3w,         kALcave_man_3w};
act11 acrashtest1_3w  = {COND_R, 0,                   PENNY_3w,    0,  kALcrashed_3w,        kALcrashtest2_3w};
act11 acrashtest2_3w  = {COND_R, 0,                   PENNY_3w,    1,  kALcryhelp_3w,        0};
act11 adartedtest_3w  = {COND_R, 0,                   BLOWPIPE_3w, 0,  kALdodart_3w,         kALdarted_3w};
act11 adoctest_3w     = {COND_R, 0,                   DOCTOR_3w,   0,  kALdoc_3w,            0};
act11 adrinktest_3w   = {COND_R, 0,                   FLASK_3w,    0,  kALdrinkno_3w,        kALdrinkyes_3w};
act11 aeletest1_3w    = {COND_R, 0,                   ELEPHANT_3w, 1,  kALele_sleep_3w,      kALeletest2_3w};
act11 aeletest2_3w    = {COND_R, 0,                   ELEPHANT_3w, 0,  kALeleblink_3w,       0};
act11 aemptytest1_3w  = {COND_R, 0,                   FLASK_3w,    2,  kALemptymagic_3w,     kALemptytest2_3w};
act11 aemptytest2_3w  = {COND_R, 0,                   FLASK_3w,    1,  kALemptyord_3w,       kALdrinkno_3w};
act11 aentertest1_3w  = {COND_R, 0,                   CHEESE_3w,   1,  kALentertest2_3w,     0};
act11 aentertest2_3w  = {COND_R, 0,                   MOUSE_3w,    0,  kALentertest3_3w,     0};
act11 aexotest1_3w    = {COND_R, 0,                   GHOST_3w,    0,  kALexotest2_3w,       kALexordone_3w};
act11 afindbtest_3w   = {COND_R, 0,                   BOOK_3w,     0,  kALfindit_3w,         0};
act11 aflasktest2_3w  = {COND_R, 0,                   FLASK_3w,    2,  kALremedy_3w,         kALflasktest3_3w};
act11 aflasktest3_3w  = {COND_R, 0,                   FLASK_3w,    1,  kALnoremedy_3w,       kALrefuseflask_3w};
act11 agettest2_3w    = {COND_R, 0,                   DOCTOR_3w,   0,  kALgot_3w,            0};
act11 agivetest_3w    = {COND_R, 0,                   NAT1_3w,     10, kALrefuse_3w,         kALgive_3w};
act11 ahoriz1_3w      = {COND_R, 0,                   HERO,        0,  kALup_3w,             0};
act11 ahoriz2_3w      = {COND_R, 0,                   HERO,        1,  kALdn_3w,             0};
act11 anat0_3w        = {COND_R, 0,                   NAT1_3w,     0,  kALcom0_3w,           kALnat1_3w};
act11 anat1_3w        = {COND_R, 0,                   NAT1_3w,     1,  kALcom1_3w,           kALnat2_3w};
act11 anat2_3w        = {COND_R, 0,                   NAT1_3w,     2,  kALcom2_3w,           kALnat3_3w};
act11 anat3_3w        = {COND_R, 0,                   NAT1_3w,     3,  kALcom3_3w,           kALnat4_3w};
act11 anat4_3w        = {COND_R, 0,                   NAT1_3w,     4,  kALcom4_3w,           kALnat5_3w};
act11 anat5_3w        = {COND_R, 0,                   NAT1_3w,     5,  kALcom5_3w,           kALnat6_3w};
act11 anat6_3w        = {COND_R, 0,                   NAT1_3w,     6,  kALcom6_3w,           kALnat7_3w};
act11 anat7_3w        = {COND_R, 0,                   NAT1_3w,     7,  kALcom7_3w,           kALnat8_3w};
act11 anat8_3w        = {COND_R, 0,                   NAT1_3w,     8,  kALcom8_3w,           0};
act11 aold5_3w        = {COND_R, 0,                   FLASK_3w,    2,  kALold6_3w,           kALwrong_3w};
act11 aoldmantest_3w  = {COND_R, 0,                   MOUTH_3w,    0,  kALoldfirst_3w,       kALoldsubseq_3w};
act11 aopentest_3w    = {COND_R, 0,                   DOCTOR_3w,   1,  kALprod_3w,           kALopencdoor_3w};
act11 apausetest_3w   = {COND_R, 0,                   NAT2_3w,     1,  0,                    kALcomment_3w};
act11 apostest_3w     = {COND_R, 0,                   MOUSE_3w,    0,  kALmousel_3w,         kALmouser_3w};
act11 aspirittest_3w  = {COND_R, 0,                   GHOST_3w,    0,  kALwarn_3w,           0};
act11 asticktest1_3w  = {COND_R, 0,                   DOCTOR_3w,   2,  kALstuckpin_3w,       kALsticktest2_3w};
act11 asticktest2_3w  = {COND_R, 0,                   CLAY_3w,     1,  kALstickpin_3w,       kALnostickpin_3w};
act11 ataketest1_3w   = {COND_R, 0,                   MOUSE_3w,    2,  kALtakeit_3w,         kALcanttake_3w};
act11 ataketest2_3w   = {COND_R, 0,                   MOUSE_3w,    3,  kALtakeit_3w,         kALtaketest1_3w};
act11 atalktest1_3w   = {COND_R, 0,                   NAT1_3w,     9,  kALnat9_3w,           kALnative_3w};
act11 atdtest_3w      = {COND_R, 0,                   DOCTOR_3w,   2,  kALtalkdoc2_3w,       kALtalkdoc1_3w};
act11 atiptest_3w     = {COND_R, 60 * NORMAL_TPS_v2d, BLOCK1_3w,   0,  kALbtipprompt_3w,     0};
act11 auntietest_3w   = {COND_R, 0,                   BLOCK1_3w,   0,  kALnottied_3w,        kALuntie_3w};
act11 avinetest_3w    = {COND_R, 0,                   BLOCK1_3w,   0,  kALtievine_3w,        kALtied_3w};
act11 awebtest1_3w    = {COND_R, 0,                   PENNY_3w,    0,  kALspider_3w,         kALwebtest2_3w};
act11 awebtest2_3w    = {COND_R, 0,                   PENNY_3w,    1,  kALmission_3w,        kALreturn_3w};

act12 ablk1_3w         = {TEXT, 0,   kSTBlk1_3w};
act12 abook1_3w        = {TEXT, 0,   kSTBook1_3w};
act12 abtip_3w         = {TEXT, 0,   kSTBridgetip_3w};
act12 acanttake_3w     = {TEXT, 0,   kSTCanttake_3w};
act12 acheese2_3w      = {TEXT, 0,   kSTYummy_3w};
act12 acubestip_3w     = {TEXT, 0,   kSTCubestip_3w};
act12 adammedtip_3w    = {TEXT, 0,   kSTDammedtip_3w};
act12 adart0_3w        = {TEXT, 0,   kSTDartElephant_3w};
act12 adarted_3w       = {TEXT, 0,   kSTDarted_3w};
act12 adrinkno_3w      = {TEXT, 0,   kSTDrinkno_3w};
act12 adrinkyes_3w     = {TEXT, 0,   kSTDrinkyes_3w};
act12 adropcheese4_3w  = {TEXT, 0,   kSTDropCheese_3w};
act12 aemptymagic_3w   = {TEXT, 0,   kSTEmptymagic_3w};
act12 aemptyord_3w     = {TEXT, 0,   kSTEmptyord_3w};
act12 aex5_3w          = {TEXT, 0,   kSTExor1_3w};
act12 aex6_3w          = {TEXT, 0,   kSTExor2_3w};
act12 aexordone_3w     = {TEXT, 0,   kSTExordone_3w};
act12 afillmagic2_3w   = {TEXT, 0,   kSTFillmagic_3w};
act12 afillord1_3w     = {TEXT, 0,   kSTFillord_3w};
act12 afindb4_3w       = {TEXT, 2,   kSTFoundbook_3w};
act12 ago1_3w          = {TEXT, 0,   kSTMousefree_3w};
act12 alookfall_3w     = {TEXT, 0,   kSTLookwfall1_3w};
act12 alooknofall_3w   = {TEXT, 0,   kSTLookwfall2_3w};
act12 amagictip_3w     = {TEXT, 0,   kSTMagictip_3w};
act12 amakeclay3_3w    = {TEXT, 0,   kSTMakeeffigy_3w};
act12 amodeltip_3w     = {TEXT, 0,   kSTModeltip_3w};
act12 amousegone_3w    = {TEXT, 0,   kSTMousegone_3w};
act12 amousetip_3w     = {TEXT, 0,   kSTMousetip_3w};
act12 anoblow_3w       = {TEXT, 0,   kSTNoblow_3w};
act12 anoclay_3w       = {TEXT, 0,   kSTNoclay_3w};
act12 anofill_3w       = {TEXT, 0,   kSTNofill_3w};
act12 anomake_3w       = {TEXT, 0,   kSTNomake_3w};
act12 anoremedy1_3w    = {TEXT, 0,   kSTNoremedy_3w};
act12 anospell_3w      = {TEXT, 0,   kSTNospell_3w};
act12 anostick_3w      = {TEXT, 0,   kSTNostick_3w};
act12 anostickpin_3w   = {TEXT, 0,   kSTNostickpin_3w};
act12 anotakecb_3w     = {TEXT, 0,   kSTOldmannotake_3w};
act12 anotip_3w        = {TEXT, 0,   kSTNotip_3w};
act12 anottied_3w      = {TEXT, 0,   kSTNottied_3w};
act12 aold7_3w         = {TEXT, 0,   kSTAllwrong_3w};
act12 aoldmantip_3w    = {TEXT, 0,   kSTOldmantip_3w};
act12 aplanetip_3w     = {TEXT, 0,   kSTPlanetip_3w};
act12 aputitdown_3w    = {TEXT, 0,   kSTPutitdown_3w};
act12 arefuse_3w       = {TEXT, 0,   kSTRefuse_3w};
act12 arefuseflask_3w  = {TEXT, 0,   kSTRefuseflask_3w};
act12 aremedytip_3w    = {TEXT, 0,   kSTRemedytip_3w};
act12 arub_3w          = {TEXT, 0,   kSTRubcrystal_3w};
act12 asteps1_3w       = {TEXT, 0,   kSTStep1_3w};
act12 astick2_3w       = {TEXT, 0,   kSTStickpin_3w};
act12 asticktip_3w     = {TEXT, 0,   kSTSticktip_3w};
act12 astuckpin_3w     = {TEXT, 0,   kSTStuckpin_3w};
act12 aswingtip_3w     = {TEXT, 0,   kSTSwingtip_3w};
act12 atakecb2_3w      = {TEXT, 0,   kSTOldmantakeball_3w};
act12 atalkdoc1_3w     = {TEXT, 0,   kSTTalkdoc_3w};
act12 atalkdoc2_3w     = {TEXT, 0,   kSTTalkdoc2_3w};
act12 atalkdoc3_3w     = {TEXT, 0,   kSTTalkdoc3_3w};
act12 atalkweb_3w      = {TEXT, 0,   kSTTalkweb_3w};
act12 athing_3w        = {TEXT, 0,   kSTVillagething_3w};
act12 atied_3w         = {TEXT, 0,   kSTTiedvine_3w};
act12 auntie_3w        = {TEXT, 0,   kSTUntievine_3w};
act12 avine6_3w        = {TEXT, 0,   kSTBlk2_3w};
act12 awarn_3w         = {TEXT, 0,   kSTCavewarn_3w};
act12 awaterfalling_3w = {TEXT, 0,   kSTWaterfalling_3w};
act12 awrong1_3w       = {TEXT, 0,   kSTWrong_3w};
act12 aclick_3w        = {TEXT, 0,   kSTClick_3w};
act12 aempty_3w        = {TEXT, 0,   kSTEmpty1_3w};
act12 agotit_3w        = {TEXT, 0,   kSTGotit_3w};
act12 anocarry_3w      = {TEXT, 0,   kSTNocarry_3w};
act12 anopurps_3w      = {TEXT, 0,   kSTNopurps_3w};
act12 anothanks_3w     = {TEXT, 0,   kSTNothanks_3w};
act12 aok_3w           = {TEXT, 0,   kSTOkgen_3w};
act12 astalk_3w        = {TEXT, 0,   kSTStalk_3w};

act13 acrash1_3w  = {SWAP_IMAGES, 0,  HERO, HERO_OLD_3w};
act13 aswing2_3w  = {SWAP_IMAGES, 2,  HERO, SWINGER_3w};
act13 aswing7_3w  = {SWAP_IMAGES, 15, HERO, SWINGER_3w};
act13 aweb24_3w   = {SWAP_IMAGES, 0,  HERO, HERO_OLD_3w};
act13 aweb25_3w   = {SWAP_IMAGES, 0,  HERO, WHERO_3w};
act13 aweehero_3w = {SWAP_IMAGES, 0,  HERO, WHERO_3w};

act14 acagetest3_3w  = {COND_SCR, 0, CAGE_3w,   PATH_3w,   kALcagetest4_3w,  kALmousego_3w};
act14 adocscrtest_3w = {COND_SCR, 0, HERO,      HUT_IN_3w, kALtdtest_3w,     kALtalkdoc3_3w};
act14 adroptest1_3w  = {COND_SCR, 0, HERO,      HUT_IN_3w, kALdropincage_3w, kALnocarry_3w};
act14 aexotest2_3w   = {COND_SCR, 0, HERO,      CAVE_3w,   kALexor_3w,       kALnospell_3w};
act14 afilltest1_3w  = {COND_SCR, 0, HERO,      GARDEN_3w, kALfillmagic_3w,  kALfilltest2_3w};
act14 afilltest2_3w  = {COND_SCR, 0, HERO,      STREAM_3w, kALfillord_3w,    kALfilltest3_3w};
act14 afilltest3_3w  = {COND_SCR, 0, HERO,      WFALL_3w,  kALfillord_3w,    kALnofill_3w};
act14 aflasktest1_3w = {COND_SCR, 0, HERO,      WEB_3w,    kALflasktest2_3w, kAL_nothanks_3w};
act14 agettest1_3w   = {COND_SCR, 0, DOCTOR_3w, HUT_IN_3w, kALgettest2_3w,   0};
act14 agivetest1_3w  = {COND_SCR, 0, HERO,      CAMP_3w,   kALgivetest_3w,   kALnothanks_3w};
act14 amaketest_3w   = {COND_SCR, 0, HERO,      HUT_IN_3w, kALmakeit_3w,     kALnomake_3w};
act14 apath2test_3w  = {COND_SCR, 0, HERO,      PATH_3w,   kALdartedtest_3w, kALnoblow_3w};
act14 atalktest2_3w  = {COND_SCR, 0, HERO,      CAMP_3w,   kALtalktest1_3w,  kALstalk_3w};
act14 atalktest3_3w  = {COND_SCR, 0, HERO,      WEB_3w,    kALtalkweb_3w,    kALtalktest2_3w};

act15 agot2_3w  = {AUTOPILOT, 3,  HERO,      CDOOR_3w, DX, DY};
act15 agot2a_3w = {AUTOPILOT, 22, HERO,      CDOOR_3w, DX, DY};
act15 agot3_3w  = {AUTOPILOT, 0,  DOCTOR_3w, CDOOR_3w, DX, DY};
act15 agot3a_3w = {AUTOPILOT, 20, DOCTOR_3w, CDOOR_3w, DX, DY};

act16 abridge1_3w   = {INIT_OBJ_SEQ, 0,            HERO,        LEFT};
act16 acamp5a_3w    = {INIT_OBJ_SEQ, 1,            NAT2_3w,     RIGHT};
act16 acamp5b_3w    = {INIT_OBJ_SEQ, 1,            NATG_3w,     RIGHT};
act16 acamp6b_3w    = {INIT_OBJ_SEQ, 36,           NAT2_3w,     DOWN};
act16 acamp7b_3w    = {INIT_OBJ_SEQ, 40,           NATG_3w,     2};
act16 acrash10_3w   = {INIT_OBJ_SEQ, 8,            HERO,        LEFT};
act16 acrash15_3w   = {INIT_OBJ_SEQ, 21,           PENNY_3w,    DOWN};
act16 acrash16_3w   = {INIT_OBJ_SEQ, 22,           PENNY_3w,    LEFT};
act16 acrash18_3w   = {INIT_OBJ_SEQ, 40,           HERO,        __UP};
act16 acrash2_3w    = {INIT_OBJ_SEQ, 1,            PENNY_3w,    DOWN};
act16 acrash3_3w    = {INIT_OBJ_SEQ, 1,            HERO,        DOWN};
act16 acrash8_3w    = {INIT_OBJ_SEQ, 4,            PENNY_3w,    RIGHT};
act16 adart6_3w     = {INIT_OBJ_SEQ, DARTTIME - 1, E_EYES_3w,   1};
act16 adoc1_3w      = {INIT_OBJ_SEQ, 0,            HERO,        __UP};
act16 aeleblink1_3w = {INIT_OBJ_SEQ, 41,           E_EYES_3w,   1};
act16 aeleblink2_3w = {INIT_OBJ_SEQ, 42,           E_EYES_3w,   0};
act16 aeleblink3_3w = {INIT_OBJ_SEQ, 43,           E_EYES_3w,   1};
act16 aeleblink4_3w = {INIT_OBJ_SEQ, 44,           E_EYES_3w,   0};
act16 aenter7_3w    = {INIT_OBJ_SEQ, 4,            CAGE_3w,     1};
act16 agot11_3w     = {INIT_OBJ_SEQ, 58,           DOCTOR_3w,   LEFT};
act16 ahelp5_3w     = {INIT_OBJ_SEQ, 8,            HERO,        LEFT};
act16 ahole2a_3w    = {INIT_OBJ_SEQ, 0,            MOUSE_3w,    0};
act16 ahole2b_3w    = {INIT_OBJ_SEQ, 0,            MOUSE_3w,    1};
act16 aleft1_3w     = {INIT_OBJ_SEQ, 0,            HERO,        LEFT};
act16 amission13_3w = {INIT_OBJ_SEQ, 10,           NATG_3w,     DOWN};
act16 amission19_3w = {INIT_OBJ_SEQ, 48,           NATG_3w,     RIGHT};
act16 aprod2_3w     = {INIT_OBJ_SEQ, 1,            DOCTOR_3w,   RIGHT};
act16 aprod7_3w     = {INIT_OBJ_SEQ, 25,           DOCTOR_3w,   LEFT};
act16 aright1_3w    = {INIT_OBJ_SEQ, 0,            HERO,        RIGHT};
act16 ascare10_3w   = {INIT_OBJ_SEQ, 18,           MOUSE_3w,    0};
act16 ascare2_3w    = {INIT_OBJ_SEQ, 0,            CAGE_3w,     0};
act16 ascare3_3w    = {INIT_OBJ_SEQ, 0,            ELEPHANT_3w, 1};
act16 ascare8_3w    = {INIT_OBJ_SEQ, 0,            MOUSE_3w,    1};
act16 aweb16_3w     = {INIT_OBJ_SEQ, 105,          PENNY_3w,    RIGHT};
act16 aweb27_3w     = {INIT_OBJ_SEQ, 0,            HERO,        DOWN};
act16 aweb8_3w      = {INIT_OBJ_SEQ, 41,           PENNY_3w,    DOWN};

act17 adart3_3w  = {SET_STATE_BITS, DARTTIME, ELEPHANT_3w, 1};
act17 ascare1_3w = {SET_STATE_BITS, 0,        ELEPHANT_3w, 2};

act19 abittest_3w       = {TEST_STATE_BITS, 0,        ELEPHANT_3w, 1, kALsleepy_3w,      kALscared_3w};
act19 acagetest4_3w     = {TEST_STATE_BITS, 0,        ELEPHANT_3w, 1, kALasleep_3w,      kALscare_3w};
act19 adarttest1_3w     = {TEST_STATE_BITS, DARTTIME, ELEPHANT_3w, 3, kALdammed_3w,      kALbittest_3w};
act19 alookwfalltest_3w = {TEST_STATE_BITS, 0,        ELEPHANT_3w, 3, kALlooknofall_3w,  kALlookfall_3w};
act19 astreamtest_3w    = {TEST_STATE_BITS, 0,        ELEPHANT_3w, 3, kALstream2_3w,     kALstream1_3w};
act19 awfalltest_3w     = {TEST_STATE_BITS, 0,        ELEPHANT_3w, 3, kALwaternofall_3w, kALwaterfall_3w};

act20 adart4_3w   = {DEL_EVENTS, DARTTIME, ASCHEDULE};
act20 adart5_3w   = {DEL_EVENTS, DARTTIME, INIT_OBJ_SEQ};
act20 aold6e_3w   = {DEL_EVENTS, 0,        ASCHEDULE};
act20 aridtest_3w = {DEL_EVENTS, 0,        TEST_STATE_BITS};

act21 adead5_3w = {GAMEOVER, 0};

act23 asunset4_3w = {EXIT, 80};

act24 adammed1_3w     = {BONUS, 0,        11};
act24 adart2_3w       = {BONUS, DARTTIME, 10};
act24 adropcheese1_3w = {BONUS, 0,        6};
act24 aenter0_3w      = {BONUS, 0,        7};
act24 aex1_3w         = {BONUS, 0,        15};
act24 afillmagic1_3w  = {BONUS, 0,        8};
act24 aflask1_3w      = {BONUS, 0,        1};
act24 agive2_3w       = {BONUS, 0,        14};
act24 amakeclay1_3w   = {BONUS, 0,        4};
act24 aold6b_3w       = {BONUS, 0,        12};
act24 ascarebonus_3w  = {BONUS, 0,        9};
act24 astick1_3w      = {BONUS, 0,        5};
act24 aswing0_3w      = {BONUS, 0,        3};
act24 atakecb1_3w     = {BONUS, 0,        16};
act24 atakencheese_3w = {BONUS, 0,        13};
act24 avine1_3w       = {BONUS, 0,        2};

act25 aentertest3_3w = {COND_BOX, 1, MOUSE_3w, 156, 133, 163, 148, kALtrapped_3w, 0};
act25 ahorizpos_3w   = {COND_BOX, 0, HERO,     0,   0,   320, 151, kALhorizup_3w, kALhorizdn_3w};
act25 amousel_3w     = {COND_BOX, 0, HERO,     0,   0,   254, 199, kALholel_3w,   0};
act25 amouser_3w     = {COND_BOX, 0, HERO,     255, 0,   319, 199, kALholer_3w,   0};
act25 aswing4_3w     = {COND_BOX, 1, HERO,     0,   0,   160, 200, kALright_3w,   kALleft_3w};

act26 aexplode_3w    = {SOUND, 0,   BOOM_3w};
act26 afinale_3w     = {SOUND, 0,   T_TRACK1};
act26 aweb17a_3w     = {SOUND, 108, SCREAM_3w};
act26 aballsong_3w   = {SOUND, 0,   MAGIC_3w};
act26 aentersnd_3w   = {SOUND, 0,   SQUEAK_3w};
act26 aex5a_3w       = {SOUND, 0,   WHOOSH_3w};
act26 afillsong_3w   = {SOUND, 0,   MAGIC_3w};
act26 ahey_3w        = {SOUND, 0,   HEY_3w};
act26 anelesong_3w   = {SOUND, 5,   NELLIE_3w};
act26 aoldsnd_3w     = {SOUND, 20,  WHOOSH_3w};
act26 aprodsong_3w   = {SOUND, 20,  POINK_3w};
act26 aspidersong_3w = {SOUND, 105, CHOMP_3w};
act26 asticksong_3w  = {SOUND, 0,   ARGH_3w};
act26 aswingsong_3w  = {SOUND, 0,   YODEL_3w};
act26 atiesong_3w    = {SOUND, 0,   MAGIC_3w};

act27 aaddcheese_3w = {ADD_SCORE, 0, CHEESE_3w};
act27 agive1_3w     = {ADD_SCORE, 0, BLOWPIPE_3w};
act27 atakecage3_3w = {ADD_SCORE, 0, CAGE_3w};

act28 asubcheese_3w = {SUB_SCORE, 0, CHEESE_3w};

act29 acagetest2_3w   = {COND_CARRY, 0, CAGE_3w,    kALputitdown_3w,  kALcagetest3_3w};
act29 acbtest_3w      = {COND_CARRY, 0, CRYSTAL_3w, kALtakecb_3w,     kALnotakecb_3w};
act29 adroptest3_3w   = {COND_CARRY, 0, CHEESE_3w,  kALdroptest2_3w,  kALnocarry_3w};
act29 aold4_3w        = {COND_CARRY, 0, FLASK_3w,   kALold5_3w,       kALwrong_3w};
act29 areadtest1_3w   = {COND_CARRY, 0, BELL_3w,    kALreadtest2_3w,  kALreadord_3w};
act29 areadtest2_3w   = {COND_CARRY, 0, CANDLE_3w,  kALexorcise_3w,   kALreadord_3w};
act29 asticktest4_3w  = {COND_CARRY, 0, CLAY_3w,    kALsticktest1_3w, kALnoclay_3w};
act29 atakechstest_3w = {COND_CARRY, 0, CHEESE_3w,  0,             kALtakechs_3w};
act29 ataketest3_3w   = {COND_CARRY, 0, CAGE_3w,    0,             kALtaketest2_3w};

act33 acamp0c_3w   = {INIT_SCREEN, 0, NATG_3w,   CAMP_3w};
act33 adoc3_3w     = {INIT_SCREEN, 0, DOCTOR_3w, HUT_IN_3w};
act33 amission2_3w = {INIT_SCREEN, 0, NATG_3w,   WEB_3w};
act33 areturn5_3w  = {INIT_SCREEN, 0, NATG_3w,   WEB_3w};
act33 aweb3_3w     = {INIT_SCREEN, 0, PENNY_3w,  WEB_3w};

act35 amap1_3w  = {REMAPPAL, 0, _TLIGHTMAGENTA, _TBLACK};
act35 amap4a_3w = {REMAPPAL, 0, _TGRAY,         _TBLACK};
act35 amap4b_3w = {REMAPPAL, 1, _TGRAY,         _TGRAY};
act35 amap4c_3w = {REMAPPAL, 2, _TGRAY,         _TBLACK};
act35 amap4d_3w = {REMAPPAL, 3, _TGRAY,         _TGRAY};

act36 adroptest2_3w  = {COND_NOUN, 0, kNCage_3w, kALcagetest_3w,   kALdropord_3w};
act36 asticktest3_3w = {COND_NOUN, 0, kNClay_3w, kALsticktest4_3w, kALnostick_3w};

act37 aex8_3w     = {SCREEN_STATE, 0,  CAVE_3w,   1};
act37 ascare17_3w = {SCREEN_STATE, 64, PATH_3w,   1};
act37 astick7_3w  = {SCREEN_STATE, 17, HUT_IN_3w, 1};

act38 amission14_3w = {INIT_LIPS, 20,  LIPS_3w, NATG_3w,  4,     LIPDY + 1};
act38 aweb18_3w     = {INIT_LIPS, 108, LIPS_3w, PENNY_3w, LIPDX, LIPDY};
act38 alips_3w      = {INIT_LIPS, 0,   LIPS_3w, PENNY_3w, LIPDX, LIPDY};

act39 amission23_3w  = {INIT_STORY_MODE, 50, false};
act39 astory_mode_3w = {INIT_STORY_MODE, 0,  true};

// All the act40 were defined as act12 with a type set to WARN
act40 aasleep_3w       = {WARN, 30,  kSTAsleep_3w};
act40 abrg_msg1_3w     = {WARN, 0,   kSTBridgedown_3w};
act40 acom0a_3w        = {WARN, 0,   kSTCom0_3w};
act40 acom1a_3w        = {WARN, 0,   kSTCom1_3w};
act40 acom2a_3w        = {WARN, 0,   kSTCom2_3w};
act40 acom3a_3w        = {WARN, 0,   kSTCom3_3w};
act40 acom4a_3w        = {WARN, 0,   kSTCom4_3w};
act40 acom5a_3w        = {WARN, 0,   kSTCom5_3w};
act40 acom6a_3w        = {WARN, 0,   kSTCom6_3w};
act40 acom7a_3w        = {WARN, 0,   kSTCom7_3w};
act40 acom8a_3w        = {WARN, 0,   kSTCom8_3w};
act40 acom9_3w         = {WARN, 0,   kSTCom9_3w};
act40 acrash11_3w      = {WARN, 20,  kSTPenny1_3w};
act40 acrash12_3w      = {WARN, 20,  kSTPenny2_3w};
act40 acrash13_3w      = {WARN, 34,  kSTPenny3_3w};
act40 adammed2_3w      = {WARN, 0,   kSTDammed_3w};
act40 aelewaking_3w    = {WARN, 0,   kSTElewaking_3w};
act40 aenter2_3w       = {WARN, 0,   kSTMouse1_3w};
act40 aflask2_3w       = {WARN, 0,   kSTEnd1_3w};
act40 aflask3_3w       = {WARN, 0,   kSTEnd2_3w};
act40 aflask4_3w       = {WARN, 0,   kSTEnd3_3w};
act40 agive4_3w        = {WARN, 0,   kSTGiveb1_3w};
act40 agive5_3w        = {WARN, 0,   kSTGiveb2_3w};
act40 agot1c_3w        = {WARN, 1,   kSTGot1_3w};
act40 agot9_3w         = {WARN, 54,  kSTGot2_3w};
act40 ahelp1_3w        = {WARN, 2,   kSTHelp1_3w};
act40 amission10_3w    = {WARN, 5,   kSTMission1_3w};
act40 amission16_3w    = {WARN, 30,  kSTMission2_3w};
act40 amission20_3w    = {WARN, 50,  kSTMission3_3w};
act40 amission21_3w    = {WARN, 50,  kSTMission4_3w};
act40 amission22_3w    = {WARN, 50,  kSTMission5_3w};
act40 amission24_3w    = {WARN, 60,  kSTMission6_3w};
act40 aold0a_3w        = {WARN, 40,  kSTOldman0a_3w};
act40 aold0b_3w        = {WARN, 40,  kSTOldman0b_3w};
act40 aold6c_3w        = {WARN, 0,   kSTOldman4_3w};
act40 aold6d_3w        = {WARN, 0,   kSTOldman5_3w};
act40 aprod4_3w        = {WARN, 8,   kSTProd1_3w};
act40 aprod5_3w        = {WARN, 20,  kSTProd2_3w};
act40 ascare13_3w      = {WARN, 12,  kSTScare1_3w};
act40 ascared_3w       = {WARN, 0,   kSTScared_3w};
act40 asleepy_3w       = {WARN, 0,   kSTSleepy_3w};
act40 asunset1_3w      = {WARN, 20,  kSTAdios1_3w};
act40 asunset2_3w      = {WARN, 50,  kSTAdios2_3w};
act40 asunset3_3w      = {WARN, 70,  kSTAdios3_3w};
act40 aweb10_3w        = {WARN, 50,  kSTPenny5_3w};
act40 aweb15_3w        = {WARN, 105, kSTSpider1_3w};
act40 aweb17_3w        = {WARN, 108, kSTSpider2_3w};
act40 aweb20_3w        = {WARN, 110, kSTSpider3_3w};
act40 aweb6_3w         = {WARN, 25,  kSTPenny4_3w};

act41 ac1_3w = {COND_BONUS, 0, 3,  kALac2_3w,       kALswingtip_3w};
act41 ac2_3w = {COND_BONUS, 0, 13, kALac3_3w,       kALplanetip_3w};
act41 ac3_3w = {COND_BONUS, 0, 4,  kALac4_3w,       kALmodeltip_3w};
act41 ac4_3w = {COND_BONUS, 0, 5,  kALac5_3w,       kALsticktip_3w};
act41 ac5_3w = {COND_BONUS, 0, 7,  kALac6_3w,       kALmousetip_3w};
act41 ac6_3w = {COND_BONUS, 0, 14, kALac7_3w,       kALcubestip_3w};
act41 ac7_3w = {COND_BONUS, 0, 11, kALac8_3w,       kALdammedtip_3w};
act41 ac8_3w = {COND_BONUS, 0, 8,  kALac9_3w,       kALmagictip_3w};
act41 ac9_3w = {COND_BONUS, 0, 12, kALremedytip_3w, kALoldmantip_3w};

act42 atakecage4_3w   = {TEXT_TAKE, 0, CAGE_3w};
act42 atakecheese4_3w = {TEXT_TAKE, 0, CHEESE_3w};

act43 abtipprompt_3w   = {YESNO, 0,  kSTBridgeprompt_3w, kALbtip_3w,      kALnotip_3w};
act43 acageprompt_3w   = {YESNO, 0,  kSTCagePrompt_3w,   kALcagetest3_3w, 0};
act43 acheeseprompt_3w = {YESNO, 0,  kSTCheesePrompt_3w, kALeatit_3w,     0};
act43 aold1c_3w        = {YESNO, 60, kSTOldman1_3w,      kALwrong_3w,     kALold2_3w};
act43 aold2_3w         = {YESNO, 0,  kSTOldman2_3w,      kALwrong_3w,     kALold3_3w};
act43 aold3_3w         = {YESNO, 0,  kSTOldman3_3w,      kALold4_3w,      kALold7_3w};

act46 aexit2_3w  = {INIT_JUMPEXIT, 0, false};
act46 aplane2_3w = {INIT_JUMPEXIT, 0, true};

actListPtr ALac2_3w[]            = {&ac2_3w, 0};
actListPtr ALac3_3w[]            = {&ac3_3w, 0};
actListPtr ALac4_3w[]            = {&ac4_3w, 0};
actListPtr ALac5_3w[]            = {&ac5_3w, 0};
actListPtr ALac6_3w[]            = {&ac6_3w, 0};
actListPtr ALac7_3w[]            = {&ac7_3w, 0};
actListPtr ALac8_3w[]            = {&ac8_3w, 0};
actListPtr ALac9_3w[]            = {&ac9_3w, 0};
actListPtr ALasleep_3w[]         = {&astartaction_3w, &aasleep_3w, &amousefree_3w, &acageempty_3w, &ascare2_3w, &ascare5_3w, &ascare6_3w, &ascare7_3w, &ascare8_3w, &ascare9_3w, &ascare10_3w, &ascare11_3w, &ascare12_3w, &ascare17_3w, &aendaction_3w, 0};
actListPtr ALbittest_3w[]        = {&abittest_3w, 0};
actListPtr ALblk1_3w[]           = {&ablk1_3w, 0};
actListPtr ALblk_3w[]            = {&ablktest_3w, 0};
actListPtr ALbrg_clftop1_3w[]    = {&axy_brg_clftop_3w, &ascr_brg_clftop_3w, 0};
actListPtr ALbrg_clftop_3w[]     = {&abrgmsgtest_3w, 0};
actListPtr ALbrg_clftop_msg_3w[] = {&aexplode_3w, &abrg_msg1_3w, &abrg_msg2_3w,  &axy_brg_clftop_3w, &ascr_brg_clftop_3w, 0};
actListPtr ALbrg_down_3w[]       = {&ascr_path_brg2_3w, 0};
actListPtr ALbrg_ok_3w[]         = {&ascr_path_brg1_3w, 0};
actListPtr ALbrg_path_3w[]       = {&axy_brg_path_3w, &ascr_brg_path_3w, 0};
actListPtr ALbridge_3w[]         = {&abridge1_3w, &abridge2_3w, &abridge3_3w, 0};
actListPtr ALbridgetest_3w[]     = {&abridgetest_3w, 0};
actListPtr ALbridgetip_3w[]      = {&atiptest_3w, 0};
actListPtr ALbtip_3w[]           = {&abtip_3w, 0};
actListPtr ALbtipprompt_3w[]     = {&abtipprompt_3w, 0};
actListPtr ALcageprompt_3w[]     = {&acageprompt_3w, 0};
actListPtr ALcagetest2_3w[]      = {&acagetest2_3w, 0};
actListPtr ALcagetest3_3w[]      = {&acagetest3_3w, 0};
actListPtr ALcagetest4_3w[]      = {&acagetest4_3w, 0};
actListPtr ALcagetest_3w[]       = {&adroptest1_3w, 0};
actListPtr ALcamp_3w[]           = {&acamp0a_3w, &acamp0b_3w, &acamp0c_3w, &acamptest_3w, 0};
actListPtr ALcamp_hut_3w[]       = {&axy_camp_hut_3w, &ascr_camp_hut_3w, 0};
actListPtr ALcamp_village_c_3w[] = {&aweehero_3w, &axy_camp_village_c_3w, &ascr_camp_village_c_3w, 0};	// exit center
actListPtr ALcamp_village_l_3w[] = {&aweehero_3w, &axy_camp_village_l_3w, &ascr_camp_village_l_3w, 0};	// exit left
actListPtr ALcampers_3w[]        = {&acamp1a_3w, &acamp1b_3w, &acamp2a_3w, &acamp2b_3w, &acamp3a_3w, &acamp3b_3w, &acamp4a_3w, &acamp4b_3w, &acamp5a_3w, &acamp5b_3w, &acamp6a_3w, &acamp6b_3w, &acamp7a_3w, &acamp7b_3w, &acamp8a_3w, &acamp8b_3w, &acamp9a_3w, &acamp9b_3w, 0};
actListPtr ALcanttake_3w[]       = {&acanttake_3w, 0};
actListPtr ALcave_man_3w[]       = {&adisappear_3w, &adisable_3w, &ascr_cave_man_3w, 0};
actListPtr ALcave_oldman_3w[]    = {&acavetest_3w, 0};
actListPtr ALcave_turn_3w[]      = {&axy_cave_turn_3w, &ascr_cave_turn_3w, 0};
actListPtr ALcave_wfall_3w[]     = {&axy_cave_wfall_3w, &awfalltest_3w, 0};
actListPtr ALchase_3w[]          = {&achase1_3w, &achase2_3w, 0};
actListPtr ALclf_clftop_3w[]     = {&axy_clf_clftop_3w, &ascr_clf_clftop_3w, 0};
actListPtr ALclf_wfall_3w[]      = {&axy_clf_wfall_3w, &awfalltest_3w, 0};
actListPtr ALclftop_brg_3w[]     = {&axy_clftop_brg_3w, &ascr_clftop_brg_3w, 0};
actListPtr ALclftop_clf_3w[]     = {&axy_clftop_clf_3w, &ascr_clftop_clf_3w, 0};
actListPtr ALclftop_slope_3w[]   = {&axy_clftop_slope_3w, &ascr_clftop_slope_3w, 0};
actListPtr ALclosedoor_3w[]      = {&aclose1_3w, &aclose2_3w, 0};
actListPtr ALcom0_3w[]           = {&acom0a_3w, &acom0b_3w, 0};
actListPtr ALcom1_3w[]           = {&acom1a_3w, &acom1b_3w, 0};
actListPtr ALcom2_3w[]           = {&acom2a_3w, &acom2b_3w, 0};
actListPtr ALcom3_3w[]           = {&acom3a_3w, &acom3b_3w, 0};
actListPtr ALcom4_3w[]           = {&acom4a_3w, &acom4b_3w, 0};
actListPtr ALcom5_3w[]           = {&acom5a_3w, &acom5b_3w, 0};
actListPtr ALcom6_3w[]           = {&acom6a_3w, &acom6b_3w, 0};
actListPtr ALcom7_3w[]           = {&acom7a_3w, &acom7b_3w, 0};
actListPtr ALcom8_3w[]           = {&acom8a_3w, &acom8b_3w, 0};
actListPtr ALcomment_3w[]        = {&anat0_3w, &apause0_3w, &apause1_3w, 0};
actListPtr ALcrash_web_3w[]      = {&axy_crash_web_3w, &ascr_crash_web_3w, 0};
actListPtr ALcrashed_3w[]        = {&astory_mode_3w, &aexplode_3w, &acrash1_3w, &acrash2_3w, &acrash3_3w, &acrash4_3w, &acrash5_3w, &acrash6_3w, &acrash7_3w, &acrash8_3w, &acrash9_3w, &acrash10_3w, &acrash11_3w, &acrash12_3w, &acrash13_3w, &acrash14_3w, &acrash15_3w, &acrash16_3w, &acrash17_3w, &acrash18_3w, &acrash19_3w, &acrash20_3w, &acrash21_3w, &acrash22_3w, &acrash23_3w, 0};
actListPtr ALcrashtest2_3w[]     = {&acrashtest2_3w, 0};
actListPtr ALcryhelp_3w[]        = {&aweehero_3w, &ahelp1_3w, &ahelp2_3w, &ahelp3_3w, &ahelp4_3w, &ahelp5_3w, &ahelp6_3w, 0};
actListPtr ALcrystal_3w[]        = {&arub_3w, &ac1_3w, 0};
actListPtr ALcubestip_3w[]       = {&acubestip_3w,  0};
actListPtr ALdammed_3w[]         = {&adammed1_3w, &adammed2_3w, 0};
actListPtr ALdammedtip_3w[]      = {&adammedtip_3w, 0};
actListPtr ALdart_3w[]           = {&apath2test_3w, 0};
actListPtr ALdarted_3w[]         = {&adarted_3w, 0};
actListPtr ALdartedtest_3w[]     = {&adartedtest_3w, 0};
actListPtr ALdartsched_3w[]      = {&adarttest1_3w, 0};
actListPtr ALdn_3w[]             = {&aweehero_3w, &adn_3w, 0};
actListPtr ALdoc_3w[]            = {&aquiet_3w, &astophero_3w, &adoc1_3w, &adoc2_3w, &adoc3_3w, &adoc4_3w, 0};
actListPtr ALdocgot_3w[]         = {&agettest1_3w, 0};
actListPtr ALdodart_3w[]         = {&astartaction_3w, &adart0_3w, &adart1_3w, &adart2_3w, &adart3_3w, &adart4_3w, &adart5_3w, &adart6_3w, &aridtest_3w, &adarttest_3w, &aendaction_3w, 0};
actListPtr ALdrink_3w[]          = {&adrinktest_3w, 0};
actListPtr ALdrinkno_3w[]        = {&adrinkno_3w, 0};
actListPtr ALdrinkyes_3w[]       = {&adrinkyes_3w, &adrink_3w, 0};
actListPtr ALdropcheese_3w[]     = {&adroptest3_3w, 0};
actListPtr ALdropincage_3w[]     = {&asubcheese_3w, &adropcheese4_3w, &adropord2_3w, &adropord3_3w, &adropcheese1_3w, &adropcheese2_3w, &adropcheese3_3w, 0};
actListPtr ALdropord_3w[]        = {&asubcheese_3w, &aok_3w, &adropord1_3w, &adropord2_3w, &adropord3_3w, 0};
actListPtr ALdroptest2_3w[]      = {&adroptest2_3w, 0};
actListPtr ALeatcheese_3w[]      = {&acheeseprompt_3w, 0};
actListPtr ALeatit_3w[]          = {&asubcheese_3w, &acheese1_3w, &acheese2_3w, 0};
actListPtr ALele_sleep_3w[]      = {&aeleblink1_3w, 0};
actListPtr ALeleblink_3w[]       = {&arepblink_3w, &aeleblink1_3w, &aeleblink2_3w, &aeleblink3_3w, &aeleblink4_3w, 0};
actListPtr ALeletest2_3w[]       = {&aeletest2_3w, 0};
actListPtr AL_empty_3w[]         = {&aemptytest1_3w, 0};
actListPtr ALemptymagic_3w[]     = {&aemptyflask_3w, &aemptymagic_3w, 0};
actListPtr ALemptyord_3w[]       = {&aemptyflask_3w, &aemptyord_3w, 0};
actListPtr ALemptytest2_3w[]     = {&aemptytest2_3w, 0};
actListPtr ALentertest2_3w[]     = {&aentertest2_3w, 0};
actListPtr ALentertest3_3w[]     = {&aentertest3_3w, 0};
actListPtr ALexit_3w[]           = {&aappear_3w, &aenable_3w, &aexit1_3w, &aexit2_3w, &aexit3_3w, 0};
actListPtr ALexor_3w[]           = {&aex1_3w, &aex2_3w, &aex3_3w, &aex4_3w, &aex5_3w, &aex5a_3w, &aex6_3w, &aex7_3w, &aex8_3w, &aex9_3w, 0};
actListPtr ALexorcise_3w[]       = {&aexotest1_3w, 0};
actListPtr ALexordone_3w[]       = {&aexordone_3w, 0};
actListPtr ALexotest2_3w[]       = {&aexotest2_3w, 0};
actListPtr ALfill_3w[]           = {&afilltest1_3w, 0};
actListPtr ALfillmagic_3w[]      = {&afillsong_3w, &afillmagic1_3w, &afillmagic2_3w, &afillmagic3_3w, 0};
actListPtr ALfillord_3w[]        = {&afillord1_3w, &afillord2_3w, 0};
actListPtr ALfilltest2_3w[]      = {&afilltest2_3w, 0};
actListPtr ALfilltest3_3w[]      = {&afilltest3_3w, 0};
actListPtr ALfindbook_3w[]       = {&afindbtest_3w, 0};
actListPtr ALfindcrystal_3w[]    = {&aballsong_3w, &afind1_3w, &afind2_3w, 0};
actListPtr ALfindit_3w[]         = {&afindb1_3w, &afindb2_3w, &afindb3_3w, &afindb4_3w, 0};
actListPtr ALflash_3w[]          = {&arepflash_3w, &amap4a_3w, &amap4b_3w, &amap4c_3w, &amap4d_3w, 0};
actListPtr ALflask_3w[]          = {&aflasktest1_3w, 0};
actListPtr ALflasktest2_3w[]     = {&aflasktest2_3w, 0};
actListPtr ALflasktest3_3w[]     = {&aflasktest3_3w, 0};
actListPtr ALgarden_wbase_3w[]   = {&axy_garden_wbase_3w, &ascr_garden_wbase_3w, 0};
actListPtr ALgettest2_3w[]       = {&agettest2_3w, 0};
actListPtr ALgive_3w[]           = {&agive1_3w, &agive2_3w, &agive3_3w, &agive4_3w, &agive5_3w, &agive6_3w, &agive7_3w, 0};
actListPtr ALgiveb_3w[]          = {&agivetest1_3w, 0};
actListPtr ALgivetest_3w[]       = {&agivetest_3w, 0};
actListPtr ALgot_3w[]            = {&agot1_3w, &agot1a_3w, &agot1b_3w, &agot1c_3w, &agot2_3w, &agot3_3w, &agot2a_3w, &agot3a_3w, &agot2b_3w, &agot3b_3w, &agot4_3w, &agot5_3w, &agot5a_3w, &agot6_3w, &agot7_3w, &agot8_3w, &agot9_3w, &agot10_3w, &agot11_3w, &agot12_3w, 0};
actListPtr ALholel_3w[]          = {&ahole1_3w, &ahole2a_3w, &ahole3a_3w, &ahole4a_3w, &ahole5a_3w, &ahole6_3w, 0};
actListPtr ALholer_3w[]          = {&ahole1_3w, &ahole2b_3w, &ahole3b_3w, &ahole4b_3w, &ahole5b_3w, &ahole6_3w, 0};
actListPtr ALhorizdn_3w[]        = {&ahoriz2_3w, 0};
actListPtr ALhorizon_3w[]        = {&ahorizpos_3w, &arepeathorizon_3w, 0};
actListPtr ALhorizup_3w[]        = {&ahoriz1_3w, 0};
actListPtr ALhut_camp_3w[]       = {&axy_hut_camp_3w, &ascr_hut_camp_3w, 0};
actListPtr ALhut_enter_3w[]      = {&axy_hut_in_3w, &ascr_hut_in_3w, 0};
actListPtr ALhut_in_3w[]         = {&adoctest_3w, 0};
actListPtr ALhut_out_3w[]        = {&axy_hut_out_3w, &ascr_hut_out_3w, 0};
actListPtr ALhut_village_c_3w[]  = {&aweehero_3w, &axy_hut_village_c_3w, &ascr_hut_village_c_3w, 0};
actListPtr ALhut_village_r_3w[]  = {&aweehero_3w, &axy_hut_village_r_3w, &ascr_hut_village_r_3w, 0};
actListPtr ALleft_3w[]           = {&aleft1_3w, &aleft2_3w, &aleft3_3w, &aleft4_3w, 0};
actListPtr ALlookfall_3w[]       = {&alookfall_3w, 0};
actListPtr ALlooknofall_3w[]     = {&alooknofall_3w, 0};
actListPtr ALlookwfall_3w[]      = {&alookwfalltest_3w, 0};
actListPtr ALmagictip_3w[]       = {&amagictip_3w, 0};
actListPtr ALmakeclay_3w[]       = {&amaketest_3w, 0};
actListPtr ALmakeit_3w[]         = {&amakeclay1_3w, &amakeclay2_3w, &amakeclay3_3w, 0};
actListPtr ALmission_3w[]        = {&amission1_3w, &amission2_3w, &amission3_3w, &amission4_3w, &amission5_3w, &amission6_3w, &amission7_3w, &amission8_3w, &amission9_3w, &amission10_3w, &amission11_3w, &amission12_3w, &amission13_3w, &amission14_3w, &amission15_3w, &amission16_3w, &amission17_3w, &amission18_3w, &amission19_3w, &amission20_3w, &amission21_3w, &amission22_3w, &amission23_3w, &amission24_3w, &amission25_3w, 0};
actListPtr ALmodeltip_3w[]       = {&amodeltip_3w, 0};
actListPtr ALmouse_3w[]          = {&acagetest_3w, &aentertest1_3w, &arepeatmouse_3w, 0};
actListPtr ALmousego_3w[]        = {&ago1_3w, &amousefree_3w, &acageempty_3w, &ascare2_3w, 0};
actListPtr ALmousegone_3w[]      = {&amousegone_3w, 0};
actListPtr ALmousel_3w[]         = {&amousel_3w, 0};
actListPtr ALmouser_3w[]         = {&amouser_3w, 0};
actListPtr ALmousetip_3w[]       = {&amousetip_3w, 0};
actListPtr ALnat1_3w[]           = {&anat1_3w, 0};
actListPtr ALnat2_3w[]           = {&anat2_3w, 0};
actListPtr ALnat3_3w[]           = {&anat3_3w, 0};
actListPtr ALnat4_3w[]           = {&anat4_3w, 0};
actListPtr ALnat5_3w[]           = {&anat5_3w, 0};
actListPtr ALnat6_3w[]           = {&anat6_3w, 0};
actListPtr ALnat7_3w[]           = {&anat7_3w, 0};
actListPtr ALnat8_3w[]           = {&anat8_3w, 0};
actListPtr ALnat9_3w[]           = {&acom9_3w, 0};
actListPtr ALnative_3w[]         = {&apausetest_3w, 0};
actListPtr ALnoblow_3w[]         = {&anoblow_3w, 0};
actListPtr ALnoclay_3w[]         = {&anoclay_3w, 0};
actListPtr ALnofill_3w[]         = {&anofill_3w, 0};
actListPtr ALnomake_3w[]         = {&anomake_3w, 0};
actListPtr ALnoremedy_3w[]       = {&anoremedy1_3w, 0};
actListPtr ALnospell_3w[]        = {&anospell_3w, 0};
actListPtr ALnostick_3w[]        = {&anostick_3w, 0};
actListPtr ALnostickpin_3w[]     = {&anostickpin_3w, 0};
actListPtr ALnotakecb_3w[]       = {&anotakecb_3w, 0};
actListPtr AL_nothanks_3w[]      = {&anothanks_3w, 0};
actListPtr ALnotip_3w[]          = {&anotip_3w, 0};
actListPtr ALnottied_3w[]        = {&anottied_3w, 0};
actListPtr ALoktoleave1_3w[]     = {&aweehero_3w, &axy_path_village_3w, &ascr_path_village_3w, 0};
actListPtr ALoktoleave2_3w[]     = {&axy_path_stream_3w, &astreamtest_3w, 0};
actListPtr ALold2_3w[]           = {&aold2_3w, 0};
actListPtr ALold3_3w[]           = {&aold3_3w, 0};
actListPtr ALold4_3w[]           = {&aold4_3w, 0};
actListPtr ALold5_3w[]           = {&aold5_3w, 0};
actListPtr ALold6_3w[]           = {&aold6a_3w, &aold6b_3w, &aold6c_3w, &aold6d_3w, &aold6e_3w, &acbtest_3w, &awink1_3w, &awink2_3w, &awink3_3w, &aold6f_3w, &aoldsnd_3w, &aold6g_3w, &aold6h_3w, &aold6i_3w, 0};
actListPtr ALold7_3w[]           = {&aappear1_3w, &aenable_3w, &aold7_3w, &amap4b_3w, &areturn_3w, 0};
actListPtr ALoldfirst_3w[]       = {&aoldstate_3w, &aold0a_3w, &aold1a_3w, &aold1b_3w, &aold1c_3w, 0};
actListPtr ALoldman_3w[]         = {&aoldmantest_3w, 0};
actListPtr ALoldmantip_3w[]      = {&aoldmantip_3w, 0};
actListPtr ALoldsubseq_3w[]      = {&aoldstate_3w, &aold0b_3w, &aold1a_3w, &aold1b_3w, &aold1c_3w, 0};
actListPtr ALopencage_3w[]       = {&acagetest1_3w, 0};
actListPtr ALopencdoor_3w[]      = {&aopen1_3w, &aopen2_3w, 0};
actListPtr ALopendoor_3w[]       = {&aopentest_3w, 0};
actListPtr ALpath_3w[]           = {&aeletest1_3w, 0};
actListPtr ALpath_brg_3w[]       = {&axy_path_brg_3w, &abrgtest_3w, 0};
actListPtr ALpath_stream_3w[]    = {&aactiontest2_3w, 0};
actListPtr ALpath_village_3w[]   = {&aactiontest1_3w, 0};
actListPtr ALpath_web_3w[]       = {&axy_path_web_3w, &ascr_path_web_3w, 0};
actListPtr ALplane_3w[]          = {&adisappear_3w, &adisable_3w, &aplane1_3w, &aplane2_3w, &aplane3_3w, 0};
actListPtr ALplanetip_3w[]       = {&aplanetip_3w, 0};
actListPtr ALpostest_3w[]        = {&apostest_3w, 0};
actListPtr ALprod_3w[]           = {&ahey_3w, &aprod1_3w, &aprod2_3w, &aprod3_3w, &aprod4_3w, &aprodsong_3w, &aprod5_3w, &aprod6_3w, &aprod7_3w, &aprod8_3w, 0};
actListPtr ALputitdown_3w[]      = {&aputitdown_3w, 0};
actListPtr ALreadbook_3w[]       = {&areadtest1_3w, 0};
actListPtr ALreadord_3w[]        = {&abook1_3w, 0};
actListPtr ALreadtest2_3w[]      = {&areadtest2_3w, 0};
actListPtr ALrefuse_3w[]         = {&arefuse_3w, 0};
actListPtr ALrefuseflask_3w[]    = {&arefuseflask_3w, 0};
actListPtr ALremedy_3w[]         = {&afinale_3w, &aflask1_3w, &aflask2_3w, &aflask3_3w, &aflask4_3w, &aflask5_3w, &astophero_3w, &adisable_3w, &aflask6_3w, 0};
actListPtr ALremedytip_3w[]      = {&aremedytip_3w, 0};
actListPtr ALreturn_3w[]         = {&areturn1_3w, &areturn2_3w, &areturn3_3w, &areturn4_3w, &areturn5_3w, 0};
actListPtr ALright_3w[]          = {&aright1_3w, &aright2_3w, &aright3_3w, &aright4_3w, 0};
actListPtr ALscare_3w[]          = {&astartaction_3w, &ascarebonus_3w, &anelesong_3w, &amousefree_3w, &acageempty_3w, &ascare1_3w, &ascare2_3w, &ascare3_3w, &ascare4_3w, &ascare5_3w, &ascare6_3w, &ascare7_3w, &ascare8_3w, &ascare9_3w, &ascare10_3w, &ascare11_3w, &ascare12_3w, &ascare13_3w, &ascare14_3w, &ascare15_3w, &ascare16_3w, &ascare17_3w, &aridtest_3w, &adarttest_3w, &aendaction_3w, 0};
actListPtr ALscared_3w[]         = {&ascared_3w, 0};
actListPtr ALsleepy_3w[]         = {&asleepy_3w, 0};
actListPtr ALslope_clftop_3w[]   = {&axy_slope_clftop_3w, &ascr_slope_clftop_3w, 0};
actListPtr ALslope_stream_3w[]   = {&axy_slope_stream_3w, &astreamtest_3w, 0};
actListPtr ALspider_3w[]         = {&aplantfix_3w, &aspidersong_3w, &amap1_3w, &aweb1_3w, &aweb2_3w, &aweb3_3w, &aweb4_3w, &aweb5_3w, &aweb6_3w, &aweb7_3w, &aweb8_3w, &aweb9_3w, &aweb10_3w, &aweb11_3w, &aweb12_3w, &aweb13_3w, &aweb14_3w, &aweb15_3w, &aweb16_3w, &aweb17a_3w, &aweb17_3w, &aweb18_3w, &aweb19_3w, &aweb20_3w, &aweb21_3w, &aweb22_3w, &aweb23_3w, &aweb24_3w, &aweb25_3w, &aweb26_3w, &aweb27_3w, 0};
actListPtr ALspirit_3w[]         = {&aspirittest_3w, 0};
actListPtr ALsteps_3w[]          = {&asteps1_3w, 0};
actListPtr ALstick_3w[]          = {&asticktest3_3w, 0};
actListPtr ALstickpin_3w[]       = {&asticksong_3w, &aprod1_3w, &aprod2_3w, &aprod3_3w, &astick1_3w, &astick2_3w, &astick3_3w, &astick4_3w, &astick5_3w, &astick6_3w, &astick7_3w, 0};
actListPtr ALsticktest1_3w[]     = {&asticktest1_3w, 0};
actListPtr ALsticktest2_3w[]     = {&asticktest2_3w, 0};
actListPtr ALsticktest4_3w[]     = {&asticktest4_3w, 0};
actListPtr ALsticktip_3w[]       = {&asticktip_3w,  0};
actListPtr ALstream1_3w[]        = {&ascr_slope_stream1_3w, 0};
actListPtr ALstream2_3w[]        = {&ascr_slope_stream2_3w, 0};
actListPtr ALstream_path_3w[]    = {&axy_stream_path_3w, &ascr_stream_path_3w, 0};
actListPtr ALstream_slope_3w[]   = {&axy_stream_slope_3w, &ascr_stream_slope_3w, 0};
actListPtr ALstuckpin_3w[]       = {&astuckpin_3w, 0};
actListPtr ALsunset_3w[]         = {&asunset1_3w, &asunset2_3w, &asunset3_3w, &asunset4_3w, 0};
actListPtr ALswing_3w[]          = {&aquiet_3w, &aswingsong_3w, &aswing0_3w, &aswing1_3w, &aswing2_3w, &aswing3_3w, &aswing4_3w, &aswing5_3w, &aswing6_3w, &aswing7_3w, &aswing8_3w, 0};
actListPtr ALswingtip_3w[]       = {&aswingtip_3w,  0};
actListPtr ALtakecage_3w[]       = {&ataketest3_3w, 0};
actListPtr ALtakecb_3w[]         = {&atakecb1_3w, &atakecb2_3w, &atakecb3_3w, 0};
actListPtr ALtakecheese_3w[]     = {&atakechstest_3w, 0};
actListPtr ALtakechs_3w[]        = {&aaddcheese_3w, &atakecheese4_3w, &atakencheese_3w, &atakecheese1_3w, &atakecheese2_3w, &atakecheese3_3w, 0};
actListPtr ALtakeit_3w[]         = {&atakecage4_3w, &atakecage1_3w, &atakecage2_3w, &atakecage3_3w, 0};
actListPtr ALtaketest1_3w[]      = {&ataketest1_3w, 0};
actListPtr ALtaketest2_3w[]      = {&ataketest2_3w, 0};
actListPtr ALtalkdoc1_3w[]       = {&atalkdoc1_3w, 0};
actListPtr ALtalkdoc2_3w[]       = {&atalkdoc2_3w, 0};
actListPtr ALtalkdoc3_3w[]       = {&atalkdoc3_3w, 0};
actListPtr ALtalkdoc_3w[]        = {&adocscrtest_3w, 0};
actListPtr ALtalknat_3w[]        = {&atalktest3_3w, 0};
actListPtr ALtalktest1_3w[]      = {&atalktest1_3w, 0};
actListPtr ALtalktest2_3w[]      = {&atalktest2_3w, 0};
actListPtr ALtalkweb_3w[]        = {&atalkweb_3w, 0};
actListPtr ALtdtest_3w[]         = {&atdtest_3w, 0};
actListPtr ALtied_3w[]           = {&atied_3w, 0};
actListPtr ALtievine_3w[]        = {&atiesong_3w, &avine1_3w, &avine2_3w, &avine3_3w, &avine4_3w, &avine5_3w, &avine6_3w, &avine7_3w, 0};
actListPtr ALtrapped_3w[]        = {&aaddcheese_3w, &aenter0_3w, &aenter1_3w, &aentersnd_3w, &aenter2_3w, &aenter3_3w, &aenter4_3w, &aenter5_3w, &aenter6_3w, &aenter7_3w, &aenter8_3w, &aenter9_3w, 0};
actListPtr ALturn_cave_3w[]      = {&axy_turn_cave_3w, &ascr_turn_cave_3w, 0};
actListPtr ALturn_village_3w[]   = {&aweehero_3w, &axy_turn_village_3w, &ascr_turn_village_3w, 0};
actListPtr ALuntie_3w[]          = {&auntie_3w, 0};
actListPtr ALuntie_vine_3w[]     = {&auntietest_3w, 0};
actListPtr ALup_3w[]             = {&aweehero_3w, &aup_3w, 0};
actListPtr ALusecage_3w[]        = {&acagetest5_3w, 0};
actListPtr ALvillage_camp_l_3w[] = {&aweehero_3w, &axy_village_camp_l_3w, &ascr_village_camp_l_3w, 0};
actListPtr ALvillage_camp_r_3w[] = {&aweehero_3w, &axy_village_camp_r_3w, &ascr_village_camp_r_3w, 0};
actListPtr ALvillage_path_3w[]   = {&aweehero_3w, &axy_village_path_3w, &ascr_village_path_3w, 0};
actListPtr ALvillage_thing_3w[]  = {&athing_3w, 0};
actListPtr ALvillage_turn_3w[]   = {&aweehero_3w, &axy_village_turn_3w, &ascr_village_turn_3w, 0};
actListPtr ALvine_3w[]           = {&avinetest_3w, 0};
actListPtr ALwarn_3w[]           = {&awarn_3w, 0};
actListPtr ALwaterfall_3w[]      = {&ascr_clf_wfall_3w, 0};
actListPtr ALwaternofall_3w[]    = {&ascr_clf_wnofall_3w, 0};
actListPtr ALwbase_garden_3w[]   = {&axy_wbase_garden_3w, &ascr_wbase_garden_3w, 0};
actListPtr ALwbase_wfall_3w[]    = {&aelewaking_3w, &aelewoken_3w,  &axy_wbase_wfall_3w, &ascr_wbase_wfall_3w, 0};
actListPtr ALweb_3w[]            = {&awebtest1_3w, 0};
actListPtr ALweb_crash_3w[]      = {&axy_web_crash_3w, &ascr_web_crash_3w, 0};
actListPtr ALweb_path_3w[]       = {&axy_web_path_3w, &ascr_web_path_3w, 0};
actListPtr ALwebtest2_3w[]       = {&awebtest2_3w, 0};
actListPtr ALwfall_cave_3w[]     = {&axy_wfall_cave_3w, &ascr_wfall_cave_3w, 0};
actListPtr ALwfall_clf_3w[]      = {&axy_wfall_clf_3w, &ascr_wfall_clf_3w, 0};
actListPtr ALwfall_wbase_3w[]    = {&awaterfalling_3w, 0};
actListPtr ALwfallb_cave_3w[]    = {&axy_wfall_cave_3w, &ascr_wfall_cave_3w, 0};
actListPtr ALwfallb_clf_3w[]     = {&axy_wfall_clf_3w, &ascr_wfall_clf_3w, 0};
actListPtr ALwfallb_wbase_3w[]   = {&axy_wfallb_wbase_3w, &ascr_wfallb_wbase_3w, 0};
actListPtr ALwrong_3w[]          = {&aappear1_3w, &aenable_3w, &awrong1_3w, &amap4b_3w, &areturn_3w, 0};
actListPtr ALempty_3w[]          = {&aempty_3w, 0};
actListPtr ALgotit_3w[]          = {&agotit_3w, 0};
actListPtr ALnocarry_3w[]        = {&anocarry_3w, 0};
actListPtr ALnopurps_3w[]        = {&anopurps_3w, 0};
actListPtr ALnothanks_3w[]       = {&anothanks_3w, 0};
actListPtr ALok_3w[]             = {&aok_3w, 0};
actListPtr ALstalk_3w[]          = {&astalk_3w, 0};
actListPtr ALcrashStory_3w[]     = {&acrashtest1_3w, 0};
actListPtr ALcrashNoStory_3w[]   = {&amap1_3w, &acrashtest1_3w, 0};

actList actListArr_3w[] = {
	ALDummy,             ALac2_3w,            ALac3_3w,            ALac4_3w,            ALac5_3w,
	ALac6_3w,            ALac7_3w,            ALac8_3w,            ALac9_3w,            ALasleep_3w,
	ALbittest_3w,        ALblk1_3w,           ALblk_3w,            ALbrg_clftop1_3w,    ALbrg_clftop_3w,
	ALbrg_clftop_msg_3w, ALbrg_down_3w,       ALbrg_ok_3w,         ALbrg_path_3w,       ALbridge_3w,
	ALbridgetest_3w,     ALbridgetip_3w,      ALbtip_3w,           ALbtipprompt_3w,     ALcageprompt_3w,
	ALcagetest2_3w,      ALcagetest3_3w,      ALcagetest4_3w,      ALcagetest_3w,       ALcamp_3w,
	ALcamp_hut_3w,       ALcamp_village_c_3w, ALcamp_village_l_3w, ALcampers_3w,        ALcanttake_3w,
	ALcave_man_3w,       ALcave_oldman_3w,    ALcave_turn_3w,      ALcave_wfall_3w,     ALchase_3w,
	ALclf_clftop_3w,     ALclf_wfall_3w,      ALclftop_brg_3w,     ALclftop_clf_3w,     ALclftop_slope_3w,
	ALclosedoor_3w,      ALcom0_3w,           ALcom1_3w,           ALcom2_3w,           ALcom3_3w,
	ALcom4_3w,           ALcom5_3w,           ALcom6_3w,           ALcom7_3w,           ALcom8_3w,
	ALcomment_3w,        ALcrash_web_3w,      ALcrashed_3w,        ALcrashtest2_3w,     ALcryhelp_3w,
	ALcrystal_3w,        ALcubestip_3w,       ALdammed_3w,         ALdammedtip_3w,      ALdart_3w,
	ALdarted_3w,         ALdartedtest_3w,     ALdartsched_3w,      ALdn_3w,             ALdoc_3w,
	ALdocgot_3w,         ALdodart_3w,         ALdrink_3w,          ALdrinkno_3w,        ALdrinkyes_3w,
	ALdropcheese_3w,     ALdropincage_3w,     ALdropord_3w,        ALdroptest2_3w,      ALeatcheese_3w,
	ALeatit_3w,          ALele_sleep_3w,      ALeleblink_3w,       ALeletest2_3w,       ALempty_3w,
	ALemptymagic_3w,     ALemptyord_3w,       ALemptytest2_3w,     ALentertest2_3w,     ALentertest3_3w,
	ALexit_3w,           ALexor_3w,           ALexorcise_3w,       ALexordone_3w,       ALexotest2_3w,
	ALfill_3w,           ALfillmagic_3w,      ALfillord_3w,        ALfilltest2_3w,      ALfilltest3_3w,
	ALfindbook_3w,       ALfindcrystal_3w,    ALfindit_3w,         ALflash_3w,          ALflask_3w,
	ALflasktest2_3w,     ALflasktest3_3w,     ALgarden_wbase_3w,   ALgettest2_3w,       ALgive_3w,
	ALgiveb_3w,          ALgivetest_3w,       ALgot_3w,            ALholel_3w,          ALholer_3w,
	ALhorizdn_3w,        ALhorizon_3w,        ALhorizup_3w,        ALhut_camp_3w,       ALhut_enter_3w,
	ALhut_in_3w,         ALhut_out_3w,        ALhut_village_c_3w,  ALhut_village_r_3w,  ALleft_3w,
	ALlookfall_3w,       ALlooknofall_3w,     ALlookwfall_3w,      ALmagictip_3w,       ALmakeclay_3w,
	ALmakeit_3w,         ALmission_3w,        ALmodeltip_3w,       ALmouse_3w,          ALmousego_3w,
	ALmousegone_3w,      ALmousel_3w,         ALmouser_3w,         ALmousetip_3w,       ALnat1_3w,
	ALnat2_3w,           ALnat3_3w,           ALnat4_3w,           ALnat5_3w,           ALnat6_3w,
	ALnat7_3w,           ALnat8_3w,           ALnat9_3w,           ALnative_3w,         ALnoblow_3w,
	ALnoclay_3w,         ALnofill_3w,         ALnomake_3w,         ALnoremedy_3w,       ALnospell_3w,
	ALnostick_3w,        ALnostickpin_3w,     ALnotakecb_3w,       ALnothanks_3w,       ALnotip_3w,
	ALnottied_3w,        ALoktoleave1_3w,     ALoktoleave2_3w,     ALold2_3w,           ALold3_3w,
	ALold4_3w,           ALold5_3w,           ALold6_3w,           ALold7_3w,           ALoldfirst_3w,
	ALoldman_3w,         ALoldmantip_3w,      ALoldsubseq_3w,      ALopencage_3w,       ALopencdoor_3w,
	ALopendoor_3w,       ALpath_3w,           ALpath_brg_3w,       ALpath_stream_3w,    ALpath_village_3w,
	ALpath_web_3w,       ALplane_3w,          ALplanetip_3w,       ALpostest_3w,        ALprod_3w,
	ALputitdown_3w,      ALreadbook_3w,       ALreadord_3w,        ALreadtest2_3w,      ALrefuse_3w,
	ALrefuseflask_3w,    ALremedy_3w,         ALremedytip_3w,      ALreturn_3w,         ALright_3w,
	ALscare_3w,          ALscared_3w,         ALsleepy_3w,         ALslope_clftop_3w,   ALslope_stream_3w,
	ALspider_3w,         ALspirit_3w,         ALsteps_3w,          ALstick_3w,          ALstickpin_3w,
	ALsticktest1_3w,     ALsticktest2_3w,     ALsticktest4_3w,     ALsticktip_3w,       ALstream1_3w,
	ALstream2_3w,        ALstream_path_3w,    ALstream_slope_3w,   ALstuckpin_3w,       ALsunset_3w,
	ALswing_3w,          ALswingtip_3w,       ALtakecage_3w,       ALtakecb_3w,         ALtakecheese_3w,
	ALtakechs_3w,        ALtakeit_3w,         ALtaketest1_3w,      ALtaketest2_3w,      ALtalkdoc1_3w,
	ALtalkdoc2_3w,       ALtalkdoc3_3w,       ALtalkdoc_3w,        ALtalknat_3w,        ALtalktest1_3w,
	ALtalktest2_3w,      ALtalkweb_3w,        ALtdtest_3w,         ALtied_3w,           ALtievine_3w,
	ALtrapped_3w,        ALturn_cave_3w,      ALturn_village_3w,   ALuntie_3w,          ALuntie_vine_3w,
	ALup_3w,             ALusecage_3w,        ALvillage_camp_l_3w, ALvillage_camp_r_3w, ALvillage_path_3w,
	ALvillage_thing_3w,  ALvillage_turn_3w,   ALvine_3w,           ALwarn_3w,           ALwaterfall_3w,
	ALwaternofall_3w,    ALwbase_garden_3w,   ALwbase_wfall_3w,    ALweb_3w,            ALweb_crash_3w,
	ALweb_path_3w,       ALwebtest2_3w,       ALwfall_cave_3w,     ALwfall_clf_3w,      ALwfall_wbase_3w,
	ALwfallb_cave_3w,    ALwfallb_clf_3w,     ALwfallb_wbase_3w,   ALwrong_3w,          ALempty_3w,
	ALgotit_3w,          ALnocarry_3w,        ALnopurps_3w,        ALnothanks_3w,       ALok_3w,
	ALstalk_3w,          ALcrashStory_3w,     ALcrashNoStory_3w
};

int rep1_1d[] =    {kSTsrep1_1d, -1};
int rep2_1d[] =    {kSTsrep2_1d, -1};
int rep3_1d[] =    {kSTsrep3_1d, -1};
int rep4_1d[] =    {kSTsrep4_1d, -1};
int rep5_1d[] =    {kSTsrep5_1d, -1};
int rep6_1d[] =    {kSTsrep6_1d, -1};
int repbut2_1d[] = {kSTsbut2_1d, -1};
int repopen_1d[] = {kSTsopenr_1d, -1};

// Hugo 1 DOS
act0 aend_1d       = {ASCHEDULE, 4 * NORMAL_TPS_v1d,  kALend_1d};
act0 ajailrep_1d   = {ASCHEDULE, 4,                   kALjailrep_1d};
act0 acycle_1d     = {ASCHEDULE, 0,                   kALcycle_1d};
act0 areparm_1d    = {ASCHEDULE, 5 * NORMAL_TPS_v1d,  kALreparm_1d};
act0 arepbat_1d    = {ASCHEDULE, 12 * NORMAL_TPS_v1d, kALbat_1d};
act0 arepbata_1d   = {ASCHEDULE, 3 * NORMAL_TPS_v1d,  kALbatrep_1d};
act0 arepeye_1d    = {ASCHEDULE, 8 * NORMAL_TPS_v1d,  kALblinkeyes1_1d};
act0 arepeye2_1d   = {ASCHEDULE, 8 * NORMAL_TPS_v1d,  kALblinkeyes2_1d};
act0 areplight_1d  = {ASCHEDULE, 11 * NORMAL_TPS_v1d, kALightning_1d};
act0 areplips_1d   = {ASCHEDULE, 4 * NORMAL_TPS_v1d,  kALreplips_1d};
act0 arepredeye_1d = {ASCHEDULE, 6 * NORMAL_TPS_v1d,  kALrepredeye_1d};
act0 aweird_1d     = {ASCHEDULE, 16,                  kALweird_1d};

act1 aridchop_1d     = {START_OBJ, 0,                      CHOP_1d,     0, ALMOST_INVISIBLE};
act1 ashowchop_1d    = {START_OBJ, 0,                      CHOP_1d,     0, NOT_CYCLING};
act1 a115b_1d        = {START_OBJ, 0,                      MASK_1d,     0, NOT_CYCLING};
act1 abin2_1d        = {START_OBJ, 0,                      HERO,        0, ALMOST_INVISIBLE};
act1 about2_1d       = {START_OBJ, 0,                      HERO,        0, NOT_CYCLING};
act1 abox10_1d       = {START_OBJ, 38,                     PROF_1d,     0, CYCLE_FORWARD};
act1 abox11_1d       = {START_OBJ, 73,                     PROF_1d,     0, ALMOST_INVISIBLE};
act1 abut6d_1d       = {START_OBJ, 7,                      HERO,        0, ALMOST_INVISIBLE};
act1 abut6f_1d       = {START_OBJ, 7,                      HDLSHERO_1d, 0, NOT_CYCLING};
act1 aclosetrap_1d   = {START_OBJ, 1 * NORMAL_TPS_v1d,     TRAP_1d,     1, CYCLE_BACKWARD};
act1 adogcyc_1d      = {START_OBJ, 0,                      DOG_1d,      0, CYCLE_FORWARD};
act1 adoggy1_1d      = {START_OBJ, 0,                      HERO,        0, ALMOST_INVISIBLE};
act1 adoggy3_1d      = {START_OBJ, 0,                      HERODEAD_1d, 0, NOT_CYCLING};
act1 aguardgo1_1d    = {START_OBJ, 0,                      GUARD_1d,    0, CYCLE_FORWARD};
act1 ajail2_1d       = {START_OBJ, 0,                      HERO,        0, CYCLE_FORWARD};
act1 alab5_1d        = {START_OBJ, 0,                      PROF_1d,     0, CYCLE_FORWARD};
act1 alab8_1d        = {START_OBJ, 12,                     PROF_1d,     0, NOT_CYCLING};
act1 alab9_1d        = {START_OBJ, 16,                     IGOR_1d,     0, NOT_CYCLING};
act1 aopentrap_1d    = {START_OBJ, 1 * NORMAL_TPS_v1d,     TRAP_1d,     1, CYCLE_FORWARD};
act1 aridbung_1d     = {START_OBJ, 0,                      BUNG_1d,     0, ALMOST_INVISIBLE};
act1 atheend1_1d     = {START_OBJ, 5 * NORMAL_TPS_v1d,     HERO,        0, ALMOST_INVISIBLE};
act1 aarm_1d         = {START_OBJ, 0,                      ARM_1d,      3, CYCLE_BACKWARD};
act1 ablink1a_1d     = {START_OBJ, 0,                      EYES1_1d,    0, ALMOST_INVISIBLE};
act1 ablink1b_1d     = {START_OBJ, 1,                      EYES1_1d,    0, NOT_CYCLING};
act1 ablink1c_1d     = {START_OBJ, 2,                      EYES1_1d,    0, ALMOST_INVISIBLE};
act1 ablink1d_1d     = {START_OBJ, 3,                      EYES1_1d,    0, NOT_CYCLING};
act1 ablink2a_1d     = {START_OBJ, 3 * NORMAL_TPS_v1d + 0, EYES2_1d,    0, ALMOST_INVISIBLE};
act1 ablink2b_1d     = {START_OBJ, 3 * NORMAL_TPS_v1d + 1, EYES2_1d,    0, NOT_CYCLING};
act1 ablink2c_1d     = {START_OBJ, 3 * NORMAL_TPS_v1d + 2, EYES2_1d,    0, ALMOST_INVISIBLE};
act1 ablink2d_1d     = {START_OBJ, 3 * NORMAL_TPS_v1d + 3, EYES2_1d,    0, NOT_CYCLING};
act1 ablink3a_1d     = {START_OBJ, 0,                      EYES3_1d,    0, ALMOST_INVISIBLE};
act1 ablink3b_1d     = {START_OBJ, 1,                      EYES3_1d,    0, NOT_CYCLING};
act1 ablink3c_1d     = {START_OBJ, 2,                      EYES3_1d,    0, ALMOST_INVISIBLE};
act1 ablink3d_1d     = {START_OBJ, 3,                      EYES3_1d,    0, NOT_CYCLING};
act1 ablink4a_1d     = {START_OBJ, 3 * NORMAL_TPS_v1d + 0, EYES4_1d,    0, ALMOST_INVISIBLE};
act1 ablink4b_1d     = {START_OBJ, 3 * NORMAL_TPS_v1d + 1, EYES4_1d,    0, NOT_CYCLING};
act1 ablink4c_1d     = {START_OBJ, 3 * NORMAL_TPS_v1d + 2, EYES4_1d,    0, ALMOST_INVISIBLE};
act1 ablink4d_1d     = {START_OBJ, 3 * NORMAL_TPS_v1d + 3, EYES4_1d,    0, NOT_CYCLING};
act1 ablink5a_1d     = {START_OBJ, 0,                      REDEYES_1d,  0, ALMOST_INVISIBLE};
act1 ablink5b_1d     = {START_OBJ, 1,                      REDEYES_1d,  0, NOT_CYCLING};
act1 ablink5c_1d     = {START_OBJ, 2,                      REDEYES_1d,  0, ALMOST_INVISIBLE};
act1 ablink5d_1d     = {START_OBJ, 3,                      REDEYES_1d,  0, NOT_CYCLING};
act1 abut1_1d        = {START_OBJ, 4 * NORMAL_TPS_v1d,     BUTLER_1d,   0, CYCLE_FORWARD};
act1 aclosedoor1_1d  = {START_OBJ, 1 * NORMAL_TPS_v1d,     DOOR1_1d,    1, CYCLE_BACKWARD};
act1 aclosedoor4_1d  = {START_OBJ, 1 * NORMAL_TPS_v1d,     DOOR4_1d,    1, CYCLE_BACKWARD};
act1 aclosewdoorl_1d = {START_OBJ, 1 * NORMAL_TPS_v1d,     WDOORL_1d,   1, CYCLE_BACKWARD};
act1 aclosewdoorr_1d = {START_OBJ, 1 * NORMAL_TPS_v1d,     WDOORR_1d,   1, CYCLE_BACKWARD};
act1 adog1_1d        = {START_OBJ, 0,                      DOG_1d,      0, CYCLE_FORWARD};
act1 alips_1d        = {START_OBJ, 0,                      LIPS_1d,     6, CYCLE_FORWARD};
act1 amdoor1_1d      = {START_OBJ, 1 * NORMAL_TPS_v1d,     MDOOR_1d,    1, CYCLE_FORWARD};
act1 amovecarp1_1d   = {START_OBJ, 0,                      CARPET_1d,   0, ALMOST_INVISIBLE};
act1 amovecarp2_1d   = {START_OBJ, 0,                      TRAP_1d,     0, NOT_CYCLING};
act1 amum1_1d        = {START_OBJ, 2 * NORMAL_TPS_v1d,     MUMMY_1d,    0, CYCLE_FORWARD};
act1 aopendoor1_1d   = {START_OBJ, 1 * NORMAL_TPS_v1d,     DOOR1_1d,    1, CYCLE_FORWARD};
act1 aopendoor2_1d   = {START_OBJ, 1 * NORMAL_TPS_v1d,     DOOR2_1d,    1, CYCLE_FORWARD};
act1 aopendoor3_1d   = {START_OBJ, 1 * NORMAL_TPS_v1d,     DOOR3_1d,    1, CYCLE_FORWARD};
act1 aopendoor4_1d   = {START_OBJ, 1 * NORMAL_TPS_v1d,     DOOR4_1d,    1, CYCLE_FORWARD};
act1 aopenwdoorl_1d  = {START_OBJ, 1 * NORMAL_TPS_v1d,     WDOORL_1d,   1, CYCLE_FORWARD};
act1 aopenwdoorr_1d  = {START_OBJ, 1 * NORMAL_TPS_v1d,     WDOORR_1d,   1, CYCLE_FORWARD};
act1 aridpkin_1d     = {START_OBJ, 0,                      PKIN_1d,     0, ALMOST_INVISIBLE};
act1 aridprof_1d     = {START_OBJ, 130 / DX,               PROF_1d,     0, ALMOST_INVISIBLE};
act1 ashowkey_1d     = {START_OBJ, 0,                      KEY_1d,      0, NOT_CYCLING};

act2 achopxy_1d     = {INIT_OBJXY, 0,                       CHOP_1d, 51,  155};
act2 aigor12_1d     = {INIT_OBJXY, 30,                      HERO,    116, 112};
act2 a115d_1d       = {INIT_OBJXY, 0,                       MASK_1d, 240, 88};
act2 abata1c_1d     = {INIT_OBJXY, 0,                       BAT2_1d, 65,  25};
act2 abata2c_1d     = {INIT_OBJXY, 0,                       BAT3_1d, 55,  65};
act2 abata3c_1d     = {INIT_OBJXY, 0,                       BAT4_1d, 50,  120};
act2 abata4c_1d     = {INIT_OBJXY, 0,                       BAT5_1d, 55,  130};
act2 abox4a_1d      = {INIT_OBJXY, 20,                      HERO,    124, 122};
act2 adog4_1d       = {INIT_OBJXY, 0,                       DOG_1d,  105, 119};
act2 aheroxy78_1d   = {INIT_OBJXY, 1 * NORMAL_TPS_v1d + 12, HERO,    80,  42};
act2 alab2_1d       = {INIT_OBJXY, 0,                       PROF_1d, 100, 130};
act2 abatxy_1d      = {INIT_OBJXY, 0,                       BAT_1d,  95,  55};
act2 ahchase2_1d    = {INIT_OBJXY, 5 * NORMAL_TPS_v1d,      DOG_1d,  280, 137};
act2 aherofar_1d    = {INIT_OBJXY, 0,                       HERO,    142, 25};
act2 aheronear_1d   = {INIT_OBJXY, 0,                       HERO,    230, 132};
act2 aheroxy01_1d   = {INIT_OBJXY, 0,                       HERO,    106, 130};
act2 aheroxy10_1d   = {INIT_OBJXY, 0,                       HERO,    33,  134};
act2 aheroxy1011_1d = {INIT_OBJXY, 0,                       HERO,    76,  130};
act2 aheroxy109_1d  = {INIT_OBJXY, 0,                       HERO,    96,  105};
act2 aheroxy1110_1d = {INIT_OBJXY, 0,                       HERO,    261, 77};
act2 aheroxy1112_1d = {INIT_OBJXY, 0,                       HERO,    216, 134};
act2 aheroxy115_1d  = {INIT_OBJXY, 1,                       HERO,    27,  130};
act2 aheroxy12_1d   = {INIT_OBJXY, 1 * NORMAL_TPS_v1d + 12, HERO,    169, 90};
act2 aheroxy1211_1d = {INIT_OBJXY, 0,                       HERO,    291, 42};
act2 aheroxy1213_1d = {INIT_OBJXY, 0,                       HERO,    131, 110};
act2 aheroxy13_1d   = {INIT_OBJXY, 0,                       HERO,    40,  127};
act2 aheroxy14_1d   = {INIT_OBJXY, 1 * NORMAL_TPS_v1d + 12, HERO,    135, 115};
act2 aheroxy15_1d   = {INIT_OBJXY, 0,                       HERO,    270, 120};
act2 aheroxy151_1d  = {INIT_OBJXY, 0,                       HERO,    240, 55};
act2 aheroxy21_1d   = {INIT_OBJXY, 0,                       HERO,    130, 56};
act2 aheroxy31_1d   = {INIT_OBJXY, 0,                       HERO,    263, 126};
act2 aheroxy35_1d   = {INIT_OBJXY, 0,                       HERO,    253, 96};
act2 aheroxy41_1d   = {INIT_OBJXY, 0,                       HERO,    200, 56};
act2 aheroxy51_1d   = {INIT_OBJXY, 0,                       HERO,    200, 110};
act2 aheroxy53_1d   = {INIT_OBJXY, 0,                       HERO,    50,  90};
act2 aheroxy56_1d   = {INIT_OBJXY, 0,                       HERO,    290, 140};
act2 aheroxy57_1d   = {INIT_OBJXY, 0,                       HERO,    255, 107};
act2 aheroxy65_1d   = {INIT_OBJXY, 0,                       HERO,    215, 96};
act2 aheroxy75_1d   = {INIT_OBJXY, 0,                       HERO,    20,  110};
act2 aheroxy87_1d   = {INIT_OBJXY, 0,                       HERO,    235, 108};
act2 aheroxy89_1d   = {INIT_OBJXY, 0,                       HERO,    276, 135};
act2 aheroxy910_1d  = {INIT_OBJXY, 0,                       HERO,    50,  132};
act2 aheroxy98_1d   = {INIT_OBJXY, 0,                       HERO,    130, 120};
act2 akchase2_1d    = {INIT_OBJXY, 5 * NORMAL_TPS_v1d,      DOG_1d,  30,  120};

act3 abut4_1d =  {PROMPT, 0,                       kSTsbut1_1d,   repbut2_1d, kALbutyes_1d,  kALbutno_1d,  false};
act3 ahelp1_1d = {PROMPT, 4 * 60 * NORMAL_TPS_v1d, kSTshelpp1_1d, repbut2_1d, kALhelpy_1d,   kALhelpn_1d,  false};
act3 ahelp2_1d = {PROMPT, 5 * 60 * NORMAL_TPS_v1d, kSTshelpp2_1d, repbut2_1d, kALhelpy2_1d,  kALhelpn_1d,  false};
act3 amanq1_1d = {PROMPT, 0,                       kSTsq1_1d,     rep1_1d,    kALrepyes1_1d, kALrepno1_1d, true};
act3 amanq2_1d = {PROMPT, 0,                       kSTsq2_1d,     rep2_1d,    kALrepyes2_1d, kALrepno1_1d, true};
act3 amanq3_1d = {PROMPT, 0,                       kSTsq3_1d,     rep3_1d,    kALrepyes3_1d, kALrepno1_1d, true};
act3 amanq4_1d = {PROMPT, 0,                       kSTsq4_1d,     rep4_1d,    kALrepyes4_1d, kALrepno1_1d, true};
act3 amanq5_1d = {PROMPT, 0,                       kSTsq5_1d,     rep5_1d,    kALrepyes5_1d, kALrepno1_1d, true};
act3 amanq6_1d = {PROMPT, 0,                       kSTsq6_1d,     rep6_1d,    kALrepyes6_1d, kALrepno1_1d, true};
act3 amanq7_1d = {PROMPT, 0,                       kSTsq7_1d,     repbut2_1d, kALrepyes7_1d, kALrepno3_1d, false};
act3 aopenp_1d = {PROMPT, 0,                       kSTsopenp_1d,  repopen_1d, kALopenyes_1d, kALopenno_1d, false};

act4 abg1_1d =     {BKGD_COLOR, 0,  _TLIGHTYELLOW};
act4 abg2_1d =     {BKGD_COLOR, 2,  _TLIGHTMAGENTA};
act4 abg3_1d =     {BKGD_COLOR, 4,  _TLIGHTRED};
act4 abg4_1d =     {BKGD_COLOR, 6,  _TBLACK};
act4 abg5_1d =     {BKGD_COLOR, 8,  _TLIGHTYELLOW};
act4 abg6_1d =     {BKGD_COLOR, 10, _TLIGHTMAGENTA};
act4 abg7_1d =     {BKGD_COLOR, 12, _TLIGHTRED};
act4 abg8_1d =     {BKGD_COLOR, 14, _TBLACK};
act4 abk1_1d =     {BKGD_COLOR, 0,  _TBLUE};
act4 abk2_1d =     {BKGD_COLOR, 2,  _TBLUE};
act4 abknorm1_1d = {BKGD_COLOR, 0,  _TBLACK};
act4 abknorm2_1d = {BKGD_COLOR, 3,  _TBLACK};

act5 abox0_1d =      {INIT_OBJVXY, 0,                  GDOOR_1d,  6,      0};
act5 abox1_1d =      {INIT_OBJVXY, 9,                  GDOOR_1d,  0,      0};
act5 abox7_1d =      {INIT_OBJVXY, 50,                 GDOOR_1d,  -6,     0};
act5 abox8_1d =      {INIT_OBJVXY, 57,                 GDOOR_1d,  0,      0};
act5 abox9_1d =      {INIT_OBJVXY, 38,                 PROF_1d,   -DX,    0};
act5 adoggy5_1d =    {INIT_OBJVXY, 0,                  HERO,      0,      0};
act5 aguardgo3_1d =  {INIT_OBJVXY, 0,                  GUARD_1d,  -DX,    0};
act5 ahin2_1d =      {INIT_OBJVXY, 0,                  HERO,      0,      0};     // Stop hero!
act5 alab3_1d =      {INIT_OBJVXY, 0,                  PROF_1d,   DX,     0};
act5 alab6_1d =      {INIT_OBJVXY, 12,                 PROF_1d,   0,      0};
act5 alab7_1d =      {INIT_OBJVXY, 16,                 IGOR_1d,   0,      0};
act5 abatvxy1_1d =   {INIT_OBJVXY, 0,                  BAT_1d,    2,      -2};
act5 abatvxy2_1d =   {INIT_OBJVXY, 20,                 BAT_1d,    -1,     1};
act5 abatvxy3_1d =   {INIT_OBJVXY, 40,                 BAT_1d,    -1,     -1};
act5 abatvxy4_1d =   {INIT_OBJVXY, 50,                 BAT_1d,    -5,     0};
act5 abatvxy5_1d =   {INIT_OBJVXY, 60,                 BAT_1d,    0,      0};
act5 aboatvxy1_1d =  {INIT_OBJVXY, 0,                  BOAT_1d,   0,      -2};
act5 aboatvxy10_1d = {INIT_OBJVXY, 0,                  BOAT_1d,   9,      7};
act5 aboatvxy11_1d = {INIT_OBJVXY, 3,                  BOAT_1d,   0,      0};
act5 aboatvxy12_1d = {INIT_OBJVXY, 10,                 BOAT_1d,   1,      1};
act5 aboatvxy2_1d =  {INIT_OBJVXY, 10,                 BOAT_1d,   -5,     0};
act5 aboatvxy3_1d =  {INIT_OBJVXY, 20,                 BOAT_1d,   2,      0};
act5 aboatvxy4_1d =  {INIT_OBJVXY, 50,                 BOAT_1d,   1,      -1};
act5 aboatvxy5_1d =  {INIT_OBJVXY, 60,                 BOAT_1d,   -2,     -3};
act5 aboatvxy6_1d =  {INIT_OBJVXY, 70,                 BOAT_1d,   -3,     0};
act5 aboatvxy7_1d =  {INIT_OBJVXY, 100,                BOAT_1d,   0,      -3};
act5 aboatvxy8_1d =  {INIT_OBJVXY, 104,                BOAT_1d,   -1,     -1};
act5 aboatvxy9_1d =  {INIT_OBJVXY, 107,                BOAT_1d,   0,      0};
act5 abutvxy1_1d =   {INIT_OBJVXY, 4 * NORMAL_TPS_v1d, BUTLER_1d, DX - 2, 0};
act5 astophero_1d =  {INIT_OBJVXY, 0,                  HERO,      0,      0};     // Stop hero!

act6 adropchop_1d = {INIT_CARRY, 0, CHOP_1d, false};
act6 a115c_1d =     {INIT_CARRY, 0, MASK_1d, false};
act6 abut7_1d =     {INIT_CARRY, 0, CHOP_1d, true};
act6 adropbung_1d = {INIT_CARRY, 0, BUNG_1d, false};
act6 adropmask_1d = {INIT_CARRY, 0, MASK_1d, false};
act6 adroppkin_1d = {INIT_CARRY, 0, PKIN_1d, false};

act7 abut6e_1d =   {INIT_HH_COORD, 7,                  HDLSHERO_1d}; // Remove hero's head
act7 adoggy2_1d =  {INIT_HH_COORD, 0,                  HERODEAD_1d};
act7 ahchase1_1d = {INIT_HF_COORD, 5 * NORMAL_TPS_v1d, DOG_1d};      // Set screen to hero's
act7 akchase1_1d = {INIT_HF_COORD, 5 * NORMAL_TPS_v1d, DOG_1d};      // Set screen to hero's
act7 amovekey_1d = {INIT_HF_COORD, 0,                  KEY_1d};      // Move key to hero's coords

// Those two were originally defined as act11, but with the type INIT_HF_COORD
// They are now defined as act7 to silence GCC warnings.
act7 adog3_1d =    {INIT_HF_COORD, 0, DOG_1d};      // Set correct screen
act7 alab1_1d =    {INIT_HF_COORD, 0, PROF_1d};     // Set correct screen

act8 aopen78_1d  = {NEW_SCREEN, 1 * NORMAL_TPS_v1d + 12, 8}; // Goto to Basement
act8 atheend2_1d = {NEW_SCREEN, 5 * NORMAL_TPS_v1d,      14};
act8 ascr01_1d   = {NEW_SCREEN, 0,                   1};
act8 ascr10_1d   = {NEW_SCREEN, 0,                   0};
act8 ascr1011_1d = {NEW_SCREEN, 0,                   11};
act8 ascr109_1d  = {NEW_SCREEN, 0,                   9};
act8 ascr1110_1d = {NEW_SCREEN, 0,                   10};
act8 ascr1112_1d = {NEW_SCREEN, 0,                   12};
act8 ascr115_1d  = {NEW_SCREEN, 1,                   15};// Note delay for COND_CARRY
act8 ascr12_1d   = {NEW_SCREEN, 1 * NORMAL_TPS_v1d + 12, 2}; // Goto to Bed1
act8 ascr1211_1d = {NEW_SCREEN, 0,                   11};
act8 ascr1213_1d = {NEW_SCREEN, 0,                   13};
act8 ascr13_1d   = {NEW_SCREEN, 0,                   3};
act8 ascr14_1d   = {NEW_SCREEN, 1 * NORMAL_TPS_v1d + 12, 4}; // Goto to Bathroom
act8 ascr15_1d   = {NEW_SCREEN, 0,                   5};
act8 ascr151_1d  = {NEW_SCREEN, 0,                   1};
act8 ascr21_1d   = {NEW_SCREEN, 0,                   1};
act8 ascr31_1d   = {NEW_SCREEN, 0,                   1};
act8 ascr35_1d   = {NEW_SCREEN, 0,                   5};
act8 ascr41_1d   = {NEW_SCREEN, 0,                   1};
act8 ascr51_1d   = {NEW_SCREEN, 0,                   1};
act8 ascr53_1d   = {NEW_SCREEN, 0,                   3};
act8 ascr56_1d   = {NEW_SCREEN, 0,                   6};
act8 ascr57_1d   = {NEW_SCREEN, 0,                   7};
act8 ascr65_1d   = {NEW_SCREEN, 0,                   5};
act8 ascr75_1d   = {NEW_SCREEN, 0,                   5};
act8 ascr87_1d   = {NEW_SCREEN, 0,                   7};
act8 ascr89_1d   = {NEW_SCREEN, 0,                   9};
act8 ascr910_1d  = {NEW_SCREEN, 0,                   10};
act8 ascr98_1d   = {NEW_SCREEN, 0,                   8};

act9 aigor14_1d =     {INIT_OBJSTATE, 0,                       GDOOR_1d,  2};  // Box now in state 2
act9 aigor23_1d =     {INIT_OBJSTATE, 0,                       GDOOR_1d,  3};  // Box now in state 3
act9 aigor33_1d =     {INIT_OBJSTATE, 0,                       GDOOR_1d,  4};  // Box now in state 4
act9 a115g_1d =       {INIT_OBJSTATE, 0,                       MASK_1d,   0};  // Say mask not worn!
act9 abox12_1d =      {INIT_OBJSTATE, 0,                       GDOOR_1d,  1};  // Box now in state 1
act9 abut10_1d =      {INIT_OBJSTATE, 10 * NORMAL_TPS_v1d,     BUTLER_1d, 0};  // Ask again if we meet a bit later
act9 abut4a_1d =      {INIT_OBJSTATE, 0,                       BUTLER_1d, 1};  // Butler doesn't want to give chop
act9 abut7a_1d =      {INIT_OBJSTATE, 0,                       BUTLER_1d, 1};  // No more chops to give
act9 ahin1_1d =       {INIT_OBJSTATE, 0,                       HERO,      1};
act9 ahout_1d =       {INIT_OBJSTATE, 0,                       HERO,      0};
act9 ast78_1d =       {INIT_OBJSTATE, 1 * NORMAL_TPS_v1d + 12, TRAP_1d,   0};  // Close door after
act9 aboatfar_1d =    {INIT_OBJSTATE, 107,                     BOAT_1d,   1};  // Say boat on other side
act9 aboatmov_1d =    {INIT_OBJSTATE, 0,                       BOAT_1d,   2};  // Say boat moving
act9 aboatnear_1d =   {INIT_OBJSTATE, 13,                      BOAT_1d,   0};  // Say boat on near side
act9 aompass_1d =     {INIT_OBJSTATE, 0,                       OLDMAN_1d, 1};  // Oldman allows passage
act9 ashedoil_1d =    {INIT_OBJSTATE, 0,                       SHED_1d,   1};
act9 ast01_1d =       {INIT_OBJSTATE, 0,                       DOOR1_1d,  1};  // Close door after hero!
act9 ast12_1d =       {INIT_OBJSTATE, 1 * NORMAL_TPS_v1d + 12, DOOR2_1d,  0};  // Close door after
act9 ast14_1d =       {INIT_OBJSTATE, 1 * NORMAL_TPS_v1d + 12, DOOR3_1d,  0};  // Close door after
act9 astatedoor4_1d = {INIT_OBJSTATE, 0,                       DOOR4_1d,  1};  // Change state to open

act10 abata1a_1d  = {INIT_PATH, 0,                  BAT2_1d,   CHASE,  DX * 2, DY * 2};
act10 abata1b_1d  = {INIT_PATH, 7,                  BAT2_1d,   WANDER, DX,     DY};
act10 abata2a_1d  = {INIT_PATH, 0,                  BAT3_1d,   CHASE,  DX * 2, DY * 2};
act10 abata2b_1d  = {INIT_PATH, 6,                  BAT3_1d,   WANDER, DX,     DY};
act10 abata3a_1d  = {INIT_PATH, 0,                  BAT4_1d,   CHASE,  DX * 2, DY * 2};
act10 abata3b_1d  = {INIT_PATH, 5,                  BAT4_1d,   WANDER, DX,     DY};
act10 abata4a_1d  = {INIT_PATH, 0,                  BAT5_1d,   CHASE,  DX * 2, DY * 2};
act10 abata4b_1d  = {INIT_PATH, 4,                  BAT5_1d,   WANDER, DX,     DY};
act10 abin3_1d    = {INIT_PATH, 0,                  HERO,      AUTO,   0,      0};
act10 about3_1d   = {INIT_PATH, 0,                  HERO,      USER,   0,      0};
act10 abut2_1d    = {INIT_PATH, 8 * NORMAL_TPS_v1d, BUTLER_1d, CHASE,  DX - 2, DY - 2};
act10 abut3_1d    = {INIT_PATH, 0,                  HERO,      AUTO,   0,      0};     // Stop HERO and prompt
act10 abut8_1d    = {INIT_PATH, 0,                  BUTLER_1d, WANDER, DX - 2, DY - 2};
act10 abut9_1d    = {INIT_PATH, 0,                  HERO,      USER,   0,      0};
act10 acyc1_1d    = {INIT_PATH, 0,                  HERO,      AUTO,   0,      0};
act10 acyc2_1d    = {INIT_PATH, 57,                 HERO,      USER,   0,      0};
act10 adef2_1d    = {INIT_PATH, 1 * NORMAL_TPS_v1d, BAT2_1d,   WANDER, DX,     DY};
act10 adef3_1d    = {INIT_PATH, 1 * NORMAL_TPS_v1d, BAT3_1d,   WANDER, DX,     DY};
act10 adef4_1d    = {INIT_PATH, 1 * NORMAL_TPS_v1d, BAT4_1d,   WANDER, DX,     DY};
act10 adef5_1d    = {INIT_PATH, 1 * NORMAL_TPS_v1d, BAT5_1d,   WANDER, DX,     DY};
act10 adog2_1d    = {INIT_PATH, 0,                  DOG_1d,    CHASE,  DX * 2, DY * 2};
act10 ahchase3_1d = {INIT_PATH, 5 * NORMAL_TPS_v1d, DOG_1d,    CHASE,  DX * 2, DY * 2};
act10 ajail1_1d   = {INIT_PATH, 0,                  HERO,      AUTO,   0,      0};     // Stop user control
act10 akchase3_1d = {INIT_PATH, 5 * NORMAL_TPS_v1d, DOG_1d,    CHASE,  DX * 2, DY * 2};
act10 alab14_1d   = {INIT_PATH, 40,                 IGOR_1d,   WANDER, DX,     0};
act10 amum2_1d    = {INIT_PATH, 3 * NORMAL_TPS_v1d, MUMMY_1d,  CHASE,  DX * 2, DY * 2};

act11 achkd0_1d =   {COND_R, 0,   GDOOR_1d,    0, kALok151_1d,      kALchkd1_1d};
act11 achkd1_1d =   {COND_R, 0,   GDOOR_1d,    1, kALdmsg1_1d,      kALchkd2_1d};
act11 achkd2_1d =   {COND_R, 0,   GDOOR_1d,    2, kALdmsg2_1d,      kALchkd3_1d};
act11 achkd3_1d =   {COND_R, 0,   GDOOR_1d,    3, kALdmsg3_1d,      kALok151_1d};
act11 achklab_1d =  {COND_R, 0,   GDOOR_1d,    0, kALprof_1d,       0};
act11 aichk0_1d =   {COND_R, 0,   GDOOR_1d,    0, kALigor0_1d,      kALichk1_1d};  // State 0 ?
act11 aichk1_1d =   {COND_R, 0,   GDOOR_1d,    1, kALigor1_1d,      kALichk2_1d};  // State 1 ?
act11 aichk2_1d =   {COND_R, 0,   GDOOR_1d,    2, kALigor2_1d,      kALichk3_1d};  // State 2 ?
act11 aichk3_1d =   {COND_R, 0,   GDOOR_1d,    3, kALigor3_1d,      kALinorm_1d};  // State 3 ?
act11 a115a_1d =    {COND_R, 0,   MASK_1d,     1, kALswapmask_1d,   0};  // Remove mask if worn
act11 abin_1d =     {COND_R, 0,   BUNG_1d,     0, kALnobung_1d,     kALembark_1d};
act11 abox_1d =     {COND_R, 0,   GDOOR_1d,    0, kALbox0_1d,       0};
act11 abut3a_1d =   {COND_R, 0,   BUTLER_1d,   0, kALbutp_1d,       0};
act11 abut5_1d =    {COND_R, 0,   MASK_1d,     0, kALbutchopped_1d, kALbutchop_1d};
act11 abut6_1d =    {COND_R, 0,   MASK_1d,     0, kALbutchopped_1d, kALbutroam_1d};
act11 achkmove_1d = {COND_R, 0,   BOAT_1d,     2, kALmoving_1d,     kALmoveboat_1d};
act11 achkout_1d =  {COND_R, 0,   OLDMAN_1d,   1, kALdeboat_1d,     kALnodeboat_1d};
act11 achkpass_1d = {COND_R, 0,   OLDMAN_1d,   1, kALomasked_1d,    kALomask_1d};
act11 achkrope_1d = {COND_R, 0,   ROPE_1d,     1, kALchkmove_1d,    kALnotcut_1d};
act11 agetout_1d =  {COND_R, 0,   BOAT_1d,     0, kALdeboat_1d,     kALchkout_1d};
act11 aherochk_1d = {COND_R, 107, HERO,        1, kALherofar_1d,    0};
act11 apush_1d =    {COND_R, 0,   BOAT_1d,     0, kALgofar_1d,      kALcomenear_1d};
act11 at78b_1d =    {COND_R, 0,   MASK_1d,     1, kALswapmask_1d,   0};  // Remove mask if worn
act11 atcup0_1d =   {COND_R, 0,   CUPBOARD_1d, 0, kALcuptxt0_1d,    kALlookcupb1_1d};
act11 atcup1_1d =   {COND_R, 0,   CUPBOARD_1d, 1, kALcuptxt1_1d,    kALlookcupb2_1d};
act11 atcup2_1d =   {COND_R, 0,   CUPBOARD_1d, 2, kALcuptxt2_1d,    kALcuptxt3_1d};
act11 atrap_1d =    {COND_R, 0,   BOLT_1d,     2, kALopenpass_1d,   kALopenfail_1d};

act12 achopfail_1d =   {TEXT, 0,                      kSTsthrown_1d};
act12 achopthrown_1d = {TEXT, 5 * NORMAL_TPS_v1d,     kSTsachopthrown_1d};
act12 admsg1_1d =      {TEXT, 0,                      kSTsadmsg1_1d};
act12 admsg2_1d =      {TEXT, 0,                      kSTsadmsg2_1d};
act12 admsg3_1d =      {TEXT, 0,                      kSTsadmsg3_1d};
act12 aeatchop_1d =    {TEXT, 0,                      kSTsaeatchop_1d};
act12 agobox_1d =      {TEXT, 0,                      kSTsagobox_1d};
act12 aigor0_1d =      {TEXT, 0,                      kSTsaigor0_1d};
act12 aigor13_1d =     {TEXT, 0,                      kSTsaigor13_1d};
act12 aigor22_1d =     {TEXT, 0,                      kSTsaigor22_1d};
act12 aigor32_1d =     {TEXT, 0,                      kSTsaigor32_1d};
act12 ainorm_1d =      {TEXT, 0,                      kSTsainorm_1d};
act12 a115e_1d =       {TEXT, 0,                      kSTsa115e_1d};
act12 abat5a_1d =      {TEXT, 0,                      kSTsabat5a_1d};
act12 abat5b_1d =      {TEXT, 0,                      kSTsabat5b_1d};
act12 abin0_1d =       {TEXT, 0,                      kSTokgen_1d};
act12 ablowt_1d =      {TEXT, 3 * NORMAL_TPS_v1d,     kSTsablowt_1d};
act12 abox2_1d =       {TEXT, 16,                     kSTsabox2_1d};
act12 abox3_1d =       {TEXT, 16,                     kSTsabox3_1d};
act12 abox5_1d =       {TEXT, 38,                     kSTsabox5_1d};
act12 abox6_1d =       {TEXT, 44,                     kSTsabox6_1d};
act12 abung1_1d =      {TEXT, 0,                      kSTsabung1_1d};
act12 abut11_1d =      {TEXT, NORMAL_TPS_v1d / 3,     kSTsabut11_1d};
act12 abut6a_1d =      {TEXT, 0,                      kSTsabut6a_1d};
act12 abut6b_1d =      {TEXT, 3,                      kSTsabut6b_1d};
act12 abut6c_1d =      {TEXT, 6,                      kSTsabut6c_1d};
act12 abut9a_1d =      {TEXT, NORMAL_TPS_v1d / 3,     kSTsabut9a_1d};
act12 abut9b_1d =      {TEXT, NORMAL_TPS_v1d / 3 + 1, kSTsabut9b_1d};
act12 acuptxt0_1d =    {TEXT, 0,                      kSTsseepkdw_1d};
act12 acuptxt1_1d =    {TEXT, 0,                      kSTsseedw_1d};
act12 acuptxt2_1d =    {TEXT, 0,                      kSTsseepk_1d};
act12 acuptxt3_1d =    {TEXT, 0,                      kSTsnosee_1d};
act12 adef6_1d =       {TEXT, 1 * NORMAL_TPS_v1d,     kSTsdefbat1_1d};
act12 adef7_1d =       {TEXT, 3 * NORMAL_TPS_v1d,     kSTsdefbat2_1d};
act12 adoggy4_1d =     {TEXT, 0,                      kSTsadoggy4_1d};
act12 adwwhy_1d =      {TEXT, 0,                      kSTsadwwhy_1d};
act12 agive1_1d =      {TEXT, 0,                      kSTsagive1_1d};
act12 agive2_1d =      {TEXT, 0,                      kSTsagive2_1d};
act12 ahelps1_1d =     {TEXT, 0,                      kSTsahelps1_1d};
act12 ahelps2_1d =     {TEXT, 0,                      kSTsahelps2_1d};
act12 ahout1_1d =      {TEXT, 0,                      kSTokgen_1d};
act12 ajails1_1d =     {TEXT, 0,                      kSTsajails1_1d};
act12 ajails2_1d =     {TEXT, 0,                      kSTsajails2_1d};
act12 ajails3_1d =     {TEXT, 0,                      kSTsajails3_1d};
act12 ajails4_1d =     {TEXT, 0,                      kSTsajails4_1d};
act12 alab12_1d =      {TEXT, 24,                     kSTsalab12_1d};
act12 alab13_1d =      {TEXT, 24,                     kSTsalab13_1d};
act12 amans1_1d =      {TEXT, 0,                      kSTsamans1_1d};
act12 amans3_1d =      {TEXT, 0,                      kSTsamans3_1d};
act12 amans4_1d =      {TEXT, 0,                      kSTsamans4_1d};
act12 amans5_1d =      {TEXT, 0,                      kSTsamans5_1d};
act12 amans6_1d =      {TEXT, 0,                      kSTsamans6_1d};
act12 amans7_1d =      {TEXT, 0,                      kSTsamans7_1d};
act12 amoving_1d =     {TEXT, 0,                      kSTsamoving_1d};
act12 amum3_1d =       {TEXT, 0,                      kSTsabat5a_1d};
act12 amum4_1d =       {TEXT, 0,                      kSTsamum4_1d};
act12 anodeboat_1d =   {TEXT, 0,                      kSTsanodeboat_1d};
act12 anogive_1d =     {TEXT, 0,                      kSTsanogive_1d};
act12 anohelp_1d =     {TEXT, 0,                      kSTsanohelp_1d};
act12 anoopen_1d =     {TEXT, 0,                      kSTsanoopen_1d};
act12 anotcut_1d =     {TEXT, 0,                      kSTsanotcut_1d};
act12 anought_1d =     {TEXT, 1 * NORMAL_TPS_v1d,     kSTsanought_1d};
act12 aopen4_1d =      {TEXT, 0,                      kSTsaopen4_1d};
act12 aopenfail_1d =   {TEXT, 0,                      kSTsaopenfail_1d};
act12 apbreak_1d =     {TEXT, 0,                      kSTspbreak_1d};
act12 arepno5_1d =     {TEXT, 0,                      kSTsarepno5_1d};
act12 arepnop_1d =     {TEXT, 0,                      kSTsarepnop_1d};
act12 arepyep_1d =     {TEXT, 0,                      kSTsarepyep_1d};
act12 arepyep2_1d =    {TEXT, 0,                      kSTsarepyep2_1d};
act12 at78a_1d =       {TEXT, 0,                      kSTsat78a_1d};
act12 bye1_1d =        {TEXT, 2 * NORMAL_TPS_v1d,     kSTsabye1_1d};

act13 aigor10_1d =   {SWAP_IMAGES, 30, HERO, WHERO_1d};
act13 aigor11_1d =   {SWAP_IMAGES, 30, HERO, SPACHERO_1d};
act13 aigor20_1d =   {SWAP_IMAGES, 30, HERO, SPACHERO_1d};
act13 aigor21_1d =   {SWAP_IMAGES, 30, HERO, FUZYHERO_1d};
act13 aigor30_1d =   {SWAP_IMAGES, 30, HERO, FUZYHERO_1d};
act13 aigor31_1d =   {SWAP_IMAGES, 30, HERO, HERO};
act13 abox4_1d =     {SWAP_IMAGES, 20, HERO, WHERO_1d};
act13 aswaphero_1d = {SWAP_IMAGES, 0,  HERO, WHERO_1d};
act13 aswapmask_1d = {SWAP_IMAGES, 0,  HERO, MONKEY_1d};

act14 athrowchop_1d = {COND_SCR, 0, HERO, 7,  kALchoppass_1d,  kALchopfail_1d};
act14 ablow1_1d =     {COND_SCR, 0, HERO, 1,  kALchasehall_1d, kALcond5_1d};
act14 achkscr_1d =    {COND_SCR, 0, HERO, 12, kALgive_1d,      kALnogive_1d};
act14 acond5_1d =     {COND_SCR, 0, HERO, 5,  kALchasekit_1d,  kALcond9_1d};
act14 acond9_1d =     {COND_SCR, 0, HERO, 9,  kALdefbats_1d,   kALnought_1d};

act15 adogchop_1d = {AUTOPILOT, 0, DOG_1d, CHOP_1d, DX + 2, DY * 2};

act16 abin1_1d =       {INIT_OBJ_SEQ, 0,                       BOAT_1d,  1}; // Hero gets in boat
act16 about1_1d =      {INIT_OBJ_SEQ, 0,                       BOAT_1d,  0}; // Hero gets out of boat
act16 aclosedoor2_1d = {INIT_OBJ_SEQ, 1 * NORMAL_TPS_v1d + 12, DOOR2_1d, 0};
act16 aclosedoor3_1d = {INIT_OBJ_SEQ, 1 * NORMAL_TPS_v1d + 12, DOOR3_1d, 0};
act16 acutrope_1d =    {INIT_OBJ_SEQ, 0,                       ROPE_1d,  1};
act16 adog5_1d =       {INIT_OBJ_SEQ, 0,                       DOG_1d,   0}; // Go right
act16 adogseq_1d =     {INIT_OBJ_SEQ, 0,                       DOG_1d,   1}; // Go left
act16 adogseq2_1d =    {INIT_OBJ_SEQ, 4 * NORMAL_TPS_v1d,      DOG_1d,   2}; // Sit up
act16 aguardgo2_1d =   {INIT_OBJ_SEQ, 0,                       GUARD_1d, 1};
act16 ajail3_1d =      {INIT_OBJ_SEQ, 0,                       HERO,     0}; // Hero dances for joy!
act16 ajail4_1d =      {INIT_OBJ_SEQ, 2,                       HERO,     1};
act16 alab10_1d =      {INIT_OBJ_SEQ, 14,                      PROF_1d,  1}; // Look to left
act16 alab11_1d =      {INIT_OBJ_SEQ, 18,                      IGOR_1d,  1}; // Look to left
act16 alab4_1d =       {INIT_OBJ_SEQ, 0,                       PROF_1d,  0}; // Walk to right
act16 at78c_1d =       {INIT_OBJ_SEQ, 1 * NORMAL_TPS_v1d + 12, TRAP_1d,  0}; // Close trap
act16 aturnguard_1d =  {INIT_OBJ_SEQ, 2 * NORMAL_TPS_v1d,      GUARD_1d, 0};

act17 acupbdw_1d = {SET_STATE_BITS, 0, CUPBOARD_1d, 2};
act17 acupbpk_1d = {SET_STATE_BITS, 0, CUPBOARD_1d, 1};

act20 adef1_1d = {DEL_EVENTS, 1 * NORMAL_TPS_v1d, ASCHEDULE};     // Stop bats looping

act21 abut6g_1d =  {GAMEOVER, 7};
act21 adoggy6_1d = {GAMEOVER, 0};

act23 bye2_1d = {EXIT, 2 * NORMAL_TPS_v1d};

act24 abonus0_1d = {BONUS, 0, 0};
act24 abonus1_1d = {BONUS, 0, 1};
act24 abonus2_1d = {BONUS, 0, 2};
act24 abonus3_1d = {BONUS, 0, 3};
act24 abonus4_1d = {BONUS, 0, 4};
act24 abonus5_1d = {BONUS, 0, 5};
act24 abonus6_1d = {BONUS, 0, 6};
act24 abonus7_1d = {BONUS, 0, 7};
act24 abonus8_1d = {BONUS, 0, 8};
act24 abonus9_1d = {BONUS, 0, 9};

act25 aichkbox_1d = {COND_BOX, 0, HERO, 100, 153, 144, 163, kALichk0_1d, kALgobox_1d};

act27 abut7b_1d = {ADD_SCORE, 0, CHOP_1d};          // Manually add score

act28 a115f_1d = {SUB_SCORE, 0, MASK_1d};

act29 achkmask_1d =  {COND_CARRY, 0, MASK_1d, kALputmask_1d, 0};
act29 achkmask2_1d = {COND_CARRY, 0, MASK_1d, kALridmask_1d, 0};

//Strangerke - act26 are stored in new act49, as songs were not handled the same way in DOS version (in harcoded strings)
act49 ahchase4_1d = {OLD_SONG, 4 * NORMAL_TPS_v1d, kDTsong3_1d};
act49 asong1_1d =   {OLD_SONG, 0,                  kDTsong1_1d};
act49 asong2_1d =   {OLD_SONG, 1 * NORMAL_TPS_v1d, kDTsong2_1d};
act49 asong3_1d =   {OLD_SONG, 0,                  kDTsong3_1d};

actListPtr ALbat_1d[]         = {&abatxy_1d, &abatvxy1_1d, &abatvxy2_1d, &abatvxy3_1d, &abatvxy4_1d, &abatvxy5_1d, &arepbat_1d, 0};
actListPtr ALbatattack_1d[]   = {&abata1a_1d, &abata1b_1d, &abata1c_1d, &abata2a_1d, &abata2b_1d, &abata2c_1d, &abata3a_1d, &abata3b_1d, &abata3c_1d, &abata4a_1d, &abata4b_1d, &abata4c_1d, &arepbata_1d, 0};
actListPtr ALbatrep_1d[]      = {&abata1a_1d, &abata1b_1d, &abata2a_1d, &abata2b_1d, &abata3a_1d, &abata3b_1d, &abata4a_1d, &abata4b_1d, &arepbata_1d, 0};
actListPtr ALbats_1d[]        = {&adoggy1_1d, &adoggy2_1d, &adoggy3_1d, &abat5a_1d, &abat5b_1d, &adoggy5_1d, &adoggy6_1d, 0};
actListPtr ALblinkeyes1_1d[]  = {&ablink1a_1d, &ablink1b_1d, &ablink1c_1d, &ablink1d_1d, &ablink2a_1d, &ablink2b_1d, &ablink2c_1d, &ablink2d_1d, &arepeye_1d, 0};
actListPtr ALblinkeyes2_1d[]  = {&ablink3a_1d, &ablink3b_1d, &ablink3c_1d, &ablink3d_1d, &ablink4a_1d, &ablink4b_1d, &ablink4c_1d, &ablink4d_1d, &arepeye2_1d, 0};
actListPtr ALblowdw_1d[]      = {&ablow1_1d, 0};
actListPtr ALbox0_1d[]        = {&acycle_1d, &abox2_1d, &abox3_1d, &abox4_1d, &abox4a_1d, &abox5_1d, &abox6_1d, &abox9_1d, &abox10_1d, &abox11_1d, &abox12_1d, 0};
actListPtr ALbox_1d[]         = {&abox_1d, 0};
actListPtr ALbut_1d[]         = {&abut1_1d, &abutvxy1_1d, &abut2_1d, 0};
actListPtr ALbutchop_1d[]     = {&abut7_1d, &abut7a_1d, &abut7b_1d, &abut8_1d, &abut9_1d, &abut9a_1d, &abut9b_1d, 0};
actListPtr ALbutchopped_1d[]  = {&abut6a_1d, &abut6b_1d, &abut6c_1d, &abut6d_1d, &abut6e_1d, &abut6f_1d, &abut6g_1d, 0};
actListPtr ALbutler_1d[]      = {&abut3a_1d, 0};
actListPtr ALbutno_1d[]       = {&abut6_1d, 0};
actListPtr ALbutp_1d[]        = {/*&abut3_1d, */&abut4_1d, &abut4a_1d, 0};
actListPtr ALbutroam_1d[]     = {&abut8_1d, &abut9_1d, &abut10_1d, &abut11_1d, 0};
actListPtr ALbutyes_1d[]      = {&abut5_1d, 0};
actListPtr ALchasehall_1d[]   = {&ablowt_1d, &ahchase1_1d, &ahchase2_1d, &ahchase3_1d, &ahchase4_1d, 0};
actListPtr ALchasekit_1d[]    = {&ablowt_1d, &akchase1_1d, &akchase2_1d, &akchase3_1d, &ahchase4_1d, 0};
actListPtr ALchkd1_1d[]       = {&achkd1_1d, 0};          // State 1 ?
actListPtr ALchkd2_1d[]       = {&achkd2_1d, 0};          // State 2 ?
actListPtr ALchkd3_1d[]       = {&achkd3_1d, 0};          // State 3 ?
actListPtr ALchkmove_1d[]     = {&achkmove_1d, 0};
actListPtr ALchkout_1d[]      = {&achkout_1d, 0};
actListPtr ALchopfail_1d[]    = {&adropchop_1d, &aridchop_1d, &achopfail_1d, 0};
actListPtr ALchoppass_1d[]    = {&ashowchop_1d, &achopthrown_1d, &adropchop_1d, &achopxy_1d, &adogchop_1d, &adogseq_1d, &adogcyc_1d, &adogseq2_1d, 0};
actListPtr ALclosedoor1_1d[]  = {&aclosedoor1_1d, 0};
actListPtr ALclosedoor4_1d[]  = {&aclosedoor4_1d, 0};
actListPtr ALclosetrap_1d[]   = {&aclosetrap_1d, 0};
actListPtr ALclosewdoors_1d[] = {&aclosewdoorl_1d, &aclosewdoorr_1d, 0};
actListPtr ALcomenear_1d[]    = {&aboatmov_1d, &aboatvxy10_1d, &aboatvxy11_1d, &aboatvxy12_1d, &aheronear_1d, &aboatnear_1d, 0};
actListPtr ALcond5_1d[]       = {&acond5_1d, 0};
actListPtr ALcond9_1d[]       = {&acond9_1d, 0};
actListPtr ALcupbdw_1d[]      = {&acupbdw_1d, &adwwhy_1d, 0};
actListPtr ALcupbpk_1d[]      = {&acupbpk_1d, 0};
actListPtr ALcuptxt0_1d[]     = {&acuptxt0_1d, 0};
actListPtr ALcuptxt1_1d[]     = {&acuptxt1_1d, 0};
actListPtr ALcuptxt2_1d[]     = {&acuptxt2_1d, 0};
actListPtr ALcuptxt3_1d[]     = {&acuptxt3_1d, 0};
actListPtr ALcutrope_1d[]     = {&acutrope_1d, 0};
actListPtr ALcycle_1d[]       = {&abox0_1d, &abox1_1d, &acyc1_1d, &acyc2_1d, &aweird_1d, &abox7_1d, &abox8_1d, 0};
actListPtr ALdeboat_1d[]      = {&ahout_1d, &ahout1_1d, &about1_1d, &about2_1d, &about3_1d, 0};
actListPtr ALdefbats_1d[]     = {&abonus6_1d, &adef1_1d, &adef2_1d, &adef3_1d, &adef4_1d, &adef5_1d, &adef6_1d, &adef7_1d, 0};
actListPtr ALdmsg1_1d[]       = {&admsg1_1d, 0};
actListPtr ALdmsg2_1d[]       = {&admsg2_1d, 0};
actListPtr ALdmsg3_1d[]       = {&admsg3_1d, 0};
actListPtr ALdog_1d[]         = {&asong3_1d, &adog1_1d, &adog2_1d, &adog3_1d, &adog4_1d, &adog5_1d, 0};
actListPtr ALdoggy_1d[]       = {&adoggy1_1d, &adoggy2_1d, &adoggy3_1d, &adoggy4_1d, &adoggy5_1d, &adoggy6_1d, 0};
actListPtr ALdropmask_1d[]    = {&adropmask_1d, 0};
actListPtr ALeatchop_1d[]     = {&adropchop_1d, &aeatchop_1d, &aridchop_1d, 0};
actListPtr ALembark_1d[]      = {&abin0_1d, &ahin1_1d, &ahin2_1d, &abin1_1d, &abin2_1d, &abin3_1d, 0};
actListPtr ALend_1d[]         = {&ajails1_1d, &ajails2_1d, &ajails3_1d, &ajails4_1d, 0};
actListPtr ALgetinboat_1d[]   = {&abin_1d, 0};
actListPtr ALgetoutboat_1d[]  = {&agetout_1d, 0};
actListPtr ALgive_1d[]        = {&abonus8_1d, &agive1_1d, &agive2_1d, &aguardgo1_1d, &aguardgo2_1d, &aguardgo3_1d, &aturnguard_1d, 0};
actListPtr ALgobox_1d[]       = {&agobox_1d, 0};
actListPtr ALgofar_1d[]       = {&aboatmov_1d, &aboatvxy1_1d, &aboatvxy2_1d, &aboatvxy3_1d, &aboatvxy4_1d, &aboatvxy5_1d, &aboatvxy6_1d, &aboatvxy7_1d, &aboatvxy8_1d, &aboatvxy9_1d, &aherochk_1d, &aboatfar_1d, 0};
actListPtr ALgold_1d[]        = {&achkscr_1d, 0};
actListPtr ALhelp_1d[]        = {&ahelp1_1d, &ahelp2_1d, 0};
actListPtr ALhelpn_1d[]       = {&anohelp_1d, 0};
actListPtr ALhelpy2_1d[]      = {&ahelps2_1d, 0};
actListPtr ALhelpy_1d[]       = {&ahelps1_1d, 0};
actListPtr ALherofar_1d[]     = {&aherofar_1d, 0};
actListPtr ALichk0_1d[]       = {&aichk0_1d, 0};
actListPtr ALichk1_1d[]       = {&aichk1_1d, 0};
actListPtr ALichk2_1d[]       = {&aichk2_1d, 0};
actListPtr ALichk3_1d[]       = {&aichk3_1d, 0};
actListPtr ALightning_1d[]    = {&abk1_1d, &abknorm1_1d, &abk2_1d, &abknorm2_1d, &areplight_1d, 0};
actListPtr ALigor0_1d[]       = {&aigor0_1d, 0};
actListPtr ALigor1_1d[]       = {&acycle_1d, &aigor10_1d, &aigor11_1d, &aigor12_1d, &aigor13_1d, &aigor14_1d, 0};
actListPtr ALigor2_1d[]       = {&acycle_1d, &aigor20_1d, &aigor21_1d, &aigor22_1d, &aigor23_1d, 0};
actListPtr ALigor3_1d[]       = {&acycle_1d, &aigor30_1d, &aigor31_1d, &aigor32_1d, &aigor33_1d, 0};
actListPtr ALigor_1d[]        = {&aichkbox_1d, 0};        // Make sure hero is in box!
actListPtr ALinorm_1d[]       = {&ainorm_1d, 0};
actListPtr ALjail_1d[]        = {&ajail1_1d, &ajail2_1d, &ajailrep_1d, &aend_1d, &atheend1_1d, &atheend2_1d, 0};
actListPtr ALjailrep_1d[]     = {&ajail3_1d, &ajail4_1d, &ajailrep_1d, 0};
actListPtr ALlab_1d[]         = {&achklab_1d, 0};
actListPtr ALlookcupb1_1d[]   = {&atcup1_1d, 0};
actListPtr ALlookcupb2_1d[]   = {&atcup2_1d, 0};
actListPtr ALlookcupb_1d[]    = {&abonus2_1d, &atcup0_1d, 0};
actListPtr ALmoveboat_1d[]    = {&apush_1d, 0};
actListPtr ALmovecarp_1d[]    = {&amovecarp1_1d, &amovecarp2_1d, &abonus9_1d, 0};
actListPtr ALmoving_1d[]      = {&amoving_1d, 0};
actListPtr ALmum_1d[]         = {&asong2_1d, &amdoor1_1d, &amum1_1d, &amum2_1d, 0};
actListPtr ALmummy_1d[]       = {&amum3_1d, &amum4_1d, &adoggy5_1d, &adoggy6_1d, 0};
actListPtr ALnodeboat_1d[]    = {&anodeboat_1d, 0};
actListPtr ALnogive_1d[]      = {&anogive_1d, 0};
actListPtr ALnotcut_1d[]      = {&anotcut_1d, 0};
actListPtr ALnought_1d[]      = {&anought_1d, 0};
actListPtr ALok151_1d[]       = {&aheroxy151_1d, &ascr151_1d, 0};
actListPtr ALoldman_1d[]      = {&achkpass_1d, 0};
actListPtr ALomask_1d[]       = {&amans3_1d, &amans4_1d, &amans5_1d, &amans6_1d, &amanq1_1d, 0};
actListPtr ALomasked_1d[]     = {&amans7_1d, 0};
actListPtr ALopendoor1_1d[]   = {&aopendoor1_1d, 0};
actListPtr ALopendoor2_1d[]   = {&astophero_1d, &aopendoor2_1d, &ast12_1d, &aheroxy12_1d, &aclosedoor2_1d, &ascr12_1d, 0};
actListPtr ALopendoor3_1d[]   = {&astophero_1d, &aopendoor3_1d, &ast14_1d, &aheroxy14_1d, &aclosedoor3_1d, &ascr14_1d, 0};
actListPtr ALopendoor4_1d[]   = {&aopenp_1d, 0};
actListPtr ALopenfail_1d[]    = {&aopenfail_1d, 0};
actListPtr ALopenno_1d[]      = {&anoopen_1d, 0};
actListPtr ALopenpass_1d[]    = {&abonus4_1d, &aopentrap_1d, &aheroxy78_1d, &ast78_1d, &at78c_1d, &aopen78_1d, &achkmask2_1d, 0};
actListPtr ALopentrap_1d[]    = {&atrap_1d, 0};
actListPtr ALopenwdoors_1d[]  = {&aopenwdoorl_1d, &aopenwdoorr_1d, 0};
actListPtr ALopenyes_1d[]     = {&abonus3_1d, &aopen4_1d, &aopendoor4_1d, &astatedoor4_1d, 0};
actListPtr ALpkin_1d[]        = {&abonus0_1d, &apbreak_1d, &ashowkey_1d, &aridpkin_1d, &adroppkin_1d, &amovekey_1d, 0};
actListPtr ALplugbung_1d[]    = {&aridbung_1d, &adropbung_1d, 0};
actListPtr ALprof_1d[]        = {&alab1_1d, &alab2_1d, &alab3_1d, &alab4_1d, &alab5_1d, &alab6_1d, &alab7_1d, &alab8_1d, &alab9_1d, &alab10_1d, &alab11_1d, &alab12_1d, &alab13_1d, &alab14_1d, 0};
actListPtr ALpushboat_1d[]    = {&achkrope_1d, 0};
actListPtr ALputmask_1d[]     = {&a115a_1d, &a115b_1d, &a115c_1d, &a115d_1d, &a115e_1d, &a115f_1d, &a115g_1d, 0};
actListPtr ALreparm_1d[]      = {&aarm_1d, &areparm_1d, 0};
actListPtr ALreplips_1d[]     = {&alips_1d, &areplips_1d, 0};
actListPtr ALrepno1_1d[]      = {&arepnop_1d, &amans1_1d, &aboatvxy9_1d, 0};
actListPtr ALrepno3_1d[]      = {&arepno5_1d, &aboatvxy9_1d, 0};
actListPtr ALrepredeye_1d[]   = {&ablink5a_1d, &ablink5b_1d, &ablink5c_1d, &ablink5d_1d, &arepredeye_1d, 0};
actListPtr ALrepyes1_1d[]     = {&arepyep_1d, &amanq2_1d, 0};
actListPtr ALrepyes2_1d[]     = {&arepyep_1d, &amanq3_1d, 0};
actListPtr ALrepyes3_1d[]     = {&arepyep_1d, &amanq4_1d, 0};
actListPtr ALrepyes4_1d[]     = {&amanq5_1d, 0};
actListPtr ALrepyes5_1d[]     = {&amanq6_1d, 0};
actListPtr ALrepyes6_1d[]     = {&amanq7_1d, 0};
actListPtr ALrepyes7_1d[]     = {&abonus7_1d, &arepyep2_1d, &aompass_1d, 0};
actListPtr ALridmask_1d[]     = {&at78a_1d, &at78b_1d, &adropmask_1d, 0};
actListPtr ALridprof_1d[]     = {&aridprof_1d, 0};
actListPtr ALscr1011_1d[]     = {&aswaphero_1d, &aheroxy1011_1d, &ascr1011_1d, 0};
actListPtr ALscr109_1d[]      = {&aheroxy109_1d, &ascr109_1d, 0};
actListPtr ALscr10_1d[]       = {&aheroxy10_1d, &ast01_1d, &ascr10_1d, 0};
actListPtr ALscr1110_1d[]     = {&aswaphero_1d, &aheroxy1110_1d, &ascr1110_1d, 0};
actListPtr ALscr1112_1d[]     = {&aswaphero_1d, &aheroxy1112_1d, &ascr1112_1d, 0};
actListPtr ALscr115_1d[]      = {&achkmask_1d, &aheroxy115_1d, &ascr115_1d, 0};
actListPtr ALscr1211_1d[]     = {&aswaphero_1d, &aheroxy1211_1d, &ascr1211_1d, 0};
actListPtr ALscr1213_1d[]     = {&aheroxy1213_1d, &ascr1213_1d, 0};
actListPtr ALscr13_1d[]       = {&aheroxy13_1d, &ascr13_1d, 0};
actListPtr ALscr151_1d[]      = {&achkd0_1d, 0};          // State 0 ?
actListPtr ALscr15_1d[]       = {&aheroxy15_1d, &ascr15_1d, 0};
actListPtr ALscr1_1d[]        = {&abonus1_1d, &aheroxy01_1d, &ascr01_1d, 0};
actListPtr ALscr21_1d[]       = {&aheroxy21_1d, &ascr21_1d, 0};
actListPtr ALscr31_1d[]       = {&aheroxy31_1d, &ascr31_1d, 0};
actListPtr ALscr35_1d[]       = {&aheroxy35_1d, &ascr35_1d, 0};
actListPtr ALscr41_1d[]       = {&aheroxy41_1d, &ascr41_1d, 0};
actListPtr ALscr51_1d[]       = {&aheroxy51_1d, &ascr51_1d, 0};
actListPtr ALscr53_1d[]       = {&aheroxy53_1d, &ascr53_1d, 0};
actListPtr ALscr56_1d[]       = {&aheroxy56_1d, &ascr56_1d, 0};
actListPtr ALscr57_1d[]       = {&aheroxy57_1d, &ascr57_1d, 0};
actListPtr ALscr65_1d[]       = {&aheroxy65_1d, &ascr65_1d, 0};
actListPtr ALscr75_1d[]       = {&aheroxy75_1d, &ascr75_1d, 0};
actListPtr ALscr87_1d[]       = {&aheroxy87_1d, &ascr87_1d, 0};
actListPtr ALscr89_1d[]       = {&abonus5_1d, &aheroxy89_1d, &ascr89_1d, 0};
actListPtr ALscr910_1d[]      = {&aheroxy910_1d, &ascr910_1d, 0};
actListPtr ALscr98_1d[]       = {&aheroxy98_1d, &ascr98_1d, 0};
actListPtr ALshedoil_1d[]     = {&ashedoil_1d, 0};
actListPtr ALsong1_1d[]       = {&asong1_1d, 0};
actListPtr ALswapmask_1d[]    = {&aswapmask_1d, 0};
actListPtr ALthrowchop_1d[]   = {&athrowchop_1d, 0};
actListPtr ALweird_1d[]       = {&abg1_1d, &abg2_1d, &abg3_1d, &abg4_1d, &abg5_1d, &abg6_1d, &abg7_1d, &abg8_1d, 0};
actListPtr ALnobung_1d[]      = {&abung1_1d, 0};
actListPtr ALgoodbye_1d[]     = {&bye1_1d, &bye2_1d, 0};

actList actListArr_1d[] = {
	ALDummy,          ALbat_1d,        ALbatattack_1d,  ALbatrep_1d,     ALbats_1d,
	ALblinkeyes1_1d,  ALblinkeyes2_1d, ALblowdw_1d,     ALbox0_1d,       ALbox_1d,
	ALbut_1d,         ALbutchop_1d,    ALbutchopped_1d, ALbutler_1d,     ALbutno_1d,
	ALbutp_1d,        ALbutroam_1d,    ALbutyes_1d,     ALchasehall_1d,  ALchasekit_1d,
	ALchkd1_1d,       ALchkd2_1d,      ALchkd3_1d,      ALchkmove_1d,    ALchkout_1d,
	ALchopfail_1d,    ALchoppass_1d,   ALclosedoor1_1d, ALclosedoor4_1d, ALclosetrap_1d,
	ALclosewdoors_1d, ALcomenear_1d,   ALcond5_1d,      ALcond9_1d,      ALcupbdw_1d,
	ALcupbpk_1d,      ALcuptxt0_1d,    ALcuptxt1_1d,    ALcuptxt2_1d,    ALcuptxt3_1d,
	ALcutrope_1d,     ALcycle_1d,      ALdeboat_1d,     ALdefbats_1d,    ALdmsg1_1d,
	ALdmsg2_1d,       ALdmsg3_1d,      ALdog_1d,        ALdoggy_1d,      ALdropmask_1d,
	ALeatchop_1d,     ALembark_1d,     ALend_1d,        ALgetinboat_1d,  ALgetoutboat_1d,
	ALgive_1d,        ALgobox_1d,      ALgofar_1d,      ALgold_1d,       ALhelp_1d,
	ALhelpn_1d,       ALhelpy2_1d,     ALhelpy_1d,      ALherofar_1d,    ALichk0_1d,
	ALichk1_1d,       ALichk2_1d,      ALichk3_1d,      ALightning_1d,   ALigor0_1d,
	ALigor1_1d,       ALigor2_1d,      ALigor3_1d,      ALigor_1d,       ALinorm_1d,
	ALjail_1d,        ALjailrep_1d,    ALlab_1d,        ALlookcupb1_1d,  ALlookcupb2_1d,
	ALlookcupb_1d,    ALmoveboat_1d,   ALmovecarp_1d,   ALmoving_1d,     ALmum_1d,
	ALmummy_1d,       ALnodeboat_1d,   ALnogive_1d,     ALnotcut_1d,     ALnought_1d,
	ALok151_1d,       ALoldman_1d,     ALomask_1d,      ALomasked_1d,    ALopendoor1_1d,
	ALopendoor2_1d,   ALopendoor3_1d,  ALopendoor4_1d,  ALopenfail_1d,   ALopenno_1d,
	ALopenpass_1d,    ALopentrap_1d,   ALopenwdoors_1d, ALopenyes_1d,    ALpkin_1d,
	ALplugbung_1d,    ALprof_1d,       ALpushboat_1d,   ALputmask_1d,    ALreparm_1d,
	ALreplips_1d,     ALrepno1_1d,     ALrepno3_1d,     ALrepredeye_1d,  ALrepyes1_1d,
	ALrepyes2_1d,     ALrepyes3_1d,    ALrepyes4_1d,    ALrepyes5_1d,    ALrepyes6_1d,
	ALrepyes7_1d,     ALridmask_1d,    ALridprof_1d,    ALscr1011_1d,    ALscr109_1d,
	ALscr10_1d,       ALscr1110_1d,    ALscr1112_1d,    ALscr115_1d,     ALscr1211_1d,
	ALscr1213_1d,     ALscr13_1d,      ALscr151_1d,     ALscr15_1d,      ALscr1_1d,
	ALscr21_1d,       ALscr31_1d,      ALscr35_1d,      ALscr41_1d,      ALscr51_1d,
	ALscr53_1d,       ALscr56_1d,      ALscr57_1d,      ALscr65_1d,      ALscr75_1d,
	ALscr87_1d,       ALscr89_1d,      ALscr910_1d,     ALscr98_1d,      ALshedoil_1d,
	ALsong1_1d,       ALswapmask_1d,   ALthrowchop_1d,  ALweird_1d,      ALnobung_1d,
	ALgoodbye_1d
};

// Hugo 2 DOS
int dialrsp_2d[] = {kSTSdial2_2d, -1};
int hestrsp_2d[] = {kSTYes_2d,    -1};
int whorsp_2d[]  = {kSTNobody_2d, kSTNo_one1_2d, kSTNo_one2_2d, kSTSharry_2d, -1};

act0 aclimax_2d   = {ASCHEDULE, 20,                   kALclimax_2d};
act0 aclue09_2d   = {ASCHEDULE, 300 * NORMAL_TPS_v2d, kALchkc09_2d};
act0 ahdrink10_2d = {ASCHEDULE, 52,                   kALhfaint_2d};
act0 aschedbut_2d = {ASCHEDULE, 30,                   kALschedbut_2d};
act0 arepbuga_2d  = {ASCHEDULE, 3 * NORMAL_TPS_v2d,   kALbugrep1_2d};
act0 arepbugf_2d  = {ASCHEDULE, 2 * NORMAL_TPS_v2d,   kALbugrep2_2d};
act0 arepblah_2d  = {ASCHEDULE, 12 * NORMAL_TPS_v2d,  kALblah_2d};
act0 arepmsg1_2d  = {ASCHEDULE, 120 * NORMAL_TPS_v2d, kALrepmsg1_2d};

act1 aback1_2d       = {START_OBJ, 0,             CAT_2d,       0, ALMOST_INVISIBLE};
act1 aback2_2d       = {START_OBJ, 2,             CAT_2d,       0, NOT_CYCLING};
act1 aball3_2d       = {START_OBJ, 0,             BALLOON_2d,   0, ALMOST_INVISIBLE};
act1 abel1_2d        = {START_OBJ, 0,             BELL_2d,      0, NOT_CYCLING};
act1 acook7_2d       = {START_OBJ, 60,            COOKB_2d,     0, ALMOST_INVISIBLE};
act1 acook8_2d       = {START_OBJ, 60,            COOK_2d,      0, NOT_CYCLING};
act1 acop1_2d        = {START_OBJ, 0,             COP_2d,       0, CYCLE_FORWARD};
act1 acop3_2d        = {START_OBJ, 12,            COP_2d,       0, ALMOST_INVISIBLE};
act1 adalek5_2d      = {START_OBJ, 4,             DALEK_2d,     0, CYCLE_FORWARD};
act1 adead1_2d       = {START_OBJ, 0,             HERO,         0, ALMOST_INVISIBLE};
act1 adead3_2d       = {START_OBJ, 0,             PENNYLIE_2d,  0, NOT_CYCLING};
act1 adone12_2d      = {START_OBJ, 10,            HERO,         0, CYCLE_FORWARD};
act1 adone5_2d       = {START_OBJ, 0,             HORACE_2d,    0, CYCLE_FORWARD};
act1 adyn2_2d        = {START_OBJ, 0,             DYNAMITE_2d,  0, NOT_CYCLING};
act1 afall2_2d       = {START_OBJ, 0,             PENFALL_2d,   0, CYCLE_FORWARD};
act1 afuze2_2d       = {START_OBJ, 0,             DYNAMITE_2d,  0, CYCLE_FORWARD};
act1 agenie2_2d      = {START_OBJ, 0,             GENIE_2d,     0, CYCLE_FORWARD};
act1 agiveb4_2d      = {START_OBJ, 2,             CAT_2d,       0, CYCLE_FORWARD};
act1 agiveb7_2d      = {START_OBJ, 0,             CAT_2d,       0, ALMOST_INVISIBLE};
act1 aglightoff1_2d  = {START_OBJ, 0,             GATELIGHT_2d, 0, ALMOST_INVISIBLE};
act1 aglighton1_2d   = {START_OBJ, 0,             GATELIGHT_2d, 0, NOT_CYCLING};
act1 ahdrink3_2d     = {START_OBJ, 0,             HESTER_2d,    0, CYCLE_FORWARD};
act1 ahdrink6_2d     = {START_OBJ, 50,            HESTER_2d,    0, NOT_CYCLING};
act1 ahest11_2d      = {START_OBJ, 47,            HESTER_2d,    0, NOT_CYCLING};
act1 ahest2_2d       = {START_OBJ, 0,             HESTER_2d,    0, CYCLE_FORWARD};
act1 ahfaint1_2d     = {START_OBJ, 4,             HERO,         0, ALMOST_INVISIBLE};
act1 ahfaint3_2d     = {START_OBJ, 4,             PENNYLIE_2d,  0, NOT_CYCLING};
act1 ahfaint4_2d     = {START_OBJ, 8,             PENNYLIE_2d,  0, ALMOST_INVISIBLE};
act1 ahfaint5_2d     = {START_OBJ, 8,             HERO,         0, NOT_CYCLING};
act1 akeyhole5_2d    = {START_OBJ, 0,             HERO,         0, ALMOST_INVISIBLE};
act1 amaidb2_2d      = {START_OBJ, 0,             MAID_2d,      0, CYCLE_FORWARD};
act1 amaidb5_2d      = {START_OBJ, 10,            MAID_2d,      0, ALMOST_INVISIBLE};
act1 amaidbk3_2d     = {START_OBJ, 0,             MAID_2d,      0, CYCLE_FORWARD};
act1 amaidbk7_2d     = {START_OBJ, 10,            MAID_2d,      0, NOT_CYCLING};
act1 amat2_2d        = {START_OBJ, 0,             MATCHES_2d,   0, NOT_CYCLING};
act1 amurd3_2d       = {START_OBJ, 30,            HERO,         0, NOT_CYCLING};
act1 aom15_2d        = {START_OBJ, 59,            OLDMAN_2d,    0, ALMOST_INVISIBLE};
act1 aom18_2d        = {START_OBJ, 60,            HERO,         0, NOT_CYCLING};
act1 aom19_2d        = {START_OBJ, 60,            OLDMAN_2d,    0, NOT_CYCLING};
act1 aomridlip_2d    = {START_OBJ, 49,            LIPS_2d,      0, ALMOST_INVISIBLE};
act1 aopendoor1_2d   = {START_OBJ, 0,             DOOR1_2d,     1, CYCLE_FORWARD};
act1 apaper2_2d      = {START_OBJ, 0,             PAPER_2d,     0, NOT_CYCLING};
act1 aridban_2d      = {START_OBJ, 0,             BANANA_2d,    0, ALMOST_INVISIBLE};
act1 aridcooklips_2d = {START_OBJ, 22,            LIPS_2d,      0, ALMOST_INVISIBLE};
act1 aridgard2_2d    = {START_OBJ, 0,             GARDENER_2d,  0, CYCLE_FORWARD};
act1 aridgard6_2d    = {START_OBJ, 20,            GARDENER_2d,  0, ALMOST_INVISIBLE};
act1 aridgarl_2d     = {START_OBJ, 0,             GARLIC_2d,    0, ALMOST_INVISIBLE};
act1 aslightoff1_2d  = {START_OBJ, 0,             SHEDLIGHT_2d, 0, ALMOST_INVISIBLE};
act1 aslighton1_2d   = {START_OBJ, 0,             SHEDLIGHT_2d, 0, NOT_CYCLING};
act1 astick5_2d      = {START_OBJ, 0,             DOG_2d,       0, CYCLE_FORWARD};
act1 astick7_2d      = {START_OBJ, 16,            DOG_2d,       0, ALMOST_INVISIBLE};
act1 azapperoff1_2d  = {START_OBJ, 0,             ZAPPER_2d,    0, ALMOST_INVISIBLE};
act1 azapperon1_2d   = {START_OBJ, 0,             ZAPPER_2d,    0, NOT_CYCLING};
act1 afaint2_2d      = {START_OBJ, 10,            HERO,         0, ALMOST_INVISIBLE};
act1 afaint4_2d      = {START_OBJ, 10,            PENNYLIE_2d,  0, NOT_CYCLING};
act1 afaint6_2d      = {START_OBJ, 30,            PENNYLIE_2d,  0, ALMOST_INVISIBLE};
act1 afaint7_2d      = {START_OBJ, 30,            HERO,         0, NOT_CYCLING};
act1 agone1_2d       = {START_OBJ, 23,            HERO,         0, ALMOST_INVISIBLE};
act1 agone14_2d      = {START_OBJ, 129,           HERO,         0, CYCLE_FORWARD};
act1 agone2_2d       = {START_OBJ, 120,           PENNYLIE_2d,  0, ALMOST_INVISIBLE};
act1 agone4_2d       = {START_OBJ, 120,           HERO,         0, NOT_CYCLING};
act1 aharry1_2d      = {START_OBJ, 0,             HARRY_2d,     0, NOT_CYCLING};
act1 aharry6_2d      = {START_OBJ, 4,             HARRY_2d,     0, CYCLE_FORWARD};
act1 amaidc10_2d     = {START_OBJ, 26,            MAID_2d,      0, ALMOST_INVISIBLE};
act1 amaidc14_2d     = {START_OBJ, 30,            MAID_2d,      0, NOT_CYCLING};
act1 amaidc5_2d      = {START_OBJ, 8,             MAID_2d,      0, CYCLE_FORWARD};
act1 aopendoor2_2d   = {START_OBJ, 0,             DOOR2_2d,     1, CYCLE_FORWARD};
act1 aopendoor3_2d   = {START_OBJ, 0,             DOOR3_2d,     1, CYCLE_FORWARD};
act1 apenblie1_2d    = {START_OBJ, 30,            PENNY_2d,     0, ALMOST_INVISIBLE};
act1 apenblie2_2d    = {START_OBJ, 30,            PENNYLIE_2d,  0, NOT_CYCLING};
act1 apenbstart_2d   = {START_OBJ, 0,             PENNY_2d,     0, CYCLE_FORWARD};
act1 apenbstop_2d    = {START_OBJ, 20,            PENNY_2d,     0, NOT_CYCLING};
act1 apeng1_2d       = {START_OBJ, 23,            HERO,         0, ALMOST_INVISIBLE};
act1 apeng3_2d       = {START_OBJ, 40,            HERO,         0, NOT_CYCLING};
act1 aridmaidlips_2d = {START_OBJ, 15,            LIPS_2d,      0, ALMOST_INVISIBLE};
act1 aherostart_2d   = {START_OBJ, 0,             HERO,         0, NOT_CYCLING};
act1 apenstart_2d    = {START_OBJ, PENDELAY + 10, PENNY_2d,     0, CYCLE_FORWARD};
act1 apenstop_2d     = {START_OBJ, PENDELAY + 70, PENNY_2d,     0, NOT_CYCLING};

act2 aback3_2d      = {INIT_OBJXY, 2,         CAT_2d,      189, 69};
act2 abanana10_2d   = {INIT_OBJXY, 16,        GENIE_2d,    212, 10};
act2 abanana11_2d   = {INIT_OBJXY, 18,        GENIE_2d,    209, 20};
act2 abanana12_2d   = {INIT_OBJXY, 20,        GENIE_2d,    206, 40};
act2 abanana13_2d   = {INIT_OBJXY, 22,        GENIE_2d,    200, 65};
act2 abanana5_2d    = {INIT_OBJXY, 00,        GENIE_2d,    203, 60};
act2 abanana6_2d    = {INIT_OBJXY, 02,        GENIE_2d,    206, 40};
act2 abanana7_2d    = {INIT_OBJXY, 04,        GENIE_2d,    209, 20};
act2 abanana8_2d    = {INIT_OBJXY, 06,        GENIE_2d,    212, 10};
act2 abd1_2d        = {INIT_OBJXY, 0,         HERO,        214, 92};
act2 abd10_2d       = {INIT_OBJXY, 0,         HERO,        20,  92};
act2 abd20_2d       = {INIT_OBJXY, 0,         HERO,        20,  92};
act2 abed2_1_2d     = {INIT_OBJXY, 0,         HERO,        200, 95};
act2 abed3_1_2d     = {INIT_OBJXY, 0,         HERO,        60,  120};
act2 abel3_2d       = {INIT_OBJXY, 0,         BELL_2d,     185, 110};
act2 aboomxy_2d     = {INIT_OBJXY, 0,         PENNYLIE_2d, 120, 110};
act2 acatroom1_2d   = {INIT_OBJXY, 0,         HERO,        258, 98};
act2 achasm25_2d    = {INIT_OBJXY, 0,         HERO,        155, 20};
act2 adumb1_2d      = {INIT_OBJXY, 0,         HERO,        36,  107};
act2 adumb11_2d     = {INIT_OBJXY, 0,         HERO,        64,  103};
act2 afall1_2d      = {INIT_OBJXY, 0,         PENFALL_2d,  188, 95};
act2 agard3_2d      = {INIT_OBJXY, 0,         GARDENER_2d, 190, 90};
act2 agates1_2d     = {INIT_OBJXY, 0,         HERO,        279, 108};
act2 agates11_2d    = {INIT_OBJXY, 0,         HERO,        18,  107};
act2 agiveb6_2d     = {INIT_OBJXY, 2,         CAT_2d,      157, 96};
act2 ahall1_1_2d    = {INIT_OBJXY, 0,         HERO,        14,  92};
act2 ahall1_2_2d    = {INIT_OBJXY, 0,         HERO,        35,  130};
act2 ahall2_1_2d    = {INIT_OBJXY, 0,         HERO,        61,  105};
act2 ahall2_2_2d    = {INIT_OBJXY, 0,         HERO,        231, 120};
act2 ahall2_3_2d    = {INIT_OBJXY, 0,         HERO,        182, 122};
act2 ahall2_4_2d    = {INIT_OBJXY, 0,         HERO,        160, 140};
act2 ahall2_5_2d    = {INIT_OBJXY, 0,         HERO,        264, 117};
act2 ahall3_1_2d    = {INIT_OBJXY, 0,         HERO,        147, 112};
act2 ahall3_2_2d    = {INIT_OBJXY, 0,         HERO,        42,  41};
act2 aheroxy12_2d   = {INIT_OBJXY, DOORDELAY, HERO,        100, 125};
act2 ahest1_2d      = {INIT_OBJXY, 0,         HESTER_2d,   78,  114};
act2 ahestroom1_2d  = {INIT_OBJXY, 0,         HERO,        108, 76};
act2 ainshed1_2d    = {INIT_OBJXY, 0,         HERO,        140, 88};
act2 akennel1_2d    = {INIT_OBJXY, 0,         HERO,        283, 63};
act2 akit1_2d       = {INIT_OBJXY, 0,         HERO,        145, 120};
act2 akit11_2d      = {INIT_OBJXY, 0,         HERO,        40,  108};
act2 aladder1_2d    = {INIT_OBJXY, 0,         HERO,        282, 107};
act2 alamp1_2d      = {INIT_OBJXY, 0,         HERO,        26,  46};
act2 alounge1_2d    = {INIT_OBJXY, 0,         HERO,        232, 55};
act2 amaidbk8_2d    = {INIT_OBJXY, 12,        MAID_2d,     207, 99};
act2 amat4_2d       = {INIT_OBJXY, 0,         MATCHES_2d,  216, 146};
act2 amurd2_2d      = {INIT_OBJXY, 26,        MURDER_2d,   120, 86};
act2 amush1_2d      = {INIT_OBJXY, 0,         HERO,        272, 107};
act2 amush11_2d     = {INIT_OBJXY, 0,         HERO,        18,  107};
act2 amush21_2d     = {INIT_OBJXY, 0,         HERO,        95,  26};
act2 aom17_2d       = {INIT_OBJXY, 60,        OLDMAN_2d,   155, 95};
act2 aorgan1_2d     = {INIT_OBJXY, 0,         HERO,        241, 76};
act2 apanel2_2d     = {INIT_OBJXY, 0,         PANEL_2d,    189, 91};
act2 apanel4_2d     = {INIT_OBJXY, 20,        HERO,        192, 95};
act2 apaper4_2d     = {INIT_OBJXY, 0,         PAPER_2d,    138, 130};
act2 aparlor1_2d    = {INIT_OBJXY, 0,         HERO,        75,  75};
act2 aparlor2_2d    = {INIT_OBJXY, 0,         HERO,        86,  30};
act2 aparlor3_2d    = {INIT_OBJXY, 2,         HERO,        263, 114};
act2 aparty1b_2d    = {INIT_OBJXY, 0,         COP_2d,      80,  115};
act2 aparty2b_2d    = {INIT_OBJXY, 0,         MAID_2d,     100, 130};
act2 aparty3b_2d    = {INIT_OBJXY, 0,         HESTER_2d,   120, 120};
act2 aparty4b_2d    = {INIT_OBJXY, 0,         GARDENER_2d, 140, 120};
act2 aparty5b_2d    = {INIT_OBJXY, 0,         SNAKE_2d,    80,  150};
act2 aparty6b_2d    = {INIT_OBJXY, 0,         GENIE_2d,    70,  120};
act2 aparty7b_2d    = {INIT_OBJXY, 0,         HARRY_2d,    150, 130};
act2 aparty8b_2d    = {INIT_OBJXY, 0,         DOCTOR_2d,   170, 100};
act2 aparty9b_2d    = {INIT_OBJXY, 0,         COOK_2d,     200, 110};
act2 apass1_2d      = {INIT_OBJXY, 0,         HERO,        262, 45};
act2 apass11_2d     = {INIT_OBJXY, 0,         HERO,        18,  107};
act2 apen4_2d       = {INIT_OBJXY, 0,         HERO,        147, 112};
act2 aphone1_2d     = {INIT_OBJXY, 0,         HERO,        286, 108};
act2 aphone11c_2d   = {INIT_OBJXY, 0,         HERO,        146, 127};
act2 aphone11l_2d   = {INIT_OBJXY, 0,         HERO,        285, 87};
act2 aphone11r_2d   = {INIT_OBJXY, 0,         HERO,        18,  93};
act2 aretupxy_2d    = {INIT_OBJXY, 0,         HERO,        61,  81};
act2 aridgard3_2d   = {INIT_OBJXY, 4,         GARDENER_2d, 190, 88};
act2 arockg1_2d     = {INIT_OBJXY, 0,         HERO,        146, 122};
act2 arxy_2d        = {INIT_OBJXY, 0,         HERO,        75,  73};
act2 arxy21_2d      = {INIT_OBJXY, 0,         HERO,        205, 82};
act2 ascr33c_2d     = {INIT_OBJXY, 4,         HERO,        160, 105};
act2 ashed1_2d      = {INIT_OBJXY, 0,         HERO,        139, 139};
act2 ashed11_2d     = {INIT_OBJXY, 0,         HERO,        54,  89};
act2 ashed21_2d     = {INIT_OBJXY, 0,         HERO,        13,  124};
act2 asnake1_2d     = {INIT_OBJXY, 0,         HERO,        264, 79};
act2 asnake11_2d    = {INIT_OBJXY, 0,         HERO,        14,  66};
act2 asnake21_2d    = {INIT_OBJXY, 0,         HERO,        18,  107};
act2 astick3_2d     = {INIT_OBJXY, 0,         DOG_2d,      165, 80};
act2 astream1_2d    = {INIT_OBJXY, 0,         HERO,        283, 124};
act2 astream11_2d   = {INIT_OBJXY, 0,         HERO,        18,  107};
act2 athree1_2d     = {INIT_OBJXY, 0,         HERO,        268, 50};
act2 athree11_2d    = {INIT_OBJXY, 0,         HERO,        272, 69};
act2 athree21_2d    = {INIT_OBJXY, 0,         HERO,        19,  81};
act2 athree31_2d    = {INIT_OBJXY, 0,         HERO,        18,  107};
act2 atrap1_2d      = {INIT_OBJXY, 0,         HERO,        209, 35};
act2 atrap2_2d      = {INIT_OBJXY, 0,         HERO,        270, 87};
act2 auptrap1_2d    = {INIT_OBJXY, 0,         HERO,        193, 101};
act2 avenus1_2d     = {INIT_OBJXY, 0,         HERO,        275, 130};
act2 awell1_2d      = {INIT_OBJXY, 0,         HERO,        146, 131};
act2 awho3_2d       = {INIT_OBJXY, 0,         HERO,        255, 39};
act2 azap1_2d       = {INIT_OBJXY, 0,         HERO,        284, 110};
act2 azap11_2d      = {INIT_OBJXY, 0,         HERO,        25,  64};
act2 abuga1c_2d     = {INIT_OBJXY, 0,         BUG1_2d,     165, 25};
act2 abuga2c_2d     = {INIT_OBJXY, 0,         BUG2_2d,     265, 95};
act2 abuga3c_2d     = {INIT_OBJXY, 0,         BUG3_2d,     255, 110};
act2 abuga4c_2d     = {INIT_OBJXY, 0,         BUG4_2d,     300, 120};
act2 abuga5c_2d     = {INIT_OBJXY, 0,         BUG5_2d,     175, 130};
act2 abugf1c_2d     = {INIT_OBJXY, 0,         BUG1_2d,     65,  25};
act2 abugf2c_2d     = {INIT_OBJXY, 0,         BUG2_2d,     245, 85};
act2 abugf3c_2d     = {INIT_OBJXY, 0,         BUG3_2d,     155, 60};
act2 abugf4c_2d     = {INIT_OBJXY, 0,         BUG4_2d,     270, 25};
act2 abugf5c_2d     = {INIT_OBJXY, 0,         BUG5_2d,     175, 30};
act2 agone3_2d      = {INIT_OBJXY, 120,       HERO,        229, 119};
act2 aheroxy11_2d   = {INIT_OBJXY, 0,         HERO,        160, 100};
act2 aheroxy3435_2d = {INIT_OBJXY, DOORDELAY, HERO,        76,  133};
act2 aheroxy3436_2d = {INIT_OBJXY, DOORDELAY, HERO,        246, 120};
act2 amaidc12_2d    = {INIT_OBJXY, 30,        MAID_2d,     207, 99};
act2 amaidc3_2d     = {INIT_OBJXY, 8,         MAID_2d,     74,  78};
act2 amaidp2_2d     = {INIT_OBJXY, 0,         MAID_2d,     207, 99};
act2 amazexy_2d     = {INIT_OBJXY, 0,         HERO,        134, 92};
act2 apenbxy1_2d    = {INIT_OBJXY, 0,         PENNY_2d,    129, 119};
act2 apenbxy2_2d    = {INIT_OBJXY, 30,        PENNYLIE_2d, 215, 125};
act2 apeng2_2d      = {INIT_OBJXY, 23,        HERO,        192, 93};
act2 aheroxy01_2d   = {INIT_OBJXY, 0,         HERO,        169, 141};
act2 apenxy_2d      = {INIT_OBJXY, 0,         PENNY_2d,    109, 140};

act3 aclimax7_2d = {PROMPT, 100, kSTSclimax7_2d, whorsp_2d,  kALcheat_2d,  kALdidnt_2d,   false};
act3 adial_2d    = {PROMPT, 0,   kSTSdial1_2d,   dialrsp_2d, kALwho_2d,    kALnoreply_2d, false};
act3 ahestd3_2d  = {PROMPT, 0,   kSTShest1_2d,   hestrsp_2d, kALhdrink_2d, kALnodrink_2d, false};
act3 asafepr_2d  = {PROMPT, 0,   kSTSafepr_2d,   dialrsp_2d, kALcomb1_2d,  kALcomb2_2d,   false};

act5 aball2_2d     = {INIT_OBJVXY, 0,             BALLOON_2d,  0,       0};
act5 acop2_2d      = {INIT_OBJVXY, 0,             COP_2d,      DX,      0};
act5 adalek4_2d    = {INIT_OBJVXY, 4,             DALEK_2d,    0,       0};
act5 adead4_2d     = {INIT_OBJVXY, 0,             HERO,        0,       0};
act5 adone11_2d    = {INIT_OBJVXY, 10,            HERO,        0,       -1};
act5 adone7_2d     = {INIT_OBJVXY, 0,             HORACE_2d,   -DX,     DY};
act5 adone8_2d     = {INIT_OBJVXY, 10,            HORACE_2d,   0,       -1};
act5 afall4_2d     = {INIT_OBJVXY, 0,             PENFALL_2d,  0,       4};
act5 ahest10_2d    = {INIT_OBJVXY, 46,            HESTER_2d,   0,       0};
act5 ahest4_2d     = {INIT_OBJVXY, 0,             HESTER_2d,   DX,      0};
act5 ahest6_2d     = {INIT_OBJVXY, 22,            HESTER_2d,   0,       -DY};
act5 ahest8_2d     = {INIT_OBJVXY, 24,            HESTER_2d,   -DX,     -1};
act5 amaidb3_2d    = {INIT_OBJVXY, 0,             MAID_2d,     DX,      0};
act5 amaidb4_2d    = {INIT_OBJVXY, 10,            MAID_2d,     0,       0};
act5 amaidbk4_2d   = {INIT_OBJVXY, 0,             MAID_2d,     -DX,     0};
act5 amaidbk5_2d   = {INIT_OBJVXY, 10,            MAID_2d,     0,       0};
act5 amaidstop2_2d = {INIT_OBJVXY, 0,             MAID_2d,     0,       0};
act5 aom3_2d       = {INIT_OBJVXY, 12,            HERO,        0,       0};
act5 aom8_2d       = {INIT_OBJVXY, 40,            HERO,        0,       0};
act5 apanel3_2d    = {INIT_OBJVXY, 5,             PANEL_2d,    0,       -4};
act5 apanel8_2d    = {INIT_OBJVXY, 25,            PANEL_2d,    0,       4};
act5 aridgard5_2d  = {INIT_OBJVXY, 4,             GARDENER_2d, -2 * DX, 0};
act5 astick6_2d    = {INIT_OBJVXY, 0,             DOG_2d,      -DX * 2, 0};
act5 abkstart1_2d  = {INIT_OBJVXY, 0,             BOOKCASE_2d, -2,      0};
act5 abkstart2_2d  = {INIT_OBJVXY, 23,            BOOKCASE_2d, +2,      0};
act5 abkstop_2d    = {INIT_OBJVXY, 46,            BOOKCASE_2d, 0,       0};
act5 agone15_2d    = {INIT_OBJVXY, 133,           HERO,        -DX,     0};
act5 amaidc1_2d    = {INIT_OBJVXY, 0,             HERO,        0,       0};
act5 amaidc6_2d    = {INIT_OBJVXY, 8,             MAID_2d,     DX,      0};
act5 amaidc8_2d    = {INIT_OBJVXY, 16,            MAID_2d,     -DX,     0};
act5 amaidc9_2d    = {INIT_OBJVXY, 26,            MAID_2d,     0,       0};
act5 apenbvxy1_2d  = {INIT_OBJVXY, 0,             PENNY_2d,    DX,      0};
act5 apenbvxy2_2d  = {INIT_OBJVXY, 20,            PENNY_2d,    0,       0};
act5 astophero_2d  = {INIT_OBJVXY, 0,             HERO,        0,       0};
act5 apenvxy1_2d   = {INIT_OBJVXY, PENDELAY + 10, PENNY_2d,    -DX,     0};
act5 apenvxy2_2d   = {INIT_OBJVXY, PENDELAY + 17, PENNY_2d,    0,       -DY};
act5 apenvxy3_2d   = {INIT_OBJVXY, PENDELAY + 42, PENNY_2d,    DX,      0};
act5 apenvxy4_2d   = {INIT_OBJVXY, PENDELAY + 70, PENNY_2d,    0,       0};

act6 abanana2_2d  = {INIT_CARRY, 0, BANANA_2d,  false};
act6 abel2_2d     = {INIT_CARRY, 0, BELL_2d,    false};
act6 adrop33a_2d  = {INIT_CARRY, 0, BOTTLE_2d,  false};
act6 adrop33b_2d  = {INIT_CARRY, 0, GUN_2d,     false};
act6 adrop33c_2d  = {INIT_CARRY, 0, BELL_2d,    false};
act6 adrop33d_2d  = {INIT_CARRY, 0, SCREW_2d,   false};
act6 adrop33e_2d  = {INIT_CARRY, 0, ALBUM_2d,   false};
act6 adrop33f_2d  = {INIT_CARRY, 0, WILL_2d,    false};
act6 adrop33g_2d  = {INIT_CARRY, 0, OILLAMP_2d, false};
act6 adrop33h_2d  = {INIT_CARRY, 0, CATNIP_2d,  false};
act6 adrop33i_2d  = {INIT_CARRY, 0, MAGNIFY_2d, false};
act6 adrop33j_2d  = {INIT_CARRY, 0, MATCHES_2d, false};
act6 adropban_2d  = {INIT_CARRY, 0, BANANA_2d,  false};
act6 adropgarl_2d = {INIT_CARRY, 0, GARLIC_2d,  false};
act6 amat3_2d     = {INIT_CARRY, 0, MATCHES_2d, false};
act6 apaper3_2d   = {INIT_CARRY, 0, PAPER_2d,   false};
act6 aphoto1_2d   = {INIT_CARRY, 0, ALBUM_2d,   true};
act6 asafe4_2d    = {INIT_CARRY, 0, WILL_2d,    true};
act6 asonic5_2d   = {INIT_CARRY, 0, SCREW_2d,   true};
act6 astick9_2d   = {INIT_CARRY, 0, STICK_2d,   false};

act7 adead2_2d     = {INIT_HF_COORD, 0,  PENNYLIE_2d};
act7 agard2_2d     = {INIT_HF_COORD, 0,  GARDENER_2d};
act7 agetgarlic_2d = {INIT_HF_COORD, 0,  GARLIC_2d};
act7 agetmatch_2d  = {INIT_HF_COORD, 0,  MATCHES_2d};
act7 ahfaint2_2d   = {INIT_HF_COORD, 4,  PENNYLIE_2d};
act7 afaint3_2d    = {INIT_HF_COORD, 10, PENNYLIE_2d};
act7 apenscr_2d    = {INIT_HF_COORD, 0,  PENNY_2d};

act8 akaboom3_2d   = {NEW_SCREEN, 0,               22};
act8 arg_2d        = {NEW_SCREEN, 0,               22};
act8 arr_2d        = {NEW_SCREEN, 0,               21};
act8 arr21_2d      = {NEW_SCREEN, 0,               16};
act8 ascr0204_2d   = {NEW_SCREEN, 0,               4};
act8 ascr0305_2d   = {NEW_SCREEN, 0,               5};
act8 ascr0306_2d   = {NEW_SCREEN, 0,               6};
act8 ascr0402_2d   = {NEW_SCREEN, 30,              2};
act8 ascr0503_2d   = {NEW_SCREEN, 0,               3};
act8 ascr0603_2d   = {NEW_SCREEN, 0,               3};
act8 ascr0607_2d   = {NEW_SCREEN, 0,               7};
act8 ascr0631_2d   = {NEW_SCREEN, 0,               31};
act8 ascr0706_2d   = {NEW_SCREEN, 0,               6};
act8 ascr0708_2d   = {NEW_SCREEN, 0,               8};
act8 ascr0710_2d   = {NEW_SCREEN, 0,               10};
act8 ascr0807_2d   = {NEW_SCREEN, 0,               7};
act8 ascr0809_2d   = {NEW_SCREEN, 0,               9};
act8 ascr0811_2d   = {NEW_SCREEN, 0,               11};
act8 ascr0812_2d   = {NEW_SCREEN, 0,               12};
act8 ascr0908_2d   = {NEW_SCREEN, 0,               8};
act8 ascr1007_2d   = {NEW_SCREEN, 0,               7};
act8 ascr1108_2d   = {NEW_SCREEN, 0,               8};
act8 ascr1113_2d   = {NEW_SCREEN, 0,               13};
act8 ascr12_2d     = {NEW_SCREEN, DOORDELAY,       2};
act8 ascr1314_2d   = {NEW_SCREEN, 0,               14};
act8 ascr1413_2d   = {NEW_SCREEN, 0,               13};
act8 ascr1415_2d   = {NEW_SCREEN, 0,               15};
act8 ascr1514_2d   = {NEW_SCREEN, 0,               14};
act8 ascr1516_2d   = {NEW_SCREEN, 0,               16};
act8 ascr1517_2d   = {NEW_SCREEN, 0,               17};
act8 ascr1615_2d   = {NEW_SCREEN, 0,               15};
act8 ascr1715_2d   = {NEW_SCREEN, 0,               15};
act8 ascr1718_2d   = {NEW_SCREEN, 0,               18};
act8 ascr1720_2d   = {NEW_SCREEN, 0,               20};
act8 ascr1817_2d   = {NEW_SCREEN, 0,               17};
act8 ascr1819_2d   = {NEW_SCREEN, 0,               19};
act8 ascr1837_2d   = {NEW_SCREEN, 0,               37};
act8 ascr1918_2d   = {NEW_SCREEN, 0,               18};
act8 ascr2017_2d   = {NEW_SCREEN, 0,               17};
act8 ascr2223_2d   = {NEW_SCREEN, 0,               23};
act8 ascr2322_2d   = {NEW_SCREEN, 0,               22};
act8 ascr2324_2d   = {NEW_SCREEN, 0,               24};
act8 ascr2325_2d   = {NEW_SCREEN, 0,               25};
act8 ascr2326_2d   = {NEW_SCREEN, 0,               26};
act8 ascr2423_2d   = {NEW_SCREEN, 0,               23};
act8 ascr2523_2d   = {NEW_SCREEN, 0,               23};
act8 ascr2623_2d   = {NEW_SCREEN, 0,               23};
act8 ascr2627_2d   = {NEW_SCREEN, 0,               27};
act8 ascr2726_2d   = {NEW_SCREEN, 0,               26};
act8 ascr2827_2d   = {NEW_SCREEN, 0,               27};
act8 ascr2829_2d   = {NEW_SCREEN, 0,               29};
act8 ascr2928_2d   = {NEW_SCREEN, 0,               28};
act8 ascr2930_2d   = {NEW_SCREEN, 0,               30};
act8 ascr2931_2d   = {NEW_SCREEN, 0,               31};
act8 ascr2934_2d   = {NEW_SCREEN, 0,               34};
act8 ascr2938_2d   = {NEW_SCREEN, 0,               38};
act8 ascr3029_2d   = {NEW_SCREEN, 0,               29};
act8 ascr3106_2d   = {NEW_SCREEN, 2,               6};
act8 ascr3129_2d   = {NEW_SCREEN, 0,               29};
act8 ascr3132_2d   = {NEW_SCREEN, 0,               32};
act8 ascr3231_2d   = {NEW_SCREEN, 0,               31};
act8 ascr3334_2d   = {NEW_SCREEN, 0,               34};
act8 ascr33d_2d    = {NEW_SCREEN, 4,               33};
act8 ascr3429_2d   = {NEW_SCREEN, 0,               29};
act8 ascr3438_2d   = {NEW_SCREEN, 0,               38};
act8 ascr3534_2d   = {NEW_SCREEN, 0,               34};
act8 ascr3634_2d   = {NEW_SCREEN, 0,               34};
act8 ascr3718_2d   = {NEW_SCREEN, 0,               18};
act8 ascr3829_2d   = {NEW_SCREEN, 0,               29};
act8 ascr3834_2d   = {NEW_SCREEN, 0,               34};
act8 auptrap2_2d   = {NEW_SCREEN, 0,               28};
act8 ascr01_2d     = {NEW_SCREEN, STORYDELAY + 10, 1};
act8 ascr0203_2d   = {NEW_SCREEN, 40,              3};
act8 ascr11maze_2d = {NEW_SCREEN, 0,               MAZE_SCREEN + 59};
act8 ascr3435_2d   = {NEW_SCREEN, DOORDELAY,       35};
act8 ascr3436_2d   = {NEW_SCREEN, DOORDELAY,       36};
act8 ascrmaze_2d   = {NEW_SCREEN, 0,               11};

act9 abanana9_2d   = {INIT_OBJSTATE, 16,        GENIE_2d,    1};
act9 abite1_2d     = {INIT_OBJSTATE, 0,         SNAKE_2d,    1};
act9 acallp1_2d    = {INIT_OBJSTATE, 0,         TARDIS_2d,   1};
act9 acat2_2d      = {INIT_OBJSTATE, 0,         CATNIP_2d,   1};
act9 acook6_2d     = {INIT_OBJSTATE, 0,         COOK_2d,     2};
act9 acookp1_2d    = {INIT_OBJSTATE, 0,         COOK_2d,     1};
act9 adalek6_2d    = {INIT_OBJSTATE, 4,         DALEK_2d,    1};
act9 adynamite1_2d = {INIT_OBJSTATE, 0,         DYNAMITE_2d, 1};
act9 afinito_2d    = {INIT_OBJSTATE, 0,         HERO,        1};
act9 afuze3_2d     = {INIT_OBJSTATE, 0,         WELL_2d,     1};
act9 agarl1_2d     = {INIT_OBJSTATE, 0,         GARLIC_2d,   1};
act9 agenie4_2d    = {INIT_OBJSTATE, 0,         OILLAMP_2d,  1};
act9 agiveb2_2d    = {INIT_OBJSTATE, 0,         CATNIP_2d,   2};
act9 ahdrink1_2d   = {INIT_OBJSTATE, 0,         LETTER_2d,   3};
act9 ahdrink7_2d   = {INIT_OBJSTATE, 50,        LETTER_2d,   2};
act9 ahest16_2d    = {INIT_OBJSTATE, 50,        LETTER_2d,   1};
act9 ahestd1_2d    = {INIT_OBJSTATE, 0,         LETTER_2d,   2};
act9 akeyhole3_2d  = {INIT_OBJSTATE, 0,         KEYHOLE_2d,  1};
act9 akeyhole4_2d  = {INIT_OBJSTATE, 0,         BOOK_2d,     2};
act9 amat1_2d      = {INIT_OBJSTATE, 0,         MATCHES_2d,  1};
act9 amissed1_2d   = {INIT_OBJSTATE, 0,         GUN_2d,      1};
act9 aom4_2d       = {INIT_OBJSTATE, 12,        OLDMAN_2d,   1};
act9 apanel1_2d    = {INIT_OBJSTATE, 0,         PANEL_2d,    1};
act9 apaper1_2d    = {INIT_OBJSTATE, 0,         PAPER_2d,    1};
act9 aphoto5_2d    = {INIT_OBJSTATE, 0,         ALBUM_2d,    1};
act9 aridkey1_2d   = {INIT_OBJSTATE, 0,         PAPER_2d,    2};
act9 asafe2_2d     = {INIT_OBJSTATE, 0,         SAFE_2d,     1};
act9 asafe3_2d     = {INIT_OBJSTATE, 0,         WILL_2d,     1};
act9 asonic7_2d    = {INIT_OBJSTATE, 0,         DOCTOR_2d,   1};
act9 ast12_2d      = {INIT_OBJSTATE, DOORDELAY, DOOR1_2d,    0};
act9 astick2_2d    = {INIT_OBJSTATE, 0,         STICK_2d,    1};
act9 awho4_2d      = {INIT_OBJSTATE, 0,         TARDIS_2d,   2};
act9 amaidp9_2d    = {INIT_OBJSTATE, 12,        MAID_2d,     1};
act9 asetbk1_2d    = {INIT_OBJSTATE, 0,         BOOK_2d,     1};
act9 asetbk2_2d    = {INIT_OBJSTATE, 0,         BOOK_2d,     2};
act9 asetbk3_2d    = {INIT_OBJSTATE, 0,         BOOK_2d,     3};
act9 ast3435_2d    = {INIT_OBJSTATE, DOORDELAY, DOOR2_2d,    0};
act9 ast3436_2d    = {INIT_OBJSTATE, DOORDELAY, DOOR3_2d,    0};

act10 aball1_2d     = {INIT_PATH, 0,   BALLOON_2d,  AUTO,    0,      0};
act10 abanana14_2d  = {INIT_PATH, 24,  GENIE_2d,    WANDER,  DX,     DY};
act10 abanana4_2d   = {INIT_PATH, 0,   GENIE_2d,    AUTO,    0,      0};
act10 acook9_2d     = {INIT_PATH, 60,  COOK_2d,     WANDER,  DX,     DY};
act10 adalek3_2d    = {INIT_PATH, 0,   DALEK_2d,    AUTO,    0,      0};
act10 adalek7_2d    = {INIT_PATH, 4,   DOCTOR_2d,   CHASE,   DX / 2, DY / 2};
act10 agard11_2d    = {INIT_PATH, 300, GARDENER_2d, WANDER,  DX,     DY};
act10 agard4_2d     = {INIT_PATH, 16,  GARDENER_2d, CHASE,   DX,     DY};
act10 agenie3_2d    = {INIT_PATH, 4,   GENIE_2d,    CHASE,   DX,     DY};
act10 ahfaint6_2d   = {INIT_PATH, 8,   HERO,        USER,    0,      0};
act10 akeyhole6_2d  = {INIT_PATH, 0,   HERO,        AUTO,    0,      0};
act10 amaidstop1_2d = {INIT_PATH, 0,   MAID_2d,     AUTO,    0,      0};
act10 aom2_2d       = {INIT_PATH, 12,  HERO,        AUTO,    0,      0};
act10 aom20_2d      = {INIT_PATH, 60,  HERO,        USER,    0,      0};
act10 apanel5_2d    = {INIT_PATH, 20,  HERO,        USER,    0,      0};
act10 aparty1c_2d   = {INIT_PATH, 0,   COP_2d,      CHASE,   DX,     DY};
act10 aparty2c_2d   = {INIT_PATH, 0,   MAID_2d,     WANDER,  DX,     DY};
act10 aparty3c_2d   = {INIT_PATH, 0,   HESTER_2d,   WANDER,  DX,     DY};
act10 aparty4c_2d   = {INIT_PATH, 0,   GARDENER_2d, WANDER,  DX,     DY};
act10 aparty5c_2d   = {INIT_PATH, 0,   SNAKE_2d,    WANDER,  DX,     DY};
act10 aparty6c_2d   = {INIT_PATH, 0,   GENIE_2d,    WANDER,  DX,     DY};
act10 aparty7c_2d   = {INIT_PATH, 0,   HARRY_2d,    AUTO,    0,      0};
act10 aparty8c_2d   = {INIT_PATH, 0,   DOCTOR_2d,   WANDER,  DX,     DY};
act10 aparty9c_2d   = {INIT_PATH, 0,   COOK_2d,     WANDER,  DX,     DY};
act10 aridgard1_2d  = {INIT_PATH, 0,   GARDENER_2d, AUTO,    0,      0};
act10 ascr33a_2d    = {INIT_PATH, 4,   HERO,        USER,    0,      0};
act10 asonic8_2d    = {INIT_PATH, 16,  DOCTOR_2d,   WANDER,  DX,     DY};
act10 astick8_2d    = {INIT_PATH, 16,  HERO,        USER,    0,      0};
act10 abuga1a_2d    = {INIT_PATH, 0,   BUG1_2d,     CHASE,   DX * 2, DY * 2};
act10 abuga1b_2d    = {INIT_PATH, 7,   BUG1_2d,     WANDER2, DX,     DY};
act10 abuga2a_2d    = {INIT_PATH, 0,   BUG2_2d,     CHASE,   DX,     DY};
act10 abuga2b_2d    = {INIT_PATH, 9,   BUG2_2d,     WANDER2, DX,     DY};
act10 abuga3a_2d    = {INIT_PATH, 0,   BUG3_2d,     CHASE,   DX * 2, DY * 2};
act10 abuga3b_2d    = {INIT_PATH, 6,   BUG3_2d,     WANDER2, DX,     DY};
act10 abuga4a_2d    = {INIT_PATH, 0,   BUG4_2d,     CHASE,   DX,     DY};
act10 abuga4b_2d    = {INIT_PATH, 10,  BUG4_2d,     WANDER2, DX * 2, DY * 2};
act10 abuga5a_2d    = {INIT_PATH, 0,   BUG5_2d,     CHASE,   DX,     DY};
act10 abuga5b_2d    = {INIT_PATH, 4,   BUG5_2d,     WANDER2, DX,     DY};
act10 abugf1b_2d    = {INIT_PATH, 10,  BUG1_2d,     WANDER2, DX,     DY};
act10 abugf2b_2d    = {INIT_PATH, 12,  BUG2_2d,     WANDER2, DX,     DY};
act10 abugf3b_2d    = {INIT_PATH, 5,   BUG3_2d,     WANDER2, DX,     DY};
act10 abugf4b_2d    = {INIT_PATH, 10,  BUG4_2d,     WANDER2, DX * 2, DY * 2};
act10 abugf5b_2d    = {INIT_PATH, 5,   BUG5_2d,     WANDER2, DX,     DY};
act10 acyc2_2d      = {INIT_PATH, 120, HERO,        USER,    0,      0};
act10 adisable_2d   = {INIT_PATH, 0,   HERO,        AUTO,    0,      0};
act10 afaint8_2d    = {INIT_PATH, 30,  HERO,        USER,    0,      0};
act10 amaid2_2d     = {INIT_PATH, 18,  MAID_2d,     CHASE,   DX,     DY};
act10 amaidc15_2d   = {INIT_PATH, 30,  HERO,        USER,    0,      0};
act10 amaidgo_2d    = {INIT_PATH, 25,  HERO,        USER,    0,      0};
act10 apeng4_2d     = {INIT_PATH, 40,  HERO,        USER,    0,      0};

act11 abutchk_2d    = {COND_R, 0, GARLIC_2d,   0,    kALcantpush_2d, 0};
act11 achkbite_2d   = {COND_R, 0, SNAKE_2d,    0,    kALnocure_2d,   kALcure_2d};
act11 achkc09_2d    = {COND_R, 0, GARLIC_2d,   0,    kALclue09_2d,   0};
act11 achkcook_2d   = {COND_R, 0, COOK_2d,     1,    kALcook_2d,     0};
act11 achkcookp_2d  = {COND_R, 0, COOK_2d,     0,    kALcookp_2d,    0};
act11 achkcop_2d    = {COND_R, 0, COP_2d,      0x1f, kALcop_2d,      0};
act11 achkcop2_2d   = {COND_R, 0, COP_2d,      0x1f, kALparty_2d,    0};
act11 achkdalek_2d  = {COND_R, 0, DALEK_2d,    1,    kALchkdoc_2d,   0};
act11 achkdoc_2d    = {COND_R, 0, DOCTOR_2d,   0,    kALsonic_2d,    0};
act11 achkgive_2d   = {COND_R, 0, CATNIP_2d,   1,    kALgiveb3_2d,   kALgiveb4_2d};
act11 achkhero_2d   = {COND_R, 0, HERO,        1,    kALdone_2d,     0};
act11 achkhr1_2d    = {COND_R, 0, LETTER_2d,   0,    kALhrgreet_2d,  kALblah_2d};
act11 achkhr2_2d    = {COND_R, 0, LETTER_2d,   1,    kALhprompt_2d,  0};
act11 achkken1_2d   = {COND_R, 0, STICK_2d,    1,    kALthrown_2d,   kALdog1_2d};
act11 achkken2_2d   = {COND_R, 0, DYNAMITE_2d, 0,    kALdyn1_2d,     kALempty_2d};
act11 achkkit_2d    = {COND_R, 0, GENIE_2d,    1,    kALscrok_2d,    0};
act11 achkld1_2d    = {COND_R, 0, MATCHES_2d,  1,    kALlookm2_2d,   kALmatok_2d};
act11 achkmat2_2d   = {COND_R, 0, MATCHES_2d,  0,    kALdropmat_2d,  0};
act11 achknasty_2d  = {COND_R, 0, BOTTLE_2d,   0,    kALnasty_2d,    0};
act11 achkom_2d     = {COND_R, 0, OLDMAN_2d,   0,    kALomgag_2d,    0};
act11 achkpanel_2d  = {COND_R, 0, PANEL_2d,    0,    kALpanel_2d,    0};
act11 achkpaper1_2d = {COND_R, 0, PAPER_2d,    1,    kALchkpap2_2d,  kALridkey_2d};
act11 achkpb1_2d    = {COND_R, 0, TARDIS_2d,   0,    kALcallp_2d,    0};
act11 achkpb2_2d    = {COND_R, 0, TARDIS_2d,   1,    kALdial_2d,     0};
act11 achkphoto_2d  = {COND_R, 0, ALBUM_2d,    0,    kALphoto1_2d,   kALempty_2d};
act11 achkrr1_2d    = {COND_R, 0, WELL_2d,     1,    kALchkrr2_2d,   kALclimbup_2d};
act11 achksnake_2d  = {COND_R, 0, SNAKE_2d,    0,    kALbite_2d,     0};
act11 achktrap_2d   = {COND_R, 0, GENIE_2d,    1,    kALuptrap_2d,   kALnotrap_2d};
act11 achkwell_2d   = {COND_R, 0, WELL_2d,     0,    kALrr_2d,       kALrg_2d};
act11 achkwill_2d   = {COND_R, 0, WILL_2d,     0,    kALchksafe_2d,  kALnopurps_2d};
act11 agard1_2d     = {COND_R, 0, GARLIC_2d,   0,    kALgard1_2d,    0};
act11 akeyhole_2d   = {COND_R, 0, BOOK_2d,     0,    kALkeyhole1_2d, kALkeyhole2_2d};
act11 amatchk1_2d   = {COND_R, 0, MATCHES_2d,  0,    kALlookm1_2d,   kALlookm2_2d};
act11 amatchk2_2d   = {COND_R, 0, MATCHES_2d,  0,    kALstrike1_2d,  kALlookm2_2d};
act11 achkbed1_2d   = {COND_R, 0, BOOK_2d,     0,    kALbed1_2d,     kALfaint_2d};
act11 achkmaid_2d   = {COND_R, 0, MAID_2d,     0,    kALmaidx_2d,    kALblah_2d};
act11 achkstate0_2d = {COND_R, 0, BOOK_2d,     0,    kALhugone_2d,   kALchkstate1_2d};
act11 achkstate1_2d = {COND_R, 0, BOOK_2d,     1,    kALhole_2d,     kALpengone_2d};

act12 aball4_2d    = {TEXT, 2,                       kSTBalloon1_2d};
act12 aball5_2d    = {TEXT, 2,                       kSTBalloon2_2d};
act12 aball6_2d    = {TEXT, 2,                       kSTBalloon3_2d};
act12 abanana1_2d  = {TEXT, 0,                       kSTBanana1_2d};
act12 abanana3_2d  = {TEXT, 0,                       kSTBanana2_2d};
act12 abell_2d     = {TEXT, 0,                       kSTBell1_2d};
act12 abell1_2d    = {TEXT, 8,                       kSTMaid7_2d};
act12 abite2_2d    = {TEXT, 0,                       kSTSnake5_2d};
act12 ablah_2d     = {TEXT, 8,                       kSTBlah_2d};
act12 aboom_2d     = {TEXT, 0,                       kSTDyn4_2d};
act12 acallp2_2d   = {TEXT, 0,                       kSTCall1_2d};
act12 acallp3_2d   = {TEXT, 0,                       kSTCall2_2d};
act12 acallp4_2d   = {TEXT, 0,                       kSTCall3_2d};
act12 acallp5_2d   = {TEXT, 0,                       kSTCall4_2d};
act12 acallp6_2d   = {TEXT, 0,                       kSTCall5_2d};
act12 acallp7_2d   = {TEXT, 0,                       kSTCall6_2d};
act12 acantpush_2d = {TEXT, 0,                       kSTPush1_2d};
act12 acat1_2d     = {TEXT, 0,                       kSTCat3_2d};
act12 acat4_2d     = {TEXT, 0,                       kSTMaid8_2d};
act12 achasm1_2d   = {TEXT, 12,                      kSTChasm1_2d};
act12 acheat1_2d   = {TEXT, 0,                       kSTScheat1_2d};
act12 acheat2_2d   = {TEXT, 0,                       kSTScheat2_2d};
act12 aclimax1_2d  = {TEXT, 0,                       kSTSclimax1_2d};
act12 aclimax2_2d  = {TEXT, 8,                       kSTSclimax2_2d};
act12 aclimax3_2d  = {TEXT, 80,                      kSTSclimax3_2d};
act12 aclimax4_2d  = {TEXT, 80,                      kSTSclimax4_2d};
act12 aclimax5_2d  = {TEXT, 80,                      kSTSclimax5_2d};
act12 aclimax6_2d  = {TEXT, 80,                      kSTSclimax6_2d};
act12 aclue09a_2d  = {TEXT, 0,                       kSTSclue09a_2d};
act12 aclue09b_2d  = {TEXT, 0,                       kSTSclue09b_2d};
act12 aclue09c_2d  = {TEXT, 0,                       kSTSclue09c_2d};
act12 acomb1_2d    = {TEXT, 0,                       kSTScomb1_2d};
act12 acomb2_2d    = {TEXT, 0,                       kSTScomb2_2d};
act12 acook1_2d    = {TEXT, 8,                       kSTCook1_2d};
act12 acook2_2d    = {TEXT, 8,                       kSTCook2_2d};
act12 acook3_2d    = {TEXT, 8,                       kSTCook3_2d};
act12 acook4_2d    = {TEXT, 24,                      kSTCook4_2d};
act12 acook5_2d    = {TEXT, 24,                      kSTCook5_2d};
act12 adalek1_2d   = {TEXT, 0,                       kSTFire3_2d};
act12 adidnt1_2d   = {TEXT, 0,                       kSTSdidnt1_2d};
act12 adidnt2_2d   = {TEXT, 0,                       kSTSdidnt2_2d};
act12 adog1_2d     = {TEXT, 0,                       kSTDyn2_2d};
act12 adone1_2d    = {TEXT, 10,                      kSTSdone1_2d};
act12 adone13_2d   = {TEXT, 50,                      kSTSdone5_2d};
act12 adone14_2d   = {TEXT, 50,                      kSTSdone6_2d};
act12 adone2_2d    = {TEXT, 10,                      kSTSdone2_2d};
act12 adone3_2d    = {TEXT, 10,                      kSTSdone3_2d};
act12 adone4_2d    = {TEXT, 10,                      kSTSdone4_2d};
act12 adraught_2d  = {TEXT, 5 * 60 * NORMAL_TPS_v2d, kSTDraught_2d};
act12 adropdyn2_2d = {TEXT, 0,                       kSTDyn3_2d};
act12 adumb12_2d   = {TEXT, 0,                       kSTDumb2_2d};
act12 adumb2_2d    = {TEXT, 0,                       kSTDumb1_2d};
act12 adyn1_2d     = {TEXT, 0,                       kSTDyn1_2d};
act12 aeatban_2d   = {TEXT, 0,                       kSTSeatbanana_2d};
act12 aeatgarl1_2d = {TEXT, 0,                       kSTSgarl1_2d};
act12 aeatgarl2_2d = {TEXT, 0,                       kSTSgarl2_2d};
act12 aexplainb_2d = {TEXT, 0,                       kSTSexplainb_2d};
act12 aext1_2d     = {TEXT, 0,                       kSTSdalek1_2d};
act12 aext2_2d     = {TEXT, 0,                       kSTSdalek2_2d};
act12 aext3_2d     = {TEXT, 0,                       kSTSdalek3_2d};
act12 agard10_2d   = {TEXT, 300,                     kSTSgard6_2d};
act12 agard5_2d    = {TEXT, 20,                      kSTSgard1_2d};
act12 agard6_2d    = {TEXT, 30,                      kSTSgard2_2d};
act12 agard7_2d    = {TEXT, 40,                      kSTSgard3_2d};
act12 agard8_2d    = {TEXT, 90,                      kSTSgard4_2d};
act12 agard9_2d    = {TEXT, 100,                     kSTSgard5_2d};
act12 agenie1_2d   = {TEXT, 0,                       kSTRub2_2d};
act12 agiveb3_2d   = {TEXT, 0,                       kSTCat2_2d};
act12 ahdrink11_2d = {TEXT, 62,                      kSTHest4_2d};
act12 ahdrink12_2d = {TEXT, 64,                      kSTSay1_2d};
act12 ahdrink13_2d = {TEXT, 64,                      kSTSay2_2d};
act12 ahdrink2_2d  = {TEXT, 0,                       kSTHest1_2d};
act12 ahdrink8_2d  = {TEXT, 52,                      kSTHest2_2d};
act12 ahdrink9_2d  = {TEXT, 52,                      kSTHest3_2d};
act12 ahest12_2d   = {TEXT, 8,                       kSTHest6_2d};
act12 ahest13_2d   = {TEXT, 12,                      kSTHest7_2d};
act12 ahest14_2d   = {TEXT, 16,                      kSTHest8_2d};
act12 ahest15_2d   = {TEXT, 50,                      kSTHest9_2d};
act12 ahestd2_2d   = {TEXT, 0,                       kSTHest5_2d};
act12 ahnod1_2d    = {TEXT, 0,                       kSTNod1_2d};
act12 ahnod2_2d    = {TEXT, 0,                       kSTSay1_2d};
act12 ahnod3_2d    = {TEXT, 0,                       kSTSay2_2d};
act12 akaboom1_2d  = {TEXT, 0,                       kSTDyn5_2d};
act12 alookm1_2d   = {TEXT, 0,                       kSTMatch1_2d};
act12 alookm2_2d   = {TEXT, 0,                       kSTMatch2_2d};
act12 amat5_2d     = {TEXT, 0,                       kSTMatch4_2d};
act12 amissed2_2d  = {TEXT, 0,                       kSTFire1_2d};
act12 amissed3_2d  = {TEXT, 0,                       kSTFire2_2d};
act12 amurd4_2d    = {TEXT, 10,                      kSTArgue1_2d};
act12 anobang_2d   = {TEXT, 0,                       kSTDyn7_2d};
act12 anobang2_2d  = {TEXT, 0,                       kSTDyn6_2d};
act12 anobell_2d   = {TEXT, 8,                       kSTBell2_2d};
act12 anogenie_2d  = {TEXT, 0,                       kSTRub1_2d};
act12 anoreply_2d  = {TEXT, 0,                       kSTBrrr_2d};
act12 anotrap_2d   = {TEXT, 0,                       kSTTrap1_2d};
act12 aom1_2d      = {TEXT, 8,                       kSTSom1_2d};
act12 aom10_2d     = {TEXT, 50,                      kSTSom4_2d};
act12 aom11_2d     = {TEXT, 50,                      kSTSom5_2d};
act12 aom12_2d     = {TEXT, 50,                      kSTSom6_2d};
act12 aom13_2d     = {TEXT, 54,                      kSTSom7_2d};
act12 aom14_2d     = {TEXT, 54,                      kSTSom8_2d};
act12 aom5_2d      = {TEXT, 16,                      kSTSom2_2d};
act12 aom6_2d      = {TEXT, 20,                      kSTSom3_2d};
act12 aom9_2d      = {TEXT, 40,                      kSTSom3a_2d};
act12 apen1_2d     = {TEXT, 0,                       kSTSpen1_2d};
act12 apen2_2d     = {TEXT, 0,                       kSTSpen2_2d};
act12 apen3_2d     = {TEXT, 0,                       kSTSpen3_2d};
act12 aphoto4_2d   = {TEXT, 0,                       kSTSphoto_2d};
act12 aphoto6_2d   = {TEXT, 0,                       kSTSphoto1_2d};
act12 apois1_2d    = {TEXT, 0,                       kSTSnake1_2d};
act12 apois2_2d    = {TEXT, 0,                       kSTSnake2_2d};
act12 apois3_2d    = {TEXT, 0,                       kSTSnake3_2d};
act12 apois4_2d    = {TEXT, 0,                       kSTSnake4_2d};
act12 aridkey2_2d  = {TEXT, 0,                       kSTSridkey_2d};
act12 arok_2d      = {TEXT, 0,                       kSTWell1_2d};
act12 arumbling_2d = {TEXT, 0,                       kSTRumble_2d};
act12 arup_2d      = {TEXT, 0,                       kSTDyn8_2d};
act12 asafe1_2d    = {TEXT, 0,                       kSTSsafe1_2d};
act12 ascr31_2d    = {TEXT, 0,                       kSTLock1_2d};
act12 aserum1_2d   = {TEXT, 0,                       kSTSserum1_2d};
act12 aserum2_2d   = {TEXT, 0,                       kSTSserum2_2d};
act12 asilly_2d    = {TEXT, 0,                       kSTDyn9_2d};
act12 asniff_2d    = {TEXT, 0,                       kSTCat1_2d};
act12 asonic1_2d   = {TEXT, 0,                       kSTSsonic1_2d};
act12 asonic2_2d   = {TEXT, 0,                       kSTSsonic2_2d};
act12 asonic3_2d   = {TEXT, 0,                       kSTSsonic3_2d};
act12 asonic4_2d   = {TEXT, 0,                       kSTSsonic4_2d};
act12 astick1_2d   = {TEXT, 0,                       kSTWeee_2d};
act12 astrike1_2d  = {TEXT, 0,                       kSTMatch3_2d};
act12 astung_2d    = {TEXT, 0,                       kSTStung_2d};
act12 awarn_2d     = {TEXT, 8,                       kSTSwarn_2d};
act12 awarnz_2d    = {TEXT, 8,                       kSTSwarnz_2d};
act12 awho1_2d     = {TEXT, 0,                       kSTTard1_2d};
act12 awho2_2d     = {TEXT, 0,                       kSTTard2_2d};
act12 awill1_2d    = {TEXT, 0,                       kSTSwill1_2d};
act12 awill2_2d    = {TEXT, 0,                       kSTSwill2_2d};
act12 awill3_2d    = {TEXT, 0,                       kSTSwill3_2d};
act12 awill4_2d    = {TEXT, 0,                       kSTSwill4_2d};
act12 abell2_2d    = {TEXT, 16,                      kSTMaid6_2d};
act12 abug5a_2d    = {TEXT, 0,                       kSTStingeroo_2d};
act12 abug5b_2d    = {TEXT, 0,                       kSTSbug5b_2d};
act12 aclick_2d    = {TEXT, 0,                       kSTClick_2d};
act12 aempty_2d    = {TEXT, 0,                       kSTEmpty_2d};
act12 afaint1_2d   = {TEXT, 5,                       kSTSfaint1_2d};
act12 afaint10_2d  = {TEXT, 35,                      kSTSfaint4_2d};
act12 afaint5_2d   = {TEXT, 20,                      kSTSfaint2_2d};
act12 afaint9_2d   = {TEXT, 35,                      kSTSfaint3_2d};
act12 agone10_2d   = {TEXT, 115,                     kSTSgone6_2d};
act12 agone11_2d   = {TEXT, 115,                     kSTSgone7_2d};
act12 agone5_2d    = {TEXT, 0,                       kSTSgone1_2d};
act12 agone6_2d    = {TEXT, 34,                      kSTSgone2_2d};
act12 agone7_2d    = {TEXT, 70,                      kSTSgone3_2d};
act12 agone8_2d    = {TEXT, 90,                      kSTSgone4_2d};
act12 agone9_2d    = {TEXT, 115,                     kSTSgone5_2d};
act12 aharry3_2d   = {TEXT, 4,                       kSTOrgan1_2d};
act12 aharry4_2d   = {TEXT, 4,                       kSTOrgan2_2d};
act12 aharry5_2d   = {TEXT, 4,                       kSTOrgan3_2d};
act12 aharry7_2d   = {TEXT, 8,                       kSTOrgan4_2d};
act12 ahole_2d     = {TEXT, 0,                       kSTFirst2_2d};
act12 akeyhole1_2d = {TEXT, 0,                       kSTHole1_2d};
act12 alie1_2d     = {TEXT, 13,                      kSTTired_2d};
act12 alie2_2d     = {TEXT, 18,                      kSTTired2_2d};
act12 amaid10_2d   = {TEXT, 90 * NORMAL_TPS_v2d,     kSTSmaid1_8_2d};
act12 amaid11_2d   = {TEXT, 99 * NORMAL_TPS_v2d,     kSTSmaid1_9_2d};
act12 amaid12_2d   = {TEXT, 0,                       kSTSmaid1_10_2d};
act12 amaid3_2d    = {TEXT, 4,                       kSTSmaid1_1_2d};
act12 amaid4_2d    = {TEXT, 17,                      kSTSmaid1_2_2d};
act12 amaid5_2d    = {TEXT, 17,                      kSTSmaid1_3_2d};
act12 amaid6_2d    = {TEXT, 17,                      kSTSmaid1_4_2d};
act12 amaid7_2d    = {TEXT, 30,                      kSTSmaid1_5_2d};
act12 amaid8_2d    = {TEXT, 30 * NORMAL_TPS_v2d,     kSTSmaid1_6_2d};
act12 amaid9_2d    = {TEXT, 60 * NORMAL_TPS_v2d,     kSTSmaid1_7_2d};
act12 amaidp3_2d   = {TEXT, 8,                       kSTMaid1_2d};
act12 amaidp4_2d   = {TEXT, 8,                       kSTMaid2_2d};
act12 amaidp5_2d   = {TEXT, 8,                       kSTMaid3_2d};
act12 amaidp7_2d   = {TEXT, 12,                      kSTMaid4_2d};
act12 amaidp8_2d   = {TEXT, 12,                      kSTMaid5_2d};
act12 anocarry_2d  = {TEXT, 0,                       kSTNocarry_2d};
act12 anopurps_2d  = {TEXT, 0,                       kSTNopurps_2d};
act12 aok_2d       = {TEXT, 0,                       kSTOkgen_2d};
act12 ascr21_2d    = {TEXT, 0,                       kSTSfirst_2d};
act12 astory1_2d   = {TEXT, STORYDELAY,              kSTStory_2d};
act12 astory2_2d   = {TEXT, STORYDELAY,              kSTStory1_2d};

act13 ascr33b_2d   = {SWAP_IMAGES, 4,  HERO, PENNY_2d};
act13 aswaphero_2d = {SWAP_IMAGES, 23, HERO, PENNY_2d};

act14 achkbell1_2d = {COND_SCR, 0, HERO, 31, kALbell1_2d,    kALchkbell2_2d};
act14 achkbell2_2d = {COND_SCR, 0, HERO, 32, kALbell2_2d,    kALnobell_2d};
act14 achkcat2_2d  = {COND_SCR, 0, HERO, 32, kALbell2_2d,    kALcat5_2d};
act14 achkcat3_2d  = {COND_SCR, 0, HERO, 31, kALcat6_2d,     0};
act14 achkgun_2d   = {COND_SCR, 0, HERO, 37, kALshot_2d,     kALmissed_2d};
act14 achklamp_2d  = {COND_SCR, 0, HERO, 27, kALgenie_2d,    kALnogenie_2d};
act14 achkld2_2d   = {COND_SCR, 0, HERO, 21, kALchkld3_2d,   kALnobang_2d};
act14 achkrr_2d    = {COND_SCR, 0, HERO, 21, kALkaboom_2d,   kALboom_2d};
act14 adropdyn1_2d = {COND_SCR, 0, HERO, 21, kALdropdyn1_2d, 0};
act14 adumbchk_2d  = {COND_SCR, 0, HERO, 3,  kALscr0306_2d,  kALscr0603_2d};
act14 agarchk_2d   = {COND_SCR, 0, HERO, 9,  kALridgard_2d,  kALridgarl_2d};
act14 agiveb1_2d   = {COND_SCR, 0, HERO, 32, kALgiveb2_2d,   kALnopurps_2d};

act15 aom7_2d    = {AUTOPILOT, 20, HERO,    OLDMAN_2d, DX, DY};
act15 abugf1a_2d = {AUTOPILOT, 4,  BUG1_2d, ZAPPER_2d, DX, DY};
act15 abugf2a_2d = {AUTOPILOT, 6,  BUG2_2d, ZAPPER_2d, DX, DY};
act15 abugf3a_2d = {AUTOPILOT, 13, BUG3_2d, ZAPPER_2d, DX, DY};
act15 abugf4a_2d = {AUTOPILOT, 5,  BUG4_2d, ZAPPER_2d, DX, DY};
act15 abugf5a_2d = {AUTOPILOT, 11, BUG5_2d, ZAPPER_2d, DX, DY};

act16 aback4_2d      = {INIT_OBJ_SEQ, 2,             CAT_2d,      0};
act16 abd11_2d       = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 abd2_2d        = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 abd21_2d       = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 aclosedoor1_2d = {INIT_OBJ_SEQ, DOORDELAY,     DOOR1_2d,    0};
act16 adalek2_2d     = {INIT_OBJ_SEQ, 0,             DALEK_2d,    2};
act16 adone10_2d     = {INIT_OBJ_SEQ, 10,            HERO,        __UP};
act16 adone6_2d      = {INIT_OBJ_SEQ, 0,             HORACE_2d,   LEFT};
act16 adone9_2d      = {INIT_OBJ_SEQ, 10,            HORACE_2d,   __UP};
act16 adumb13_2d     = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 adumb3_2d      = {INIT_OBJ_SEQ, 0,             HERO,        RIGHT};
act16 afuze1_2d      = {INIT_OBJ_SEQ, 0,             DYNAMITE_2d, 1};
act16 agiveb5_2d     = {INIT_OBJ_SEQ, 2,             CAT_2d,      1};
act16 ahall1_3_2d    = {INIT_OBJ_SEQ, 0,             HERO,        RIGHT};
act16 ahall2_2a_2d   = {INIT_OBJ_SEQ, 0,             HERO,        LEFT};
act16 ahall3_1a_2d   = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 ahdrink4_2d    = {INIT_OBJ_SEQ, 3,             HESTER_2d,   __UP};
act16 ahdrink5_2d    = {INIT_OBJ_SEQ, 50,            HESTER_2d,   DOWN};
act16 ahest3_2d      = {INIT_OBJ_SEQ, 0,             HESTER_2d,   RIGHT};
act16 ahest5_2d      = {INIT_OBJ_SEQ, 22,            HESTER_2d,   __UP};
act16 ahest7_2d      = {INIT_OBJ_SEQ, 24,            HESTER_2d,   LEFT};
act16 ahest9_2d      = {INIT_OBJ_SEQ, 45,            HESTER_2d,   DOWN};
act16 ainshed2_2d    = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 akit2_2d       = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 amaidb1_2d     = {INIT_OBJ_SEQ, 0,             MAID_2d,     RIGHT};
act16 amaidbk2_2d    = {INIT_OBJ_SEQ, 0,             MAID_2d,     LEFT};
act16 amaidbk6_2d    = {INIT_OBJ_SEQ, 10,            MAID_2d,     DOWN};
act16 amurd1_2d      = {INIT_OBJ_SEQ, 26,            MURDER_2d,   1};
act16 aom16_2d       = {INIT_OBJ_SEQ, 60,            OLDMAN_2d,   1};
act16 apanel7_2d     = {INIT_OBJ_SEQ, 5,             HERO,        DOWN};
act16 apen5_2d       = {INIT_OBJ_SEQ, 0,             HERO,        RIGHT};
act16 aridgard4_2d   = {INIT_OBJ_SEQ, 4,             GARDENER_2d, LEFT};
act16 ashed12_2d     = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 ashed2_2d      = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 ashed22_2d     = {INIT_OBJ_SEQ, 0,             HERO,        RIGHT};
act16 astick4_2d     = {INIT_OBJ_SEQ, 0,             DOG_2d,      1};
act16 avenus2_2d     = {INIT_OBJ_SEQ, 0,             HERO,        DOWN};
act16 aclosedoor2_2d = {INIT_OBJ_SEQ, DOORDELAY,     DOOR2_2d,    0};
act16 aclosedoor3_2d = {INIT_OBJ_SEQ, DOORDELAY,     DOOR3_2d,    0};
act16 agone12_2d     = {INIT_OBJ_SEQ, 125,           HERO,        DOWN};
act16 agone13_2d     = {INIT_OBJ_SEQ, 127,           HERO,        LEFT};
act16 aharry2_2d     = {INIT_OBJ_SEQ, 0,             HARRY_2d,    1};
act16 aheroseq1_2d   = {INIT_OBJ_SEQ, 1,             HERO,        LEFT};
act16 amaid1_2d      = {INIT_OBJ_SEQ, 1,             MAID_2d,     DOWN};
act16 amaidc13_2d    = {INIT_OBJ_SEQ, 30,            MAID_2d,     DOWN};
act16 amaidc4_2d     = {INIT_OBJ_SEQ, 8,             MAID_2d,     RIGHT};
act16 amaidc7_2d     = {INIT_OBJ_SEQ, 16,            MAID_2d,     LEFT};
act16 amaidp6_2d     = {INIT_OBJ_SEQ, 10,            MAID_2d,     DOWN};
act16 apenbseq1_2d   = {INIT_OBJ_SEQ, 0,             PENNY_2d,    RIGHT};
act16 apenbseq2_2d   = {INIT_OBJ_SEQ, 25,            PENNY_2d,    __UP};
act16 apenseq1_2d    = {INIT_OBJ_SEQ, 0,             PENNY_2d,    RIGHT};
act16 apenseq2_2d    = {INIT_OBJ_SEQ, PENDELAY + 7,  PENNY_2d,    DOWN};
act16 apenseq3_2d    = {INIT_OBJ_SEQ, PENDELAY + 10, PENNY_2d,    LEFT};
act16 apenseq4_2d    = {INIT_OBJ_SEQ, PENDELAY + 17, PENNY_2d,    __UP};
act16 apenseq5_2d    = {INIT_OBJ_SEQ, PENDELAY + 42, PENNY_2d,    RIGHT};
act16 apenseq6_2d    = {INIT_OBJ_SEQ, PENDELAY + 74, PENNY_2d,    2};

act17 acopbit1_2d    = {SET_STATE_BITS,   0, COP_2d,    1};
act17 acopbit16_2d   = {SET_STATE_BITS,   0, COP_2d,    16};
act17 acopbit4_2d    = {SET_STATE_BITS,   0, COP_2d,    4};
act17 agatesopn_2d   = {SET_STATE_BITS,   0, BUTTON_2d, 32};
act17 aglighton2_2d  = {SET_STATE_BITS,   0, BUTTON_2d, 2};
act17 aphoto3_2d     = {SET_STATE_BITS,   0, COP_2d,    2};
act17 ashed23_2d     = {SET_STATE_BITS,   0, BUTTON_2d, 16};
act17 aslighton2_2d  = {SET_STATE_BITS,   0, BUTTON_2d, 1};
act17 astream12_2d   = {SET_STATE_BITS,   0, BUTTON_2d, 4};
act17 azapperon2_2d  = {SET_STATE_BITS,   0, BUTTON_2d, 8};
act17 acopbit8_2d    = {SET_STATE_BITS,   0, COP_2d,    8};

// all act18 were defined as act17 with a type set to CLEAR_STATE_BITS
act18 agatescls_2d   = {CLEAR_STATE_BITS, 0, BUTTON_2d, 32};
act18 aglightoff2_2d = {CLEAR_STATE_BITS, 0, BUTTON_2d, 2};
act18 aslightoff2_2d = {CLEAR_STATE_BITS, 0, BUTTON_2d, 1};
act18 azapperoff2_2d = {CLEAR_STATE_BITS, 0, BUTTON_2d, 8};

act19 achkbugs_2d   = {TEST_STATE_BITS, 0, BUTTON_2d, 8,  kALbugflit_2d,   kALbugattack_2d};
act19 achkgates_2d  = {TEST_STATE_BITS, 2, BUTTON_2d, 16, kALswgates_2d,   kALrumbling_2d};
act19 achkglight_2d = {TEST_STATE_BITS, 2, BUTTON_2d, 2,  kALglightoff_2d, kALglighton_2d};
act19 achkgo_2d     = {TEST_STATE_BITS, 0, BUTTON_2d, 32, kALgoopen_2d,    kALgoclosed_2d};
act19 achkslight_2d = {TEST_STATE_BITS, 2, BUTTON_2d, 1,  kALslightoff_2d, kALslighton_2d};
act19 achksong_2d   = {TEST_STATE_BITS, 0, COP_2d,    8,  0,            kALsong4_2d};
act19 achkzapper_2d = {TEST_STATE_BITS, 2, BUTTON_2d, 4,  kALswzapper_2d,  kALclick_2d};
act19 aswgates_2d   = {TEST_STATE_BITS, 2, BUTTON_2d, 32, kALgatescls_2d,  kALgatesopn_2d};
act19 aswzapper_2d  = {TEST_STATE_BITS, 2, BUTTON_2d, 8,  kALzapperoff_2d, kALzapperon_2d};

act20 acure_2d     = {DEL_EVENTS, 0, AGSCHEDULE};
act20 aridpath_2d  = {DEL_EVENTS, 5, INIT_PATH};
act20 aridsched_2d = {DEL_EVENTS, 0, ASCHEDULE};
act20 aridtests_2d = {DEL_EVENTS, 0, TEST_STATE_BITS};
act20 aridtext_2d  = {DEL_EVENTS, 5, TEXT};

act21 adead5_2d   = {GAMEOVER, 0};
act21 afall5_2d   = {GAMEOVER, 12};

act23 adone15_2d  = {EXIT, 54};

act24 abonus10_2d = {BONUS, 0,  10};
act24 abonus11_2d = {BONUS, 57, 11};
act24 abonus12_2d = {BONUS, 0,  12};
act24 abonus13_2d = {BONUS, 0,  13};
act24 abonus14_2d = {BONUS, 0,  14};
act24 abonus15_2d = {BONUS, 0,  15};
act24 abonus16_2d = {BONUS, 0,  16};
act24 abonus17_2d = {BONUS, 0,  17};
act24 abonus18_2d = {BONUS, 0,  18};
act24 abonus19_2d = {BONUS, 0,  19};
act24 abonus2_2d  = {BONUS, 0,  2};
act24 abonus20_2d = {BONUS, 0,  20};
act24 abonus21_2d = {BONUS, 0,  21};
act24 abonus22_2d = {BONUS, 0,  22};
act24 abonus24_2d = {BONUS, 0,  24};
act24 abonus25_2d = {BONUS, 0,  25};
act24 abonus26_2d = {BONUS, 0,  26};
act24 abonus27_2d = {BONUS, 0,  27};
act24 abonus30_2d = {BONUS, 0,  30};
act24 abonus6_2d  = {BONUS, 0,  6};
act24 abonus7_2d  = {BONUS, 0,  7};
act24 abonus8_2d  = {BONUS, 0,  8};
act24 abonus9_2d  = {BONUS, 0,  9};
act24 abonus1_2d  = {BONUS, 0,  1};
act24 abonus23_2d = {BONUS, 8,  23};
act24 abonus3_2d  = {BONUS, 0,  3};

act25 achkdrop_2d = {COND_BOX, 0, HERO,        196, 78,  283, 115, 0,         kALdropdyn2_2d};
act25 achkld3_2d  = {COND_BOX, 0, DYNAMITE_2d, 196, 78,  283, 125, kALchkld4_2d, kALnobang2_2d};
act25 achkph2_2d  = {COND_BOX, 0, MAID_2d,     192, 134, 251, 149, kALphoto2_2d, kALphoto3_2d};

act27 aphoto2_2d = {ADD_SCORE, 0, ALBUM_2d};
act27 asafe5_2d  = {ADD_SCORE, 0, WILL_2d};
act27 asonic6_2d = {ADD_SCORE, 0, SCREW_2d};

act28 abel4_2d     = {SUB_SCORE, 0, BELL_2d};
act28 amat6_2d     = {SUB_SCORE, 0, MATCHES_2d};
act28 asubban_2d   = {SUB_SCORE, 0, BANANA_2d};
act28 subgarlic_2d = {SUB_SCORE, 0, GARLIC_2d};

act29 achkbel_2d    = {COND_CARRY, 0, BELL_2d,     kALcat2_2d,     kALnocarry_2d};
act29 achkcarry_2d  = {COND_CARRY, 0, BELL_2d,     kALcat3_2d,     kALcat4_2d};
act29 achkld4_2d    = {COND_CARRY, 0, DYNAMITE_2d, kALnobang2_2d,  kALbang1_2d};
act29 achkmag_2d    = {COND_CARRY, 0, MAGNIFY_2d,  kALwill1_2d,    kALwill2_2d};
act29 achkmat1_2d   = {COND_CARRY, 0, MATCHES_2d,  kALchkmat2_2d,  0};
act29 achkpaper2_2d = {COND_CARRY, 0, PAPER_2d,    kALridkey_2d,   kALpencil_2d};
act29 achkrr3_2d    = {COND_CARRY, 0, DYNAMITE_2d, kALsilly_2d,    kALclimbup_2d};
act29 achksafe_2d   = {COND_CARRY, 0, SCREW_2d,    kALchkscrew_2d, kALsafepr_2d};

act30 ainitmaze_2d = {INIT_MAZE, 0, 8, 50, 37, 260, 154, 140, 175, MAZE_SCREEN};

act31 aexitmaze_2d = {EXIT_MAZE, 0};

act32 apanel6_2d = {INIT_PRIORITY, 20, PANEL_2d, BACKGROUND};

act33 acookp3_2d  = {INIT_SCREEN, 0,  COOKB_2d,     6};
act33 afall3_2d   = {INIT_SCREEN, 0,  PENFALL_2d,   25};
act33 amaidbk1_2d = {INIT_SCREEN, 0,  MAID_2d,      31};
act33 aparty1a_2d = {INIT_SCREEN, 0,  COP_2d,       30};
act33 aparty2a_2d = {INIT_SCREEN, 0,  MAID_2d,      30};
act33 aparty3a_2d = {INIT_SCREEN, 0,  HESTER_2d,    30};
act33 aparty4a_2d = {INIT_SCREEN, 0,  GARDENER_2d,  30};
act33 aparty5a_2d = {INIT_SCREEN, 0,  SNAKE_2d,     30};
act33 aparty6a_2d = {INIT_SCREEN, 0,  GENIE_2d,     30};
act33 aparty7a_2d = {INIT_SCREEN, 0,  HARRY_2d,     30};
act33 aparty8a_2d = {INIT_SCREEN, 0,  DOCTOR_2d,    30};
act33 aparty9a_2d = {INIT_SCREEN, 0,  COOK_2d,      30};
act33 ashed24_2d  = {INIT_SCREEN, 0,  GATELIGHT_2d, 11};
act33 ashed25_2d  = {INIT_SCREEN, 0,  GATELIGHT_2d, 12};
act33 astick10_2d = {INIT_SCREEN, 0,  STICK_2d,     -1};
act33 amaidc11_2d = {INIT_SCREEN, 30, MAID_2d,      31};
act33 amaidc2_2d  = {INIT_SCREEN, 8,  MAID_2d,      32};
act33 amaidp1_2d  = {INIT_SCREEN, 0,  MAID_2d,      31};

act34 abang1_2d   = {AGSCHEDULE, 5 * NORMAL_TPS_v2d,   kALbang2_2d};
act34 abite3_2d   = {AGSCHEDULE, 60 * NORMAL_TPS_v2d,  kALpois1_2d};
act34 abite4_2d   = {AGSCHEDULE, 200 * NORMAL_TPS_v2d, kALpois2_2d};
act34 abite5_2d   = {AGSCHEDULE, 290 * NORMAL_TPS_v2d, kALpois3_2d};
act34 abite6_2d   = {AGSCHEDULE, 300 * NORMAL_TPS_v2d, kALpois4_2d};
act34 acat3_2d    = {AGSCHEDULE, 8 * NORMAL_TPS_v2d,   kALchkcarry_2d};
act34 akaboom2_2d = {AGSCHEDULE, 1,                    kALkaboom3_2d};
act34 amaidb6_2d  = {AGSCHEDULE, 8 * NORMAL_TPS_v2d,   kALmaidbk_2d};

act35 amap0_2d = {REMAPPAL, 0, _TLIGHTMAGENTA, _TLIGHTMAGENTA};
act35 amap1_2d = {REMAPPAL, 0, _TLIGHTMAGENTA, _TBLACK};

act36 achkcat1_2d   = {COND_NOUN, 0, kNBell_2d,  kALcat1_2d,    kALnopurps_2d};
act36 achkpencil_2d = {COND_NOUN, 0, kNKey_2d,   kALchkpap1_2d, kALnopurps_2d};
act36 achkscrew_2d  = {COND_NOUN, 0, kNScrew_2d, kALsafe1_2d,   kALsafepr_2d};

act37 acookp2_2d   = {SCREEN_STATE, 0, 6,  1};
act37 ascreen2_2d  = {SCREEN_STATE, 0, 2,  1};
act37 ascreen35_2d = {SCREEN_STATE, 0, 35, 1};

act38 acooklips_2d = {INIT_LIPS, 10, LIPS_2d, COOKB_2d,  LIPDX - 1, LIPDY + 1};
act38 aomlips_2d   = {INIT_LIPS, 41, LIPS_2d, OLDMAN_2d, 24,        LIPDY};
act38 alips_2d     = {INIT_LIPS, 0,  LIPS_2d, MAID_2d,   LIPDX,     LIPDY};
act38 amaidlips_2d = {INIT_LIPS, 5,  LIPS_2d, MAID_2d,   LIPDX,     LIPDY};

//Strangerke - act26 are stored in new act49, as songs were not handled the same way in DOS version (in harcoded strings)
act49 asong2a_2d  = {OLD_SONG, 0,  kDTsong2_2d};
act49 asong2dn_2d = {OLD_SONG, 30, kDTsong2a_2d};
act49 asong2up_2d = {OLD_SONG, 10, kDTsong2_2d};
act49 asong5_2d   = {OLD_SONG, 0,  kDTsong5_2d};
act49 asong0_2d   = {OLD_SONG, 0,  kDTsong0_2d};
act49 asong1_2d   = {OLD_SONG, 0,  kDTsong1_2d};
act49 asong2_2d   = {OLD_SONG, 24, kDTsong2_2d};
act49 asong3_2d   = {OLD_SONG, 0,  kDTsong3_2d};
act49 asong4_2d   = {OLD_SONG, 0,  kDTsong4_2d};
act49 asong6_2d   = {OLD_SONG, 4,  kDTsong6_2d};

actListPtr AL11maze_2d[]        = {&ainitmaze_2d, &amazexy_2d, &ascr11maze_2d, 0};
actListPtr ALballoon_2d[]       = {&aball1_2d, &aball2_2d, &aball3_2d, &aball4_2d, &aball5_2d, &aball6_2d, &asong6_2d, 0};
actListPtr ALbanana_2d[]        = {&abonus18_2d, &abanana1_2d, &abanana2_2d, &abanana3_2d, &abanana4_2d, &abanana5_2d, &abanana6_2d, &abanana7_2d, &abanana8_2d, &abanana9_2d, &abanana10_2d, &abanana11_2d, &abanana12_2d, &abanana13_2d, &abanana14_2d, 0};
actListPtr ALbang1_2d[]         = {&abang1_2d, &afuze1_2d, &afuze2_2d, &afuze3_2d, 0};
actListPtr ALbang2_2d[]         = {&achkrr_2d, 0};
actListPtr ALbed1_2d[]          = {&apenscr_2d, &apenbxy1_2d, &apenbseq1_2d, &apenbstart_2d, &apenbvxy1_2d, &apenbvxy2_2d, &apenbseq2_2d, &apenbstop_2d, &apenbxy2_2d, &apenblie1_2d, &apenblie2_2d, &alie1_2d, &alie2_2d, 0};
actListPtr ALbell_2d[]          = {&achkbell1_2d, 0};
actListPtr ALbell1_2d[]         = {&abell_2d, &abell1_2d, 0};
actListPtr ALbell2_2d[]         = {&abell_2d, &adisable_2d, &amaidc1_2d, &amaidc2_2d, &amaidc3_2d, &amaidc4_2d, &amaidc5_2d, &amaidc6_2d, &abell2_2d, &amaidc7_2d, &amaidc8_2d, &amaidc9_2d, &amaidc10_2d, &amaidc11_2d, &amaidc12_2d, &amaidc13_2d, &amaidc14_2d, &amaidc15_2d, 0};
actListPtr ALbite_2d[]          = {&abite1_2d, &abite2_2d, &achknasty_2d, 0};
actListPtr ALblah_2d[]          = {&ablah_2d, &arepblah_2d, 0};
actListPtr ALboom_2d[]          = {&asong5_2d, &aboom_2d, &abonus17_2d, 0};
actListPtr ALbottle_2d[]        = {&abonus10_2d, &achkbite_2d, 0};
actListPtr ALbridge_2d[]        = {&achkmat1_2d, 0};
actListPtr ALbugattack_2d[]     = {&abuga1a_2d, &abuga1b_2d, &abuga1c_2d, &abuga2a_2d, &abuga2b_2d, &abuga2c_2d, &abuga3a_2d, &abuga3b_2d, &abuga3c_2d, &abuga4a_2d, &abuga4b_2d, &abuga4c_2d, &abuga5a_2d, &abuga5b_2d, &abuga5c_2d, &arepbuga_2d, 0};
actListPtr ALbugflit_2d[]       = {&abugf1a_2d, &abugf1b_2d, &abugf1c_2d, &abugf2a_2d, &abugf2b_2d, &abugf2c_2d, &abugf3a_2d, &abugf3b_2d, &abugf3c_2d, &abugf4a_2d, &abugf4b_2d, &abugf4c_2d, &abugf5a_2d, &abugf5b_2d, &abugf5c_2d, &arepbugf_2d, 0};
actListPtr ALbugrep1_2d[]       = {&abuga1a_2d, &abuga1b_2d, &abuga2a_2d, &abuga2b_2d, &abuga3a_2d, &abuga3b_2d, &abuga4a_2d, &abuga4b_2d, &abuga5a_2d, &abuga5b_2d, &arepbuga_2d, 0};
actListPtr ALbugrep2_2d[]       = {&abugf1a_2d, &abugf1b_2d, &abugf2a_2d, &abugf2b_2d, &abugf3a_2d, &abugf3b_2d, &abugf4a_2d, &abugf4b_2d, &abugf5a_2d, &abugf5b_2d, &arepbugf_2d, 0};
actListPtr ALbugs_2d[]          = {&adead1_2d, &adead2_2d, &adead3_2d, &abug5a_2d, &abug5b_2d, &adead4_2d, &adead5_2d, 0};
actListPtr ALbugzapper_2d[]     = {&abutchk_2d, &achkzapper_2d, 0};
actListPtr ALcallp_2d[]         = {&acallp1_2d, &acallp2_2d, &acallp3_2d, &acallp4_2d, &acallp5_2d, &acallp6_2d, &acallp7_2d, &abonus13_2d, 0};
actListPtr ALcantpush_2d[]      = {&acantpush_2d, &aridtests_2d, 0};
actListPtr ALcat1_2d[]          = {&achkbel_2d, 0};
actListPtr ALcat2_2d[]          = {&acat1_2d, &acat2_2d, 0};
actListPtr ALcat3_2d[]          = {&aback1_2d, &aback2_2d, &aback3_2d, &aback4_2d, 0};
actListPtr ALcat4_2d[]          = {&aback1_2d, &aback2_2d, &aback3_2d, &aback4_2d, &achkcat2_2d, 0};
actListPtr ALcat5_2d[]          = {&achkcat3_2d, 0};
actListPtr ALcat6_2d[]          = {&aridsched_2d, &abell_2d, &acat4_2d, &amaidb1_2d, &amaidb2_2d, &amaidb3_2d, &amaidb4_2d, &amaidb5_2d, &amaidb6_2d, 0};
actListPtr ALcatnip_2d[]        = {&achkcat1_2d, 0};
actListPtr ALchasm_2d[]         = {&achasm1_2d, &adead1_2d, &afall1_2d, &afall2_2d, &afall3_2d, &afall4_2d, &adead4_2d, &afall5_2d, 0};
actListPtr ALcheat_2d[]         = {&acheat1_2d, &acheat2_2d, &abonus25_2d, &ascr33a_2d, &ascr33b_2d, &ascr33c_2d, &ascr33d_2d, 0};
actListPtr ALchkbell2_2d[]      = {&achkbell2_2d, 0};
actListPtr ALchkc09_2d[]        = {&achkc09_2d, 0};
actListPtr ALchkcarry_2d[]      = {&achkcarry_2d, 0};
actListPtr ALchkdoc_2d[]        = {&achkdoc_2d, 0};
actListPtr ALchkld3_2d[]        = {&achkld3_2d, 0};
actListPtr ALchkld4_2d[]        = {&achkld4_2d, 0};
actListPtr ALchkmat2_2d[]       = {&achkmat2_2d, 0};
actListPtr ALchkpap1_2d[]       = {&achkpaper1_2d, 0};
actListPtr ALchkpap2_2d[]       = {&achkpaper2_2d, 0};
actListPtr ALchkrr2_2d[]        = {&achkrr3_2d, 0};
actListPtr ALchksafe_2d[]       = {&achksafe_2d, 0};
actListPtr ALchkscrew_2d[]      = {&achkscrew_2d, 0};
actListPtr ALchkstate1_2d[]     = {&achkstate1_2d, 0};
actListPtr ALclick_2d[]         = {&aclick_2d, 0};
actListPtr ALclimax_2d[]        = {&aclimax1_2d, &aclimax2_2d, &aclimax3_2d, &aclimax4_2d, &aclimax5_2d, &aclimax6_2d, &aclimax7_2d, 0};
actListPtr ALclimbrope_2d[]     = {&achkrr1_2d, 0};
actListPtr ALclimbup_2d[]       = {&arxy21_2d, &arup_2d, &arr21_2d, 0};
actListPtr ALclimbwell_2d[]     = {&abonus16_2d, &achkwell_2d, 0};
actListPtr ALclue09_2d[]        = {&aclue09a_2d, &aclue09b_2d, &aclue09c_2d, 0};
actListPtr ALcomb1_2d[]         = {&acomb1_2d, 0};
actListPtr ALcomb2_2d[]         = {&acomb2_2d, 0};
actListPtr ALcook_2d[]          = {&acooklips_2d, &aridcooklips_2d, &acook1_2d, &acook2_2d, &acook3_2d, &acook4_2d, &acook5_2d, &acook6_2d, &acopbit4_2d, &acook7_2d, &acook8_2d, &acook9_2d, 0};
actListPtr ALcookp_2d[]         = {&abonus22_2d, &acookp1_2d, &acookp2_2d, &acookp3_2d, 0};
actListPtr ALcop_2d[]           = {&acop1_2d, &acop2_2d, &acop3_2d, 0};
actListPtr ALcure_2d[]          = {&aserum1_2d, &aserum2_2d, &acure_2d, 0};
actListPtr ALdalek_2d[]         = {&adead1_2d, &adead2_2d, &adead3_2d, &adead4_2d, &aext1_2d, &aext2_2d, &aext3_2d, &adead5_2d, 0};
actListPtr ALdial_2d[]          = {&adial_2d, 0};
actListPtr ALdidnt_2d[]         = {&adidnt1_2d, &adidnt2_2d, &ascr33a_2d, &ascr33b_2d, &ascr33c_2d, &ascr33d_2d, 0};
actListPtr ALdoctor_2d[]        = {&achkdalek_2d, 0};
actListPtr ALdog1_2d[]          = {&adog1_2d, 0};
actListPtr ALdone_2d[]          = {&adisable_2d, &adone1_2d, &adone2_2d, &adone3_2d, &adone4_2d, &adone5_2d, &adone6_2d, &adone7_2d, &adone8_2d, &adone9_2d, &adone10_2d, &adone11_2d, &adone12_2d, &adone13_2d, &adone14_2d, &adone15_2d, 0};
actListPtr ALdropdyn1_2d[]      = {&achkdrop_2d, 0};
actListPtr ALdropdyn2_2d[]      = {&adropdyn2_2d, 0};
actListPtr ALdropdynamite_2d[]  = {&adropdyn1_2d, 0};
actListPtr ALdropmat_2d[]       = {&amat1_2d, &amat2_2d, &amat3_2d, &amat4_2d, &amat5_2d, &amat6_2d, 0};
actListPtr ALdumb_2d[]          = {&abonus6_2d, &adumbchk_2d, 0};
actListPtr ALdyn1_2d[]          = {&adyn1_2d, &adyn2_2d, 0};
actListPtr ALeatbanana_2d[]     = {&aridban_2d, &aeatban_2d, &asubban_2d, &adropban_2d, 0};
actListPtr ALempty_2d[]         = {&aempty_2d, 0};
actListPtr ALexitmaze_2d[]      = {&aexitmaze_2d, &aheroxy11_2d, &ascrmaze_2d, 0};
actListPtr ALfaint_2d[]         = {&afaint1_2d, &adisable_2d, &afaint2_2d, &afaint3_2d, &afaint4_2d, &afaint5_2d, &afaint6_2d, &afaint7_2d, &afaint8_2d, &afaint9_2d, &afaint10_2d, 0};
actListPtr ALgard1_2d[]         = {&agard2_2d, &agard3_2d, &agard4_2d, &agard5_2d, &agard6_2d, &agard7_2d, &agard8_2d, &agard9_2d, &agard10_2d, &agard11_2d, 0};
actListPtr ALgarlic_2d[]        = {&agarchk_2d, 0};
actListPtr ALgatelight_2d[]     = {&abutchk_2d, &achkglight_2d, 0};
actListPtr ALgatescls_2d[]      = {&agatescls_2d, 0};
actListPtr ALgatesopn_2d[]      = {&abonus8_2d, &agatesopn_2d, 0};
actListPtr ALgenie_2d[]         = {&asong2a_2d, &abonus30_2d, &agenie1_2d, &agenie2_2d, &agenie3_2d, &agenie4_2d, 0};
actListPtr ALgetbook_2d[]       = {&achkstate0_2d, 0};
actListPtr ALgetdynamite_2d[]   = {&adynamite1_2d, 0};
actListPtr ALgetgarlic_2d[]     = {&agetgarlic_2d, 0};
actListPtr ALgetmatch_2d[]      = {&agetmatch_2d, 0};
actListPtr ALgiveb2_2d[]        = {&achkgive_2d, 0};
actListPtr ALgiveb3_2d[]        = {&agiveb2_2d, &agiveb3_2d, &agiveb4_2d, &agiveb5_2d, &agiveb6_2d, &agiveb7_2d, &abel1_2d, &abel2_2d, &abel3_2d, &abel4_2d, &acat3_2d, &abonus21_2d, 0};
actListPtr ALgiveb4_2d[]        = {&asniff_2d, 0};
actListPtr ALgivebel_2d[]       = {&agiveb1_2d, 0};
actListPtr ALglightoff_2d[]     = {&aglightoff1_2d, &aglightoff2_2d, &aclick_2d, 0};
actListPtr ALglighton_2d[]      = {&aglighton1_2d, &aglighton2_2d, &aclick_2d, 0};
actListPtr ALgoclosed_2d[]      = {&ashed25_2d, &ascr0812_2d, 0};
actListPtr ALgoopen_2d[]        = {&ashed24_2d, &ascr0811_2d, 0};
actListPtr ALgun_2d[]           = {&achkgun_2d, 0};
actListPtr ALharry_2d[]         = {&aharry1_2d, &aharry2_2d, &aharry3_2d, &aharry4_2d, &aharry5_2d, &aharry6_2d, &aharry7_2d, &acopbit8_2d, &abonus23_2d, &asong0_2d, &ascreen35_2d, 0};
actListPtr ALhdrink_2d[]        = {&ahdrink1_2d, &ahdrink2_2d, &ahdrink3_2d, &ahdrink4_2d, &ahdrink5_2d, &ahdrink6_2d, &ahdrink7_2d, &ahdrink8_2d, &ahdrink9_2d, &ahdrink10_2d, &ahdrink11_2d, &ahdrink12_2d, &ahdrink13_2d, &arepblah_2d, 0};
actListPtr ALheroxy01_2d[]      = {&aheroxy01_2d, &aherostart_2d, &aheroseq1_2d, 0};
actListPtr ALhfaint_2d[]        = {&adisable_2d, &ahfaint1_2d, &ahfaint2_2d, &ahfaint3_2d, &ahfaint4_2d, &ahfaint5_2d, &ahfaint6_2d, 0};
actListPtr ALhole_2d[]          = {&ahole_2d, 0};
actListPtr ALhprompt_2d[]       = {&ahestd1_2d, &ahestd2_2d, &ahestd3_2d, 0};
actListPtr ALhrgreet_2d[]       = {&ahest1_2d, &ahest2_2d, &ahest3_2d, &ahest4_2d, &ahest5_2d, &ahest6_2d, &ahest7_2d, &ahest8_2d, &ahest9_2d, &ahest10_2d, &ahest11_2d, &ahest12_2d, &ahest13_2d, &ahest14_2d, &ahest15_2d, &ahest16_2d, 0};
actListPtr ALhtable_2d[]        = {&achkhr2_2d, 0};
actListPtr ALhugone_2d[]        = {&asetbk1_2d, &abkstart1_2d, &abkstart2_2d, &abkstop_2d, &aswaphero_2d, &ascreen2_2d, &abonus1_2d, &asong2_2d, &adisable_2d, &agone1_2d, &agone2_2d, &agone3_2d, &agone4_2d, &acyc2_2d, &agone5_2d, &agone6_2d, &agone7_2d, &agone8_2d, &agone9_2d, &agone10_2d, &agone11_2d, &agone12_2d, &agone13_2d, &agone14_2d, &agone15_2d, 0};
actListPtr ALkaboom_2d[]        = {&asong5_2d, &akaboom1_2d, &akaboom2_2d, &akaboom3_2d, 0};
actListPtr ALkaboom3_2d[]       = {&adead1_2d, &adead2_2d, &aboomxy_2d, &adead3_2d, &adead4_2d, &adead5_2d, 0};
actListPtr ALkeyhole_2d[]       = {&akeyhole_2d, 0};
actListPtr ALkeyhole1_2d[]      = {&akeyhole1_2d, 0};
actListPtr ALkeyhole2_2d[]      = {&aok_2d, &akeyhole3_2d, &akeyhole4_2d, &akeyhole5_2d, &akeyhole6_2d, &ascr0204_2d, 0};
actListPtr ALlamp_2d[]          = {&achklamp_2d, 0};
actListPtr ALlightdynamite_2d[] = {&achkld1_2d, 0};
actListPtr ALlookkennel_2d[]    = {&achkken1_2d, 0};
actListPtr ALlookm1_2d[]        = {&alookm1_2d, 0};
actListPtr ALlookm2_2d[]        = {&alookm2_2d, 0};
actListPtr ALlookmatch_2d[]     = {&amatchk1_2d, 0};
actListPtr ALmaid_2d[]          = {&amaidlips_2d, &aridmaidlips_2d, &amaid1_2d, &amaid2_2d, &amaid3_2d, &amaid4_2d, &amaid5_2d, &amaid6_2d, &amaidgo_2d, &amaid7_2d, &amaid8_2d, &amaid9_2d, &amaid10_2d, &amaid11_2d, &arepmsg1_2d, 0};
actListPtr ALmaidbk_2d[]        = {&amaidbk1_2d, &amaidbk2_2d, &amaidbk3_2d, &amaidbk4_2d, &amaidbk5_2d, &amaidbk6_2d, &amaidbk7_2d, &amaidbk8_2d, 0};
actListPtr ALmaidp_2d[]         = {&achkmaid_2d, 0};
actListPtr ALmaidx_2d[]         = {&amaidp1_2d, &amaidp2_2d, &amaidp3_2d, &amaidp4_2d, &amaidp5_2d, &amaidp6_2d, &amaidp7_2d, &amaidp8_2d, &amaidp9_2d, &arepblah_2d, 0};
actListPtr ALmap0_2d[]          = {&amap0_2d, 0};
actListPtr ALmap1_2d[]          = {&amap1_2d, 0};
actListPtr ALmatok_2d[]         = {&achkld2_2d, 0};
actListPtr ALmissed_2d[]        = {&amissed1_2d, &amissed2_2d, &amissed3_2d, 0};
actListPtr ALnasty_2d[]         = {&abite3_2d, &abite4_2d, &abite5_2d, &abite6_2d, 0};
actListPtr ALnobang_2d[]        = {&anobang_2d, 0};
actListPtr ALnobang2_2d[]       = {&anobang2_2d, 0};
actListPtr ALnobell_2d[]        = {&abell_2d, &anobell_2d, 0};
actListPtr ALnocarry_2d[]       = {&anocarry_2d, 0};
actListPtr ALnocure_2d[]        = {&aserum1_2d, 0};
actListPtr ALnodrink_2d[]       = {&ahnod1_2d, &ahnod2_2d, &ahnod3_2d, &arepblah_2d, 0};
actListPtr ALnogenie_2d[]       = {&anogenie_2d, 0};
actListPtr ALnopurps_2d[]       = {&anopurps_2d, 0};
actListPtr ALnoreply_2d[]       = {&anoreply_2d, 0};
actListPtr ALnotrap_2d[]        = {&anotrap_2d, 0};
actListPtr ALomgag_2d[]         = {&aomlips_2d, &aomridlip_2d, &aom1_2d, &aom2_2d, &aom3_2d, &aom4_2d, &aom5_2d, &aom6_2d, &aom7_2d, &aom8_2d, &aom9_2d, &aom10_2d, &aom11_2d, &aom12_2d, &aom13_2d, &aom14_2d, &aom15_2d, &aom16_2d, &aom17_2d, &aom18_2d, &aom19_2d, &aom20_2d, &abonus11_2d, 0};
actListPtr ALopendoor1_2d[]     = {&astophero_2d, &amaidstop1_2d, &amaidstop2_2d, &aopendoor1_2d, &ast12_2d, &aheroxy12_2d, &aclosedoor1_2d, &ascr12_2d, 0};
actListPtr ALopendoor2_2d[]     = {&astophero_2d, &aopendoor2_2d, &ast3435_2d, &aheroxy3435_2d, &aclosedoor2_2d, &ascr3435_2d, 0};
actListPtr ALopendoor3_2d[]     = {&astophero_2d, &aopendoor3_2d, &ast3436_2d, &aheroxy3436_2d, &aclosedoor3_2d, &ascr3436_2d, 0};
actListPtr ALpanel_2d[]         = {&asong2up_2d, &asong2dn_2d, &adisable_2d, &apanel1_2d, &apanel2_2d, &apanel3_2d, &apanel4_2d, &apanel5_2d, &apanel6_2d, &apanel7_2d, &apanel8_2d, 0};
actListPtr ALparty_2d[]         = {&adisable_2d, &aparty1a_2d, &aparty1b_2d, &aparty1c_2d, &aparty2a_2d, &aparty2b_2d, &aparty2c_2d, &aparty3a_2d, &aparty3b_2d, &aparty3c_2d, &aparty4a_2d, &aparty4b_2d, &aparty4c_2d, &aparty5a_2d, &aparty5b_2d, &aparty5c_2d, &aparty6a_2d, &aparty6b_2d, &aparty6c_2d, &aparty7a_2d, &aparty7b_2d, &aparty7c_2d, &aparty8a_2d, &aparty8b_2d, &aparty8c_2d, &aparty9a_2d, &aparty9b_2d, &aparty9c_2d, &aclimax_2d, 0};
actListPtr ALpencil_2d[]        = {&apen1_2d, &apen2_2d, &apen3_2d, &apen4_2d, &apen5_2d, &abonus27_2d, &afinito_2d, &ascr3334_2d, 0};
actListPtr ALpengone_2d[]       = {&abonus3_2d, &abkstart1_2d, &abkstart2_2d, &abkstop_2d, &asong2_2d, &adisable_2d, &apeng1_2d, &apeng2_2d, &apeng3_2d, &apeng4_2d, &ascr0203_2d, 0};
actListPtr ALpenny1_2d[]        = {&apenxy_2d, &apenseq1_2d, &apenseq2_2d, &apenseq3_2d, &apenseq4_2d, &apenstart_2d, &apenvxy1_2d, &apenvxy2_2d, &apenvxy3_2d, &apenvxy4_2d, &apenseq5_2d, &apenseq6_2d, &apenstop_2d, 0};
actListPtr ALphone_2d[]         = {&achkpb2_2d, 0};
actListPtr ALphonebox_2d[]      = {&achkpb1_2d, 0};
actListPtr ALphoto_2d[]         = {&achkphoto_2d, 0};
actListPtr ALphoto1_2d[]        = {&achkph2_2d, 0};
actListPtr ALphoto2_2d[]        = {&aphoto6_2d, 0};
actListPtr ALphoto3_2d[]        = {&aphoto1_2d, &aphoto2_2d, &aphoto3_2d, &aphoto4_2d, &aphoto5_2d, 0};
actListPtr ALpois1_2d[]         = {&apois1_2d, 0};
actListPtr ALpois2_2d[]         = {&apois2_2d, 0};
actListPtr ALpois3_2d[]         = {&apois3_2d, 0};
actListPtr ALpois4_2d[]         = {&apois4_2d, &adead1_2d, &adead2_2d, &adead3_2d, &adead4_2d, &adead5_2d, 0};
actListPtr ALpushpaper_2d[]     = {&abonus26_2d, &apaper1_2d, &apaper2_2d, &apaper3_2d, &apaper4_2d, 0};
actListPtr ALpushpencil_2d[]    = {&achkpencil_2d, 0};
actListPtr ALreadlet_2d[]       = {&abonus24_2d, &acopbit16_2d, 0};
actListPtr ALrepmsg1_2d[]       = {&amaid12_2d, &arepmsg1_2d, 0};
actListPtr ALrg_2d[]            = {&arxy_2d, &arok_2d, &arg_2d, 0};
actListPtr ALridgard_2d[]       = {&adropgarl_2d, &aridgarl_2d, &aeatgarl2_2d, &aridgard1_2d, &aridgard2_2d, &aridgard3_2d, &aridgard4_2d, &aridgard5_2d, &aridpath_2d, &aridtext_2d, &aridgard6_2d, &abonus7_2d, &agarl1_2d, &aschedbut_2d, 0};
actListPtr ALridgarl_2d[]       = {&adropgarl_2d, &aridgarl_2d, &aeatgarl1_2d, &subgarlic_2d, 0};
actListPtr ALridkey_2d[]        = {&aridkey1_2d, &aridkey2_2d, 0};
actListPtr ALrr_2d[]            = {&arxy_2d, &arok_2d, &arr_2d, 0};
actListPtr ALrumbling_2d[]      = {&arumbling_2d, 0};
actListPtr ALsafe_2d[]          = {&achkwill_2d, 0};
actListPtr ALsafe1_2d[]         = {&abonus19_2d, &asafe1_2d, &asafe2_2d, &asafe3_2d, &asafe4_2d, &asafe5_2d, 0};
actListPtr ALsafepr_2d[]        = {&asafepr_2d, 0};
actListPtr ALschedbut_2d[]      = {&aexplainb_2d, 0};
actListPtr ALscr01_2d[]         = {&astory1_2d, &astory2_2d, &ascr01_2d, 0};
actListPtr ALscr02_2d[]         = {&achkbed1_2d, 0};
actListPtr ALscr0201_2d[]       = {&ascr21_2d, 0};
actListPtr ALscr03_2d[]         = {&achkpanel_2d, 0};
actListPtr ALscr0301_2d[]       = {&ascr31_2d, 0};
actListPtr ALscr0305_2d[]       = {&abed2_1_2d, &ascr0305_2d, 0};
actListPtr ALscr0306_2d[]       = {&adumb1_2d, &adumb2_2d, &adumb3_2d, &ascr0306_2d, 0};
actListPtr ALscr04_2d[]         = {&abonus2_2d, &amurd1_2d, &amurd2_2d, &amurd3_2d, &amurd4_2d, &ascr0402_2d, 0};
actListPtr ALscr0503_2d[]       = {&abed3_1_2d, &ascr0503_2d, 0};
actListPtr ALscr06_2d[]         = {&achkcook_2d, 0};
actListPtr ALscr0603_2d[]       = {&adumb11_2d, &adumb12_2d, &adumb13_2d, &ascr0603_2d, 0};
actListPtr ALscr0607_2d[]       = {&akit1_2d, &akit2_2d, &ascr0607_2d, 0};
actListPtr ALscr0631_2d[]       = {&achkkit_2d, 0};
actListPtr ALscr0706_2d[]       = {&abd1_2d, &abd2_2d, &ascr0706_2d, 0};
actListPtr ALscr0708_2d[]       = {&abd20_2d, &abd21_2d, &ascr0708_2d, 0};
actListPtr ALscr0710_2d[]       = {&abd10_2d, &abd11_2d, &ascr0710_2d, 0};
actListPtr ALscr0807_2d[]       = {&ashed1_2d, &ashed2_2d, &ascr0807_2d, 0};
actListPtr ALscr0809_2d[]       = {&ashed11_2d, &ashed12_2d, &ascr0809_2d, 0};
actListPtr ALscr09_2d[]         = {&agard1_2d, &aclue09_2d, 0};
actListPtr ALscr0908_2d[]       = {&ainshed1_2d, &ainshed2_2d, &ascr0908_2d, 0};
actListPtr ALscr10_2d[]         = {&awarn_2d, 0};
actListPtr ALscr1007_2d[]       = {&avenus1_2d, &avenus2_2d, &ascr1007_2d, 0};
actListPtr ALscr1108_2d[]       = {&agates1_2d, &ascr1108_2d, 0};
actListPtr ALscr1113_2d[]       = {&agates11_2d, &ascr1113_2d, 0};
actListPtr ALscr1314_2d[]       = {&astream11_2d, &astream12_2d, &ascr1314_2d, 0};
actListPtr ALscr14_2d[]         = {&achkbugs_2d, &awarnz_2d, 0};
actListPtr ALscr1413_2d[]       = {&azap1_2d, &ascr1413_2d, 0};
actListPtr ALscr1415_2d[]       = {&azap11_2d, &ascr1415_2d, 0};
actListPtr ALscr15_2d[]         = {&achkom_2d, 0};
actListPtr ALscr1514_2d[]       = {&amush1_2d, &ascr1514_2d, 0};
actListPtr ALscr1516_2d[]       = {&amush21_2d, &ascr1516_2d, 0};
actListPtr ALscr1517_2d[]       = {&amush11_2d, &ascr1517_2d, 0};
actListPtr ALscr1615_2d[]       = {&awell1_2d, &ascr1615_2d, 0};
actListPtr ALscr1715_2d[]       = {&asnake1_2d, &ascr1715_2d, 0};
actListPtr ALscr1718_2d[]       = {&asnake11_2d, &ascr1718_2d, 0};
actListPtr ALscr1720_2d[]       = {&asnake21_2d, &ascr1720_2d, 0};
actListPtr ALscr1817_2d[]       = {&aphone1_2d, &ascr1817_2d, 0};
actListPtr ALscr1819c_2d[]      = {&aphone11c_2d, &ascr1819_2d, 0};
actListPtr ALscr1819l_2d[]      = {&aphone11l_2d, &ascr1819_2d, 0};
actListPtr ALscr1819r_2d[]      = {&aphone11r_2d, &ascr1819_2d, 0};
actListPtr ALscr1918c_2d[]      = {&aphone11c_2d, &ascr1918_2d, 0};
actListPtr ALscr1918l_2d[]      = {&aphone11l_2d, &ascr1918_2d, 0};
actListPtr ALscr1918r_2d[]      = {&aphone11r_2d, &ascr1918_2d, 0};
actListPtr ALscr2017_2d[]       = {&akennel1_2d, &ascr2017_2d, 0};
actListPtr ALscr2223_2d[]       = {&arockg1_2d, &ascr2223_2d, 0};
actListPtr ALscr2322_2d[]       = {&athree1_2d, &ascr2322_2d, 0};
actListPtr ALscr2324_2d[]       = {&athree11_2d, &ascr2324_2d, 0};
actListPtr ALscr2325_2d[]       = {&athree21_2d, &ascr2325_2d, 0};
actListPtr ALscr2326_2d[]       = {&athree31_2d, &ascr2326_2d, 0};
actListPtr ALscr2423_2d[]       = {&alamp1_2d, &ascr2423_2d, 0};
actListPtr ALscr2523_2d[]       = {&achasm25_2d, &ascr2523_2d, 0};
actListPtr ALscr2623_2d[]       = {&apass1_2d, &ascr2623_2d, 0};
actListPtr ALscr2627_2d[]       = {&apass11_2d, &ascr2627_2d, 0};
actListPtr ALscr2726_2d[]       = {&aladder1_2d, &ascr2726_2d, 0};
actListPtr ALscr2827_2d[]       = {&atrap1_2d, &ascr2827_2d, 0};
actListPtr ALscr2829_2d[]       = {&atrap2_2d, &ascr2829_2d, 0};
actListPtr ALscr29_2d[]         = {&achkcop_2d, 0};
actListPtr ALscr2928_2d[]       = {&ahall2_1_2d, &ascr2928_2d, 0};
actListPtr ALscr2930_2d[]       = {&ahall2_2_2d, &ahall2_2a_2d, &ascr2930_2d, 0};
actListPtr ALscr2931_2d[]       = {&ahall2_3_2d, &ascr2931_2d, 0};
actListPtr ALscr2934_2d[]       = {&ahall2_4_2d, &ascr2934_2d, 0};
actListPtr ALscr2938_2d[]       = {&ahall2_5_2d, &ascr2938_2d, 0};
actListPtr ALscr30_2d[]         = {&achkcop2_2d, 0};
actListPtr ALscr3029_2d[]       = {&alounge1_2d, &ascr3029_2d, 0};
actListPtr ALscr3106_2d[]       = {&achkcookp_2d, &aparlor3_2d, &ascr3106_2d, 0};
actListPtr ALscr3129_2d[]       = {&aparlor2_2d, &ascr3129_2d, 0};
actListPtr ALscr3132_2d[]       = {&aparlor1_2d, &ascr3132_2d, 0};
actListPtr ALscr3231_2d[]       = {&acatroom1_2d, &ascr3231_2d, 0};
actListPtr ALscr33_2d[]         = {&adraught_2d, &adrop33a_2d, &adrop33b_2d, &adrop33c_2d, &adrop33d_2d, &adrop33e_2d, &adrop33f_2d, &adrop33g_2d, &adrop33h_2d, &adrop33i_2d, &adrop33j_2d, 0};
actListPtr ALscr34_2d[]         = {&achkhero_2d, 0};
actListPtr ALscr3429_2d[]       = {&ahall3_2_2d, &ascr3429_2d, 0};
actListPtr ALscr3438_2d[]       = {&ahall3_1_2d, &ahall3_1a_2d, &ascr3438_2d, 0};
actListPtr ALscr35_2d[]         = {&achksong_2d, 0};
actListPtr ALscr3534_2d[]       = {&asong0_2d, &aorgan1_2d, &ascr3534_2d, 0};
actListPtr ALscr36_2d[]         = {&achkhr1_2d, 0};
actListPtr ALscr3634_2d[]       = {&ahestroom1_2d, &ascr3634_2d, 0};
actListPtr ALscr3718_2d[]       = {&aretupxy_2d, &ascr3718_2d, 0};
actListPtr ALscr3829_2d[]       = {&ahall1_1_2d, &ascr3829_2d, 0};
actListPtr ALscr3834_2d[]       = {&ahall1_2_2d, &ahall1_3_2d, &ascr3834_2d, 0};
actListPtr ALscrgate1_2d[]      = {&ashed21_2d, &ashed22_2d, &ashed23_2d, &achkgo_2d, 0};
actListPtr ALscrgate2_2d[]      = {&astream1_2d, &achkgo_2d, 0};
actListPtr ALscrok_2d[]         = {&akit11_2d, &ascr0631_2d, 0};
actListPtr ALshedlight_2d[]     = {&abutchk_2d, &achkslight_2d, 0};
actListPtr ALshot_2d[]          = {&aridsched_2d, &amissed1_2d, &amissed2_2d, &amissed3_2d, &adalek1_2d, &adalek2_2d, &adalek3_2d, &adalek4_2d, &adalek5_2d, &adalek6_2d, &adalek7_2d, &abonus15_2d, 0};
actListPtr ALsilly_2d[]         = {&asilly_2d, 0};
actListPtr ALslightoff_2d[]     = {&aslightoff1_2d, &aslightoff2_2d, 0};
actListPtr ALslighton_2d[]      = {&aslighton1_2d, &aslighton2_2d, 0};
actListPtr ALsnake_2d[]         = {&achksnake_2d, 0};
actListPtr ALsong1_2d[]         = {&asong1_2d, 0};
actListPtr ALsong3_2d[]         = {&asong3_2d, 0};
actListPtr ALsong4_2d[]         = {&asong4_2d, 0};
actListPtr ALsonic_2d[]         = {&asonic1_2d, &asonic2_2d, &asonic3_2d, &asonic4_2d, &asonic5_2d, &asonic6_2d, &asonic7_2d, &asonic8_2d, 0};
actListPtr ALstrike1_2d[]       = {&astrike1_2d, 0};
actListPtr ALstrikematch_2d[]   = {&amatchk2_2d, 0};
actListPtr ALswgates_2d[]       = {&aswgates_2d, &arumbling_2d, 0};
actListPtr ALswzapper_2d[]      = {&aswzapper_2d, &aclick_2d, 0};
actListPtr ALthrown_2d[]        = {&achkken2_2d, 0};
actListPtr ALthrowstick_2d[]    = {&astick1_2d, &adisable_2d, &astick2_2d, &astick3_2d, &astick4_2d, &astick5_2d, &astick6_2d, &astick7_2d, &astick8_2d, &astick9_2d, &astick10_2d, &abonus12_2d, 0};
actListPtr ALtrap_2d[]          = {&achktrap_2d, 0};
actListPtr ALuptrap_2d[]        = {&auptrap1_2d, &auptrap2_2d, 0};
actListPtr ALvenus_2d[]         = {&astung_2d, &adead1_2d, &adead2_2d, &adead3_2d, &adead4_2d, &adead5_2d, 0};
actListPtr ALwho_2d[]           = {&abonus14_2d, &awho1_2d, &awho2_2d, &awho3_2d, &awho4_2d, &ascr1837_2d, 0};
actListPtr ALwill_2d[]          = {&achkmag_2d, 0};
actListPtr ALwill1_2d[]         = {&awill1_2d, &awill2_2d, &acopbit1_2d, &abonus20_2d, 0};
actListPtr ALwill2_2d[]         = {&awill3_2d, &awill4_2d, 0};
actListPtr ALworkgates_2d[]     = {&abutchk_2d, &achkgates_2d, 0};
actListPtr ALzapperoff_2d[]     = {&azapperoff1_2d, &azapperoff2_2d, 0};
actListPtr ALzapperon_2d[]      = {&abonus9_2d, &azapperon1_2d, &azapperon2_2d, 0};

// Special action list for maze
act2  aheroxy_2d   = {INIT_OBJXY,  0, HERO, 0, 0};
act1  aherostop_2d = {START_OBJ,   0, HERO, 0, NOT_CYCLING};
act8  anewscr_2d   = {NEW_SCREEN,  0, 0};
actListPtr ALnewscr_2d[] = {&aheroxy_2d, &astophero_2d, &aherostop_2d, &anewscr_2d, 0};

actList actListArr_2d[] = {
	ALDummy,           AL11maze_2d,     ALballoon_2d,       ALbanana_2d,     ALbang1_2d,
	ALbang2_2d,        ALbed1_2d,       ALbell_2d,          ALbell1_2d,      ALbell2_2d,
	ALbite_2d,         ALblah_2d,       ALboom_2d,          ALbottle_2d,     ALbridge_2d,
	ALbugattack_2d,    ALbugflit_2d,    ALbugrep1_2d,       ALbugrep2_2d,    ALbugs_2d,
	ALbugzapper_2d,    ALcallp_2d,      ALcantpush_2d,      ALcat1_2d,       ALcat2_2d,
	ALcat3_2d,         ALcat4_2d,       ALcat5_2d,          ALcat6_2d,       ALcatnip_2d,
	ALchasm_2d,        ALcheat_2d,      ALchkbell2_2d,      ALchkc09_2d,     ALchkcarry_2d,
	ALchkdoc_2d,       ALchkld3_2d,     ALchkld4_2d,        ALchkmat2_2d,    ALchkpap1_2d,
	ALchkpap2_2d,      ALchkrr2_2d,     ALchksafe_2d,       ALchkscrew_2d,   ALchkstate1_2d,
	ALclick_2d,        ALclimax_2d,     ALclimbrope_2d,     ALclimbup_2d,    ALclimbwell_2d,
	ALclue09_2d,       ALcomb1_2d,      ALcomb2_2d,         ALcook_2d,       ALcookp_2d,
	ALcop_2d,          ALcure_2d,       ALdalek_2d,         ALdial_2d,       ALdidnt_2d,
	ALdoctor_2d,       ALdog1_2d,       ALdone_2d,          ALdropdyn1_2d,   ALdropdyn2_2d,
	ALdropdynamite_2d, ALdropmat_2d,    ALdumb_2d,          ALdyn1_2d,       ALeatbanana_2d,
	ALempty_2d,        ALexitmaze_2d,   ALfaint_2d,         ALgard1_2d,      ALgarlic_2d,
	ALgatelight_2d,    ALgatescls_2d,   ALgatesopn_2d,      ALgenie_2d,      ALgetbook_2d,
	ALgetdynamite_2d,  ALgetgarlic_2d,  ALgetmatch_2d,      ALgiveb2_2d,     ALgiveb3_2d,
	ALgiveb4_2d,       ALgivebel_2d,    ALglightoff_2d,     ALglighton_2d,   ALgoclosed_2d,
	ALgoopen_2d,       ALgun_2d,        ALharry_2d,         ALhdrink_2d,     ALheroxy01_2d,
	ALhfaint_2d,       ALhole_2d,       ALhprompt_2d,       ALhrgreet_2d,    ALhtable_2d,
	ALhugone_2d,       ALkaboom_2d,     ALkaboom3_2d,       ALkeyhole_2d,    ALkeyhole1_2d,
	ALkeyhole2_2d,     ALlamp_2d,       ALlightdynamite_2d, ALlookkennel_2d, ALlookm1_2d,
	ALlookm2_2d,       ALlookmatch_2d,  ALmaid_2d,          ALmaidbk_2d,     ALmaidp_2d,
	ALmaidx_2d,        ALmap0_2d,       ALmap1_2d,          ALmatok_2d,      ALmissed_2d,
	ALnasty_2d,        ALnobang_2d,     ALnobang2_2d,       ALnobell_2d,     ALnocarry_2d,
	ALnocure_2d,       ALnodrink_2d,    ALnogenie_2d,       ALnopurps_2d,    ALnoreply_2d,
	ALnotrap_2d,       ALomgag_2d,      ALopendoor1_2d,     ALopendoor2_2d,  ALopendoor3_2d,
	ALpanel_2d,        ALparty_2d,      ALpencil_2d,        ALpengone_2d,    ALpenny1_2d,
	ALphone_2d,        ALphonebox_2d,   ALphoto_2d,         ALphoto1_2d,     ALphoto2_2d,
	ALphoto3_2d,       ALpois1_2d,      ALpois2_2d,         ALpois3_2d,      ALpois4_2d,
	ALpushpaper_2d,    ALpushpencil_2d, ALreadlet_2d,       ALrepmsg1_2d,    ALrg_2d,
	ALridgard_2d,      ALridgarl_2d,    ALridkey_2d,        ALrr_2d,         ALrumbling_2d,
	ALsafe_2d,         ALsafe1_2d,      ALsafepr_2d,        ALschedbut_2d,   ALscr01_2d,
	ALscr02_2d,        ALscr0201_2d,    ALscr03_2d,         ALscr0301_2d,    ALscr0305_2d,
	ALscr0306_2d,      ALscr04_2d,      ALscr0503_2d,       ALscr06_2d,      ALscr0603_2d,
	ALscr0607_2d,      ALscr0631_2d,    ALscr0706_2d,       ALscr0708_2d,    ALscr0710_2d,
	ALscr0807_2d,      ALscr0809_2d,    ALscr09_2d,         ALscr0908_2d,    ALscr10_2d,
	ALscr1007_2d,      ALscr1108_2d,    ALscr1113_2d,       ALscr1314_2d,    ALscr14_2d,
	ALscr1413_2d,      ALscr1415_2d,    ALscr15_2d,         ALscr1514_2d,    ALscr1516_2d,
	ALscr1517_2d,      ALscr1615_2d,    ALscr1715_2d,       ALscr1718_2d,    ALscr1720_2d,
	ALscr1817_2d,      ALscr1819c_2d,   ALscr1819l_2d,      ALscr1819r_2d,   ALscr1918c_2d,
	ALscr1918l_2d,     ALscr1918r_2d,   ALscr2017_2d,       ALscr2223_2d,    ALscr2322_2d,
	ALscr2324_2d,      ALscr2325_2d,    ALscr2326_2d,       ALscr2423_2d,    ALscr2523_2d,
	ALscr2623_2d,      ALscr2627_2d,    ALscr2726_2d,       ALscr2827_2d,    ALscr2829_2d,
	ALscr29_2d,        ALscr2928_2d,    ALscr2930_2d,       ALscr2931_2d,    ALscr2934_2d,
	ALscr2938_2d,      ALscr30_2d,      ALscr3029_2d,       ALscr3106_2d,    ALscr3129_2d,
	ALscr3132_2d,      ALscr3231_2d,    ALscr33_2d,         ALscr34_2d,      ALscr3429_2d,
	ALscr3438_2d,      ALscr35_2d,      ALscr3534_2d,       ALscr36_2d,      ALscr3634_2d,
	ALscr3718_2d,      ALscr3829_2d,    ALscr3834_2d,       ALscrgate1_2d,   ALscrgate2_2d,
	ALscrok_2d,        ALshedlight_2d,  ALshot_2d,          ALsilly_2d,      ALslightoff_2d,
	ALslighton_2d,     ALsnake_2d,      ALsong1_2d,         ALsong3_2d,      ALsong4_2d,
	ALsonic_2d,        ALstrike1_2d,    ALstrikematch_2d,   ALswgates_2d,    ALswzapper_2d,
	ALthrown_2d,       ALthrowstick_2d, ALtrap_2d,          ALuptrap_2d,     ALvenus_2d,
	ALwho_2d,          ALwill_2d,       ALwill1_2d,         ALwill2_2d,      ALworkgates_2d,
	ALzapperoff_2d,    ALzapperon_2d,   ALnewscr_2d
};

int oldrsp_3d[] = {kSTOldrsp1_3d, -1};
int tiprsp_3d[] = {kSTTiprsp_3d,  -1};

act0 adarttest_3d =      {ASCHEDULE, 0,  kALdartsched_3d};
act0 arepblink_3d =      {ASCHEDULE, 60, kALeleblink_3d};
act0 arepeathorizon_3d = {ASCHEDULE, 2,  kALhorizon_3d};
act0 arepeatmouse_3d =   {ASCHEDULE, 4,  kALmouse_3d};
act0 arepflash_3d =      {ASCHEDULE, 10, kALflash_3d};

act1 aappear1_3d =     {START_OBJ, 1,   HERO,        0, NOT_CYCLING};
act1 acamp2a_3d =      {START_OBJ, 0,   NAT2_3d,     0, CYCLE_FORWARD};
act1 acamp2b_3d =      {START_OBJ, 0,   NATG_3d,     0, CYCLE_FORWARD};
act1 acamp8a_3d =      {START_OBJ, 34,  NAT2_3d,     0, NOT_CYCLING};
act1 acamp8b_3d =      {START_OBJ, 34,  NATG_3d,     0, NOT_CYCLING};
act1 acrash14_3d =     {START_OBJ, 20,  PENNY_3d,    0, CYCLE_FORWARD};
act1 acrash19_3d =     {START_OBJ, 42,  HERO,        0, CYCLE_FORWARD};
act1 acrash22_3d =     {START_OBJ, 50,  HERO,        0, INVISIBLE};
act1 acrash9_3d =      {START_OBJ, 5,   HERO,        0, NOT_CYCLING};
act1 adead1_3d =       {START_OBJ, 0,   HERO,        0, ALMOST_INVISIBLE};
act1 adead3_3d =       {START_OBJ, 0,   PENNYLIE_3d, 0, NOT_CYCLING};
act1 adropord2_3d =    {START_OBJ, 0,   CHEESE_3d,   0, NOT_CYCLING};
act1 aenter5_3d =      {START_OBJ, 4,   MOUSE_3d,    0, ALMOST_INVISIBLE};
act1 aenter6_3d =      {START_OBJ, 4,   CHEESE_3d,   0, ALMOST_INVISIBLE};
act1 aenter8_3d =      {START_OBJ, 4,   CAGE_3d,     0, CYCLE_FORWARD};
act1 aex4_3d =         {START_OBJ, 0,   GHOST_3d,    0, ALMOST_INVISIBLE};
act1 afind2_3d =       {START_OBJ, 0,   CRYSTAL_3d,  0, NOT_CYCLING};
act1 afindb3_3d =      {START_OBJ, 0,   BOOK_3d,     0, NOT_CYCLING};
act1 aflask5_3d =      {START_OBJ, 0,   HERO,        0, ALMOST_INVISIBLE};
act1 agot10_3d =       {START_OBJ, 60,  DOCTOR_3d,   0, CYCLE_FORWARD};
act1 agot1a_3d =       {START_OBJ, 0,   HERO,        0, CYCLE_FORWARD};
act1 agot1b_3d =       {START_OBJ, 0,   DOCTOR_3d,   0, CYCLE_FORWARD};
act1 ahelp3_3d =       {START_OBJ, 0,   HERO,        0, CYCLE_FORWARD};
act1 amission12_3d =   {START_OBJ, 10,  NATG_3d,     0, NOT_CYCLING};
act1 amission15_3d =   {START_OBJ, 29,  LIPS_3d,     0, ALMOST_INVISIBLE};
act1 amission18_3d =   {START_OBJ, 34,  HERO,        0, NOT_CYCLING};
act1 amission4_3d =    {START_OBJ, 0,   NATG_3d,     0, CYCLE_FORWARD};
act1 amission5_3d =    {START_OBJ, 0,   HERO,        0, CYCLE_FORWARD};
act1 aold1a_3d =       {START_OBJ, 0,   MOUTH_3d,    0, NOT_CYCLING};
act1 aold1b_3d =       {START_OBJ, 20,  MOUTH_3d,    0, CYCLE_FORWARD};
act1 aold6a_3d =       {START_OBJ, 0,   MOUTH_3d,    0, NOT_CYCLING};
act1 aold6g_3d =       {START_OBJ, 22,  HERO,        0, NOT_CYCLING};
act1 aplantfix_3d =    {START_OBJ, 0,   PLANT1_3d,   0, NOT_CYCLING};
act1 aprod1_3d =       {START_OBJ, 0,   DOCTOR_3d,   0, CYCLE_FORWARD};
act1 aprod6_3d =       {START_OBJ, 24,  DOCTOR_3d,   0, CYCLE_FORWARD};
act1 areturn2_3d =     {START_OBJ, 0,   NATG_3d,     0, NOT_CYCLING};
act1 ascare15_3d =     {START_OBJ, 0,   ELEPHANT_3d, 0, CYCLE_FORWARD};
act1 ascare16_3d =     {START_OBJ, 64,  ELEPHANT_3d, 0, ALMOST_INVISIBLE};
act1 ascare4_3d =      {START_OBJ, 0,   E_EYES_3d,   0, ALMOST_INVISIBLE};
act1 ascare6_3d =      {START_OBJ, 0,   MOUSE_3d,    0, CYCLE_FORWARD};
act1 astick4_3d =      {START_OBJ, 16,  DOCTOR_3d,   0, ALMOST_INVISIBLE};
act1 astick6_3d =      {START_OBJ, 17,  DOCLIE_3d,   0, CYCLE_FORWARD};
act1 aswing1_3d =      {START_OBJ, 0,   HERO,        0, ALMOST_INVISIBLE};
act1 aswing3_3d =      {START_OBJ, 1,   HERO,        0, CYCLE_FORWARD};
act1 aswing8_3d =      {START_OBJ, 15,  HERO,        0, NOT_CYCLING};
act1 atakecage1_3d =   {START_OBJ, 0,   CAGE_3d,     0, ALMOST_INVISIBLE};
act1 atakecheese2_3d = {START_OBJ, 0,   CHEESE_3d,   0, ALMOST_INVISIBLE};
act1 avine2_3d =       {START_OBJ, 0,   BLOCK1_3d,   0, ALMOST_INVISIBLE};
act1 avine3_3d =       {START_OBJ, 0,   BLOCK2_3d,   0, ALMOST_INVISIBLE};
act1 avine4_3d =       {START_OBJ, 0,   BLOCK3_3d,   0, ALMOST_INVISIBLE};
act1 aweb19_3d =       {START_OBJ, 110, LIPS_3d,     0, ALMOST_INVISIBLE};
act1 aweb21_3d =       {START_OBJ, 115, PENNY_3d,    0, ALMOST_INVISIBLE};
act1 aweb22_3d =       {START_OBJ, 116, PENNYLIE_3d, 0, NOT_CYCLING};
act1 aweb4_3d =        {START_OBJ, 0,   PENNY_3d,    0, CYCLE_FORWARD};
act1 aweb7_3d =        {START_OBJ, 38,  PENNY_3d,    0, NOT_CYCLING};
act1 awink1_3d =       {START_OBJ, 8,   O_EYE_3d,    1, CYCLE_FORWARD};
act1 awink2_3d =       {START_OBJ, 16,  O_EYE_3d,    1, CYCLE_BACKWARD};
act1 awink3_3d =       {START_OBJ, 19,  O_EYE_3d,    0, ALMOST_INVISIBLE};
act1 aappear_3d =      {START_OBJ, 0,   HERO,        0, NOT_CYCLING};
act1 adisappear_3d =   {START_OBJ, 0,   HERO,        0, ALMOST_INVISIBLE};

act2 acamp0b_3d            = {INIT_OBJXY, 0,  NATG_3d,   128, 101};
act2 acamp3a_3d            = {INIT_OBJXY, 0,  NAT2_3d,   17,  97};
act2 acamp3b_3d            = {INIT_OBJXY, 0,  NATG_3d,   28,  101};
act2 adoc4_3d              = {INIT_OBJXY, 0,  DOCTOR_3d, 70,  110};
act2 adropcheese2_3d       = {INIT_OBJXY, 0,  CHEESE_3d, 158, 142};
act2 aexit1_3d             = {INIT_OBJXY, 0,  HERO,      170, 110};
act2 agot5a_3d             = {INIT_OBJXY, 40, HERO,      239, 104};
act2 ahole4a_3d            = {INIT_OBJXY, 10, MOUSE_3d,  280, 135};
act2 ahole4b_3d            = {INIT_OBJXY, 10, MOUSE_3d,  200, 135};
act2 aleft2_3d             = {INIT_OBJXY, 0,  HERO,      218, 106};
act2 aleft4_3d             = {INIT_OBJXY, 15, HERO,      53,  133};
act2 amission3_3d          = {INIT_OBJXY, 0,  NATG_3d,   30,  120};
act2 aold6f_3d             = {INIT_OBJXY, 0,  HERO,      289, 91};
act2 aplane1_3d            = {INIT_OBJXY, 0,  HERO,      170, 50};
act2 areturn4_3d           = {INIT_OBJXY, 0,  NATG_3d,   85,  115};
act2 aright2_3d            = {INIT_OBJXY, 0,  HERO,      77,  106};
act2 aright4_3d            = {INIT_OBJXY, 15, HERO,      243, 133};
act2 astick5_3d            = {INIT_OBJXY, 17, DOCLIE_3d, 238, 133};
act2 aweb2_3d              = {INIT_OBJXY, 0,  PENNY_3d,  270, 133};
act2 aweb26_3d             = {INIT_OBJXY, 0,  HERO,      174, 107};
act2 axy_brg_clftop_3d     = {INIT_OBJXY, 0,  HERO,      280, 30};
act2 axy_brg_path_3d       = {INIT_OBJXY, 0,  HERO,      16,  91};
act2 axy_camp_hut_3d       = {INIT_OBJXY, 0,  HERO,      27,  133};
act2 axy_camp_village_c_3d = {INIT_OBJXY, 0,  HERO,      100, 143};
act2 axy_camp_village_l_3d = {INIT_OBJXY, 0,  HERO,      45,  145};
act2 axy_cave_turn_3d      = {INIT_OBJXY, 0,  HERO,      22,  136};
act2 axy_cave_wfall_3d     = {INIT_OBJXY, 0,  HERO,      287, 140};
act2 axy_clf_clftop_3d     = {INIT_OBJXY, 0,  HERO,      269, 130};
act2 axy_clf_wfall_3d      = {INIT_OBJXY, 0,  HERO,      28,  140};
act2 axy_clftop_brg_3d     = {INIT_OBJXY, 0,  HERO,      28,  91};
act2 axy_clftop_clf_3d     = {INIT_OBJXY, 0,  HERO,      28,  140};
act2 axy_clftop_slope_3d   = {INIT_OBJXY, 0,  HERO,      28,  120};
act2 axy_crash_web_3d      = {INIT_OBJXY, 0,  HERO,      280, 133};
act2 axy_garden_wbase_3d   = {INIT_OBJXY, 0,  HERO,      24,  114};
act2 axy_hut_camp_3d       = {INIT_OBJXY, 0,  HERO,      280, 121};
act2 axy_hut_in_3d         = {INIT_OBJXY, 0,  HERO,      155, 141};
act2 axy_hut_out_3d        = {INIT_OBJXY, 0,  HERO,      148, 108};
act2 axy_hut_village_c_3d  = {INIT_OBJXY, 0,  HERO,      207, 143};
act2 axy_hut_village_r_3d  = {INIT_OBJXY, 0,  HERO,      207, 143};
act2 axy_path_brg_3d       = {INIT_OBJXY, 0,  HERO,      289, 91};
act2 axy_path_stream_3d    = {INIT_OBJXY, 0,  HERO,      289, 133};
act2 axy_path_village_3d   = {INIT_OBJXY, 0,  HERO,      25,  143};
act2 axy_path_web_3d       = {INIT_OBJXY, 0,  HERO,      25,  140};
act2 axy_slope_clftop_3d   = {INIT_OBJXY, 0,  HERO,      280, 92};
act2 axy_slope_stream_3d   = {INIT_OBJXY, 0,  HERO,      28,  133};
act2 axy_stream_path_3d    = {INIT_OBJXY, 0,  HERO,      27,  140};
act2 axy_stream_slope_3d   = {INIT_OBJXY, 0,  HERO,      275, 90};
act2 axy_turn_cave_3d      = {INIT_OBJXY, 0,  HERO,      272, 140};
act2 axy_turn_village_3d   = {INIT_OBJXY, 0,  HERO,      283, 143};
act2 axy_village_camp_l_3d = {INIT_OBJXY, 0,  HERO,      64,  130};
act2 axy_village_camp_r_3d = {INIT_OBJXY, 0,  HERO,      280, 130};
act2 axy_village_path_3d   = {INIT_OBJXY, 0,  HERO,      280, 140};
act2 axy_village_turn_3d   = {INIT_OBJXY, 0,  HERO,      27,  87};
act2 axy_wbase_garden_3d   = {INIT_OBJXY, 0,  HERO,      272, 133};
act2 axy_wbase_wfall_3d    = {INIT_OBJXY, 0,  HERO,      254, 114};
act2 axy_web_crash_3d      = {INIT_OBJXY, 0,  HERO,      28,  140};
act2 axy_web_path_3d       = {INIT_OBJXY, 0,  HERO,      280, 140};
act2 axy_wfall_cave_3d     = {INIT_OBJXY, 0,  HERO,      27,  140};
act2 axy_wfall_clf_3d      = {INIT_OBJXY, 0,  HERO,      280, 140};
act2 axy_wfallb_wbase_3d   = {INIT_OBJXY, 0,  HERO,      273, 114};

act3 abtipprompt_3d = {PROMPT, 0,  kSTBridgeprompt_3d, tiprsp_3d,   kALnotip_3d, kALbtip_3d,  false};
//HACK: Use YESNO Box instead of PROMPT box as the PROMPT box is not currently implemented
//This allows to test if the game is completable
//act3 aold1c_3d      = {PROMPT, 60, kSTOldman1_3d,      oldrsp_3d,   kALold2_3d,  kALwrong_3d, false};
//act3 aold2_3d       = {PROMPT, 0,  kSTOldman2_3d,      oldrsp_3d,   kALold3_3d,  kALwrong_3d, false};
//act3 aold3_3d       = {PROMPT, 0,  kSTOldman3_3d,      oldrsp_3d,   kALold7_3d,  kALold4_3d,  false};
act43 aold1c_3d     = {YESNO, 60,  kSTOldman1_3d,      kALwrong_3d, kALold2_3d};
act43 aold2_3d      = {YESNO, 0,   kSTOldman2_3d,      kALwrong_3d, kALold3_3d};
act43 aold3_3d      = {YESNO, 0,   kSTOldman3_3d,      kALold4_3d,  kALold7_3d};

act5 acamp4a_3d =    {INIT_OBJVXY, 0,   NAT2_3d,     4,       0};
act5 acamp4b_3d =    {INIT_OBJVXY, 0,   NATG_3d,     3,       0};
act5 acamp6a_3d =    {INIT_OBJVXY, 33,  NAT2_3d,     0,       0};
act5 acamp7a_3d =    {INIT_OBJVXY, 34,  NATG_3d,     0,       0};
act5 aclose1_3d =    {INIT_OBJVXY, 0,   CDOOR_3d,    DX,      0};
act5 aclose2_3d =    {INIT_OBJVXY, 6,   CDOOR_3d,    0,       0};
act5 acrash17_3d =   {INIT_OBJVXY, 23,  PENNY_3d,    -DX,     0};
act5 acrash20_3d =   {INIT_OBJVXY, 42,  HERO,        0,       -1};
act5 acrash21_3d =   {INIT_OBJVXY, 50,  HERO,        0,       0};
act5 acrash4_3d =    {INIT_OBJVXY, 1,   PENNY_3d,    0,       DY};
act5 acrash5_3d =    {INIT_OBJVXY, 1,   HERO,        0,       DY};
act5 acrash6_3d =    {INIT_OBJVXY, 4,   PENNY_3d,    DX,      0};
act5 acrash7_3d =    {INIT_OBJVXY, 4,   HERO,        0,       0};
act5 adead4_3d =     {INIT_OBJVXY, 0,   HERO,        0,       0};
act5 aenter4_3d =    {INIT_OBJVXY, 0,   MOUSE_3d,    -DX,     0};
act5 aex3_3d =       {INIT_OBJVXY, 0,   GHOST_3d,    0,       0};
act5 agot12_3d =     {INIT_OBJVXY, 60,  DOCTOR_3d,   -DX,     0};
act5 agot2b_3d =     {INIT_OBJVXY, 26,  HERO,        0,       0};
act5 agot3b_3d =     {INIT_OBJVXY, 28,  DOCTOR_3d,   0,       -DX};
act5 agot4_3d =      {INIT_OBJVXY, 22,  CDOOR_3d,    -DX,     0};
act5 agot5_3d =      {INIT_OBJVXY, 36,  HERO,        0,       -DX};
act5 agot7_3d =      {INIT_OBJVXY, 36,  CDOOR_3d,    DX,      0};
act5 agot8_3d =      {INIT_OBJVXY, 42,  CDOOR_3d,    0,       0};
act5 ahelp4_3d =     {INIT_OBJVXY, 0,   HERO,        0,       DY};
act5 ahelp6_3d =     {INIT_OBJVXY, 8,   HERO,        -DX * 2, 0};
act5 ahole3a_3d =    {INIT_OBJVXY, 0,   MOUSE_3d,    DX,      0};
act5 ahole3b_3d =    {INIT_OBJVXY, 0,   MOUSE_3d,    -DX,     0};
act5 aleft3_3d =     {INIT_OBJVXY, 0,   HERO,        -DX * 2, 0};
act5 amission11_3d = {INIT_OBJVXY, 10,  NATG_3d,     0,       0};
act5 amission17_3d = {INIT_OBJVXY, 36,  HERO,        0,       0};
act5 amission6_3d =  {INIT_OBJVXY, 0,   NATG_3d,     DX,      -1};
act5 amission7_3d =  {INIT_OBJVXY, 0,   SPIDER_3d,   0,       -1};
act5 amission8_3d =  {INIT_OBJVXY, 0,   HERO,        -DX,     0};
act5 amission9_3d =  {INIT_OBJVXY, 6,   HERO,        -DX,     -1};
act5 aopen1_3d =     {INIT_OBJVXY, 0,   CDOOR_3d,    -DX,     0};
act5 aprod3_3d =     {INIT_OBJVXY, 2,   DOCTOR_3d,   DX * 4,  0};
act5 aprod8_3d =     {INIT_OBJVXY, 26,  DOCTOR_3d,   -DX,     0};
act5 areturn3_3d =   {INIT_OBJVXY, 0,   NATG_3d,     0,  0};
act5 aright3_3d =    {INIT_OBJVXY, 0,   HERO,        DX * 2,  0};
act5 ascare11_3d =   {INIT_OBJVXY, 18,  MOUSE_3d,    DX * 4,  -DY};
act5 ascare14_3d =   {INIT_OBJVXY, 0,   ELEPHANT_3d, -3,      0};
act5 ascare9_3d =    {INIT_OBJVXY, 0,   MOUSE_3d,    -DX * 4, 0};
act5 aswing6_3d =    {INIT_OBJVXY, 15,  HERO,        0,       0};
act5 aweb11_3d =     {INIT_OBJVXY, 50,  SPIDER_3d,   0,       1};
act5 aweb12_3d =     {INIT_OBJVXY, 75,  SPIDER_3d,   0,       -4};
act5 aweb13_3d =     {INIT_OBJVXY, 80,  SPIDER_3d,   0,       2};
act5 aweb14_3d =     {INIT_OBJVXY, 105, SPIDER_3d,   0,       0};
act5 aweb5_3d =      {INIT_OBJVXY, 0,   PENNY_3d,    -DX,     -1};
act5 aweb9_3d =      {INIT_OBJVXY, 40,  PENNY_3d,    0,       0};
act5 astophero_3d =  {INIT_OBJVXY, 0,   HERO,        0,       0};

act6 acheese1_3d     = {INIT_CARRY, 0, CHEESE_3d,   false};
act6 adropord3_3d    = {INIT_CARRY, 0, CHEESE_3d,   false};
act6 agive6_3d       = {INIT_CARRY, 0, BLOWPIPE_3d, true};
act6 agive7_3d       = {INIT_CARRY, 0, BOUILLON_3d, false};
act6 atakecage2_3d   = {INIT_CARRY, 0, CAGE_3d,     true};
act6 atakecb3_3d     = {INIT_CARRY, 0, CRYSTAL_3d,  false};
act6 atakecheese3_3d = {INIT_CARRY, 0, CHEESE_3d,   true};

act7 adead2_3d =    {INIT_HF_COORD, 0, PENNYLIE_3d};
act7 adropord1_3d = {INIT_HF_COORD, 0, CHEESE_3d};
act7 afind1_3d =    {INIT_HF_COORD, 0, CRYSTAL_3d};
act7 afindb2_3d =   {INIT_HF_COORD, 0, BOOK_3d};
act7 ascare5_3d =   {INIT_HF_COORD, 0, MOUSE_3d};

act8 acrash23_3d =            {NEW_SCREEN, 50,  WEB_3d};
act8 aexit3_3d =              {NEW_SCREEN, 0,   CRASH_3d};
act8 aflask6_3d =             {NEW_SCREEN, 0,   SUNSET_3d};
act8 aold6i_3d =              {NEW_SCREEN, 22,  BRIDGE2_3d};
act8 aplane3_3d =             {NEW_SCREEN, 0,   PLANE_3d};
act8 areturn_3d =             {NEW_SCREEN, 1,   CAVE_3d};
act8 ascr_brg_clftop_3d =     {NEW_SCREEN, 0,   CLIFFTOP_3d};
act8 ascr_brg_path_3d =       {NEW_SCREEN, 0,   PATH_UL_3d};
act8 ascr_camp_hut_3d =       {NEW_SCREEN, 0,   HUT_OUT_3d};
act8 ascr_camp_village_c_3d = {NEW_SCREEN, 0,   VILLAGE_3d};
act8 ascr_camp_village_l_3d = {NEW_SCREEN, 0,   VILLAGE_3d};
act8 ascr_cave_man_3d =       {NEW_SCREEN, 0,   OLDMAN_3d};
act8 ascr_cave_turn_3d =      {NEW_SCREEN, 0,   TURN_3d};
act8 ascr_clf_clftop_3d =     {NEW_SCREEN, 0,   CLIFFTOP_3d};
act8 ascr_clf_wfall_3d =      {NEW_SCREEN, 0,   WFALL_3d};
act8 ascr_clf_wnofall_3d =    {NEW_SCREEN, 0,   WFALL_B_3d};
act8 ascr_clftop_brg_3d =     {NEW_SCREEN, 0,   BRIDGE2_3d};
act8 ascr_clftop_clf_3d =     {NEW_SCREEN, 0,   CLIFF_3d};
act8 ascr_clftop_slope_3d =   {NEW_SCREEN, 0,   SLOPE_3d};
act8 ascr_crash_web_3d =      {NEW_SCREEN, 0,   WEB_3d};
act8 ascr_garden_wbase_3d =   {NEW_SCREEN, 0,   WBASE_3d};
act8 ascr_hut_camp_3d =       {NEW_SCREEN, 0,   CAMP_3d};
act8 ascr_hut_in_3d =         {NEW_SCREEN, 0,   HUT_IN_3d};
act8 ascr_hut_out_3d =        {NEW_SCREEN, 0,   HUT_OUT_3d};
act8 ascr_hut_village_c_3d =  {NEW_SCREEN, 0,   VILLAGE_3d};
act8 ascr_hut_village_r_3d =  {NEW_SCREEN, 0,   VILLAGE_3d};
act8 ascr_path_brg1_3d =      {NEW_SCREEN, 0,   BRIDGE_3d};
act8 ascr_path_brg2_3d =      {NEW_SCREEN, 0,   BRIDGE2_3d};
act8 ascr_path_village_3d =   {NEW_SCREEN, 0,   VILLAGE_3d};
act8 ascr_path_web_3d =       {NEW_SCREEN, 0,   WEB_3d};
act8 ascr_slope_clftop_3d =   {NEW_SCREEN, 0,   CLIFFTOP_3d};
act8 ascr_slope_stream1_3d =  {NEW_SCREEN, 0,   STREAM_3d};
act8 ascr_slope_stream2_3d =  {NEW_SCREEN, 0,   STREAM2_3d};
act8 ascr_stream_path_3d =    {NEW_SCREEN, 0,   PATH_3d};
act8 ascr_stream_slope_3d =   {NEW_SCREEN, 0,   SLOPE_3d};
act8 ascr_turn_cave_3d =      {NEW_SCREEN, 0,   CAVE_3d};
act8 ascr_turn_village_3d =   {NEW_SCREEN, 0,   VILLAGE_3d};
act8 ascr_village_camp_l_3d = {NEW_SCREEN, 0,   CAMP_3d};
act8 ascr_village_camp_r_3d = {NEW_SCREEN, 0,   CAMP_3d};
act8 ascr_village_path_3d =   {NEW_SCREEN, 0,   PATH_3d};
act8 ascr_village_turn_3d =   {NEW_SCREEN, 0,   TURN_3d};
act8 ascr_wbase_garden_3d =   {NEW_SCREEN, 0,   GARDEN_3d};
act8 ascr_wbase_wfall_3d =    {NEW_SCREEN, 0,   WFALL_3d};
act8 ascr_web_crash_3d =      {NEW_SCREEN, 0,   CRASH_3d};
act8 ascr_web_path_3d =       {NEW_SCREEN, 0,   PATH_UL_3d};
act8 ascr_wfall_cave_3d =     {NEW_SCREEN, 0,   CAVE_3d};
act8 ascr_wfall_clf_3d =      {NEW_SCREEN, 0,   CLIFF_3d};
act8 ascr_wfallb_wbase_3d =   {NEW_SCREEN, 0,   WBASE_3d};
act8 aweb23_3d =              {NEW_SCREEN, 117, CRASH_3d};

act9 abrg_msg2_3d =    {INIT_OBJSTATE, 0,                  VINE_3d,     1};
act9 acamp0a_3d =      {INIT_OBJSTATE, 4,                  NAT2_3d,     0};
act9 acom0b_3d =       {INIT_OBJSTATE, 0,                  NAT1_3d,     1};
act9 acom1b_3d =       {INIT_OBJSTATE, 0,                  NAT1_3d,     2};
act9 acom2b_3d =       {INIT_OBJSTATE, 0,                  NAT1_3d,     3};
act9 acom3b_3d =       {INIT_OBJSTATE, 0,                  NAT1_3d,     4};
act9 acom4b_3d =       {INIT_OBJSTATE, 0,                  NAT1_3d,     5};
act9 acom5b_3d =       {INIT_OBJSTATE, 0,                  NAT1_3d,     6};
act9 acom6b_3d =       {INIT_OBJSTATE, 0,                  NAT1_3d,     7};
act9 acom7b_3d =       {INIT_OBJSTATE, 0,                  NAT1_3d,     8};
act9 acom8b_3d =       {INIT_OBJSTATE, 0,                  NAT1_3d,     9};
act9 adart1_3d =       {INIT_OBJSTATE, 0,                  BLOWPIPE_3d, 1};
act9 adn_3d =          {INIT_OBJSTATE, 0,                  HERO,        0};
act9 adrink_3d =       {INIT_OBJSTATE, 0,                  FLASK_3d,    0};
act9 adropcheese3_3d = {INIT_OBJSTATE, 0,                  CHEESE_3d,   1};
act9 aelewoken_3d =    {INIT_OBJSTATE, 0,                  ELEPHANT_3d, 0};
act9 aemptyflask_3d =  {INIT_OBJSTATE, 0,                  FLASK_3d,    0};
act9 aendaction_3d =   {INIT_OBJSTATE, DARTTIME + 30,      E_EYES_3d,   0};
act9 aenter1_3d =      {INIT_OBJSTATE, 0,                  MOUSE_3d,    2};
act9 aex7_3d =         {INIT_OBJSTATE, 0,                  GHOST_3d,    1};
act9 afillmagic3_3d =  {INIT_OBJSTATE, 0,                  FLASK_3d,    2};
act9 afillord2_3d =    {INIT_OBJSTATE, 0,                  FLASK_3d,    1};
act9 afindb1_3d =      {INIT_OBJSTATE, 0,                  BOOK_3d,     1};
act9 agive3_3d =       {INIT_OBJSTATE, 0,                  NAT1_3d,     10};
act9 agot1_3d =        {INIT_OBJSTATE, 0,                  DOCTOR_3d,   1};
act9 ahelp2_3d =       {INIT_OBJSTATE, 0,                  HERO,        1};
act9 ahole5a_3d =      {INIT_OBJSTATE, 0,                  MOUSE_3d,    1};
act9 ahole5b_3d =      {INIT_OBJSTATE, 0,                  MOUSE_3d,    0};
act9 amakeclay2_3d =   {INIT_OBJSTATE, 0,                  CLAY_3d,     1};
act9 amission1_3d =    {INIT_OBJSTATE, 0,                  PENNY_3d,    2};
act9 amousefree_3d =   {INIT_OBJSTATE, 0,                  MOUSE_3d,    3};
act9 aoldstate_3d =    {INIT_OBJSTATE, 0,                  MOUTH_3d,    1};
act9 aopen2_3d =       {INIT_OBJSTATE, 0,                  CDOOR_3d,    1};
act9 apause0_3d =      {INIT_OBJSTATE, 3 * NORMAL_TPS_v2d, NAT2_3d,     0};
act9 apause1_3d =      {INIT_OBJSTATE, 0,                  NAT2_3d,     1};
act9 astartaction_3d = {INIT_OBJSTATE, 0,                  E_EYES_3d,   1};
act9 astick3_3d =      {INIT_OBJSTATE, 0,                  DOCTOR_3d,   2};
act9 atakecheese1_3d = {INIT_OBJSTATE, 0,                  CHEESE_3d,   0};
act9 aup_3d =          {INIT_OBJSTATE, 0,                  HERO,        1};
act9 avine6_3d =       {INIT_OBJSTATE, 0,                  BLOCK1_3d,   1};
act9 aweb1_3d =        {INIT_OBJSTATE, 0,                  PENNY_3d,    1};

act10 acamp1a_3d =  {INIT_PATH, 0,  NAT2_3d,   AUTO,    0,      0};
act10 acamp1b_3d =  {INIT_PATH, 0,  NATG_3d,   AUTO,    0,      0};
act10 acamp9a_3d =  {INIT_PATH, 60, NATG_3d,   CHASE,   DX / 2, DY / 2};
act10 acamp9b_3d =  {INIT_PATH, 55, NAT2_3d,   WANDER,  DX,     2};
act10 achase1_3d =  {INIT_PATH, 0,  NATG_3d,   AUTO,    0,      0};
act10 achase2_3d =  {INIT_PATH, 8,  NATG_3d,   CHASE,   DX / 2, DY / 2};
act10 adoc2_3d =    {INIT_PATH, 0,  DOCTOR_3d, CHASE,   DX,     DY};
act10 aenter3_3d =  {INIT_PATH, 0,  MOUSE_3d,  AUTO,    0,      0};
act10 aex2_3d =     {INIT_PATH, 0,  GHOST_3d,  AUTO,    0,      0};
act10 agot6_3d =    {INIT_PATH, 40, HERO,      USER,    0,      0};
act10 ahole1_3d =   {INIT_PATH, 0,  MOUSE_3d,  AUTO,    0,      0};
act10 ahole6_3d =   {INIT_PATH, 30, MOUSE_3d,  WANDER2, DX,     0};
act10 aold6h_3d =   {INIT_PATH, 22, HERO,      USER,    0,      0};
act10 areturn1_3d = {INIT_PATH, 0,  NATG_3d,   AUTO,    0,      0};
act10 ascare12_3d = {INIT_PATH, 34, MOUSE_3d,  WANDER2, DX * 4, DY};
act10 ascare7_3d =  {INIT_PATH, 0,  MOUSE_3d,  AUTO,    0,      0};
act10 aswing5_3d =  {INIT_PATH, 20, HERO,      USER,    0,      0};
act10 adisable_3d = {INIT_PATH, 0,  HERO,      AUTO,    0,      0};
act10 aenable_3d =  {INIT_PATH, 0,  HERO,      USER,    0,      0};
act10 aquiet_3d =   {INIT_PATH, 0,  HERO,      QUIET,   0,      0};

act11 aactiontest1_3d = {COND_R, 0,                   E_EYES_3d,   1,  0,                    kALoktoleave1_3d};
act11 aactiontest2_3d = {COND_R, 0,                   E_EYES_3d,   1,  0,                    kALoktoleave2_3d};
act11 ablktest_3d =     {COND_R, 0,                   BLOCK1_3d,   0,  kALblk1_3d,           0};
act11 abrgmsgtest_3d =  {COND_R, 0,                   VINE_3d,     0,  kALbrg_clftop_msg_3d, kALbrg_clftop1_3d};
act11 abrgtest_3d =     {COND_R, 0,                   VINE_3d,     0,  kALbrg_ok_3d,         kALbrg_down_3d};
act11 acagetest_3d =    {COND_R, 0,                   MOUSE_3d,    2,  0,                    kALpostest_3d};
act11 acagetest1_3d =   {COND_R, 0,                   MOUSE_3d,    2,  kALcagetest2_3d,      kALok_3d};
act11 acamptest_3d =    {COND_R, 0,                   NAT1_3d,     0,  kALcampers_3d,        kALchase_3d};
act11 acavetest_3d =    {COND_R, 0,                   GHOST_3d,    0,  kALspirit_3d,         kALcave_man_3d};
act11 acrashtest1_3d =  {COND_R, 0,                   PENNY_3d,    0,  kALcrashed_3d,        kALcrashtest2_3d};
act11 acrashtest2_3d =  {COND_R, 0,                   PENNY_3d,    1,  kALcryhelp_3d,        0};
act11 adartedtest_3d =  {COND_R, 0,                   BLOWPIPE_3d, 0,  kALdodart_3d,         kALdarted_3d};
act11 adoctest_3d =     {COND_R, 0,                   DOCTOR_3d,   0,  kALdoc_3d,            0};
act11 adrinktest_3d =   {COND_R, 0,                   FLASK_3d,    0,  kALdrinkno_3d,        kALdrinkyes_3d};
act11 aeletest1_3d =    {COND_R, 0,                   ELEPHANT_3d, 1,  kALele_sleep_3d,      kALeletest2_3d};
act11 aeletest2_3d =    {COND_R, 0,                   ELEPHANT_3d, 0,  kALeleblink_3d,       0};
act11 aemptytest1_3d =  {COND_R, 0,                   FLASK_3d,    2,  kALemptymagic_3d,     kALemptytest2_3d};
act11 aemptytest2_3d =  {COND_R, 0,                   FLASK_3d,    1,  kALemptyord_3d,       kALdrinkno_3d};
act11 aentertest1_3d =  {COND_R, 0,                   CHEESE_3d,   1,  kALentertest2_3d,     0};
act11 aentertest2_3d =  {COND_R, 0,                   MOUSE_3d,    0,  kALentertest3_3d,     0};
act11 aexotest1_3d =    {COND_R, 0,                   GHOST_3d,    0,  kALexotest2_3d,       kALexordone_3d};
act11 afindbtest_3d =   {COND_R, 0,                   BOOK_3d,     0,  kALfindit_3d,         0};
act11 aflasktest2_3d =  {COND_R, 0,                   FLASK_3d,    2,  kALremedy_3d,         kALflasktest3_3d};
act11 aflasktest3_3d =  {COND_R, 0,                   FLASK_3d,    1,  kALnoremedy_3d,       kALrefuseflask_3d};
act11 agettest2_3d =    {COND_R, 0,                   DOCTOR_3d,   0,  kALgot_3d,            0};
act11 agivetest_3d =    {COND_R, 0,                   NAT1_3d,     10, kALrefuse_3d,         kALgive_3d};
act11 ahoriz1_3d =      {COND_R, 0,                   HERO,        0,  kALup_3d,             0};
act11 ahoriz2_3d =      {COND_R, 0,                   HERO,        1,  kALdn_3d,             0};
act11 anat0_3d =        {COND_R, 0,                   NAT1_3d,     0,  kALcom0_3d,           kALnat1_3d};
act11 anat1_3d =        {COND_R, 0,                   NAT1_3d,     1,  kALcom1_3d,           kALnat2_3d};
act11 anat2_3d =        {COND_R, 0,                   NAT1_3d,     2,  kALcom2_3d,           kALnat3_3d};
act11 anat3_3d =        {COND_R, 0,                   NAT1_3d,     3,  kALcom3_3d,           kALnat4_3d};
act11 anat4_3d =        {COND_R, 0,                   NAT1_3d,     4,  kALcom4_3d,           kALnat5_3d};
act11 anat5_3d =        {COND_R, 0,                   NAT1_3d,     5,  kALcom5_3d,           kALnat6_3d};
act11 anat6_3d =        {COND_R, 0,                   NAT1_3d,     6,  kALcom6_3d,           kALnat7_3d};
act11 anat7_3d =        {COND_R, 0,                   NAT1_3d,     7,  kALcom7_3d,           kALnat8_3d};
act11 anat8_3d =        {COND_R, 0,                   NAT1_3d,     8,  kALcom8_3d,           0};
act11 aold5_3d =        {COND_R, 0,                   FLASK_3d,    2,  kALold6_3d,           kALwrong_3d};
act11 aoldmantest_3d =  {COND_R, 0,                   MOUTH_3d,    0,  kALoldfirst_3d,       kALoldsubseq_3d};
act11 aopentest_3d =    {COND_R, 0,                   DOCTOR_3d,   1,  kALprod_3d,           kALopencdoor_3d};
act11 apausetest_3d =   {COND_R, 0,                   NAT2_3d,     1,  0,                 kALcomment_3d};
act11 apostest_3d =     {COND_R, 0,                   MOUSE_3d,    0,  kALmousel_3d,         kALmouser_3d};
act11 aspirittest_3d =  {COND_R, 0,                   GHOST_3d,    0,  kALwarn_3d,           0};
act11 asticktest1_3d =  {COND_R, 0,                   DOCTOR_3d,   2,  kALstuckpin_3d,       kALsticktest2_3d};
act11 asticktest2_3d =  {COND_R, 0,                   CLAY_3d,     1,  kALstickpin_3d,       kALnostickpin_3d};
act11 ataketest1_3d =   {COND_R, 0,                   MOUSE_3d,    2,  kALtakeit_3d,         kALcanttake_3d};
act11 ataketest2_3d =   {COND_R, 0,                   MOUSE_3d,    3,  kALmousegone_3d,      kALtaketest1_3d};
act11 atalktest1_3d =   {COND_R, 0,                   NAT1_3d,     9,  kALnat9_3d,           kALnative_3d};
act11 atiptest_3d =     {COND_R, 60 * NORMAL_TPS_v2d, BLOCK1_3d,   0,  kALbtipprompt_3d,     0};
act11 auntietest_3d =   {COND_R, 0,                   BLOCK1_3d,   0,  kALnottied_3d,        kALuntie_3d};
act11 avinetest_3d =    {COND_R, 0,                   BLOCK1_3d,   0,  kALtievine_3d,        kALtied_3d};
act11 awebtest1_3d =    {COND_R, 0,                   PENNY_3d,    0,  kALspider_3d,         kALwebtest2_3d};
act11 awebtest2_3d =    {COND_R, 0,                   PENNY_3d,    1,  kALmission_3d,        kALreturn_3d};

act12 ablk1_3d =         {TEXT, 0,   kSTBlk1_3d};
act12 abook1_3d =        {TEXT, 0,   kSTBook1_3d};
act12 abtip_3d =         {TEXT, 0,   kSTBridgetip_3d};
act12 acanttake_3d =     {TEXT, 0,   kSTCanttake_3d};
act12 acheese2_3d =      {TEXT, 0,   kSTYummy_3d};
act12 acubestip_3d =     {TEXT, 0,   kSTCubestip_3d};
act12 adammedtip_3d =    {TEXT, 0,   kSTDammedtip_3d};
act12 adarted_3d =       {TEXT, 0,   kSTDarted_3d};
act12 adrinkno_3d =      {TEXT, 0,   kSTDrinkno_3d};
act12 adrinkyes_3d =     {TEXT, 0,   kSTDrinkyes_3d};
act12 aemptymagic_3d =   {TEXT, 0,   kSTEmptymagic_3d};
act12 aemptyord_3d =     {TEXT, 0,   kSTEmptyord_3d};
act12 aex5_3d =          {TEXT, 0,   kSTExor1_3d};
act12 aex6_3d =          {TEXT, 0,   kSTExor2_3d};
act12 aexordone_3d =     {TEXT, 0,   kSTExordone_3d};
act12 afillmagic2_3d =   {TEXT, 0,   kSTFillmagic_3d};
act12 afillord1_3d =     {TEXT, 0,   kSTFillord_3d};
act12 afindb4_3d =       {TEXT, 2,   kSTFoundbook_3d};
act12 ago1_3d =          {TEXT, 0,   kSTMousefree_3d};
act12 alookfall_3d =     {TEXT, 0,   kSTLookwfall1_3d};
act12 alooknofall_3d =   {TEXT, 0,   kSTLookwfall2_3d};
act12 amagictip_3d =     {TEXT, 0,   kSTMagictip_3d};
act12 amakeclay3_3d =    {TEXT, 0,   kSTMakeeffigy_3d};
act12 amodeltip_3d =     {TEXT, 0,   kSTModeltip_3d};
act12 amousegone_3d =    {TEXT, 0,   kSTMousegone_3d};
act12 amousetip_3d =     {TEXT, 0,   kSTMousetip_3d};
act12 anoblow_3d =       {TEXT, 0,   kSTNoblow_3d};
act12 anoclay_3d =       {TEXT, 0,   kSTNoclay_3d};
act12 anofill_3d =       {TEXT, 0,   kSTNofill_3d};
act12 anomake_3d =       {TEXT, 0,   kSTNomake_3d};
act12 anoremedy1_3d =    {TEXT, 0,   kSTNoremedy_3d};
act12 anospell_3d =      {TEXT, 0,   kSTNospell_3d};
act12 anostick_3d =      {TEXT, 0,   kSTNostick_3d};
act12 anostickpin_3d =   {TEXT, 0,   kSTNostickpin_3d};
act12 anotakecb_3d =     {TEXT, 0,   kSTOldmannotake_3d};
act12 anotip_3d =        {TEXT, 0,   kSTNotip_3d};
act12 anottied_3d =      {TEXT, 0,   kSTNottied_3d};
act12 aold7_3d =         {TEXT, 0,   kSTAllwrong_3d};
act12 aoldmantip_3d =    {TEXT, 0,   kSTOldmantip_3d};
act12 aplanetip_3d =     {TEXT, 0,   kSTPlanetip_3d};
act12 aputitdown_3d =    {TEXT, 0,   kSTPutitdown_3d};
act12 arefuse_3d =       {TEXT, 0,   kSTRefuse_3d};
act12 arefuseflask_3d =  {TEXT, 0,   kSTRefuseflask_3d};
act12 aremedytip_3d =    {TEXT, 0,   kSTRemedytip_3d};
act12 arub_3d =          {TEXT, 0,   kSTRubcrystal_3d};
act12 astick2_3d =       {TEXT, 0,   kSTStickpin_3d};
act12 asticktip_3d =     {TEXT, 0,   kSTSticktip_3d};
act12 astuckpin_3d =     {TEXT, 0,   kSTStuckpin_3d};
act12 aswingtip_3d =     {TEXT, 0,   kSTSwingtip_3d};
act12 atakecb2_3d =      {TEXT, 0,   kSTOldmantakeball_3d};
act12 atalkweb_3d =      {TEXT, 0,   kSTTalkweb_3d};
act12 athing_3d =        {TEXT, 0,   kSTVillagething_3d};
act12 atied_3d =         {TEXT, 0,   kSTTiedvine_3d};
act12 auntie_3d =        {TEXT, 0,   kSTUntievine_3d};
act12 avine5_3d =        {TEXT, 0,   kSTBlk2_3d};
act12 awarn_3d =         {TEXT, 0,   kSTCavewarn_3d};
act12 awaterfalling_3d = {TEXT, 0,   kSTWaterfalling_3d};
act12 awrong1_3d =       {TEXT, 0,   kSTWrong_3d};
act12 aclick_3d =        {TEXT, 0,   kSTClick_3d};
act12 aempty_3d =        {TEXT, 0,   kSTEmpty1_3d};
act12 agotit_3d =        {TEXT, 0,   kSTGotit_3d};
act12 anocarry_3d =      {TEXT, 0,   kSTNocarry_3d};
act12 anopurps_3d =      {TEXT, 0,   kSTNopurps_3d};
act12 anothanks_3d =     {TEXT, 0,   kSTNothanks_3d};
act12 aok_3d =           {TEXT, 0,   kSTOkgen_3d};
act12 astalk_3d =        {TEXT, 0,   kSTStalk_3d};

act13 acrash1_3d =  {SWAP_IMAGES, 0,  HERO, HERO_OLD_3d};
act13 aswing2_3d =  {SWAP_IMAGES, 2,  HERO, SWINGER_3d};
act13 aswing7_3d =  {SWAP_IMAGES, 15, HERO, SWINGER_3d};
act13 aweb24_3d =   {SWAP_IMAGES, 0,  HERO, HERO_OLD_3d};
act13 aweb25_3d =   {SWAP_IMAGES, 0,  HERO, WHERO_3d};
act13 aweehero_3d = {SWAP_IMAGES, 0,  HERO, WHERO_3d};

act14 acagetest3_3d =  {COND_SCR, 0, CAGE_3d,   PATH_3d,   kALcagetest4_3d,  kALmousego_3d};
act14 adroptest1_3d =  {COND_SCR, 0, HERO,      HUT_IN_3d, kALdropincage_3d, kALnocarry_3d};
act14 aexotest2_3d =   {COND_SCR, 0, HERO,      CAVE_3d,   kALexor_3d,       kALnospell_3d};
act14 afilltest1_3d =  {COND_SCR, 0, HERO,      GARDEN_3d, kALfillmagic_3d,  kALfilltest2_3d};
act14 afilltest2_3d =  {COND_SCR, 0, HERO,      STREAM_3d, kALfillord_3d,    kALfilltest3_3d};
act14 afilltest3_3d =  {COND_SCR, 0, HERO,      WFALL_3d,  kALfillord_3d,    kALnofill_3d};
act14 aflasktest1_3d = {COND_SCR, 0, HERO,      WEB_3d,    kALflasktest2_3d, kALnothanks_3d};
act14 agettest1_3d =   {COND_SCR, 0, DOCTOR_3d, HUT_IN_3d, kALgettest2_3d,   0};
act14 agivetest1_3d =  {COND_SCR, 0, HERO,      CAMP_3d,   kALgivetest_3d,   kALnothanks2_3d};
act14 amaketest_3d =   {COND_SCR, 0, HERO,      HUT_IN_3d, kALmakeit_3d,     kALnomake_3d};
act14 apath2test_3d =  {COND_SCR, 0, HERO,      PATH_3d,   kALdartedtest_3d, kALnoblow_3d};
act14 atalktest2_3d =  {COND_SCR, 0, HERO,      CAMP_3d,   kALtalktest1_3d,  kALstalk_3d};
act14 atalktest3_3d =  {COND_SCR, 0, HERO,      WEB_3d,    kALtalkweb_3d,    kALtalktest2_3d};

act15 agot2_3d =  {AUTOPILOT, 3,  HERO,      CDOOR_3d, DX, DY};
act15 agot2a_3d = {AUTOPILOT, 22, HERO,      CDOOR_3d, DX, DY};
act15 agot3_3d =  {AUTOPILOT, 0,  DOCTOR_3d, CDOOR_3d, DX, DY};
act15 agot3a_3d = {AUTOPILOT, 20, DOCTOR_3d, CDOOR_3d, DX, DY};

act16 acamp5a_3d =    {INIT_OBJ_SEQ, 1,            NAT2_3d,     RIGHT};
act16 acamp5b_3d =    {INIT_OBJ_SEQ, 1,            NATG_3d,     RIGHT};
act16 acamp6b_3d =    {INIT_OBJ_SEQ, 36,           NAT2_3d,     DOWN};
act16 acamp7b_3d =    {INIT_OBJ_SEQ, 40,           NATG_3d,     2};
act16 acrash10_3d =   {INIT_OBJ_SEQ, 8,            HERO,        LEFT};
act16 acrash15_3d =   {INIT_OBJ_SEQ, 21,           PENNY_3d,    DOWN};
act16 acrash16_3d =   {INIT_OBJ_SEQ, 22,           PENNY_3d,    LEFT};
act16 acrash18_3d =   {INIT_OBJ_SEQ, 40,           HERO,        __UP};
act16 acrash2_3d =    {INIT_OBJ_SEQ, 1,            PENNY_3d,    DOWN};
act16 acrash3_3d =    {INIT_OBJ_SEQ, 1,            HERO,        DOWN};
act16 acrash8_3d =    {INIT_OBJ_SEQ, 4,            PENNY_3d,    RIGHT};
act16 adart6_3d =     {INIT_OBJ_SEQ, DARTTIME - 1, E_EYES_3d,   1};
act16 adoc1_3d =      {INIT_OBJ_SEQ, 0,            HERO,        __UP};
act16 aeleblink1_3d = {INIT_OBJ_SEQ, 41,           E_EYES_3d,   1};
act16 aeleblink2_3d = {INIT_OBJ_SEQ, 42,           E_EYES_3d,   0};
act16 aeleblink3_3d = {INIT_OBJ_SEQ, 43,           E_EYES_3d,   1};
act16 aeleblink4_3d = {INIT_OBJ_SEQ, 44,           E_EYES_3d,   0};
act16 aenter7_3d =    {INIT_OBJ_SEQ, 4,            CAGE_3d,     1};
act16 agot11_3d =     {INIT_OBJ_SEQ, 58,           DOCTOR_3d,   LEFT};
act16 ahelp5_3d =     {INIT_OBJ_SEQ, 8,            HERO,        LEFT};
act16 ahole2a_3d =    {INIT_OBJ_SEQ, 0,            MOUSE_3d,    0};
act16 ahole2b_3d =    {INIT_OBJ_SEQ, 0,            MOUSE_3d,    1};
act16 aleft1_3d =     {INIT_OBJ_SEQ, 0,            HERO,        LEFT};
act16 amission13_3d = {INIT_OBJ_SEQ, 10,           NATG_3d,     DOWN};
act16 amission19_3d = {INIT_OBJ_SEQ, 48,           NATG_3d,     RIGHT};
act16 aprod2_3d =     {INIT_OBJ_SEQ, 1,            DOCTOR_3d,   RIGHT};
act16 aprod7_3d =     {INIT_OBJ_SEQ, 25,           DOCTOR_3d,   LEFT};
act16 aright1_3d =    {INIT_OBJ_SEQ, 0,            HERO,        RIGHT};
act16 ascare10_3d =   {INIT_OBJ_SEQ, 18,           MOUSE_3d,    0};
act16 ascare2_3d =    {INIT_OBJ_SEQ, 0,            CAGE_3d,     0};
act16 ascare3_3d =    {INIT_OBJ_SEQ, 0,            ELEPHANT_3d, 1};
act16 ascare8_3d =    {INIT_OBJ_SEQ, 0,            MOUSE_3d,    1};
act16 aweb16_3d =     {INIT_OBJ_SEQ, 105,          PENNY_3d,    RIGHT};
act16 aweb27_3d =     {INIT_OBJ_SEQ, 0,            HERO,        DOWN};
act16 aweb8_3d =      {INIT_OBJ_SEQ, 41,           PENNY_3d,    DOWN};

act17 adart3_3d =  {SET_STATE_BITS, DARTTIME, ELEPHANT_3d, 1};
act17 ascare1_3d = {SET_STATE_BITS, 0,        ELEPHANT_3d, 2};

act19 abittest_3d =       {TEST_STATE_BITS, 0,        ELEPHANT_3d, 1, kALsleepy_3d,      kALscared_3d};
act19 acagetest4_3d =     {TEST_STATE_BITS, 0,        ELEPHANT_3d, 1, kALasleep_3d,      kALscare_3d};
act19 adarttest1_3d =     {TEST_STATE_BITS, DARTTIME, ELEPHANT_3d, 3, kALdammed_3d,      kALbittest_3d};
act19 alookwfalltest_3d = {TEST_STATE_BITS, 0,        ELEPHANT_3d, 3, kALlooknofall_3d,  kALlookfall_3d};
act19 astreamtest_3d =    {TEST_STATE_BITS, 0,        ELEPHANT_3d, 3, kALstream2_3d,     kALstream1_3d};
act19 awfalltest_3d =     {TEST_STATE_BITS, 0,        ELEPHANT_3d, 3, kALwaternofall_3d, kALwaterfall_3d};

act20 adart4_3d =   {DEL_EVENTS, DARTTIME, ASCHEDULE};
act20 adart5_3d =   {DEL_EVENTS, DARTTIME, INIT_OBJ_SEQ};
act20 aold6e_3d =   {DEL_EVENTS, 0,        ASCHEDULE};
act20 aridtest_3d = {DEL_EVENTS, 0,        TEST_STATE_BITS};

act21 adead5_3d =   {GAMEOVER, 0};

act23 asunset3_3d = {EXIT, 50};

act24 adammed1_3d =     {BONUS, 0,        11};
act24 adart2_3d =       {BONUS, DARTTIME, 10};
act24 adropcheese1_3d = {BONUS, 0,        6};
act24 aenter0_3d =      {BONUS, 0,        7};
act24 aex1_3d =         {BONUS, 0,        15};
act24 afillmagic1_3d =  {BONUS, 0,        8};
act24 aflask1_3d =      {BONUS, 0,        1};
act24 agive2_3d =       {BONUS, 0,        14};
act24 amakeclay1_3d =   {BONUS, 0,        4};
act24 aold6b_3d =       {BONUS, 0,        12};
act24 ascarebonus_3d =  {BONUS, 0,        9};
act24 astick1_3d =      {BONUS, 0,        5};
act24 aswing0_3d =      {BONUS, 0,        3};
act24 atakecb1_3d =     {BONUS, 0,        16};
act24 atakencheese_3d = {BONUS, 0,        13};
act24 avine1_3d =       {BONUS, 0,        2};

act25 ahorizpos_3d =   {COND_BOX, 0, HERO,     0,   0,   320, 150, kALhorizup_3d, kALhorizdn_3d};
act25 aentertest3_3d = {COND_BOX, 1, MOUSE_3d, 156, 133, 163, 148, kALtrapped_3d, 0};
act25 amousel_3d =     {COND_BOX, 0, HERO,     0,   0,   254, 199, kALholel_3d,   0};
act25 amouser_3d =     {COND_BOX, 0, HERO,     255, 0,   319, 199, kALholer_3d,   0};
act25 aswing4_3d =     {COND_BOX, 1, HERO,     0,   0,   160, 200, kALright_3d,   kALleft_3d};

act27 aaddcheese_3d =  {ADD_SCORE, 0, CHEESE_3d};
act27 adammed2_3d =    {ADD_SCORE, 0, CAGE_3d};
act27 agive1_3d =      {ADD_SCORE, 0, BLOWPIPE_3d};
act27 atakecage3_3d =  {ADD_SCORE, 0, CAGE_3d};

act28 asubcheese_3d =  {SUB_SCORE, 0, CHEESE_3d};

act29 acagetest2_3d =   {COND_CARRY, 0, CAGE_3d,    kALputitdown_3d,  kALcagetest3_3d};
act29 acbtest_3d =      {COND_CARRY, 0, CRYSTAL_3d, kALtakecb_3d,     kALnotakecb_3d};
act29 adroptest3_3d =   {COND_CARRY, 0, CHEESE_3d,  kALdroptest2_3d,  kALnocarry_3d};
act29 aold4_3d =        {COND_CARRY, 0, FLASK_3d,   kALold5_3d,       kALwrong_3d};
act29 areadtest1_3d =   {COND_CARRY, 0, BELL_3d,    kALreadtest2_3d,  kALreadord_3d};
act29 areadtest2_3d =   {COND_CARRY, 0, CANDLE_3d,  kALexorcise_3d,   kALreadord_3d};
act29 asticktest4_3d =  {COND_CARRY, 0, CLAY_3d,    kALsticktest1_3d, kALnoclay_3d};
act29 atakechstest_3d = {COND_CARRY, 0, CHEESE_3d,  0,             kALtakechs_3d};
act29 ataketest3_3d =   {COND_CARRY, 0, CAGE_3d,    0,             kALtaketest2_3d};

act33 acamp0c_3d =   {INIT_SCREEN, 0, NATG_3d,   CAMP_3d};
act33 adoc3_3d =     {INIT_SCREEN, 0, DOCTOR_3d, HUT_IN_3d};
act33 amission2_3d = {INIT_SCREEN, 0, NATG_3d,   WEB_3d};
act33 areturn5_3d =  {INIT_SCREEN, 0, NATG_3d,   WEB_3d};
act33 aweb3_3d =     {INIT_SCREEN, 0, PENNY_3d,  WEB_3d};

act35 amap0_3d =  {REMAPPAL, 0, _TRED,          _TLIGHTYELLOW};
act35 amap1_3d =  {REMAPPAL, 0, _TLIGHTMAGENTA, _TBLACK};
act35 amap4a_3d = {REMAPPAL, 0, _TGRAY,         _TBLACK};
act35 amap4b_3d = {REMAPPAL, 1, _TGRAY,         _TGRAY};
act35 amap4c_3d = {REMAPPAL, 2, _TGRAY,         _TBLACK};
act35 amap4d_3d = {REMAPPAL, 3, _TGRAY,         _TGRAY};

act36 adroptest2_3d =  {COND_NOUN, 0, kNCage_3d, kALcagetest_3d,   kALdropord_3d};
act36 asticktest3_3d = {COND_NOUN, 0, kNClay_3d, kALsticktest4_3d, kALnostick_3d};

act37 aex8_3d =     {SCREEN_STATE, 0,  CAVE_3d,   1};
act37 ascare17_3d = {SCREEN_STATE, 64, PATH_3d,   1};
act37 astick7_3d =  {SCREEN_STATE, 17, HUT_IN_3d, 1};

act38 amission14_3d = {INIT_LIPS, 20,  LIPS_3d, NATG_3d,  4,     LIPDY + 1};
act38 aweb18_3d =     {INIT_LIPS, 108, LIPS_3d, PENNY_3d, LIPDX, LIPDY};
act38 alips_3d =      {INIT_LIPS, 0,   LIPS_3d, PENNY_3d, LIPDX, LIPDY};

act39 amission23_3d =  {INIT_STORY_MODE, 50, false};
act39 astory_mode_3d = {INIT_STORY_MODE, 0,  true};

// all the act40 were previously defined as act12 with a type set to WARN
act40 aasleep_3d =    {WARN, 30,  kSTAsleep_3d};
act40 abrg_msg1_3d =  {WARN, 0,   kSTBridgedown_3d};
act40 acom0a_3d =     {WARN, 0,   kSTCom0_3d};
act40 acom1a_3d =     {WARN, 0,   kSTCom1_3d};
act40 acom2a_3d =     {WARN, 0,   kSTCom2_3d};
act40 acom3a_3d =     {WARN, 0,   kSTCom3_3d};
act40 acom4a_3d =     {WARN, 0,   kSTCom4_3d};
act40 acom5a_3d =     {WARN, 0,   kSTCom5_3d};
act40 acom6a_3d =     {WARN, 0,   kSTCom6_3d};
act40 acom7a_3d =     {WARN, 0,   kSTCom7_3d};
act40 acom8a_3d =     {WARN, 0,   kSTCom8_3d};
act40 acom9_3d =      {WARN, 0,   kSTCom9_3d};
act40 acrash11_3d =   {WARN, 20,  kSTPenny1_3d};
act40 acrash12_3d =   {WARN, 20,  kSTPenny2_3d};
act40 acrash13_3d =   {WARN, 34,  kSTPenny3_3d};
act40 adammed3_3d =   {WARN, 0,   kSTDammed_3d};
act40 aelewaking_3d = {WARN, 0,   kSTElewaking_3d};
act40 aenter2_3d =    {WARN, 0,   kSTMouse1_3d};
act40 aflask2_3d =    {WARN, 0,   kSTEnd1_3d};
act40 aflask3_3d =    {WARN, 0,   kSTEnd2_3d};
act40 aflask4_3d =    {WARN, 0,   kSTEnd3_3d};
act40 agive4_3d =     {WARN, 0,   kSTGiveb1_3d};
act40 agive5_3d =     {WARN, 0,   kSTGiveb2_3d};
act40 agot1c_3d =     {WARN, 1,   kSTGot1_3d};
act40 agot9_3d =      {WARN, 54,  kSTGot2_3d};
act40 ahelp1_3d =     {WARN, 2,   kSTHelp1_3d};
act40 amission10_3d = {WARN, 5,   kSTMission1_3d};
act40 amission16_3d = {WARN, 30,  kSTMission2_3d};
act40 amission20_3d = {WARN, 50,  kSTMission3_3d};
act40 amission21_3d = {WARN, 50,  kSTMission4_3d};
act40 amission22_3d = {WARN, 50,  kSTMission5_3d};
act40 amission24_3d = {WARN, 60,  kSTMission6_3d};
act40 aold0a_3d =     {WARN, 40,  kSTOldman0a_3d};
act40 aold0b_3d =     {WARN, 40,  kSTOldman0b_3d};
act40 aold6c_3d =     {WARN, 0,   kSTOldman4_3d};
act40 aold6d_3d =     {WARN, 0,   kSTOldman5_3d};
act40 aprod4_3d =     {WARN, 8,   kSTProd1_3d};
act40 aprod5_3d =     {WARN, 20,  kSTProd2_3d};
act40 ascare13_3d =   {WARN, 12,  kSTScare1_3d};
act40 ascared_3d =    {WARN, 0,   kSTScared_3d};
act40 asleepy_3d =    {WARN, 0,   kSTSleepy_3d};
act40 asunset1_3d =   {WARN, 20,  kSTAdios1_3d};
act40 asunset2_3d =   {WARN, 40,  kSTAdios2_3d};
act40 aweb10_3d =     {WARN, 50,  kSTPenny5_3d};
act40 aweb15_3d =     {WARN, 105, kSTSpider1_3d};
act40 aweb17_3d =     {WARN, 108, kSTSpider2_3d};
act40 aweb20_3d =     {WARN, 110, kSTSpider3_3d};
act40 aweb6_3d =      {WARN, 25,  kSTPenny4_3d};

act41 ac1_3d = {COND_BONUS, 0, 3,  kALac2_3d,       kALswingtip_3d};
act41 ac2_3d = {COND_BONUS, 0, 13, kALac3_3d,       kALplanetip_3d};
act41 ac3_3d = {COND_BONUS, 0, 4,  kALac4_3d,       kALmodeltip_3d};
act41 ac4_3d = {COND_BONUS, 0, 5,  kALac5_3d,       kALsticktip_3d};
act41 ac5_3d = {COND_BONUS, 0, 7,  kALac6_3d,       kALmousetip_3d};
act41 ac6_3d = {COND_BONUS, 0, 14, kALac7_3d,       kALcubestip_3d};
act41 ac7_3d = {COND_BONUS, 0, 11, kALac8_3d,       kALdammedtip_3d};
act41 ac8_3d = {COND_BONUS, 0, 8,  kALac9_3d,       kALmagictip_3d};
act41 ac9_3d = {COND_BONUS, 0, 12, kALremedytip_3d, kALoldmantip_3d};

//Strangerke Added to handle properly 'exits'
act46 aexit2_3d  = {INIT_JUMPEXIT, 0, false};
act46 aplane2_3d = {INIT_JUMPEXIT, 0, true};

//Strangerke - act26 are stored in new act49, as songs were not handled the same way in DOS version (in harcoded strings)
act49 aballsong_3d =   {OLD_SONG, 0,   kDTsong10_3d};
act49 afillsong_3d =   {OLD_SONG, 0,   kDTsong3_3d};
act49 anelesong_3d =   {OLD_SONG, 5,   kDTsong2_3d};
act49 aprodsong_3d =   {OLD_SONG, 20,  kDTsong1_3d};
act49 aspidersong_3d = {OLD_SONG, 105, kDTsong12_3d};
act49 asticksong_3d =  {OLD_SONG, 0,   kDTsong1_3d};
act49 asunsetsong_3d = {OLD_SONG, 0,   kDTsong11_3d};
act49 aswingsong_3d =  {OLD_SONG, 0,   kDTsong9_3d};
act49 atiesong_3d =    {OLD_SONG, 0,   kDTsong3_3d};
act49 asong0_3d =      {OLD_SONG, 0,   kDTsong0_3d};
act49 asong3_3d =      {OLD_SONG, 0,   kDTsong3_3d};
act49 asong6_3d =      {OLD_SONG, 4,   kDTsong6_3d};

actListPtr ALac2_3d[] =            {&ac2_3d, 0};
actListPtr ALac3_3d[] =            {&ac3_3d, 0};
actListPtr ALac4_3d[] =            {&ac4_3d, 0};
actListPtr ALac5_3d[] =            {&ac5_3d, 0};
actListPtr ALac6_3d[] =            {&ac6_3d, 0};
actListPtr ALac7_3d[] =            {&ac7_3d, 0};
actListPtr ALac8_3d[] =            {&ac8_3d, 0};
actListPtr ALac9_3d[] =            {&ac9_3d, 0};
actListPtr ALasleep_3d[] =         {&astartaction_3d, &aasleep_3d, &amousefree_3d, &ascare2_3d, &ascare5_3d, &ascare6_3d, &ascare7_3d, &ascare8_3d, &ascare9_3d, &ascare10_3d, &ascare11_3d, &ascare12_3d, &ascare17_3d, &aendaction_3d, 0};
actListPtr ALbittest_3d[] =        {&abittest_3d, 0};
actListPtr ALblk1_3d[] =           {&ablk1_3d, 0};
actListPtr ALblk_3d[] =            {&ablktest_3d, 0};
actListPtr ALbrg_clftop1_3d[] =    {&axy_brg_clftop_3d, &ascr_brg_clftop_3d, 0};
actListPtr ALbrg_clftop_3d[] =     {&abrgmsgtest_3d, 0};
actListPtr ALbrg_clftop_msg_3d[] = {&abrg_msg1_3d, &abrg_msg2_3d, &axy_brg_clftop_3d, &ascr_brg_clftop_3d, 0};
actListPtr ALbrg_down_3d[] =       {&ascr_path_brg2_3d, 0};
actListPtr ALbrg_ok_3d[] =         {&ascr_path_brg1_3d, 0};
actListPtr ALbrg_path_3d[] =       {&axy_brg_path_3d, &ascr_brg_path_3d, 0};
actListPtr ALbridgetip_3d[] =      {&atiptest_3d, 0};
actListPtr ALbtip_3d[] =           {&abtip_3d, 0};
actListPtr ALbtipprompt_3d[] =     {&abtipprompt_3d, 0};
actListPtr ALcagetest2_3d[] =      {&acagetest2_3d, 0};
actListPtr ALcagetest3_3d[] =      {&acagetest3_3d, 0};
actListPtr ALcagetest4_3d[] =      {&acagetest4_3d, 0};
actListPtr ALcagetest_3d[] =       {&adroptest1_3d, 0};
actListPtr ALcamp_3d[] =           {&acamp0a_3d, &acamp0b_3d, &acamp0c_3d, &acamptest_3d, 0};
actListPtr ALcamp_hut_3d[] =       {&axy_camp_hut_3d, &ascr_camp_hut_3d, 0};
actListPtr ALcamp_village_c_3d[] = {&aweehero_3d, &axy_camp_village_c_3d, &ascr_camp_village_c_3d, 0};    // exit center
actListPtr ALcamp_village_l_3d[] = {&aweehero_3d, &axy_camp_village_l_3d, &ascr_camp_village_l_3d, 0};    // exit left
actListPtr ALcampers_3d[] =        {&acamp1a_3d, &acamp1b_3d, &acamp2a_3d, &acamp2b_3d, &acamp3a_3d, &acamp3b_3d, &acamp4a_3d, &acamp4b_3d, &acamp5a_3d, &acamp5b_3d, &acamp6a_3d, &acamp6b_3d, &acamp7a_3d, &acamp7b_3d, &acamp8a_3d, &acamp8b_3d, &acamp9a_3d, &acamp9b_3d, 0};
actListPtr ALcanttake_3d[] =       {&acanttake_3d, 0};
actListPtr ALcave_man_3d[] =       {&adisappear_3d, &adisable_3d, &ascr_cave_man_3d, 0};
actListPtr ALcave_oldman_3d[] =    {&acavetest_3d, 0};
actListPtr ALcave_turn_3d[] =      {&axy_cave_turn_3d, &ascr_cave_turn_3d, 0};
actListPtr ALcave_wfall_3d[] =     {&axy_cave_wfall_3d, &awfalltest_3d, 0};
actListPtr ALchase_3d[] =          {&achase1_3d, &achase2_3d, 0};
actListPtr ALclf_clftop_3d[] =     {&axy_clf_clftop_3d, &ascr_clf_clftop_3d, 0};
actListPtr ALclf_wfall_3d[] =      {&axy_clf_wfall_3d, &awfalltest_3d, 0};
actListPtr ALclftop_brg_3d[] =     {&axy_clftop_brg_3d, &ascr_clftop_brg_3d, 0};
actListPtr ALclftop_clf_3d[] =     {&axy_clftop_clf_3d, &ascr_clftop_clf_3d, 0};
actListPtr ALclftop_slope_3d[] =   {&axy_clftop_slope_3d, &ascr_clftop_slope_3d, 0};
actListPtr ALclosedoor_3d[] =      {&aclose1_3d, &aclose2_3d, 0};
actListPtr ALcom0_3d[] =           {&acom0a_3d, &acom0b_3d, 0};
actListPtr ALcom1_3d[] =           {&acom1a_3d, &acom1b_3d, 0};
actListPtr ALcom2_3d[] =           {&acom2a_3d, &acom2b_3d, 0};
actListPtr ALcom3_3d[] =           {&acom3a_3d, &acom3b_3d, 0};
actListPtr ALcom4_3d[] =           {&acom4a_3d, &acom4b_3d, 0};
actListPtr ALcom5_3d[] =           {&acom5a_3d, &acom5b_3d, 0};
actListPtr ALcom6_3d[] =           {&acom6a_3d, &acom6b_3d, 0};
actListPtr ALcom7_3d[] =           {&acom7a_3d, &acom7b_3d, 0};
actListPtr ALcom8_3d[] =           {&acom8a_3d, &acom8b_3d, 0};
actListPtr ALcomment_3d[] =        {&anat0_3d, &apause0_3d, &apause1_3d, 0};
actListPtr ALcrashNoStory_3d[] =   {&amap1_3d, &acrashtest1_3d, 0};
actListPtr ALcrashStory_3d[] =     {&acrashtest1_3d, 0};
actListPtr ALcrash_web_3d[] =      {&axy_crash_web_3d, &ascr_crash_web_3d, 0};
actListPtr ALcrashed_3d[] =        {&astory_mode_3d, &acrash1_3d, &acrash2_3d, &acrash3_3d, &acrash4_3d, &acrash5_3d, &acrash6_3d, &acrash7_3d, &acrash8_3d, &acrash9_3d, &acrash10_3d, &acrash11_3d, &acrash12_3d, &acrash13_3d, &acrash14_3d, &acrash15_3d, &acrash16_3d, &acrash17_3d, &acrash18_3d, &acrash19_3d, &acrash20_3d, &acrash21_3d, &acrash22_3d, &acrash23_3d, 0};
actListPtr ALcrashtest2_3d[] =     {&acrashtest2_3d, 0};
actListPtr ALcryhelp_3d[] =        {&aweehero_3d, &ahelp1_3d, &ahelp2_3d, &ahelp3_3d, &ahelp4_3d, &ahelp5_3d, &ahelp6_3d, 0};
actListPtr ALcrystal_3d[] =        {&arub_3d, &ac1_3d, 0};
actListPtr ALcubestip_3d[] =       {&acubestip_3d, 0};
actListPtr ALdammed_3d[] =         {&adammed1_3d, &adammed2_3d, &adammed3_3d, 0};
actListPtr ALdammedtip_3d[] =      {&adammedtip_3d, 0};
actListPtr ALdart_3d[] =           {&apath2test_3d, 0};
actListPtr ALdarted_3d[] =         {&adarted_3d, 0};
actListPtr ALdartedtest_3d[] =     {&adartedtest_3d, 0};
actListPtr ALdartsched_3d[] =      {&adarttest1_3d, 0};
actListPtr ALdn_3d[] =             {&aweehero_3d, &adn_3d, 0};
actListPtr ALdoc_3d[] =            {&aquiet_3d, &astophero_3d, &adoc1_3d, &adoc2_3d, &adoc3_3d, &adoc4_3d, 0};
actListPtr ALdocgot_3d[] =         {&agettest1_3d, 0};
actListPtr ALdodart_3d[] =         {&astartaction_3d, &aok_3d, &adart1_3d, &adart2_3d, &adart3_3d, &adart4_3d, &adart5_3d, &adart6_3d, &aridtest_3d, &adarttest_3d, &aendaction_3d, 0};
actListPtr ALdrink_3d[] =          {&adrinktest_3d, 0};
actListPtr ALdrinkno_3d[] =        {&adrinkno_3d, 0};
actListPtr ALdrinkyes_3d[] =       {&adrinkyes_3d, &adrink_3d, 0};
actListPtr ALdropcheese_3d[] =     {&adroptest3_3d, 0};
actListPtr ALdropincage_3d[] =     {&asubcheese_3d, &aok_3d, &adropord2_3d, &adropord3_3d, &adropcheese1_3d, &adropcheese2_3d, &adropcheese3_3d, 0};
actListPtr ALdropord_3d[] =        {&asubcheese_3d, &aok_3d, &adropord1_3d, &adropord2_3d, &adropord3_3d, 0};
actListPtr ALdroptest2_3d[] =      {&adroptest2_3d, 0};
actListPtr ALeatcheese_3d[] =      {&asubcheese_3d, &acheese1_3d, &acheese2_3d, 0};
actListPtr ALele_sleep_3d[] =      {&aeleblink1_3d, 0};
actListPtr ALeleblink_3d[] =       {&arepblink_3d, &aeleblink1_3d, &aeleblink2_3d, &aeleblink3_3d, &aeleblink4_3d, 0};
actListPtr ALeletest2_3d[] =       {&aeletest2_3d, 0};
actListPtr ALempty_3d[] =          {&aemptytest1_3d, 0};
actListPtr ALempty2_3d[] =         {&aempty_3d, 0};
actListPtr ALemptymagic_3d[] =     {&aemptyflask_3d, &aemptymagic_3d, 0};
actListPtr ALemptyord_3d[] =       {&aemptyflask_3d, &aemptyord_3d, 0};
actListPtr ALemptytest2_3d[] =     {&aemptytest2_3d, 0};
actListPtr ALentertest2_3d[] =     {&aentertest2_3d, 0};
actListPtr ALentertest3_3d[] =     {&aentertest3_3d, 0};
actListPtr ALexit_3d[] =           {&aappear_3d, &aenable_3d, &aexit1_3d, &aexit2_3d, &aexit3_3d, 0};
actListPtr ALexor_3d[] =           {&aex1_3d, &aex2_3d, &aex3_3d, &aex4_3d, &aex5_3d, &aex6_3d, &aex7_3d, &aex8_3d, 0};
actListPtr ALexorcise_3d[] =       {&aexotest1_3d, 0};
actListPtr ALexordone_3d[] =       {&aexordone_3d, 0};
actListPtr ALexotest2_3d[] =       {&aexotest2_3d, 0};
actListPtr ALfill_3d[] =           {&afilltest1_3d, 0};
actListPtr ALfillmagic_3d[] =      {&afillsong_3d, &afillmagic1_3d, &afillmagic2_3d, &afillmagic3_3d, 0};
actListPtr ALfillord_3d[] =        {&afillord1_3d, &afillord2_3d, 0};
actListPtr ALfilltest2_3d[] =      {&afilltest2_3d, 0};
actListPtr ALfilltest3_3d[] =      {&afilltest3_3d, 0};
actListPtr ALfindbook_3d[] =       {&afindbtest_3d, 0};
actListPtr ALfindcrystal_3d[] =    {&aballsong_3d, &afind1_3d, &afind2_3d, 0};
actListPtr ALfindit_3d[] =         {&afindb1_3d, &afindb2_3d, &afindb3_3d, &afindb4_3d, 0};
actListPtr ALflash_3d[] =          {&arepflash_3d, &amap4a_3d, &amap4b_3d, &amap4c_3d, &amap4d_3d, 0};
actListPtr ALflask_3d[] =          {&aflasktest1_3d, 0};
actListPtr ALflasktest2_3d[] =     {&aflasktest2_3d, 0};
actListPtr ALflasktest3_3d[] =     {&aflasktest3_3d, 0};
actListPtr ALgarden_wbase_3d[] =   {&axy_garden_wbase_3d, &ascr_garden_wbase_3d, 0};
actListPtr ALgettest2_3d[] =       {&agettest2_3d, 0};
actListPtr ALgive_3d[] =           {&agive1_3d, &agive2_3d, &agive3_3d, &agive4_3d, &agive5_3d, &agive6_3d, &agive7_3d, 0};
actListPtr ALgiveb_3d[] =          {&agivetest1_3d, 0};
actListPtr ALgivetest_3d[] =       {&agivetest_3d, 0};
actListPtr ALgot_3d[] =            {&agot1_3d, &agot1a_3d, &agot1b_3d, &agot1c_3d, &agot2_3d, &agot3_3d, &agot2a_3d, &agot3a_3d, &agot2b_3d, &agot3b_3d, &agot4_3d, &agot5_3d, &agot5a_3d, &agot6_3d, &agot7_3d, &agot8_3d, &agot9_3d, &agot10_3d, &agot11_3d, &agot12_3d, 0};
actListPtr ALgotit_3d[] =          {&agotit_3d, 0};
actListPtr ALholel_3d[] =          {&ahole1_3d, &ahole2a_3d, &ahole3a_3d, &ahole4a_3d, &ahole5a_3d, &ahole6_3d, 0};
actListPtr ALholer_3d[] =          {&ahole1_3d, &ahole2b_3d, &ahole3b_3d, &ahole4b_3d, &ahole5b_3d, &ahole6_3d, 0};
actListPtr ALhut_camp_3d[] =       {&axy_hut_camp_3d, &ascr_hut_camp_3d, 0};
actListPtr ALhut_enter_3d[] =      {&axy_hut_in_3d, &ascr_hut_in_3d, 0};
actListPtr ALhut_in_3d[] =         {&adoctest_3d, 0};
actListPtr ALhut_out_3d[] =        {&axy_hut_out_3d, &ascr_hut_out_3d, 0};
actListPtr ALhut_village_c_3d[] =  {&aweehero_3d, &axy_hut_village_c_3d, &ascr_hut_village_c_3d, 0};
actListPtr ALhut_village_r_3d[] =  {&aweehero_3d, &axy_hut_village_r_3d, &ascr_hut_village_r_3d, 0};
actListPtr ALleft_3d[] =           {&aleft1_3d, &aleft2_3d, &aleft3_3d, &aleft4_3d, 0};
actListPtr ALlookfall_3d[] =       {&alookfall_3d, 0};
actListPtr ALlooknofall_3d[] =     {&alooknofall_3d, 0};
actListPtr ALlookwfall_3d[] =      {&alookwfalltest_3d, 0};
actListPtr ALmagictip_3d[] =       {&amagictip_3d, 0};
actListPtr ALmakeclay_3d[] =       {&amaketest_3d, 0};
actListPtr ALmakeit_3d[] =         {&amakeclay1_3d, &amakeclay2_3d, &amakeclay3_3d, 0};
actListPtr ALmap0_3d[] =           {&amap0_3d, 0};
actListPtr ALmap1_3d[] =           {&amap1_3d, 0};
actListPtr ALmission_3d[] =        {&amission1_3d, &amission2_3d, &amission3_3d, &amission4_3d, &amission5_3d, &amission6_3d, &amission7_3d, &amission8_3d, &amission9_3d, &amission10_3d, &amission11_3d, &amission12_3d, &amission13_3d, &amission14_3d, &amission15_3d, &amission16_3d, &amission17_3d, &amission18_3d, &amission19_3d, &amission20_3d, &amission21_3d, &amission22_3d, &amission23_3d, &amission24_3d, 0};
actListPtr ALmodeltip_3d[] =       {&amodeltip_3d, 0};
actListPtr ALmouse_3d[] =          {&acagetest_3d, &aentertest1_3d, &arepeatmouse_3d, 0};
actListPtr ALmousego_3d[] =        {&ago1_3d, &amousefree_3d, &ascare2_3d, 0};
actListPtr ALmousegone_3d[] =      {&amousegone_3d, 0};
actListPtr ALmousel_3d[] =         {&amousel_3d, 0};
actListPtr ALmouser_3d[] =         {&amouser_3d, 0};
actListPtr ALmousetip_3d[] =       {&amousetip_3d, 0};
actListPtr ALnat1_3d[] =           {&anat1_3d, 0};
actListPtr ALnat2_3d[] =           {&anat2_3d, 0};
actListPtr ALnat3_3d[] =           {&anat3_3d, 0};
actListPtr ALnat4_3d[] =           {&anat4_3d, 0};
actListPtr ALnat5_3d[] =           {&anat5_3d, 0};
actListPtr ALnat6_3d[] =           {&anat6_3d, 0};
actListPtr ALnat7_3d[] =           {&anat7_3d, 0};
actListPtr ALnat8_3d[] =           {&anat8_3d, 0};
actListPtr ALnat9_3d[] =           {&acom9_3d, 0};
actListPtr ALnative_3d[] =         {&apausetest_3d, 0};
actListPtr ALnoblow_3d[] =         {&anoblow_3d, 0};
actListPtr ALnocarry_3d[] =        {&anocarry_3d, 0};
actListPtr ALnoclay_3d[] =         {&anoclay_3d, 0};
actListPtr ALnofill_3d[] =         {&anofill_3d, 0};
actListPtr ALnomake_3d[] =         {&anomake_3d, 0};
actListPtr ALnopurps_3d[] =        {&anopurps_3d, 0};
actListPtr ALnoremedy_3d[] =       {&anoremedy1_3d, 0};
actListPtr ALnospell_3d[] =        {&anospell_3d, 0};
actListPtr ALnostick_3d[] =        {&anostick_3d, 0};
actListPtr ALnostickpin_3d[] =     {&anostickpin_3d, 0};
actListPtr ALnotakecb_3d[] =       {&anotakecb_3d, 0};
actListPtr ALnothanks2_3d[] =      {&anothanks_3d, 0};
actListPtr ALnothanks_3d[] =       {&anothanks_3d, 0};
actListPtr ALnotip_3d[] =          {&anotip_3d, 0};
actListPtr ALnottied_3d[] =        {&anottied_3d, 0};
actListPtr ALok_3d[] =             {&aok_3d, 0};
actListPtr ALoktoleave1_3d[] =     {&aweehero_3d, &axy_path_village_3d, &ascr_path_village_3d, 0};
actListPtr ALoktoleave2_3d[] =     {&axy_path_stream_3d, &astreamtest_3d, 0};
actListPtr ALold2_3d[] =           {&aold2_3d, 0};
actListPtr ALold3_3d[] =           {&aold3_3d, 0};
actListPtr ALold4_3d[] =           {&aold4_3d, 0};
actListPtr ALold5_3d[] =           {&aold5_3d, 0};
actListPtr ALold6_3d[] =           {&aold6a_3d, &aold6b_3d, &aold6c_3d, &aold6d_3d, &aold6e_3d, &acbtest_3d, &awink1_3d, &awink2_3d, &awink3_3d, &aold6f_3d, &aold6g_3d, &aold6h_3d, &aold6i_3d, 0};
actListPtr ALold7_3d[] =           {&aappear1_3d, &aenable_3d, &aold7_3d, &amap4b_3d, &areturn_3d, 0};
actListPtr ALoldfirst_3d[] =       {&aoldstate_3d, &aold0a_3d, &aold1a_3d, &aold1b_3d, &aold1c_3d, 0};
actListPtr ALoldman_3d[] =         {&aoldmantest_3d, 0};
actListPtr ALoldmantip_3d[] =      {&aoldmantip_3d, 0};
actListPtr ALoldsubseq_3d[] =      {&aoldstate_3d, &aold0b_3d, &aold1a_3d, &aold1b_3d, &aold1c_3d, 0};
actListPtr ALopencage_3d[] =       {&acagetest1_3d, 0};
actListPtr ALopencdoor_3d[] =      {&aopen1_3d, &aopen2_3d, 0};
actListPtr ALopendoor_3d[] =       {&aopentest_3d, 0};
actListPtr ALpath_3d[] =           {&aeletest1_3d, 0};
actListPtr ALpath_brg_3d[] =       {&axy_path_brg_3d, &abrgtest_3d, 0};
actListPtr ALpath_stream_3d[] =    {&aactiontest2_3d, 0};
actListPtr ALpath_village_3d[] =   {&aactiontest1_3d, 0};
actListPtr ALpath_web_3d[] =       {&axy_path_web_3d, &ascr_path_web_3d, 0};
actListPtr ALplane_3d[] =          {&adisappear_3d, &adisable_3d, &aplane1_3d, &aplane2_3d, &aplane3_3d,0};
actListPtr ALplanetip_3d[] =       {&aplanetip_3d, 0};
actListPtr ALpostest_3d[] =        {&apostest_3d, 0};
actListPtr ALprod_3d[] =           {&aprod1_3d, &aprod2_3d, &aprod3_3d, &aprod4_3d, &aprodsong_3d, &aprod5_3d, &aprod6_3d, &aprod7_3d, &aprod8_3d, 0};
actListPtr ALputitdown_3d[] =      {&aputitdown_3d, 0};
actListPtr ALreadbook_3d[] =       {&areadtest1_3d, 0};
actListPtr ALreadord_3d[] =        {&abook1_3d, 0};
actListPtr ALreadtest2_3d[] =      {&areadtest2_3d, 0};
actListPtr ALrefuse_3d[] =         {&arefuse_3d, 0};
actListPtr ALrefuseflask_3d[] =    {&arefuseflask_3d, 0};
actListPtr ALremedy_3d[] =         {&aflask1_3d, &aflask2_3d, &aflask3_3d, &aflask4_3d, &aflask5_3d, &astophero_3d, &adisable_3d, &aflask6_3d, 0};
actListPtr ALremedytip_3d[] =      {&aremedytip_3d, 0};
actListPtr ALreturn_3d[] =         {&areturn1_3d, &areturn2_3d, &areturn3_3d, &areturn4_3d, &areturn5_3d, 0};
actListPtr ALright_3d[] =          {&aright1_3d, &aright2_3d, &aright3_3d, &aright4_3d, 0};
actListPtr ALscare_3d[] =          {&astartaction_3d, &ascarebonus_3d, &anelesong_3d, &amousefree_3d, &ascare1_3d, &ascare2_3d, &ascare3_3d, &ascare4_3d, &ascare5_3d, &ascare6_3d, &ascare7_3d, &ascare8_3d, &ascare9_3d, &ascare10_3d, &ascare11_3d, &ascare12_3d, &ascare13_3d, &ascare14_3d, &ascare15_3d, &ascare16_3d, &ascare17_3d, &aridtest_3d, &adarttest_3d, &aendaction_3d, 0};
actListPtr ALscared_3d[] =         {&ascared_3d, 0};
actListPtr ALsleepy_3d[] =         {&asleepy_3d, 0};
actListPtr ALslope_clftop_3d[] =   {&axy_slope_clftop_3d, &ascr_slope_clftop_3d, 0};
actListPtr ALslope_stream_3d[] =   {&axy_slope_stream_3d, &astreamtest_3d, 0};
actListPtr ALsong3_3d[] =          {&asong3_3d, 0};
actListPtr ALspider_3d[] =         {&aplantfix_3d, &aspidersong_3d, &amap1_3d, &aweb1_3d, &aweb2_3d, &aweb3_3d, &aweb4_3d, &aweb5_3d, &aweb6_3d, &aweb7_3d, &aweb8_3d, &aweb9_3d, &aweb10_3d, &aweb11_3d, &aweb12_3d, &aweb13_3d, &aweb14_3d, &aweb15_3d, &aweb16_3d, &aweb17_3d, &aweb18_3d, &aweb19_3d, &aweb20_3d, &aweb21_3d, &aweb22_3d, &aweb23_3d, &aweb24_3d, &aweb25_3d, &aweb26_3d, &aweb27_3d, 0};
actListPtr ALspirit_3d[] =         {&aspirittest_3d, 0};
actListPtr ALstalk_3d[] =          {&astalk_3d, 0};
actListPtr ALstick_3d[] =          {&asticktest3_3d, 0};
actListPtr ALstickpin_3d[] =       {&asticksong_3d, &aprod1_3d, &aprod2_3d, &aprod3_3d, &astick1_3d, &astick2_3d, &astick3_3d, &astick4_3d, &astick5_3d, &astick6_3d, &astick7_3d, 0};
actListPtr ALsticktest1_3d[] =     {&asticktest1_3d, 0};
actListPtr ALsticktest2_3d[] =     {&asticktest2_3d, 0};
actListPtr ALsticktest4_3d[] =     {&asticktest4_3d, 0};
actListPtr ALsticktip_3d[] =       {&asticktip_3d, 0};
actListPtr ALstream1_3d[] =        {&ascr_slope_stream1_3d, 0};
actListPtr ALstream2_3d[] =        {&ascr_slope_stream2_3d, 0};
actListPtr ALstream_path_3d[] =    {&axy_stream_path_3d, &ascr_stream_path_3d, 0};
actListPtr ALstream_slope_3d[] =   {&axy_stream_slope_3d, &ascr_stream_slope_3d, 0};
actListPtr ALstuckpin_3d[] =       {&astuckpin_3d, 0};
actListPtr ALsunset_3d[] =         {&asunsetsong_3d, &asunset1_3d, &asunset2_3d, &asunset3_3d, 0};
actListPtr ALswing_3d[] =          {&aquiet_3d, &aswingsong_3d, &aswing0_3d, &aswing1_3d, &aswing2_3d, &aswing3_3d, &aswing4_3d, &aswing5_3d, &aswing6_3d, &aswing7_3d, &aswing8_3d, 0};
actListPtr ALswingtip_3d[] =       {&aswingtip_3d, 0};
actListPtr ALtakecage_3d[] =       {&ataketest3_3d, 0};
actListPtr ALtakecb_3d[] =         {&atakecb1_3d, &atakecb2_3d, &atakecb3_3d, 0};
actListPtr ALtakecheese_3d[] =     {&atakechstest_3d, 0};
actListPtr ALtakechs_3d[] =        {&aaddcheese_3d, &aok_3d, &atakencheese_3d, &atakecheese1_3d, &atakecheese2_3d, &atakecheese3_3d, 0};
actListPtr ALtakeit_3d[] =         {&aok_3d, &atakecage1_3d, &atakecage2_3d, &atakecage3_3d, 0};
actListPtr ALtaketest1_3d[] =      {&ataketest1_3d, 0};
actListPtr ALtaketest2_3d[] =      {&ataketest2_3d, 0};
actListPtr ALtalknat_3d[] =        {&atalktest3_3d, 0};
actListPtr ALtalktest1_3d[] =      {&atalktest1_3d, 0};
actListPtr ALtalktest2_3d[] =      {&atalktest2_3d, 0};
actListPtr ALtalkweb_3d[] =        {&atalkweb_3d, 0};
actListPtr ALtied_3d[] =           {&atied_3d, 0};
actListPtr ALtievine_3d[] =        {&atiesong_3d, &avine1_3d, &avine2_3d, &avine3_3d, &avine4_3d, &avine5_3d, &avine6_3d, 0};
actListPtr ALtrapped_3d[] =        {&aaddcheese_3d, &aenter0_3d, &aenter1_3d, &aenter2_3d, &aenter3_3d, &aenter4_3d, &aenter5_3d, &aenter6_3d, &aenter7_3d, &aenter8_3d, 0};
actListPtr ALturn_cave_3d[] =      {&axy_turn_cave_3d, &ascr_turn_cave_3d, 0};
actListPtr ALturn_village_3d[] =   {&aweehero_3d, &axy_turn_village_3d, &ascr_turn_village_3d, 0};
actListPtr ALuntie_3d[] =          {&auntie_3d, 0};
actListPtr ALuntie_vine_3d[] =     {&auntietest_3d, 0};
actListPtr ALup_3d[] =             {&aweehero_3d, &aup_3d, 0};
actListPtr ALvillage_camp_l_3d[] = {&aweehero_3d, &axy_village_camp_l_3d, &ascr_village_camp_l_3d, 0};
actListPtr ALvillage_camp_r_3d[] = {&aweehero_3d, &axy_village_camp_r_3d, &ascr_village_camp_r_3d, 0};
actListPtr ALvillage_path_3d[] =   {&aweehero_3d, &axy_village_path_3d, &ascr_village_path_3d, 0};
actListPtr ALvillage_thing_3d[] =  {&athing_3d, 0};
actListPtr ALvillage_turn_3d[] =   {&aweehero_3d, &axy_village_turn_3d, &ascr_village_turn_3d, 0};
actListPtr ALvine_3d[] =           {&avinetest_3d, 0};
actListPtr ALwarn_3d[] =           {&awarn_3d, 0};
actListPtr ALwaterfall_3d[] =      {&ascr_clf_wfall_3d, 0};
actListPtr ALwaternofall_3d[] =    {&ascr_clf_wnofall_3d, 0};
actListPtr ALwbase_garden_3d[] =   {&axy_wbase_garden_3d, &ascr_wbase_garden_3d, 0};
actListPtr ALwbase_wfall_3d[] =    {&aelewaking_3d, &aelewoken_3d, &axy_wbase_wfall_3d, &ascr_wbase_wfall_3d, 0};
actListPtr ALweb_3d[] =            {&awebtest1_3d, 0};
actListPtr ALweb_crash_3d[] =      {&axy_web_crash_3d, &ascr_web_crash_3d, 0};
actListPtr ALweb_path_3d[] =       {&axy_web_path_3d, &ascr_web_path_3d, 0};
actListPtr ALwebtest2_3d[] =       {&awebtest2_3d, 0};
actListPtr ALwfall_cave_3d[] =     {&axy_wfall_cave_3d, &ascr_wfall_cave_3d, 0};
actListPtr ALwfall_clf_3d[] =      {&axy_wfall_clf_3d, &ascr_wfall_clf_3d, 0};
actListPtr ALwfall_wbase_3d[] =    {&awaterfalling_3d, 0};
actListPtr ALwfallb_cave_3d[] =    {&axy_wfall_cave_3d, &ascr_wfall_cave_3d, 0};
actListPtr ALwfallb_clf_3d[] =     {&axy_wfall_clf_3d, &ascr_wfall_clf_3d, 0};
actListPtr ALwfallb_wbase_3d[] =   {&axy_wfallb_wbase_3d, &ascr_wfallb_wbase_3d, 0};
actListPtr ALwrong_3d[] =          {&aappear1_3d, &aenable_3d, &awrong1_3d, &amap4b_3d, &areturn_3d, 0};
//Added by Strangerke to improve mouse support
actListPtr ALhorizdn_3d[] =        {&ahoriz2_3d, 0};
actListPtr ALhorizon_3d[] =        {&ahorizpos_3d, &arepeathorizon_3d, 0};
actListPtr ALhorizup_3d[] =        {&ahoriz1_3d, 0};

actList actListArr_3d[] = {
	ALDummy,             ALac2_3d,            ALac3_3d,            ALac4_3d,            ALac5_3d,
	ALac6_3d,            ALac7_3d,            ALac8_3d,            ALac9_3d,            ALasleep_3d,
	ALbittest_3d,        ALblk1_3d,           ALblk_3d,            ALbrg_clftop1_3d,    ALbrg_clftop_3d,
	ALbrg_clftop_msg_3d, ALbrg_down_3d,       ALbrg_ok_3d,         ALbrg_path_3d,       ALbridgetip_3d,
	ALbtip_3d,           ALbtipprompt_3d,     ALcagetest2_3d,      ALcagetest3_3d,      ALcagetest4_3d,
	ALcagetest_3d,       ALcamp_3d,           ALcamp_hut_3d,       ALcamp_village_c_3d, ALcamp_village_l_3d,
	ALcampers_3d,        ALcanttake_3d,       ALcave_man_3d,       ALcave_oldman_3d,    ALcave_turn_3d,
	ALcave_wfall_3d,     ALchase_3d,          ALclf_clftop_3d,     ALclf_wfall_3d,      ALclftop_brg_3d,
	ALclftop_clf_3d,     ALclftop_slope_3d,   ALclosedoor_3d,      ALcom0_3d,           ALcom1_3d,
	ALcom2_3d,           ALcom3_3d,           ALcom4_3d,           ALcom5_3d,           ALcom6_3d,
	ALcom7_3d,           ALcom8_3d,           ALcomment_3d,        ALcrashNoStory_3d,   ALcrashStory_3d,
	ALcrash_web_3d,      ALcrashed_3d,        ALcrashtest2_3d,     ALcryhelp_3d,        ALcrystal_3d,
	ALcubestip_3d,       ALdammed_3d,         ALdammedtip_3d,      ALdart_3d,           ALdarted_3d,
	ALdartedtest_3d,     ALdartsched_3d,      ALdn_3d,             ALdoc_3d,            ALdocgot_3d,
	ALdodart_3d,         ALdrink_3d,          ALdrinkno_3d,        ALdrinkyes_3d,       ALdropcheese_3d,
	ALdropincage_3d,     ALdropord_3d,        ALdroptest2_3d,      ALeatcheese_3d,      ALele_sleep_3d,
	ALeleblink_3d,       ALeletest2_3d,       ALempty_3d,          ALempty2_3d,         ALemptymagic_3d,
	ALemptyord_3d,       ALemptytest2_3d,     ALentertest2_3d,     ALentertest3_3d,     ALexit_3d,
	ALexor_3d,           ALexorcise_3d,       ALexordone_3d,       ALexotest2_3d,       ALfill_3d,
	ALfillmagic_3d,      ALfillord_3d,        ALfilltest2_3d,      ALfilltest3_3d,      ALfindbook_3d,
	ALfindcrystal_3d,    ALfindit_3d,         ALflash_3d,          ALflask_3d,          ALflasktest2_3d,
	ALflasktest3_3d,     ALgarden_wbase_3d,   ALgettest2_3d,       ALgive_3d,           ALgiveb_3d,
	ALgivetest_3d,       ALgot_3d,            ALgotit_3d,          ALholel_3d,          ALholer_3d,
	ALhut_camp_3d,       ALhut_enter_3d,      ALhut_in_3d,         ALhut_out_3d,        ALhut_village_c_3d,
	ALhut_village_r_3d,  ALleft_3d,           ALlookfall_3d,       ALlooknofall_3d,     ALlookwfall_3d,
	ALmagictip_3d,       ALmakeclay_3d,       ALmakeit_3d,         ALmap0_3d,           ALmap1_3d,
	ALmission_3d,        ALmodeltip_3d,       ALmouse_3d,          ALmousego_3d,        ALmousegone_3d,
	ALmousel_3d,         ALmouser_3d,         ALmousetip_3d,       ALnat1_3d,           ALnat2_3d,
	ALnat3_3d,           ALnat4_3d,           ALnat5_3d,           ALnat6_3d,           ALnat7_3d,
	ALnat8_3d,           ALnat9_3d,           ALnative_3d,         ALnoblow_3d,         ALnocarry_3d,
	ALnoclay_3d,         ALnofill_3d,         ALnomake_3d,         ALnopurps_3d,        ALnoremedy_3d,
	ALnospell_3d,        ALnostick_3d,        ALnostickpin_3d,     ALnotakecb_3d,       ALnothanks2_3d,
	ALnothanks_3d,       ALnotip_3d,          ALnottied_3d,        ALok_3d,             ALoktoleave1_3d,
	ALoktoleave2_3d,     ALold2_3d,           ALold3_3d,           ALold4_3d,           ALold5_3d,
	ALold6_3d,           ALold7_3d,           ALoldfirst_3d,       ALoldman_3d,         ALoldmantip_3d,
	ALoldsubseq_3d,      ALopencage_3d,       ALopencdoor_3d,      ALopendoor_3d,       ALpath_3d,
	ALpath_brg_3d,       ALpath_stream_3d,    ALpath_village_3d,   ALpath_web_3d,       ALplane_3d,
	ALplanetip_3d,       ALpostest_3d,        ALprod_3d,           ALputitdown_3d,      ALreadbook_3d,
	ALreadord_3d,        ALreadtest2_3d,      ALrefuse_3d,         ALrefuseflask_3d,    ALremedy_3d,
	ALremedytip_3d,      ALreturn_3d,         ALright_3d,          ALscare_3d,          ALscared_3d,
	ALsleepy_3d,         ALslope_clftop_3d,   ALslope_stream_3d,   ALsong3_3d,          ALspider_3d,
	ALspirit_3d,         ALstalk_3d,          ALstick_3d,          ALstickpin_3d,       ALsticktest1_3d,
	ALsticktest2_3d,     ALsticktest4_3d,     ALsticktip_3d,       ALstream1_3d,        ALstream2_3d,
	ALstream_path_3d,    ALstream_slope_3d,   ALstuckpin_3d,       ALsunset_3d,         ALswing_3d,
	ALswingtip_3d,       ALtakecage_3d,       ALtakecb_3d,         ALtakecheese_3d,     ALtakechs_3d,
	ALtakeit_3d,         ALtaketest1_3d,      ALtaketest2_3d,      ALtalknat_3d,        ALtalktest1_3d,
	ALtalktest2_3d,      ALtalkweb_3d,        ALtied_3d,           ALtievine_3d,        ALtrapped_3d,
	ALturn_cave_3d,      ALturn_village_3d,   ALuntie_3d,          ALuntie_vine_3d,     ALup_3d,
	ALvillage_camp_l_3d, ALvillage_camp_r_3d, ALvillage_path_3d,   ALvillage_thing_3d,  ALvillage_turn_3d,
	ALvine_3d,           ALwarn_3d,           ALwaterfall_3d,      ALwaternofall_3d,    ALwbase_garden_3d,
	ALwbase_wfall_3d,    ALweb_3d,            ALweb_crash_3d,      ALweb_path_3d,       ALwebtest2_3d,
	ALwfall_cave_3d,     ALwfall_clf_3d,      ALwfall_wbase_3d,    ALwfallb_cave_3d,    ALwfallb_clf_3d,
	ALwfallb_wbase_3d,   ALwrong_3d,
// Added by Strangerke to improve mouse support
	ALhorizdn_3d,        ALhorizon_3d,        ALhorizup_3d
};

// Default tune selection - repeats indefinitely
int16 def_tunes_1w[] = {T_TRACK1, T_TRACK2, T_TRACK3, T_TRACK6, -1};
int16 def_tunes_2w[] = {T_TRACK4, T_TRACK5, T_TRACK6, -1};
int16 def_tunes_3w[] = {T_TRACK7, T_TRACK8, T_TRACK9, -1};
// Dummy initialisation
int16 def_tunes_1d[] = {-1};
int16 def_tunes_2d[] = {-1};
int16 def_tunes_3d[] = {-1};

#endif
