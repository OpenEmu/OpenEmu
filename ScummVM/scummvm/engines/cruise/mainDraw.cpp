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

#include "cruise/cruise_main.h"
#include "cruise/polys.h"
#include "common/endian.h"
#include "common/util.h"

namespace Cruise {

struct autoCellStruct {
	struct autoCellStruct *next;
	short int ovlIdx;
	short int objIdx;
	short int type;
	short int newValue;
	cellStruct *pCell;
};

autoCellStruct autoCellHead;

void addAutoCell(int overlayIdx, int idx, int type, int newVal, cellStruct *pObject) {
	autoCellStruct *pNewEntry;

	pNewEntry = new autoCellStruct;

	pNewEntry->next = autoCellHead.next;
	autoCellHead.next = pNewEntry;

	pNewEntry->ovlIdx = overlayIdx;
	pNewEntry->objIdx = idx;
	pNewEntry->type = type;
	pNewEntry->newValue = newVal;
	pNewEntry->pCell = pObject;
}

void freeAutoCell() {
	autoCellStruct *pCurrent = autoCellHead.next;

	while (pCurrent) {
		autoCellStruct *next = pCurrent->next;

		if (pCurrent->type == 5) {
			objInit(pCurrent->ovlIdx, pCurrent->objIdx, pCurrent->newValue);
		} else {
			setObjectPosition(pCurrent->ovlIdx, pCurrent->objIdx, pCurrent->type, pCurrent->newValue);
		}

		if (pCurrent->pCell->animWait < 0) {
			objectParamsQuery params;

			getMultipleObjectParam(pCurrent->ovlIdx, pCurrent->objIdx, &params);

			pCurrent->pCell->animCounter = params.state2 - 1;
		}

		delete pCurrent;

		pCurrent = next;
	}
}

void calcRGB(uint8* pColorSrc, uint8* pColorDst, int* offsetTable) {
	for (unsigned long int i = 0; i < 3; i++) {
		int color = *(pColorSrc++);
		int offset = offsetTable[i];

		color += offset;
		if (color < 0)
			color = 0;
		if (color > 0xFF)
			color = 0xFF;

		*(pColorDst++) = (uint8)color;
	}
}

void fadeIn() {
	for (long int i = 256; i >= 0; i -= 32) {
		for (long int j = 0; j < 256; j++) {
			int offsetTable[3];
			offsetTable[0] = -i;
			offsetTable[1] = -i;
			offsetTable[2] = -i;
			calcRGB(&palScreen[masterScreen][3*j], &workpal[3*j], offsetTable);
		}
		gfxModuleData_setPal256(workpal);

		gfxModuleData_updatePalette();
		gfxModuleData_updateScreen();
	}

	for (long int j = 0; j < 256; j++) {
		int offsetTable[3];
		offsetTable[0] = 0;
		offsetTable[1] = 0;
		offsetTable[2] = 0;
		calcRGB(&palScreen[masterScreen][3*j], &workpal[3*j], offsetTable);
	}

	gfxModuleData_setPal256(workpal);

	fadeFlag = 0;
	PCFadeFlag = 0;
}

void flipScreen() {
	if (switchPal) {
		for (unsigned long int i = 0; i < 256*3; i++) {
			workpal[i] = palScreen[masterScreen][i];
		}
		switchPal = 0;
		gfxModuleData_setPal256(workpal);
	}

	SWAP(gfxModuleData.pPage00, gfxModuleData.pPage10);

	gfxModuleData_flipScreen();

	if (doFade) {
		fadeIn();
		doFade = 0;
	}
}

int spriteX1;
int spriteX2;
int spriteY1;
int spriteY2;

char *polyOutputBuffer;

void pixel(int x, int y, char color) {
	if (x >= 0 && x < 320 && y >= 0 && y < 200)
		polyOutputBuffer[320 * y + x] = color;
}

// this function checks if the dataPtr is not 0, else it retrives the data for X, Y, scale and DataPtr again (OLD: mainDrawSub1Sub1)
void flipPoly(int fileId, int16 *dataPtr, int scale, char** newFrame, int X, int Y, int *outX, int *outY, int *outScale) {
	if (*dataPtr == 0) {
		int16 offset;
		int16 newX;
		int16 newY;

		dataPtr ++;

		offset = (int16)READ_BE_UINT16(dataPtr);
		dataPtr++;

		newX = (int16)READ_BE_UINT16(dataPtr);
		dataPtr++;

		newY = (int16)READ_BE_UINT16(dataPtr);
		dataPtr++;

		offset += fileId;

		if (offset >= 0) {
			if (filesDatabase[offset].resType == OBJ_TYPE_LINE && filesDatabase[offset].subData.ptr) {
				dataPtr = (int16 *)filesDatabase[offset].subData.ptr;
			}
		}

		scale = -scale;
		X -= newX;
		Y -= newY;
	}

	*newFrame = (char *)dataPtr;
	*outX = X;
	*outY = Y;
	*outScale = scale;
}

int upscaleValue(int value, int scale) {
	return (((value * scale) << 8) / 2);
}

int m_flipLeftRight;
int m_useSmallScale;
int m_lowerX;
int m_lowerY;
int m_coordCount;
int m_first_X;
int m_first_Y;
int m_scaleValue;
int m_color;

/*
   FIXME: Whether intentional or not, the game often seems to use negative indexing
   of one or more of the arrays below and expects(?) to end up in the preceding one.
   This "worked" on many platforms so far, but on OSX apparently the buffers don't
   occupy contiguous memory, and this causes severe corruption and subsequent crashes.
   Since I'm not really familiar with how the strange drawing code is supposed to work,
   or whether this behavior is intentional or not, the short-term fix is to allocate a big
   buffer and setup pointers within it.  This fixes the crashes I'm seeing without causing any
   (visual) side-effects.
   If anyone wants to look, this is easily reproduced by starting the game and examining the rug.
   drawPolyMode1() will then (indirectly) negatively index polyBuffer4.   Good luck!
*/

//int16 DIST_3D[512];
//int16 polyBuffer2[512];
//int16 XMIN_XMAX[404];
//int16 polyBuffer4[512];

int16 bigPolyBuf[512 + 512 + 404 + 512];	/* consolidates the 4 separate buffers above */

//set up the replacement index pointers.
int16 *DIST_3D = &bigPolyBuf[0];
int16 *polyBuffer2 = &bigPolyBuf[512];
int16 *XMIN_XMAX = &bigPolyBuf[512 + 512];
int16 *polyBuffer4 = &bigPolyBuf[512 + 512 + 404];




// this function fills the sizeTable for the poly (OLD: mainDrawSub1Sub2)
void getPolySize(int positionX, int positionY, int scale, int sizeTable[4], unsigned char *dataPtr) {
	int upperBorder;
	int lowerBorder;
	m_flipLeftRight = 0;

	if (scale < 0) {		// flip left right
		m_flipLeftRight = 1;
		scale = -scale;
	}
	// X1

	upperBorder = *(dataPtr + 3);

	if (m_flipLeftRight) {
		upperBorder = -upperBorder;
	}

	upperBorder = (upscaleValue(upperBorder, scale) + 0x8000) >> 16;
	upperBorder = -upperBorder;
	lowerBorder = upperBorder;

	// X2

	upperBorder = *(dataPtr + 1);
	upperBorder -= *(dataPtr + 3);

	if (m_flipLeftRight) {
		upperBorder = -upperBorder;
	}

	upperBorder = (upscaleValue(upperBorder, scale) + 0x8000) >> 16;

	if (upperBorder < lowerBorder) {	// exchange borders if lower > upper
		SWAP(upperBorder, lowerBorder);
	}

	sizeTable[0] = lowerBorder + positionX;	// left
	sizeTable[1] = upperBorder + positionX;	// right

	// Y1

	upperBorder = *(dataPtr + 4);
	upperBorder = (upscaleValue(upperBorder, scale) + 0x8000) >> 16;
	upperBorder = -upperBorder;
	lowerBorder = upperBorder;

	// Y2

	upperBorder = *(dataPtr + 2);
	upperBorder -= *(dataPtr + 4);
	upperBorder = (upscaleValue(upperBorder, scale) + 0x8000) >> 16;

	if (upperBorder < lowerBorder) {	// exchange borders if lower > upper
		SWAP(upperBorder, lowerBorder);
	}

	sizeTable[2] = lowerBorder + positionY;	// bottom
	sizeTable[3] = upperBorder + positionY;	// top
}

int nbseg;
int16 nbligne;

void blitPolyMode1(char *dest, char *pMask, int16 * buffer, char color) {
	int Y = XMIN_XMAX[0];

	for (int i = 0; i < nbligne; i++) {
		int currentY = Y + i;
		int XMIN = XMIN_XMAX[1+i*2];
		int XMAX = XMIN_XMAX[1+i*2+1];

		for (int x = XMIN; x <= XMAX; x++) {
			if (testMask(x, currentY, (unsigned char*)pMask, 40)) {
				*(dest + currentY * 320 + x) = color;
			}
		}
		//line(XMIN, currentY, XMAX, currentY, color);
	}
}

void blitPolyMode2(char *dest, int16 * buffer, char color) {
	int Y = XMIN_XMAX[0];

	for (int i = 0; i < nbligne; i++) {
		int currentY = Y + i;
		int XMIN = XMIN_XMAX[1+i*2];
		int XMAX = XMIN_XMAX[1+i*2+1];

		for (int x = XMIN; x <= XMAX; x++) {
			*(dest + currentY * 320 + x) = color;
		}
	}
}

int polyXMin;
int polyXMax;
int polyYMax;
int polyYMin;

int16 *A2ptr;



void buildSegment() {
	int16* pOut = XMIN_XMAX;

	if ((polyXMin >= 320) || (polyXMax < 0) || (polyYMax < 0) || (polyYMin >= 200)) {
		XMIN_XMAX[0] = -1;
		nbligne = -1;
		return;
	}

	if (polyYMin == polyYMax) { // line
		*(pOut++) = polyYMin; // store initial Y

		int cx = nbseg - 1;
		int16* pIn = A2ptr;

		int XLeft;
		int XRight;

		XLeft = XRight = *pIn; // init to first X
		pIn += 2;

		do {
			int X = *pIn;
			if (XLeft > X)
				XLeft = X;
			if (XRight < X)
				XRight = X;
			pIn += 2;
		} while (--cx);

		// now store left and right coordinates in XMIN_XMAX

		int XMin = XLeft;
		int XMax = XRight;

		if (XLeft < 0)
			XMin = 0;

		if (XRight >= 320)
			XMax = 319;

		*(pOut++) = XMin;
		*(pOut++) = XMax;
		*(pOut++) = -1;

		nbligne = 1;
		return;
	}

	// true polygon

	int ydep;

	if (polyYMin < 0)
		ydep = 0;
	else
		ydep = polyYMin;

	int yfin;

	if (polyYMax > 199)
		yfin = 199;
	else
		yfin = polyYMax;

	nbligne = yfin - ydep + 1;

	int16* ptrMini = XMIN_XMAX + 1;
	XMIN_XMAX[0] = ydep;

	int16* ptrMax = XMIN_XMAX + ((yfin - ydep) * 2) + 1;
	ptrMax[2] = -1; // mark the end

	// init table with default values
	int16* si = XMIN_XMAX + 1;
	int tempCount = nbligne;
	do {
		si[0] = 5000;
		si[1] = -5000;
		si += 2;
	} while (--tempCount);

	int16* di = A2ptr;
	int segCount = nbseg;

	do {
		int X2 = di[2];
		int X1 = di[0];
		int Y2 = di[3];
		int Y1 = di[1];


		int tempAX = Y1;
		int tempDX = Y2;
		if (tempAX > tempDX) {
			// swap
			tempAX = Y2;
			tempDX = Y1;
		}

		// is segment on screen ?
		if (!((tempAX > 199) || (tempDX < 0))) {
			int cx = X2 - X1;
			if (cx == 0) {
				// vertical line
				int CX = X2;
				if (CX < 0)
					CX = 0;

				int DX = X2;
				if (DX > 319)
					DX = 319;

				int16* BX = XMIN_XMAX + (Y2 - ydep) * 2 + 1;
				int16* DI = XMIN_XMAX + (Y1 - ydep) * 2 + 1;

				if (Y2 >= Y1) {
					SWAP(BX, DI);
				}

				do {
					if ((BX <= ptrMax) && (BX >= ptrMini)) { // are we in screen ?
						if (CX < BX[0])
							BX[0] = CX;

						if (DX > BX[1])
							BX[1] = DX;
					}

					BX += 2;
				} while (BX <= DI);
			} else {
				if (cx < 0) {
					cx = -cx;

					SWAP(X1, X2);
					SWAP(Y1, Y2);
				}
				// swap again ?
				SWAP(X1, X2);

				int patchAdd = 2;

				int dy = Y2 - Y1;

				if (dy == 0) {
					// hline
					int16* ptr = (Y1 - ydep) * 2 + XMIN_XMAX + 1;

					if ((ptr <= ptrMax) && (ptr >= ptrMini)) { // are we in screen ?
						int CX = X1;
						if (CX < 0)
							CX = 0;

						int SI = X2;
						if (SI > 319)
							SI = 319;

						if (CX < ptr[0])
							ptr[0] = CX;

						if (SI > ptr[1])
							ptr[1] = SI;
					}
				} else {
					if (dy < 0) {
						dy = -dy;
						patchAdd = -2;
					}

					int stepType = 0; // small DY <= DX

					if (dy > cx) {
						stepType = 1; // DX < DY

						SWAP(dy, cx);
					}
					int patchinc1 = 2 * dy;

					int d = 2 * dy - cx;
					int bx = 2 * (dy - cx);

					int patchinc2 = bx;

					cx++; // cx is the number of pixels to trace

					int16* ptr = (Y1 - ydep) * 2 + XMIN_XMAX + 1;

					if (stepType == 0) {
						// small step
						int BP = X2;

						int SI = BP;
						if (SI < 0)
							SI = 0;
						int DX = BP;
						if (DX > 319)
							DX = 319;

						do {
							if ((ptr <= ptrMax) && (ptr >= ptrMini)) { // are we in screen ?
								if (SI < ptr[0])
									ptr[0] = SI;

								if (DX > ptr[1])
									ptr[1] = DX;
							}

							BP ++;

							// test limits
							SI = BP;
							if (SI < 0)
								SI = 0;
							DX = BP;
							if (DX > 319)
								DX = 319;

							if (d < 0) {
								d += patchinc1;
								if (cx == 1) {   // last ?
									if ((ptr <= ptrMax) && (ptr >= ptrMini)) { // are we in screen ?
										if (SI < ptr[0])
											ptr[0] = SI;

										if (DX > ptr[1])
											ptr[1] = DX;
									}
								}
							} else {
								d += patchinc2;
								ptr += patchAdd;
							}
						} while (--cx);
					} else {
						// big step
						int BP = X2;

						int SI = BP;
						if (SI < 0)
							SI = 0;
						int DX = BP;
						if (DX > 319)
							DX = 319;

						do {
							if ((ptr <= ptrMax) && (ptr >= ptrMini)) { // are we in screen ?
								if (SI < ptr[0])
									ptr[0] = SI;

								if (DX > ptr[1])
									ptr[1] = DX;
							}

							ptr += patchAdd; // next line

							if (d < 0) {
								d += patchinc1;
							} else {
								d += patchinc2;
								BP ++;

								// test limits
								SI = BP;
								if (SI < 0)
									SI = 0;
								DX = BP;
								if (DX > 319)
									DX = 319;
							}
						} while (--cx);
					}

				}
			}
		}

		di += 2;
	} while (--segCount);
}

unsigned char *drawPolyMode1(unsigned char *dataPointer, int linesToDraw) {
	int index;
	int16 *pBufferDest = polyBuffer4 + nbseg * 2;

	nbseg = linesToDraw;
	index = *(dataPointer++);

	polyXMin = polyXMax = pBufferDest[-2] = pBufferDest[-2 + linesToDraw * 2] = polyBuffer2[index * 2];
	polyYMin = polyYMax = pBufferDest[-1] = pBufferDest[-1 + linesToDraw * 2] = polyBuffer2[(index * 2) + 1];

	linesToDraw--;

	pBufferDest -= 2;

	A2ptr = pBufferDest;

	do {
		int value;

		index = *(dataPointer++);
		value = pBufferDest[-2] = pBufferDest[-2 + nbseg * 2] = polyBuffer2[index * 2];

		if (value < polyXMin) {
			polyXMin = value;
		}
		if (value > polyXMax) {
			polyXMax = value;
		}

		value = pBufferDest[-1] = pBufferDest[-1 + nbseg * 2] = polyBuffer2[(index * 2) + 1];

		if (value < polyYMin) {
			polyYMin = value;
		}
		if (value > polyYMax) {
			polyYMax = value;
			A2ptr = pBufferDest;
		}

		pBufferDest -= 2;

	} while (--linesToDraw);

	buildSegment();

	return dataPointer;
}

unsigned char *drawPolyMode2(unsigned char *dataPointer, int linesToDraw) {
	int index;
	int16 *pBufferDest;

	pBufferDest = polyBuffer4;
	nbseg = linesToDraw;
	A2ptr = polyBuffer4;
	index = *(dataPointer++);

	polyXMin = polyXMax = pBufferDest[0] = pBufferDest[linesToDraw * 2] = polyBuffer2[index * 2];
	polyYMin = polyYMax = pBufferDest[1] = pBufferDest[linesToDraw * 2 + 1] = polyBuffer2[(index * 2) + 1];

	linesToDraw--;

	pBufferDest += 2;

	do {
		int value;

		index = *(dataPointer++);
		value = pBufferDest[0] = pBufferDest[nbseg * 2] = polyBuffer2[index * 2];

		if (value < polyXMin) {
			polyXMin = value;
		}
		if (value > polyXMax) {
			polyXMax = value;
		}

		value = pBufferDest[1] = pBufferDest[nbseg * 2 + 1] = polyBuffer2[(index * 2) + 1];

		if (value < polyYMin) {
			polyYMin = value;
		}
		if (value > polyYMax) {
			polyYMax = value;
			A2ptr = pBufferDest;
		}

		pBufferDest += 2;

	} while (--linesToDraw);

	buildSegment();

	return dataPointer;
}

// this function builds the poly model and then calls the draw functions (OLD: mainDrawSub1Sub5)
void buildPolyModel(int positionX, int positionY, int scale, char *pMask, char *destBuffer, char *dataPtr) {
	int counter = 0;	// numbers of coordinates to process
	int startX = 0;		// first X in model
	int startY = 0;		// first Y in model
	int x = 0;		// current X
	int y = 0;		// current Y
	int offsetXinModel = 0;	// offset of the X value in the model
	int offsetYinModel = 0;	// offset of the Y value in the model
	unsigned char *dataPointer = (unsigned char *)dataPtr;
	int16 *ptrPoly_1_Buf = DIST_3D;
	int16 *ptrPoly_2_Buf;
	polyOutputBuffer = destBuffer;	// global

	m_flipLeftRight = 0;
	m_useSmallScale = 0;
	m_lowerX = *(dataPointer + 3);
	m_lowerY = *(dataPointer + 4);

	if (scale < 0) {
		scale = -scale;	// flip left right
		m_flipLeftRight = 1;
	}

	if (scale < 0x180) {	// If scale is smaller than 384
		m_useSmallScale = 1;
		m_scaleValue = scale << 1;	// double scale
	} else {
		m_scaleValue = scale;
	}

	dataPointer += 5;

	m_coordCount = (*(dataPointer++)) + 1;	// original uses +1 here but its later substracted again, we could skip it
	m_first_X = *(dataPointer);
	dataPointer++;
	m_first_Y = *(dataPointer);
	dataPointer++;
	startX = m_lowerX - m_first_X;
	startY = m_lowerY - m_first_Y;

	if (m_useSmallScale) {
		startX >>= 1;
		startY >>= 1;
	}

	if (m_flipLeftRight) {
		startX = -startX;
	}

	/*
	 * NOTE:
	 *
	 * The original code continues here with using X, Y instead of startX and StartY.
	 *
	 * Original code:
	 * positionX -= (upscaleValue(startX, m_scaleValue) + 0x8000) >> 16;
	 * positionY -= (upscaleValue(startX, m_scaleValue) + 0x8000) >> 16;
	 */

	// get coordinates from data

	startX = positionX - ((upscaleValue(startX, m_scaleValue) + 0x8000) >> 16);
	startY = positionY - ((upscaleValue(startY, m_scaleValue) + 0x8000) >> 16);

	ptrPoly_1_Buf[0] = 0;
	ptrPoly_1_Buf[1] = 0;
	ptrPoly_1_Buf += 2;
	counter = m_coordCount - 1 - 1;	// skip the first pair, we already have the values

	// dpbcl0
	do {
		x = *(dataPointer) - m_first_X;
		dataPointer++;
		if (m_useSmallScale) {	// shrink all coordinates by factor 2 if a scale smaller than 384 is used
			x >>= 1;
		}
		ptrPoly_1_Buf[0] = offsetXinModel - x;
		ptrPoly_1_Buf++;
		offsetXinModel = x;

		y = *(dataPointer) - m_first_Y;
		dataPointer++;
		if (m_useSmallScale) {
			y >>= 1;
		}
		ptrPoly_1_Buf[0] = -(offsetYinModel - y);
		ptrPoly_1_Buf++;
		offsetYinModel = y;

	} while (--counter);

	// scale and adjust coordinates with offset (using two polybuffers by doing that)
	ptrPoly_2_Buf = DIST_3D;
	ptrPoly_1_Buf = polyBuffer2;
	counter = m_coordCount - 1;	// reset counter // process first pair two
	int m_current_X = 0;
	int m_current_Y = 0;

	do {
		x = ptrPoly_2_Buf[0];

		if (m_flipLeftRight == 0) {
			x = -x;
		}
		//////////////////

		m_current_X += upscaleValue(x, m_scaleValue);
		ptrPoly_1_Buf[0] = ((m_current_X + 0x8000) >> 16) + startX;	// adjust X value with start offset

		m_current_Y += upscaleValue(ptrPoly_2_Buf[1], m_scaleValue);
		ptrPoly_1_Buf[1] = ((m_current_Y + 0x8000) >> 16) + startY;	// adjust Y value with start offset

		/////////////////

		ptrPoly_1_Buf += 2;
		ptrPoly_2_Buf += 2;

	} while (--counter);

	// position of the dataPointer is m_coordCount * 2

	int polygonCount = 0;

	do {
		int linesToDraw = *dataPointer++;

		if (linesToDraw > 1) {	// if value not zero
			uint16 minimumScale;

			m_color = *dataPointer;	// color
			dataPointer += 2;

			minimumScale = READ_BE_UINT16(dataPointer);
			dataPointer += 2;

			if ((minimumScale <= scale)) {
				if (m_flipLeftRight) {
					drawPolyMode1((unsigned char *)dataPointer, linesToDraw);
				} else {
					drawPolyMode2((unsigned char *)dataPointer, linesToDraw);
				}

				if (destBuffer) {
					if (pMask) {
						blitPolyMode1(destBuffer, pMask, polyBuffer4, m_color & 0xFF);
					} else {
						blitPolyMode2(destBuffer, polyBuffer4, m_color & 0xFF);
					}
				}
			}

			dataPointer += linesToDraw;
		} else {
			dataPointer += 4;
		}

		polygonCount ++;
	} while (*dataPointer != 0xFF);
}

bool findPoly(char* dataPtr, int positionX, int positionY, int scale, int mouseX, int mouseY) {
	int counter = 0;	// numbers of coordinates to process
	int startX = 0;		// first X in model
	int startY = 0;		// first Y in model
	int x = 0;		// current X
	int y = 0;		// current Y
	int offsetXinModel = 0;	// offset of the X value in the model
	int offsetYinModel = 0;	// offset of the Y value in the model
	unsigned char *dataPointer = (unsigned char *)dataPtr;
	int16 *ptrPoly_1_Buf = DIST_3D;
	int16 *ptrPoly_2_Buf;

	m_flipLeftRight = 0;
	m_useSmallScale = 0;
	m_lowerX = *(dataPointer + 3);
	m_lowerY = *(dataPointer + 4);

	if (scale < 0) {
		scale = -scale;	// flip left right
		m_flipLeftRight = 1;
	}

	if (scale < 0x180) {	// If scale is smaller than 384
		m_useSmallScale = 1;
		m_scaleValue = scale << 1;	// double scale
	} else {
		m_scaleValue = scale;
	}

	dataPointer += 5;

	m_coordCount = (*(dataPointer++)) + 1;	// original uses +1 here but its later substracted again, we could skip it
	m_first_X = *(dataPointer);
	dataPointer++;
	m_first_Y = *(dataPointer);
	dataPointer++;
	startX = m_lowerX - m_first_X;
	startY = m_lowerY - m_first_Y;

	if (m_useSmallScale) {
		startX >>= 1;
		startY >>= 1;
	}

	if (m_flipLeftRight) {
		startX = -startX;
	}

	/*
	 * NOTE:
	 *
	 * The original code continues here with using X, Y instead of startX and StartY.
	 *
	 * Original code:
	 * positionX -= (upscaleValue(startX, m_scaleValue) + 0x8000) >> 16;
	 * positionY -= (upscaleValue(startX, m_scaleValue) + 0x8000) >> 16;
	 */

	// get coordinates from data

	startX = positionX - ((upscaleValue(startX, m_scaleValue) + 0x8000) >> 16);
	startY = positionY - ((upscaleValue(startY, m_scaleValue) + 0x8000) >> 16);

	ptrPoly_1_Buf[0] = 0;
	ptrPoly_1_Buf[1] = 0;
	ptrPoly_1_Buf += 2;
	counter = m_coordCount - 1 - 1;	// skip the first pair, we already have the values

	// dpbcl0
	do {
		x = *(dataPointer) - m_first_X;
		dataPointer++;
		if (m_useSmallScale) {	// shrink all coordinates by factor 2 if a scale smaller than 384 is used
			x >>= 1;
		}
		ptrPoly_1_Buf[0] = offsetXinModel - x;
		ptrPoly_1_Buf++;
		offsetXinModel = x;

		y = *(dataPointer) - m_first_Y;
		dataPointer++;
		if (m_useSmallScale) {
			y >>= 1;
		}
		ptrPoly_1_Buf[0] = -(offsetYinModel - y);
		ptrPoly_1_Buf++;
		offsetYinModel = y;

	} while (--counter);

	// scale and adjust coordinates with offset (using two polybuffers by doing that)
	ptrPoly_2_Buf = DIST_3D;
	ptrPoly_1_Buf = polyBuffer2;
	counter = m_coordCount - 1;	// reset counter // process first pair two
	int m_current_X = 0;
	int m_current_Y = 0;

	do {
		x = ptrPoly_2_Buf[0];

		if (m_flipLeftRight == 0) {
			x = -x;
		}
		//////////////////

		m_current_X += upscaleValue(x, m_scaleValue);
		ptrPoly_1_Buf[0] = ((m_current_X + 0x8000) >> 16) + startX;	// adjust X value with start offset

		m_current_Y += upscaleValue(ptrPoly_2_Buf[1], m_scaleValue);
		ptrPoly_1_Buf[1] = ((m_current_Y + 0x8000) >> 16) + startY;	// adjust Y value with start offset

		/////////////////

		ptrPoly_1_Buf += 2;
		ptrPoly_2_Buf += 2;

	} while (--counter);

	// position of the dataPointer is m_coordCount * 2

	int polygonCount = 0;

	do {
		int linesToDraw = *dataPointer++;

		if (linesToDraw > 1) {	// if value not zero
			uint16 minimumScale;

			m_color = *dataPointer;	// color
			dataPointer += 2;

			minimumScale = READ_BE_UINT16(dataPointer);
			dataPointer += 2;

			if ((minimumScale <= scale)) {
				if (m_flipLeftRight) {
					drawPolyMode1((unsigned char *)dataPointer, linesToDraw);
				} else {
					drawPolyMode2((unsigned char *)dataPointer, linesToDraw);
				}

				int polygonYMin = XMIN_XMAX[0];
				int polygonYMax = polygonYMin + nbligne;

				if ((mouseY >= polygonYMin) && (mouseY < polygonYMax)) {
					int polygonLineNumber = mouseY - polygonYMin;

					int XMIN = XMIN_XMAX[1+polygonLineNumber*2];
					int XMAX = XMIN_XMAX[1+polygonLineNumber*2+1];

					if ((mouseX >= XMIN) && (mouseX <= XMAX))
						return true;
				}
			}

			dataPointer += linesToDraw;
		} else {
			dataPointer += 4;
		}

		polygonCount ++;
	} while (*dataPointer != 0xFF);

	return false;
}

void clearMaskBit(int x, int y, unsigned char* pData, int stride) {
	unsigned char* ptr = y * stride + x / 8 + pData;

	unsigned char bitToTest = 0x80 >> (x & 7);

	*(ptr) &= ~bitToTest;
}


void drawMask(unsigned char* workBuf, int wbWidth, int wbHeight, unsigned char* pMask, int maskWidth, int maskHeight, int maskX, int maskY, int passIdx) {
	for (int y = 0; y < maskHeight; y++) {
		for (int x = 0; x < maskWidth*8; x++) {
			if (testMask(x, y, pMask, maskWidth)) {
				int destX = maskX + x;
				int destY = maskY + y;

				if ((destX >= 0) && (destX < wbWidth*8) && (destY >= 0) && (destY < wbHeight))
					clearMaskBit(destX, destY, workBuf, wbWidth);
			}
		}
	}
}

unsigned char polygonMask[(320*200)/8];

// draw poly sprite (OLD: mainDrawSub1)
void mainDrawPolygons(int fileIndex, cellStruct *plWork, int X, int scale, int Y, char *destBuffer, char *dataPtr) {
	int newX;
	int newY;
	int newScale;
	char *newFrame;

	int sizeTable[4];	// 0 = left, 1 = right, 2 = bottom, 3 = top

	// this function checks if the dataPtr is not 0, else it retrives the data for X, Y, scale and DataPtr again (OLD: mainDrawSub1Sub1)
	flipPoly(fileIndex, (int16 *)dataPtr, scale, &newFrame, X, Y, &newX, &newY, &newScale);

	// this function fills the sizeTable for the poly (OLD: mainDrawSub1Sub2)
	getPolySize(newX, newY, newScale, sizeTable, (unsigned char*)newFrame);

	spriteX2 = sizeTable[0] - 2;	// left   border
	spriteX1 = sizeTable[1] + 18;	// right  border
	spriteY2 = sizeTable[2] - 2;	// bottom border
	spriteY1 = sizeTable[3] + 2;	// top    border

	if (spriteX2 >= 320)
		return;
	if (spriteX1 < 0)
		return;
	if (spriteY2 >= 200)
		return;
	if (spriteY1 < 0)
		return;

	if (spriteX2 < 0) {
		spriteX2 = 0;
	}
	if (spriteX1 > 320) {
		spriteX1 = 320;
	}
	if (spriteY2 < 0) {
		spriteY2 = 0;
	}
	if (spriteY1 > 200) {
		spriteY1 = 200;
	}

	if (spriteX1 == spriteX2)
		return;
	if (spriteY1 == spriteY2)
		return;

	gfxModuleData_addDirtyRect(Common::Rect(spriteX2, spriteY2, spriteX1, spriteY1));

	memset(polygonMask, 0xFF, (320*200) / 8);

	int numPasses = 0;

	while (plWork) {
		if (plWork->type == OBJ_TYPE_BGMASK && plWork->freeze == 0) {
			objectParamsQuery params;

			getMultipleObjectParam(plWork->overlay, plWork->idx, &params);

			int maskX = params.X;
			int maskY = params.Y;
			int maskFrame = params.fileIdx;

			if (filesDatabase[maskFrame].subData.resourceType == OBJ_TYPE_BGMASK && filesDatabase[maskFrame].subData.ptrMask) {
				drawMask(polygonMask, 40, 200, filesDatabase[maskFrame].subData.ptrMask, filesDatabase[maskFrame].width / 8, filesDatabase[maskFrame].height, maskX, maskY, numPasses++);
			} else
				if (filesDatabase[maskFrame].subData.resourceType == OBJ_TYPE_SPRITE && filesDatabase[maskFrame].subData.ptrMask) {
					drawMask(polygonMask, 40, 200, filesDatabase[maskFrame].subData.ptrMask, filesDatabase[maskFrame].width / 8, filesDatabase[maskFrame].height, maskX, maskY, numPasses++);
				}

		}

		plWork = plWork->next;
	}

	// this function builds the poly model and then calls the draw functions (OLD: mainDrawSub1Sub5)
	buildPolyModel(newX, newY, newScale, (char *)polygonMask, destBuffer, newFrame);
}

void drawMessage(const gfxEntryStruct *pGfxPtr, int globalX, int globalY, int width, int newColor, uint8 *ouputPtr) {
	// this is used for font only

	if (pGfxPtr) {
		uint8 *initialOuput;
		uint8 *output;
		int xp, yp;
		int x, y;
		const uint8 *ptr = pGfxPtr->imagePtr;
		int height = pGfxPtr->height;

		if (width > 310)
			width = 310;
		if (width + globalX > 319)
			globalX = 319 - width;
		if (globalY < 0)
			globalY = 0;
		if (globalX < 0)
			globalX = 0;

		if (globalY + pGfxPtr->height >= 198) {
			globalY = 198 - pGfxPtr->height;
		}

		gfxModuleData_addDirtyRect(Common::Rect(globalX, globalY, globalX + width, globalY + height));

		initialOuput = ouputPtr + (globalY * 320) + globalX;

		for (yp = 0; yp < height; yp++) {
			output = initialOuput + 320 * yp;
			y = globalY + yp;

			for (xp = 0; xp < pGfxPtr->width; xp++) {
				x = globalX + xp;
				uint8 color = *(ptr++);

				if (color) {
					if ((x >= 0) && (x < 320) && (y >= 0) && (y < 200)) {
						if (color == 1) {
							*output = (uint8) 0;
						} else {
							*output = (uint8) newColor;
						}
					}
				}
				output++;
			}
		}
	}
}

void drawSprite(int width, int height, cellStruct *currentObjPtr, const uint8 *dataIn, int ys, int xs, uint8 *output, const uint8 *dataBuf) {
	int x = 0;
	int y = 0;

	// Flag the given area as having been changed
	Common::Point ps = Common::Point(MAX(MIN(xs, 320), 0), MAX(MIN(ys, 200), 0));
	Common::Point pe = Common::Point(MAX(MIN(xs + width, 320), 0), MAX(MIN(ys + height, 200), 0));
	if ((ps.x != pe.x) && (ps.y != pe.y))
		// At least part of sprite is on-screen
		gfxModuleData_addDirtyRect(Common::Rect(ps.x, ps.y, pe.x, pe.y));

	cellStruct* plWork = currentObjPtr;
	int workBufferSize = height * (width / 8);

	unsigned char* workBuf = (unsigned char*)MemAlloc(workBufferSize);
	memcpy(workBuf, dataBuf, workBufferSize);

	int numPasses = 0;

	while (plWork) {
		if (plWork->type == OBJ_TYPE_BGMASK && plWork->freeze == 0) {
			objectParamsQuery params;

			getMultipleObjectParam(plWork->overlay, plWork->idx, &params);

			int maskX = params.X;
			int maskY = params.Y;
			int maskFrame = params.fileIdx;

			if (filesDatabase[maskFrame].subData.resourceType == OBJ_TYPE_BGMASK && filesDatabase[maskFrame].subData.ptrMask) {
				drawMask(workBuf, width / 8, height, filesDatabase[maskFrame].subData.ptrMask, filesDatabase[maskFrame].width / 8, filesDatabase[maskFrame].height, maskX - xs, maskY - ys, numPasses++);
			} else
				if (filesDatabase[maskFrame].subData.resourceType == OBJ_TYPE_SPRITE && filesDatabase[maskFrame].subData.ptrMask) {
					drawMask(workBuf, width / 8, height, filesDatabase[maskFrame].subData.ptrMask, filesDatabase[maskFrame].width / 8, filesDatabase[maskFrame].height, maskX - xs, maskY - ys, numPasses++);
				}

		}

		plWork = plWork->next;
	}

	for (y = 0; y < height; y++) {
		for (x = 0; x < (width); x++) {
			uint8 color = *dataIn++;

			if ((x + xs) >= 0 && (x + xs) < 320 && (y + ys) >= 0 && (y + ys) < 200) {
				if (testMask(x, y, workBuf, width / 8)) {
					output[320 * (y + ys) + x + xs] = color;
				}
			}
		}
	}

	MemFree(workBuf);
}

#ifdef _DEBUG
void drawCtp() {
	/*	int i;

		if (ctp_walkboxTable) {
			for (i = 0; i < 15; i++) {
				uint16 *dataPtr = &ctp_walkboxTable[i * 40];
				int type = walkboxColor[i];	// show different types in different colors

				if (*dataPtr) {
					int j;
					fillpoly((short *)dataPtr + 1, *dataPtr, type);

					for (j = 0; j < (*dataPtr - 1); j++) {
						line(dataPtr[1 + j * 2],
						    dataPtr[1 + j * 2 + 1],
						    dataPtr[1 + (j + 1) * 2],
						    dataPtr[1 + (j + 1) * 2 + 1], 0);
					}

					line(dataPtr[1 + j * 2],
					    dataPtr[1 + j * 2 + 1], dataPtr[1],
					    dataPtr[2], 0);
				}
			}
		}*/
}
#endif

void drawMenu(menuStruct *pMenu) {
	if (pMenu == NULL)
		return;

	if (pMenu->numElements == 0)
		return;

	int hline = pMenu->gfx->height;
	int x = pMenu->x;
	int y = pMenu->y + hline;

	int numItemByLine = (199 - hline * 2) / hline;
	int nbcol = pMenu->numElements / numItemByLine;

	if (!nbcol) {
		nbcol++;

		if (y + pMenu->numElements*hline > 199 - hline) {
			y = 200 - (pMenu->numElements * hline) - hline;
		}
	} else {
		if (pMenu->numElements % numItemByLine) {
			nbcol++;
		}

		y = hline;
	}

	if (x > (320 - (nbcol*160)))
		x = 320 - (nbcol * 160);

	if (x < 0)
		x = 0;

	int wx = x + (nbcol - 1) * (160 / 2);

	if (wx <= 320 - 160) {
		drawMessage(pMenu->gfx, wx, y - hline, 160, titleColor, gfxModuleData.pPage10);
	}

	wx = x;
	int wy = y;
	int wc = 0;
	menuElementStruct* p1 = pMenu->ptrNextElement;

	while (p1) {
		gfxEntryStruct *p2 = p1->gfx;

		p1->x = wx;
		p1->y = wy;
		p1->varA = 160;

		int color;

		if (p1->selected) {
			color = selectColor;
		} else {
			if (p1->color != 255) {
				color = p1->color;
			} else {
				color = itemColor;
			}
		}

		if (wx <= (320 - 160)) {
			drawMessage(p2, wx, wy, 160, color, gfxModuleData.pPage10);
		}

		wy += hline;
		wc ++;

		if (wc == numItemByLine) {
			wc = 0;
			wx += 160;
			wy = y;
		}

		p1 = p1->next;
	}
}

int getValueFromObjectQuerry(objectParamsQuery *params, int idx) {
	switch (idx) {
	case 0:
		return params->X;
	case 1:
		return params->Y;
	case 2:
		return params->baseFileIdx;
	case 3:
		return params->fileIdx;
	case 4:
		return params->scale;
	case 5:
		return params->state;
	case 6:
		return params->state2;
	case 7:
		return params->nbState;
	}

	assert(0);

	return 0;
}

void mainDraw(int16 param) {
	uint8 *bgPtr;
	cellStruct *currentObjPtr;
	int16 currentObjIdx;
	int16 objX1 = 0;
	int16 objY1 = 0;
	int16 objZ1 = 0;
	int16 objX2 = 0;
	int16 objY2 = 0;
	int16 objZ2 = 0;
	int16 spriteHeight;

	/*if (PCFadeFlag) {
		return;
	}*/

	bgPtr = backgroundScreens[masterScreen];

	if (bgPtr) {
		gfxModuleData_gfxCopyScreen(bgPtr, gfxModuleData.pPage10);
		if (backgroundChanged[masterScreen]) {
			backgroundChanged[masterScreen] = false;
			switchBackground(bgPtr);
		}
	}

	autoCellHead.next = NULL;

	currentObjPtr = cellHead.next;

#ifdef _DEBUG
	/*	polyOutputBuffer = (char *)bgPtr;
		drawCtp(); */
#endif

	//-------------------------------------------------- PROCESS SPRITES -----------------------------------------//

	while (currentObjPtr) {
		if ((masterScreen == currentObjPtr->backgroundPlane) && (currentObjPtr->freeze == 0) && (currentObjPtr->type == OBJ_TYPE_SPRITE)) {
			objectParamsQuery params;

			currentObjIdx = currentObjPtr->idx;

			if ((currentObjPtr->followObjectOverlayIdx != currentObjPtr->overlay) || (currentObjPtr->followObjectIdx != currentObjPtr->idx)) {
				// Declaring this twice ?
				// objectParamsQuery params;

				getMultipleObjectParam(currentObjPtr->followObjectOverlayIdx, currentObjPtr->followObjectIdx, &params);

				objX1 = params.X;
				objY1 = params.Y;
				objZ1 = params.fileIdx;
			} else {
				objX1 = 0;
				objY1 = 0;
				objZ1 = 0;
			}

			getMultipleObjectParam(currentObjPtr->overlay, currentObjIdx, &params);

			objX2 = objX1 + params.X;
			objY2 = objY1 + params.Y;
			objZ2 = params.fileIdx;

			if (objZ2 >= 0) {
				objZ2 += objZ1;
			}

			if ((params.state >= 0) && (objZ2 >= 0) && filesDatabase[objZ2].subData.ptr) {
				if (filesDatabase[objZ2].subData.resourceType == 8) {	// Poly
					mainDrawPolygons(objZ2, currentObjPtr, objX2, params.scale, objY2, (char *)gfxModuleData.pPage10, (char *)filesDatabase[objZ2].subData.ptr);	// poly
				} else if (filesDatabase[objZ2].subData.resourceType == OBJ_TYPE_SOUND) {
				} else if (filesDatabase[objZ2].resType == OBJ_TYPE_MASK) {
				} else if (filesDatabase[objZ2].subData.resourceType == OBJ_TYPE_SPRITE) {
					objX1 = filesDatabase[objZ2].width;	// width
					spriteHeight = filesDatabase[objZ2].height;	// height

					if (filesDatabase[objZ2].subData.ptr) {
						drawSprite(objX1, spriteHeight, currentObjPtr, filesDatabase[objZ2].subData.ptr, objY2, objX2, gfxModuleData.pPage10, filesDatabase[objZ2].subData.ptrMask);
					}
				}
			}

			// automatic animation process
			if (currentObjPtr->animStep && !param) {
				if (currentObjPtr->animCounter <= 0) {

					bool change = true;

					int newVal = getValueFromObjectQuerry(&params, currentObjPtr->animChange) + currentObjPtr->animStep;

					if (currentObjPtr->animStep > 0) {
						if (newVal > currentObjPtr->animEnd) {
							if (currentObjPtr->animLoop) {
								newVal = currentObjPtr->animStart;
								if (currentObjPtr->animLoop > 0)
									currentObjPtr->animLoop--;
							} else {
								change = false;
								currentObjPtr->animStep = 0;

								if (currentObjPtr->animType) {	// should we resume the script ?
									if (currentObjPtr->parentType == 20) {
										changeScriptParamInList(currentObjPtr->parentOverlay, currentObjPtr->parent, &procHead, -1, 0);
									} else if (currentObjPtr->parentType == 30) {
										changeScriptParamInList(currentObjPtr->parentOverlay, currentObjPtr->parent, &relHead, -1, 0);
									}
								}
							}
						}
					} else {
						if (newVal < currentObjPtr->animEnd) {
							if (currentObjPtr->animLoop) {
								newVal = currentObjPtr->animStart;
								if (currentObjPtr->animLoop > 0)
									currentObjPtr->animLoop--;
							} else {
								change = false;
								currentObjPtr->animStep = 0;

								if (currentObjPtr->animType) {	// should we resume the script ?
									if (currentObjPtr->parentType == 20) {
										changeScriptParamInList(currentObjPtr->parentOverlay, currentObjPtr->parent, &procHead, -1, 0);
									} else if (currentObjPtr->parentType == 30) {
										changeScriptParamInList(currentObjPtr->parentOverlay, currentObjPtr->parent, &relHead, -1, 0);
									}
								}
							}
						}
					}

					if (currentObjPtr->animWait >= 0) {
						currentObjPtr->animCounter = currentObjPtr->animWait;
					}

					if ((currentObjPtr->animSignal >= 0) && (currentObjPtr->animSignal == newVal) && (currentObjPtr->animType != 0)) {
						if (currentObjPtr->parentType == 20) {
							changeScriptParamInList(currentObjPtr->parentOverlay, currentObjPtr->parent, &procHead, -1, 0);
						} else if (currentObjPtr->parentType == 30) {
							changeScriptParamInList(currentObjPtr->parentOverlay, currentObjPtr->parent, &relHead, -1, 0);
						}

						currentObjPtr->animType = 0;
					}

					if (change) {
						addAutoCell(currentObjPtr->overlay, currentObjPtr->idx, currentObjPtr->animChange, newVal, currentObjPtr);
					}
				} else {
					currentObjPtr->animCounter--;
				}
			}
		}

		currentObjPtr = currentObjPtr->next;
	}

	//----------------------------------------------------------------------------------------------------------------//

	freeAutoCell();
	isMessage = 0;

	//-------------------------------------------------- DRAW OBJECTS TYPE 5 (MSG)-----------------------------------------//

	currentObjPtr = cellHead.next;

	while (currentObjPtr) {
		if (currentObjPtr->type == OBJ_TYPE_MESSAGE && currentObjPtr->freeze == 0) {
			drawMessage(currentObjPtr->gfxPtr, currentObjPtr->x, currentObjPtr->field_C, currentObjPtr->spriteIdx, currentObjPtr->color, gfxModuleData.pPage10);
			isMessage = 1;
		}
		currentObjPtr = currentObjPtr->next;
	}

	//----------------------------------------------------------------------------------------------------------------//

	if (currentActiveMenu != -1) {
		if (menuTable[currentActiveMenu]) {
			drawMenu(menuTable[currentActiveMenu]);
			return;
		}
	} else if ((linkedRelation) && (linkedMsgList)) {
		int16 mouseX;
		int16 mouseY;
		int16 button;
		getMouseStatus(&main10, &mouseX, &button, &mouseY);

		if (mouseY > (linkedMsgList->height)*2)
			drawMessage(linkedMsgList, 0, 0, 320, findHighColor(), gfxModuleData.pPage10);
		else
			drawMessage(linkedMsgList, 0, 200, 320, findHighColor(), gfxModuleData.pPage10);
	}
}

} // End of namespace Cruise
