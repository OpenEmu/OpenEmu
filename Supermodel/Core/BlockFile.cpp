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
 * BlockFile.cpp
 *
 * Block format container file management. Implementation of the CBlockFile
 * class.
 */

#include <cstdio>
#include <cstring>
#include "Supermodel.h"


/******************************************************************************
 Output Functions
******************************************************************************/

void CBlockFile::ReadString(char *str, unsigned strLen, unsigned maxLen)
{
	if (NULL == fp)
		return;
	if (strLen>maxLen)
		strLen = maxLen;
	fread(str, sizeof(char), strLen, fp);
	str[strLen] = '\0';
}

unsigned CBlockFile::ReadBytes(void *data, unsigned numBytes)
{
	if (NULL == fp)
		return 0;
	return fread(data, sizeof(UINT8), numBytes, fp);
}

unsigned CBlockFile::ReadDWord(UINT32 *data)
{
	if (NULL == fp)
		return 0;
	fread(data, sizeof(UINT32), 1, fp);
	return 4;
}
	
void CBlockFile::UpdateBlockSize(void)
{
	long int	curPos;
	unsigned	newBlockSize;
	
	if (NULL == fp)
		return;
	curPos = ftell(fp);				// save current file position
	fseek(fp, blockStartPos, SEEK_SET);
	newBlockSize = curPos - blockStartPos;
	fwrite(&newBlockSize, sizeof(UINT32), 1, fp);
	fseek(fp, curPos, SEEK_SET);	// go back
}

void CBlockFile::WriteByte(UINT8 data)
{
	if (NULL == fp)
		return;
	fwrite(&data, sizeof(UINT8), 1, fp);
	UpdateBlockSize();
}

void CBlockFile::WriteDWord(UINT32 data)
{
	if (NULL == fp)
		return;
	fwrite(&data, sizeof(UINT32), 1, fp);
	UpdateBlockSize();
}

void CBlockFile::WriteBytes(const void *data, unsigned numBytes)
{
	if (NULL == fp)
		return;
	fwrite(data, sizeof(UINT8), numBytes, fp);
	UpdateBlockSize();
}

void CBlockFile::WriteBlockHeader(const char *name, const char *comment)
{
	unsigned	nameLen, commentLen;
	const char	nullComment[1] = {'\0'};

	if (NULL == fp)
		return;
		
	if (comment == NULL)
		comment = nullComment;
		
	nameLen = strlen(name);
	commentLen = strlen(comment);
	if (nameLen > 1024)
		nameLen = 1024;
	if (commentLen > 1024)
		commentLen = 1024;

	// Record current block starting position
	blockStartPos = ftell(fp);

	// Write the total block length field
	WriteDWord(0);	// will be automatically updated as we write the file
	
	// Write name and comment lengths
	WriteDWord(nameLen+1);
	WriteDWord(commentLen+1);
	WriteBytes(name, nameLen);
	WriteByte(0);
	WriteBytes(comment, commentLen);
	WriteByte(0);
	
	// Record the start of the current data section
	dataStartPos = ftell(fp);
}	


/******************************************************************************
 Block Format Container File Implementation
 
 Files are just a consecutive array of blocks that must be searched.
 
 Block Format
 ------------
 blockLength	(UINT32)	Total length of block in bytes.
 nameLength		(UINT32)	Length of name field including terminating 0 (up to
							1025).
 commentLength	(UINT32)	Same as above, but for comment string.
 name			...			Name string (null-terminated, up to 1025 bytes).
 comment		...			Comment string (same as above).
 data			...			Raw data (blockLength - total header size).
******************************************************************************/

unsigned CBlockFile::Read(void *data, unsigned numBytes)
{
	if (mode == 'r')
		return ReadBytes(data, numBytes);
	return 0;
}

void CBlockFile::Write(const void *data, unsigned numBytes)
{
	if (mode == 'w')
		WriteBytes(data, numBytes);
}

void CBlockFile::NewBlock(const char *name, const char *comment)
{
	if (mode == 'w')
		WriteBlockHeader(name, comment);
}

bool CBlockFile::FindBlock(const char *name)
{
	long int	curPos = 0;
	unsigned	blockLen, nameLen, commentLen;
	
	if (mode != 'r')
		return FAIL;
		
	fseek(fp, 0, SEEK_SET);
	
	while (curPos < fileSize)
	{
		blockStartPos = curPos;
		
		// Read header
		curPos += ReadDWord(&blockLen);
		curPos += ReadDWord(&nameLen);
		curPos += ReadDWord(&commentLen);
		ReadString(strBuf,nameLen,1025);
		
		// Is this the block we want?
		if (!strcmp(strBuf,name))
		{
			fseek(fp, blockStartPos+12+nameLen+commentLen, SEEK_SET);	// move to beginning of data
			dataStartPos = ftell(fp);
			return OKAY;
		}
		
		// Move to next block
		fseek(fp, blockStartPos+blockLen, SEEK_SET);
		curPos = blockStartPos+blockLen;
		if (blockLen == 0)	// this would never advance
			break;
	}
	
	return FAIL;
}

bool CBlockFile::Create(const char *file, const char *headerName, const char *comment)
{
	fp = fopen(file, "wb");
	if (NULL == fp)
		return FAIL;
	mode = 'w';
	WriteBlockHeader(headerName, comment);
	return OKAY;
}
	
bool CBlockFile::Load(const char *file)
{
	fp = fopen(file, "rb");
	if (NULL == fp)
		return FAIL;
	mode = 'r';
	
	// TODO: is this a valid block file?
	
	// Get the file size
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	return OKAY;
}
	
void CBlockFile::Close(void)
{
	if (fp != NULL)
		fclose(fp);
	fp = NULL;
	mode = 0;
}

CBlockFile::CBlockFile(void)
{
	fp = NULL;
	mode = 0;		// neither reading nor writing (do nothing)
}

CBlockFile::~CBlockFile(void)
{
	if (fp != NULL)	// in case user forgot
		fclose(fp);
	fp = NULL;
	mode = 0;
}
