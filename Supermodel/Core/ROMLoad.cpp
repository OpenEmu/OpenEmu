/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson 
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/
 
/*
 * ROMLoad.cpp
 * 
 * ROM loading functions.
 */
 
#include <new>
#include <cstring>
#include "Supermodel.h"
#include "Pkgs/unzip.h"


/*
 * CopyRegion(dest, destOffset, destSize, src, srcSize):
 *
 * Repeatedly mirror (copy) to destination from source until destination is
 * filled.
 *
 * Parameters:
 *		dest		Destination region.
 *		destOffset	Offset within destination to begin mirroring to.
 *		destSize	Size in bytes of destination region.
 *		src			Source region to copy from.
 *		srcSize		Size of region to copy from.
 */
void CopyRegion(UINT8 *dest, unsigned destOffset, unsigned destSize, UINT8 *src, unsigned srcSize)
{
	while (destOffset < destSize)
	{
		// If we'll overrun the destination, trim the copy length
		if ((destOffset+srcSize) >= destSize)
			srcSize = destSize-destOffset;
		
		// Copy!
		memcpy(&dest[destOffset], src, srcSize);
		
		destOffset += srcSize;
	}
}

// Search for a ROM within a single game based on its CRC
static bool FindROMByCRCInGame(const struct GameInfo **gamePtr, int *romIdxPtr, const struct GameInfo *Game, UINT32 crc)
{
	unsigned	j;
	
	for (j = 0; Game->ROM[j].region != NULL; j++)
	{
		if (crc == Game->ROM[j].crc)	// found it!
		{
			*gamePtr = Game;	// I know this seems redundant, but we do it here because FindROMByCRC() uses this function
			*romIdxPtr = j;
			return OKAY;
		}
	}
	
	return FAIL;
}
	
// Search for a ROM in the complete game list based on CRC32 and return its GameInfo and ROMInfo entries
static bool FindROMByCRC(const struct GameInfo **gamePtr, int *romIdxPtr, const struct GameInfo *GameList, const struct GameInfo *TryGame, UINT32 crc)
{
	unsigned	i;
	
	if (TryGame != NULL)
	{
		if (FindROMByCRCInGame(gamePtr, romIdxPtr, TryGame, crc) == OKAY)
			return OKAY;
	}
	
	for (i = 0; GameList[i].title != NULL; i++)
	{
		if (FindROMByCRCInGame(gamePtr, romIdxPtr, &(GameList[i]), crc) == OKAY)
			return OKAY;
	}
	
	return FAIL;
}

// Returns true if this ROM appears only a single time in the entire game list (ie., it is not shared between games)
static bool ROMIsUnique(const struct GameInfo *GameList, UINT32 crc)
{
	int	timesFound = 0;
	
	for (int i = 0; GameList[i].title != NULL; i++)
	{
		for (int j = 0; GameList[i].ROM[j].region != NULL; j++)
		{
			if (crc == GameList[i].ROM[j].crc)
				timesFound++;
		}
	}
	
	return (timesFound == 1) ? true : false;
}

static void ByteSwap(UINT8 *buf, unsigned size)
{
	unsigned	i;
	UINT8		x;
	
	for (i = 0; i < size; i += 2)
	{
		x = buf[i+0];
		buf[i+0] = buf[i+1];
		buf[i+1] = x;
	}	 
}

