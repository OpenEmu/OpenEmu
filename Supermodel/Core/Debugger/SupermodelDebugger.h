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
 * SupermodelDebugger.h
 */

#ifdef SUPERMODEL_DEBUGGER
#ifndef INCLUDED_SUPERMODELDEBUGGER_H
#define INCLUDED_SUPERMODELDEBUGGER_H

#include "ConsoleDebugger.h"

#include <stdarg.h>

#define MODEL3_STATEFILE_VERSION 0

class CModel3;
class CInputs;
class CInput;

namespace Debugger
{
	class CCPUDebug;

	/*
	 * Console-based debugger used by Supermodel.
	 */
	class CSupermodelDebugger : public CConsoleDebugger
	{
	private:
		::CModel3 *m_model3;
		::CInputs *m_inputs;
		::CLogger *m_logger;

		bool m_loadEmuState;
		bool m_saveEmuState;
		bool m_resetEmu;
		char m_stateFile[255];

		bool InputIsValid(CInput *input);

		void ListInputs();

	protected:
		void AddCPUs();

		void WaitCommand(CCPUDebug *cpu);

		bool ProcessToken(const char *token, const char *cmd);

		void Attached();

		void Detaching();

	public:
		static CCPUDebug *CreateMainBoardCPUDebug(::CModel3 *model3);
		
		static CCPUDebug *CreateSoundBoardCPUDebug(::CModel3 *model3);

		static CCPUDebug *CreateDSBCPUDebug(::CModel3 *model3);

		static CCPUDebug *CreateDriveBoardCPUDebug(::CModel3 *model3);

		CSupermodelDebugger(::CModel3 *model3, ::CInputs *inputs, ::CLogger *logger);

		void Poll();
		
		bool LoadModel3State(const char *fileName);

		bool SaveModel3State(const char *fileName);

		void ResetModel3();

		void DebugLog(const char *fmt, va_list vl);
		
		void InfoLog(const char *fmt, va_list vl);
		
		void ErrorLog(const char *fmt, va_list vl);
	};
}

#endif	// INCLUDED_SUPERMODELDEBUGGER_H
#endif  // SUPERMODEL_DEBUGGER