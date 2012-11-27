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
 * Save and restore scene and game.
 */

#include "tinsel/actors.h"
#include "tinsel/config.h"
#include "tinsel/dialogs.h"
#include "tinsel/drives.h"
#include "tinsel/dw.h"
#include "tinsel/rince.h"
#include "tinsel/savescn.h"
#include "tinsel/timers.h"
#include "tinsel/tinlib.h"
#include "tinsel/tinsel.h"

#include "common/serializer.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "gui/message.h"

namespace Tinsel {


/**
 * The current savegame format version.
 * Our save/load system uses an elaborate scheme to allow us to modify the
 * savegame while keeping full backward compatibility, in the sense that newer
 * ScummVM versions always are able to load old savegames.
 * In order to achieve that, we store a version in the savegame files, and whenever
 * the savegame layout is modified, the version is incremented.
 *
 * This roughly works by marking each savegame entry with a range of versions
 * for which it is valid; the save/load code iterates over all entries, but
 * only saves/loads those which are valid for the version of the savegame
 * which is being loaded/saved currently.
 */
#define CURRENT_VER 2

/**
 * An auxillary macro, used to specify savegame versions. We use this instead
 * of just writing the raw version, because this way they stand out more to
 * the reading eye, making it a bit easier to navigate through the code.
 */
#define VER(x) x


//----------------- GLOBAL GLOBAL DATA --------------------

int	g_thingHeld = 0;
int	g_restoreCD = 0;
SRSTATE g_SRstate = SR_IDLE;

//----------------- EXTERN FUNCTIONS --------------------

// in DOS_DW.C
extern void syncSCdata(Common::Serializer &s);

// in PCODE.C
extern void syncGlobInfo(Common::Serializer &s);

// in POLYGONS.C
extern void syncPolyInfo(Common::Serializer &s);

extern int g_sceneCtr;

extern bool g_ASceneIsSaved;

//----------------- LOCAL DEFINES --------------------

struct SaveGameHeader {
	uint32 id;
	uint32 size;
	uint32 ver;
	char desc[SG_DESC_LEN];
	TimeDate dateTime;
	bool scnFlag;
	byte language;
	uint16 numInterpreters;			// Savegame version 2 or later only
};

enum {
	DW1_SAVEGAME_ID = 0x44575399,	// = 'DWSc' = "DiscWorld 1 ScummVM"
	DW2_SAVEGAME_ID = 0x44573253,	// = 'DW2S' = "DiscWorld 2 ScummVM"
	SAVEGAME_HEADER_SIZE = 4 + 4 + 4 + SG_DESC_LEN + 7 + 1 + 1 + 2
};

#define SAVEGAME_ID (TinselV2 ? (uint32)DW2_SAVEGAME_ID : (uint32)DW1_SAVEGAME_ID)

enum {
	// FIXME: Save file names in ScummVM can be longer than 8.3, overflowing the
	// name field in savedFiles. Raising it to 256 as a preliminary fix.
	FNAMELEN	= 256 // 8.3
};

struct SFILES {
	char	name[FNAMELEN];
	char	desc[SG_DESC_LEN + 2];
	TimeDate dateTime;
};

//----------------- LOCAL GLOBAL DATA --------------------

// FIXME: Avoid non-const global vars

static int	g_numSfiles = 0;
static SFILES	g_savedFiles[MAX_SAVED_FILES];

static bool g_NeedLoad = true;

static SAVED_DATA *g_srsd = 0;
static int g_RestoreGameNumber = 0;
static char *g_SaveSceneName = 0;
static const char *g_SaveSceneDesc = 0;
static int *g_SaveSceneSsCount = 0;
static SAVED_DATA *g_SaveSceneSsData = 0;	// points to 'SAVED_DATA ssdata[MAX_NEST]'

//------------- SAVE/LOAD SUPPORT METHODS ----------------

void setNeedLoad() {
	g_NeedLoad = true;
}

static void syncTime(Common::Serializer &s, TimeDate &t) {
	s.syncAsUint16LE(t.tm_year);
	s.syncAsByte(t.tm_mon);
	s.syncAsByte(t.tm_mday);
	s.syncAsByte(t.tm_hour);
	s.syncAsByte(t.tm_min);
	s.syncAsByte(t.tm_sec);
}

static bool syncSaveGameHeader(Common::Serializer &s, SaveGameHeader &hdr) {
	s.syncAsUint32LE(hdr.id);
	s.syncAsUint32LE(hdr.size);
	s.syncAsUint32LE(hdr.ver);

	s.syncBytes((byte *)hdr.desc, SG_DESC_LEN);
	hdr.desc[SG_DESC_LEN - 1] = 0;

	syncTime(s, hdr.dateTime);

	int tmp = hdr.size - s.bytesSynced();

	// NOTE: We can't use SAVEGAME_ID here when attempting to remove a saved game from the launcher,
	// as there is no TinselEngine initialized then. This means that we can't check if this is a DW1
	// or DW2 savegame in this case, but it doesn't really matter, as the saved game is about to be
	// deleted anyway. Refer to bug #3387551.
	bool correctID = _vm ? (hdr.id == SAVEGAME_ID) : (hdr.id == DW1_SAVEGAME_ID || hdr.id == DW2_SAVEGAME_ID);

	// Perform sanity check
	if (tmp < 0 || !correctID || hdr.ver > CURRENT_VER || hdr.size > 1024)
		return false;

	if (tmp > 0) {
		// If there's header space left, handling syncing the Scn flag and game language
		s.syncAsByte(hdr.scnFlag);
		s.syncAsByte(hdr.language);
		tmp -= 2;

		if (_vm && s.isLoading()) {
			// If the engine is loaded, ensure the Scn/Gra usage is correct, and it's the correct language
			if ((hdr.scnFlag != ((_vm->getFeatures() & GF_SCNFILES) != 0)) ||
					(hdr.language != _vm->_config->_language))
				return false;
		}
	}

	// Handle the number of interpreter contexts that will be saved in the savegame
	if (tmp >= 2) {
		tmp -= 2;
		hdr.numInterpreters = NUM_INTERPRET;
		s.syncAsUint16LE(hdr.numInterpreters);
	} else {
		hdr.numInterpreters = (TinselV2 ? 70 : 64) - 20;
	}

	// Skip over any extra bytes
	s.skip(tmp);
	return true;
}

static void syncSavedMover(Common::Serializer &s, SAVED_MOVER &sm) {
	int i, j;

	s.syncAsUint32LE(sm.bActive);
	s.syncAsSint32LE(sm.actorID);
	s.syncAsSint32LE(sm.objX);
	s.syncAsSint32LE(sm.objY);
	s.syncAsUint32LE(sm.hLastfilm);

	// Sync walk reels
	for (i = 0; i < TOTAL_SCALES; ++i)
		for (j = 0; j < 4; ++j)
			s.syncAsUint32LE(sm.walkReels[i][j]);

	// Sync stand reels
	for (i = 0; i < TOTAL_SCALES; ++i)
		for (j = 0; j < 4; ++j)
			s.syncAsUint32LE(sm.standReels[i][j]);

	// Sync talk reels
	for (i = 0; i < TOTAL_SCALES; ++i)
		for (j = 0; j < 4; ++j)
			s.syncAsUint32LE(sm.talkReels[i][j]);


	if (TinselV2) {
		s.syncAsByte(sm.bHidden);

		s.syncAsSint32LE(sm.brightness);
		s.syncAsSint32LE(sm.startColor);
		s.syncAsSint32LE(sm.paletteLength);
	}
}

static void syncSavedActor(Common::Serializer &s, SAVED_ACTOR &sa) {
	s.syncAsUint16LE(sa.actorID);
	s.syncAsUint16LE(sa.zFactor);
	s.syncAsUint16LE(sa.bAlive);
	s.syncAsUint16LE(sa.bHidden);
	s.syncAsUint32LE(sa.presFilm);
	s.syncAsUint16LE(sa.presRnum);
	s.syncAsUint16LE(sa.presPlayX);
	s.syncAsUint16LE(sa.presPlayY);
}

extern void syncAllActorsAlive(Common::Serializer &s);

static void syncNoScrollB(Common::Serializer &s, NOSCROLLB &ns) {
	s.syncAsSint32LE(ns.ln);
	s.syncAsSint32LE(ns.c1);
	s.syncAsSint32LE(ns.c2);
}

static void syncZPosition(Common::Serializer &s, Z_POSITIONS &zp) {
	s.syncAsSint16LE(zp.actor);
	s.syncAsSint16LE(zp.column);
	s.syncAsSint32LE(zp.z);
}

static void syncPolyVolatile(Common::Serializer &s, POLY_VOLATILE &p) {
	s.syncAsByte(p.bDead);
	s.syncAsSint16LE(p.xoff);
	s.syncAsSint16LE(p.yoff);
}

static void syncSoundReel(Common::Serializer &s, SOUNDREELS &sr) {
	s.syncAsUint32LE(sr.hFilm);
	s.syncAsSint32LE(sr.column);
	s.syncAsSint32LE(sr.actorCol);
}

static void syncSavedData(Common::Serializer &s, SAVED_DATA &sd, int numInterp) {
	s.syncAsUint32LE(sd.SavedSceneHandle);
	s.syncAsUint32LE(sd.SavedBgroundHandle);
	for (int i = 0; i < MAX_MOVERS; ++i)
		syncSavedMover(s, sd.SavedMoverInfo[i]);
	for (int i = 0; i < MAX_SAVED_ACTORS; ++i)
		syncSavedActor(s, sd.SavedActorInfo[i]);

	s.syncAsSint32LE(sd.NumSavedActors);
	s.syncAsSint32LE(sd.SavedLoffset);
	s.syncAsSint32LE(sd.SavedToffset);
	for (int i = 0; i < numInterp; ++i)
		sd.SavedICInfo[i].syncWithSerializer(s);
	for (int i = 0; i < MAX_POLY; ++i)
		s.syncAsUint32LE(sd.SavedDeadPolys[i]);
	s.syncAsUint32LE(sd.SavedControl);
	s.syncAsUint32LE(sd.SavedMidi);
	s.syncAsUint32LE(sd.SavedLoop);
	s.syncAsUint32LE(sd.SavedNoBlocking);

	// SavedNoScrollData
	for (int i = 0; i < MAX_VNOSCROLL; ++i)
		syncNoScrollB(s, sd.SavedNoScrollData.NoVScroll[i]);
	for (int i = 0; i < MAX_HNOSCROLL; ++i)
		syncNoScrollB(s, sd.SavedNoScrollData.NoHScroll[i]);
	s.syncAsUint32LE(sd.SavedNoScrollData.NumNoV);
	s.syncAsUint32LE(sd.SavedNoScrollData.NumNoH);

	// Tinsel 2 fields
	if (TinselV2) {
		// SavedNoScrollData
		s.syncAsUint32LE(sd.SavedNoScrollData.xTrigger);
		s.syncAsUint32LE(sd.SavedNoScrollData.xDistance);
		s.syncAsUint32LE(sd.SavedNoScrollData.xSpeed);
		s.syncAsUint32LE(sd.SavedNoScrollData.yTriggerTop);
		s.syncAsUint32LE(sd.SavedNoScrollData.yTriggerBottom);
		s.syncAsUint32LE(sd.SavedNoScrollData.yDistance);
		s.syncAsUint32LE(sd.SavedNoScrollData.ySpeed);

		for (int i = 0; i < NUM_ZPOSITIONS; ++i)
			syncZPosition(s, sd.zPositions[i]);
		s.syncBytes(sd.savedActorZ, MAX_SAVED_ACTOR_Z);
		for (int i = 0; i < MAX_POLY; ++i)
			syncPolyVolatile(s, sd.SavedPolygonStuff[i]);
		for (int i = 0; i < 3; ++i)
			s.syncAsUint32LE(sd.SavedTune[i]);
		s.syncAsByte(sd.bTinselDim);
		s.syncAsSint32LE(sd.SavedScrollFocus);
		for (int i = 0; i < SV_TOPVALID; ++i)
			s.syncAsSint32LE(sd.SavedSystemVars[i]);
		for (int i = 0; i < MAX_SOUNDREELS; ++i)
			syncSoundReel(s, sd.SavedSoundReels[i]);
	}
}

/**
 * Compare two TimeDate structs to see which one was earlier.
 * Returns 0 if they are equal, a negative value if a is lower / first, and
 * a positive value if b is lower / first.
 */
static int cmpTimeDate(const TimeDate &a, const TimeDate &b) {
	int tmp;

	#define CMP_ENTRY(x) tmp = a.x - b.x; if (tmp != 0) return tmp

	CMP_ENTRY(tm_year);
	CMP_ENTRY(tm_mon);
	CMP_ENTRY(tm_mday);
	CMP_ENTRY(tm_hour);
	CMP_ENTRY(tm_min);
	CMP_ENTRY(tm_sec);

	#undef CMP_ENTRY

	return 0;
}

/**
 * Compute a list of all available saved game files.
 * Store the file details, ordered by time, in savedFiles[] and return
 * the number of files found.
 */
int getList(Common::SaveFileManager *saveFileMan, const Common::String &target) {
	// No change since last call?
	// TODO/FIXME: Just always reload this data? Be careful about slow downs!!!
	if (!g_NeedLoad)
		return g_numSfiles;

	int i;

	const Common::String pattern = target +  ".???";
	Common::StringArray files = saveFileMan->listSavefiles(pattern);

	g_numSfiles = 0;

	for (Common::StringArray::const_iterator file = files.begin(); file != files.end(); ++file) {
		if (g_numSfiles >= MAX_SAVED_FILES)
			break;

		const Common::String &fname = *file;
		Common::InSaveFile *f = saveFileMan->openForLoading(fname);
		if (f == NULL) {
			continue;
		}

		// Try to load save game header
		Common::Serializer s(f, 0);
		SaveGameHeader hdr;
		bool validHeader = syncSaveGameHeader(s, hdr);
		delete f;
		if (!validHeader) {
			continue;	// Invalid header, or savegame too new -> skip it
			// TODO: In SCUMM, we still show an entry for the save, but with description
			// "incompatible version".
		}

		i = g_numSfiles;
#ifndef DISABLE_SAVEGAME_SORTING
		for (i = 0; i < g_numSfiles; i++) {
			if (cmpTimeDate(hdr.dateTime, g_savedFiles[i].dateTime) > 0) {
				Common::copy_backward(&g_savedFiles[i], &g_savedFiles[g_numSfiles], &g_savedFiles[g_numSfiles + 1]);
				break;
			}
		}
#endif

		Common::strlcpy(g_savedFiles[i].name, fname.c_str(), FNAMELEN);
		Common::strlcpy(g_savedFiles[i].desc, hdr.desc, SG_DESC_LEN);
		g_savedFiles[i].dateTime = hdr.dateTime;

		++g_numSfiles;
	}

	// Next getList() needn't do its stuff again
	g_NeedLoad = false;

	return g_numSfiles;
}

int getList() {
	// No change since last call?
	// TODO/FIXME: Just always reload this data? Be careful about slow downs!!!
	if (!g_NeedLoad)
		return g_numSfiles;

	return getList(_vm->getSaveFileMan(), _vm->getTargetName());
}

char *ListEntry(int i, letype which) {
	if (i == -1)
		i = g_numSfiles;

	assert(i >= 0);

	if (i < g_numSfiles)
		return which == LE_NAME ? g_savedFiles[i].name : g_savedFiles[i].desc;
	else
		return NULL;
}

static bool DoSync(Common::Serializer &s, int numInterp) {
	int	sg = 0;

	if (TinselV2) {
		if (s.isSaving())
			g_restoreCD = GetCurrentCD();
		s.syncAsSint16LE(g_restoreCD);
	}

	if (TinselV2 && s.isLoading())
		HoldItem(INV_NOICON);

	syncSavedData(s, *g_srsd, numInterp);
	syncGlobInfo(s);		// Glitter globals
	syncInvInfo(s);			// Inventory data

	// Held object
	if (s.isSaving())
		sg = WhichItemHeld();
	s.syncAsSint32LE(sg);
	if (s.isLoading()) {
		if (sg != -1 && !GetIsInvObject(sg))
			// Not a valid inventory object, so return false
			return false;

		if (TinselV2)
			g_thingHeld = sg;
		else
			HoldItem(sg);
	}

	syncTimerInfo(s);		// Timer data
	if (!TinselV2)
		syncPolyInfo(s);		// Dead polygon data
	syncSCdata(s);			// Hook Scene and delayed scene

	s.syncAsSint32LE(*g_SaveSceneSsCount);

	if (*g_SaveSceneSsCount != 0) {
		SAVED_DATA *sdPtr = g_SaveSceneSsData;
		for (int i = 0; i < *g_SaveSceneSsCount; ++i, ++sdPtr)
			syncSavedData(s, *sdPtr, numInterp);

		// Flag that there is a saved scene to return to. Note that in this context 'saved scene'
		// is a stored scene to return to from another scene, such as from the Summoning Book close-up
		// in Discworld 1 to whatever scene Rincewind was in prior to that
		g_ASceneIsSaved = true;
	}

	if (!TinselV2)
		syncAllActorsAlive(s);

	return true;
}

/**
 * DoRestore
 */
static bool DoRestore() {
	Common::InSaveFile *f =  _vm->getSaveFileMan()->openForLoading(g_savedFiles[g_RestoreGameNumber].name);

	if (f == NULL) {
		return false;
	}

	Common::Serializer s(f, 0);
	SaveGameHeader hdr;
	if (!syncSaveGameHeader(s, hdr)) {
		delete f;	// Invalid header, or savegame too new -> skip it
		return false;
	}

	// Load in the data. For older savegame versions, we potentially need to load the data twice, once
	// for pre 1.5 savegames, and if that fails, a second time for 1.5 savegames
	int numInterpreters = hdr.numInterpreters;
	int32 currentPos = f->pos();
	for (int tryNumber = 0; tryNumber < ((hdr.ver >= 2) ? 1 : 2); ++tryNumber) {
		// If it's the second loop iteration, try with the 1.5 savegame number of interpreter contexts
		if (tryNumber == 1) {
			f->seek(currentPos);
			numInterpreters = 80;
		}

		// Load the savegame data
		if (DoSync(s, numInterpreters))
			// Data load was successful (or likely), so break out of loop
			break;
	}

	uint32 id = f->readSint32LE();
	if (id != (uint32)0xFEEDFACE)
		error("Incompatible saved game");

	bool failed = (f->eos() || f->err());

	delete f;

	if (failed) {
		GUI::MessageDialog dialog(_("Failed to load game state from file."));
		dialog.runModal();
	}

	return !failed;
}

static void SaveFailure(Common::OutSaveFile *f) {
	if (f) {
		delete f;
		_vm->getSaveFileMan()->removeSavefile(g_SaveSceneName);
		g_SaveSceneName = NULL;	// Invalidate save name
	}
	GUI::MessageDialog dialog(_("Failed to save game state to file."));
	dialog.runModal();
}

/**
 * DoSave
 */
static void DoSave() {
	Common::OutSaveFile *f;
	char tmpName[FNAMELEN];

	// Next getList() must do its stuff again
	g_NeedLoad = true;

	if (g_SaveSceneName == NULL) {
		// Generate a new unique save name
		int	i;
		int	ano = 1;	// Allocated number

		while (1) {
			Common::String fname = _vm->getSavegameFilename(ano);
			strcpy(tmpName, fname.c_str());

			for (i = 0; i < g_numSfiles; i++)
				if (!strcmp(g_savedFiles[i].name, tmpName))
					break;

			if (i == g_numSfiles)
				break;
			ano++;
		}

		g_SaveSceneName = tmpName;
	}


	if (g_SaveSceneDesc[0] == 0)
		g_SaveSceneDesc = "unnamed";

	f = _vm->getSaveFileMan()->openForSaving(g_SaveSceneName);
	Common::Serializer s(0, f);

	if (f == NULL) {
		SaveFailure(f);
		return;
	}

	// Write out a savegame header
	SaveGameHeader hdr;
	hdr.id = SAVEGAME_ID;
	hdr.size = SAVEGAME_HEADER_SIZE;
	hdr.ver = CURRENT_VER;
	memcpy(hdr.desc, g_SaveSceneDesc, SG_DESC_LEN);
	hdr.desc[SG_DESC_LEN - 1] = 0;
	g_system->getTimeAndDate(hdr.dateTime);
	hdr.scnFlag = _vm->getFeatures() & GF_SCNFILES;
	hdr.language = _vm->_config->_language;

	if (!syncSaveGameHeader(s, hdr) || f->err()) {
		SaveFailure(f);
		return;
	}

	DoSync(s, hdr.numInterpreters);

	// Write out the special Id for Discworld savegames
	f->writeUint32LE(0xFEEDFACE);
	if (f->err()) {
		SaveFailure(f);
		return;
	}

	f->finalize();
	delete f;
	g_SaveSceneName = NULL;	// Invalidate save name
}

/**
 * ProcessSRQueue
 */
void ProcessSRQueue() {
	switch (g_SRstate) {
	case SR_DORESTORE:
		// If a load has been done directly from title screens, set a larger value for scene ctr so the
		// code used to skip the title screens in Discworld 1 gets properly disabled
		if (g_sceneCtr < 10)
			g_sceneCtr = 10;

		if (DoRestore()) {
			DoRestoreScene(g_srsd, false);
		}
		g_SRstate = SR_IDLE;
		break;

	case SR_DOSAVE:
		DoSave();
		g_SRstate = SR_IDLE;
		break;
	default:
		break;
	}
}


void RequestSaveGame(char *name, char *desc, SAVED_DATA *sd, int *pSsCount, SAVED_DATA *pSsData) {
	assert(g_SRstate == SR_IDLE);

	g_SaveSceneName = name;
	g_SaveSceneDesc = desc;
	g_SaveSceneSsCount = pSsCount;
	g_SaveSceneSsData = pSsData;
	g_srsd = sd;
	g_SRstate = SR_DOSAVE;
}

void RequestRestoreGame(int num, SAVED_DATA *sd, int *pSsCount, SAVED_DATA *pSsData) {
	if (TinselV2) {
		if (num == -1)
			return;
		else if (num == -2) {
			// From CD change for restore
			num = g_RestoreGameNumber;
		}
	}

	assert(num >= 0);

	g_RestoreGameNumber = num;
	g_SaveSceneSsCount = pSsCount;
	g_SaveSceneSsData = pSsData;
	g_srsd = sd;
	g_SRstate = SR_DORESTORE;
}

/**
 * Returns the index of the most recently saved savegame. This will always be
 * the file at the first index, since the list is sorted by date/time
 */
int NewestSavedGame() {
	int numFiles = getList();

	return (numFiles == 0) ? -1 : 0;
}

} // End of namespace Tinsel
