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
 * CodeAnalyser.cpp
 */

#ifdef SUPERMODEL_DEBUGGER

#include "CodeAnalyser.h"
#include "CPUDebug.h"
#include "Label.h"

#include <cctype>
#include <string>

namespace Debugger
{
	CEntryPoint::CEntryPoint(const CEntryPoint &other) : addr(other.addr), autoFlag(other.autoFlag)
	{
		if (other.autoLabel != NULL)
		{
			strncpy(autoLabelStr, other.autoLabel, 254);
			autoLabelStr[254] = '\0';
			autoLabel = autoLabelStr;
		}
		else
			autoLabel = NULL;
	}

	CEntryPoint::CEntryPoint(UINT32 eAddr, ELabelFlags eAutoFlag, const char *eAutoLabel) :
		addr(eAddr), autoFlag(eAutoFlag)
	{
		if (eAutoLabel != NULL)
		{
			strncpy(autoLabelStr, eAutoLabel, 254);
			autoLabelStr[254] = '\0';
			autoLabel = autoLabelStr;
		}
		else
			autoLabel = NULL;
	}

	CEntryPoint &CEntryPoint::operator=(const CEntryPoint &other)
	{
		addr = other.addr;
		autoFlag = other.autoFlag;
		if (other.autoLabel != NULL)
		{
			strncpy(autoLabelStr, other.autoLabel, 254);
			autoLabelStr[254] = '\0';
			autoLabel = autoLabelStr;
		}
		else
			autoLabel = NULL;
		return *this;
	}

	bool CEntryPoint::operator==(const CEntryPoint &other)
	{
		return addr == other.addr && autoFlag == other.autoFlag;
	}

	bool CEntryPoint::operator!=(const CEntryPoint &other)
	{
		return addr != other.addr || autoFlag != other.autoFlag;
	}

	const char *CAutoLabel::s_defaultLabelFmts[] = { "Entry%s", "Ex%s", "Int%s", "Jmp%s", "Loop%s", "Sub%s", NULL };

	const unsigned CAutoLabel::numLabelFlags = sizeof(s_defaultLabelFmts) / sizeof(char*);

	ELabelFlags CAutoLabel::GetLabelFlag(int index)
	{
		if (index < 0 || index >= numLabelFlags)
			return LFNone;
		return (ELabelFlags)(1 << index);
	}

	int CAutoLabel::GetFlagIndex(ELabelFlags flag)
	{
		switch (flag)
		{
			case LFEntryPoint:   return 0;
			case LFExcepHandler: return 1;
			case LFInterHandler: return 2;
			case LFJumpTarget:   return 3;
			case LFLoopPoint:    return 4;
			case LFSubroutine:   return 5;
			case LFUnseenCode:   return 6;
			default:             return -1;
		}
	}

	const char *CAutoLabel::GetFlagString(ELabelFlags flag)
	{
		switch (flag)
		{
			case LFEntryPoint:   return "Entry Point";
			case LFExcepHandler: return "Exception Handler";
			case LFInterHandler: return "Interrupt Handler";
			case LFJumpTarget:   return "Jump Target";
			case LFLoopPoint:    return "Loop Point";
			case LFSubroutine:   return "Subroutine";
			case LFUnseenCode:   return "Unseen Code";
			default:             return "";
		}
	}

	CAutoLabel::CAutoLabel(CCPUDebug *lCPU, UINT32 lAddr) : CAddressRef(lCPU, lAddr), flags(LFNone), m_acquired(0)
	{
		memset(m_subLabels, NULL, sizeof(m_subLabels));
	}

	CAutoLabel::~CAutoLabel()
	{
		// Delete all sub-labels
		for (int index = 0; index < numLabelFlags; index++)
		{
			if (m_subLabels[index] != NULL)
			{
				delete[] m_subLabels[index];
				m_subLabels[index] = NULL;
			}
		}
	}

	void CAutoLabel::Acquire()
	{
		m_acquired++;
	}

	void CAutoLabel::Release()
	{
		if (--m_acquired == 0)
			delete this;
	}

