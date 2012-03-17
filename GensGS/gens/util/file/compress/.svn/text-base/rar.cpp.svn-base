/***************************************************************************
 * Gens: RAR File Compression Class.                                       *
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

#include "rar.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"

// Error number variable.
#include <cerrno>

// popen wrapper
#include "popen_wrapper.h"

#include <sstream>
using std::string;
using std::stringstream;
using std::list;

// Newline constant: "\r\n" on Win32, "\n" on everything else.
#ifdef GENS_OS_WIN32
#define RAR_NEWLINE "\r\n"
#define RAR_NEWLINE_LENGTH 2
#define RAR_NAME "WinRAR"
#else
#define RAR_NEWLINE "\n"
#define RAR_NEWLINE_LENGTH 1
#define RAR_NAME "rar"
#endif


RAR::RAR(const bool showErrMsg)
{
	m_showErrMsg = showErrMsg;
}

RAR::~RAR()
{
}


void RAR::errOpeningRAR(int errorNumber)
{
	fprintf(stderr, "Error opening rar: %s.\n", strerror(errorNumber));
	
	if (m_showErrMsg)
	{
		string sErr = "Could not run rar. Please make sure " RAR_NAME " is installed\n"
			      "and is configured properly in the \"BIOS/Misc Files\" window.\n\n"
			      "Error description: " + string(strerror(errorNumber)) + ".";
		
		GensUI::msgBox(sErr, "RAR Error", GensUI::MSGBOX_ICON_WARNING);
	}
}


/**
 * detectFormat(): Detect if a file is in RAR format.
 * @param f File pointer of the file to check.
 * @return True if this file is in GZip format.
 */
bool RAR::detectFormat(FILE *f)
{
	// Magic Number for RAR:
	// First four bytes: "Rar!"
	static const unsigned char magicRar[] = {'R', 'a', 'r', '!'};
	
	unsigned char buf[4];
	fseek(f, 0, SEEK_SET);
	fread(buf, 4, sizeof(unsigned char), f);
	
	return (memcmp(buf, magicRar, sizeof(magicRar)) == 0);
}


bool RAR::checkExternalExec(void)
{
	// Check that the external RAR executable is working.
	FILE *pRAR;
	char buf[512];
	
	// Build the command line.
	stringstream ssCmd;
	ssCmd << "\"" << Misc_Filenames.RAR_Binary << "\"";
#ifndef GENS_OS_WIN32
	ssCmd << " 2>&1";
#endif
	
	pRAR = gens_popen(ssCmd.str().c_str(), "r");
	if (!pRAR)
	{
		// External RAR executable is broken.
		errOpeningRAR(errno);
		return false;
	}
	
	// Read the first 512 bytes from the pipe.
	fread(buf, 1, sizeof(buf), pRAR);
	gens_pclose(pRAR);
	
	// Check if the header matches 7-Zip's header.
	const char* strRARHeader = RAR_NEWLINE "RAR";
	if (strncmp(buf, strRARHeader, strlen(strRARHeader)) != 0)
	{
		// Incorrect header. External RAR executable is broken.
		// Assume that this means the file wasn't found.
		// TODO: More comprehensive error handling.
		errOpeningRAR(ENOENT);
		return false;
	}
	
	// Correct header. External RAR executable is working.
	return true;
}


/**
 * getNumFiles(): Gets the number of files in the specified archive.
 * @param filename Filename of the archive.
 * @return Number of files, or 0 on error. (-1 if 7-Zip couldn't be opened.)
 */
int RAR::getNumFiles(const string& zFilename)
{
	FILE *pRAR;
	char buf[1025];
	int rv;
	stringstream ss;
	string data;
	int numFiles = 0;
	
	// Build the command line.
	stringstream ssCmd;
	ssCmd << "\"" << Misc_Filenames.RAR_Binary << "\" v \"" << zFilename << "\"";
	
	// Open the RAR file.
	pRAR = gens_popen(ssCmd.str().c_str(), "r");
	if (!pRAR)
	{
		errOpeningRAR(errno);
		return -1;
	}
	
	// Read from the pipe.
	while ((rv = fread(buf, 1, 1024, pRAR)))
	{
		buf[rv] = 0x00;
		ss << buf;
	}
	gens_pclose(pRAR);
	
	// Get the string and go through it to get the number of files.
	data = ss.str();
	ss.clear();
	
	// Find the ---, which indicates the start of the file listing.
	unsigned int listStart = data.find("---");
	if (listStart == string::npos)
	{
		// Not found. Either there are no files, or the archive is broken.
		return 0;
	}
	
	// Find the newline after the list start.
	unsigned int listStartLF = data.find(RAR_NEWLINE, listStart);
	if (listStart == string::npos)
	{
		// Not found. Either there are no files, or the archive is broken.
		return 0;
	}
	
	// Parse all lines until we hit another "---" (or EOF).
	unsigned int curStartPos = listStartLF + RAR_NEWLINE_LENGTH;
	unsigned int curEndPos;
	string curLine;
	bool endOfRAR = false;
	while (!endOfRAR)
	{
		curEndPos = data.find(RAR_NEWLINE, curStartPos);
		if (curEndPos == string::npos)
		{
			// End of list.
			break;
		}
		
		// Get the second line, which contains the filesize and filetype.
		curStartPos = curEndPos + RAR_NEWLINE_LENGTH;
		curEndPos = data.find(RAR_NEWLINE, curStartPos);
		if (curEndPos == string::npos)
		{
			// End of list.
			break;
		}
		
		// Get the current line.
		curLine = data.substr(curStartPos, curEndPos - curStartPos);
		
		// Check if this is a normal file.
		if (curLine.length() < 62)
			break;
		
		if (curLine.at(52) == '-' || curLine.at(54) == '.')
		{
			// Normal file.
			numFiles++;
		}
		
		// Go to the next file in the listing.
		curStartPos = curEndPos + RAR_NEWLINE_LENGTH;
	}
	
	// Return the number of files found.
	return numFiles;
}


