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
#include "TextFile.h"
#include "KmpSearch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint16 findCptId(char *name, TextFile *cptFile) {
	KmpSearch *kmp = new KmpSearch();
	kmp->init(name);
	int cLine = 0;
	do {
		cLine = cptFile->findLine(kmp, cLine);
		if (cLine >= 0) {
			char *line = cptFile->giveLine(cLine);
			if ((strncmp(line, "COMPACT::", 9) == 0) ||
				(strncmp(line, "SCRATCH::", 9) == 0) ||
				(strncmp(line, "GET_TOS::", 9) == 0)) {
				char *stopCh;
				uint16 resId = (uint16)strtoul(line + 9, &stopCh, 16);
				if ((stopCh[0] == ':') && (stopCh[1] == ':') && (strcmp(stopCh + 2, name) == 0)) {
					delete kmp;
					return resId;
				}
			}
			cLine++;
		}
	} while (cLine != -1);
	delete kmp;
	return 0;
}