	void CAutoLabel::AddFlag(ELabelFlags flag, const char *subLabel)
	{
		int index = GetFlagIndex(flag);
		if (index == -1)
			return;
		flags = (ELabelFlags)((unsigned)flags | (unsigned)flag);
		if (subLabel != NULL)
		{
			size_t len = strlen(subLabel);
			char *label = new char[len + 1];
			strncpy(label, subLabel, len);
			label[len] = '\0';
			m_subLabels[index] = label;
		}
		else
			m_subLabels[index] = CreateDefaultSubLabel(flag);
	}

	bool CAutoLabel::GetLabel(char *labelStr, ELabelFlags subFlags)
	{	
		char *p = labelStr;
		*p = '\0';
		for (int index = 0; index < numLabelFlags; index++)
		{
			ELabelFlags flag = GetLabelFlag(index);
			if (!(subFlags & flag))
				continue;
			const char *subLabel = m_subLabels[index];
			if (subLabel == NULL)
				continue;
			if (p > labelStr)
			{
				(*p++) = '/';
				*p = '\0';
			}
			strcat(p, subLabel);
			p += strlen(subLabel);
			*p = '\0';
		}
		return p > labelStr;
	}

	bool CAutoLabel::ContainsSubLabel(const char *subLabel)
	{
		for (int i = 0; i < numLabelFlags; i++)
		{
			if (m_subLabels[i] != NULL && stricmp(subLabel, m_subLabels[i]) == 0)
				return true;
		}
		return false;
	}

	const char *CAutoLabel::CreateDefaultSubLabel(ELabelFlags flag)
	{
		int index = GetFlagIndex(flag);
		if (index == -1)
			return NULL;
		const char *labelFmt = s_defaultLabelFmts[index];
		if (labelFmt == NULL)
			return NULL;
		char addrStr[50];
		cpu->debugger->FormatData(addrStr, Hex, (unsigned)(cpu->memBusWidth / 8), (UINT64)addr);
		char *label = new char[255];
		sprintf(label, labelFmt, addrStr);
		return label;
	}

	CCodeAnalysis::CCodeAnalysis(CCodeAnalyser *aAnalyser) : analyser(aAnalyser)
	{
		//
	}

	CCodeAnalysis::CCodeAnalysis(CCodeAnalyser *aAnalyser, unsigned aTotalIndices, vector<CEntryPoint> &entryPoints, vector<UINT32> &unseenEntryAddrs) : 
		analyser(aAnalyser), m_entryPoints(entryPoints), m_unseenEntryAddrs(unseenEntryAddrs), 
		m_seenIndices(aTotalIndices), m_validIndices(aTotalIndices), m_acquired(0)
	{
		//
	}

	CCodeAnalysis::CCodeAnalysis(CCodeAnalysis *oldAnalysis, vector<CEntryPoint> &entryPoints, vector<UINT32> &unseenEntryAddrs) :
		analyser(oldAnalysis->analyser), m_entryPoints(entryPoints), m_unseenEntryAddrs(unseenEntryAddrs), 
		m_seenIndices(oldAnalysis->m_seenIndices), m_validIndices(oldAnalysis->m_validIndices), 
		m_autoLabelsMap(oldAnalysis->m_autoLabelsMap), m_acquired(0), validIndexSet(oldAnalysis->validIndexSet)
	{
		for (map<unsigned,CAutoLabel*>::iterator it = m_autoLabelsMap.begin(); it != m_autoLabelsMap.end(); it++)
			it->second->Acquire();
	}

	CCodeAnalysis::~CCodeAnalysis()
	{
		for (vector<CAutoLabel*>::iterator it = autoLabels.begin(); it != autoLabels.end(); it++)
			(*it)->Release();
		autoLabels.clear();
		m_autoLabelsMap.clear();
	}

	void CCodeAnalysis::Acquire()
	{
		m_acquired++;
	}

	void CCodeAnalysis::Release()
	{
		if (--m_acquired == 0)
			delete this;
	}

	void CCodeAnalysis::FinishAnalysis()
	{
		for (map<UINT32,CAutoLabel*>::iterator it = m_autoLabelsMap.begin(); it != m_autoLabelsMap.end(); it++)
			autoLabels.push_back(it->second);
	}

	bool CCodeAnalysis::IsAddrValid(UINT32 addr)
	{
		unsigned index;
		if (!analyser->GetIndexOfAddr(addr, index))
			return false;
		return IsIndexValid(index);
	}