// Load a single ROM file
static bool LoadROM(UINT8 *buf, unsigned bufSize, const struct ROMMap *Map, const struct ROMInfo *ROM, unzFile zf, const char *zipFile, bool loadAll)
{
	char			file[2048+1];
	int				err, bytes;
	unz_file_info	fileInfo;
	unsigned		i, j, destIdx, srcIdx;
	
	// Read the file into the buffer
	err = unzGetCurrentFileInfo(zf, &fileInfo, file, 2048, NULL, 0, NULL, 0);
	if (err != UNZ_OK)
		return ErrorLog("Unable to extract a file name from '%s'.", zipFile);
	if (fileInfo.uncompressed_size != ROM->fileSize)
		return ErrorLog("'%s' in '%s' is not the correct size (must be %d bytes).", file, zipFile, ROM->fileSize);
	err = unzOpenCurrentFile(zf);
	if (UNZ_OK != err)
		return ErrorLog("Unable to read '%s' from '%s'.", file, zipFile);
	bytes = unzReadCurrentFile(zf, buf, bufSize);
	if (bytes != ROM->fileSize)
	{
		unzCloseCurrentFile(zf);
		return ErrorLog("Unable to read '%s' from '%s'.", file, zipFile);
	}
	err = unzCloseCurrentFile(zf);
	if (UNZ_CRCERROR == err)
		ErrorLog("CRC error reading '%s' from '%s'. File may be corrupt.", file, zipFile);
		
	// Byte swap
	if (ROM->byteSwap)
		ByteSwap(buf, ROM->fileSize);
	
	// Find out how to map the ROM and do it
	for (i = 0; Map[i].region != NULL; i++)
	{
		if (!strcmp(Map[i].region, ROM->region))
		{
			destIdx = ROM->offset;
			for (srcIdx = 0; srcIdx < ROM->fileSize; )
			{
				for (j = 0; j < ROM->groupSize; j++)
					Map[i].ptr[destIdx+j] = buf[srcIdx++];
				
				destIdx += ROM->stride;
			}
			
			return OKAY;
		}
	}
	
	if (loadAll)	// need to load all ROMs, so there should be no unmapped regions
		return ErrorLog("%s:%d: No mapping for '%s'.", __FILE__, __LINE__, ROM->region);
	else
		return OKAY;
}	
	
/*
 * LoadROMSetFromZIPFile(Map, GameList, zipFile):
 *
 * Loads a complete ROM set from a ZIP archive. Automatically detects the game.
 * If multiple games exist within the archive, an error will be printed and all
 * but the first detected game will be ignored.
 *
 * Parameters:
 *		Map			A list of pointers to the memory buffers for each ROM 
 *					region.
 *		GameList	List of all supported games and their ROMs.
 *		zipFile		ZIP file to load from.
 *		loadAll		If true, will check to ensure all ROMs were loaded.
 *					Otherwise, omits this check and loads only specified 
 *					regions.
 *
 * Returns:
 *		Pointer to GameInfo struct for loaded game if successful, NULL 
 *		otherwise. Prints errors.
 */
