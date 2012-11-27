// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
  Table file format for scummvm/queen
	uint32 : 'QTBL'
	uint32 : version/tag
	repeat (number of resources tables) {
		uint16 : number of entries
		repeat (number of entries) {
			char[12] : name
			uint8    : queen.%d
			uint32   : offset in queen.%d
			uint32   : length in queen.%d
		}
	}
*/

#define MAX_VERSIONS  20
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

typedef unsigned char   uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;

typedef struct DataFileEntry {
	uint8 bundle;
	uint32 offset;
	uint32 length;
	char name[13];
} DataFileEntry;

typedef struct DataFileEntriesTable {
	const DataFileEntry *fileEntries;
	uint16 fileEntriesCount;
} DataFileEntriesTable;

typedef struct TableFile {
	const char *outfile;
	DataFileEntriesTable dataFileEntriesTable[MAX_VERSIONS];
	uint16 dataFileEntriesTableCount;
} TableFile;

typedef struct GameVersion {
	const char *id;
	uint32 dataSize;
	DataFileEntry *dataFileEntries;
	uint16 dataFileEntriesCount;
	uint8 queenTblVersion;
	uint32 queenTblOffset;
} GameVersion;

#include "fat_eng_floppy.h"
#include "fat_eng_cdrom.h"
#include "fat_fre_floppy.h"
#include "fat_fre_cdrom.h"
#include "fat_ger_floppy.h"
#include "fat_ger_cdrom.h"
#include "fat_ita_floppy.h"
#include "fat_ita_cdrom.h"
#include "fat_spa_cdrom.h"
#include "fat_heb_cdrom.h"
#include "fat_pc_demo_pcgames.h"
#include "fat_pc_demo.h"
#include "fat_pc_interview.h"
#include "fat_amiga_eng_floppy.h"
#include "fat_amiga_demo.h"
#include "fat_amiga_interview.h"


#define FAT(x) x, (sizeof(x)/sizeof(x[0]))

static GameVersion gameVersionsTable[] = {
	{ "PEM10",  22677657, FAT(fatEngFl),          1, 0 },
	{ "CEM10", 190787021, FAT(fatEngCd),          1, 0 },
	{ "PFM10",  22157304, FAT(fatFreFl),          1, 0 },
	{ "CFM10", 186689095, FAT(fatFreCd),          1, 0 },
	{ "PGM10",  22240013, FAT(fatGerFl),          1, 0 },
	{ "CGM10", 217648975, FAT(fatGerCd),          1, 0 },
	{ "PIM10",  22461366, FAT(fatItaFl),          1, 0 },
	{ "CIM10", 190795582, FAT(fatItaCd),          1, 0 },
	{ "CSM10", 190730602, FAT(fatSpaCd),          1, 0 },
	{ "CHM10", 190705558, FAT(fatHebCd),          1, 0 },
	{ "PE100",   3724538, FAT(fatPCDemoPcGames),  1, 0 },
	{ "PE100",   3732177, FAT(fatPCDemo),         1, 0 },
	{ "PEint",   1915913, FAT(fatPCInterview),    1, 0 },
	{ "aEM10",    351775, FAT(fatAmigaEngFl),     2, 0 },
	{ "CE101",    563335, FAT(fatAmigaDemo),      2, 0 },
	{ "PE100",    597032, FAT(fatAmigaInterview), 2, 0 }
};

static const uint32 QTBL_TAG = 0x5154424C;
static const uint32 CURRENT_VERSION = 2;

static void writeByte(FILE *fp, uint8 b) {
	fwrite(&b, 1, 1, fp);
}

static void writeUint16BE(FILE *fp, uint16 value) {
	writeByte(fp, (uint8)(value >> 8));
	writeByte(fp, (uint8)(value & 0xFF));
}

static void writeUint32BE(FILE *fp, uint32 value) {
	writeUint16BE(fp, (uint16)(value >> 16));
	writeUint16BE(fp, (uint16)(value & 0xFFFF));
}

static void writeEntry(FILE *fp, const DataFileEntry *dfe) {
	fwrite(dfe->name, 12, 1, fp);
	writeByte(fp, dfe->bundle);
	writeUint32BE(fp, dfe->offset);
	writeUint32BE(fp, dfe->length);
}

static void createTableFile(TableFile *tf) {
	FILE *out;
	uint16 i, j;
	uint32 offset; /* dump offset */

	/* setup file entries table */
	assert(ARRAYSIZE(gameVersionsTable) < MAX_VERSIONS);
	for (i = 0; i < ARRAYSIZE(gameVersionsTable); ++i) {
		const GameVersion *gv = &gameVersionsTable[i];
		tf->dataFileEntriesTable[i].fileEntries = gv->dataFileEntries;
		tf->dataFileEntriesTable[i].fileEntriesCount = gv->dataFileEntriesCount;
	}
	tf->dataFileEntriesTableCount = ARRAYSIZE(gameVersionsTable);

	/* write queen table file */
	out = fopen(tf->outfile, "wb");
	if (!out) {
		printf("ERROR: can't write output file.\n");
		return;
	}
	/* write header tag */
	writeUint32BE(out, QTBL_TAG);
	/* write version */
	writeUint32BE(out, CURRENT_VERSION);
	/* write tables */
	offset = 4 + 4;
	for (i = 0; i < tf->dataFileEntriesTableCount; ++i) {
		const DataFileEntriesTable *dfet = &tf->dataFileEntriesTable[i];
		/* write number of entries in table */
		writeUint16BE(out, dfet->fileEntriesCount);
		/* write table entries */
		for (j = 0; j < dfet->fileEntriesCount; ++j) {
			const DataFileEntry *dfe = &dfet->fileEntries[j];
			writeEntry(out, dfe);
		}
		assert(gameVersionsTable[i].queenTblVersion <= CURRENT_VERSION);
		gameVersionsTable[i].queenTblOffset = offset;
		/* update offset */
		offset += 2 + dfet->fileEntriesCount * (12 + 1 + 4 + 4);
	}
	fclose(out);
}

static void printGameVersionTable() {
	unsigned int i;
	printf("const RetailGameVersion Resource::_gameVersions[] = {\n");
	for (i = 0; i < ARRAYSIZE(gameVersionsTable); ++i) {
		const GameVersion *gv = &gameVersionsTable[i];
		printf("\t{ \"%s\", %d, 0x%08X, %9d },\n", gv->id, gv->queenTblVersion, gv->queenTblOffset, gv->dataSize);
	}
	printf("};\n");
}

int main(int argc, char *argv[]) {
	TableFile tf;
	if (argc < 2) {
		printf("syntax: %s tablefile\n", argv[0]);
		exit(0);
	}
	memset(&tf, 0, sizeof(tf));
	tf.outfile = argv[1];
	createTableFile(&tf);
	printGameVersionTable();
	return 0;
}
