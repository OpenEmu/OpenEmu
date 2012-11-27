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

#ifndef SCI_SOUND_DRIVERS_MAP_MT32_TO_GM_H
#define SCI_SOUND_DRIVERS_MAP_MT32_TO_GM_H

namespace Sci {

#include "common/list.h"

// Patch not mapped
#define MIDI_UNMAPPED 0xff
// Patch mapped to rhythm key
#define MIDI_MAPPED_TO_RHYTHM 0xfe

struct Mt32ToGmMap {
	const char *name;
	uint8 gmInstr;
	uint8 gmRhythmKey;
};

/*******************************************
 * Fancy instrument mappings begin here... *
 *******************************************/


static const Mt32ToGmMap Mt32PresetTimbreMaps[] = {
	/*000*/  {"AcouPiano1", 0, MIDI_UNMAPPED},
	/*001*/  {"AcouPiano2", 1, MIDI_UNMAPPED},
	/*002*/  {"AcouPiano3", 0, MIDI_UNMAPPED},
	/*003*/  {"ElecPiano1", 4, MIDI_UNMAPPED},
	/*004*/  {"ElecPiano2", 5, MIDI_UNMAPPED},
	/*005*/  {"ElecPiano3", 4, MIDI_UNMAPPED},
	/*006*/  {"ElecPiano4", 5, MIDI_UNMAPPED},
	/*007*/  {"Honkytonk ", 3, MIDI_UNMAPPED},
	/*008*/  {"Elec Org 1", 16, MIDI_UNMAPPED},
	/*009*/  {"Elec Org 2", 17, MIDI_UNMAPPED},
	/*010*/  {"Elec Org 3", 18, MIDI_UNMAPPED},
	/*011*/  {"Elec Org 4", 18, MIDI_UNMAPPED},
	/*012*/  {"Pipe Org 1", 19, MIDI_UNMAPPED},
	/*013*/  {"Pipe Org 2", 19, MIDI_UNMAPPED},
	/*014*/  {"Pipe Org 3", 20, MIDI_UNMAPPED},
	/*015*/  {"Accordion ", 21, MIDI_UNMAPPED},
	/*016*/  {"Harpsi 1  ", 6, MIDI_UNMAPPED},
	/*017*/  {"Harpsi 2  ", 6, MIDI_UNMAPPED},
	/*018*/  {"Harpsi 3  ", 6, MIDI_UNMAPPED},
	/*019*/  {"Clavi 1   ", 7, MIDI_UNMAPPED},
	/*020*/  {"Clavi 2   ", 7, MIDI_UNMAPPED},
	/*021*/  {"Clavi 3   ", 7, MIDI_UNMAPPED},
	/*022*/  {"Celesta 1 ", 8, MIDI_UNMAPPED},
	/*023*/  {"Celesta 2 ", 8, MIDI_UNMAPPED},
	/*024*/  {"Syn Brass1", 62, MIDI_UNMAPPED},
	/*025*/  {"Syn Brass2", 63, MIDI_UNMAPPED},
	/*026*/  {"Syn Brass3", 62, MIDI_UNMAPPED},
	/*027*/  {"Syn Brass4", 63, MIDI_UNMAPPED},
	/*028*/  {"Syn Bass 1", 38, MIDI_UNMAPPED},
	/*029*/  {"Syn Bass 2", 39, MIDI_UNMAPPED},
	/*030*/  {"Syn Bass 3", 38, MIDI_UNMAPPED},
	/*031*/  {"Syn Bass 4", 39, MIDI_UNMAPPED},
	/*032*/  {"Fantasy   ", 88, MIDI_UNMAPPED},
	/*033*/  {"Harmo Pan ", 89, MIDI_UNMAPPED},
	/*034*/  {"Chorale   ", 52, MIDI_UNMAPPED},
	/*035*/  {"Glasses   ", 98, MIDI_UNMAPPED},
	/*036*/  {"Soundtrack", 97, MIDI_UNMAPPED},
	/*037*/  {"Atmosphere", 99, MIDI_UNMAPPED},
	/*038*/  {"Warm Bell ", 89, MIDI_UNMAPPED},
	/*039*/  {"Funny Vox ", 85, MIDI_UNMAPPED},
	/*040*/  {"Echo Bell ", 39, MIDI_UNMAPPED},
	/*041*/  {"Ice Rain  ", 101, MIDI_UNMAPPED},
	/*042*/  {"Oboe 2001 ", 68, MIDI_UNMAPPED},
	/*043*/  {"Echo Pan  ", 87, MIDI_UNMAPPED},
	/*044*/  {"DoctorSolo", 86, MIDI_UNMAPPED},
	/*045*/  {"Schooldaze", 103, MIDI_UNMAPPED},
	/*046*/  {"BellSinger", 88, MIDI_UNMAPPED},
	/*047*/  {"SquareWave", 80, MIDI_UNMAPPED},
	/*048*/  {"Str Sect 1", 48, MIDI_UNMAPPED},
	/*049*/  {"Str Sect 2", 48, MIDI_UNMAPPED},
	/*050*/  {"Str Sect 3", 49, MIDI_UNMAPPED},
	/*051*/  {"Pizzicato ", 45, MIDI_UNMAPPED},
	/*052*/  {"Violin 1  ", 40, MIDI_UNMAPPED},
	/*053*/  {"Violin 2  ", 40, MIDI_UNMAPPED},
	/*054*/  {"Cello 1   ", 42, MIDI_UNMAPPED},
	/*055*/  {"Cello 2   ", 42, MIDI_UNMAPPED},
	/*056*/  {"Contrabass", 43, MIDI_UNMAPPED},
	/*057*/  {"Harp 1    ", 46, MIDI_UNMAPPED},
	/*058*/  {"Harp 2    ", 46, MIDI_UNMAPPED},
	/*059*/  {"Guitar 1  ", 24, MIDI_UNMAPPED},
	/*060*/  {"Guitar 2  ", 25, MIDI_UNMAPPED},
	/*061*/  {"Elec Gtr 1", 26, MIDI_UNMAPPED},
	/*062*/  {"Elec Gtr 2", 27, MIDI_UNMAPPED},
	/*063*/  {"Sitar     ", 104, MIDI_UNMAPPED},
	/*064*/  {"Acou Bass1", 32, MIDI_UNMAPPED},
	/*065*/  {"Acou Bass2", 33, MIDI_UNMAPPED},
	/*066*/  {"Elec Bass1", 34, MIDI_UNMAPPED},
	/*067*/  {"Elec Bass2", 39, MIDI_UNMAPPED},
	/*068*/  {"Slap Bass1", 36, MIDI_UNMAPPED},
	/*069*/  {"Slap Bass2", 37, MIDI_UNMAPPED},
	/*070*/  {"Fretless 1", 35, MIDI_UNMAPPED},
	/*071*/  {"Fretless 2", 35, MIDI_UNMAPPED},
	/*072*/  {"Flute 1   ", 73, MIDI_UNMAPPED},
	/*073*/  {"Flute 2   ", 73, MIDI_UNMAPPED},
	/*074*/  {"Piccolo 1 ", 72, MIDI_UNMAPPED},
	/*075*/  {"Piccolo 2 ", 72, MIDI_UNMAPPED},
	/*076*/  {"Recorder  ", 74, MIDI_UNMAPPED},
	/*077*/  {"Panpipes  ", 75, MIDI_UNMAPPED},
	/*078*/  {"Sax 1     ", 64, MIDI_UNMAPPED},
	/*079*/  {"Sax 2     ", 65, MIDI_UNMAPPED},
	/*080*/  {"Sax 3     ", 66, MIDI_UNMAPPED},
	/*081*/  {"Sax 4     ", 67, MIDI_UNMAPPED},
	/*082*/  {"Clarinet 1", 71, MIDI_UNMAPPED},
	/*083*/  {"Clarinet 2", 71, MIDI_UNMAPPED},
	/*084*/  {"Oboe      ", 68, MIDI_UNMAPPED},
	/*085*/  {"Engl Horn ", 69, MIDI_UNMAPPED},
	/*086*/  {"Bassoon   ", 70, MIDI_UNMAPPED},
	/*087*/  {"Harmonica ", 22, MIDI_UNMAPPED},
	/*088*/  {"Trumpet 1 ", 56, MIDI_UNMAPPED},
	/*089*/  {"Trumpet 2 ", 56, MIDI_UNMAPPED},
	/*090*/  {"Trombone 1", 57, MIDI_UNMAPPED},
	/*091*/  {"Trombone 2", 57, MIDI_UNMAPPED},
	/*092*/  {"Fr Horn 1 ", 60, MIDI_UNMAPPED},
	/*093*/  {"Fr Horn 2 ", 60, MIDI_UNMAPPED},
	/*094*/  {"Tuba      ", 58, MIDI_UNMAPPED},
	/*095*/  {"Brs Sect 1", 61, MIDI_UNMAPPED},
	/*096*/  {"Brs Sect 2", 61, MIDI_UNMAPPED},
	/*097*/  {"Vibe 1    ", 11, MIDI_UNMAPPED},
	/*098*/  {"Vibe 2    ", 11, MIDI_UNMAPPED},
	/*099*/  {"Syn Mallet", 15, MIDI_UNMAPPED},
	/*100*/  {"Wind Bell ", 88, MIDI_UNMAPPED},
	/*101*/  {"Glock     ", 9, MIDI_UNMAPPED},
	/*102*/  {"Tube Bell ", 14, MIDI_UNMAPPED},
	/*103*/  {"Xylophone ", 13, MIDI_UNMAPPED},
	/*104*/  {"Marimba   ", 12, MIDI_UNMAPPED},
	/*105*/  {"Koto      ", 107, MIDI_UNMAPPED},
	/*106*/  {"Sho       ", 111, MIDI_UNMAPPED},
	/*107*/  {"Shakuhachi", 77, MIDI_UNMAPPED},
	/*108*/  {"Whistle 1 ", 78, MIDI_UNMAPPED},
	/*109*/  {"Whistle 2 ", 78, MIDI_UNMAPPED},
	/*110*/  {"BottleBlow", 76, MIDI_UNMAPPED},
	/*111*/  {"BreathPipe", 121, MIDI_UNMAPPED},
	/*112*/  {"Timpani   ", 47, MIDI_UNMAPPED},
	/*113*/  {"MelodicTom", 117, MIDI_UNMAPPED},
	/*114*/  {"Deep Snare", MIDI_MAPPED_TO_RHYTHM, 38},
	/*115*/  {"Elec Perc1", 115, MIDI_UNMAPPED}, // ?
	/*116*/  {"Elec Perc2", 118, MIDI_UNMAPPED}, // ?
	/*117*/  {"Taiko     ", 116, MIDI_UNMAPPED},
	/*118*/  {"Taiko Rim ", 118, MIDI_UNMAPPED},
	/*119*/  {"Cymbal    ", MIDI_MAPPED_TO_RHYTHM, 51},
	/*120*/  {"Castanets ", MIDI_MAPPED_TO_RHYTHM, 75},	// approximation
	/*121*/  {"Triangle  ", 112, MIDI_UNMAPPED},
	/*122*/  {"Orche Hit ", 55, MIDI_UNMAPPED},
	/*123*/  {"Telephone ", 124, MIDI_UNMAPPED},
	/*124*/  {"Bird Tweet", 123, MIDI_UNMAPPED},
	/*125*/  {"OneNoteJam", 8, MIDI_UNMAPPED}, // approximation
	/*126*/  {"WaterBells", 98, MIDI_UNMAPPED},
	/*127*/  {"JungleTune", 75, MIDI_UNMAPPED} // approximation
};

static const Mt32ToGmMap Mt32RhythmTimbreMaps[] = {
	/*00*/  {"Acou BD   ", MIDI_MAPPED_TO_RHYTHM, 35},
	/*01*/  {"Acou SD   ", MIDI_MAPPED_TO_RHYTHM, 38},
	/*02*/  {"Acou HiTom", 117, 50},
	/*03*/  {"AcouMidTom", 117, 47},
	/*04*/  {"AcouLowTom", 117, 41},
	/*05*/  {"Elec SD   ", MIDI_MAPPED_TO_RHYTHM, 40},
	/*06*/  {"Clsd HiHat", MIDI_MAPPED_TO_RHYTHM, 42},
	/*07*/  {"OpenHiHat1", MIDI_MAPPED_TO_RHYTHM, 46},
	/*08*/  {"Crash Cym ", MIDI_MAPPED_TO_RHYTHM, 49},
	/*09*/  {"Ride Cym  ", MIDI_MAPPED_TO_RHYTHM, 51},
	/*10*/  {"Rim Shot  ", MIDI_MAPPED_TO_RHYTHM, 37},
	/*11*/  {"Hand Clap ", MIDI_MAPPED_TO_RHYTHM, 39},
	/*12*/  {"Cowbell   ", MIDI_MAPPED_TO_RHYTHM, 56},
	/*13*/  {"Mt HiConga", MIDI_MAPPED_TO_RHYTHM, 62},
	/*14*/  {"High Conga", MIDI_MAPPED_TO_RHYTHM, 63},
	/*15*/  {"Low Conga ", MIDI_MAPPED_TO_RHYTHM, 64},
	/*16*/  {"Hi Timbale", MIDI_MAPPED_TO_RHYTHM, 65},
	/*17*/  {"LowTimbale", MIDI_MAPPED_TO_RHYTHM, 66},
	/*18*/  {"High Bongo", MIDI_MAPPED_TO_RHYTHM, 60},
	/*19*/  {"Low Bongo ", MIDI_MAPPED_TO_RHYTHM, 61},
	/*20*/  {"High Agogo", 113, 67},
	/*21*/  {"Low Agogo ", 113, 68},
	/*22*/  {"Tambourine", MIDI_MAPPED_TO_RHYTHM, 54},
	/*23*/  {"Claves    ", MIDI_MAPPED_TO_RHYTHM, 75},
	/*24*/  {"Maracas   ", MIDI_MAPPED_TO_RHYTHM, 70},
	/*25*/  {"SmbaWhis L", 78, 72},
	/*26*/  {"SmbaWhis S", 78, 71},
	/*27*/  {"Cabasa    ", MIDI_MAPPED_TO_RHYTHM, 69},
	/*28*/  {"Quijada   ", MIDI_MAPPED_TO_RHYTHM, 73},
	/*29*/  {"OpenHiHat2", MIDI_MAPPED_TO_RHYTHM, 44}
};

static const uint8 Mt32PresetRhythmKeymap[] = {
	MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED,
	MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED,
	MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED,
	MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
	50, 51, MIDI_UNMAPPED, MIDI_UNMAPPED, 54, MIDI_UNMAPPED, 56, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED,
	60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
	70, 71, 72, 73, MIDI_UNMAPPED, 75, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED,
	MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED,
	MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED,
	MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED,
	MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED,
	MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED, MIDI_UNMAPPED
};

/* +++ - Don't change unless you've got a good reason
   ++  - Looks good, sounds ok
   +   - Not too bad, but is it right?
   ?   - Where do I map this one?
   ??  - Any good ideas?
   ??? - I'm clueless?
   R   - Rhythm...
*/
static const Mt32ToGmMap Mt32MemoryTimbreMaps[] = {
	{"AccPnoKA2 ", 1, MIDI_UNMAPPED},     // ++ (KQ1)
	{"Acou BD   ", MIDI_MAPPED_TO_RHYTHM, 35},   // R (PQ2)
	{"Acou SD   ", MIDI_MAPPED_TO_RHYTHM, 38},   // R (PQ2)
	{"AcouPnoKA ", 0, MIDI_UNMAPPED},     // ++ (KQ1)
	{"BASS      ", 32, MIDI_UNMAPPED},    // + (LSL3)
	{"BASSOONPCM", 70, MIDI_UNMAPPED},    // + (LB1)
	{"BEACH WAVE", 122, MIDI_UNMAPPED},   // + (LSL3)
	{"BagPipes  ", 109, MIDI_UNMAPPED},
	{"BassPizzMS", 45, MIDI_UNMAPPED},    // ++ (QFG1)
	{"BassoonKA ", 70, MIDI_UNMAPPED},    // ++ (KQ1)
	{"Bell    MS", 112, MIDI_UNMAPPED},   // ++ (Iceman)
	{"Bells   MS", 112, MIDI_UNMAPPED},   // + (QFG1)
	{"Big Bell  ", 14, MIDI_UNMAPPED},    // + (LB1)
	{"Bird Tweet", 123, MIDI_UNMAPPED},
	{"BrsSect MS", 61, MIDI_UNMAPPED},    // +++ (Iceman)
	{"CLAPPING  ", 126, MIDI_UNMAPPED},   // ++ (LSL3)
	{"Cabasa    ", MIDI_MAPPED_TO_RHYTHM, 69},   // R (Hoyle)
	{"Calliope  ", 82, MIDI_UNMAPPED},    // +++ (QFG1)
	{"CelticHarp", 46, MIDI_UNMAPPED},    // ++ (Camelot)
	{"Chicago MS", 1, MIDI_UNMAPPED},     // ++ (Iceman)
	{"Chop      ", 117, MIDI_UNMAPPED},
	{"Chorale MS", 52, MIDI_UNMAPPED},    // + (Camelot)
	{"ClarinetMS", 71, MIDI_UNMAPPED},
	{"Claves    ", MIDI_MAPPED_TO_RHYTHM, 75},   // R (PQ2)
	{"Claw    MS", 118, MIDI_UNMAPPED},    // + (QFG1)
	{"ClockBell ", 14, MIDI_UNMAPPED},    // + (LB1)
	{"ConcertCym", MIDI_MAPPED_TO_RHYTHM, 55},   // R ? (KQ1)
	{"Conga   MS", MIDI_MAPPED_TO_RHYTHM, 64},   // R (QFG1)
	{"CoolPhone ", 124, MIDI_UNMAPPED},   // ++ (LSL3)
	{"CracklesMS", 115, MIDI_UNMAPPED}, // ? (Camelot, QFG1)
	{"CreakyD MS", MIDI_UNMAPPED, MIDI_UNMAPPED}, // ??? (KQ1)
	{"Cricket   ", 120, MIDI_UNMAPPED}, // ? (LB1)
	{"CrshCymbMS", MIDI_MAPPED_TO_RHYTHM, 57},   // R +++ (Iceman)
	{"CstlGateMS", MIDI_UNMAPPED, MIDI_UNMAPPED}, // ? (QFG1)
	{"CymSwellMS", MIDI_MAPPED_TO_RHYTHM, 55},   // R ? (Camelot, QFG1)
	{"CymbRollKA", MIDI_MAPPED_TO_RHYTHM, 57},   // R ? (KQ1)
	{"Cymbal Lo ", MIDI_UNMAPPED, MIDI_UNMAPPED}, // R ? (LSL3)
	{"card      ", MIDI_UNMAPPED, MIDI_UNMAPPED}, // ? (Hoyle)
	{"DirtGtr MS", 30, MIDI_UNMAPPED},    // + (Iceman)
	{"DirtGtr2MS", 29, MIDI_UNMAPPED},    // + (Iceman)
	{"E Bass  MS", 33, MIDI_UNMAPPED},    // + (SQ3)
	{"ElecBassMS", 33, MIDI_UNMAPPED},
	{"ElecGtr MS", 27, MIDI_UNMAPPED},    // ++ (Iceman)
	{"EnglHornMS", 69, MIDI_UNMAPPED},
	{"FantasiaKA", 88, MIDI_UNMAPPED},
	{"Fantasy   ", 99, MIDI_UNMAPPED},    // + (PQ2)
	{"Fantasy2MS", 99, MIDI_UNMAPPED},    // ++ (Camelot, QFG1)
	{"Filter  MS", 95, MIDI_UNMAPPED},    // +++ (Iceman)
	{"Filter2 MS", 95, MIDI_UNMAPPED},    // ++ (Iceman)
	{"Flame2  MS", 121, MIDI_UNMAPPED},   // ? (QFG1)
	{"Flames  MS", 121, MIDI_UNMAPPED},   // ? (QFG1)
	{"Flute   MS", 73, MIDI_UNMAPPED},    // +++ (QFG1)
	{"FogHorn MS", 58, MIDI_UNMAPPED},
	{"FrHorn1 MS", 60, MIDI_UNMAPPED},    // +++ (QFG1)
	{"FunnyTrmp ", 56, MIDI_UNMAPPED},    // ++ (LB1)
	{"GameSnd MS", 80, MIDI_UNMAPPED},
	{"Glock   MS", 9, MIDI_UNMAPPED},     // +++ (QFG1)
	{"Gunshot   ", 127, MIDI_UNMAPPED},   // +++ (LB1)
	{"Hammer  MS", MIDI_UNMAPPED, MIDI_UNMAPPED}, // ? (QFG1)
	{"Harmonica2", 22, MIDI_UNMAPPED},    // +++ (LB1)
	{"Harpsi 1  ", 6, MIDI_UNMAPPED},     // + (Hoyle)
	{"Harpsi 2  ", 6, MIDI_UNMAPPED},     // +++ (LB1)
	{"Heart   MS", 116, MIDI_UNMAPPED},   // ? (Iceman)
	{"Horse1  MS", 115, MIDI_UNMAPPED},   // ? (Camelot, QFG1)
	{"Horse2  MS", 115, MIDI_UNMAPPED},   // ? (Camelot, QFG1)
	{"InHale  MS", 121, MIDI_UNMAPPED},   // ++ (Iceman)
	{"KNIFE     ", 120, MIDI_UNMAPPED},   // ? (LSL3)
	{"KenBanjo  ", 105, MIDI_UNMAPPED},   // +++ (LB1)
	{"Kiss    MS", 25, MIDI_UNMAPPED},    // ++ (QFG1)
	{"KongHit   ", MIDI_UNMAPPED, MIDI_UNMAPPED}, // ??? (KQ1)
	{"Koto      ", 107, MIDI_UNMAPPED},   // +++ (PQ2)
	{"Laser   MS", 81, MIDI_UNMAPPED},    // ?? (QFG1)
	{"Meeps   MS", 62, MIDI_UNMAPPED},    // ? (QFG1)
	{"MTrak   MS", 62, MIDI_UNMAPPED},    // ?? (Iceman)
	{"MachGun MS", 127, MIDI_UNMAPPED},   // ? (Iceman)
	{"OCEANSOUND", 122, MIDI_UNMAPPED},   // + (LSL3)
	{"Oboe 2001 ", 68, MIDI_UNMAPPED},    // + (PQ2)
	{"Ocean   MS", 122, MIDI_UNMAPPED},   // + (Iceman)
	{"PPG 2.3 MS", 75, MIDI_UNMAPPED},    // ? (Iceman)
	{"PianoCrank", MIDI_UNMAPPED, MIDI_UNMAPPED}, // ? (LB1)
	{"PicSnareMS", MIDI_MAPPED_TO_RHYTHM, 40},   // R ? (Iceman)
	{"PiccoloKA ", 72, MIDI_UNMAPPED},    // +++ (KQ1)
	{"PinkBassMS", 39, MIDI_UNMAPPED},
	{"Pizz2     ", 45, MIDI_UNMAPPED},    // ++ (LB1)
	{"Portcullis", MIDI_UNMAPPED, MIDI_UNMAPPED}, // ? (KQ1)
	{"Raspbry MS", 81, MIDI_UNMAPPED},    // ? (QFG1)
	{"RatSqueek ", 72, MIDI_UNMAPPED},    // ? (LauraBow1, Camelot)
	{"Record78  ", MIDI_UNMAPPED, MIDI_UNMAPPED}, // +++ (LB1)
	{"RecorderMS", 74, MIDI_UNMAPPED},    // +++ (Camelot)
	{"Red Baron ", 125, MIDI_UNMAPPED},   // ? (LB1)
	{"ReedPipMS ", 20, MIDI_UNMAPPED},    // +++ (Camelot)
	{"RevCymb MS", 119, MIDI_UNMAPPED},
	{"RifleShot ", 127, MIDI_UNMAPPED},   // + (LB1)
	{"RimShot MS", MIDI_MAPPED_TO_RHYTHM, 37},   // R
	{"SHOWER    ", 52, MIDI_UNMAPPED},    // ? (LSL3)
	{"SQ Bass MS", 32, MIDI_UNMAPPED},    // + (SQ3)
	{"ShakuVibMS", 79, MIDI_UNMAPPED},    // + (Iceman)
	{"SlapBassMS", 36, MIDI_UNMAPPED},    // +++ (Iceman)
	{"Snare   MS", MIDI_MAPPED_TO_RHYTHM, 38},   // R (QFG1)
	{"Some Birds", 123, MIDI_UNMAPPED},   // + (LB1)
	{"Sonar   MS", 78, MIDI_UNMAPPED},    // ? (Iceman)
	{"Soundtrk2 ", 97, MIDI_UNMAPPED},    // +++ (LB1)
	{"Soundtrack", 97, MIDI_UNMAPPED},    // ++ (Camelot)
	{"SqurWaveMS", 80, MIDI_UNMAPPED},
	{"StabBassMS", 34, MIDI_UNMAPPED},    // + (Iceman)
	{"SteelDrmMS", 114, MIDI_UNMAPPED},   // +++ (Iceman)
	{"StrSect1MS", 48, MIDI_UNMAPPED},    // ++ (QFG1)
	{"String  MS", 45, MIDI_UNMAPPED},    // + (Camelot)
	{"Syn-Choir ", 91, MIDI_UNMAPPED},
	{"Syn Brass4", 63, MIDI_UNMAPPED},    // ++ (PQ2)
	{"SynBass MS", 38, MIDI_UNMAPPED},
	{"SwmpBackgr", 120, MIDI_UNMAPPED},    // ?? (LB1, QFG1)
	{"T-Bone2 MS", 57, MIDI_UNMAPPED},    // +++ (QFG1)
	{"Taiko     ", 116, 35},      // +++ (Camelot)
	{"Taiko Rim ", 118, 37},      // +++ (LSL3)
	{"Timpani1  ", 47, MIDI_UNMAPPED},    // +++ (LB1)
	{"Tom     MS", 117, 48},      // +++ (Iceman)
	{"Toms    MS", 117, 48},      // +++ (Camelot, QFG1)
	{"Tpt1prtl  ", 56, MIDI_UNMAPPED},    // +++ (KQ1)
	{"TriangleMS", 112, 81},      // R (Camelot)
	{"Trumpet 1 ", 56, MIDI_UNMAPPED},    // +++ (Camelot)
	{"Type    MS", MIDI_MAPPED_TO_RHYTHM, 39},   // + (Iceman)
	{"Warm Pad"  , 89, MIDI_UNMAPPED},	// ++ (PQ3)
	{"WaterBells", 98, MIDI_UNMAPPED},    // + (PQ2)
	{"WaterFallK", MIDI_UNMAPPED, MIDI_UNMAPPED}, // ? (KQ1)
	{"Whiporill ", 123, MIDI_UNMAPPED},   // + (LB1)
	{"Wind      ", MIDI_UNMAPPED, MIDI_UNMAPPED}, // ? (LB1)
	{"Wind    MS", MIDI_UNMAPPED, MIDI_UNMAPPED}, // ? (QFG1, Iceman)
	{"Wind2   MS", MIDI_UNMAPPED, MIDI_UNMAPPED}, // ? (Camelot)
	{"Woodpecker", 115, MIDI_UNMAPPED},   // ? (LB1)
	{"WtrFall MS", MIDI_UNMAPPED, MIDI_UNMAPPED}, // ? (Camelot, QFG1, Iceman)
	{0, 0, 0}
};

	typedef Common::List<Mt32ToGmMap> Mt32ToGmMapList;
	extern Mt32ToGmMapList *Mt32dynamicMappings;

} // End of namespace Sci

#endif // SCI_SOUND_DRIVERS_MAP_MT32_TO_GM_H
