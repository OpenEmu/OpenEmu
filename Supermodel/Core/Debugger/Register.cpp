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
 * Register.cpp
 */

#ifdef SUPERMODEL_DEBUGGER

#include "CPUDebug.h"
#include "Debugger.h"
#include "Register.h"

#include <cstdio>
#include <string>

namespace Debugger
{
	CRegister::CRegister(CCPUDebug *regCPU, const char *regName, const char *regGroup, UINT8 regDataWidth) : 
		cpu(regCPU), name(regName), group(regGroup), dataWidth(regDataWidth)
	{
		//
	}

	double CRegister::GetValueAsFPoint()
	{
		return (double)GetValueAsInt();
	}

	CRegMonitor::CRegMonitor(CRegister *theReg) : reg(theReg)
	{
		//
	}

	CPCRegister::CPCRegMonitor::CPCRegMonitor(CPCRegister *theReg) : CRegMonitor(theReg)
	{
		Reset();
	}

	bool CPCRegister::CPCRegMonitor::HasChanged()
	{
		return m_prevPC != reg->cpu->pc;
	}

	void CPCRegister::CPCRegMonitor::GetBeforeValue(char *str)
	{
		reg->cpu->FormatAddress(str, m_prevPC);
	}

	void CPCRegister::CPCRegMonitor::Reset()
	{
		m_prevPC = reg->cpu->pc;
	}

	CPCRegister::CPCRegister(CCPUDebug *cpu, const char *name, const char *group) : CRegister(cpu, name, group, cpu->memBusWidth)
	{
		//
	}

	UINT32 CPCRegister::Get()
	{
		return cpu->pc;
	}

	bool CPCRegister::Set(UINT32 val)
	{
		return false;
	}

	void CPCRegister::GetValue(char *str)
	{
		if (cpu->instrCount > 0)
			cpu->FormatAddress(str, cpu->pc);
		else
		{
			str[0] = '-';
			str[1] = '\0';
		}
	}

	UINT64 CPCRegister::GetValueAsInt()
	{
		return cpu->pc;
	}

	bool CPCRegister::SetValue(const char *str)
	{
		return false;
	}

	CRegMonitor *CPCRegister::CreateMonitor()
	{ 
		return new CPCRegMonitor(this);
	}

	CAddrRegister::CAddrRegMonitor::CAddrRegMonitor(CAddrRegister *theReg) : CRegMonitor(theReg)
	{
		Reset();
	}

	bool CAddrRegister::CAddrRegMonitor::HasChanged()
	{
		return m_prevAddr != ((CAddrRegister*)reg)->Get();
	}

	void CAddrRegister::CAddrRegMonitor::GetBeforeValue(char *str)
	{
		reg->cpu->FormatAddress(str, m_prevAddr);
	}

	void CAddrRegister::CAddrRegMonitor::Reset()
	{
		m_prevAddr = ((CAddrRegister*)reg)->Get();
	}

