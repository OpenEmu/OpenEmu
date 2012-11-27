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

#ifndef SCI_SOUND_DRIVERS_GM_NAMES_H
#define SCI_SOUND_DRIVERS_GM_NAMES_H

namespace Sci {

// These tables are only used for debugging. Don't include them for devices
// with not enough available memory (e.g. phones), where REDUCE_MEMORY_USAGE
// is defined
#ifndef REDUCE_MEMORY_USAGE

static const char *const GmInstrumentNames[] = {
	/*000*/  "Acoustic Grand Piano",
	/*001*/  "Bright Acoustic Piano",
	/*002*/  "Electric Grand Piano",
	/*003*/  "Honky-tonk Piano",
	/*004*/  "Electric Piano 1",
	/*005*/  "Electric Piano 2",
	/*006*/  "Harpsichord",
	/*007*/  "Clavinet",
	/*008*/  "Celesta",
	/*009*/  "Glockenspiel",
	/*010*/  "Music Box",
	/*011*/  "Vibraphone",
	/*012*/  "Marimba",
	/*013*/  "Xylophone",
	/*014*/  "Tubular Bells",
	/*015*/  "Dulcimer",
	/*016*/  "Drawbar Organ",
	/*017*/  "Percussive Organ",
	/*018*/  "Rock Organ",
	/*019*/  "Church Organ",
	/*020*/  "Reed Organ",
	/*021*/  "Accordion",
	/*022*/  "Harmonica",
	/*023*/  "Tango Accordion",
	/*024*/  "Acoustic Guitar (nylon)",
	/*025*/  "Acoustic Guitar (steel)",
	/*026*/  "Electric Guitar (jazz)",
	/*027*/  "Electric Guitar (clean)",
	/*028*/  "Electric Guitar (muted)",
	/*029*/  "Overdriven Guitar",
	/*030*/  "Distortion Guitar",
	/*031*/  "Guitar Harmonics",
	/*032*/  "Acoustic Bass",
	/*033*/  "Electric Bass (finger)",
	/*034*/  "Electric Bass (pick)",
	/*035*/  "Fretless Bass",
	/*036*/  "Slap Bass 1",
	/*037*/  "Slap Bass 2",
	/*038*/  "Synth Bass 1",
	/*039*/  "Synth Bass 2",
	/*040*/  "Violin",
	/*041*/  "Viola",
	/*042*/  "Cello",
	/*043*/  "Contrabass",
	/*044*/  "Tremolo Strings",
	/*045*/  "Pizzicato Strings",
	/*046*/  "Orchestral Harp",
	/*047*/  "Timpani",
	/*048*/  "String Ensemble 1",
	/*049*/  "String Ensemble 2",
	/*050*/  "SynthStrings 1",
	/*051*/  "SynthStrings 2",
	/*052*/  "Choir Aahs",
	/*053*/  "Voice Oohs",
	/*054*/  "Synth Voice",
	/*055*/  "Orchestra Hit",
	/*056*/  "Trumpet",
	/*057*/  "Trombone",
	/*058*/  "Tuba",
	/*059*/  "Muted Trumpet",
	/*060*/  "French Horn",
	/*061*/  "Brass Section",
	/*062*/  "SynthBrass 1",
	/*063*/  "SynthBrass 2",
	/*064*/  "Soprano Sax",
	/*065*/  "Alto Sax",
	/*066*/  "Tenor Sax",
	/*067*/  "Baritone Sax",
	/*068*/  "Oboe",
	/*069*/  "English Horn",
	/*070*/  "Bassoon",
	/*071*/  "Clarinet",
	/*072*/  "Piccolo",
	/*073*/  "Flute",
	/*074*/  "Recorder",
	/*075*/  "Pan Flute",
	/*076*/  "Blown Bottle",
	/*077*/  "Shakuhachi",
	/*078*/  "Whistle",
	/*079*/  "Ocarina",
	/*080*/  "Lead 1 (square)",
	/*081*/  "Lead 2 (sawtooth)",
	/*082*/  "Lead 3 (calliope)",
	/*083*/  "Lead 4 (chiff)",
	/*084*/  "Lead 5 (charang)",
	/*085*/  "Lead 6 (voice)",
	/*086*/  "Lead 7 (fifths)",
	/*087*/  "Lead 8 (bass+lead)",
	/*088*/  "Pad 1 (new age)",
	/*089*/  "Pad 2 (warm)",
	/*090*/  "Pad 3 (polysynth)",
	/*091*/  "Pad 4 (choir)",
	/*092*/  "Pad 5 (bowed)",
	/*093*/  "Pad 6 (metallic)",
	/*094*/  "Pad 7 (halo)",
	/*095*/  "Pad 8 (sweep)",
	/*096*/  "FX 1 (rain)",
	/*097*/  "FX 2 (soundtrack)",
	/*098*/  "FX 3 (crystal)",
	/*099*/  "FX 4 (atmosphere)",
	/*100*/  "FX 5 (brightness)",
	/*101*/  "FX 6 (goblins)",
	/*102*/  "FX 7 (echoes)",
	/*103*/  "FX 8 (sci-fi)",
	/*104*/  "Sitar",
	/*105*/  "Banjo",
	/*106*/  "Shamisen",
	/*107*/  "Koto",
	/*108*/  "Kalimba",
	/*109*/  "Bag pipe",
	/*110*/  "Fiddle",
	/*111*/  "Shannai",
	/*112*/  "Tinkle Bell",
	/*113*/  "Agogo",
	/*114*/  "Steel Drums",
	/*115*/  "Woodblock",
	/*116*/  "Taiko Drum",
	/*117*/  "Melodic Tom",
	/*118*/  "Synth Drum",
	/*119*/  "Reverse Cymbal",
	/*120*/  "Guitar Fret Noise",
	/*121*/  "Breath Noise",
	/*122*/  "Seashore",
	/*123*/  "Bird Tweet",
	/*124*/  "Telephone Ring",
	/*125*/  "Helicopter",
	/*126*/  "Applause",
	/*127*/  "Gunshot"
};

// The GM Percussion map is downwards compatible to the MT32 map, which is used in SCI
static const char *const GmPercussionNames[] = {
	/*00*/  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/*10*/  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/*20*/  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/*30*/  0, 0, 0, 0, 0,
	// The preceeding percussions are not covered by the GM standard
	/*35*/  "Acoustic Bass Drum",
	/*36*/  "Bass Drum 1",
	/*37*/  "Side Stick",
	/*38*/  "Acoustic Snare",
	/*39*/  "Hand Clap",
	/*40*/  "Electric Snare",
	/*41*/  "Low Floor Tom",
	/*42*/  "Closed Hi-Hat",
	/*43*/  "High Floor Tom",
	/*44*/  "Pedal Hi-Hat",
	/*45*/  "Low Tom",
	/*46*/  "Open Hi-Hat",
	/*47*/  "Low-Mid Tom",
	/*48*/  "Hi-Mid Tom",
	/*49*/  "Crash Cymbal 1",
	/*50*/  "High Tom",
	/*51*/  "Ride Cymbal 1",
	/*52*/  "Chinese Cymbal",
	/*53*/  "Ride Bell",
	/*54*/  "Tambourine",
	/*55*/  "Splash Cymbal",
	/*56*/  "Cowbell",
	/*57*/  "Crash Cymbal 2",
	/*58*/  "Vibraslap",
	/*59*/  "Ride Cymbal 2",
	/*60*/  "Hi Bongo",
	/*61*/  "Low Bongo",
	/*62*/  "Mute Hi Conga",
	/*63*/  "Open Hi Conga",
	/*64*/  "Low Conga",
	/*65*/  "High Timbale",
	/*66*/  "Low Timbale",
	/*67*/  "High Agogo",
	/*68*/  "Low Agogo",
	/*69*/  "Cabasa",
	/*70*/  "Maracas",
	/*71*/  "Short Whistle",
	/*72*/  "Long Whistle",
	/*73*/  "Short Guiro",
	/*74*/  "Long Guiro",
	/*75*/  "Claves",
	/*76*/  "Hi Wood Block",
	/*77*/  "Low Wood Block",
	/*78*/  "Mute Cuica",
	/*79*/  "Open Cuica",
	/*80*/  "Mute Triangle",
	/*81*/  "Open Triangle"
};

#endif	// REDUCE_MEMORY_USAGE

} // End of namespace Sci

#endif // SCI_SOUND_DRIVERS_GM_NAMES_H