const struct GameInfo * LoadROMSetFromZIPFile(const struct ROMMap *Map, const struct GameInfo *GameList, const char *zipFile, bool loadAll)
{
	unzFile					zf;
	unz_file_info			fileInfo;
	const struct GameInfo	*Game = NULL;
	const struct GameInfo	*CurGame;	// this is the game to which the last ROM found is thought to belong
	int						romIdx;		// index within Game->ROM
	unsigned				romsFound[sizeof(Game->ROM)/sizeof(struct ROMInfo)], numROMs;
	int						err;
	unsigned				i, maxSize;
	bool					multipleGameError = false;
	UINT8					*buf;
	
	// Try to open file
	zf = unzOpen(zipFile);
	if (NULL == zf)
	{
		ErrorLog("Could not open '%s'.", zipFile);
		return NULL;
	}
		
	// First pass: scan every file and determine the game
	err = unzGoToFirstFile(zf);
	if (UNZ_OK != err)
	{
		ErrorLog("Unable to read the contents of '%s' (code %X)", zipFile, err);
		return NULL;
	}
	for (; err != UNZ_END_OF_LIST_OF_FILE; err = unzGoToNextFile(zf))
	{
		// Identify the file we're looking at
		err = unzGetCurrentFileInfo(zf, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
		if (err != UNZ_OK)
			continue;			
		if (OKAY != FindROMByCRC(&CurGame, &romIdx, GameList, Game, fileInfo.crc))
			continue;
		
		// If the ROM appears in multiple games, do not use it to identify the game!
		if (!ROMIsUnique(GameList, fileInfo.crc))
			continue;
			
		// We have a unique ROM used by a single game; identify that game
		if (Game == NULL)	// this is the first game we've identified within the ZIP
		{
			Game = CurGame;
			DebugLog("ROM set identified: %s (%s), %s\n", Game->id, Game->title, zipFile);
		}
		else				// we've already identified a game
		{
			if (CurGame != Game)	// another game?
			{
				DebugLog("%s also contains: %s (%s)\n", zipFile, CurGame->id, CurGame->title);
				if (multipleGameError == false)	// only warn about this once
				{
					ErrorLog("Multiple games were found in '%s'; loading '%s'.", zipFile, Game->title);
					multipleGameError = true;
				}
			}
		}
	}
	
	// Second pass: check if all ROM files for the identified game are present
	err = unzGoToFirstFile(zf);
	if (UNZ_OK != err)
	{
		ErrorLog("Unable to read the contents of '%s' (code %X)", zipFile, err);
		return NULL;
	}
	memset(romsFound, 0, sizeof(romsFound));	// here, romsFound[] indicates which ROMs were found in the ZIP file for the game
	for (; err != UNZ_END_OF_LIST_OF_FILE; err = unzGoToNextFile(zf))
	{
		// Identify the file we're looking at
		err = unzGetCurrentFileInfo(zf, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
		if (err != UNZ_OK)
			continue;			
		
		// If it's not part of the game we've identified, skip it
		if (OKAY != FindROMByCRCInGame(&CurGame, &romIdx, Game, fileInfo.crc))
			continue;
		
		// If we have found a ROM for the correct game, mark its corresponding indicator
		romsFound[romIdx] = 1;
	}
	
	// Compute how many ROM files this game has
	for (numROMs = 0; Game->ROM[numROMs].region != NULL; numROMs++)
		;
	
	// If not all ROMs were present, tell the user
	err = OKAY;
	for (i = 0; i < numROMs; i++)
	{
		if ((0 == romsFound[i]) && !Game->ROM[i].optional)	// if not found and also not optional
			err |= (int) ErrorLog("'%s' (CRC=%08X) is missing from '%s'.", Game->ROM[i].fileName, Game->ROM[i].crc, zipFile);
	}
	if (err != OKAY)
	{
		unzClose(zf);
		return NULL;
	}
		
	// Allocate a scratch buffer big enough to hold the biggest ROM
	maxSize = 0;
	for (i = 0; i < numROMs; i++)
	{
		if (Game->ROM[i].fileSize > maxSize)
			maxSize = Game->ROM[i].fileSize;
	}
	buf = new(std::nothrow) UINT8[maxSize];
	if (NULL == buf)
	{
		unzClose(zf);
		ErrorLog("Insufficient memory to load ROM files (%d bytes).", maxSize);
		return NULL;
	}
		
	// Third pass: load the ROMs
	memset(romsFound, 0, sizeof(romsFound));	// now, romsFound[] is used to indicate whether we successfully loaded the ROM
	err = unzGoToFirstFile(zf);
	if (UNZ_OK != err)
	{
		ErrorLog("Unable to read the contents of '%s' (code %X).", zipFile, err);
		err = FAIL;
		goto Quit;
	}
	for (; err != UNZ_END_OF_LIST_OF_FILE; err = unzGoToNextFile(zf))
	{
		err = unzGetCurrentFileInfo(zf, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
		if (err != UNZ_OK)
			continue;			
		
		// If this ROM is not part of the game we're loading, skip it
		if (OKAY != FindROMByCRCInGame(&CurGame, &romIdx, Game, fileInfo.crc))
			continue;

		// Load the ROM and mark that we did so successfully
		if (OKAY == LoadROM(buf, maxSize, Map, &Game->ROM[romIdx], zf, zipFile, loadAll))
			romsFound[romIdx] = 1;	// success! mark as loaded
	}
	
	// Ensure all ROMs were loaded
	if (loadAll)
	{
		// See if any ROMs (that are not optional) could not be found
		err = OKAY;
		for (i = 0; i < numROMs; i++)
		{
			if (!(romsFound[i] || Game->ROM[i].optional))	// if ROM not found and also not optional
				err = ErrorLog("Could not load '%s' (CRC=%08X) from '%s'.", Game->ROM[i].fileName, Game->ROM[i].crc, zipFile);
		}
	}
	else
		err = OKAY;
				
Quit:
	unzClose(zf);
	delete [] buf;
	return (err == OKAY) ? Game : NULL;
}
