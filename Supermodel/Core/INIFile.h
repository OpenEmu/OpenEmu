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
 * INIFile.h
 * 
 * Header file for INI file management.
 */

#ifndef INCLUDED_INIFILE_H
#define INCLUDED_INIFILE_H

// Standard C++ and STL headers
#include <fstream>
#include <string>
#include <vector>
using namespace std;

#include "Types.h"

/*
 * CINIFile:
 *
 * INI file parser.
 */
class CINIFile
{
public:
	/*
	 * Get(SectionName, SettingName, value):
	 * Get(SectionName, SettingName, String):
	 *
	 * Obtains the value of a setting associated with a particular section. It
	 * is assumed that the caller knows whether the setting should be a string
	 * or an integer value.  If the setting was specified as a string in the
	 * INI file, the value will be set to 0. Otherwise, the string will be set
	 * to "".
	 *
	 * Parameters:
	 *		SectionName		String defining the section name.
	 *		SettingName		String defining the setting name.
	 *		value			Reference to where the setting value will be
	 *						copied.
	 *		String			Reference to where the string will be copied.
	 *
	 * Returns:
	 *		OKAY if the setting was found, FAIL otherwise. The type is not 
	 *		checked. If the setting is not found, the output parameter will not
	 *		be modified.
	 */
	bool	Get(string SectionName, string SettingName, int& value);
	bool	Get(string SectionName, string SettingName, unsigned& value);
	bool	Get(string SectionName, string SettingName, string& String);
	
	/*
	 * Set(SectionName, SettingName, value):
	 * Set(SectionName, SettingName, String):
	 *
	 * Sets a setting associated with a particular section. For each overload
	 * type (string or integer), the opposite type is cleard (0 is written to
	 * values when a string is set, "" is written to strings when a value is
	 * set). If the setting does not exist, it is created. If the section does
	 * not exist, it will be added as well.
	 *
	 * Parameters:
	 *		SectionName		String defining the section name.
	 *		SettingName		String defining the setting name.
	 *		value			Value to write. String will be set to "".
	 *		String			String to write. Value will be set to 0.
	 */
	void 	Set(string SectionName, string SettingName, int value);
	void 	Set(string SectionName, string SettingName, string String);
	
	/*
	 * Write(comment):
	 *
	 * Outputs the parse tree to the INI file. The current contents of the file
	 * on the disk are discarded and overwritten. This means that comments are
	 * lost. The file is also put into write mode and cannot be re-parsed.
	 *
	 * Parameters:
	 *		comment		An optional C-string containing a comment to insert at
	 *					the beginning of the file, otherwise NULL. The comment
	 *					lines must include semicolons at the beginning, they
	 *					will not be inserted automatically. The string is
	 *					output directly, as-is.
	 *
	 * Returns:
	 *		OKAY if successful. FAIL if an error occurred at any point. In
	 * 		order to truncate the original contents of the file, this function
	 *		closes the file, reopends it as write-only, then reopens it again
	 *		as a read/write file. If an error occurs during the reopening 
	 *		procedure, it is possible that nothing will be output and the
	 *		previous contents will be lost.
	 */
	bool	Write(const char *comment);
	
	/*
	 * Parse(void):
	 *
	 * Parses the contents of the file, building the parse tree. Settings 
	 * already present in the parse tree will continue to exist and will be
	 * overwritten if new, matching settings are found in the file.
	 *
	 * This should be done once and at the beginning.
	 *
	 * Returns:
	 *		OKAY if successful, FAIL if there was a file or parse error.
	 */
	bool	Parse(void);
	
	/*
	 * SetDefaultSectionName(SectionName):
	 *
	 * Sets the default section name. Any settings not associated with a
	 * particular section (ie. those assigned to a blank section, "") will be
	 * output under this section when Write() is called. 
	 *
	 * This should be called before parsing and before adding any settings.
	 * If the same setting is added to the default section, "", and a section
	 * named "Global", and afterwards "Global" is set as the default name,
	 * the setting originally in the default section will be the one that is
	 * subsequently accessible (the Get()/Set() methods look for the first
	 * match). However, when the file is written out, the second setting
	 * will be written out as well using the same section name and, thus,
	 * when the INI file is re-parsed, its value will be used instead.
	 *
	 * Parameters:
	 *		SectionName		String defining the section name.
	 */
	void	SetDefaultSectionName(string SectionName);
	
	/*
	 * Open(fileNameStr):
	 *
	 * Opens an INI file. The file must already exist. A new one will not be
	 * created. Do not open another file before closing the current one!
	 *
	 * Parameters:
	 *		fileNameStr		File path (C string).
	 *
	 * Returns:
	 *		OKAY if successful, FAIL if unable to open for reading and writing.
	 */
	bool	Open(const char *fileNameStr);
	
	/*
	 * OpenAndCreate(fileNameStr):
	 *
	 * Opens an INI file and, if it does not exist, creates a new one. Do not
	 * open another file before closing the current one!
	 *
	 * Parameters:
	 *		fileNameStr		File path (C string).
	 *
	 * Returns:
	 *		OKAY if successful, FAIL if unable to open file.
	 */
	bool	OpenAndCreate(const char *fileNameStr);
	
	/*
	 * Close(void):
	 *
	 * Closes the INI file and clears the parse tree.
	 */
	void	Close(void);
	
private:
	// Token
	class CToken
	{
	public:
		int			type;	// token type (defined privately in INIFile.cpp)
		int         number;	// numbers and bools
		string		String;	// strings and identifiers
	
		// Constructor (initialize to null token)
		CToken(void);
	};

	// Parse tree 
	bool	LookUpSection(unsigned *idx, string SectionName);
	void	InitParseTree(void);
	
	// Tokenizer
	CToken	GetString(void);
	CToken	GetNumber(void);
	CToken	GetIdentifier(void);
	CToken	GetToken(void);
	
	// File state
	fstream		File;
	string		FileName;		// name of last file opened
	
	// Default section name (name to use for the blank section)
	string		DefaultSectionName;
	
	// Parser state
	char		lineBuf[2048];	// holds current line
	char		*linePtr;		// points to current position within line (for tokenization)
	unsigned	lineNum;		// line number
	
	// Parse tree: a list of sections each of which is a list of settings for that section
	struct Setting	// it is up to caller to determine whether to use value or string
	{
		string		Name;		// setting name
		bool		isNumber;	// internal flag: true if the setting is a number, false if it is a string
		int      	value;		// value of number
		string		String;		// string
		
		Setting(void)
		{
			value = 0;			// initialize value to 0
			isNumber = true;	// indicate the setting is initially a number
		}
	};
	struct Section
	{
		string					Name;		// section name
		vector<struct Setting>	Settings;	// list of settings associated w/ this section
	};
	vector<struct Section>		Sections;	// a list of sections
};


#endif	// INCLUDED_INIFILE_H
