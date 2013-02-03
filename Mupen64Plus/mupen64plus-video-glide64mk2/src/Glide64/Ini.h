/*
*   Glide64 - Glide video plugin for Nintendo 64 emulators.
*   Copyright (c) 2002  Dave2001
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public
*   Licence along with this program; if not, write to the Free
*   Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
*   Boston, MA  02110-1301, USA
*/

//****************************************************************
//
// Glide64 - Glide Plugin for Nintendo 64 emulators (tested mostly with Project64)
// Project started on December 29th, 2001
//
// To modify Glide64:
// * Write your name and (optional)email, commented by your work, so I know who did it, and so that you can find which parts you modified when it comes time to send it to me.
// * Do NOT send me the whole project or file that you modified.  Take out your modified code sections, and tell me where to put them.  If people sent the whole thing, I would have many different versions, but no idea how to combine them all.
//
// Official Glide64 development channel: #Glide64 on EFnet
//
// Original author: Dave2001 (Dave2999@hotmail.com)
// Other authors: Gonetz, Gugaman
//
//****************************************************************

//nmn: Fix for MAX_PATH
#include <limits.h>
#include <stdio.h>
#include "winlnxdefs.h"

BOOL INI_Open ();
void INI_Close ();
void INI_InsertSpace(int space);
BOOL INI_FindSection (const char *sectionname, BOOL create=TRUE);
const char *INI_ReadString (const char *itemname, const char *value, const char *def_value, BOOL create=TRUE);
void INI_WriteString (const char *itemname, const char *value);
int INI_ReadInt (const char *itemname, int def_value, BOOL create=TRUE);
void INI_WriteInt (const char *itemname, int value);
void SetConfigDir( const char *configDir );

class Ini
{
private:
	Ini();
	static Ini *singleton;
public:
	static Ini *OpenIni();
	void SetPath(const char *path);
	bool Read(const char *key, int *l);
	bool Read(const char *key, int *l, int defaultVal);
	int Read(const char *key, int defaultVal);
};
