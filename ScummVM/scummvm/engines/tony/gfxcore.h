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

/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#ifndef TONY_GFXCORE_H
#define TONY_GFXCORE_H

#include "common/system.h"
#include "common/coroutines.h"
#include "tony/utils.h"

namespace Tony {

/****************************************************************************\
*       Class prototype
\****************************************************************************/

//    Class Name                Family Treee            Abstract?
class RMGfxTask;             //                             Yes
class RMGfxTaskSetPrior;     //     Task                    Yes
class RMGfxBuffer;           //
class RMGfxSourceBuffer;     //     TaskP+[Buffer]          Yes
class RMGfxTargetBuffer;     //     [Buffer]
class RMGfxSourceBufferPal;  //     Source                  Yes
class RMGfxSourceBuffer4;    //     SourcePal
class RMGfxSourceBuffer8;    //     SourcePal
class RMGfxSourceBuffer16;   //     Source
class RMGfxWoodyBuffer;      //     Source16+Target
class RMGfxClearTask;        //     Task

/**
 * Graphics buffer
 */
class RMGfxBuffer {
protected:
	int _dimx, _dimy;
	byte *_buf;
	byte *_origBuf;

public:
	RMGfxBuffer();
	RMGfxBuffer(int dimx, int dimy, int nBpp);
	virtual ~RMGfxBuffer();

	// Attributes
	int getDimx();
	int getDimy();

	// Creation
	virtual void create(int dimx, int dimy, int nBpp);
	virtual void destroy();

	// These are valid only if the buffer is locked
	operator byte *();
	operator void *();

	// Getting the offset for a given Y position
	void offsetY(int nLines, int nBpp);
};

/**
 * Graphics primitive
 */
class RMGfxPrimitive {
public:
	RMGfxTask *_task;

protected:
	RMRect _src;
	RMRect _dst;

	bool _bStretch;
	byte _bFlag;

public:
	RMGfxPrimitive();
	RMGfxPrimitive(RMGfxTask *task);
	RMGfxPrimitive(RMGfxTask *task, const RMRect &src, RMRect &dst);
	RMGfxPrimitive(RMGfxTask *task, const RMPoint &src, RMRect &dst);
	RMGfxPrimitive(RMGfxTask *task, const RMPoint &src, RMPoint &dst);
	RMGfxPrimitive(RMGfxTask *task, const RMRect &src, RMPoint &dst);
	RMGfxPrimitive(RMGfxTask *task, const RMRect &dst);
	RMGfxPrimitive(RMGfxTask *task, const RMPoint &dst);
	virtual ~RMGfxPrimitive();
	void setFlag(byte bFlag);
	void setTask(RMGfxTask *task);
	void setSrc(const RMRect &src);
	void setSrc(const RMPoint &src);
	void setDst(const RMRect &dst);
	void setDst(const RMPoint &dst);
	void setStretch(bool bStretch);
	bool haveDst();
	RMRect &getDst();
	bool haveSrc();
	RMRect &getSrc();

	// Flags
	bool isFlipped();

	// Duplicate
	virtual RMGfxPrimitive *duplicate();
};

/**
 * Graphic drawing task
 */
class RMGfxTask {
protected:
	int _nPrior;
	int _nInList;

public:
	// Standard constructor
	RMGfxTask();
	virtual ~RMGfxTask() { }

	virtual int priority();
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) = 0;
	virtual void removeThis(CORO_PARAM, bool &result);

	// Registration
	virtual void Register();
	virtual void unregister();
};

/**
 * Graphic drawing with priority
 */
class RMGfxTaskSetPrior : public RMGfxTask {
public:
	virtual ~RMGfxTaskSetPrior() { }
	void setPriority(int nPrior);
};

/**
 * Task that cleans the destination buffer
 */
class RMGfxClearTask : public RMGfxTask {
public:
	virtual ~RMGfxClearTask() { }

	int priority();
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	virtual void removeThis(CORO_PARAM, bool &result);
};

/**
 * Task that draws a colored box
 */
class RMGfxBox : public RMGfxTaskSetPrior {
protected:
	uint16 _wFillColor;

public:
	virtual ~RMGfxBox() { }

	void setColor(byte r, byte g, byte b);
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	virtual void removeThis(CORO_PARAM, bool &result);
};

/**
 * Buffer source for the design, which is a task. This is an abstract base.
 */
class RMGfxSourceBuffer : public virtual RMGfxBuffer, public RMGfxTaskSetPrior {
public:
	// Load the data for the surface
	virtual int init(uint32 resID, int dimx, int dimy, bool bLoadPalette = false);
	virtual int init(const byte *buf, int dimx, int dimy, bool bLoadPalette = false);
	virtual void init(Common::ReadStream &ds, int dimx, int dimy, bool bLoadPalette = false);

