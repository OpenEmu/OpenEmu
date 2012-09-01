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
 * BlockFile.h
 * 
 * Header file for block format container file management.
 */

#ifndef INCLUDED_BLOCKFILE_H
#define INCLUDED_BLOCKFILE_H


/*
 * CBlockFile:
 *
 * Block format container file. The file format is a series of consecutive,
 * variable-length blocks referenced by unique name strings. 
 *
 * All strings (comments and names) will be truncated to 1024 bytes, not
 * including the null terminator.
 *
 * Members do not generate any output messages.
 */
class CBlockFile
{
public:
	/*
	 * Read(data, numBytes):
	 *
	 * Reads data from the current file position. 
	 *
	 * Parameters:
	 *		data		Buffer to read to.
	 *		numBytes	Number of bytes to read.
	 *
	 * Returns:
	 *		Number of bytes read. If not the same as numBytes, an error
	 *		occurred.
	 */
	unsigned Read(void *data, unsigned numBytes);
	
	/*
	 * FindBlock(name):
	 *
	 * Scans the file for a block with the given name string. When it is found,
	 * the file pointer is set to the beginning of the data region.
	 *
	 * Parameters:
	 *		name	Name of block to locate.
	 *
	 * Returns:
	 *		OKAY if found, FAIL if unable to locate.
	 */
	bool FindBlock(const char *name);
	
	/*
	 * Write(data, numBytes):
	 *
 	 * Outputs data at the current file pointer position. Updates the block
 	 * header appropriately.
	 *
	 * Parameters:
	 *		data		Data to write.
	 *		numBytes	Number of bytes to write.
	 */
	void Write(const void *data, unsigned numBytes);

	/*
	 * NewBlock(name, comment):
	 *
	 * Begins a new block. Writes the block header and sets the file pointer to
	 * the beginning of its data area.
	 *
	 * Parameters:
	 *		name		Block name. Must be unique and not NULL.
	 *		comment		Comment string to embed in the block header.
	 */
	void NewBlock(const char *title, const char *comment);

	/*
	 * Create(file, headerName, comment):
	 *
	 * Opens a block file for writing and creates the header block. This  
	 * function must be called before attempting to write data. Otherwise, all
	 * write commands will be silently ignored. Read commands will be ignored
	 * and will always return 0's.
	 * 
	 * Parameters:
	 *		file		File path.
	 * 		headerName	Block name for header. Must be unique and not NULL.
	 *		comment		Comment string that will be embedded into file header.
	 *
	 * Returns:
  	 * 		OKAY if successfully opened, otherwise FAIL.
	 */
	bool Create(const char *file, const char *headerName, const char *comment);

	/*
	 * Load(file):
	 *
	 * Open a block file file for reading.
	 *
	 * Parameters:
	 *		file	File path.
	 *
	 * Returns:
	 *		OKAY if successfully opened and confirmed to be a valid Supermodel
	 *		block file, otherwise FAIL. If the file could not be opened, all 
	 *		subsequent operations will be silently ignored (reads will return
	 *		0's). Write commands will be ignored.
	 */
	bool Load(const char *file);

	/*
	 * Close(void):
	 *
	 * Closes the file.
	 */
	void Close(void);

	/*
	 * CBlockFile(void):
	 * ~CBlockFile(void):
 	 * 
 	 * Constructor and destructor.
	 */
	CBlockFile(void);
	~CBlockFile(void);

private:
	// Helper functions
	void		ReadString(char *str, unsigned strLen, unsigned maxLen);
	unsigned	ReadBytes(void *data, unsigned numBytes);
	unsigned 	ReadDWord(UINT32 *data);
	void 		UpdateBlockSize(void);
	void 		WriteByte(UINT8 data);
	void 		WriteDWord(UINT32 data);
	void 		WriteBytes(const void *data, unsigned numBytes);
	void 		WriteBlockHeader(const char *name, const char *comment);

	// File state data
	char		strBuf[1026];	// buffers up to a 1024-character string, its terminator, and an extra terminator (just in case)
	FILE		*fp;
	int			mode;			// 'r' for read, 'w' for write
	long int	fileSize;		// size of file in bytes
	long int	blockStartPos;	// points to beginning of current block (or file) header
	long int	dataStartPos;	// points to beginning of current block's data section 
};


#endif	// INCLUDED_BLOCKFILE_H
