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
 * Watch.h
 */

#ifdef SUPERMODEL_DEBUGGER
#ifndef INCLUDED_WATCH_H
#define INCLUDED_WATCH_H

#include "AddressTable.h"
#include "Types.h"

#include <vector>
using namespace std;

namespace Debugger
{
	class CCPUDebug;
	class CIOPort;

	/*
	 * Base class for a memory/IO watch.  All watches keep track of the last read/write to the memory location or I/O port.
	 */
	class CWatch : public CAddressRef
	{
	private:
		static UINT32 GetIOAddress(CIO *io);

	protected:
		CWatch(CCPUDebug *wCPU, UINT32 wAddr, unsigned wSize, char wSymbol, const char *wType, bool wTrigRead, bool wTrigWrite);

		CWatch(CCPUDebug *wCPU, CIO *wIO, char wSymbol, const char *wType, bool wTrigInput, bool wTrigOutput);

	public:
		CIO *io;
		const char symbol;
		const char *type;
		bool trigRead;
		bool trigWrite;
		
		unsigned num;
		bool active;

		unsigned readCount;
		unsigned writeCount;

		bool HasValue();

		UINT64 GetValue();

		bool CheckRead(UINT32 cAddr, unsigned cDataSize, UINT64 data);

		bool CheckWrite(UINT32 cAddr, unsigned cDataSize, UINT64 data);
		
		bool CheckInput(CIO *cIO, UINT64 data);

		bool CheckOutput(CIO *cIO, UINT64 data);

		virtual bool CheckBreak(bool isRead, UINT64 data) = 0;

		virtual void Reset();

		virtual bool GetInfo(char *str);
	};

	/*
	 * Simple watch that will always halt execution when read from and/or written to.
	 */ 
	class CSimpleWatch : public CWatch
	{
	public:
		CSimpleWatch(CCPUDebug *wCPU, UINT32 wAddr, unsigned wSize, bool wTrigRead, bool wTrigWrite);

		CSimpleWatch(CCPUDebug *wCPU, CIO *wIO, bool wTrigInput, bool wTrigOutput);

		bool CheckBreak(bool isRead, UINT64 data);
	};

	/*
	 * Count watch that will halt execution after it has been read from and/or written to a certain number of times.
	 */
	class CCountWatch : public CWatch
	{
	private:
		unsigned m_count;
		unsigned m_counter;

	public:
		CCountWatch(CCPUDebug *wCPU, UINT32 wAddr, unsigned wSize, bool wTrigRead, bool wTrigWrite, unsigned count);

		CCountWatch(CCPUDebug *wCPU, CIO *wIO, bool wTrigInput, bool wTrigOutput, unsigned count);

		bool CheckBreak(bool isRead, UINT64 data);

		void Reset();

		bool GetInfo(char *str);
	};

	/*
	 * Match watch that will halt execution after a particular data sequence has been read from and/or written to it.
	 */
	class CMatchWatch : public CWatch
	{
	private:
		unsigned m_dataLen;
		UINT64 *m_dataArray;
		unsigned m_counter;

	public:
		CMatchWatch(CCPUDebug *wCPU, UINT32 wAddr, unsigned wSize, bool wTrigRead, bool wTrigWrite, vector<UINT64> &dataSeq);

		CMatchWatch(CCPUDebug *wCPU, CIO *wIO, bool wTrigInput, bool wTrigOutput, vector<UINT64> &dataSeq);

		~CMatchWatch();

		bool CheckBreak(bool isRead, UINT64 data);

		void Reset();

		bool GetInfo(char *str);
	};

	/*
	 * Capture watch that will halt not execution but will keep track of all data read from and/or written to it.
	 */
	class CCaptureWatch : public CWatch
	{
	private:
		unsigned m_maxLen;
		unsigned m_len;
		unsigned m_index;
		UINT64 m_dataVals[255];
		bool m_dataRW[255];

	public:
		CCaptureWatch(CCPUDebug *wCPU, UINT32 wAddr, unsigned wSize, bool wTrigRead, bool wTrigWrite, unsigned wMaxLen);

		CCaptureWatch(CCPUDebug *wCPU, CIO *wIO, bool wTrigInput, bool wTrigOutput, unsigned wMaxLen);

		bool CheckBreak(bool isRead, UINT64 data);

		void Reset();

		bool GetInfo(char *str);
	};

	/*
	 * Print watch that will halt not execution but will output all data read from and/or written to it.
	 */
	class CPrintWatch : public CWatch
	{
	public:
		CPrintWatch(CCPUDebug *wCPU, UINT32 wAddr, unsigned wSize, bool wTrigRead, bool wTrigWrite);

		CPrintWatch(CCPUDebug *wCPU, CIO *wIO, bool wTrigInput, bool wTrigOutput);

		bool CheckBreak(bool isRead, UINT64 data);
	};
}

#endif	// INCLUDED_WATCH_H
#endif  // SUPERMODEL_DEBUGGER