	bool CCodeAnalysis::GetNextValidAddr(UINT32 &addr)
	{
		unsigned index;
		if (!analyser->GetIndexOfAddr(addr, index))
			return false;
		if (IsIndexValid(index))
			return true;
		set<unsigned>::iterator it = validIndexSet.lower_bound(index);
		if (it == validIndexSet.end())
			return false;
		return analyser->GetAddrOfIndex(*it, addr);
	}

	bool CCodeAnalysis::IsIndexValid(unsigned index)
	{
		return index < m_validIndices.size() && m_validIndices[index];
	}

	bool CCodeAnalysis::GetNextValidIndex(unsigned &index)
	{
		if (IsIndexValid(index))
			return true;
		set<unsigned>::iterator it = validIndexSet.lower_bound(index);
		if (it == validIndexSet.end())
			return false;
		index = *it;
		return true;
	}

	bool CCodeAnalysis::HasSeenAddr(UINT32 addr)
	{
		unsigned index;
		if (!analyser->GetIndexOfAddr(addr, index))
			return false;
		return HaveSeenIndex(index);
	}

	bool CCodeAnalysis::HaveSeenIndex(unsigned index)
	{
		return index < m_seenIndices.size() && m_seenIndices[index];
	}

	CAutoLabel *CCodeAnalysis::GetAutoLabel(UINT32 addr)
	{
		map<UINT32,CAutoLabel*>::iterator it = m_autoLabelsMap.find(addr);
		if (it == m_autoLabelsMap.end())
			return NULL;
		return it->second;
	}

	CAutoLabel *CCodeAnalysis::GetAutoLabel(const char *subLabel)
	{
		for (vector<CAutoLabel*>::iterator it = autoLabels.begin(); it != autoLabels.end(); it++)
		{
			if ((*it)->ContainsSubLabel(subLabel))
				return *it;
		}
		return NULL;
	}

	vector<CAutoLabel*> CCodeAnalysis::GetAutoLabels(ELabelFlags flag)
	{
		vector<CAutoLabel*> matched;
		for (vector<CAutoLabel*>::iterator it = autoLabels.begin(); it != autoLabels.end(); it++)
		{
			if ((*it)->flags & flag)
				matched.push_back(*it);
		}
		return matched;
	}	

	CCodeAnalyser::CCodeAnalyser(CCPUDebug *aCPU) : cpu(aCPU), emptyAnalysis(this), analysis(&emptyAnalysis)
	{
		instrAlign = cpu->minInstrLen;

		totalIndices = 0;
		for (vector<CRegion*>::iterator it = cpu->regions.begin(); it != cpu->regions.end(); it++)
		{
			if (!(*it)->isCode)
				continue;
			m_codeRegions.push_back(*it);
			totalIndices += (*it)->size / instrAlign;
			m_indexBounds.push_back(totalIndices);
		}
	}

	CCodeAnalyser::~CCodeAnalyser()
	{
		if (analysis != &emptyAnalysis)
			analysis->Release();
	}

	void CCodeAnalyser::Reset()
	{
		CCodeAnalysis *oldAnalysis = analysis;
		analysis = &emptyAnalysis;
		if (oldAnalysis != &emptyAnalysis)
			oldAnalysis->Release();
	}

	bool CCodeAnalyser::GetAddrOfIndex(unsigned index, UINT32 &addr)
	{
		unsigned regIndex = 0;
		unsigned prevBound = 0;
		for (vector<unsigned>::iterator it = m_indexBounds.begin(); it != m_indexBounds.end(); it++)
		{
			if (*it > index)
			{
				addr = m_codeRegions[regIndex]->addr + (UINT32)(index - prevBound) * instrAlign;
				return true;
			}
			prevBound = *it;
			regIndex++;
		}
		return false;
	}

	bool CCodeAnalyser::GetIndexOfAddr(UINT32 addr, unsigned &index)
	{
		unsigned regIndex = 0;
		for (vector<CRegion*>::iterator it = m_codeRegions.begin(); it != m_codeRegions.end(); it++)
		{
			if ((*it)->addr <= addr && addr <= (*it)->addrEnd)
			{
				unsigned offset = (unsigned)((addr - (*it)->addr) / instrAlign);
				index = (regIndex > 0 ? m_indexBounds[regIndex - 1] + offset : (unsigned)offset);
				return true;
			}
			regIndex++;
		}
		return false;
	}

