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
 * ROMLoad.h
 * 
 * Header file for ROM loading functions.
 */
 
#ifndef INCLUDED_ROMLOAD_H
#define INCLUDED_ROMLOAD_H


/******************************************************************************
 Data Structures
******************************************************************************/

/*
 * ROMInfo:
 *
 * Describes a single ROM file.
 */
struct ROMInfo
{
	// Function
	const char	*region;	// ROM region identifier (used as a key to search ROMMap)
	bool		optional;	// whether needs to be present or not
	
	// Information used to identify files
	const char	*fileName;	// file name
	UINT32		crc;		// CRC-32 checksum (same as zip format)
	unsigned	fileSize;	// file size in bytes (must be the same as all other ROMs with same region ID)
	
	// Interleaving information
	unsigned	groupSize;	// number of consecutive bytes to fetch each time (groupSize%2 must = 0, must be consistent for region)
	unsigned	offset;		// starting offset within ROM region
	unsigned	stride;		// number of bytes to skip before loading next group of bytes from file (must be >= groupSize)
	bool		byteSwap;	// swap every pair of bytes if true
};

/*
 * ROMMap:
 *
 * Describes how to map ROM regions (where to load them). This structure is
 * assembled after memory allocation is completed and tells the ROM loading
 * functions where to load files by matching region identifiers with memory
 * pointers. 
 *
 * Both pointers must be set to NULL to terminate an array of ROM maps.
 */
struct ROMMap
{
	const char	*region;	// ROM region identifier
	UINT8		*ptr;		// pointer to memory region
};


/******************************************************************************
 Functions
******************************************************************************/

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
extern void CopyRegion(UINT8 *dest, unsigned destOffset, unsigned destSize, UINT8 *src, unsigned srcSize);

/*
 * LoadROMSetFromZIPFile(Map, GameList, zipFile):
 *
 * Loads a complete ROM set from a zip archive. Automatically detects the game.
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
extern const struct GameInfo * LoadROMSetFromZIPFile(const struct ROMMap *Map, const struct GameInfo *GameList, const char *zipFile,
													 bool loadAll);


#endif	// INCLUDED_ROMLOAD_H
