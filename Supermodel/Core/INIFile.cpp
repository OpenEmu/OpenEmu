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
 * INIFile.cpp
 *
 * INI file management. Implementation of the CINIFile class.
 *
 * To-Do List
 * ----------
 * - Add an iterator to retrieve all settings associated with a given section.
 *   This will allow detection of invalid setting names, if the caller desires.
 * - Add boolean on/off, true/false keywords.
 * - Note that linePtr does not necessarily correspond to actual lines in the
 *   file (newlines should be counted by the tokenizer for that).
 *
 * Grammar
 * -------
 *
 * Section:		'[' Identifier ']'
 * Line:		Identifier '=' Argument
 * Argument:	Number
 *				String
 *
 * Overview
 * --------
 * INI files are linear in nature, divided into sections. Each section has
 * settings associated with it which may be either numerical values or strings.
 * Presently, only unsigned 32-bit integers are supported. 
 *
 * INI files are first opened, then parsed, and finally closed. The parse tree
 * is cleared only during object construction and when the INI file is closed.
 * Default settings may be inserted before a file is opened or parsed. When an
 * INI file is written out, the current INI file on the disk is cleared. If an
 * error occurs during this process, the data will be lost.
 *
 * Sections are added only when a setting is found in that section. Empty
 * sections are not added to the tree with the exception of the default ("")
 * section. A default section name can be specified, which creates an alias
 * for the default section. The alias will be output when the file is
 * written (there will be no settings without an explicit section).
 */
 
#include "Supermodel.h"
using namespace std;


/******************************************************************************
 Basic Functions
******************************************************************************/

bool CINIFile::Write(const char *comment)
{
	bool	writeSuccess;
	
	// In order to truncate, we must close and reopen as truncated
	File.close();
	File.clear();	// required to clear EOF flag (open() does not clear)
	File.open(FileName.c_str(),fstream::out|fstream::trunc);
	if (File.fail())
	{
		return FAIL;
	}
	
	// Output comment
	if (comment != NULL)
		File << comment << endl;
		
	// Iterate through all sections sequentially
	for (unsigned i = 0; i < Sections.size(); i++)
	{
		if (Sections[i].Settings.size() != 0)
		{
			// Output section name
			if (Sections[i].Name != "")
				File << "[ " << Sections[i].Name << " ]" << endl << endl;
			else	// if null name, use default section name (if exists) or nothing at all
			{
				if (DefaultSectionName != "")
					File << "[ " << DefaultSectionName << " ]" << endl << endl;
			}
		
			// Iterate through all settings within this section
			for (unsigned j = 0; j < Sections[i].Settings.size(); j++)
			{
				// Output setting
				File << Sections[i].Settings[j].Name << " = ";
				if (Sections[i].Settings[j].isNumber)
					File << Sections[i].Settings[j].value << endl;
				else
					File << '\"' << Sections[i].Settings[j].String << '\"' << endl;
			}
		
			// New line
			File << endl;
		}
	}
	
	writeSuccess = File.good()?OKAY:FAIL;
		
	// Close and reopen as read/write
	File.close();
	File.open(FileName.c_str(),fstream::in|fstream::out);
	if (File.fail())
	{
		//printf("unable to re-open %s for reading/writing\n", FileName.c_str());
		return FAIL;
	}
	
	// Report any errors that occurred during writing
	return writeSuccess;
}

bool CINIFile::Open(const char *fileNameStr)
{
	FileName = fileNameStr;

	// Try to open for reading AND writing
	File.open(fileNameStr, fstream::in|fstream::out);
	if (File.fail())
		return FAIL;
		
	InitParseTree();
	return OKAY;
}

bool CINIFile::OpenAndCreate(const char *fileNameStr)
{
	FileName = fileNameStr;

	// Try to open for reading and writing
	File.open(fileNameStr, fstream::in|fstream::out);
	if (File.fail())
	{
		// File does not exist, try opening as write only (create it)
		File.clear();
		File.open(fileNameStr, fstream::out);
		if (File.fail())
			return FAIL;
	}
	
	InitParseTree();
	return OKAY;
}

void CINIFile::Close(void)
{
	File.close();
	Sections.clear();	// clear the parse tree!
}


/******************************************************************************
 Management of Settings
******************************************************************************/

// Finds index of first matching section in the section list. Returns FAIL if not found.
bool CINIFile::LookUpSection(unsigned *idx, string SectionName)
{
	for (unsigned i = 0; i < Sections.size(); i++)
	{
		if ((Sections[i].Name == SectionName) ||
			((Sections[i].Name == "") && (SectionName == DefaultSectionName)) ||	// if default section, also accept its alias
			((Sections[i].Name == DefaultSectionName) && (SectionName == "")))		// ...
		{
			*idx = i;
			return OKAY;
		}
	}
	return FAIL;
}