	void CCodeAnalyser::CheckEntryPoints(vector<CEntryPoint> &entryPoints, vector<UINT32> &unseenEntryAddrs, vector<CEntryPoint> &prevPoints,
		bool &needsAnalysis, bool &reanalyse)
	{
		needsAnalysis = false;

		// Gather entry points
		GatherEntryPoints(entryPoints, unseenEntryAddrs, reanalyse);
		if (reanalyse)
		{
			needsAnalysis = true;
			return;
		}

		// Compare new entry points with previous ones
		for (size_t i = 0; i < entryPoints.size(); i++)
		{
			// Check if have more than before
			if (i >= prevPoints.size())
			{
				needsAnalysis = true;
				return;
			}
			// Check if any have changed
			if (entryPoints[i] != prevPoints[i])
			{
				// If entry points, exception handlers or interrupt handlers have changed, then force reanalysis
				if (entryPoints[i].autoFlag == LFEntryPoint || prevPoints[i].autoFlag == LFEntryPoint ||
					entryPoints[i].autoFlag == LFExcepHandler || prevPoints[i].autoFlag == LFExcepHandler ||
					entryPoints[i].autoFlag == LFInterHandler || prevPoints[i].autoFlag == LFInterHandler)
					reanalyse = true;
				needsAnalysis = true;
				return;
			}
		}
		// Check if have less than before
		if (entryPoints.size() < prevPoints.size())
		{
			// If so, force reanalysis
			reanalyse = true;
			needsAnalysis = true;
			return;
		}		
	}

	void CCodeAnalyser::GatherEntryPoints(vector<CEntryPoint> &entryPoints, vector<UINT32> &unseenEntryAddrs, bool &reanalyse)
	{
		char labelStr[255];
		UINT32 addr;
		unsigned index;

		entryPoints.clear();
		reanalyse = false;

		// Add reset address as main entry point
		AddEntryPoint(entryPoints, cpu->GetResetAddr(), LFEntryPoint, "MainEntry");

		// Add exception handlers as entry points
		for (vector<CException*>::iterator it = cpu->exceps.begin(); it != cpu->exceps.end(); it++)
		{
			if (!cpu->GetHandlerAddr(*it, addr))
				continue;
			sprintf(labelStr, "Ex%s", (*it)->id);
			AddEntryPoint(entryPoints, addr, LFExcepHandler, labelStr);
		}

		// Add interrupt handlers as entry points
		for (vector<CInterrupt*>::iterator it = cpu->inters.begin(); it != cpu->inters.end(); it++)
		{
			if (!cpu->GetHandlerAddr(*it, addr))
				continue;
			sprintf(labelStr, "Int%s", (*it)->id);
			AddEntryPoint(entryPoints, addr, LFInterHandler, labelStr);
		}

		// Add custom entry addresses
		unsigned i = 0;
		for (vector<UINT32>::iterator it = m_customEntryAddrs.begin(); it != m_customEntryAddrs.end(); it++)
		{
			sprintf(labelStr, "Custom%u", i++);
			AddEntryPoint(entryPoints, *it, LFEntryPoint, labelStr);
		}

		// If current PC address is at an unseen location or at location that was previously invalid, then add address as unseen entry point
		if (cpu->instrCount > 0 && GetIndexOfAddr(cpu->pc, index) && (!analysis->HaveSeenIndex(index) || !analysis->IsIndexValid(index)))
		{
			// If at location that was previously seen and was invalid, then force reanalysis (ie because code may have been modified)
			if (analysis->HaveSeenIndex(index) && !analysis->IsIndexValid(index))
				reanalyse = true;

			// Check that address not already included in previous entry points
			bool unseen = true;
			for (vector<CEntryPoint>::iterator it = entryPoints.begin(); it != entryPoints.end(); it++)
			{
				if ((*it).addr == cpu->pc)
				{
					unseen = false;
					break;
				}
			}
			if (unseen && find(unseenEntryAddrs.begin(), unseenEntryAddrs.end(), cpu->pc) == unseenEntryAddrs.end())
				unseenEntryAddrs.push_back(cpu->pc);
		}

		// Add unseen entry points
		for (vector<UINT32>::iterator it = unseenEntryAddrs.begin(); it != unseenEntryAddrs.end(); it++)
			AddEntryPoint(entryPoints, *it, LFUnseenCode, NULL);
	}