/**
 * getFileInfo(): Get information about all files in the specified archive.
 * @param zFilename Filename of the archive.
 * @return Pointer to list of CompressedFile structs, or NULL on error.
 */
list<CompressedFile>* RAR::getFileInfo(const string& zFilename)
{
	list<CompressedFile> *lst;
	CompressedFile file;
	
	FILE *pRAR;
	char buf[1025];
	int rv;
	stringstream ss;
	string data;
	int numFiles = 0;
	
	// Build the command line.
	stringstream ssCmd;
	ssCmd << "\"" << Misc_Filenames.RAR_Binary << "\" v \"" << zFilename << "\"";
	
	// Open the RAR file.
	pRAR = gens_popen(ssCmd.str().c_str(), "r");
	if (!pRAR)
	{
		errOpeningRAR(errno);
		return 0;
	}
	
	// Read from the pipe.
	while ((rv = fread(buf, 1, 1024, pRAR)))
	{
		buf[rv] = 0x00;
		ss << buf;
	}
	gens_pclose(pRAR);
	
	// Get the string and go through it to get the number of files.
	data = ss.str();
	ss.clear();
	
	// Find the ---, which indicates the start of the file listing.
	unsigned int listStart = data.find("---");
	if (listStart == string::npos)
	{
		// Not found. Either there are no files, or the archive is broken.
		return NULL;
	}
	
	// Find the newline after the list start.
	unsigned int listStartLF = data.find(RAR_NEWLINE, listStart);
	if (listStart == string::npos)
	{
		// Not found. Either there are no files, or the archive is broken.
		return NULL;
	}
	
	// Create the list.
	lst = new list<CompressedFile>;
	
	// Parse all lines until we hit another "---" (or EOF).
	unsigned int curStartPos = listStartLF + RAR_NEWLINE_LENGTH;
	unsigned int curEndPos;
	string curLine;
	bool endOfRAR = false;
	while (!endOfRAR)
	{
		curEndPos = data.find(RAR_NEWLINE, curStartPos);
		if (curEndPos == string::npos)
		{
			// End of list.
			break;
		}
		
		// Get the current line.
		curLine = data.substr(curStartPos, curEndPos - curStartPos);
		
		// First line in a RAR file listing is the filename. (starting at the second character)
		if (curLine.length() < 2)
			break;
		if (curLine.at(0) == '-')
		{
			// End of file listing.
			break;
		}
		file.filename = curLine.substr(1);
		
		// Get the second line, which contains the filesize and filetype.
		curStartPos = curEndPos + RAR_NEWLINE_LENGTH;
		curEndPos = data.find(RAR_NEWLINE, curStartPos);
		if (curEndPos == string::npos)
		{
			// End of list.
			break;
		}
		
		// Get the current line.
		curLine = data.substr(curStartPos, curEndPos - curStartPos);
		
		// Check if this is a normal file.
		if (curLine.length() < 62)
			break;
		
		if (curLine.at(52) == '-' || curLine.at(54) == '.')
		{
			// Normal file.
			file.filesize = atoi(curLine.substr(12, 10).c_str());
			lst->push_back(file);
			numFiles++;
		}
		
		// Go to the next file in the listing.
		curStartPos = curEndPos + RAR_NEWLINE_LENGTH;
	}
	
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
int RAR::getFile(const string& zFilename, const CompressedFile *fileInfo,
		 unsigned char *buf, const int size)
{
	FILE *pRAR;
	char bufRAR[1024];
	int rv;
	stringstream ss;
	string data;
	int totalSize = 0;
	
	// Build the command line.
	stringstream ssCmd;
	ssCmd << "\"" << Misc_Filenames.RAR_Binary << "\" p -ierr \"" << zFilename
	      << "\" \"" << fileInfo->filename << "\"";
#ifndef GENS_OS_WIN32
	ssCmd << " 2>/dev/null";
#endif
	
	pRAR = gens_popen(ssCmd.str().c_str(), "r");
	if (!pRAR)
	{
		errOpeningRAR(errno);
		return -1;
	}
	
	// Read from the pipe.
	while ((rv = fread(bufRAR, 1, 1024, pRAR)))
	{
		if (totalSize + rv > size)
			break;
		memcpy(&buf[totalSize], &bufRAR, rv);
		totalSize += rv;
	}
	gens_pclose(pRAR);
	
	// Return the filesize.
	return totalSize;
}