// Assigns a value to the given setting, creating the setting if it does not exist. Nulls out the string (sets it to "").
void CINIFile::Set(string SectionName, string SettingName, int value)
{
	struct Setting	NewSetting;
	unsigned		sectionIdx, settingIdx;
	
	// Check if the section exists anywhere in parse tree. If not, create it
	if (OKAY != LookUpSection(&sectionIdx, SectionName))
	{
		//printf("unable to find %s:%s, creating section\n", SectionName.c_str(), SettingName.c_str());
		struct Section	NewSection;
		
		NewSection.Name = SectionName;
		Sections.push_back(NewSection);
		sectionIdx = Sections.size()-1;	// the new section will be at the last index
	}
	
	// Search through all sections with the requested name for the first occurance of the desired setting
	for (unsigned i = 0; i < Sections.size(); i++)
	{
		if ((Sections[i].Name == SectionName) ||
			((Sections[i].Name == "") && (SectionName == DefaultSectionName)) ||	// accept alias for default section
			((Sections[i].Name == DefaultSectionName) && (SectionName == "")))		// ...
		{
			for (unsigned j = 0; j < Sections[i].Settings.size(); j++)
			{
				if (Sections[i].Settings[j].Name == SettingName)
				{
					// Found it! Update value of this setting
					sectionIdx = i;
					settingIdx = j;
					goto UpdateIntValue;
				}
			}
		}
	}
	
	// Couldn't find setting, create it in the first matching section found earlier	
	NewSetting.Name = SettingName;
	Sections[sectionIdx].Settings.push_back(NewSetting);
	settingIdx = Sections[sectionIdx].Settings.size()-1;

	// Update the setting!	
UpdateIntValue:
	Sections[sectionIdx].Settings[settingIdx].isNumber = true;
	Sections[sectionIdx].Settings[settingIdx].value = value;	
	Sections[sectionIdx].Settings[settingIdx].String = "";
}

// Assigns the string to the given setting, creating the setting if it does not exist. Zeros out the value.
void CINIFile::Set(string SectionName, string SettingName, string String)
{
	struct Setting	NewSetting;
	unsigned		sectionIdx, settingIdx;
	
	// Check if the section exists anywhere in parse tree. If not, create it
	if (OKAY != LookUpSection(&sectionIdx, SectionName))
	{
		//printf("unable to find %s:%s, creating section\n", SectionName.c_str(), SettingName.c_str());
		struct Section	NewSection;
		
		NewSection.Name = SectionName;
		Sections.push_back(NewSection);
		sectionIdx = Sections.size()-1;	// the new section will be at the last index
	}
	
	// Search through all sections with the requested name for the first occurance of the desired setting
	for (unsigned i = 0; i < Sections.size(); i++)
	{
		if ((Sections[i].Name == SectionName) ||
			((Sections[i].Name == "") && (SectionName == DefaultSectionName)) ||	// accept alias for default section
			((Sections[i].Name == DefaultSectionName) && (SectionName == "")))		// ...
		{
			for (unsigned j = 0; j < Sections[i].Settings.size(); j++)
			{
				if (Sections[i].Settings[j].Name == SettingName)
				{
					// Found it! Update value of this setting
					sectionIdx = i;
					settingIdx = j;
					goto UpdateString;
				}
			}
		}
	}
	
	// Couldn't find setting, create it in the first matching section found earlier
	NewSetting.Name = SettingName;
	Sections[sectionIdx].Settings.push_back(NewSetting);
	settingIdx = Sections[sectionIdx].Settings.size()-1;

	// Update the setting!	
UpdateString:
	Sections[sectionIdx].Settings[settingIdx].isNumber = false;
	Sections[sectionIdx].Settings[settingIdx].String = String;
	Sections[sectionIdx].Settings[settingIdx].value = 0;
}

// Obtains a numerical setting, if it exists, otherwise does nothing.
bool CINIFile::Get(string SectionName, string SettingName, int& value)
{
	for (unsigned i = 0; i < Sections.size(); i++)
	{
		if ((Sections[i].Name == SectionName) ||
			((Sections[i].Name == "") && (SectionName == DefaultSectionName)) ||	// accept alias for default section
			((Sections[i].Name == DefaultSectionName) && (SectionName == "")))		// ...
		{
			for (unsigned j = 0; j < Sections[i].Settings.size(); j++)
			{
				if (Sections[i].Settings[j].Name == SettingName)
				{
					value = Sections[i].Settings[j].value;
					return OKAY;
				}
			}
		}
	}

	return FAIL;
}