	void CCodeAnalyser::AddEntryPoint(vector<CEntryPoint> &entryPoints, UINT32 addr, ELabelFlags autoFlag, const char *autoLabel)
	{
		CEntryPoint entryPoint(addr, autoFlag, autoLabel);
		if (find(entryPoints.begin(), entryPoints.end(), entryPoint) == entryPoints.end())
			entryPoints.push_back(entryPoint);
	}

	bool CCodeAnalyser::NeedsAnalysis()
	{
		vector<CEntryPoint> entryPoints;
		vector<UINT32> unseenEntryAddrs(analysis->m_unseenEntryAddrs);
		bool needsAnalysis;
		bool reanalyse;
		CheckEntryPoints(entryPoints, unseenEntryAddrs, analysis->m_entryPoints, needsAnalysis, reanalyse);
		return needsAnalysis;
	}

	bool CCodeAnalyser::AnalyseCode()
	{
		m_abortAnalysis = false;

		CCodeAnalysis *oldAnalysis = analysis;
		
		vector<CEntryPoint> entryPoints;
		vector<UINT32> unseenEntryAddrs(oldAnalysis->m_unseenEntryAddrs);
		bool needsAnalysis;
		bool reanalyse;
		CheckEntryPoints(entryPoints, unseenEntryAddrs, oldAnalysis->m_entryPoints, needsAnalysis, reanalyse);
		if (!needsAnalysis)
			return false;

		CCodeAnalysis *newAnalysis;
		if (reanalyse || oldAnalysis == &emptyAnalysis)
			newAnalysis = new CCodeAnalysis(this, totalIndices, entryPoints, unseenEntryAddrs);
		else
			newAnalysis = new CCodeAnalysis(oldAnalysis, entryPoints, unseenEntryAddrs);
		newAnalysis->Acquire();

		for (vector<CEntryPoint>::iterator it = newAnalysis->m_entryPoints.begin(); it != newAnalysis->m_entryPoints.end(); it++)
		{
			AddFlagToAddr(newAnalysis->m_autoLabelsMap, it->addr, it->autoFlag, it->autoLabel);
			AnalyseCode(newAnalysis->m_seenIndices, newAnalysis->m_validIndices, newAnalysis->validIndexSet, newAnalysis->m_autoLabelsMap, it->addr);
		}
		newAnalysis->FinishAnalysis();

		if (m_abortAnalysis)
		{
			newAnalysis->Release();
			return false;
		}

		analysis = newAnalysis;
		if (oldAnalysis != &emptyAnalysis)
			oldAnalysis->Release();

		cpu->debugger->AnalysisUpdated(this);
		return true;
	}

	void CCodeAnalyser::AnalyseCode(vector<bool> &seenIndices, vector<bool> &validIndices, set<unsigned> &validIndexSet,
		map<UINT32,CAutoLabel*> &autoLabelsMap, UINT32 addr)
	{
		if (m_abortAnalysis)
			return;

		unsigned index;
		if (!GetIndexOfAddr(addr, index) || seenIndices[index])
			return;
		
		CRegion *region = cpu->GetRegion(addr);
		if (region == NULL || !region->isCode)
			return;

		set<unsigned>::iterator setIt = validIndexSet.end();

		unsigned startIndex = index;
		do
		{
			if (m_abortAnalysis)
				return;

			// Flag that have seen this address index
			seenIndices[index] = true;

			// If unit is not valid (ie doesn't disassemble) then code block must be invalid (TODO - invalidate whole code block?)
			int codesLen = cpu->GetOpLength(addr);
			if (codesLen <= 0)
				return;

			validIndices[index] = true;
			if (setIt != validIndexSet.end())
				setIt = validIndexSet.insert(setIt, index);
			else
				setIt = validIndexSet.insert(index).first;
			
			UINT32 opcode = cpu->GetOpcode(addr);
			EOpFlags opFlags = cpu->GetOpFlags(addr, opcode);

			// See if instruction is jump
			if (opFlags & (JumpSimple|JumpLoop|JumpSub))
			{
				// If so, see if address is valid (ie known at disassemble time)
				UINT32 jumpAddr;
				if (cpu->GetJumpAddr(addr, opcode, jumpAddr))
				{
					// If so, add flags to jump address and analyse destination code block too
					if      (opFlags & JumpSub)  AddFlagToAddr(autoLabelsMap, jumpAddr, LFSubroutine, NULL);
					else if (opFlags & JumpLoop) AddFlagToAddr(autoLabelsMap, jumpAddr, LFLoopPoint, NULL);
					else                         AddFlagToAddr(autoLabelsMap, jumpAddr, LFJumpTarget, NULL);
					AnalyseCode(seenIndices, validIndices, validIndexSet, autoLabelsMap, jumpAddr);
				}
			}

			// Finish if instruction terminates code block (ie is not conditional and is either a non-returning jump, a return or some sort
			// of reset/halting instruction)
			if (!(opFlags & Conditional) && (opFlags & (JumpSimple|JumpLoop|ReturnEx|ReturnSub|HaltExec)))
				return;

			// Move to next index
			index += (unsigned)codesLen / instrAlign;

			// If reach end of address indices, code block must be invalid (TODO - invalidate whole code block?)
			if (index >= totalIndices)
				return;

			// Move to next address
			addr += (UINT32)codesLen;
			
			// If move between regions, check new region is valid
			if (addr > region->addrEnd)
			{
				region = cpu->GetRegion(addr);
				if (region == NULL || !region->isCode) // (TODO - invalidate whole code block?)
					return;
			}
		}
		while (!seenIndices[index]);
	}

