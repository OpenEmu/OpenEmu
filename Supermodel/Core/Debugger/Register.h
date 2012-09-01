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
 * Register.h
 */

#ifdef SUPERMODEL_DEBUGGER
#ifndef INCLUDED_REGISTER_H
#define INCLUDED_REGISTER_H

#include "Types.h"

namespace Debugger
{
	class CCPUDebug;
	class CRegMonitor;

	typedef UINT8 (*GetInt8FPtr)(CCPUDebug*, unsigned);
	typedef bool (*SetInt8FPtr)(CCPUDebug*, unsigned, UINT8);
	typedef UINT16 (*GetInt16FPtr)(CCPUDebug*, unsigned);
	typedef bool (*SetInt16FPtr)(CCPUDebug*, unsigned, UINT16);
	typedef UINT32 (*GetInt32FPtr)(CCPUDebug*, unsigned);
	typedef bool (*SetInt32FPtr)(CCPUDebug*, unsigned, UINT32);
	typedef UINT64 (*GetInt64FPtr)(CCPUDebug*, unsigned);
	typedef bool (*SetInt64FPtr)(CCPUDebug*, unsigned, UINT64);
	typedef double (*GetFPointFPtr)(CCPUDebug*, unsigned);
	typedef bool (*SetFPointFPtr)(CCPUDebug*, unsigned, double);

	/*
	 * Base class for a CPU register.
	 */
	class CRegister
	{
	protected:
		CRegister(CCPUDebug *regCPU, const char *regName, const char *regGroup, UINT8 regDataWidth);

	public:
		CCPUDebug *cpu;
		const char *name;
		const char *group;
		const UINT8 dataWidth;

		virtual void GetValue(char *str) = 0;

		virtual UINT64 GetValueAsInt() = 0;

		virtual double GetValueAsFPoint();

		virtual bool SetValue(const char *str) = 0;

		virtual CRegMonitor *CreateMonitor() = 0;
	};

	/*
	 * Class that monitors a register to see if its value has changed.
	 */
	class CRegMonitor
	{
	protected:
		CRegMonitor(CRegister *theReg);

	public:
		CRegister *reg;

		virtual bool HasChanged() = 0;
		
		virtual void GetBeforeValue(char *str) = 0;

		virtual void Reset() = 0;
	};

	/*
	 * Class that represents the program counter of a CPU.
	 */
	class CPCRegister : public CRegister
	{
	private:
		class CPCRegMonitor : public CRegMonitor
		{
		private:
			UINT32 m_prevPC;

		public:
			CPCRegMonitor(CPCRegister *theReg);

			bool HasChanged();

			void GetBeforeValue(char *str);

			void Reset();
		};

	public:
		CPCRegister(CCPUDebug *cpu, const char *name, const char *group);

		UINT32 Get();

		bool Set(UINT32 val);

		virtual void GetValue(char *str);

		UINT64 GetValueAsInt();

		virtual bool SetValue(const char *str);
		
		CRegMonitor *CreateMonitor();
	};

	/* 
	 * Class that represents a CPU address register.
	 * TODO - formatting as per CPCRegister?
	 */
	class CAddrRegister : public CRegister
	{
	private:
		unsigned m_id;
		GetInt32FPtr m_getFunc;
		SetInt32FPtr m_setFunc;

		class CAddrRegMonitor : public CRegMonitor
		{
		private:
			UINT32 m_prevAddr;

		public:
			CAddrRegMonitor(CAddrRegister *theReg);

			bool HasChanged();

			void GetBeforeValue(char *str);

			void Reset();
		};

	public:
		CAddrRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt32FPtr getFunc, SetInt32FPtr setFunc = NULL);

		UINT32 Get();

		bool Set(UINT32 val);

		virtual void GetValue(char *str);

		UINT64 GetValueAsInt();

		virtual bool SetValue(const char *str);
		
		CRegMonitor *CreateMonitor();
	};

	/* 
	 * Class that represents a general purpose integer CPU register.
	 */
	class CIntRegister : public CRegister
	{
	private:
		unsigned m_id;
		GetInt8FPtr m_get8Func;
		SetInt8FPtr m_set8Func;
		GetInt16FPtr m_get16Func;
		SetInt16FPtr m_set16Func;
		GetInt32FPtr m_get32Func;
		SetInt32FPtr m_set32Func;
		GetInt64FPtr m_get64Func;
		SetInt64FPtr m_set64Func;

		class CIntRegMonitor : public CRegMonitor
		{
		private:
			UINT64 m_prevVal;

		public:
			CIntRegMonitor(CIntRegister *theReg);

			bool HasChanged();

			void GetBeforeValue(char *str);

			void Reset();
		};

	public:
		CIntRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt8FPtr getFunc, SetInt8FPtr setFunc);

		CIntRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt16FPtr getFunc, SetInt16FPtr setFunc);
		
		CIntRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt32FPtr getFunc, SetInt32FPtr setFunc);
		
		CIntRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt64FPtr getFunc, SetInt64FPtr setFunc);

		UINT64 Get();

		bool Set(UINT64 val);

		virtual void GetValue(char *str);

		UINT64 GetValueAsInt();

		virtual bool SetValue(const char *str);
		
		CRegMonitor *CreateMonitor();
	};

	/*
	 * Class that represents a CPU status register that contains bit flags.
	 */
	class CStatusRegister : public CIntRegister
	{
	private:
		char m_bitChrs[64];

	public:
		unsigned numBits;
		char offChr;

		CStatusRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt8FPtr getFunc, SetInt8FPtr setFunc);

		CStatusRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt16FPtr getFunc, SetInt16FPtr setFunc);
		
		CStatusRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt32FPtr getFunc, SetInt32FPtr setFunc);
		
		CStatusRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetInt64FPtr getFunc, SetInt64FPtr setFunc);

		char &operator[](const unsigned index);

		void SetBits(const char *bitStr);

		void GetValue(char *str);

		bool SetValue(const char *str);
	};

	/*
	 * Class that represents a floating-point CPU register.
	 */
	class CFPointRegister : public CRegister
	{
	private:
		unsigned m_id;
		GetFPointFPtr m_getFunc;
		SetFPointFPtr m_setFunc;

		class CFPointRegMonitor : public CRegMonitor
		{
		private:
			double m_prevVal;

		public:
			CFPointRegMonitor(CFPointRegister *theReg);

			bool HasChanged();

			void GetBeforeValue(char *str);

			void Reset();
		};

	public:
		CFPointRegister(CCPUDebug *cpu, const char *name, const char *group, unsigned id, GetFPointFPtr getFunc, SetFPointFPtr setFunc);

		double Get();

		bool Set(double val);

		void GetValue(char *str);

		UINT64 GetValueAsInt();

		double GetValueAsFPoint();

		bool SetValue(const char *str);

		CRegMonitor *CreateMonitor();
	};
}

#endif	// INCLUDED_REGISTER_H
#endif  // SUPERMODEL_DEBUGGER