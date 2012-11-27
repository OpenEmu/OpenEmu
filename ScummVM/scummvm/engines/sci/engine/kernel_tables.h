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

#ifndef SCI_ENGINE_KERNEL_TABLES_H
#define SCI_ENGINE_KERNEL_TABLES_H

#include "sci/engine/workarounds.h"
#include "sci/engine/vm_types.h" // for opcode_formats

namespace Sci {

// [io] -> either integer or object
// (io) -> optionally integer AND an object
// (i) -> optional integer
// . -> any type
// i* -> optional multiple integers
// .* -> any parameters afterwards (or none)

struct SciKernelMapSubEntry {
	SciVersion fromVersion;
	SciVersion toVersion;

	uint16 id;

	const char *name;
	KernelFunctionCall *function;

	const char *signature;
	const SciWorkaroundEntry *workarounds;
};

#define SCI_SUBOPENTRY_TERMINATOR { SCI_VERSION_NONE, SCI_VERSION_NONE, 0, NULL, NULL, NULL, NULL }


#define SIG_SCIALL         SCI_VERSION_NONE, SCI_VERSION_NONE
#define SIG_SCI0           SCI_VERSION_NONE, SCI_VERSION_01
#define SIG_SCI1           SCI_VERSION_1_EGA_ONLY, SCI_VERSION_1_LATE
#define SIG_SCI11          SCI_VERSION_1_1, SCI_VERSION_1_1
#define SIG_SINCE_SCI11    SCI_VERSION_1_1, SCI_VERSION_NONE
#define SIG_SCI21          SCI_VERSION_2_1, SCI_VERSION_3

#define SIG_SCI16          SCI_VERSION_NONE, SCI_VERSION_1_1
#define SIG_SCI32          SCI_VERSION_2, SCI_VERSION_NONE

// SCI-Sound-Version
#define SIG_SOUNDSCI0      SCI_VERSION_0_EARLY, SCI_VERSION_0_LATE
#define SIG_SOUNDSCI1EARLY SCI_VERSION_1_EARLY, SCI_VERSION_1_EARLY
#define SIG_SOUNDSCI1LATE  SCI_VERSION_1_LATE, SCI_VERSION_1_LATE
#define SIG_SOUNDSCI21     SCI_VERSION_2_1, SCI_VERSION_3

#define SIGFOR_ALL   0x3f
#define SIGFOR_DOS   1 << 0
#define SIGFOR_PC98  1 << 1
#define SIGFOR_WIN   1 << 2
#define SIGFOR_MAC   1 << 3
#define SIGFOR_AMIGA 1 << 4
#define SIGFOR_ATARI 1 << 5
#define SIGFOR_PC    SIGFOR_DOS|SIGFOR_WIN

#define SIG_EVERYWHERE  SIG_SCIALL, SIGFOR_ALL

#define MAP_CALL(_name_) #_name_, k##_name_
#define MAP_EMPTY(_name_) #_name_, kEmpty
#define MAP_DUMMY(_name_) #_name_, kDummy

//    version,         subId, function-mapping,                    signature,              workarounds
static const SciKernelMapSubEntry kDoSound_subops[] = {
	{ SIG_SOUNDSCI0,       0, MAP_CALL(DoSoundInit),               "o",                    NULL },
	{ SIG_SOUNDSCI0,       1, MAP_CALL(DoSoundPlay),               "o",                    NULL },
	{ SIG_SOUNDSCI0,       2, MAP_CALL(DoSoundRestore),            "(o)",                  NULL },
	{ SIG_SOUNDSCI0,       3, MAP_CALL(DoSoundDispose),            "o",                    NULL },
	{ SIG_SOUNDSCI0,       4, MAP_CALL(DoSoundMute),               "(i)",                  NULL },
	{ SIG_SOUNDSCI0,       5, MAP_CALL(DoSoundStop),               "o",                    NULL },
	{ SIG_SOUNDSCI0,       6, MAP_CALL(DoSoundPause),              "i",                    NULL },
	{ SIG_SOUNDSCI0,       7, MAP_CALL(DoSoundResumeAfterRestore), "",                     NULL },
	{ SIG_SOUNDSCI0,       8, MAP_CALL(DoSoundMasterVolume),       "(i)",                  NULL },
	{ SIG_SOUNDSCI0,       9, MAP_CALL(DoSoundUpdate),             "o",                    NULL },
	{ SIG_SOUNDSCI0,      10, MAP_CALL(DoSoundFade),               "[o0]",                 kDoSoundFade_workarounds },
	{ SIG_SOUNDSCI0,      11, MAP_CALL(DoSoundGetPolyphony),       "",                     NULL },
	{ SIG_SOUNDSCI0,      12, MAP_CALL(DoSoundStopAll),            "",                     NULL },
	{ SIG_SOUNDSCI1EARLY,  0, MAP_CALL(DoSoundMasterVolume),       NULL,                   NULL },
	{ SIG_SOUNDSCI1EARLY,  1, MAP_CALL(DoSoundMute),               NULL,                   NULL },
	{ SIG_SOUNDSCI1EARLY,  2, MAP_CALL(DoSoundRestore),            NULL,                   NULL },
	{ SIG_SOUNDSCI1EARLY,  3, MAP_CALL(DoSoundGetPolyphony),       NULL,                   NULL },
	{ SIG_SOUNDSCI1EARLY,  4, MAP_CALL(DoSoundUpdate),             NULL,                   NULL },
	{ SIG_SOUNDSCI1EARLY,  5, MAP_CALL(DoSoundInit),               NULL,                   NULL },
	{ SIG_SOUNDSCI1EARLY,  6, MAP_CALL(DoSoundDispose),            NULL,                   NULL },
	{ SIG_SOUNDSCI1EARLY,  7, MAP_CALL(DoSoundPlay),               "oi",                   NULL },
	// ^^ TODO: In SCI1-SCI1.1 DoSound (play) is called by 2 methods of the Sound object: play and
	//  playBed. The methods are the same, apart from the second integer parameter: it's 0 in
	//  play and 1 in playBed, to distinguish the caller. It's passed on, we should find out what
	//  it actually does internally
	{ SIG_SOUNDSCI1EARLY,  8, MAP_CALL(DoSoundStop),               NULL,                   NULL },
	{ SIG_SOUNDSCI1EARLY,  9, MAP_CALL(DoSoundPause),              "[o0]i",                NULL },
	{ SIG_SOUNDSCI1EARLY, 10, MAP_CALL(DoSoundFade),               "oiiii",                kDoSoundFade_workarounds },
	{ SIG_SOUNDSCI1EARLY, 11, MAP_CALL(DoSoundUpdateCues),         "o",                    NULL },
	{ SIG_SOUNDSCI1EARLY, 12, MAP_CALL(DoSoundSendMidi),           "oiii",                 NULL },
	{ SIG_SOUNDSCI1EARLY, 13, MAP_CALL(DoSoundGlobalReverb),       "(i)",                  NULL },
	{ SIG_SOUNDSCI1EARLY, 14, MAP_CALL(DoSoundSetHold),            "oi",                   NULL },
	{ SIG_SOUNDSCI1EARLY, 15, MAP_CALL(DoSoundDummy),              "",                     NULL },
	//  ^^ Longbow demo
	{ SIG_SOUNDSCI1LATE,   0, MAP_CALL(DoSoundMasterVolume),       NULL,                   NULL },
	{ SIG_SOUNDSCI1LATE,   1, MAP_CALL(DoSoundMute),               NULL,                   NULL },
	{ SIG_SOUNDSCI1LATE,   2, MAP_CALL(DoSoundRestore),            "",                     NULL },
	{ SIG_SOUNDSCI1LATE,   3, MAP_CALL(DoSoundGetPolyphony),       NULL,                   NULL },
	{ SIG_SOUNDSCI1LATE,   4, MAP_CALL(DoSoundGetAudioCapability), "",                     NULL },
	{ SIG_SOUNDSCI1LATE,   5, MAP_CALL(DoSoundSuspend),            "i",                    NULL },
	{ SIG_SOUNDSCI1LATE,   6, MAP_CALL(DoSoundInit),               NULL,                   NULL },
	{ SIG_SOUNDSCI1LATE,   7, MAP_CALL(DoSoundDispose),            NULL,                   NULL },
	{ SIG_SOUNDSCI1LATE,   8, MAP_CALL(DoSoundPlay),               NULL,                   NULL },
	{ SIG_SOUNDSCI1LATE,   9, MAP_CALL(DoSoundStop),               NULL,                   NULL },
	{ SIG_SOUNDSCI1LATE,  10, MAP_CALL(DoSoundPause),              NULL,                   NULL },
	{ SIG_SOUNDSCI1LATE,  11, MAP_CALL(DoSoundFade),               "oiiii(i)",             kDoSoundFade_workarounds },
	{ SIG_SOUNDSCI1LATE,  12, MAP_CALL(DoSoundSetHold),            NULL,                   NULL },
	{ SIG_SOUNDSCI1LATE,  13, MAP_CALL(DoSoundDummy),              NULL,                   NULL },
	{ SIG_SOUNDSCI1LATE,  14, MAP_CALL(DoSoundSetVolume),          "oi",                   NULL },
	{ SIG_SOUNDSCI1LATE,  15, MAP_CALL(DoSoundSetPriority),        "oi",                   NULL },
	{ SIG_SOUNDSCI1LATE,  16, MAP_CALL(DoSoundSetLoop),            "oi",                   NULL },
	{ SIG_SOUNDSCI1LATE,  17, MAP_CALL(DoSoundUpdateCues),         NULL,                   NULL },
	{ SIG_SOUNDSCI1LATE,  18, MAP_CALL(DoSoundSendMidi),           "oiii(i)",              NULL },
	{ SIG_SOUNDSCI1LATE,  19, MAP_CALL(DoSoundGlobalReverb),       NULL,                   NULL },
	{ SIG_SOUNDSCI1LATE,  20, MAP_CALL(DoSoundUpdate),             NULL,                   NULL },
#ifdef ENABLE_SCI32
	{ SIG_SOUNDSCI21,      0, MAP_CALL(DoSoundMasterVolume),       NULL,                   NULL },
	{ SIG_SOUNDSCI21,      1, MAP_CALL(DoSoundMute),               NULL,                   NULL },
	{ SIG_SOUNDSCI21,      2, MAP_CALL(DoSoundRestore),            NULL,                   NULL },
	{ SIG_SOUNDSCI21,      3, MAP_CALL(DoSoundGetPolyphony),       NULL,                   NULL },
	{ SIG_SOUNDSCI21,      4, MAP_CALL(DoSoundGetAudioCapability), NULL,                   NULL },
	{ SIG_SOUNDSCI21,      5, MAP_CALL(DoSoundSuspend),            NULL,                   NULL },
	{ SIG_SOUNDSCI21,      6, MAP_CALL(DoSoundInit),               NULL,                   NULL },
	{ SIG_SOUNDSCI21,      7, MAP_CALL(DoSoundDispose),            NULL,                   NULL },
	{ SIG_SOUNDSCI21,      8, MAP_CALL(DoSoundPlay),               "o(i)",                 NULL },
	// ^^ TODO: if this is really the only change between SCI1LATE AND SCI21, we could rename the
	//     SIG_SOUNDSCI1LATE #define to SIG_SINCE_SOUNDSCI1LATE and make it being SCI1LATE+. Although
	//     I guess there are many more changes somewhere
	// TODO: Quest for Glory 4 (SCI2.1) uses the old scheme, we need to detect it accordingly
	//        signature for SCI21 should be "o"
	{ SIG_SOUNDSCI21,      9, MAP_CALL(DoSoundStop),               NULL,                   NULL },
	{ SIG_SOUNDSCI21,     10, MAP_CALL(DoSoundPause),              NULL,                   NULL },
	{ SIG_SOUNDSCI21,     11, MAP_CALL(DoSoundFade),               NULL,                   NULL },
	{ SIG_SOUNDSCI21,     12, MAP_CALL(DoSoundSetHold),            NULL,                   NULL },
	{ SIG_SOUNDSCI21,     13, MAP_CALL(DoSoundDummy),              NULL,                   NULL },
	{ SIG_SOUNDSCI21,     14, MAP_CALL(DoSoundSetVolume),          NULL,                   NULL },
	{ SIG_SOUNDSCI21,     15, MAP_CALL(DoSoundSetPriority),        NULL,                   NULL },
	{ SIG_SOUNDSCI21,     16, MAP_CALL(DoSoundSetLoop),            NULL,                   NULL },
	{ SIG_SOUNDSCI21,     17, MAP_CALL(DoSoundUpdateCues),         NULL,                   NULL },
	{ SIG_SOUNDSCI21,     18, MAP_CALL(DoSoundSendMidi),           NULL,                   NULL },
	{ SIG_SOUNDSCI21,     19, MAP_CALL(DoSoundGlobalReverb),       NULL,                   NULL },
	{ SIG_SOUNDSCI21,     20, MAP_CALL(DoSoundUpdate),             NULL,                   NULL },
#endif
	SCI_SUBOPENTRY_TERMINATOR
};

//    version,         subId, function-mapping,                    signature,              workarounds
static const SciKernelMapSubEntry kGraph_subops[] = {
	{ SIG_SCI32,           1, MAP_CALL(StubNull),                  "",                     NULL }, // called by gk1 sci32 right at the start
	{ SIG_SCIALL,          2, MAP_CALL(GraphGetColorCount),        "",                     NULL },
	// 3 - set palette via resource
	{ SIG_SCIALL,          4, MAP_CALL(GraphDrawLine),             "iiiii(i)(i)",          kGraphDrawLine_workarounds },
	// 5 - nop
	// 6 - draw pattern
	{ SIG_SCIALL,          7, MAP_CALL(GraphSaveBox),              "iiiii",                kGraphSaveBox_workarounds },
	{ SIG_SCIALL,          8, MAP_CALL(GraphRestoreBox),           "[r0!]",                kGraphRestoreBox_workarounds },
	// ^ this may get called with invalid references, we check them within restoreBits() and sierra sci behaves the same
	{ SIG_SCIALL,          9, MAP_CALL(GraphFillBoxBackground),    "iiii",                 NULL },
	{ SIG_SCIALL,         10, MAP_CALL(GraphFillBoxForeground),    "iiii",                 kGraphFillBoxForeground_workarounds },
	{ SIG_SCIALL,         11, MAP_CALL(GraphFillBoxAny),           "iiiiii(i)(i)",         kGraphFillBoxAny_workarounds },
	{ SIG_SCI11,          12, MAP_CALL(GraphUpdateBox),            "iiii(i)(r0)",          kGraphUpdateBox_workarounds }, // kq6 hires
	{ SIG_SCIALL,         12, MAP_CALL(GraphUpdateBox),            "iiii(i)",              kGraphUpdateBox_workarounds },
	{ SIG_SCIALL,         13, MAP_CALL(GraphRedrawBox),            "iiii",                 kGraphRedrawBox_workarounds },
	{ SIG_SCIALL,         14, MAP_CALL(GraphAdjustPriority),       "ii",                   NULL },
	{ SIG_SCI11,          15, MAP_CALL(GraphSaveUpscaledHiresBox), "iiii",                 NULL }, // kq6 hires
	SCI_SUBOPENTRY_TERMINATOR
};

//    version,         subId, function-mapping,                    signature,              workarounds
static const SciKernelMapSubEntry kPalVary_subops[] = {
	{ SIG_SCI21,           0, MAP_CALL(PalVaryInit),               "ii(i)(i)(i)",          NULL },
	{ SIG_SCIALL,          0, MAP_CALL(PalVaryInit),               "ii(i)(i)",             NULL },
	{ SIG_SCIALL,          1, MAP_CALL(PalVaryReverse),            "(i)(i)(i)",            NULL },
	{ SIG_SCIALL,          2, MAP_CALL(PalVaryGetCurrentStep),     "",                     NULL },
	{ SIG_SCIALL,          3, MAP_CALL(PalVaryDeinit),             "",                     NULL },
	{ SIG_SCIALL,          4, MAP_CALL(PalVaryChangeTarget),       "i",                    NULL },
	{ SIG_SCIALL,          5, MAP_CALL(PalVaryChangeTicks),        "i",                    NULL },
	{ SIG_SCIALL,          6, MAP_CALL(PalVaryPauseResume),        "i",                    NULL },
	{ SIG_SCI32,           8, MAP_CALL(PalVaryUnknown),            "i",                    NULL },
	SCI_SUBOPENTRY_TERMINATOR
};

//    version,         subId, function-mapping,                    signature,              workarounds
static const SciKernelMapSubEntry kPalette_subops[] = {
	{ SIG_SCIALL,          1, MAP_CALL(PaletteSetFromResource),    "i(i)",                 NULL },
	{ SIG_SCIALL,          2, MAP_CALL(PaletteSetFlag),            "iii",                  NULL },
	{ SIG_SCIALL,          3, MAP_CALL(PaletteUnsetFlag),          "iii",                  kPaletteUnsetFlag_workarounds },
	{ SIG_SCIALL,          4, MAP_CALL(PaletteSetIntensity),       "iii(i)",               NULL },
	{ SIG_SCIALL,          5, MAP_CALL(PaletteFindColor),          "iii",                  NULL },
	{ SIG_SCIALL,          6, MAP_CALL(PaletteAnimate),            "i*",                   NULL },
	{ SIG_SCIALL,          7, MAP_CALL(PaletteSave),               "",                     NULL },
	{ SIG_SCIALL,          8, MAP_CALL(PaletteRestore),            "[r0]",                 NULL },
	SCI_SUBOPENTRY_TERMINATOR
};

static const SciKernelMapSubEntry kFileIO_subops[] = {
	{ SIG_SCI32,           0, MAP_CALL(FileIOOpen),                "r(i)",                 NULL },
	{ SIG_SCIALL,          0, MAP_CALL(FileIOOpen),                "ri",                   NULL },
	{ SIG_SCIALL,          1, MAP_CALL(FileIOClose),               "i",                    NULL },
	{ SIG_SCIALL,          2, MAP_CALL(FileIOReadRaw),             "iri",                  NULL },
	{ SIG_SCIALL,          3, MAP_CALL(FileIOWriteRaw),            "iri",                  NULL },
	{ SIG_SCIALL,          4, MAP_CALL(FileIOUnlink),              "r",                    NULL },
	{ SIG_SCIALL,          5, MAP_CALL(FileIOReadString),          "rii",                  NULL },
	{ SIG_SCIALL,          6, MAP_CALL(FileIOWriteString),         "ir",                   NULL },
	{ SIG_SCIALL,          7, MAP_CALL(FileIOSeek),                "iii",                  NULL },
	{ SIG_SCIALL,          8, MAP_CALL(FileIOFindFirst),           "rri",                  NULL },
	{ SIG_SCIALL,          9, MAP_CALL(FileIOFindNext),            "r",                    NULL },
	{ SIG_SCIALL,         10, MAP_CALL(FileIOExists),              "r",                    NULL },
	{ SIG_SINCE_SCI11,    11, MAP_CALL(FileIORename),              "rr",                   NULL },
#ifdef ENABLE_SCI32
	{ SIG_SCI32,          13, MAP_CALL(FileIOReadByte),            "i",                    NULL },
	{ SIG_SCI32,          14, MAP_CALL(FileIOWriteByte),           "ii",                   NULL },
	{ SIG_SCI32,          15, MAP_CALL(FileIOReadWord),            "i",                    NULL },
	{ SIG_SCI32,          16, MAP_CALL(FileIOWriteWord),           "ii",                   NULL },
	{ SIG_SCI32,          17, MAP_CALL(FileIOCreateSaveSlot),      "ir",                   NULL },
	{ SIG_SCI32,          18, MAP_EMPTY(FileIOChangeDirectory),    "r",                    NULL }, // for SQ6, when changing the savegame directory in the save/load dialog
	{ SIG_SCI32,          19, MAP_CALL(FileIOIsValidDirectory),    "r",                    NULL }, // for Torin / Torin demo
#endif
	SCI_SUBOPENTRY_TERMINATOR
};

#ifdef ENABLE_SCI32

static const SciKernelMapSubEntry kSave_subops[] = {
	{ SIG_SCI32,           0, MAP_CALL(SaveGame),                  "[r0]i[r0](r0)",        NULL },
	{ SIG_SCI32,           1, MAP_CALL(RestoreGame),               "[r0]i[r0]",            NULL },
	{ SIG_SCI32,           2, MAP_CALL(GetSaveDir),                "(r*)",                 NULL },
	{ SIG_SCI32,           3, MAP_CALL(CheckSaveGame),             ".*",                   NULL },
	// Subop 4 hasn't been encountered yet
	{ SIG_SCI32,           5, MAP_CALL(GetSaveFiles),              "rrr",                  NULL },
	{ SIG_SCI32,           6, MAP_CALL(MakeSaveCatName),           "rr",                   NULL },
	{ SIG_SCI32,           7, MAP_CALL(MakeSaveFileName),          "rri",                  NULL },
	{ SIG_SCI32,           8, MAP_CALL(AutoSave),                  "[o0]",                 NULL },
	SCI_SUBOPENTRY_TERMINATOR
};

//    version,         subId, function-mapping,                    signature,              workarounds
static const SciKernelMapSubEntry kList_subops[] = {
	{ SIG_SCI21,           0, MAP_CALL(NewList),                   "",                     NULL },
	{ SIG_SCI21,           1, MAP_CALL(DisposeList),               "l",                    NULL },
	{ SIG_SCI21,           2, MAP_CALL(NewNode),                   ".(.)",                 NULL },
	{ SIG_SCI21,           3, MAP_CALL(FirstNode),                 "[l0]",                 NULL },
	{ SIG_SCI21,           4, MAP_CALL(LastNode),                  "l",                    NULL },
	{ SIG_SCI21,           5, MAP_CALL(EmptyList),                 "l",                    NULL },
	{ SIG_SCI21,           6, MAP_CALL(NextNode),                  "n",                    NULL },
	{ SIG_SCI21,           7, MAP_CALL(PrevNode),                  "n",                    NULL },
	{ SIG_SCI21,           8, MAP_CALL(NodeValue),                 "[n0]",                 NULL },
	{ SIG_SCI21,           9, MAP_CALL(AddAfter),                  "lnn.",                 NULL },
	{ SIG_SCI21,          10, MAP_CALL(AddToFront),                "ln.",                  NULL },
	{ SIG_SCI21,          11, MAP_CALL(AddToEnd),                  "ln(.)",                NULL },
	{ SIG_SCI21,          12, MAP_CALL(AddBefore),                 "ln.",                  NULL },
	{ SIG_SCI21,          13, MAP_CALL(MoveToFront),               "ln",                   NULL },
	{ SIG_SCI21,          14, MAP_CALL(MoveToEnd),                 "ln",                   NULL },
	{ SIG_SCI21,          15, MAP_CALL(FindKey),                   "l.",                   NULL },
	{ SIG_SCI21,          16, MAP_CALL(DeleteKey),                 "l.",                   NULL },
	{ SIG_SCI21,          17, MAP_CALL(ListAt),                    "li",                   NULL },
	{ SIG_SCI21,          18, MAP_CALL(ListIndexOf) ,              "l[io]",                NULL },
	{ SIG_SCI21,          19, MAP_CALL(ListEachElementDo),         "li(.*)",               NULL },
	{ SIG_SCI21,          20, MAP_CALL(ListFirstTrue),             "li(.*)",               NULL },
	{ SIG_SCI21,          21, MAP_CALL(ListAllTrue),               "li(.*)",               NULL },
	{ SIG_SCI21,          22, MAP_CALL(Sort),                      "ooo",                  NULL },
	SCI_SUBOPENTRY_TERMINATOR
};
#endif

struct SciKernelMapEntry {
	const char *name;
	KernelFunctionCall *function;

