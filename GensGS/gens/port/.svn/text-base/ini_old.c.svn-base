/***************************************************************************
 * Gens: Old INI file handler functions. [BROKEN]                          *
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

// TODO: Eliminate all uses of the following functions.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "ini_old.h"

static void WriteKey(const char *var, const char *var_name, FILE *file)
{
	fwrite(var, strlen(var), 1, file);
	fwrite("=", 1, 1, file);
	fwrite(var_name, strlen(var_name), 1, file);
	fwrite("\n", 1, 1, file);
}

static void InsertSectionKey(const char *section, const char *var,
			     const char *var_name, FILE *file, const int saut)
{
	if (saut)
		fwrite("\n", 1, 1, file);
	fwrite("[", 1, 1, file);
	fwrite(section, strlen(section), 1, file);
	fwrite("]\n", 2, 1, file);
	WriteKey(var, var_name, file);
}

static char* SearchKey(const char *fromWhere, const char *key)
{
	char *found = NULL;
	char *ext_key;
	
	ext_key = (char*)malloc(1 + strlen(key) + 1 + 1);	// \n key \0
	ext_key[0] = '\n';
	ext_key[1] = '\0';
	strcat(ext_key, key);
	found = strstr (fromWhere, ext_key);
	if (found)
		return found + 1;
	else
		return NULL;
}

static char *SearchSection(const char *buf, const char *section)
{
	char *found = NULL;
	char *ext_section;
	
	ext_section = (char*)malloc(1 + strlen(section) + 1 + 1);	// [ section ] \0 
	ext_section[0] = '[';
	ext_section[1] = '\0';
	strcat(ext_section, section);
	strcat(ext_section, "]");
	found = strstr(buf, ext_section);
	if (found)
		return found + 1;
	else
		return NULL;
}


static char* file2buf(const char *filename)
{
	char *buf;
	int filesize;
	struct stat sb;
	FILE *file;
	int res;
	
	res = stat(filename, &sb);
	if (-1 == res)
		return NULL;
	filesize = sb.st_size - 1;
	if (filesize >= 0)
	{
		buf = (char*)calloc(filesize + 1, 1);
		file = fopen(filename, "r");
		fread(buf, filesize, 1, file);
		fclose(file);
		buf[filesize] = '\0';
	}
	else
	{
		buf = (char*)calloc(1, 1);
	}
	return buf;
}


int GetPrivateProfileInt(const char *section, const char *var,
			 const int def, const char *filename)
{
	char get[10];
	char defstr[10];
	int res;
	
	memset(get, 0, 10);
	memset(defstr, 0, 10);
	
	if (!filename)
		return def;
	
	sprintf(defstr, "%d", def);
	
	GetPrivateProfileString(section, var, defstr, get, 10, filename);
	sscanf(get, "%d", &res);
	
	return res;
}

void GetPrivateProfileString(const char *section, const char *var,
			     const char *def, char *get, const int length,
			     const char *filename)
{
	char *buf;
	char *section_found;
	char *key_found;
	
	memset(get, 0, length);
	
	if (!filename)
	{
		strncpy(get, def, length);
		return;
	}
	
	buf = file2buf(filename);
	if (buf)
	{
		section_found = SearchSection(buf, section);
		if (section_found)
		{
			key_found = SearchKey(section_found, var);
			if (key_found)
			{
				char *val_start;
				char *val_end;
				val_start = strchr (key_found, '=') + 1;
				val_end = val_start;
				while ((*val_end != '\0') && (*val_end != '\n') && (*val_end != '\r'))
					val_end++;
				strncpy (get, val_start, val_end - val_start);
				get[val_end - val_start] = '\0';
				return;
			}
		}
	}
	strncpy (get, def, length);
}


void WritePrivateProfileString(const char *section, const char *var,
			       const char *var_name, const char *filename)
{
	FILE *file;
	struct stat sb;
	int res;
	
	res = stat(filename, &sb);
	if (-1 == res)		// fichier n'existe pas
	{
		file = fopen(filename, "w");
		InsertSectionKey(section, var, var_name, file, 0);
		fclose(file);
	}
	else
	{
		int filesize;
		filesize = sb.st_size - 1;	// on ne lit pas EOT
		if (filesize >= 0)
		{
			char *buf;
			char *section_begin;
			int size1, size2;
			
			buf = file2buf(filename);
			
			section_begin = SearchSection(buf, section);
			if (NULL != section_begin)	//section existe déjà
			{
				char *key_begin;
				key_begin = SearchKey(section_begin, var);
				if (NULL != key_begin)	// la clé existe : on copie la partie d'avant l'ancienne clé, la nouvelle clé, puis la partie d'après l'ancienne clé
				{
					int old_key_size;
					char *key_end;
					
					size1 = key_begin - buf;
					key_end = strstr(key_begin, "\n");
					if (NULL != key_end)
					{
						old_key_size = key_end - key_begin + 1;
						size2 = filesize - (size1 + old_key_size);
						file = fopen(filename, "w");
						fwrite(buf, size1, 1, file);
						WriteKey(var, var_name, file);
						fwrite(buf + size1 + old_key_size, size2, 1, file);
						fclose(file);
					}
					else
					{		// la clé est la dernière du fichier
						file = fopen(filename, "w");
						fwrite(buf, size1, 1, file);
						WriteKey(var, var_name, file);
						fclose(file);
					}

				}
				else
				{		// clé n'existe pas : on la rajoute avant la prochaine section
					char *next_section_begin;
					next_section_begin = strstr (section_begin, "*");
					if (NULL == next_section_begin)	// la section recherchée est unique : il suffit de copier la clé à la fin du fichier
					{
						file = fopen(filename, "a");
						WriteKey(var, var_name, file);
						fclose(file);
					}
					else
					{		// on insère la clé avant la prochaine section
						size1 = next_section_begin - buf;
						size2 = filesize - size1;
						file = fopen(filename, "w");
						fwrite(buf, size1, 1, file);
						WriteKey(var, var_name, file);
						fwrite(buf + size1, size2, 1, file);
						fclose(file);
					}
				}
			}
			else
			{			//section n'existe pas : on la rajoute ainsi que la clé
				file = fopen(filename, "a");
				InsertSectionKey(section, var, var_name, file, 1);
				fclose(file);
			}
			free (buf);
		}
		else
		{			//taille fichier < 0
			file = fopen(filename, "w");
			InsertSectionKey(section, var, var_name, file, 0);
			fclose(file);
		}
	}
}