bool CINIFile::Get(string SectionName, string SettingName, unsigned& value)
{
	int intVal;
	if (Get(SectionName, SettingName, intVal) == FAIL || intVal < 0)
		return FAIL;

	value = (unsigned)intVal;

	return OKAY;
}

// Obtains a string setting, if it exists, otherwise does nothing.
bool CINIFile::Get(string SectionName, string SettingName, string& String)
{
	for (unsigned i = 0; i < Sections.size(); i++)
	{
		if ((Sections[i].Name == SectionName) ||
			((Sections[i].Name == "") && (SectionName == DefaultSectionName)) ||	// accept alias for default section
			((Sections[i].Name == DefaultSectionName) && (SectionName == "")))		// ...
		{
			for (unsigned j = 0; j < Sections[i].Settings.size(); j++)
			{
				if (Sections[i].Settings[j].Name == SettingName)
				{
					String = Sections[i].Settings[j].String;
					return OKAY;
				}
			}
		}
	}

	return FAIL;
}

void CINIFile::SetDefaultSectionName(string SectionName)
{
	DefaultSectionName = SectionName;
}


/******************************************************************************
 Tokenizer
 
 Never need to check for newlines because it is assumed these have been
 stripped by getline().
******************************************************************************/

// Token types
#define TOKEN_INVALID		-1
#define TOKEN_NULL			0
#define TOKEN_IDENTIFIER	1
#define TOKEN_NUMBER		2
#define TOKEN_STRING		3


// Token constructor (initializes token to null)
CINIFile::CToken::CToken(void)
{
	type = TOKEN_NULL;
}

// Returns true for white space, comment symbol, or null terminator.
static bool IsBlank(char c)
{
	if (isspace(c) || (c==';') || (c=='\0'))
		return true;
	return false;
}

// Fetches a string. Tolerates all characters between quotes, except \n.
CINIFile::CToken CINIFile::GetString(void)
{
	CToken	T;
	
	T.type = TOKEN_STRING;
	
	// Search for next quote
	++linePtr;
	while (1)
	{
		if (linePtr[0] == '\"')
		{
			++linePtr;	// so we can find next token
			break;
		}
		else if ((linePtr[0] == '\0') || (linePtr[0] == '\n'))
		{
			//printf("tokenizer: warning: string is missing end quote\n");
			break;
		}
		else
			T.String += linePtr[0];
		
		++linePtr;
	}
	
	return T;
}			

// Fetch number (decimal or hexadecimal positive/negative integer).
// linePtr must point to a character and therefore linePtr[1] is guaranteed to be within bounds.
CINIFile::CToken CINIFile::GetNumber(void)
{
	CToken	T;
	unsigned long long	number = 0;
	bool                isNeg = false;
	int					overflow = 0;
	
	T.type = TOKEN_NUMBER;
	
	// See if begins with minus sign 
	if (linePtr[0]=='-')
	{
		isNeg = true;
		linePtr++;
	}

	// Hexadecimal?
	if ((linePtr[0]=='0') && ((linePtr[1]=='X') || (linePtr[1]=='x')))
	{
		linePtr += 2;	// advance to digits
		
		// Ensure that at we have at least one digit
		if (!isxdigit(linePtr[0]))
		{
			//printf("tokenizer: invalid hexadecimal number\n");
			T.type = TOKEN_INVALID;
			return T;
		}
			
		// Read number digit by digit
		while (1)
		{
			if (isxdigit(linePtr[0]))
			{
				number <<= 4;
				if (isdigit(linePtr[0]))
					number |= (linePtr[0]-'0');
				else if (isupper(linePtr[0]))
					number |= (linePtr[0]-'A');
				else	// must be lowercase...
					number |= (linePtr[0]-'a');
				++linePtr;
				
				// Check for overflows
				if (!isNeg && number > 0x000000007FFFFFFFULL || isNeg && number > 0x0000000080000000ULL)
					overflow = 1;
			}
			else if (IsBlank(linePtr[0]))
				break;
			else
			{
				//printf("tokenizer: invalid hexadecimal number\n");
				T.type = TOKEN_INVALID;
				return T;
			}
		}
	}
	
	// Decimal?
	else
	{
		// Read number digit by digit
		while (1)
		{
			if (isdigit(linePtr[0]))
			{
				number *= 10;
				number += (linePtr[0]-'0');
				++linePtr;
				
				// Check for overflows
				if (!isNeg && number > 0x000000007FFFFFFFULL || isNeg && number > 0x0000000080000000ULL)
					overflow = 1;
			}
			else if (IsBlank(linePtr[0]))
				break;
			else
			{
				//printf("tokenizer: invalid number\n");
				T.type = TOKEN_INVALID;
				return T;
			}
		}
	}
	
	//if (overflow)
	//	printf("tokenizer: number exceeds 32 bits and has been truncated\n");
	
	T.number = (isNeg ? -(int)number : (int)number);
	return T;
}