	SciVersion fromVersion;
	SciVersion toVersion;
	byte forPlatform;

	const char *signature;
	const SciKernelMapSubEntry *subFunctions;
	const SciWorkaroundEntry *workarounds;
};

//    name,                        version/platform,         signature,              sub-signatures,  workarounds
static SciKernelMapEntry s_kernelMap[] = {
	{ MAP_CALL(Abs),               SIG_EVERYWHERE,           "i",                     NULL,            kAbs_workarounds },
	{ MAP_CALL(AddAfter),          SIG_EVERYWHERE,           "lnn",                   NULL,            NULL },
	{ MAP_CALL(AddMenu),           SIG_EVERYWHERE,           "rr",                    NULL,            NULL },
	{ MAP_CALL(AddToEnd),          SIG_EVERYWHERE,           "ln",                    NULL,            NULL },
	{ MAP_CALL(AddToFront),        SIG_EVERYWHERE,           "ln",                    NULL,            NULL },
	{ MAP_CALL(AddToPic),          SIG_EVERYWHERE,           "[il](iiiiii)",          NULL,            NULL },
	{ MAP_CALL(Animate),           SIG_EVERYWHERE,           "(l0)(i)",               NULL,            NULL },
	{ MAP_CALL(AssertPalette),     SIG_EVERYWHERE,           "i",                     NULL,            NULL },
	{ MAP_CALL(AvoidPath),         SIG_EVERYWHERE,           "ii(.*)",                NULL,            NULL },
	{ MAP_CALL(BaseSetter),        SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(CanBeHere),         SIG_EVERYWHERE,           "o(l)",                  NULL,            NULL },
#ifdef ENABLE_SCI32
	{ "CantBeHere", kCantBeHere32, SIG_SCI32, SIGFOR_ALL,    "ol",                    NULL,            NULL },
#endif
	{ MAP_CALL(CantBeHere),        SIG_EVERYWHERE,           "o(l)",                  NULL,            NULL },
	{ MAP_CALL(CelHigh),           SIG_EVERYWHERE,           "ii(i)",                 NULL,            kCelHigh_workarounds },
	{ MAP_CALL(CelWide),           SIG_EVERYWHERE,           "ii(i)",                 NULL,            kCelWide_workarounds },
	{ MAP_CALL(CheckFreeSpace),    SIG_SCI32, SIGFOR_ALL,    "r.*",                   NULL,            NULL },
	{ MAP_CALL(CheckFreeSpace),    SIG_SCI11, SIGFOR_ALL,    "r(i)",                  NULL,            NULL },
	{ MAP_CALL(CheckFreeSpace),    SIG_EVERYWHERE,           "r",                     NULL,            NULL },
	{ MAP_CALL(CheckSaveGame),     SIG_EVERYWHERE,           ".*",                    NULL,            NULL },
	{ MAP_CALL(Clone),             SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(CoordPri),          SIG_EVERYWHERE,           "i(i)",                  NULL,            NULL },
	{ MAP_CALL(CosDiv),            SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
	{ MAP_CALL(DeleteKey),         SIG_EVERYWHERE,           "l.",                    NULL,            NULL },
	{ MAP_CALL(DeviceInfo),        SIG_EVERYWHERE,           "i(r)(r)(i)",            NULL,            kDeviceInfo_workarounds }, // subop
	{ MAP_CALL(Display),           SIG_EVERYWHERE,           "[ir]([ir!]*)",          NULL,            kDisplay_workarounds },
	// ^ we allow invalid references here, because kDisplay gets called with those in e.g. pq3 during intro
	//    restoreBits() checks and skips invalid handles, so that's fine. Sierra SCI behaved the same
	{ MAP_CALL(DirLoop),           SIG_EVERYWHERE,           "oi",                    NULL,            kDirLoop_workarounds },
	{ MAP_CALL(DisposeClone),      SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(DisposeList),       SIG_EVERYWHERE,           "l",                     NULL,            NULL },
	{ MAP_CALL(DisposeScript),     SIG_EVERYWHERE,           "i(i*)",                 NULL,            kDisposeScript_workarounds },
	{ MAP_CALL(DisposeWindow),     SIG_EVERYWHERE,           "i(i)",                  NULL,            NULL },
	{ MAP_CALL(DoAudio),           SIG_EVERYWHERE,           "i(.*)",                 NULL,            NULL }, // subop
	{ MAP_CALL(DoAvoider),         SIG_EVERYWHERE,           "o(i)",                  NULL,            NULL },
	{ MAP_CALL(DoBresen),          SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(DoSound),           SIG_EVERYWHERE,           "i(.*)",                 kDoSound_subops, NULL },
	{ MAP_CALL(DoSync),            SIG_EVERYWHERE,           "i(.*)",                 NULL,            NULL }, // subop
	{ MAP_CALL(DrawCel),           SIG_SCI11, SIGFOR_PC,     "iiiii(i)(i)([ri])",     NULL,            NULL }, // reference for kq6 hires
	{ MAP_CALL(DrawCel),           SIG_EVERYWHERE,           "iiiii(i)(i)",           NULL,            NULL },
	{ MAP_CALL(DrawControl),       SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(DrawMenuBar),       SIG_EVERYWHERE,           "i",                     NULL,            NULL },
	{ MAP_CALL(DrawPic),           SIG_EVERYWHERE,           "i(i)(i)(i)",            NULL,            NULL },
	{ MAP_CALL(DrawStatus),        SIG_EVERYWHERE,           "[r0](i)(i)",            NULL,            NULL },
	{ MAP_CALL(EditControl),       SIG_EVERYWHERE,           "[o0][o0]",              NULL,            NULL },
	{ MAP_CALL(Empty),             SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_CALL(EmptyList),         SIG_EVERYWHERE,           "l",                     NULL,            NULL },
	{ "FClose", kFileIOClose,      SIG_EVERYWHERE,           "i",                     NULL,            NULL },
	{ "FGets", kFileIOReadString,  SIG_EVERYWHERE,           "rii",                   NULL,            NULL },
	{ "FOpen", kFileIOOpen,        SIG_EVERYWHERE,           "ri",                    NULL,            NULL },
	{ "FPuts", kFileIOWriteString, SIG_EVERYWHERE,           "ir",                    NULL,            NULL },
	{ MAP_CALL(FileIO),            SIG_EVERYWHERE,           "i(.*)",                 kFileIO_subops,  NULL },
	{ MAP_CALL(FindKey),           SIG_EVERYWHERE,           "l.",                    NULL,            kFindKey_workarounds },
	{ MAP_CALL(FirstNode),         SIG_EVERYWHERE,           "[l0]",                  NULL,            NULL },
	{ MAP_CALL(FlushResources),    SIG_EVERYWHERE,           "i",                     NULL,            NULL },
	{ MAP_CALL(Format),            SIG_EVERYWHERE,           "r[ri](.*)",             NULL,            NULL },
	{ MAP_CALL(GameIsRestarting),  SIG_EVERYWHERE,           "(i)",                   NULL,            NULL },
	{ MAP_CALL(GetAngle),          SIG_EVERYWHERE,           "iiii",                  NULL,            kGetAngle_workarounds },
	{ MAP_CALL(GetCWD),            SIG_EVERYWHERE,           "r",                     NULL,            NULL },
	{ MAP_CALL(GetDistance),       SIG_EVERYWHERE,           "ii(i)(i)(i)(i)",        NULL,            NULL },
	{ MAP_CALL(GetEvent),          SIG_SCIALL, SIGFOR_MAC,   "io(i*)",                NULL,            NULL },
	{ MAP_CALL(GetEvent),          SIG_EVERYWHERE,           "io",                    NULL,            NULL },
	{ MAP_CALL(GetFarText),        SIG_EVERYWHERE,           "ii[r0]",                NULL,            NULL },
	{ MAP_CALL(GetMenu),           SIG_EVERYWHERE,           "i.",                    NULL,            NULL },
	{ MAP_CALL(GetMessage),        SIG_EVERYWHERE,           "iiir",                  NULL,            NULL },
	{ MAP_CALL(GetPort),           SIG_EVERYWHERE,           "",                      NULL,            NULL },
	{ MAP_CALL(GetSaveDir),        SIG_SCI32, SIGFOR_ALL,    "(r*)",                  NULL,            NULL },
	{ MAP_CALL(GetSaveDir),        SIG_EVERYWHERE,           "",                      NULL,            NULL },
	{ MAP_CALL(GetSaveFiles),      SIG_EVERYWHERE,           "rrr",                   NULL,            NULL },
	{ MAP_CALL(GetTime),           SIG_EVERYWHERE,           "(i)",                   NULL,            NULL },
	{ MAP_CALL(GlobalToLocal),     SIG_SCI32, SIGFOR_ALL,    "oo",                    NULL,            NULL },
	{ MAP_CALL(GlobalToLocal),     SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(Graph),             SIG_EVERYWHERE,           NULL,                    kGraph_subops,   NULL },
	{ MAP_CALL(HaveMouse),         SIG_EVERYWHERE,           "",                      NULL,            NULL },
	{ MAP_CALL(HiliteControl),     SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(InitBresen),        SIG_EVERYWHERE,           "o(i)",                  NULL,            NULL },
	{ MAP_CALL(Intersections),     SIG_EVERYWHERE,           "iiiiriiiri",            NULL,            NULL },
	{ MAP_CALL(IsItSkip),          SIG_EVERYWHERE,           "iiiii",                 NULL,            NULL },
	{ MAP_CALL(IsObject),          SIG_EVERYWHERE,           ".",                     NULL,            kIsObject_workarounds },
	{ MAP_CALL(Joystick),          SIG_EVERYWHERE,           "i(.*)",                 NULL,            NULL }, // subop
	{ MAP_CALL(LastNode),          SIG_EVERYWHERE,           "l",                     NULL,            NULL },
	{ MAP_CALL(Load),              SIG_EVERYWHERE,           "ii(i*)",                NULL,            NULL },
	{ MAP_CALL(LocalToGlobal),     SIG_SCI32, SIGFOR_ALL,    "oo",                    NULL,            NULL },
	{ MAP_CALL(LocalToGlobal),     SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(Lock),              SIG_EVERYWHERE,           "ii(i)",                 NULL,            NULL },
	{ MAP_CALL(MapKeyToDir),       SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(Memory),            SIG_EVERYWHERE,           "i(.*)",                 NULL,            kMemory_workarounds }, // subop
	{ MAP_CALL(MemoryInfo),        SIG_EVERYWHERE,           "i",                     NULL,            NULL },
	{ MAP_CALL(MemorySegment),     SIG_EVERYWHERE,           "ir(i)",                 NULL,            NULL }, // subop
	{ MAP_CALL(MenuSelect),        SIG_EVERYWHERE,           "o(i)",                  NULL,            NULL },
	{ MAP_CALL(MergePoly),         SIG_EVERYWHERE,           "rli",                   NULL,            NULL },
	{ MAP_CALL(Message),           SIG_EVERYWHERE,           "i(.*)",                 NULL,            NULL }, // subop
	{ MAP_CALL(MoveCursor),        SIG_EVERYWHERE,           "ii",                    NULL,            kMoveCursor_workarounds },
	{ MAP_CALL(NewList),           SIG_EVERYWHERE,           "",                      NULL,            NULL },
	{ MAP_CALL(NewNode),           SIG_EVERYWHERE,           "..",                    NULL,            NULL },
	{ MAP_CALL(NewWindow),         SIG_SCIALL, SIGFOR_MAC,   ".*",                    NULL,            NULL },
	{ MAP_CALL(NewWindow),         SIG_SCI0, SIGFOR_ALL,     "iiii[r0]i(i)(i)(i)",    NULL,            NULL },
	{ MAP_CALL(NewWindow),         SIG_SCI1, SIGFOR_ALL,     "iiii[ir]i(i)(i)([ir])(i)(i)(i)(i)", NULL, NULL },
	{ MAP_CALL(NewWindow),         SIG_SCI11, SIGFOR_ALL,    "iiiiiiii[r0]i(i)(i)(i)", NULL,           kNewWindow_workarounds },
	{ MAP_CALL(NextNode),          SIG_EVERYWHERE,           "n",                     NULL,            NULL },
	{ MAP_CALL(NodeValue),         SIG_EVERYWHERE,           "[n0]",                  NULL,            NULL },
	{ MAP_CALL(NumCels),           SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(NumLoops),          SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(OnControl),         SIG_EVERYWHERE,           "ii(i)(i)(i)",           NULL,            NULL },
	{ MAP_CALL(PalVary),           SIG_EVERYWHERE,           "i(i*)",                 kPalVary_subops, NULL },
	{ MAP_CALL(Palette),           SIG_EVERYWHERE,           "i(.*)",                 kPalette_subops, NULL },
	{ MAP_CALL(Parse),             SIG_EVERYWHERE,           "ro",                    NULL,            NULL },
	{ MAP_CALL(PicNotValid),       SIG_EVERYWHERE,           "(i)",                   NULL,            NULL },
	{ MAP_CALL(Platform),          SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_CALL(Portrait),          SIG_EVERYWHERE,           "i(.*)",                 NULL,            NULL }, // subop
	{ MAP_CALL(PrevNode),          SIG_EVERYWHERE,           "n",                     NULL,            NULL },
	{ MAP_CALL(PriCoord),          SIG_EVERYWHERE,           "i",                     NULL,            NULL },
	{ MAP_CALL(Random),            SIG_EVERYWHERE,           "i(i)(i)",               NULL,            NULL },
	{ MAP_CALL(ReadNumber),        SIG_EVERYWHERE,           "r",                     NULL,            NULL },
	{ MAP_CALL(RemapColors),       SIG_SCI11, SIGFOR_ALL,    "i(i)(i)(i)(i)",         NULL,            NULL },
#ifdef ENABLE_SCI32
	{ "RemapColors", kRemapColors32, SIG_SCI32, SIGFOR_ALL,  "i(i)(i)(i)(i)(i)",      NULL,            NULL },
#endif
	{ MAP_CALL(ResCheck),          SIG_EVERYWHERE,           "ii(iiii)",              NULL,            NULL },
	{ MAP_CALL(RespondsTo),        SIG_EVERYWHERE,           ".i",                    NULL,            NULL },
	{ MAP_CALL(RestartGame),       SIG_EVERYWHERE,           "",                      NULL,            NULL },
	{ MAP_CALL(RestoreGame),       SIG_EVERYWHERE,           "[r0]i[r0]",             NULL,            NULL },
	{ MAP_CALL(Said),              SIG_EVERYWHERE,           "[r0]",                  NULL,            NULL },
	{ MAP_CALL(SaveGame),          SIG_EVERYWHERE,           "[r0]i[r0](r)",          NULL,            NULL },
	{ MAP_CALL(ScriptID),          SIG_EVERYWHERE,           "[io](i)",               NULL,            NULL },
	{ MAP_CALL(SetCursor),         SIG_SCI21, SIGFOR_ALL,    "i(i)([io])(i*)",        NULL,            NULL },
	// TODO: SCI2.1 may supply an object optionally (mother goose sci21 right on startup) - find out why
	{ MAP_CALL(SetCursor),         SIG_SCI11, SIGFOR_ALL,    "i(i)(i)(i)(iiiiii)",    NULL,            NULL },
	{ MAP_CALL(SetCursor),         SIG_EVERYWHERE,           "i(i)(i)(i)(i)",         NULL,            kSetCursor_workarounds },
	{ MAP_CALL(SetDebug),          SIG_EVERYWHERE,           "(i*)",                  NULL,            NULL },
	{ MAP_CALL(SetJump),           SIG_EVERYWHERE,           "oiii",                  NULL,            NULL },
	{ MAP_CALL(SetMenu),           SIG_EVERYWHERE,           "i(.*)",                 NULL,            NULL },
	{ MAP_CALL(SetNowSeen),        SIG_EVERYWHERE,           "o(i)",                  NULL,            NULL },
	{ MAP_CALL(SetPort),           SIG_EVERYWHERE,           "i(iiiii)(i)",           NULL,            kSetPort_workarounds },
	{ MAP_CALL(SetQuitStr),        SIG_EVERYWHERE,           "r",                     NULL,            NULL },
	{ MAP_CALL(SetSynonyms),       SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(SetVideoMode),      SIG_EVERYWHERE,           "i",                     NULL,            NULL },
	{ MAP_CALL(ShakeScreen),       SIG_EVERYWHERE,           "(i)(i)",                NULL,            NULL },
	{ MAP_CALL(ShowMovie),         SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_CALL(Show),              SIG_EVERYWHERE,           "i",                     NULL,            NULL },
	{ MAP_CALL(SinDiv),            SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
	{ MAP_CALL(Sort),              SIG_EVERYWHERE,           "ooo",                   NULL,            NULL },
	{ MAP_CALL(Sqrt),              SIG_EVERYWHERE,           "i",                     NULL,            NULL },
	{ MAP_CALL(StrAt),             SIG_EVERYWHERE,           "ri(i)",                 NULL,            kStrAt_workarounds },
	{ MAP_CALL(StrCat),            SIG_EVERYWHERE,           "rr",                    NULL,            NULL },
	{ MAP_CALL(StrCmp),            SIG_EVERYWHERE,           "rr(i)",                 NULL,            NULL },
	{ MAP_CALL(StrCpy),            SIG_EVERYWHERE,           "r[r0](i)",              NULL,            NULL },
	{ MAP_CALL(StrEnd),            SIG_EVERYWHERE,           "r",                     NULL,            NULL },
	{ MAP_CALL(StrLen),            SIG_EVERYWHERE,           "[r0]",                  NULL,            kStrLen_workarounds },
	{ MAP_CALL(StrSplit),          SIG_EVERYWHERE,           "rr[r0]",                NULL,            NULL },
	{ MAP_CALL(TextColors),        SIG_EVERYWHERE,           "(i*)",                  NULL,            NULL },
	{ MAP_CALL(TextFonts),         SIG_EVERYWHERE,           "(i*)",                  NULL,            NULL },
	{ MAP_CALL(TextSize),          SIG_SCIALL, SIGFOR_MAC,   "r[r0]i(i)(r0)(i)",      NULL,            NULL },
	{ MAP_CALL(TextSize),          SIG_EVERYWHERE,           "r[r0]i(i)(r0)",         NULL,            NULL },
	{ MAP_CALL(TimesCos),          SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
	{ "CosMult", kTimesCos,        SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
	{ MAP_CALL(TimesCot),          SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
	{ MAP_CALL(TimesSin),          SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
	{ "SinMult", kTimesSin,        SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
	{ MAP_CALL(TimesTan),          SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
	{ MAP_CALL(UnLoad),            SIG_EVERYWHERE,           "i[ri]",                 NULL,            kUnLoad_workarounds },
	{ MAP_CALL(ValidPath),         SIG_EVERYWHERE,           "r",                     NULL,            NULL },
	{ MAP_CALL(Wait),              SIG_EVERYWHERE,           "i",                     NULL,            NULL },

	// Unimplemented SCI0-SCI1.1 unused functions, always mapped to kDummy
	{ MAP_DUMMY(InspectObj),      SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(ShowSends),       SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(ShowObjs),        SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(ShowFree),        SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(StackUsage),      SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(Profiler),        SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(ShiftScreen),     SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(ListOps),         SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	// Used by the sysLogger class (e.g. script 952 in GK1CD), a class used to report bugs by Sierra's testers
	{ MAP_DUMMY(ATan),            SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(Record),          SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(PlayBack),        SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(DbugStr),         SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },

	// =======================================================================================================

#ifdef ENABLE_SCI32
	// SCI2 Kernel Functions
	// TODO: whoever knows his way through those calls, fix the signatures.
	{ MAP_CALL(AddPlane),          SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(AddScreenItem),     SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(Array),             SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_CALL(CreateTextBitmap),  SIG_EVERYWHERE,           "i(.*)",                 NULL,            NULL },
	{ MAP_CALL(DeletePlane),       SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(DeleteScreenItem),  SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(DisposeTextBitmap), SIG_EVERYWHERE,           "r",                     NULL,            NULL },
	{ MAP_CALL(FrameOut),          SIG_EVERYWHERE,           "",                      NULL,            NULL },
	{ MAP_CALL(GetHighPlanePri),   SIG_EVERYWHERE,           "",                      NULL,            NULL },
	{ MAP_CALL(InPolygon),         SIG_EVERYWHERE,           "iio",                   NULL,            NULL },
	{ MAP_CALL(IsHiRes),           SIG_EVERYWHERE,           "",                      NULL,            NULL },
	{ MAP_CALL(ListAllTrue),       SIG_EVERYWHERE,           "li(.*)",                NULL,            NULL },
	{ MAP_CALL(ListAt),            SIG_EVERYWHERE,           "li",                    NULL,            NULL },
	{ MAP_CALL(ListEachElementDo), SIG_EVERYWHERE,           "li(.*)",                NULL,            NULL },
	{ MAP_CALL(ListFirstTrue),     SIG_EVERYWHERE,           "li(.*)",                NULL,            NULL },
	{ MAP_CALL(ListIndexOf),       SIG_EVERYWHERE,           "l[o0]",                 NULL,            NULL },
	{ "OnMe", kIsOnMe,             SIG_EVERYWHERE,           "iioi",                  NULL,            NULL },
	// Purge is used by the memory manager in SSCI to ensure that X number of bytes (the so called "unmovable
	// memory") are available when the current room changes. This is similar to the SCI0-SCI1.1 FlushResources
	// call, with the added functionality of ensuring that a specific amount of memory is available. We have
	// our own memory manager and garbage collector, thus we simply call FlushResources, which in turn invokes
	// our garbage collector (i.e. the SCI0-SCI1.1 semantics).
	{ "Purge", kFlushResources,    SIG_EVERYWHERE,           "i",                     NULL,            NULL },
	{ MAP_CALL(SetShowStyle),      SIG_EVERYWHERE,           "ioiiiii([ri])(i)",      NULL,            NULL },
	{ MAP_CALL(String),            SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_CALL(UpdatePlane),       SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(UpdateScreenItem),  SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(ObjectIntersect),   SIG_EVERYWHERE,           "oo",                    NULL,            NULL },
	{ MAP_CALL(EditText),          SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_CALL(MakeSaveCatName),   SIG_EVERYWHERE,           "rr",                    NULL,            NULL },
	{ MAP_CALL(MakeSaveFileName),  SIG_EVERYWHERE,           "rri",                   NULL,            NULL },
	{ MAP_CALL(SetScroll),         SIG_EVERYWHERE,           "oiiiii(i)",             NULL,            NULL },
	{ MAP_CALL(PalCycle),          SIG_EVERYWHERE,           "i(.*)",                 NULL,            NULL },

	// SCI2 Empty functions

	// Debug function used to track resources
	{ MAP_EMPTY(ResourceTrack),     SIG_EVERYWHERE,          "(.*)",                  NULL,            NULL },
	// Future TODO: This call is used in the floppy version of QFG4 to add
	// vibration to exotic mice with force feedback, such as the Logitech
	// Cyberman and Wingman mice. Since this is only used for very exotic
	// hardware and we have no direct and cross-platform way of communicating
	// with them via SDL, plus we would probably need to make changes to common
	// code, this call is mapped to an empty function for now as it's a rare
	// feature not worth the effort.
	{ MAP_EMPTY(VibrateMouse),      SIG_EVERYWHERE,          "(.*)",                  NULL,            NULL },

	// Unused / debug SCI2 unused functions, always mapped to kDummy

	// AddMagnify/DeleteMagnify are both called by script 64979 (the Magnifier
	// object) in GK1 only. There is also an associated empty magnifier view
	// (view 1), however, it doesn't seem to be used anywhere, as all the
	// magnifier closeups (e.g. in scene 470) are normal views. Thus, these
	// are marked as dummy, so if they're ever used the engine will error out.
	{ MAP_DUMMY(AddMagnify),       SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(DeleteMagnify),    SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(RepaintPlane),     SIG_EVERYWHERE,           "o",                     NULL,            NULL },
	{ MAP_DUMMY(InspectObject),    SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	// Profiler (same as SCI0-SCI1.1)
	// Record (same as SCI0-SCI1.1)
	// PlayBack (same as SCI0-SCI1.1)
	{ MAP_DUMMY(MonoOut),          SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(SetFatalStr),      SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(IntegrityChecking),SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(CheckIntegrity),   SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(MarkMemory),       SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(GetHighItemPri),   SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(ShowStylePercent), SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(InvertRect),       SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(InputText),        SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(TextWidth),        SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_DUMMY(PointSize),        SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	// SetScroll is called by script 64909, Styler::doit(), but it doesn't seem to
	// be used at all (plus, it was then changed to a dummy function in SCI3).
	// Since this is most likely unused, and we got no test case, error out when
	// it is called in order to find an actual call to it.
	{ MAP_DUMMY(SetScroll),        SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },

	// SCI2.1 Kernel Functions
	{ MAP_CALL(CD),                SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_CALL(IsOnMe),            SIG_EVERYWHERE,           "iioi",                  NULL,            NULL },
	{ MAP_CALL(List),              SIG_SCI21, SIGFOR_ALL,    "(.*)",                  kList_subops,    NULL },
	{ MAP_CALL(MulDiv),            SIG_EVERYWHERE,           "iii",                   NULL,            NULL },
	{ MAP_CALL(PlayVMD),           SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_CALL(Robot),             SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_CALL(Save),              SIG_EVERYWHERE,           "i(.*)",                 kSave_subops,    NULL },
	{ MAP_CALL(Text),              SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_CALL(AddPicAt),          SIG_EVERYWHERE,           "oiii",                  NULL,            NULL },
	{ MAP_CALL(GetWindowsOption),  SIG_EVERYWHERE,           "i",                     NULL,            NULL },
	{ MAP_CALL(WinHelp),           SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_CALL(GetConfig),         SIG_EVERYWHERE,           "ro",                    NULL,            NULL },
	{ MAP_CALL(GetSierraProfileInt), SIG_EVERYWHERE,         "rri",                   NULL,            NULL },
	{ MAP_CALL(CelInfo),           SIG_EVERYWHERE,           "iiiiii",                NULL,            NULL },
	{ MAP_CALL(SetLanguage),       SIG_EVERYWHERE,           "r",                     NULL,            NULL },
	{ MAP_CALL(ScrollWindow),      SIG_EVERYWHERE,           "io(.*)",                NULL,            NULL },
	{ MAP_CALL(SetFontRes),        SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
	{ MAP_CALL(Font),              SIG_EVERYWHERE,           "i(.*)",                 NULL,            NULL },
	{ MAP_CALL(Bitmap),            SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
	{ MAP_CALL(AddLine),           SIG_EVERYWHERE,           "oiiiiiiiii",            NULL,            NULL },
	{ MAP_CALL(UpdateLine),        SIG_EVERYWHERE,           "[r0]oiiiiiiiii",        NULL,            NULL },
	{ MAP_CALL(DeleteLine),        SIG_EVERYWHERE,           "[r0]o",                 NULL,            NULL },

	// SCI2.1 Empty Functions

	// Debug function, used in of Shivers (demo and full). It's marked as a
	// stub in the original interpreters, but it gets called by the game scripts.
	// Usually, it gets called with a string (which is the output format) and a
	// variable number of parameters
	{ MAP_EMPTY(PrintDebug),        SIG_EVERYWHERE,          "(.*)",                  NULL,            NULL },

	// SetWindowsOption is used to set Windows specific options, like for example the title bar visibility of
	// the game window in Phantasmagoria 2. We ignore these settings completely.
	{ MAP_EMPTY(SetWindowsOption), SIG_EVERYWHERE,             "ii",                  NULL,            NULL },

	// Debug function called whenever the current room changes
	{ MAP_EMPTY(NewRoom),           SIG_EVERYWHERE,          "(.*)",                  NULL,            NULL },

	// Unused / debug SCI2.1 unused functions, always mapped to kDummy

	// The debug functions are called from the inbuilt debugger or polygon
	// editor in SCI2.1 games. Related objects are: PEditor, EditablePolygon,
	// aeDisplayClass and scalerCode
	{ MAP_DUMMY(FindSelector),      SIG_EVERYWHERE,           "(.*)",                 NULL,            NULL },
	{ MAP_DUMMY(FindClass),         SIG_EVERYWHERE,           "(.*)",                 NULL,            NULL },
	{ MAP_DUMMY(CelRect),           SIG_EVERYWHERE,           "(.*)",                 NULL,            NULL },
	{ MAP_DUMMY(BaseLineSpan),      SIG_EVERYWHERE,           "(.*)",                 NULL,            NULL },
	{ MAP_DUMMY(CelLink),           SIG_EVERYWHERE,           "(.*)",                 NULL,            NULL },
	{ MAP_DUMMY(AddPolygon),        SIG_EVERYWHERE,           "(.*)",                 NULL,            NULL },
	{ MAP_DUMMY(DeletePolygon),     SIG_EVERYWHERE,           "(.*)",                 NULL,            NULL },
	{ MAP_DUMMY(UpdatePolygon),     SIG_EVERYWHERE,           "(.*)",                 NULL,            NULL },
	{ MAP_DUMMY(Table),             SIG_EVERYWHERE,           "(.*)",                 NULL,            NULL },
	{ MAP_DUMMY(LoadChunk),         SIG_EVERYWHERE,           "(.*)",                 NULL,            NULL },
	{ MAP_DUMMY(Priority),          SIG_EVERYWHERE,           "(.*)",                 NULL,            NULL },
	{ MAP_DUMMY(WinDLL),            SIG_EVERYWHERE,           "(.*)",                 NULL,            NULL },
	{ MAP_DUMMY(DeletePic),         SIG_EVERYWHERE,           "(.*)",                 NULL,            NULL },
	{ MAP_DUMMY(GetSierraProfileString), SIG_EVERYWHERE,      "(.*)",                 NULL,            NULL },

	// Unused / debug functions in the in-between SCI2.1 interpreters
	{ MAP_DUMMY(PreloadResource),   SIG_EVERYWHERE,           "(.*)",                 NULL,            NULL },
	{ MAP_DUMMY(CheckCDisc),        SIG_EVERYWHERE,           "(.*)",                 NULL,            NULL },
	{ MAP_DUMMY(GetSaveCDisc),      SIG_EVERYWHERE,           "(.*)",                 NULL,            NULL },
	{ MAP_DUMMY(TestPoly),          SIG_EVERYWHERE,           "(.*)",                 NULL,            NULL },

	// SCI2.1 unmapped functions - TODO!

	// SetHotRectangles - used by Phantasmagoria 1, script 64981 (used in the chase scene)
	//     <lskovlun> The idea, if I understand correctly, is that the engine generates events
	//     of a special HotRect type continuously when the mouse is on that rectangle

	// MovePlaneItems - used by SQ6 to scroll through the inventory via the up/down buttons
	// SetPalStyleRange - 2 integer parameters, start and end. All styles from start-end
	//   (inclusive) are set to 0
	// MorphOn - used by SQ6, script 900, the datacorder reprogramming puzzle (from room 270)

	// SCI3 Kernel Functions
	{ MAP_CALL(PlayDuck),         SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
#endif

	{ NULL, NULL,                  SIG_EVERYWHERE,           NULL,                    NULL,            NULL }
};

/** Default kernel name table. */
static const char *const s_defaultKernelNames[] = {
	/*0x00*/ "Load",
	/*0x01*/ "UnLoad",
	/*0x02*/ "ScriptID",
	/*0x03*/ "DisposeScript",
	/*0x04*/ "Clone",
	/*0x05*/ "DisposeClone",
	/*0x06*/ "IsObject",
	/*0x07*/ "RespondsTo",
	/*0x08*/ "DrawPic",
	/*0x09*/ "Show",
	/*0x0a*/ "PicNotValid",
	/*0x0b*/ "Animate",
	/*0x0c*/ "SetNowSeen",
	/*0x0d*/ "NumLoops",
	/*0x0e*/ "NumCels",
	/*0x0f*/ "CelWide",
	/*0x10*/ "CelHigh",
	/*0x11*/ "DrawCel",
	/*0x12*/ "AddToPic",
	/*0x13*/ "NewWindow",
	/*0x14*/ "GetPort",
	/*0x15*/ "SetPort",
	/*0x16*/ "DisposeWindow",
	/*0x17*/ "DrawControl",
	/*0x18*/ "HiliteControl",
	/*0x19*/ "EditControl",
	/*0x1a*/ "TextSize",
	/*0x1b*/ "Display",
	/*0x1c*/ "GetEvent",
	/*0x1d*/ "GlobalToLocal",
	/*0x1e*/ "LocalToGlobal",
	/*0x1f*/ "MapKeyToDir",
	/*0x20*/ "DrawMenuBar",
	/*0x21*/ "MenuSelect",
	/*0x22*/ "AddMenu",
	/*0x23*/ "DrawStatus",
	/*0x24*/ "Parse",
	/*0x25*/ "Said",
	/*0x26*/ "SetSynonyms",	// Portrait (KQ6 hires)
	/*0x27*/ "HaveMouse",
	/*0x28*/ "SetCursor",
	// FOpen (SCI0)
	// FPuts (SCI0)
	// FGets (SCI0)
	// FClose (SCI0)
	/*0x29*/ "SaveGame",
	/*0x2a*/ "RestoreGame",
	/*0x2b*/ "RestartGame",
	/*0x2c*/ "GameIsRestarting",
	/*0x2d*/ "DoSound",
	/*0x2e*/ "NewList",
	/*0x2f*/ "DisposeList",
	/*0x30*/ "NewNode",
	/*0x31*/ "FirstNode",
	/*0x32*/ "LastNode",
	/*0x33*/ "EmptyList",
	/*0x34*/ "NextNode",
	/*0x35*/ "PrevNode",
	/*0x36*/ "NodeValue",
	/*0x37*/ "AddAfter",
	/*0x38*/ "AddToFront",
	/*0x39*/ "AddToEnd",
	/*0x3a*/ "FindKey",
	/*0x3b*/ "DeleteKey",
	/*0x3c*/ "Random",
	/*0x3d*/ "Abs",
	/*0x3e*/ "Sqrt",
	/*0x3f*/ "GetAngle",
	/*0x40*/ "GetDistance",
	/*0x41*/ "Wait",
	/*0x42*/ "GetTime",
	/*0x43*/ "StrEnd",
	/*0x44*/ "StrCat",
	/*0x45*/ "StrCmp",
	/*0x46*/ "StrLen",
	/*0x47*/ "StrCpy",
	/*0x48*/ "Format",
	/*0x49*/ "GetFarText",
	/*0x4a*/ "ReadNumber",
	/*0x4b*/ "BaseSetter",
	/*0x4c*/ "DirLoop",
	/*0x4d*/ "CanBeHere",       // CantBeHere in newer SCI versions
	/*0x4e*/ "OnControl",
	/*0x4f*/ "InitBresen",
	/*0x50*/ "DoBresen",
	/*0x51*/ "Platform",        // DoAvoider (SCI0)
	/*0x52*/ "SetJump",
	/*0x53*/ "SetDebug",        // for debugging
	/*0x54*/ "InspectObj",      // for debugging
	/*0x55*/ "ShowSends",       // for debugging
	/*0x56*/ "ShowObjs",        // for debugging
	/*0x57*/ "ShowFree",        // for debugging
	/*0x58*/ "MemoryInfo",
	/*0x59*/ "StackUsage",      // for debugging
	/*0x5a*/ "Profiler",        // for debugging
	/*0x5b*/ "GetMenu",
	/*0x5c*/ "SetMenu",
	/*0x5d*/ "GetSaveFiles",
	/*0x5e*/ "GetCWD",
	/*0x5f*/ "CheckFreeSpace",
	/*0x60*/ "ValidPath",
	/*0x61*/ "CoordPri",
	/*0x62*/ "StrAt",
	/*0x63*/ "DeviceInfo",
	/*0x64*/ "GetSaveDir",
	/*0x65*/ "CheckSaveGame",
	/*0x66*/ "ShakeScreen",
	/*0x67*/ "FlushResources",
	/*0x68*/ "SinMult",
	/*0x69*/ "CosMult",
	/*0x6a*/ "SinDiv",
	/*0x6b*/ "CosDiv",
	/*0x6c*/ "Graph",
	/*0x6d*/ "Joystick",
	// End of kernel function table for SCI0
	/*0x6e*/ "ShiftScreen",     // never called?
	/*0x6f*/ "Palette",
	/*0x70*/ "MemorySegment",
	/*0x71*/ "Intersections",	// MoveCursor (SCI1 late), PalVary (SCI1.1)
	/*0x72*/ "Memory",
	/*0x73*/ "ListOps",         // never called?
	/*0x74*/ "FileIO",
	/*0x75*/ "DoAudio",
	/*0x76*/ "DoSync",
	/*0x77*/ "AvoidPath",
	/*0x78*/ "Sort",            // StrSplit (SCI01)
	/*0x79*/ "ATan",            // never called?
	/*0x7a*/ "Lock",
	/*0x7b*/ "StrSplit",
	/*0x7c*/ "GetMessage",      // Message (SCI1.1)
	/*0x7d*/ "IsItSkip",
	/*0x7e*/ "MergePoly",
	/*0x7f*/ "ResCheck",
	/*0x80*/ "AssertPalette",
	/*0x81*/ "TextColors",
	/*0x82*/ "TextFonts",
	/*0x83*/ "Record",          // for debugging
	/*0x84*/ "PlayBack",        // for debugging
	/*0x85*/ "ShowMovie",
	/*0x86*/ "SetVideoMode",
	/*0x87*/ "SetQuitStr",
	/*0x88*/ "DbugStr"          // for debugging
};

#ifdef ENABLE_SCI32

// NOTE: 0x72-0x79, 0x85-0x86, 0x88 are from the GK2 demo (which has debug support) and are
// just Dummy in other SCI2 games.
static const char *const sci2_default_knames[] = {
	/*0x00*/ "Load",
	/*0x01*/ "UnLoad",
	/*0x02*/ "ScriptID",
	/*0x03*/ "DisposeScript",
	/*0x04*/ "Lock",
	/*0x05*/ "ResCheck",
	/*0x06*/ "Purge",
	/*0x07*/ "Clone",
	/*0x08*/ "DisposeClone",
	/*0x09*/ "RespondsTo",
	/*0x0a*/ "SetNowSeen",
	/*0x0b*/ "NumLoops",
	/*0x0c*/ "NumCels",
	/*0x0d*/ "CelWide",
	/*0x0e*/ "CelHigh",
	/*0x0f*/ "GetHighPlanePri",
	/*0x10*/ "GetHighItemPri",		// unused function
	/*0x11*/ "ShakeScreen",
	/*0x12*/ "OnMe",
	/*0x13*/ "ShowMovie",
	/*0x14*/ "SetVideoMode",
	/*0x15*/ "AddScreenItem",
	/*0x16*/ "DeleteScreenItem",
	/*0x17*/ "UpdateScreenItem",
	/*0x18*/ "FrameOut",
	/*0x19*/ "AddPlane",
	/*0x1a*/ "DeletePlane",
	/*0x1b*/ "UpdatePlane",
	/*0x1c*/ "RepaintPlane",		// unused function
	/*0x1d*/ "SetShowStyle",
	/*0x1e*/ "ShowStylePercent",	// unused function
	/*0x1f*/ "SetScroll",
	/*0x20*/ "AddMagnify",
	/*0x21*/ "DeleteMagnify",
	/*0x22*/ "IsHiRes",
	/*0x23*/ "Graph",		// Robot in early SCI2.1 games with a SCI2 kernel table
	/*0x24*/ "InvertRect",	// only in SCI2, not used in any SCI2 game
	/*0x25*/ "TextSize",
	/*0x26*/ "Message",
	/*0x27*/ "TextColors",
	/*0x28*/ "TextFonts",
	/*0x29*/ "Dummy",
	/*0x2a*/ "SetQuitStr",
	/*0x2b*/ "EditText",
	/*0x2c*/ "InputText",			// unused function
	/*0x2d*/ "CreateTextBitmap",
	/*0x2e*/ "DisposeTextBitmap",	// Priority in early SCI2.1 games with a SCI2 kernel table
	/*0x2f*/ "GetEvent",
	/*0x30*/ "GlobalToLocal",
	/*0x31*/ "LocalToGlobal",
	/*0x32*/ "MapKeyToDir",
	/*0x33*/ "HaveMouse",
	/*0x34*/ "SetCursor",
	/*0x35*/ "VibrateMouse",
	/*0x36*/ "SaveGame",
	/*0x37*/ "RestoreGame",
	/*0x38*/ "RestartGame",
	/*0x39*/ "GameIsRestarting",
	/*0x3a*/ "MakeSaveCatName",
	/*0x3b*/ "MakeSaveFileName",
	/*0x3c*/ "GetSaveFiles",
	/*0x3d*/ "GetSaveDir",
	/*0x3e*/ "CheckSaveGame",
	/*0x3f*/ "CheckFreeSpace",
	/*0x40*/ "DoSound",
	/*0x41*/ "DoAudio",
	/*0x42*/ "DoSync",
	/*0x43*/ "NewList",
	/*0x44*/ "DisposeList",
	/*0x45*/ "NewNode",
	/*0x46*/ "FirstNode",
	/*0x47*/ "LastNode",
	/*0x48*/ "EmptyList",
	/*0x49*/ "NextNode",
	/*0x4a*/ "PrevNode",
	/*0x4b*/ "NodeValue",
	/*0x4c*/ "AddAfter",
	/*0x4d*/ "AddToFront",
	/*0x4e*/ "AddToEnd",
	/*0x4f*/ "Dummy",
	/*0x50*/ "Dummy",
	/*0x51*/ "FindKey",
	/*0x52*/ "Dummy",
	/*0x53*/ "Dummy",
	/*0x54*/ "Dummy",
	/*0x55*/ "DeleteKey",
	/*0x56*/ "Dummy",
	/*0x57*/ "Dummy",
	/*0x58*/ "ListAt",
	/*0x59*/ "ListIndexOf",
	/*0x5a*/ "ListEachElementDo",
	/*0x5b*/ "ListFirstTrue",
	/*0x5c*/ "ListAllTrue",
	/*0x5d*/ "Random",
	/*0x5e*/ "Abs",
	/*0x5f*/ "Sqrt",
	/*0x60*/ "GetAngle",
	/*0x61*/ "GetDistance",
	/*0x62*/ "ATan",
	/*0x63*/ "SinMult",
	/*0x64*/ "CosMult",
	/*0x65*/ "SinDiv",
	/*0x66*/ "CosDiv",
	/*0x67*/ "GetTime",
	/*0x68*/ "Platform",
	/*0x69*/ "BaseSetter",
	/*0x6a*/ "DirLoop",
	/*0x6b*/ "CantBeHere",
	/*0x6c*/ "InitBresen",
	/*0x6d*/ "DoBresen",
	/*0x6e*/ "SetJump",
	/*0x6f*/ "AvoidPath",
	/*0x70*/ "InPolygon",
	/*0x71*/ "MergePoly",
	/*0x72*/ "SetDebug",
	/*0x73*/ "InspectObject",     // for debugging
	/*0x74*/ "MemoryInfo",
	/*0x75*/ "Profiler",          // for debugging
	/*0x76*/ "Record",            // for debugging
	/*0x77*/ "PlayBack",          // for debugging
	/*0x78*/ "MonoOut",           // for debugging
	/*0x79*/ "SetFatalStr",       // for debugging
	/*0x7a*/ "GetCWD",
	/*0x7b*/ "ValidPath",
	/*0x7c*/ "FileIO",
	/*0x7d*/ "Dummy",
	/*0x7e*/ "DeviceInfo",
	/*0x7f*/ "Palette",
	/*0x80*/ "PalVary",
	/*0x81*/ "PalCycle",
	/*0x82*/ "Array",
	/*0x83*/ "String",
	/*0x84*/ "RemapColors",
	/*0x85*/ "IntegrityChecking", // for debugging
	/*0x86*/ "CheckIntegrity",	  // for debugging
	/*0x87*/ "ObjectIntersect",
	/*0x88*/ "MarkMemory",	      // for debugging
	/*0x89*/ "TextWidth",		  // for debugging(?), only in SCI2, not used in any SCI2 game
	/*0x8a*/ "PointSize",	      // for debugging(?), only in SCI2, not used in any SCI2 game

	// GK2 Demo (and similar) only kernel functions
	/*0x8b*/ "AddLine",
	/*0x8c*/ "DeleteLine",
	/*0x8d*/ "UpdateLine",
	/*0x8e*/ "AddPolygon",
	/*0x8f*/ "DeletePolygon",
	/*0x90*/ "UpdatePolygon",
	/*0x91*/ "Bitmap",
	/*0x92*/ "ScrollWindow",
	/*0x93*/ "SetFontRes",
	/*0x94*/ "MovePlaneItems",
	/*0x95*/ "PreloadResource",
	/*0x96*/ "Dummy",
	/*0x97*/ "ResourceTrack",
	/*0x98*/ "CheckCDisc",
	/*0x99*/ "GetSaveCDisc",
	/*0x9a*/ "TestPoly",
	/*0x9b*/ "WinHelp",
	/*0x9c*/ "LoadChunk",
	/*0x9d*/ "SetPalStyleRange",
	/*0x9e*/ "AddPicAt",
	/*0x9f*/ "MessageBox"
};

static const char *const sci21_default_knames[] = {
	/*0x00*/ "Load",
	/*0x01*/ "UnLoad",
	/*0x02*/ "ScriptID",
	/*0x03*/ "DisposeScript",
	/*0x04*/ "Lock",
	/*0x05*/ "ResCheck",
	/*0x06*/ "Purge",
	/*0x07*/ "SetLanguage",
	/*0x08*/ "Dummy",
	/*0x09*/ "Dummy",
	/*0x0a*/ "Clone",
	/*0x0b*/ "DisposeClone",
	/*0x0c*/ "RespondsTo",
	/*0x0d*/ "FindSelector",
	/*0x0e*/ "FindClass",
	/*0x0f*/ "Dummy",
	/*0x10*/ "Dummy",
	/*0x11*/ "Dummy",
	/*0x12*/ "Dummy",
	/*0x13*/ "Dummy",
	/*0x14*/ "SetNowSeen",
	/*0x15*/ "NumLoops",
	/*0x16*/ "NumCels",
	/*0x17*/ "IsOnMe",
	/*0x18*/ "AddMagnify",		// dummy in SCI3
	/*0x19*/ "DeleteMagnify",	// dummy in SCI3
	/*0x1a*/ "CelRect",
	/*0x1b*/ "BaseLineSpan",
	/*0x1c*/ "CelWide",
	/*0x1d*/ "CelHigh",
	/*0x1e*/ "AddScreenItem",
	/*0x1f*/ "DeleteScreenItem",
	/*0x20*/ "UpdateScreenItem",
	/*0x21*/ "FrameOut",
	/*0x22*/ "CelInfo",
	/*0x23*/ "Bitmap",
	/*0x24*/ "CelLink",
	/*0x25*/ "Dummy",
	/*0x26*/ "Dummy",
	/*0x27*/ "Dummy",
	/*0x28*/ "AddPlane",
	/*0x29*/ "DeletePlane",
	/*0x2a*/ "UpdatePlane",
	/*0x2b*/ "RepaintPlane",
	/*0x2c*/ "GetHighPlanePri",
	/*0x2d*/ "GetHighItemPri",		// unused function
	/*0x2e*/ "SetShowStyle",
	/*0x2f*/ "ShowStylePercent",	// unused function
	/*0x30*/ "SetScroll",			// dummy in SCI3
	/*0x31*/ "MovePlaneItems",
	/*0x32*/ "ShakeScreen",
	/*0x33*/ "Dummy",
	/*0x34*/ "Dummy",
	/*0x35*/ "Dummy",
	/*0x36*/ "Dummy",
	/*0x37*/ "IsHiRes",
	/*0x38*/ "SetVideoMode",
	/*0x39*/ "ShowMovie",			// dummy in SCI3
	/*0x3a*/ "Robot",
	/*0x3b*/ "CreateTextBitmap",
	/*0x3c*/ "Random",
	/*0x3d*/ "Abs",
	/*0x3e*/ "Sqrt",
	/*0x3f*/ "GetAngle",
	/*0x40*/ "GetDistance",
	/*0x41*/ "ATan",
	/*0x42*/ "SinMult",
	/*0x43*/ "CosMult",
	/*0x44*/ "SinDiv",
	/*0x45*/ "CosDiv",
	/*0x46*/ "Text",
	/*0x47*/ "Dummy",
	/*0x48*/ "Message",
	/*0x49*/ "Font",
	/*0x4a*/ "EditText",
	/*0x4b*/ "InputText",		// unused function
	/*0x4c*/ "ScrollWindow",	// Dummy in SCI3
	/*0x4d*/ "Dummy",
	/*0x4e*/ "Dummy",
	/*0x4f*/ "Dummy",
	/*0x50*/ "GetEvent",
	/*0x51*/ "GlobalToLocal",
	/*0x52*/ "LocalToGlobal",
	/*0x53*/ "MapKeyToDir",
	/*0x54*/ "HaveMouse",
	/*0x55*/ "SetCursor",
	/*0x56*/ "VibrateMouse",	// Dummy in SCI3
	/*0x57*/ "Dummy",
	/*0x58*/ "Dummy",
	/*0x59*/ "Dummy",
	/*0x5a*/ "List",
	/*0x5b*/ "Array",
	/*0x5c*/ "String",
	/*0x5d*/ "FileIO",
	/*0x5e*/ "BaseSetter",
	/*0x5f*/ "DirLoop",
	/*0x60*/ "CantBeHere",
	/*0x61*/ "InitBresen",
	/*0x62*/ "DoBresen",
	/*0x63*/ "SetJump",
	/*0x64*/ "AvoidPath",		// dummy in SCI3
	/*0x65*/ "InPolygon",
	/*0x66*/ "MergePoly",		// dummy in SCI3
	/*0x67*/ "ObjectIntersect",
	/*0x68*/ "Dummy",
	/*0x69*/ "MemoryInfo",
	/*0x6a*/ "DeviceInfo",
	/*0x6b*/ "Palette",
	/*0x6c*/ "PalVary",
	/*0x6d*/ "PalCycle",
	/*0x6e*/ "RemapColors",
	/*0x6f*/ "AddLine",
	/*0x70*/ "DeleteLine",
	/*0x71*/ "UpdateLine",
	/*0x72*/ "AddPolygon",
	/*0x73*/ "DeletePolygon",
	/*0x74*/ "UpdatePolygon",
	/*0x75*/ "DoSound",
	/*0x76*/ "DoAudio",
	/*0x77*/ "DoSync",
	/*0x78*/ "Save",
	/*0x79*/ "GetTime",
	/*0x7a*/ "Platform",
	/*0x7b*/ "CD",
	/*0x7c*/ "SetQuitStr",
	/*0x7d*/ "GetConfig",
	/*0x7e*/ "Table",
	/*0x7f*/ "WinHelp",		// Windows only
	/*0x80*/ "Dummy",
	/*0x81*/ "Dummy",		// called when changing rooms in most SCI2.1 games (e.g. KQ7, GK2, MUMG deluxe, Phant1)
	/*0x82*/ "Dummy",
	/*0x83*/ "PrintDebug",	// debug function, used by Shivers (demo and full)
	/*0x84*/ "Dummy",
	/*0x85*/ "Dummy",
	/*0x86*/ "Dummy",
	/*0x87*/ "Dummy",
	/*0x88*/ "Dummy",
	/*0x89*/ "Dummy",
	/*0x8a*/ "LoadChunk",
	/*0x8b*/ "SetPalStyleRange",
	/*0x8c*/ "AddPicAt",
	/*0x8d*/ "Dummy",	// MessageBox in SCI3
	/*0x8e*/ "NewRoom",		// debug function
	/*0x8f*/ "Dummy",
	/*0x90*/ "Priority",
	/*0x91*/ "MorphOn",
	/*0x92*/ "PlayVMD",
	/*0x93*/ "SetHotRectangles",
	/*0x94*/ "MulDiv",
	/*0x95*/ "GetSierraProfileInt", // , Windows only
	/*0x96*/ "GetSierraProfileString", // , Windows only
	/*0x97*/ "SetWindowsOption", // Windows only
	/*0x98*/ "GetWindowsOption", // Windows only
	/*0x99*/ "WinDLL", // Windows only
	/*0x9a*/ "Dummy",
	/*0x9b*/ "Dummy",	// Minimize in SCI3
	/*0x9c*/ "DeletePic",
	// == SCI3 only ===============
	/*0x9d*/ "Dummy",
	/*0x9e*/ "WebConnect",
	/*0x9f*/ "Dummy",
	/*0xa0*/ "PlayDuck"
};

#endif

// Base set of opcode formats. They're copied and adjusted slightly in
// script_adjust_opcode_format depending on SCI version.
static const opcode_format g_base_opcode_formats[128][4] = {
	// 00 - 03 / bnot, add, sub, mul
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	// 04 - 07 / div, mod, shr, shl
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	// 08 - 0B / xor, and, or, neg
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	// 0C - 0F / not, eq, ne, gt
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	// 10 - 13 / ge, lt, le, ugt
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	// 14 - 17 / uge, ult, ule, bt
	{Script_None}, {Script_None}, {Script_None}, {Script_SRelative},
	// 18 - 1B / bnt, jmp, ldi, push
	{Script_SRelative}, {Script_SRelative}, {Script_SVariable}, {Script_None},
	// 1C - 1F / pushi, toss, dup, link
	{Script_SVariable}, {Script_None}, {Script_None}, {Script_Variable},
	// 20 - 23 / call, callk, callb, calle
	{Script_SRelative, Script_Byte}, {Script_Variable, Script_Byte}, {Script_Variable, Script_Byte}, {Script_Variable, Script_SVariable, Script_Byte},
	// 24 - 27 / ret, send, dummy, dummy
	{Script_End}, {Script_Byte}, {Script_Invalid}, {Script_Invalid},
	// 28 - 2B / class, dummy, self, super
	{Script_Variable}, {Script_Invalid}, {Script_Byte}, {Script_Variable, Script_Byte},
	// 2C - 2F / rest, lea, selfID, dummy
	{Script_SVariable}, {Script_SVariable, Script_Variable}, {Script_None}, {Script_Invalid},
	// 30 - 33 / pprev, pToa, aTop, pTos
	{Script_None}, {Script_Property}, {Script_Property}, {Script_Property},
	// 34 - 37 / sTop, ipToa, dpToa, ipTos
	{Script_Property}, {Script_Property}, {Script_Property}, {Script_Property},
	// 38 - 3B / dpTos, lofsa, lofss, push0
	{Script_Property}, {Script_SRelative}, {Script_SRelative}, {Script_None},
	// 3C - 3F / push1, push2, pushSelf, line
	{Script_None}, {Script_None}, {Script_None}, {Script_Word},
	// ------------------------------------------------------------------------
	// 40 - 43 / lag, lal, lat, lap
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// 44 - 47 / lsg, lsl, lst, lsp
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// 48 - 4B / lagi, lali, lati, lapi
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// 4C - 4F / lsgi, lsli, lsti, lspi
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// ------------------------------------------------------------------------
	// 50 - 53 / sag, sal, sat, sap
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// 54 - 57 / ssg, ssl, sst, ssp
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// 58 - 5B / sagi, sali, sati, sapi
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// 5C - 5F / ssgi, ssli, ssti, sspi
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// ------------------------------------------------------------------------
	// 60 - 63 / plusag, plusal, plusat, plusap
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// 64 - 67 / plussg, plussl, plusst, plussp
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// 68 - 6B / plusagi, plusali, plusati, plusapi
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// 6C - 6F / plussgi, plussli, plussti, plusspi
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// ------------------------------------------------------------------------
	// 70 - 73 / minusag, minusal, minusat, minusap
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// 74 - 77 / minussg, minussl, minusst, minussp
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// 78 - 7B / minusagi, minusali, minusati, minusapi
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// 7C - 7F / minussgi, minussli, minussti, minusspi
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param}
};
#undef END

} // End of namespace Sci

#endif // SCI_ENGINE_KERNEL_TABLES_H