	virtual ~RMGfxSourceBuffer();

protected:
	virtual void prepareImage();
	bool clip2D(int &x1, int &y1, int &u, int &v, int &width, int &height, bool bUseSrc, RMGfxTargetBuffer *buf);
	void offsetY(int nLines);

public:
	virtual int getBpp() = 0;
};

/**
 * 16-bit color source
 */
class RMGfxSourceBuffer16 : public RMGfxSourceBuffer {
protected:
	virtual void prepareImage();
	bool _bTrasp0;

public:
	RMGfxSourceBuffer16(bool bUseTrasp = false);
	RMGfxSourceBuffer16(int dimx, int dimy);
	virtual ~RMGfxSourceBuffer16();

	// Initialization
	void create(int dimx, int dimy);

	int getBpp();
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
};

/**
 * Buffer source with palette
 */
class RMGfxSourceBufferPal : public RMGfxSourceBuffer {
protected:
	// The size of the palette is  (1 << Bpp()) * 4
	byte _pal[256 * 3];
	uint16 _palFinal[256];

	// Post process to prepare the palette for drawing
	virtual void preparePalette();

public:
	virtual ~RMGfxSourceBufferPal();

	virtual int init(const byte *buf, int dimx, int dimy, bool bLoadPalette = false);
	virtual void init(Common::ReadStream &ds, int dimx, int dimy, bool bLoadPalette = false);

	int loadPaletteWA(uint32 resID, bool bSwapped = false);
	int loadPaletteWA(const byte *buf, bool bSwapped = false);
	int loadPalette(uint32 resID);
	int loadPalette(const byte *buf);
};

/**
 * Buffer source with a 256 color palette
 */
class RMGfxSourceBuffer8 : public RMGfxSourceBufferPal {
protected:
	bool _bTrasp0;

public:
	RMGfxSourceBuffer8(bool bTrasp0 = true);
	RMGfxSourceBuffer8(int dimx, int dimy);
	virtual ~RMGfxSourceBuffer8();

	// Initialization
	void create(int dimx, int dimy);

	int getBpp();
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
};

/**
 * Buffer source with a 256 color palette, and alpha blending
 */
class RMGfxSourceBuffer8AB : public RMGfxSourceBuffer8 {
protected:
	int calcTrasp(int f, int b);

public:
	virtual ~RMGfxSourceBuffer8AB();
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
};

/**
 * Buffer source with a 256 color palette, RLE compressed
 */

class RMGfxSourceBuffer8RLE : public virtual RMGfxSourceBuffer8 {
protected:
	int _alphaBlendColor;
	int _alphaR, _alphaB, _alphaG;
	bool _bNeedRLECompress;

protected:
	static byte _megaRLEBuf[];

	virtual void rleWriteTrasp(byte *&cur, int rep) = 0;
	virtual void rleWriteData(byte *&cur, int rep, byte *src) = 0;
	virtual void rleWriteEOL(byte *&cur) = 0;
	virtual void rleWriteAlphaBlend(byte *&cur, int rep) = 0;
	virtual void rleDecompressLine(uint16 *dst, byte *src, int nStartSkip, int nLength) = 0;
	virtual void rleDecompressLineFlipped(uint16 *dst, byte *src, int nStartSkip, int nLength) = 0;

	// Perform image compression in RLE
	void compressRLE();

protected:
	// Overriding initialization methods
	virtual void prepareImage();
	virtual void preparePalette();

public:
	RMGfxSourceBuffer8RLE();
	virtual ~RMGfxSourceBuffer8RLE();

	// Overload of the initialization method
	virtual void init(Common::ReadStream &ds, int dimx, int dimy, bool bLoadPalette = false);
	virtual int init(const byte *buf, int dimx, int dimy, bool bLoadPalette = false);

	// Draw image with RLE decompression
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Sets the color that will be alpha blended
	void setAlphaBlendColor(int color);

	// Warn if the data is already compressed
	void setAlreadyCompressed();
};

class RMGfxSourceBuffer8RLEByte : public RMGfxSourceBuffer8RLE {
protected:
	void rleWriteTrasp(byte *  &cur, int rep);
	void rleWriteAlphaBlend(byte *  &cur, int rep);
	void rleWriteData(byte *  &cur, int rep, byte *src);
	void rleWriteEOL(byte *  &cur);
	void rleDecompressLine(uint16 *dst, byte *src, int nStartSkip, int nLength);
	void rleDecompressLineFlipped(uint16 *dst, byte *src, int nStartSkip, int nLength);

public:
	virtual ~RMGfxSourceBuffer8RLEByte();
};