	void CCodeAnalyser::AddFlagToAddr(map<UINT32,CAutoLabel*> &autoLabelsMap, UINT32 addr, ELabelFlags flag, const char *subLabel)
	{
		if (flag == LFNone)
			return;
		map<UINT32,CAutoLabel*>::iterator it = autoLabelsMap.find(addr);
		CAutoLabel *label;
		if (it == autoLabelsMap.end())
		{
			label = new CAutoLabel(cpu, addr);
			label->Acquire();
			autoLabelsMap[addr] = label;
		}
		else
			label = it->second;
		label->AddFlag(flag, subLabel);
	}
	
	void CCodeAnalyser::AbortAnalysis()
	{
		m_abortAnalysis = true;
	}
	
	void CCodeAnalyser::ClearCustomEntryAddrs()
	{
		m_customEntryAddrs.clear();
	}

	void CCodeAnalyser::AddCustomEntryAddr(UINT32 entryAddr)
	{
		if (find(m_customEntryAddrs.begin(), m_customEntryAddrs.end(), entryAddr) == m_customEntryAddrs.end())
			m_customEntryAddrs.push_back(entryAddr);
	}

	bool CCodeAnalyser::RemoveCustomEntryAddr(UINT32 entryAddr)
	{
		vector<UINT32>::iterator it = find(m_customEntryAddrs.begin(), m_customEntryAddrs.end(), entryAddr);
		if (it == m_customEntryAddrs.end())
			return false;
		m_customEntryAddrs.erase(it);
		return true;
	}

#ifdef DEBUGGER_HASBLOCKFILE
	bool CCodeAnalyser::LoadState(CBlockFile *state)
	{
		// Load custom entry addresses
		char blockStr[255];
		sprintf(blockStr, "%s.entryaddrs", cpu->name);
		if (state->FindBlock(blockStr) == OKAY)
		{
			m_customEntryAddrs.clear();
			UINT32 numAddrs;
			state->Read(&numAddrs, sizeof(numAddrs));
			for (UINT32 i = 0; i < numAddrs; i++)
			{
				UINT32 addr;
				state->Read(&addr, sizeof(addr));
				m_customEntryAddrs.push_back(addr);
			}
		}
		return true;
	}

	bool CCodeAnalyser::SaveState(CBlockFile *state)
	{
		// Save custom entry addresses
		char blockStr[255];
		sprintf(blockStr, "%s.entryaddrs", cpu->name);
		state->NewBlock(blockStr, __FILE__);
		UINT32 numAddrs = m_customEntryAddrs.size();
		state->Write(&numAddrs, sizeof(numAddrs));
		for (UINT32 i = 0; i < numAddrs; i++)
		{
			UINT32 addr = m_customEntryAddrs[i];
			state->Write(&addr, sizeof(addr));
		}
		return true;
	}
#endif // DEBUGGER_HASBLOCKFILE
}
#endif  // SUPERMODEL_DEBUGGER