/* Extended string routines
 *
 * Copyright notice for this file:
 *  Copyright (C) 2004 Jason Oster (Parasyte)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/// \file
/// \brief various string manipulation utilities

#include "xstring.h"

//#ifdef __cplusplus
//extern "C" {
//#endif

//int tolower(int);

//#ifdef __cplusplus
//}
//#endif

//this code was taken from WINE (LGPL)
//http://google.com/codesearch?hl=en&q=+lang:c+splitpath+show:CPvw9Z-euls:_RSotQzmLeU:KGzljMEYFbY&sa=N&cd=9&ct=rc&cs_p=http://gentoo.osuosl.org/distfiles/Wine-20050524.tar.gz&cs_f=wine-20050524/programs/winefile/splitpath.c
void splitpath(const char* path, char* drv, char* dir, char* name, char* ext)
{
	const char* end; /* end of processed string */
	const char* p;	 /* search pointer */
	const char* s;	 /* copy pointer */

	/* extract drive name */
	if (path[0] && path[1]==':') {
		if (drv) {
			*drv++ = *path++;
			*drv++ = *path++;
			*drv = '\0';
		} else path+=2;
	} else if (drv)
		*drv = '\0';

	/* search for end of string or stream separator */
	for(end=path; *end && *end!=':'; )
		end++;

	/* search for begin of file extension */
	for(p=end; p>path && *--p!='\\' && *p!='/'; )
		if (*p == '.') {
			end = p;
			break;
		}

	if (ext)
		for(s=end; (*ext=*s++); )
			ext++;
	else
		for(s=end; *s++; ) {}

	/* search for end of directory name */
	for(p=end; p>path; )
		if (*--p=='\\' || *p=='/') {
			p++;
			break;
		}

	if (name) {
		for(s=p; s<end; )
			*name++ = *s++;

		*name = '\0';
	} else
		for(s=p; s<end; )
			*s++;

	if (dir) {
		for(s=path; s<p; )
			*dir++ = *s++;

		*dir = '\0';
	}
}

// replace all instances of victim with replacement
std::string mass_replace(const std::string &source, const std::string &victim, const std::string &replacement)
{
	std::string answer = source;
	std::string::size_type j = 0;
	while ((j = answer.find(victim, j)) != std::string::npos )
	answer.replace(j, victim.length(), replacement);
	return answer;
}

//TODO - dont we already have another  function that can do this
std::string getExtension(const char* input) {
	char buf[1024];
	strcpy(buf,input);
	char* dot=strrchr(buf,'.');
	if(!dot)
		return "";
	char ext [512];
	strcpy(ext, dot+1);
	int k, extlen=strlen(ext);
	for(k=0;k<extlen;k++)
		ext[k]=tolower(ext[k]);
	return ext;
}
