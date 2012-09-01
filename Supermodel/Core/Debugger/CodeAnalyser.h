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
 * CodeAnalyser.h
 */

#ifdef SUPERMODEL_DEBUGGER
#ifndef INCLUDED_CODEANALYSER_H
#define INCLUDED_CODEANALYSER_H

#include <stdio.h>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
using namespace std;

#include "Types.h"

#include "Debugger.h"
#include "AddressTable.h"

#ifdef DEBUGGER_HASBLOCKFILE
#include "BlockFile.h"
#endif // DEBUGGER_HASBLOCKFILE

namespace Debugger
{
	class CCPUDebug;
	class CRegion;
	class CLabel;

	enum ELabelFlags
	{
		LFNone         = 0,
		LFEntryPoint   = 1, 
		LFExcepHandler = 2, 
		LFInterHandler = 4,
		LFJumpTarget   = 8, 
		LFLoopPoint    = 16,
		LFSubroutine   = 32,
		LFUnseenCode   = 64,
		LFAll          = (LFEntryPoint | LFExcepHandler | LFInterHandler | LFJumpTarget | LFSubroutine | LFLoopPoint | LFUnseenCode)
	};

	/*
	 * Class that represents an entry point into a program's code in memory, eg a CPU's reset address or exception handler's address etc.
	 */
	class CEntryPoint
	{
	private:
		char autoLabelStr[255];

	public:
		UINT32 addr;
		ELabelFlags autoFlag;
		const char *autoLabel;

		CEntryPoint(const CEntryPoint &other);

		CEntryPoint(UINT32 eAddr, ELabelFlags eAutoFlag, const char *eAutoLabel);

		CEntryPoint &operator=(const CEntryPoint &other);

		bool operator==(const CEntryPoint &other);
		
		bool operator!=(const CEntryPoint &other);
	};

	/*
	 * Class that represents an auto-generated label that resulted from code analysis, eg a known entry point or the destination of a subroutine call.
	 */
	class CAutoLabel : public CAddressRef
	{
	private:
		static const char *s_defaultLabelFmts[];

		const char *m_subLabels[7];
		
		unsigned m_acquired;

		const char *CreateDefaultSubLabel(ELabelFlags flag);

	public:
		static const unsigned numLabelFlags;

		static ELabelFlags GetLabelFlag(int index);

		static int GetFlagIndex(ELabelFlags flag);

		static const char *GetFlagString(ELabelFlags flag);

		ELabelFlags flags;
		
		CAutoLabel(CCPUDebug *lCPU, UINT32 lAddr);

		~CAutoLabel();

		void Acquire();

		void Release();

		void AddFlag(ELabelFlags flag, const char *subLabel);

		bool GetLabel(char *label, ELabelFlags flag = LFAll);

		bool ContainsSubLabel(const char *subLabel);
	};

	class CCodeAnalyser;

	/*
     * Class that holds the results of having analysed the program code.
	 */
	class CCodeAnalysis
	{
	friend class CCodeAnalyser;

	private:
		vector<CEntryPoint> m_entryPoints;
		vector<UINT32> m_unseenEntryAddrs;
		vector<bool> m_seenIndices;
		vector<bool> m_validIndices;
		map<UINT32,CAutoLabel*> m_autoLabelsMap;

		unsigned m_acquired;

		CCodeAnalysis(CCodeAnalyser *aAnalyser);

		CCodeAnalysis(CCodeAnalyser *aAnalyser, unsigned aTotalIndices, vector<CEntryPoint> &entryPoints, vector<UINT32> &m_unseenEntryAddrs);

		CCodeAnalysis(CCodeAnalysis *oldAnalysis, vector<CEntryPoint> &entryPoints, vector<UINT32> &m_unseenEntryAddrs);

		void FinishAnalysis();

	public:
		CCodeAnalyser *analyser;
		set<unsigned> validIndexSet;
		vector<CAutoLabel*> autoLabels;
		
		~CCodeAnalysis();

		void Acquire();

		void Release();

		bool IsAddrValid(UINT32 addr);

		bool GetNextValidAddr(UINT32 &addr);

		bool IsIndexValid(unsigned index);

		bool GetNextValidIndex(unsigned &index);

		bool HasSeenAddr(UINT32 addr);

		bool HaveSeenIndex(unsigned index);

		CAutoLabel *GetAutoLabel(UINT32 addr);

		CAutoLabel *GetAutoLabel(const char *subLabel);

		vector<CAutoLabel*> GetAutoLabels(ELabelFlags flag);
	};

	/*
	 * Class that analyses a program's code to work out all the reachable program locations from a given set of entry points into the code.
	 * During the analysis, it keeps track of valid memory locations (to help with the disassembly of code for CPUs with variable-length 
	 * instruction sets) and generates a set of auto-labels that identify places of interest such as subroutines, jump destinations and 
	 * exception handlers etc.
	 * This sort of analysis works well for static addressing modes but not so well for dynamic address referencing or self-modifying code.
	 * To allow for the latter cases the analyser updates its analysis whenever it encounters an unseen memory location or it sees 
	 * that code has changed from a previous inspection.
	 */
	class CCodeAnalyser
	{	
	private:
		vector<CRegion*> m_codeRegions;
		vector<unsigned> m_indexBounds;

		vector<UINT32> m_customEntryAddrs;

		bool m_abortAnalysis;

		void CheckEntryPoints(vector<CEntryPoint> &entryPoints, vector<UINT32> &unseenEntryAddrs, vector<CEntryPoint> &prevPoints,
			bool &needsAnalysis, bool &reanalyse);
		
		void GatherEntryPoints(vector<CEntryPoint> &entryPoints, vector<UINT32> &unseenEntryAddrs, bool &reanalyse);

		void AddEntryPoint(vector<CEntryPoint> &entryPoints, UINT32 addr, ELabelFlags autoFlag, const char *autoLabel);

		void AnalyseCode(vector<bool> &seenIndices, vector<bool> &validIndices, set<unsigned> &validIndexSet, map<UINT32,CAutoLabel*> &autoLabelsMap, UINT32 addr);

		void AddFlagToAddr(map<UINT32,CAutoLabel*> &autoLabelsMap, UINT32 addr, ELabelFlags autoFlag, const char *autoLabel);

	public:
		CCPUDebug *cpu;

		CCodeAnalysis emptyAnalysis;

		CCodeAnalysis *analysis;
		
		unsigned instrAlign;
		unsigned totalIndices;

		CCodeAnalyser(CCPUDebug *aCPU);

		~CCodeAnalyser();

		void Reset();

		bool GetAddrOfIndex(unsigned index, UINT32 &addr);

		bool GetIndexOfAddr(UINT32 addr, unsigned &index);

		//
		// Methods to check, run or abort analysis
		//

		bool NeedsAnalysis();

		bool AnalyseCode();

		void AbortAnalysis();

		//
		// Methods to manipulate custom entry addresses
		//

		void ClearCustomEntryAddrs();

		void AddCustomEntryAddr(UINT32 entryAddr);

		bool RemoveCustomEntryAddr(UINT32 entryAddr);

#ifdef DEBUGGER_HASBLOCKFILE
		bool LoadState(CBlockFile *state);

		bool SaveState(CBlockFile *state);
#endif // DEBUGGER_HASBLOCKFILE
	};
}

#endif	// INCLUDED_CODEANALYSER_H
#endif  // SUPERMODEL_DEBUGGER
