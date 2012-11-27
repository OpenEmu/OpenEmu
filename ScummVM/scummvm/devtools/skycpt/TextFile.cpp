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

TextFile::TextFile(const char *name) {
	FILE *inf = fopen(name, "r");
	_lines = NULL;
	if (!inf) {
		printf("Unable to open file %s\n", name);
		getchar();
	} else {
		read(inf);
		fclose(inf);
	}
}

uint32 crop(char *line) {
	char *start = line;
	while (*line)
		if ((*line == 0xA) || (*line == 0xD))
			*line = '\0';
		else if ((line[0] == '/') && (line[1] == '/'))
			*line = '\0';
		else
			line++;

	while (((*(line - 1) == ' ') || ((*(line - 1)) == '\t')) && (line > start)) {
		line--;
		*line = '\0';
	}
	return (uint32)(line - start);
}

char *TextFile::giveLine(uint32 num) {
	if (num >= _numLines)
		return NULL;
	else
		return _lines[num];
}

void TextFile::read(FILE *inf) {
	char *line = (char *)malloc(4096);
	_lines = (char**)malloc(4096 * sizeof(char *));
	_numLines = 0;
	uint32 linesMax = 4096;
	while (fgets(line, 4096, inf)) {
		if (_numLines >= linesMax) {
			_lines = (char**)realloc(_lines, linesMax * 2 * sizeof(char *));
			linesMax *= 2;
		}
		char *start = line;
		while ((*start == '\t') || (*start == ' '))
			start++;
		if (memcmp(start, "(int16)", 7) == 0) {
			start += 7;
			while ((*start == '\t') || (*start == ' '))
				start++;
		}
		uint32 length = crop(start);
		_lines[_numLines] = (char *)malloc(length + 1);
		memcpy(_lines[_numLines], start, length + 1);
		_numLines++;
	}
	free(line);
}

int32 TextFile::findLine(KmpSearch *kmp, uint32 fromLine) {
	for (uint32 cnt = fromLine; cnt < _numLines; cnt++) {
		if (kmp->search(_lines[cnt]))
			return cnt;
	}
	return -1;
}