// Fetch identifier
CINIFile::CToken CINIFile::GetIdentifier(void)
{
	CToken	T;
	
	T.type = TOKEN_IDENTIFIER;
	while (1)
	{
		if (isalpha(linePtr[0]) || isdigit(linePtr[0]) || (linePtr[0]=='_'))
		{
			T.String += linePtr[0];
			++linePtr;
		}
		else
			break;
	}
	
	return T;
}

// Fetch token
CINIFile::CToken CINIFile::GetToken(void)
{
	CToken	T;
	
	while (1)
	{
		// Gobble up whitespace
		if (isspace(linePtr[0]))
			++linePtr;
		
		// Comment or end of line
		else if ((linePtr[0]==';') || (linePtr[0]=='\0'))
		{
			T.type = TOKEN_NULL;
			return T;	// do not advance linePtr (so we do not de-sync the parser)
		}
			
		// Delimiters
		else if ((linePtr[0]=='[') || (linePtr[0]==']') || (linePtr[0]=='='))
		{
			T.type = *linePtr++;
			return T;
		}
		
		// Identifier?
		else if (isalpha(linePtr[0]) || (linePtr[0] == '_'))
		{
			T = GetIdentifier();
			return T;
		}
		
		// Number? (+/-?)
		else if (linePtr[0]=='-' || isdigit(linePtr[0]))
		{
			T = GetNumber();
			return T;
		}
		
		// String?
		else if (linePtr[0]=='\"')
		{
			T = GetString();
			return T;
		}
		
		// Illegal symbol
		else
			break;	// return null token
	}
	
	// If we got here, invalid token
	T.type = TOKEN_INVALID;
	return T;
}


/******************************************************************************
 Parser
******************************************************************************/

/*
 * Parse tree is initialized with a blank section in case the user adds 
 * settings with defined sections before adding settings without an explicit
 * section. If this is not done, settings without a section will be wrongly
 * output as part of the previous section in the parse tree.
 */
void CINIFile::InitParseTree(void)
{
	struct Section	FirstSection;
		
	FirstSection.Name = "";
	Sections.clear();
	Sections.push_back(FirstSection);
}

bool CINIFile::Parse(void)
{
	CToken	T, U, V, W;
	string	currentSection;	// current section we're processing
	bool	parseStatus = OKAY;
	
	lineNum = 0;
	
	if (!File.is_open())
		return FAIL;
	File.clear();
	if (!File.good())
		return FAIL;
	
	while (!File.eof())
	{
		++lineNum;
		File.getline(lineBuf,2048);
		if (File.fail())
			return FAIL;
		linePtr = lineBuf;	// beginning of line
		
		// Top level
		T = GetToken();
		U = GetToken();
		V = GetToken();
		W = GetToken();	// should always be null
		switch (T.type)
		{
		// [ Identifier ]
		case '[':
			if ((U.type==TOKEN_IDENTIFIER) && (V.type==']') && (W.type==TOKEN_NULL))
			{
				//printf("Section: %s\n", U.String.c_str());
				currentSection = U.String;
			}
			else
			{
				parseStatus = FAIL;
				//printf("%d: parse error\n", lineNum);
			}
			break;
		
		// Identifier '=' Argument
		case TOKEN_IDENTIFIER:
			if (U.type != '=')
			{
				parseStatus = FAIL;
				//printf("%d: expected '=' after identifier\n", lineNum);
			}
			else
			{
				if (((V.type==TOKEN_NUMBER) || (V.type==TOKEN_STRING)) && (W.type==TOKEN_NULL))
				{
					if (V.type == TOKEN_NUMBER)
					{
						//printf("\t%s = %X\n", T.String.c_str(), V.number);
						Set(currentSection, T.String, V.number);
					}
					else if (V.type == TOKEN_STRING)
					{
						//printf("\t%s = %s\n", T.String.c_str(), V.String.c_str());
						Set(currentSection, T.String, V.String);
					}
				}
				else
				{
					parseStatus = FAIL;
					//printf("%d: expected a number or string after '='\n", lineNum);
				}
			}
			break;
		
		// Blank line
		case TOKEN_NULL:
			break;
		
		// Illegal
		default:
			parseStatus = FAIL;
			//printf("%d: parse error\n", lineNum);
			break;
		}
	}	
	
	//printf("end of file reached\n");
	return parseStatus;
}
