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

#ifndef CRUISE_MAINDRAW_H
#define CRUISE_MAINDRAW_H

namespace Cruise {

extern int currentTransparent;
extern int16 *polyBuffer2;
extern int16 *XMIN_XMAX;
extern int m_color;

int upscaleValue(int value, int scale);

void pixel(int x, int y, char color);
void mainDraw(int16 param);
void flipScreen();
void buildPolyModel(int X, int Y, int scale, char *ptr2, char *destBuffer, char *dataPtr);
void drawSprite(int width, int height, cellStruct *currentObjPtr, const uint8 *dataIn, int ys, int xs, uint8 *output, const uint8 *dataBuf);
void flipPoly(int fileId, int16 *dataPtr, int scale, char** newFrame, int X, int Y, int *outX, int *outY, int *outScale);
void getPolySize(int positionX, int positionY, int scale, int sizeTable[4], unsigned char *dataPtr);
bool findPoly(char* dataPtr, int x, int y, int zoom, int mouseX, int mouseY);
unsigned char *drawPolyMode2(unsigned char *dataPointer, int linesToDraw);
void calcRGB(uint8* pColorSrc, uint8* pColorDst, int* offsetTable);
void drawMessage(const gfxEntryStruct *pGfxPtr, int globalX, int globalY, int width, int newColor, uint8 *ouputPtr);

} // End of namespace Cruise

#endif