	CAddrRegister::CAddrRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt32FPtr getFunc, SetInt32FPtr setFunc) : 
		CRegister(cpu, name, group, cpu->memBusWidth), m_id(id), m_getFunc(getFunc), m_setFunc(setFunc)
	{
		//
	}

	UINT32 CAddrRegister::Get()
	{
		return m_getFunc(cpu, m_id);
	}

	bool CAddrRegister::Set(UINT32 val)
	{
		return m_setFunc(cpu, m_id, val);
	}

	void CAddrRegister::GetValue(char *str)
	{
		cpu->FormatAddress(str, Get());
	}

	UINT64 CAddrRegister::GetValueAsInt()
	{
		return Get();
	}

	bool CAddrRegister::SetValue(const char *str)
	{
		UINT32 addr;
		if (!cpu->ParseAddress(str, &addr))
			return false;
		return Set(addr);
	}

	CRegMonitor *CAddrRegister::CreateMonitor()
	{ 
		return new CAddrRegMonitor(this);
	}

	CIntRegister::CIntRegMonitor::CIntRegMonitor(CIntRegister *theReg) : CRegMonitor(theReg)
	{
		Reset();
	}

	bool CIntRegister::CIntRegMonitor::HasChanged()
	{
		return m_prevVal != ((CIntRegister*)reg)->Get();
	}

	void CIntRegister::CIntRegMonitor::GetBeforeValue(char *str)
	{
		reg->cpu->FormatData(str, reg->dataWidth / 8, m_prevVal);
	}

	void CIntRegister::CIntRegMonitor::Reset()
	{
		m_prevVal = ((CIntRegister*)reg)->Get();
	}

	CIntRegister::CIntRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt8FPtr getFunc, SetInt8FPtr setFunc) :
		CRegister(cpu, name, group, 8), m_id(id),
		m_get8Func(getFunc), m_set8Func(setFunc), m_get16Func(NULL), m_set16Func(NULL), 
		m_get32Func(NULL), m_set32Func(NULL), m_get64Func(NULL), m_set64Func(NULL)
	{
		//
	}

	CIntRegister::CIntRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt16FPtr getFunc, SetInt16FPtr setFunc) :
		CRegister(cpu, name, group, 16), m_id(id),
		m_get8Func(NULL), m_set8Func(NULL), m_get16Func(getFunc), m_set16Func(setFunc), 
		m_get32Func(NULL), m_set32Func(NULL), m_get64Func(NULL), m_set64Func(NULL)
	{
		//
	}

	CIntRegister::CIntRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt32FPtr getFunc, SetInt32FPtr setFunc) :
		CRegister(cpu, name, group, 32), m_id(id),
		m_get8Func(NULL), m_set8Func(NULL), m_get16Func(NULL), m_set16Func(NULL), 
		m_get32Func(getFunc), m_set32Func(setFunc), m_get64Func(NULL), m_set64Func(NULL)
	{
		//
	}

	CIntRegister::CIntRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt64FPtr getFunc, SetInt64FPtr setFunc) :
		CRegister(cpu, name, group, 64), m_id(id),
		m_get8Func(NULL), m_set8Func(NULL), m_get16Func(NULL), m_set16Func(NULL), 
		m_get32Func(NULL), m_set32Func(NULL), m_get64Func(getFunc), m_set64Func(setFunc)
	{
		//
	}

	UINT64 CIntRegister::Get()
	{
		switch (dataWidth / 8)
		{
			case 1:  return (UINT64)m_get8Func(cpu, m_id);
			case 2:  return (UINT64)m_get16Func(cpu, m_id);
			case 4:  return (UINT64)m_get32Func(cpu, m_id);
			case 8:  return m_get64Func(cpu, m_id);
			default: return 0; 
		}
	}

	bool CIntRegister::Set(UINT64 val)
	{
		switch (dataWidth / 8)
		{
			case 1:  return m_set8Func != NULL && m_set8Func(cpu, m_id, (UINT8)val);
			case 2:  return m_set16Func != NULL && m_set16Func(cpu, m_id, (UINT16)val);
			case 4:  return m_set32Func != NULL && m_set32Func(cpu, m_id, (UINT32)val);
			case 8:  return m_set64Func != NULL && m_set64Func(cpu, m_id, val);
			default: return false;
		}
	}

	void CIntRegister::GetValue(char *str)
	{
		cpu->FormatData(str, dataWidth / 8, Get());
	}

	UINT64 CIntRegister::GetValueAsInt()
	{
		return Get();
	}

	bool CIntRegister::SetValue(const char *str)
	{
		UINT64 data;
		if (!cpu->ParseData(str, dataWidth / 8, &data))
			return false;
		return Set(data);
	}

	CRegMonitor *CIntRegister::CreateMonitor()
	{ 
		return new CIntRegMonitor(this);
	}

	CStatusRegister::CStatusRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt8FPtr getFunc, SetInt8FPtr setFunc) :
		CIntRegister(cpu, name, group, id, getFunc, setFunc), numBits(0), offChr('.')
	{
		memset(&m_bitChrs, '\0', sizeof(m_bitChrs));
	}

	CStatusRegister::CStatusRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt16FPtr getFunc, SetInt16FPtr setFunc) :
		CIntRegister(cpu, name, group, id, getFunc, setFunc), numBits(0), offChr('.')
	{
		memset(&m_bitChrs, '\0', sizeof(m_bitChrs));
	}

	CStatusRegister::CStatusRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt32FPtr getFunc, SetInt32FPtr setFunc) :
		CIntRegister(cpu, name, group, id, getFunc, setFunc), numBits(0), offChr('.')
	{
		memset(&m_bitChrs, '\0', sizeof(m_bitChrs));
	}

	CStatusRegister::CStatusRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt64FPtr getFunc, SetInt64FPtr setFunc) :
		CIntRegister(cpu, name, group, id, getFunc, setFunc), numBits(0), offChr('.')
	{
		memset(&m_bitChrs, '\0', sizeof(m_bitChrs));
	}

	char &CStatusRegister::operator[](const unsigned index)
	{
		if (index >= dataWidth)
			exit(1);
		numBits = max<unsigned>(numBits, index);
		return m_bitChrs[index];
	}

	void CStatusRegister::SetBits(const char *bitStr)
	{
		// Find end
		const char *p = bitStr;
		while (*p != '\0')
			p++;
		unsigned i = 0;
		while (p-- > bitStr && i < dataWidth)
		{ 
			if (*p != offChr)
				m_bitChrs[i] = *p;
			i++;
		}
		numBits = i;
	}

	void CStatusRegister::GetValue(char *str)
	{
		UINT64 val = Get();
		char *p = str + numBits;
		*p-- = '\0';
		for (unsigned i = 0; i < numBits; i++)
		{
			*p-- = (val&0x01 ? m_bitChrs[i] : offChr);
			val >>= 1;
		}
	}

	bool CStatusRegister::SetValue(const char *str)
	{
		UINT64 val = 0;
		while (*str != '\0')
		{
			if (*str != offChr)
			{
				for (unsigned i = 0; i < numBits; i++)
				{
					if (m_bitChrs[i] == *str)
					{
						val |= 1LL<<i;
						break;
					}
				}
			}
			str++;
		}
		return Set(val);
	}

	CFPointRegister::CFPointRegMonitor::CFPointRegMonitor(CFPointRegister *theReg) : CRegMonitor(theReg)
	{
		Reset();
	}

	bool CFPointRegister::CFPointRegMonitor::HasChanged()
	{
		return m_prevVal != ((CFPointRegister*)reg)->Get();
	}

	void CFPointRegister::CFPointRegMonitor::GetBeforeValue(char *str)
	{
		sprintf(str, "%f", m_prevVal);
	}

	void CFPointRegister::CFPointRegMonitor::Reset()
	{
		m_prevVal = ((CFPointRegister*)reg)->Get();
	}

	CFPointRegister::CFPointRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetFPointFPtr getFunc, SetFPointFPtr setFunc) : 
		CRegister(cpu, name, group, 8), m_id(id), m_getFunc(getFunc), m_setFunc(setFunc)
	{
		//
	}

	double CFPointRegister::Get()
	{
		return m_getFunc(cpu, m_id);
	}

	bool CFPointRegister::Set(double val)
	{
		if (m_setFunc == NULL)
			return false;
		m_setFunc(cpu, m_id, val);
		return true;
	}

	void CFPointRegister::GetValue(char *str)
	{
		sprintf(str, "%lf", Get());
	}

	UINT64 CFPointRegister::GetValueAsInt()
	{
		return (UINT64)Get();
	}

	double CFPointRegister::GetValueAsFPoint()
	{
		return Get();
	}

	bool CFPointRegister::SetValue(const char *str)
	{
		double val;
		if (sscanf(str, "%lf", &val) != 1)
			return false;
		return Set(val);
	}

	CRegMonitor *CFPointRegister::CreateMonitor()
	{
		return new CFPointRegMonitor(this);
	}
}

#endif  // SUPERMODEL_DEBUGGER
