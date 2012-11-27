/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "stdafx.h"
#include "cpthelp.h"

void assertEmpty(CptObj *cpt) {
	assert((cpt->len == 0) && (cpt->data == NULL) && (cpt->dbgName == NULL) && (cpt->type == 0));
}

uint16 getInfo(const char *line, const char *type, char *nameDest) {
	assert(*line == '\t');
	line++;
	assert(!memcmp(line, type, strlen(type)));
	line += strlen(type);
	assert((line[0] == ':') && (line[1] == ':'));
	line += 2;
	char *stopCh;
	uint16 res = (uint16)strtoul(line, &stopCh, 16);
	assert(line != stopCh);
	assert((stopCh[0] == ':') && (stopCh[1] == ':'));
	stopCh += 2;
	strcpy(nameDest, stopCh);
	assert(strlen(nameDest) < 32);
	assert(res);
	return res;
}

void dofgets(char *dest, int len, FILE *inf) {
	fgets(dest, len, inf);
	while (*dest)
		if ((*dest == 0xA) || (*dest == 0xD))
			*dest = '\0';
		else
			dest++;
}

bool lineMatchSection(const char *line, const char *sec) {
	if (memcmp(line, "SECTION::", 9))
		return false;
	if (memcmp(line + 9, sec, strlen(sec)))
		return false;
	return true;
}

bool isEndOfSection(const char *line) {
	if (strcmp(line, "SECTION::ENDS"))
		return false;
	return true;
}

bool isEndOfObject(const char *line, const char *type, uint16 id) {
	if (*line != '\t')
		return false;
	line++;
	if (memcmp(line, type, strlen(type)))
		return false;
	line += strlen(type);
	if ((line[0] != ':') || (line[1] != ':'))
		return false;
	line += 2;
	char *stopCh;
	uint16 idVal = (uint16)strtoul(line, &stopCh, 16);
	assert(stopCh != line);
	if (strcmp(stopCh, "::ENDS"))
		return false;
	assert(id == idVal);
	return true;
}
