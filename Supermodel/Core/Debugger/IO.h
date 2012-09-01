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
 * IO.h
 */

#ifdef SUPERMODEL_DEBUGGER
#ifndef INCLUDED_IO_H
#define INCLUDED_IO_H

#include "Types.h"

#define MAX_LABEL_LENGTH 255

namespace Debugger
{
	class CCPUDebug;
	class CWatch;
	class CSimpleWatch;
	class CCountWatch;
	class CMatchWatch;
	class CPrintWatch;
	class CCaptureWatch;

	/*
	 * Base class that represents CPU input/output.
	 */
	class CIO
	{
	public:
		CCPUDebug *cpu;
		const char *name;
		const char *group;
		unsigned dataSize;

		CWatch *watch;

		unsigned inCount;
		unsigned outCount;
		UINT64 lastIn;
		UINT64 lastOut;
		UINT64 *last;
		
		CIO(CCPUDebug *ioCPU, const char *ioName, const char *ioGroup, unsigned ioDataSize);

		void RecordInput(UINT64 data);

		void RecordOutput(UINT64 data);

		bool CheckInput(UINT64 data);

		bool CheckOutput(UINT64 data);

		virtual void AddLabel(const char *name) = 0;

		virtual void RemoveLabel() = 0;

		CSimpleWatch *AddSimpleWatch(bool trigInput, bool trigOutput);

		CCountWatch *AddCountWatch(bool trigInput, bool trigOutput, unsigned count);

		CMatchWatch *AddMatchWatch(bool trigInput, bool trigOutput, vector<UINT64> &dataSeq);

		CPrintWatch *AddPrintWatch(bool trigInput, bool trigOutput);

		CCaptureWatch *AddCaptureWatch(bool trigInput, bool trigOutput, unsigned maxLen);

		bool RemoveWatch();

		virtual void GetLocation(char *str) = 0;

		virtual UINT64 Read() = 0;

		virtual bool Write(UINT64 data) = 0;
	};

	/*
	 * Class that represents a CPU input/ouput port.
	 */
	class CPortIO : public CIO
	{
	private:
		char labelStr[MAX_LABEL_LENGTH + 1];

	public:
		const UINT16 portNum;

		const char *label;
		
		CPortIO(CCPUDebug *ioCPU, const char *ioName, const char *ioGroup, unsigned ioDataSize, UINT16 ioPortNum);

		void AddLabel(const char *pLabel);

		void RemoveLabel();
		
		void GetLocation(char *str);

		UINT64 Read();

		bool Write(UINT64 data);
	};

	/*
	 * Class that represents CPU input/output that is mapped to a memory location.
	 */
	class CMappedIO : public CIO, public CAddressRef
	{
	public:
		CMappedIO(CCPUDebug *ioCPU, const char *ioName, const char *ioGroup, unsigned ioDataSize, UINT32 ioAddr);
			
		bool CheckInput(UINT32 cAddr, unsigned cDataSize, UINT64 data);

		bool CheckOutput(UINT32 cAddr, unsigned cDataSize, UINT64 data);

		void AddLabel(const char *name);

		void RemoveLabel();

		void GetLocation(char *str);

		UINT64 Read();

		bool Write(UINT64 data);
	};

	//
	// Inlined methods
	//

	inline void CIO::RecordInput(UINT64 data)
	{
		// Keep track of data coming in
		lastIn = data;
		last = &lastIn;
	}

	inline void CIO::RecordOutput(UINT64 data)
	{
		// Keep track of data going out
		lastOut = data;
		last = &lastOut;
	}
}

#endif	// INCLUDED_IO_H
#endif  // SUPERMODEL_DEBUGGER