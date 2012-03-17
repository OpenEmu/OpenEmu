/***************************************************************************
 * Gens: Zip File Compression Class.                                       *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "Zip.hpp"

#include <stdlib.h>
#include <string.h>

#include "minizip/unzip.h"
#include "ui/gens_ui.hpp"

using std::string;
using std::list;


Zip::Zip(const bool showErrMsg)
{
	m_showErrMsg = showErrMsg;
}

Zip::~Zip()
{
}


/**
 * detectFormat(): Detect if a file is in GZip format.
 * @param f File pointer of the file to check.
 * @return True if this file is in GZip format.
 */
bool Zip::detectFormat(FILE *f)
{
	// Magic Number for Zip:
	// First four bytes: "PK\x03\x04"
	static const unsigned char magicZip[] = {'P', 'K', 0x03, 0x04};
	
	unsigned char buf[4];
	fseek(f, 0, SEEK_SET);
	fread(buf, 4, sizeof(unsigned char), f);
	
	return (memcmp(buf, magicZip, sizeof(magicZip)) == 0);
}


/**
 * getNumFiles(): Gets the number of files in the specified archive.
 * @param filename Filename of the archive.
 * @return Number of files, or 0 on error.
 */
int Zip::getNumFiles(const string& zFilename)
{
	unzFile f;
	int i, numFiles;
	
	// Open the Zip file.
	f = unzOpen(zFilename.c_str());
	if (!f)
		return 0;
	
	// Count the number of files in the archive.
	numFiles = 0;
	i = unzGoToFirstFile(f);
	while (i == UNZ_OK)
	{
		if (i == UNZ_OK)
			numFiles++;
		i = unzGoToNextFile(f);
	}
	unzClose(f);
	
	// Return the number of files found.
	return numFiles;
}


/**
 * getFileInfo(): Get information about all files in the specified archive.
 * @param zFilename Filename of the archive.
 * @return Pointer to list of CompressedFile structs, or NULL on error.
 */
list<CompressedFile>* Zip::getFileInfo(const string& zFilename)
{
	list<CompressedFile> *lst;
	CompressedFile file;
	
	unzFile f;
	unz_file_info zinfo;
	int i;
	char ROMFileName[132];
	
	// Open the Zip file.
	f = unzOpen(zFilename.c_str());
	if (!f)
		return 0;
	
	// Create the list.
	lst = new list<CompressedFile>;
	
	// Find the first ROM file in the Zip archive.
	i = unzGoToFirstFile(f);
	while (i == UNZ_OK)
	{
		unzGetCurrentFileInfo(f, &zinfo, ROMFileName, 128, NULL, 0, NULL, 0);
		
		// Store the ROM file information.
		file.filename = string(ROMFileName);
		file.filesize = zinfo.uncompressed_size;
		lst->push_back(file);
		
		// Go to the next file.
		i = unzGoToNextFile(f);
	}
	unzClose(f);
	
	// Return the list of files.
	return lst;
}


/**
 * getFile(): Gets the specified file from the specified archive.
 * @param zFilename Filename of the archive.
 * @param fileInfo Information about the file to extr
 * @param buf Buffer to write the file to.
 * @param size Size of the buffer, in bytes.
 * @return Number of bytes read, or -1 on error.
 */
int Zip::getFile(const string& zFilename, const CompressedFile *fileInfo,
		 unsigned char *buf, const int size)
{
	unzFile f;
	int zResult;
	
	// All parameters must be specified.
	if (!fileInfo || !buf || !size)
		return 0;
	
	f = unzOpen(zFilename.c_str());
	if (!f)
		return 0;
	
	// Locate the ROM in the Zip file.
	if (unzLocateFile(f, fileInfo->filename.c_str(), 1) != UNZ_OK ||
	    unzOpenCurrentFile(f) != UNZ_OK)
	{
		GensUI::msgBox("Error loading the ROM file from the ZIP archive.", "ZIP File Error");
		unzClose(f);
		return -1;
	}
	
	// Decompress the ROM.
	zResult = unzReadCurrentFile(f, buf, size);
	unzClose(f);
	if ((zResult <= 0) || (zResult != size))
	{
		char tmp[64];
		strcpy(tmp, "Error in ZIP file: \n");
		
		switch (zResult)
		{
			case UNZ_ERRNO:
				strcat(tmp, "Unknown...");
				break;
			case UNZ_EOF:
				strcat(tmp, "Unexpected end of file.");
				break;
			case UNZ_PARAMERROR:
				strcat(tmp, "Parameter error.");
				break;
			case UNZ_BADZIPFILE:
				strcat(tmp, "Bad ZIP file.");
				break;
			case UNZ_INTERNALERROR:
				strcat(tmp, "Internal error.");
				break;
			case UNZ_CRCERROR:
				strcat(tmp, "CRC error.");
				break;
		}
		GensUI::msgBox(tmp, "ZIP File Error");
		return -1;
	}
	
	// Return the filesize.
	return size;
}