class RMGfxSourceBuffer8RLEWord : public RMGfxSourceBuffer8RLE {
protected:
	void rleWriteTrasp(byte *  &cur, int rep);
	void rleWriteAlphaBlend(byte *  &cur, int rep);
	void rleWriteData(byte *  &cur, int rep, byte *src);
	void rleWriteEOL(byte *  &cur);
	virtual void rleDecompressLine(uint16 *dst, byte *src, int nStartSkip, int nLength);
	virtual void rleDecompressLineFlipped(uint16 *dst, byte *src, int nStartSkip, int nLength);

public:
	virtual ~RMGfxSourceBuffer8RLEWord();
};

class RMGfxSourceBuffer8RLEWordAB : public RMGfxSourceBuffer8RLEWord {
protected:
	virtual void rleDecompressLine(uint16 *dst, byte *src, int nStartSkip, int nLength);

public:
	virtual ~RMGfxSourceBuffer8RLEWordAB();
};

/**
 * Buffer source with a 256 color palette, with anti-aliasing
 */
class RMGfxSourceBuffer8AA : public virtual RMGfxSourceBuffer8 {
protected:
	static byte _megaAABuf[];
	static byte _megaAABuf2[];
	byte *_aabuf;

	// Calculate the buffer for the anti-aliasing
	void calculateAA();

	// Draw the AA
	void drawAA(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

protected:
	void prepareImage();

public:
	RMGfxSourceBuffer8AA();
	virtual ~RMGfxSourceBuffer8AA();

	// Draw with anti-aliasing
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
};

class RMGfxSourceBuffer8RLEByteAA : public RMGfxSourceBuffer8RLEByte, public RMGfxSourceBuffer8AA {
protected:
	void prepareImage();

public:
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Overloaded initialization methods
	virtual void init(Common::ReadStream &ds, int dimx, int dimy, bool bLoadPalette = false);
	virtual int init(const byte *buf, int dimx, int dimy, bool bLoadPalette = false);

	virtual ~RMGfxSourceBuffer8RLEByteAA();
};

class RMGfxSourceBuffer8RLEWordAA : public RMGfxSourceBuffer8RLEWord, public RMGfxSourceBuffer8AA {
protected:
	void prepareImage();

public:
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Overloaded initialization methods
	virtual void init(Common::ReadStream &ds, int dimx, int dimy, bool bLoadPalette = false);
	virtual int init(byte *buf, int dimx, int dimy, bool bLoadPalette = false);

	virtual ~RMGfxSourceBuffer8RLEWordAA();
};

/**
 * Source buffer with 16 colors
 */
class RMGfxSourceBuffer4 : public RMGfxSourceBufferPal {
public:
	RMGfxSourceBuffer4();
	RMGfxSourceBuffer4(int dimx, int dimy);

	// Initialization
	void create(int dimx, int dimy);

	int getBpp();
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
};

/**
 * Destination buffer which manages its own internal list of tasks
 */
class RMGfxTargetBuffer : public virtual RMGfxBuffer {
private:
	struct OTList {
		RMGfxPrimitive *_prim;
		OTList *_next;

		OTList();
		OTList(RMGfxPrimitive *pr) {
			_prim = pr;
		}
	};

	bool _trackDirtyRects;
	Common::List<Common::Rect> _currentDirtyRects, _previousDirtyRects, _dirtyRects;

	void mergeDirtyRects();

private:
	//OSystem::MutexRef csModifyingOT;

protected:
	OTList *_otlist;
	int _otSize;

public:
	RMGfxTargetBuffer();
	virtual ~RMGfxTargetBuffer();

	static uint16 *_precalcTable;
	static void createBWPrecalcTable();
	static void freeBWPrecalcTable();

	// management of the OT list
	void clearOT();
	void drawOT(CORO_PARAM);
	void addPrim(RMGfxPrimitive *prim); // The pointer must be delted

	operator byte *();
	operator void *();
	operator uint16 *();

	// Offseting buffer
	void offsetY(int nLines);

	// Dirty rect methods
	void addDirtyRect(const Common::Rect &r);
	Common::List<Common::Rect> &getDirtyRects();
	void clearDirtyRects();
	void setTrackDirtyRects(bool v);
	bool getTrackDirtyRects() const;
};

/**
 * Ring buffer, which is both source and by destination
 */
class RMGfxWoodyBuffer: public RMGfxSourceBuffer16, public RMGfxTargetBuffer {
public:
	RMGfxWoodyBuffer();
	RMGfxWoodyBuffer(int dimx, int dimy);
	virtual ~RMGfxWoodyBuffer();

	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
};

} // End of namespace Tony

#endif
