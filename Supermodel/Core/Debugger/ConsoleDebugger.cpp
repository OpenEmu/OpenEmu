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
 * ConsoleDebugger.cpp
 */

#ifdef SUPERMODEL_DEBUGGER

#include "ConsoleDebugger.h"
#include "CPUDebug.h"
#include "CodeAnalyser.h"
#include "Label.h"

#include <cctype>
#include <string>

namespace Debugger
{
	CConsoleDebugger::CConsoleDebugger() : CDebugger(), 
		m_nextFrame(false), m_listDism(0), m_listMem(0), m_analyseCode(true), 
		m_addrFmt(HexDollar), m_portFmt(Decimal), m_dataFmt(HexDollar),
		m_showLabels(true), m_labelsOverAddr(true), m_showOpCodes(false), m_memBytesPerRow(12), m_file(NULL)
	{
		//
	}

	// TODO - tidy up this function, ie do some proper parsing of commands - it is a mess!
	void CConsoleDebugger::WaitCommand(CCPUDebug *cpu)
	{
		m_cpu = cpu;

		UINT32 pc = m_cpu->pc;
		m_listDism = (m_cpu->instrCount > 0 && pc > 10 * m_cpu->minInstrLen ? pc - 10 * m_cpu->minInstrLen : 0);

		char bpChr;
		char addrStr[20];
		char labelStr[13];
		char opCodes[50];
		int codesLen;
		char mnemonic[255];
		char operands[255];
		char cmd[255];
		
		for (;;)
		{
			// Get code analyser and if available analyse code now if required
			if (m_analyseCode)
			{
				CCodeAnalyser *analyser = m_cpu->GetCodeAnalyser();
				if (analyser->NeedsAnalysis())
				{
					Print("Analysing %s...\n", m_cpu->name);
					analyser->AnalyseCode();
				}
			}
		
			// Close redirected output file, if exists
			if (m_file != NULL)
			{
				fclose(m_file);
				m_file = NULL;
			}

			// Get details for current PC address
			bool hasLabel;
			if (m_cpu->instrCount > 0)
			{
				m_cpu->FormatAddress(addrStr, pc);
				hasLabel = GetLabelText(labelStr, 12, pc);
				codesLen = m_cpu->Disassemble(pc, mnemonic, operands);
				FormatOpCodes(opCodes, pc, abs(codesLen));
			}
			else
			{
				addrStr[0] = '-';
				addrStr[1] = '\0';
				hasLabel = false;
				labelStr[0] = '\0';
				opCodes[0] = '-';
				opCodes[1] = '\0';
				codesLen = 0;
			}
			CBreakpoint *bp = m_cpu->GetBreakpoint(pc);
			bpChr = (bp != NULL ? bp->symbol : ' ');
		
			// Output command prompt
			Print("%s%c", m_cpu->name, bpChr);
			if (m_showLabels)
			{
				if (m_labelsOverAddr)
					Print("%-12s ", (hasLabel ? labelStr : addrStr));
				else
					Print("%s %-12s ", addrStr, labelStr);
			}
			else
				Print("%s ", addrStr);
			if (m_showOpCodes) 
				Print("[%s] ", opCodes);
			if (codesLen > 0)
				Print("%-*s %s > ", (int)m_cpu->maxMnemLen, mnemonic, operands);
			else
				Print("??? > ");
			Flush();

			// Wait for command
			Read(cmd, 255);

			if (cmd[0] == '\0')
			{
				m_cpu->SetStepMode(StepInto);
				break;
			}

			// Check for redirection
			char *pos = strchr(cmd, '>');
			if (pos != NULL)
			{
				*pos = '\0';
				pos++;
				const char *mode;
				if (*pos == '>')
				{
					mode = "ab";
					pos++;
				}
				else
					mode = "w";
				while (*pos == ' ')
					pos++;
				if (*pos != '\0')
				{
					m_file = fopen(pos, mode);
					if (m_file == NULL)
					{
						Error("Unable to direct output to file %s\n", pos);	
						continue;
					}
				}
				else
				{
					Error("Missing file to direct output to\n");
					continue;
				}
			}

			char *token = strtok(cmd, " ");
			if (ProcessToken(token, cmd))
				break;

			pc = m_cpu->pc;
		}
	}

	bool CConsoleDebugger::ProcessToken(const char *token, const char *cmd)
	{
		UINT32 pc = m_cpu->pc;

		int number;
		unsigned size;
		const char *sizeStr;
		UINT32 addr;
		UINT16 portNum;
		UINT64 data;
		char addrStr[50];
		char portNumStr[50];
		char dataStr[50];
		char mod[10];
		EFormat fmt;

		//
		// Execution
		//
		if (CheckToken(token, "n", "next"))							// next [<count>=1]
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token != NULL)
			{
				if (!ParseInt(token, &number) || number <= 0)
				{
					Error("Enter a valid instruction count.\n");
					return false;
				}
				
				if (number > 1)
					Print("Running %d instructions.\n", number);
				m_cpu->SetCount(number);
			}
			else
				m_cpu->SetStepMode(StepInto);
			return true;
		}
		else if (CheckToken(token, "nf", "nextframe"))				// nextframe [<count>=1]
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token != NULL)
			{
				if (!ParseInt(token, &number) || number <= 0)
				{
					Error("Enter a valid frame count.\n");
					return false;
				}
				
				if (number > 1)
					Print("Running %d frames.\n", number);
				m_nextFrameCount = number;
			}
			else
				m_nextFrameCount = 1;
			m_nextFrame = true;
			return true;
		}
		else if (CheckToken(token, "s", "stepover"))				// stepover
		{
			m_cpu->SetStepMode(StepOver);
			return true;
		}
		else if (CheckToken(token, "si", "stepinto"))				// stepinto
		{
			m_cpu->SetStepMode(StepInto);
			return true;
		}
		else if (CheckToken(token, "so", "stepout"))				// stepout
		{
			m_cpu->SetStepMode(StepOut);
			return true;
		}
		else if (CheckToken(token, "c", "continue"))				// continue [<addr>]
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token != NULL)
			{
				if (!ParseAddress(token, &addr))
				{
					Error("Enter a valid address.\n");
					return false;
				}

				m_cpu->FormatAddress(addrStr, addr);
				Print("Continuing until %s.\n", addrStr);
				m_cpu->SetUntil(addr);
			}
			else
				m_cpu->SetContinue();
			return true;
		}
		else if (CheckToken(token, "spc", "setpc"))					// setpc <addr>
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Error("Missing address.\n");
				return false;
			}
			if (!ParseAddress(token, &addr))
			{
				Error("Enter a valid address.\n");
				return false;
			}

			if (!m_cpu->SetPC(addr))
			{
				Error("Unable to set PC.\n");
				return false;
			}

			m_cpu->FormatAddress(addrStr, addr);
			Print("PC set to %s.\n", addrStr);
			return true;
		}
		//
		// CPUs
		//
		else if (CheckToken(token, "lc", "listcpus"))				// listcpus
		{
			ListCPUs();
		}
		else if (CheckToken(token, "sc", "switchcpu"))				// switchcpu (<name>|<num>)
		{	
			// Parse arguments
			token = strtok(NULL, " ");
			CCPUDebug *cpu;
			if (!ParseCPU(token, cpu))
				return false;
			
			if (!cpu->enabled)
			{
				Error("CPU %s is currently disabled for debugging.\n", cpu->name);
				return false;
			}

			m_cpu = cpu;
			pc = cpu->pc;
			m_listDism = (cpu->instrCount > 0 && pc > 10 * cpu->minInstrLen ? pc - 10 * cpu->minInstrLen : 0);
			return false;
		}
		else if (CheckToken(token, "dc", "disablecpu"))				// disablecpu (<name>|<num>)
		{
			// Parse arguments
			token = strtok(NULL, " ");
			CCPUDebug *cpu;
			if (!ParseCPU(token, cpu))
				return false;
			
			if (cpu == m_cpu)
			{
				Error("Cannot enable/disable debugging on current CPU.\n");
				return false;
			}
			cpu->enabled = false;
			Print("Disabled debugging on CPU %s.\n", cpu->name);
		}
		else if (CheckToken(token, "ec", "enablecpu"))				// enablecpu (<name>|<num>)
		{
			// Parse arguments
			token = strtok(NULL, " ");
			CCPUDebug *cpu;
			if (!ParseCPU(token, cpu))
				return false;

			if (cpu == m_cpu)
			{
				Error("Cannot enable/disable debugging on current CPU.\n");
				return false;
			}
			cpu->enabled = true;
			Print("Enabled debugging on CPU %s.\n", cpu->name);
		}
		//
		// Registers
		//
		else if (CheckToken(token, "lr", "listregisters"))			// listregisters
		{
			ListRegisters();
		}
		else if (CheckToken(token, "pr", "printregister"))			// printregister <reg>
		{
			// Parse arguments
			token = strtok(NULL, " ");
			CRegister *reg;
			if (!ParseRegister(token, reg))
				return false;
			
			reg->GetValue(dataStr);
			Print("Register %s = %s\n", reg->name, dataStr);
		}
		else if (CheckToken(token, "sr", "setregister"))			// setregister <reg> <expr>
		{
			// Parse arguments
			token = strtok(NULL, " ");
			CRegister *reg;
			if (!ParseRegister(token, reg))
				return false;
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Error("Missing value to set.\n");
				return false;
			}

			if (!reg->SetValue(token))
			{
				Error("Unable to set value of register %s.\n", reg->name);
				return false;
			}
			reg->GetValue(dataStr);
			Print("Set register %s to %s.\n", reg->name, dataStr);
		}
		else if (CheckToken(token, "lm", "listmonitors"))			// listmonitors
		{
			ListMonitors();
		}	
		else if (CheckToken(token, "m", "monitor") ||				// addmonitor <reg>
			CheckToken(token, "am", "addmonitor"))
		{
			// Parse arguments
			token = strtok(NULL, " ");
			CRegister *reg;
			if (!ParseRegister(token, reg))
				return false;

			m_cpu->AddRegMonitor(reg->name);
			Print("Monitor added to register %s.\n", reg->name);
		}
		else if (CheckToken(token, "rm", "removemonitor"))			// removemonitor <reg>
		{
			// Parse arguments
			token = strtok(NULL, " ");
			CRegister *reg;
			if (!ParseRegister(token, reg))
				return false;

			m_cpu->RemoveRegMonitor(reg->name);
			Print("Monitor for register %s removed.\n", reg->name);
		}
		else if (CheckToken(token, "ram", "removeallmonitors"))     // removeallmonitors
		{
			m_cpu->RemoveAllRegMonitors();
			Print("All register monitors removed.\n");
		}
		//
		// Exceptions & interrupts
		//
		else if (CheckToken(token, "le", "listexceptions"))			// listexceptions
		{
			ListExceptions();
		}
		else if (CheckToken(token, "li", "listinterrupts"))			// listinterrupts
		{
			ListInterrupts();
		}
		else if (CheckToken(token, "t", "trap") ||					// addtrap ((e)xception|(i)nterrupt) <id>
			CheckToken(token, "at", "addtrap"))
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Error("Missing type (e)xception or (i)interrupt\n");
				return false;
			}
			
			if (CheckToken(token, "e", "exception"))
			{
				token = strtok(NULL, " ");
				if (token == NULL)
				{
					Error("Missing exception id.\n");
					return false;
				}
				CException *ex = m_cpu->GetException(token);
				if (ex == NULL)
				{
					Error("Enter a valid exception id.\n");
					return false;
				}

				ex->trap = true;
				Print("Trap added for exceptions of type %s.\n", ex->id);
			}
			else if (CheckToken(token, "i", "interrupt"))
			{
				token = strtok(NULL, " ");
				if (token == NULL)
				{
					Error("Missing interrupt id.\n");
					return false;
				}
				CInterrupt *in = m_cpu->GetInterrupt(token);
				if (in == NULL)
				{
					Error("Enter a valid interrupt id.\n");
					return false;
				}

				in->trap = true;
				Print("Trap added for interrupts of type %s.\n", in->id);
			}
			else
			{
				Error("Enter valid type (e)xception or (i)interrupt.\n");
				return false;
			}
		}
		else if (CheckToken(token, "rt", "removetrap"))				// removetrap ((e)xception|(i)nterrupt) <id>
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Error("Missing type (e)xception or (i)interrupt.\n");
				return false;
			}
			
			if (CheckToken(token, "e", "exception"))
			{
				token = strtok(NULL, " ");
				if (token == NULL)
				{
					Error("Missing exception id.\n");
					return false;
				}
				CException *ex = m_cpu->GetException(token);
				if (ex == NULL)
				{
					Error("Enter a valid exception id.\n");
					return false;
				}

				ex->trap = false;
				Print("Trap for exceptions of type %s removed.\n", ex->id);
			}
			else if (CheckToken(token, "i", "interrupt"))
			{
				token = strtok(NULL, " ");
				if (token == NULL)
				{
					Error("Missing interrupt id.\n");
					return false;
				}
				CInterrupt *in = m_cpu->GetInterrupt(token);
				if (in == NULL)
				{
					Error("Enter a valid interrupt id.\n");
					return false;
				}

				in->trap = false;
				Print("Trap for interrupts ot type %s removed.\n", in->id);
			}
			else
			{
				Error("Enter a valid type (e)xception or (i)interrupt.\n");
				return false;
			}
		}
		else if (CheckToken(token, "rat", "removealltraps"))		// removealltraps [(a)ll|(e)xceptions|(i)nterrupts]
		{
			bool removeExs;
			bool removeInts;
			const char *coverage;
			if (token == NULL || CheckToken(token, "a", "all"))
			{
				removeExs  = true;
				removeInts = true;
				coverage = "exceptions and interrupts";
			}
			else if (CheckToken(token, "e", "exceptions"))
			{
				removeExs = true;
				removeInts = false;
				coverage = "exceptions";
			}
			else if (CheckToken(token, "i", "interrupts"))
			{
				removeExs = false;
				removeInts = true;
				coverage = "interrupts";
			}
			else
			{
				Error("Enter a valid mode (a)ll, (e)xceptions or (i)nterrupts\n");
				return false;
			}

			if (removeExs)
			{	
				for (vector<CException*>::iterator it = m_cpu->exceps.begin(); it != m_cpu->exceps.end(); it++)
					(*it)->trap = false;
			}
			if (removeInts)
			{
				for (vector<CInterrupt*>::iterator it = m_cpu->inters.begin(); it != m_cpu->inters.end(); it++)
					(*it)->trap = false;
			}
			Print("All traps for %s removed.\n", coverage);
		}
		//
		// Disassembly, labels & comments
		//
		else if (CheckToken(token, "l", "list") ||					// listdisassembly [<start>=last [#<instrs>=20|<end>]]
			CheckToken(token, "ld", "listdisassembly"))
		{
			// Get start address
			UINT32 start;
			token = strtok(NULL, " ");
			if (token == NULL)
				token = "last";
			if (stricmp(token, "last") == 0)
			{
				// Use end of last listing
				start = m_listDism;
			}
			else if (!ParseAddress(token, &start))
			{
				Error("Enter a valid start address.\n");
				return false;
			}

			// Get end address
			UINT32 end;
			unsigned numInstrs;
			token = strtok(NULL, " ");
			if (token != NULL)
			{
				if (token[0] == '#')
				{
					if (!ParseInt(token + 1, &number) || number <= 0)
					{
						Error("Enter a valid number of instructions.\n");
						return false;
					}
					numInstrs = (unsigned)number;
					end = 0xFFFFFFFF;
				}
				else 
				{
					if (!ParseAddress(token, &end))
					{
						Error("Enter a valid end address.\n");
						return false;
					}
					numInstrs = 0xFFFFFFFF;
				}
			}
			else
			{
				// Default is 20 instructions after start
				end = 0xFFFFFFFF;
				numInstrs = 20;
			}

			// List the disassembled code
			m_listDism = ListDisassembly(start, end, numInstrs);
		}
		else if (CheckToken(token, "ll", "listlabels"))				// listlabels [(d)efault|(c)ustom|(a)utos|(e)ntrypoints|e(x)cephandlers|(i)interhandlers|(j)umptargets|(l)ooppoints]
		{
			// Parse arguments
			token = strtok(NULL, " ");
			bool customLabels;
			ELabelFlags labelFlags;
			if (token == NULL || CheckToken(token, "d", "default"))
			{
				customLabels = true;
				labelFlags = (ELabelFlags)(LFEntryPoint | LFExcepHandler | LFInterHandler | LFSubroutine);
			}
			else if (CheckToken(token, "c", "custom"))
			{
				customLabels = true;
				labelFlags = LFNone;
			}
			else if (CheckToken(token, "a", "autos"))
			{
				customLabels = true;
				labelFlags = (ELabelFlags)(LFEntryPoint | LFExcepHandler | LFInterHandler | LFSubroutine | LFJumpTarget | LFLoopPoint);
			}
			else if (CheckToken(token, "e", "entrypoints"))
			{
				customLabels = false;
				labelFlags = LFEntryPoint;
			}
			else if (CheckToken(token, "x", "excephandlers"))
			{
				customLabels = false;
				labelFlags = LFExcepHandler;
			}
			else if (CheckToken(token, "i", "interhandlers"))
			{
				customLabels = false;
				labelFlags = LFInterHandler;
			}
			else if (CheckToken(token, "s", "subroutines"))
			{
				customLabels = false;
				labelFlags = LFSubroutine;
			}
			else if (CheckToken(token, "j", "jumptargets"))
			{
				customLabels = false;
				labelFlags = LFJumpTarget;
			}
			else if (CheckToken(token, "l", "looppoints"))
			{
				customLabels = false;
				labelFlags = LFLoopPoint;
			}
			else
			{
				Error("Enter a valid filter (a)ll, (c)ustom, (e)ntrypoints, e(x)cephandlers, (i)interhandlers, (j)umptargets or (l)ooppoints.\n");
				return false;
			}

			ListLabels(customLabels, labelFlags);
		}
		else if (CheckToken(token, "al", "addlabel"))				// addlabel <addr> <name>
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Error("Missing label address.\n");
				return false;
			}
			else if (!ParseAddress(token, &addr))
			{
				Error("Enter a valid label address.\n");
				return false;
			}

			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Error("Missing label name.\n");
				return false;
			}
			const char *name = token;
			
			// Add label
			CLabel *label = m_cpu->AddLabel(addr, name);
			m_cpu->FormatAddress(addrStr, label->addr);
			Print("Label %s added at %s.\n", label->name, addrStr);
		}
		else if (CheckToken(token, "rl", "removelabel"))			// removelabel [<name>|<addr>]
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
				token = "-";
			
			if (!ParseAddress(token, &addr))
			{
				Error("Enter a valid label name or address.\n");
				return false;
			}

			CLabel *label = m_cpu->GetLabel(addr);
			if (label == NULL)
			{
				m_cpu->FormatAddress(addrStr, addr);
				Error("No label at %s.\n", addrStr);
				return false;
			}
			
			const char *name = label->name;
			m_cpu->FormatAddress(addrStr, label->addr);
			Print("Custom label '%s' removed at address %s.\n", name, addrStr);
		}
		else if (CheckToken(token, "ral", "removealllabels"))		// removealllabels
		{
			m_cpu->RemoveAllLabels();
			Print("All custom labels removed.\n");
		}
		else if (CheckToken(token, "ac", "addcomment"))				// addcomment <addr> <text...>
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Error("Missing comment address.\n");
				return false;
			}
			else if (!ParseAddress(token, &addr))
			{
				Error("Enter a valid comment address.\n");
				return false;
			}

			char text[255];
			text[0] = '\0';
			token = strtok(NULL, " ");
			while (token != NULL)
			{
				size_t len = strlen(text);
				if (len + strlen(token) > 253)
					break;
				if (len > 0)
					strcat(text, " ");
				strcat(text, token);
				token = strtok(NULL, " ");
			}
			if (text[0] == '\0')
			{
				Error("Missing comment text.\n");
				return false;
			}

			// Add comment
			CComment *comment = m_cpu->AddComment(addr, text);
			m_cpu->FormatAddress(addrStr, comment->addr);
			Print("Comment added at %s.\n", addrStr);
		}
		else if (CheckToken(token, "rc", "removecomment"))			// removecomment [<addr>]
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
				token = "-";
			if (!ParseAddress(token, &addr))
			{
				Error("Enter a valid comment address.\n");
				return false;
			}

			m_cpu->FormatAddress(addrStr, addr);
			if (m_cpu->RemoveComment(addr))
				Print("Comment at address %s removed.\n", addrStr);
			else
				Error("No comment at address %s.\n", addrStr);
		}
		else if (CheckToken(token, "rac", "removeallcomments"))     // removeallcomments
		{
			m_cpu->RemoveAllComments();
			Print("All comments removed.\n");
		}
		//
		// Breakpoints
		//
		else if (CheckToken(token, "lb", "listbreakpoints"))		// listbreakpoints
		{
			ListBreakpoints();
		}
		else if (CheckToken(token, "b", "breakpoint") ||			// addbreakpoint [<addr> [[s)imple|(c)ount <count>)]]
			CheckToken(token, "ab", "addbreakpoint"))
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
				token = "-";
			if (!ParseAddress(token, &addr))
			{
				Error("Enter a valid address.\n");
				return false;
			}
			token = strtok(NULL, " ");
			CBreakpoint *bp;
			if (token == NULL || CheckToken(token, "s", "simple"))
				bp = m_cpu->AddSimpleBreakpoint(addr);
			else if (CheckToken(token, "c", "count"))
			{
				token = strtok(NULL, " ");
				if (token == NULL)
				{
					Error("Missing count.\n");
					return false;
				}
				int count;
				if (!ParseInt(token, &count) || count <= 0)
				{
					Error("Enter a valid count.\n");
					return false;
				}

				bp = m_cpu->AddCountBreakpoint(addr, count);
			}
			else if (CheckToken(token, "p", "print"))
				bp = m_cpu->AddPrintBreakpoint(addr);
			else
			{
				Error("Enter a valid breakpoint type (s)imple or (c)ount.\n");
				return false;
			}

			m_cpu->FormatAddress(addrStr, bp->addr);
			Print("Breakpoint #%d added at address %s.\n", bp->num, addrStr);
		}
		else if (CheckToken(token, "rb", "removebreakpoint"))		// removebreakpoint [#<num>|<addr>]
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
				token = "-";
			if (token[0] == '#')
			{
				// Remove breakpoint by number
				if (!ParseInt(token + 1, &number) || number < 0 || number >= m_cpu->bps.size())
				{
					Error("Enter a valid breakpoint number.\n");
					return false;
				}

				// Remove breakpoint
				m_cpu->RemoveBreakpoint(m_cpu->bps[number]);
				Print("Breakpoint #%d removed.\n", number);
			}
			else
			{
				// Remove breakpoint by address
				if (!ParseAddress(token, &addr))
				{
					Error("Enter a valid address.\n");
					return false;
				}

				// Remove breakpoint
				m_cpu->FormatAddress(addrStr, addr);
				if (m_cpu->RemoveBreakpoint(addr))
					Print("Breakpoint at address %s removed.\n", addrStr);
				else
					Error("No breakpoint at address %s.\n", addrStr);
			}
		}
		else if (CheckToken(token, "rab", "removeallbreakpoints"))  // removeallbreakpoints
		{
			m_cpu->RemoveAllBreakpoints();
			Print("All breakpoints removed.\n");
		}	
		//
		// Memory, I/O & watches
		//
		else if (CheckToken(token, "ln", "listregions"))			// listregions
		{
			ListRegions();
		}
		else if (CheckToken(token, "ly", "listmemory"))				// listmemory [<start>=last [#<rows>=8|<end>]]
		{
			// Get start address
			UINT32 start;
			token = strtok(NULL, " ");
			if (token == NULL)
				token = "last";
			if (stricmp(token, "last") == 0)
			{
				// Use end of last listing
				start = m_listMem;
			}
			else if (!ParseAddress(token, &start))
			{
				Error("Enter a valid start address.\n");
				return false;
			}
	
			// Get end address
			UINT32 end;
			token = strtok(NULL, " ");
			if (token != NULL)
			{
				if (token[0] == '#')
				{
					if (!ParseInt(token + 1, &number) || number <= 0)
					{
						Error("Enter a valid number of rows.\n");
						return false;
					}
					end = start + number * m_memBytesPerRow;
				}
				else
				{
					if (!ParseAddress(token, &end))
					{
						Error("Enter a valid end address.\n");
						return false;
					}
				}
			}
			else
				// Default is 8 rows after start
				end = start + 8 * m_memBytesPerRow;

			// List the memory
			m_listMem = ListMemory(start, end, m_memBytesPerRow);
		}
		else if (CheckToken(token, "py", "printmemory", mod, 9, "b")) // printmemory[.<size>=b] <addr> [(h)ex|hexdo(l)lar|hex(p)osth|(d)ecimal|(b)inary]
		{
			// Parse arguments
			if (!ParseDataSize(mod, size))
				return false;
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Error("Missing address.\n");
				return false;
			}
			if (!ParseAddress(token, &addr))
			{
				Error("Enter a valid address.\n");
				return false;
			}
			token = strtok(NULL, " ");
			if (token != NULL)
			{
				if (!ParseFormat(token, fmt))
					return false;
			}
			else
				fmt = m_dataFmt;

			// Read and print memory
			char uSizeStr[12];
			sizeStr = GetDataSizeStr(size, false);
			UpperFirst(uSizeStr, sizeStr);
			data = m_cpu->ReadMem(addr, size);
			FormatData(dataStr, fmt, size, data);
			m_cpu->FormatAddress(addrStr, addr);
			Print("%s data at %s = %s.\n", uSizeStr, addrStr, dataStr);
		}
		else if (CheckToken(token, "sy", "setmemory", mod, 9, "b"))	// setmemory[.<size>=b] <addr> <value>
		{
			// Parse arguments
			if (!ParseDataSize(mod, size))
				return false;
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Error("Missing address.\n");
				return false;
			}
			if (!ParseAddress(token, &addr))
			{
				Error("Enter a valid address.\n");
				return false;
			}
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Error("Missing value to set.\n");
				return false;
			}
			sizeStr = GetDataSizeStr(size, false);
			if (!m_cpu->ParseData(token, size, &data))
			{
				Error("Enter a valid %s value.\n", sizeStr);
				return false;
			}	
			
			// Set memory
			char uSizeStr[12];
			UpperFirst(uSizeStr, sizeStr);
			m_cpu->WriteMem(addr, size, data);
			m_cpu->FormatData(dataStr, size, data);
			m_cpu->FormatAddress(addrStr, addr);
			Print("Set %s data at %s to %s.\n", addrStr, dataStr);
		}
		else if (CheckToken(token, "lo", "listios"))				// listios
		{
			ListIOs();
		}		
		else if (CheckToken(token, "lw", "listmemwatches"))			// listmemwatches
		{
			ListMemWatches();
		}
		else if (CheckToken(token, "w", "memwatch", mod, 9, "b") ||	// addmemwatch[.<size>=b] <addr> [((n)one|(r)ead|(w)rite|(rw)eadwrite) [((s)imple|(c)ount <count>|(m)atch <sequence>|captu(r)e <maxlen>|(p)rint)]]
			CheckToken(token, "aw", "addmemwatch", mod, 9, "b"))
		{
			// Parse arguments
			if (!ParseDataSize(mod, size))
				return false;
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Error("Missing address.\n");
				return false;
			}
			if (!ParseAddress(token, &addr))
			{
				Error("Enter a valid address.\n");
				return false;
			}
			token = strtok(NULL, " ");
			bool read;
			bool write;
			if (token == NULL || CheckToken(token, "n", "none"))
			{
				read = false;
				write = false;
			}
			else if (CheckToken(token, "r", "read"))
			{
				read = true;
				write = false;
			}
			else if (CheckToken(token, "w", "write"))
			{
				read = false;
				write = true;
			}
			else if (CheckToken(token, "rw", "read/write") || CheckToken(token, "wr", "write/read"))
			{
				read = true;
				write = true;
			}
			else
			{
				Error("Enter valid read/write flags (n)one, (r)ead, (w)rite or (rw)ead/write.\n");
				return false;
			}

			// Add mem watch
			CWatch *watch;
			token = strtok(NULL, " ");
			if (token == NULL || CheckToken(token, "s", "simple"))
				watch = m_cpu->AddSimpleMemWatch(addr, size, read, write);
			else if (CheckToken(token, "c", "count"))
			{
				token = strtok(NULL, " ");
				if (token == NULL)
				{
					Error("Missing count.\n");
					return false;
				}
				int count;
				if (!ParseInt(token, &count) || count <= 0)
				{
					Error("Enter a valid count.\n");
					return false;
				}
				watch = m_cpu->AddCountMemWatch(addr, size, read, write, count);
			}
			else if (CheckToken(token, "m", "match"))
			{
				vector<UINT64> dataSeq;
				while ((token = strtok(NULL, " ")) != NULL)
				{
					if (m_cpu->ParseData(token, size, &data))
						dataSeq.push_back(data);
				}
				if (dataSeq.size() == 0)
				{
					sizeStr = GetDataSizeStr(size, false);
					Error("Enter a sequence of %s data to match.", sizeStr);
					return false;
				}
				watch = m_cpu->AddMatchMemWatch(addr, size, read, write, dataSeq);
			}
			else if (CheckToken(token, "r", "capture"))
			{
				token = strtok(NULL, " ");
				if (token == NULL)
				{
					Error("Missing maximum capture length.\n");
					return false;
				}
				int maxLen;
				if (!ParseInt(token, &maxLen) || maxLen <= 0)
				{
					Error("Enter a valid maximum capture length.\n");
					return false;
				}
				watch = m_cpu->AddCaptureMemWatch(addr, size, read, write, maxLen);
			}
			else if (CheckToken(token, "p", "print"))
				watch = m_cpu->AddPrintMemWatch(addr, size, read, write);
			else
			{
				Error("Enter a valid watch type (s)imple, (c)ount, (m)atch, captu(r)e or (p)rint.\n");
				return false;
			}

			m_cpu->FormatAddress(addrStr, watch->addr);
			number = GetIndexOfMemWatch(watch);
			Print("Memory watch #%d added at address %s.\n", number, addrStr);
		}
		else if (CheckToken(token, "rw", "removememwatch"))			// removememwatch (#<num>|<addr>)
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Error("Missing watch number or address.\n");
				return false;
			}
			if (token[0] == '#')
			{
				// Remove watch by number
				vector<CWatch*> watches;
				GetAllMemWatches(watches);
				if (!ParseInt(token + 1, &number) || number < 0 || number >= watches.size())
				{
					Error("Enter a valid watch number.\n");
					return false;
				}

				// Remove watch
				m_cpu->RemoveWatch(watches[number]);
				Print("Memory watch #%d removed.\n", number);
			}
			else
			{
				// Remove watch by address
				if (!ParseAddress(token, &addr))
				{
					Error("Enter a valid address.\n");
					return false;
				}

				// Remove watch
				m_cpu->FormatAddress(addrStr, addr);
				if (m_cpu->RemoveMemWatch(addr, 1))
					Print("Memory watch at address %s removed.\n", addrStr);
				else
					Error("No memory watch at address %s.\n", addrStr);
			}
		}
		else if (CheckToken(token, "raw", "removeallmemwatches"))	// removeallmemwatches
		{
			// Remove all memory watches
			vector<CWatch*> watches;
			GetAllMemWatches(watches);
			for (vector<CWatch*>::iterator it = watches.begin(); it != watches.end(); it++)
				m_cpu->RemoveWatch(*it);
			Print("All memory watches removed.\n");
		}
		else if (CheckToken(token, "lpw", "listportwatches"))		// listportwatches
		{
			ListPortWatches();
		}
		else if (CheckToken(token, "pw", "portwatch") ||			// addportwatch <port> [((n)one|(i)nput|(o)|(io)nputoutput) [((s)imple|(c)ount <count>|(m)atch <sequence>|captu(r)e <maxlen>|(p)rint)]]
			CheckToken(token, "apw", "addportwatch"))
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Error("Missing port number.\n");
				return false;
			}
			if (!m_cpu->ParsePortNum(token, &portNum))
			{
				Error("Enter a valid port number.\n");
				return false;
			}
			token = strtok(NULL, " ");
			bool input;
			bool output;
			if (token == NULL || CheckToken(token, "n", "none"))
			{
				input = false;
				output = false;
			}
			else if (CheckToken(token, "i", "input"))
			{
				input = true;
				output = false;
			}
			else if (CheckToken(token, "o", "output"))
			{
				input = false;
				output = true;
			}
			else if (CheckToken(token, "io", "input/output") || CheckToken(token, "oi", "output/input"))
			{
				input = true;
				output = true;
			}
			else
			{
				Error("Enter valid input/output flags (n)one, (i)nput, (o)utput or (io)nput/output.\n");
				return false;
			}

			// Add watch
			CPortIO *port = m_cpu->GetPortIO(portNum);
			CWatch *watch;
			token = strtok(NULL, " ");
			if (token == NULL || CheckToken(token, "s", "simple"))
				watch = port->AddSimpleWatch(input, output);
			else if (CheckToken(token, "c", "count"))
			{
				token = strtok(NULL, " ");
				if (token == NULL)
				{
					Error("Missing count.\n");
					return false;
				}
				int count;
				if (!ParseInt(token, &count) || count <= 0)
				{
					Error("Enter a valid count.\n");
					return false;
				}
				watch = port->AddCountWatch(input, output, count);
			}
			else if (CheckToken(token, "m", "match"))
			{
				vector<UINT64> dataSeq;
				while ((token = strtok(NULL, " ")) != NULL)
				{
					if (m_cpu->ParseData(token, port->dataSize, &data))
						dataSeq.push_back(data);
				}
				if (dataSeq.size() == 0)
				{
					Error("Enter a sequence of %s to match.", GetDataSizeStr(port->dataSize, false));
					return false;
				}
				watch = port->AddMatchWatch(input, output, dataSeq);
			}
			else if (CheckToken(token, "r", "capture"))
			{
				token = strtok(NULL, " ");
				if (token == NULL)
				{
					Error("Missing maximum capture length.\n");
					return false;
				}
				int maxLen;
				if (!ParseInt(token, &maxLen) || maxLen <= 0)
				{
					Error("Enter a valid maximum capture length.\n");
					return false;
				}
				watch = port->AddCaptureWatch(input, output, maxLen);
			}
			else if (CheckToken(token, "p", "print"))
				watch = port->AddPrintWatch(input, output);
			else
			{
				Error("Enter a valid watch type (s)imple, (c)ount, (m)atch, captu(r)e or (p)rint.\n");
				return false;
			}
			
			m_cpu->FormatPortNum(portNumStr, portNum);
			number = GetIndexOfPortWatch(watch);
			Print("Port watch %d added for port %u.\n", number, portNumStr);
		}
		else if (CheckToken(token, "rpw", "removeportwatch"))		// removeportwatch (a|n|p) [NUM|PORT]
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Error("Missing watch number or port number.\n");
				return false;
			}
			if (token[0] == '#')
			{
				// Remove watch by number
				vector<CWatch*> watches;
				GetAllPortWatches(watches);
				if (!ParseInt(token + 1, &number) || number < 0 || number >= watches.size())
				{
					Error("Enter a valid watch number.\n");
					return false;
				}

				// Remove watch
				m_cpu->RemoveWatch(watches[number]);
				Print("Port watch #%d removed.\n", number);
			}
			else
			{
				// Remove watch by port number
				if (!m_cpu->ParsePortNum(token, &portNum))
				{
					Error("Enter a valid port number.\n");
					return false;
				}

				// Remove watch
				CPortIO *port = m_cpu->GetPortIO(portNum);
				m_cpu->FormatPortNum(portNumStr, portNum);
				if (port->RemoveWatch())
					Print("Port watch for port %s removed.\n", portNumStr);
				else
					Error("No port watch for port %s.\n", portNumStr);
			}
		}
		else if (CheckToken(token, "rapw", "removeallportwatches")) // removeallportwatches
		{
			// Remove all port watches
			vector<CWatch*> watches;
			GetAllPortWatches(watches);
			for (vector<CWatch*>::iterator it = watches.begin(); it != watches.end(); it++)
				m_cpu->RemoveWatch(*it);
			Print("All port watches removed.\n");
		}
		//
		// General
		//		
		else if (CheckToken(token, "p", "print", mod, 9, ""))		// print[.<size>=v] <expr> [(h)ex|hexdo(l)lar|hex(p)osth|(d)ecimal|(b)inary]
		{
			// Parse arguments
			bool useDefSize;
			if (mod[0] == '\0')
				useDefSize = true;
			else if (ParseDataSize(mod, size))
				useDefSize = false;
			else
				return false;
			token = strtok(NULL, " ");
			if (token == NULL)
			{	
				Error("Missing expression.\n");
				return false;
			}
			const char *expr = token;
			token = strtok(NULL, " ");
			if (token != NULL)
			{
				if (!ParseFormat(token, fmt))
					return false;
			}
			else
				fmt = m_dataFmt;

			// Check labels and registers
			CLabel *label = m_cpu->GetLabel(expr);
			if (label != NULL)
			{
				data = label->addr;
				if (useDefSize)
					size = m_cpu->memBusWidth / 8;
			}
			else
			{
				CCodeAnalyser *analyser = m_cpu->GetCodeAnalyser();
				CAutoLabel *autoLabel = analyser->analysis->GetAutoLabel(expr);
				if (autoLabel != NULL)
				{
					data = autoLabel->addr;
					if (useDefSize)
						size = m_cpu->memBusWidth / 8;
				}
				else
				{
					CRegister *reg = m_cpu->GetRegister(expr);
					if (reg != NULL)
					{
						data = reg->GetValueAsInt();
						if (useDefSize)
							size = reg->dataWidth / 8;
					}
					else
					{
						if (!ParseData(expr, m_dataFmt, 8, &data))
						{
							Print("Unable to parse expression %s\n", expr);
							return false;
						}
						if (useDefSize)
							size = 8;
					}
				}
			}
			
			char result[255];
			FormatData(result, fmt, size, data);
			if (useDefSize)
				Print("%s = %s\n", expr, result);
			else
			{
				sizeStr = GetDataSizeStr(size, true);
				Print("%s = %s.%s\n", expr, result, sizeStr);
			}
		}
		else if (CheckToken(token, "cfg", "configure"))				// configure <options...>
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				// If no arguments, then print out current configuration
				Print("Configuration:\n");
				Print(" %-20s %-12s %s\n", "Code Analysis",     (m_analyseCode ? "On" : "Off"), "(a)nalysis"); 
				Print(" %-20s %-12s %s\n", "Address Format",    GetFmtConfig(m_addrFmt),        "a(d)dressfmt"); 
				Print(" %-20s %-12s %s\n", "Port Format",       GetFmtConfig(m_portFmt),        "(p)ortfmt"); 
				Print(" %-20s %-12s %s\n", "Data Format",       GetFmtConfig(m_dataFmt),        "da(t)afmt");
				Print(" %-20s %-12s %s\n", "Show Labels",       (m_showLabels  ? "On" : "Off"), "show(l)abels");
				Print(" %-20s %-12s %s\n", "Show Opcodes",      (m_showOpCodes ? "On" : "Off"), "show(o)pcodes");
				Print(" %-20s %-12u %s\n", "Mem Bytes Per Row", m_memBytesPerRow,               "mem(b)ytesrow");
				return false;
			}

			if (CheckToken(token, "a", "analysis"))
			{
				token = strtok(NULL, " ");
				SetBoolConfig(token, m_analyseCode);
			}
			else if (CheckToken(token, "d", "addressfmt"))
			{
				token = strtok(NULL, " ");
				SetFmtConfig(token, m_addrFmt);
			}
			else if (CheckToken(token, "p", "portfmt"))
			{
				token = strtok(NULL, " ");
				SetFmtConfig(token, m_portFmt);
			}
			else if (CheckToken(token, "t", "datafmt"))
			{            
				token = strtok(NULL, " ");
				SetFmtConfig(token, m_dataFmt);
			}
			else if (CheckToken(token, "l", "showlabels"))
			{
				token = strtok(NULL, " ");
				SetBoolConfig(token, m_showLabels);
			}
			else if (CheckToken(token, "o", "showopcodes"))
			{
				token = strtok(NULL, " ");
				SetBoolConfig(token, m_showOpCodes);
			}
			else if (CheckToken(token, "b", "membytesrow"))
			{
				token = strtok(NULL, " ");
				SetNumConfig(token, m_memBytesPerRow);
			}
			else
			{
				Error("Enter a valid option (a)nalysis, a(d)dressfmt, (p)ortfmt, da(t)afmt, show(l)abels, show(o)pcodes, mem(b)ytesrow.\n");
				return false;
			}
		}
		else if (CheckToken(token, "ls", "loadstate"))				// loadstate <filename>
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Error("Missing filename.\n");
				return false;
			}

			if (LoadState(token))
				Print("Debugger state successfully loaded from <%s>\n", token);
			else
				Error("Unable to load debugger state from <%s>\n", token);
		}
		else if (CheckToken(token, "ss", "savestate"))				// savestate <filename>
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Error("Missing filename.\n");
				return false;
			}

			if (SaveState(token))
				Print("Debugger state successfully saved to <%s>\n", token);
			else
				Error("Unable to save debugger state to <%s>\n", token);
		}
		else if (CheckToken(token, "h", "help"))					// help
		{
			// TODO - improve the following
			const char *fmt = "  %-6s %-25s %s\n";

			Print("Debugger Commands:\n");
			
			Print(" Execution:\n");
			Print(fmt, "n",      "next",                   "[<count>=1]");
			Print(fmt, "nf",     "nextframe",              "[<count>=1]");
			Print(fmt, "s",      "stepover",               "");
			Print(fmt, "si",     "stepinto",               "");
			Print(fmt, "so",     "stepout",                "");
			Print(fmt, "c",      "continue",               "[<addr>]");
			Print(fmt, "spc",    "setpc",                  "<addr>");                   

			Print(" CPUs:\n");
			Print(fmt, "lc",     "listcpus",               "");
			Print(fmt, "sc",     "switchcpu",              "(<name>|<num>)");
			Print(fmt, "dc",     "disablecpu",             "(<name>|<num>)");
			Print(fmt, "ec",     "enablecpu",              "(<name>|<num>)");
			
			Print(" Registers:\n");
			Print(fmt, "lr",     "listregisters",          "");
			Print(fmt, "pr",     "printregister",          "<reg>");
			Print(fmt, "sr",     "setregister",            "<reg> <value>");
			Print(fmt, "lm",     "listmonitors",           "");
			Print(fmt, "m/am",   "addmonitor",             "<reg>");
			Print(fmt, "rm",     "removemonitor",          "<reg>");
			Print(fmt, "ram",    "removeallmonitors",      "");
			
			Print(" Exceptions & interrupts:\n");
			Print(fmt, "le",     "listexceptions",         "");
			Print(fmt, "li",     "listinterrupts",         "");
			Print(fmt, "t/at",   "addtrap",		           "((e)xception|(i)nterrupt) <id>");
			Print(fmt, "rt",     "removetrap",             "((e)xception|(i)nterrupt) <id>");
			Print(fmt, "rat",    "removealltraps",         "[(a)ll|(e)xceptions|(i)nterrupts]");

			Print(" Disassembly, labels & comments:\n");
			Print(fmt, "l/ld",   "listdisassembly",        "[<start>=last [#<instrs>=20|<end>]]");
			Print(fmt, "ll",     "listlabels",             "[(d)efault|(c)ustom|(a)utos|(e)ntrypoints|e(x)cephandlers|(i)interhandlers|(j)umptargets|(l)ooppoints]");
			Print(fmt, "al",     "addlabel",               "<addr> <name>");
			Print(fmt, "rl",     "removelabel",            "[<name>|<addr>]");
			Print(fmt, "ral",    "removealllabels",        "");
			Print(fmt, "ac",     "addcomment",             "<addr> <text...>");
			Print(fmt, "rc",     "removecomment",          "[<addr>]");
			Print(fmt, "rac",    "removeallcomments",      "");

			Print(" Breakpoints:\n");
			Print(fmt, "lb",     "listbreakpoints",        "");
			Print(fmt, "b/ab",   "addbreakpoint",          "[<addr> [[s)imple|(c)ount <count>)]]");
			Print(fmt, "rb",     "removebreakpoint",       "[#<num>|<addr>]");
			Print(fmt, "rab",    "removeallbreakpoints",   "");
			
			Print(" Memory, I/O & watches:\n");
			Print(fmt, "ln",     "listregions",            "");
			Print(fmt, "ly",     "listmemory",             "[<start>=last [#<rows>=8|<end>]]");
			Print(fmt, "py",     "printmemory[.<size>=b]", "<addr>");
			Print(fmt, "sy",     "setmemory[.<size>=b]",   "<addr> <value>");
			Print(fmt, "lo",     "listios",                "");
			Print(fmt, "lw",     "listmemwatches",         "");
			Print(fmt, "w/aw",   "addmemwatch[.<size>=b]", "<addr> [((n)one|(r)ead|(w)rite|(rw)eadwrite) [((s)imple|(c)ount <count>|(m)atch <sequence>|captu(r)e <maxlen>|(p)rint)]]");
			Print(fmt, "rw",     "removememwatch",         "(#<num>|<addr>)");
			Print(fmt, "raw",    "removeallmemwatches",    "");
			Print(fmt, "lpw",    "listportwatches",        "");
			Print(fmt, "pw/apw", "addportwatch",           "<port> [((n)one|(i)nput|(o)utput|(io)nputoutput) [(s)imple|(c)ount <count>|(m)atch <sequence>|captu(r)e <maxlen>|(p)rint]]");
			Print(fmt, "rpw",    "removeportwatch",        "(#<num>|<port>)");
			Print(fmt, "rapw",   "removeallportwatches",   "");
			
			Print("General:\n");
			Print(fmt, "p",      "print[.<size>=v]",       "<expr> [(h)ex|hexdo(l)lar|hex(p)osth|(d)ecimal|(b)inary]");
			Print(fmt, "cfg",	 "configure",			   "<options...>");
			Print(fmt, "ls",     "loststate",              "<filename>");
			Print(fmt, "ss",     "savestate",              "<filename>");
			Print(fmt, "h",      "help",                   "");
			Print(fmt, "x",      "exit",                   "");
		}
		else if (CheckToken(token, "x", "exit"))					// exit
		{
			Print("Exiting...\n");
			SetExit();
			return true;
		}
		else
			Print("Unknown command '%s'.\n", token);
		return false;
	}

	void CConsoleDebugger::Read(char *str, size_t maxSize)
	{
		if (fgets(str, maxSize, stdin) != NULL)
		{
			char *pos = strchr(str, '\n');
			if (pos)
				*pos = '\0';
		}
		else
			str[0] = '\0';
	}

	void CConsoleDebugger::Print(const char *fmtStr, ...)
	{
		va_list vl;
		va_start(vl, fmtStr);
		PrintVL(fmtStr, vl);
		va_end(vl);
	}

	void CConsoleDebugger::Error(const char *fmtStr, ...)
	{
		// Don't log errors to file
		va_list vl;
		va_start(vl, fmtStr);
		vprintf(fmtStr, vl);	
		va_end(vl);
	}

	void CConsoleDebugger::PrintVL(const char *fmtStr, va_list vl)
	{
		if (m_file != NULL)
			vfprintf(m_file, fmtStr, vl);
		else
			vprintf(fmtStr, vl);	
	}

	void CConsoleDebugger::Flush()
	{
		fflush(stdout);
	}

	bool CConsoleDebugger::CheckToken(const char *token, const char *simple, const char *full)
	{
		return stricmp(token, simple) == 0 || stricmp(token, full) == 0;
	}

	bool CConsoleDebugger::CheckToken(const char *token, const char *simple, const char *full, char *modifier, size_t modSize, const char *defaultMod)
	{
		const char *pos = strchr(token, '.');
		if (pos == NULL)
		{
			strncpy(modifier, defaultMod, modSize);
			modifier[modSize] = '\0';
			return CheckToken(token, simple, full);
		}
		else
		{
			pos++;
			if (pos == '\0')
				return false;
			strncpy(modifier, pos, modSize);
			modifier[modSize] = '\0';
			
			char actual[255];
			size_t actSize = min<size_t>(pos - token - 1, 254);
			strncpy(actual, token, actSize);
			actual[actSize] = '\0';
			return CheckToken(actual, simple, full);
		}
	}

	void CConsoleDebugger::Truncate(char *dst, size_t maxLen, const char *src)
	{
		strncpy(dst, src, maxLen);
		if (strlen(src) > maxLen)
			strncpy(&dst[maxLen - 3], "...", 3);
		dst[maxLen] = '\0';
	}

	void CConsoleDebugger::UpperFirst(char *dst, const char *src)
	{
		if (*src != '\0')
		{
			*dst++ = toupper(*src++);
			while (*src != '\0')
				*dst++ = *src++;
		}
		*dst = '\0';
	}

	void CConsoleDebugger::FormatOpCodes(char *str, int addr, int codesLen)
	{
		char *p = str;
		int i = 0;
		while (i < codesLen)
		{
			UINT8 opCode = (UINT8)m_cpu->ReadMem(addr++, 1);
			sprintf(p, "%02X", opCode);
			p += 2;
			i++;
		}
		while (i++ < m_cpu->maxInstrLen)
		{
			*p++ = ' ';
			*p++ = ' ';
		}
		*p = '\0';
	}

	bool CConsoleDebugger::GetLabelText(char *str, int maxLen, UINT32 addr)
	{
		char labelStr[255];
				
		CLabel *label = m_cpu->GetLabel(addr);
		if (label != NULL)
		{
			Truncate(str, maxLen, label->name);
			return true;
		}

		if (m_analyseCode)
		{
			CCodeAnalyser *analyser = m_cpu->GetCodeAnalyser();
			CAutoLabel *autoLabel = analyser->analysis->GetAutoLabel(addr);
			if (autoLabel != NULL && autoLabel->GetLabel(labelStr))
			{
				Truncate(str, maxLen, labelStr);		
				return true;
			}
		}
		
		str[0] = '\0';
		return false;
	}

	bool CConsoleDebugger::SetBoolConfig(const char *str, bool &cfg)
	{
		if (str == NULL)
		{
			Print("Current setting: %-12s\n", (cfg ? "On" : "Off"));
			Print("Change setting with (o)n, o(f)f.\n");
			return false;
		}

		if (CheckToken(str, "o", "on"))
		{
			cfg = true;
			Print("Changed setting: On\n");
			ApplyConfig();
			return true;
		}
		else if (CheckToken(str, "f", "off"))
		{
			cfg = false;
			Print("Changed setting: Off\n");
			ApplyConfig();
			return true;
		}
		else
		{
			Error("Enter a valid setting (o)n, o(f)f.\n");
			return false;
		}
	}

	bool CConsoleDebugger::SetNumConfig(const char *str, unsigned &cfg)
	{
		if (str == NULL)
		{
			Print("Current setting: %-12u\n", cfg);
			return false;
		}

		int number;
		if (!ParseInt(str, &number))
		{
			Error("Enter a valid number.\n");
			return false;
		}

		cfg = (unsigned)number;
		Print("Changed setting: %-12u\n", cfg);
		ApplyConfig();
		return true;
	}

	const char *CConsoleDebugger::GetFmtConfig(EFormat fmt)
	{
		switch (fmt)
		{
			case Hex0x:     return "Hex (0x00)";
			case HexDollar: return "Hex ($00)";
			case HexPostH:  return "Hex (00h)";
			case Decimal:   return "Decimal";
			case Binary:    return "Binary";
			default:        return "-";
		}
	}

	bool CConsoleDebugger::SetFmtConfig(const char *str, EFormat &cfg)
	{
		if (str == NULL)
		{
			Print("Current setting: %-12s\n", GetFmtConfig(cfg));
			Print("Change setting with (h)ex, hex(z)ero, hexdo(l)ar, hex(p)osth, (d)ecimal, (b)inary.\n");
			return false;
		}
	
		if (!ParseFormat(str, cfg))
			return false;

		Print("Changed setting: %-12s\n", GetFmtConfig(cfg));
		ApplyConfig();
		return true;
	}

	bool CConsoleDebugger::ParseAddress(const char *str, UINT32 *addr)
	{
		if (m_cpu->instrCount > 0 && CheckToken(str, "-", "current"))
		{
			*addr = m_cpu->pc;
			return true;
		}
		return m_cpu->ParseAddress(str, addr);
	}

	const char *CConsoleDebugger::GetDataSizeStr(unsigned dataSize, bool shortName)
	{
		switch (dataSize)
		{
			case 1:  return (shortName ? "b" : "byte");
			case 2:  return (shortName ? "w" : "word");
			case 4:  return (shortName ? "l" : "long");
			case 8:  return (shortName ? "v" : "vlong");
			default: return GetSizeString(dataSize);
		}
	}

	bool CConsoleDebugger::ParseDataSize(const char *str, unsigned &dataSize)
	{
		int num = -1;
		ParseInt(str, &num);
		if (CheckToken(str, "b", "byte") || num == 1)
		{
			dataSize = 1;
			return true;
		}
		else if (CheckToken(str, "w", "word") || num == 2)
		{
			dataSize = 2;
			return true;
		}
		else if (CheckToken(str, "l", "long") || num == 4)
		{
			dataSize = 4;
			return true;
		}
		else if (CheckToken(str, "v", "verylong") || num == 8)
		{
			dataSize = 8;
			return true;
		}
		else
		{
			Error("Enter a valid size (b)yte, (w)ord, (l)ong or (v)erylong or a number 1, 2, 4 or 8.\n");
			return false;
		}
	}

	bool CConsoleDebugger::ParseFormat(const char *str, EFormat &fmt)
	{
		if      (CheckToken(str, "h", "hex"))       fmt = Hex;
		else if (CheckToken(str, "z", "hexzero"))   fmt = Hex0x;
		else if (CheckToken(str, "l", "hexdollar")) fmt = HexDollar;	
		else if (CheckToken(str, "p", "hexposth"))  fmt = HexPostH;
		else if (CheckToken(str, "d", "decimal"))   fmt = Decimal;
		else if (CheckToken(str, "b", "binary"))    fmt = Binary;
		else
		{
			Error("Enter a valid format (h)ex, hex(z)ero, hexdo(l)ar, hex(p)osth, (d)ecimal, (b)inary.\n");
			return false;
		}
		return true;
	}

	bool CConsoleDebugger::ParseCPU(const char *str, CCPUDebug *&cpu)
	{
		if (str == NULL)
		{
			Error("Missing CPU name or number.\n");
			return false;
		}
		int cpuNum;
		if (ParseInt(str, &cpuNum))
		{
			if (cpuNum >= 0 && cpuNum < (int)cpus.size())
			{
				cpu = cpus[cpuNum];
				return true;
			}
		}
		cpu = GetCPU(str);
		if (cpu == NULL)
		{
			Error("No CPU with that name or number.\n");
			return false;
		}
		return true;
	}

	bool CConsoleDebugger::ParseRegister(const char *str, CRegister *&reg)
	{
		if (str == NULL)
		{
			Error("Missing register name.\n");
			return false;
		}
		reg = m_cpu->GetRegister(str);
		if (reg == NULL)
		{
			Error("Enter a valid register name.\n");
			return false;
		}
		return true;
	}

	void CConsoleDebugger::ListCPUs()
	{
		Print("CPUs:\n");
		if (cpus.size() == 0)
		{
			Print(" None\n");
			return;
		}

		Print(" %-3s %-9s %-6s %-9s %-7s %-12s %-12s %-9s\n", "Num", "Name", "Type", "Debugging", "State", "Instr Count", "Total Cycles", "Frequency"); 
		unsigned num = 0;
		double freq;
		char onCPU;
		const char *debugStr;
		const char *stateStr;
		for (vector<CCPUDebug*>::iterator it = cpus.begin(); it != cpus.end(); it++)
		{
			onCPU = (*it == m_cpu ? '*': ' ');
			debugStr = ((*it)->enabled ? "Enabled" : "Disabled");
			stateStr = ((*it)->active ? "Running" : "Waiting");
			
			if ((*it)->instrCount > 0)
			{
				if (frameCount > 0)
				{
					freq = ((*it)->cyclesPerPoll * 60.0) / 1000000.0;
					Print("%c%-3u %-9s %-6s %-9s %-7s %12llu %12llu %6.1fMHz\n", onCPU, num++, (*it)->name, (*it)->type, debugStr, stateStr, (*it)->instrCount - 1, (*it)->totalCycles, freq);
				}
				else
					Print("%c%-3u %-9s %-6s %-9s %-7s %12llu %12llu %9s\n", onCPU, num++, (*it)->name, (*it)->type, debugStr, stateStr, (*it)->instrCount - 1, (*it)->totalCycles, "-");
			}
			else
				Print("%c%-3u %-9s %-6s %-9s %-7s %12s %12s %9s\n", onCPU, num++, (*it)->name, (*it)->type, debugStr, stateStr, "-", "-", "-");
		}
	}

	void CConsoleDebugger::ListRegisters()
	{
		Print("%s Registers:\n", m_cpu->name);
		if (m_cpu->regs.size() == 0)
		{
			Print(" None\n");
			return;
		}
		
		// Get groups
		vector<const char*> groups;
		vector<vector<CRegister*> > regsByGroup;
		size_t totalRows = 0;
		for (vector<CRegister*>::iterator it = m_cpu->regs.begin(); it != m_cpu->regs.end(); it++)
		{
			const char *group = (*it)->group;
			// TODO - find with stricmp rather than default find
			if (find(groups.begin(), groups.end(), group) == groups.end())
			{
				groups.push_back(group);
				regsByGroup.resize(regsByGroup.size() + 1);
			}
			int index = find(groups.begin(), groups.end(), group) - groups.begin();
			vector<CRegister*> *pRegsInGroup = &regsByGroup[index];
			pRegsInGroup->push_back(*it);
			totalRows = max<size_t>(totalRows, pRegsInGroup->size());
		}

		// Get max label and value widths in each group and print group headers
		size_t numGroups = groups.size();
		vector<size_t> labelWidths(numGroups);
		vector<size_t> valueWidths(numGroups);
		vector<size_t> groupWidths(numGroups);
		char valStr[50];
		Print(" ");
		for (size_t index = 0; index < numGroups; index++)
		{
			labelWidths[index] = 0;
			valueWidths[index] = 0;

			vector<CRegister*> *pRegsInGroup = &regsByGroup[index];
			for (vector<CRegister*>::iterator it = pRegsInGroup->begin(); it != pRegsInGroup->end(); it++)
			{
				labelWidths[index] = max<size_t>(labelWidths[index], strlen((*it)->name));
				(*it)->GetValue(valStr);
				valueWidths[index] = max<size_t>(valueWidths[index], strlen(valStr));
			}

			const char *group = groups[index];
			groupWidths[index] = max<size_t>(labelWidths[index] + valueWidths[index] + 3, strlen(group) + 1);
			Print("%-*s", (int)groupWidths[index], group);
		}
		Print("\n");

		// Print rows of register values
		char rowStr[50];
		for (size_t row = 0; row < totalRows; row++)
		{
			Print(" ");
			for (size_t index = 0; index < numGroups; index++)
			{
				vector<CRegister*> *pRegsInGroup = &regsByGroup[index];
				if (row < pRegsInGroup->size())
				{
					CRegister *reg = (*pRegsInGroup)[row];
					reg->GetValue(valStr);
					bool hasMon = m_cpu->GetRegMonitor(reg->name) != NULL;

					sprintf(rowStr, "%c%-*s %-*s", (hasMon ? '*' : ' '), (int)labelWidths[index], reg->name, (int)valueWidths[index], valStr);
				}
				else
					rowStr[0] = '\0';

				Print("%-*s", (int)groupWidths[index], rowStr);
			}
			Print("\n");
		}
	}	

	void CConsoleDebugger::ListExceptions()
	{
		Print("%s Exceptions:\n", m_cpu->name);
		if (m_cpu->exceps.size() == 0)
		{
			Print(" None\n");
			return;
		}

		char addrStr[20];
		UINT32 handlerAddr;
		for (vector<CException*>::iterator it = m_cpu->exceps.begin(); it != m_cpu->exceps.end(); it++)
		{
			if (m_cpu->GetHandlerAddr(*it, handlerAddr))
				m_cpu->FormatAddress(addrStr, handlerAddr, true, (m_analyseCode ? LFExcepHandler : LFNone));
			else
				addrStr[0] = '\0';
			Print("%c%-12s %-30s %-12s %u\n", ((*it)->trap ? '*' : ' '), (*it)->id, (*it)->name, addrStr, (*it)->count);
		}
	}

	void CConsoleDebugger::ListInterrupts()
	{
		Print("%s Interrupts:\n", m_cpu->name);
		if (m_cpu->inters.size() == 0)
		{
			Print(" None\n");
			return;
		}

		char addrStr[20];
		UINT32 handlerAddr;
		for (vector<CInterrupt*>::iterator it = m_cpu->inters.begin(); it != m_cpu->inters.end(); it++)
		{
			if (m_cpu->GetHandlerAddr(*it, handlerAddr))
				m_cpu->FormatAddress(addrStr, handlerAddr, true, (m_analyseCode ? LFInterHandler : LFNone));
			else
				addrStr[0] = '\0';
			Print("%c%-12s %-30s %-12s %u\n", ((*it)->trap ? '*' : ' '), (*it)->id, (*it)->name, addrStr, (*it)->count);
		}
	}

	void CConsoleDebugger::ListIOs()
	{
		Print("%s I/O Ports:\n", m_cpu->name);
		if (m_cpu->ios.size() == 0)
		{
			Print(" None\n");
			return;
		}

		const char *group = NULL;
		char locStr[255];
		char dirChar;
		char inStr[50];
		char outStr[50];
		for (vector<CIO*>::iterator it = m_cpu->ios.begin(); it != m_cpu->ios.end(); it++)
		{
			// Print group heading if starting a new group
			if (group == NULL || stricmp((*it)->group, group) != 0)
			{
				group = (*it)->group;
				Print(" %s:\n", group);
			}

			// Get location string (memory address or port number)
			(*it)->GetLocation(locStr);
			
			// See whether port was last read or written
			if ((*it)->inCount + (*it)->outCount > 0)
				dirChar = ((*it)->last == &(*it)->lastIn ? '<' : '>');
			else
				dirChar = ' ';
			// Format last input
			if ((*it)->inCount > 0)
				m_cpu->FormatData(inStr, (*it)->dataSize, (*it)->lastIn);
			else
			{
				inStr[0] = '*';
				inStr[1] = '\0';
			}
			size_t inLen = strlen(inStr);
			int inLPad = 5 - (int)inLen / 2;
			int inRPad = 5 - (int)inLen + (int)inLen / 2;
			// Format last output
			if ((*it)->outCount > 0)
				m_cpu->FormatData(outStr, (*it)->dataSize, (*it)->lastOut);
			else
			{
				outStr[0] = '*';
				outStr[1] = '\0';
			}
			size_t outLen = strlen(outStr);
			int outLPad = 5 - (int)outLen / 2;
			int outRPad = 5 - (int)outLen + (int)outLen / 2;
			
			// Print details
			Print(" %c%-12s %-30s %-8s %*s%s%*s%c%*s%s%*s\n", ((*it)->watch != NULL ? '*' : ' '), locStr, (*it)->name, 
				GetDataSizeStr((*it)->dataSize, true), inLPad, "", inStr, inRPad, "", dirChar, outLPad, "", outStr, outRPad, "");
		}
	}

	void CConsoleDebugger::ListRegions()
	{
		Print("%s Regions:\n", m_cpu->name);
		if (m_cpu->regions.size() == 0)
		{
			Print(" None\n");
			return;
		}

		char startStr[255];
		char endStr[255];
		for (vector<CRegion*>::iterator it = m_cpu->regions.begin(); it != m_cpu->regions.end(); it++)
		{
			// Format start and end address
			m_cpu->FormatAddress(startStr, (*it)->addr);
			m_cpu->FormatAddress(endStr, (*it)->addrEnd);

			// Print details
			Print("%c%s-%s %s\n", ((*it)->isCode ? '*' : ' '), startStr, endStr, (*it)->name);
		}
	}

	void CConsoleDebugger::ListLabels(bool customLabels, ELabelFlags autoLabelFlags)
	{
		Print("%s Labels:\n", m_cpu->name);
		
		unsigned count = 0;

		char addrStr[20];
		if (customLabels && m_cpu->labels.size() > 0)
		{
			Print(" Custom Labels:\n");
			for (vector<CLabel*>::iterator it = m_cpu->labels.begin(); it != m_cpu->labels.end(); it++)
			{
				m_cpu->FormatAddress(addrStr, (*it)->addr);
				Print("  %s %s\n", addrStr, (*it)->name);
				count++;
			}
		}

		if (m_analyseCode)
		{
			char labelStr[255];
			CCodeAnalyser *analyser = m_cpu->GetCodeAnalyser();
			for (unsigned index = 0; index < CAutoLabel::numLabelFlags; index++)
			{
				ELabelFlags flag = CAutoLabel::GetLabelFlag(index);
				if (!(autoLabelFlags & flag))
					continue;
				vector<CAutoLabel*> withFlag = analyser->analysis->GetAutoLabels(flag);
				if (withFlag.size() == 0)
					continue;
				const char *flagStr = CAutoLabel::GetFlagString(flag);
				Print(" %ss:\n", flagStr);
				for (vector<CAutoLabel*>::iterator it = withFlag.begin(); it != withFlag.end(); it++)
				{
					if (!(*it)->GetLabel(labelStr, flag))
						continue;
					CBreakpoint *bp = m_cpu->GetBreakpoint((*it)->addr);
					char bpChr = (bp != NULL ? bp->symbol : ' ');
					m_cpu->FormatAddress(addrStr, (*it)->addr);
					Print(" %c%s %s\n", bpChr, addrStr, labelStr);
					count++;
				}
			}
		}

		if (count == 0)
		{
			Print(" None\n");
			return;
		}
	}

	void CConsoleDebugger::GetAllMemWatches(vector<CWatch*> &watches)
	{
		for (vector<CWatch*>::iterator it = m_cpu->memWatches.begin(); it != m_cpu->memWatches.end(); it++)
			watches.push_back(*it);
		for (vector<CWatch*>::iterator it = m_cpu->ioWatches.begin(); it != m_cpu->ioWatches.end(); it++)
		{
			if (dynamic_cast<CMappedIO*>((*it)->io) != NULL)
				watches.push_back(*it);
		}
	}

	int CConsoleDebugger::GetIndexOfMemWatch(CWatch *watch)
	{
		vector<CWatch*> watches;
		GetAllMemWatches(watches);
		vector<CWatch*>::iterator it = find(watches.begin(), watches.end(), watch);
		if (it == watches.end())
			return -1;
		return it - watches.begin();
	}

	void CConsoleDebugger::ListMemWatches()
	{
		Print("%s Memory Watches:\n", m_cpu->name);

		vector<CWatch*> watches;
		GetAllMemWatches(watches);

		if (watches.size() == 0)
		{
			Print(" None\n");
			return;
		}

		Print(" %-3s %-8s %-12s %-5s %-4s %-20s %s\n", "Num", "Type", "Address", "Size", "Trig", "Value", "Info");

		char typeStr[12];
		char addrStr[20];
		char sizeStr[20];
		const char *dSizeStr;
		const char *trigStr;
		char valStr[20];
		char infoStr[255];
		unsigned wNum = 0;
		for (vector<CWatch*>::iterator it = watches.begin(); it != watches.end(); it++)
		{
			UpperFirst(typeStr, (*it)->type);
			m_cpu->FormatAddress(addrStr, (*it)->addr, true);
			dSizeStr = GetDataSizeStr((*it)->size, false);
			UpperFirst(sizeStr, dSizeStr);
			if      ((*it)->trigRead && (*it)->trigWrite) trigStr = "R/W";
			else if ((*it)->trigRead)                     trigStr = "R";
			else if ((*it)->trigWrite)                    trigStr = "W";
			else                                          trigStr = "-";
			m_cpu->FormatData(valStr, (*it)->size, (*it)->GetValue());
			if (!(*it)->GetInfo(infoStr))
			{
				infoStr[0] = '-';
				infoStr[1] = '\0';
			}
			Print(" %-3u %-8s %-12s %-5s %-4s %-20s %s\n", wNum++, typeStr, addrStr, sizeStr, trigStr, valStr, infoStr);
		}
	}

	void CConsoleDebugger::GetAllPortWatches(vector<CWatch*> &watches)
	{
		for (vector<CWatch*>::iterator it = m_cpu->ioWatches.begin(); it != m_cpu->ioWatches.end(); it++)
		{
			if (dynamic_cast<CPortIO*>((*it)->io) != NULL)
				watches.push_back(*it);
		}
	}

	int CConsoleDebugger::GetIndexOfPortWatch(CWatch *watch)
	{
		vector<CWatch*> watches;
		GetAllPortWatches(watches);
		vector<CWatch*>::iterator it = find(watches.begin(), watches.end(), watch);
		if (it == watches.end())
			return -1;
		return it - watches.begin();
	}

	void CConsoleDebugger::ListPortWatches()
	{
		Print("%s I/O Port Watches:\n", m_cpu->name);

		vector<CWatch*> watches;
		GetAllPortWatches(watches);

		if (watches.size() == 0)
		{
			Print(" None\n");
			return;
		}

		Print(" %-3s %-8s %-12s %-4s %-20s %s\n", "Num", "Type", "Location", "Trig", "Last In/Out", "Info");

		char typeStr[12];
		char locStr[255];
		const char *trigStr;
		char valStr[20];
		char infoStr[255];
		unsigned wNum = 0;
		for (vector<CWatch*>::iterator it = watches.begin(); it != watches.end(); it++)
		{
			UpperFirst(typeStr, (*it)->type);
			(*it)->io->GetLocation(locStr);
			if      ((*it)->trigRead && (*it)->trigWrite) trigStr = "I/O";
			else if ((*it)->trigRead)                     trigStr = "I";
			else if ((*it)->trigWrite)                    trigStr = "O";
			else                                          trigStr = "-";
			if ((*it)->readCount + (*it)->writeCount == 0)
			{
				valStr[0] = '-';
				valStr[1] = '\0';
			}
			else
				m_cpu->FormatData(valStr, (*it)->size, (*it)->GetValue());
			if (!(*it)->GetInfo(infoStr))
			{
				infoStr[0] = '-';
				infoStr[1] = '\0';
			}
			Print(" %-3u %-8s %-12s %-4s %-20s %s\n", wNum++, typeStr, locStr, trigStr, valStr, infoStr);
		}
	}

	void CConsoleDebugger::ListBreakpoints()
	{
		Print("%s Breakpoints:\n", m_cpu->name);
		if (m_cpu->bps.size() == 0)
		{
			Print(" None\n");
			return;
		}

		Print(" %-3s %-8s %-12s %-20s\n", "Num", "Type", "Address", "Info");

		char typeStr[12];
		char addrStr[20];
		char infoStr[255];
		for (vector<CBreakpoint*>::iterator it = m_cpu->bps.begin(); it != m_cpu->bps.end(); it++)
		{
			UpperFirst(typeStr, (*it)->type);
			m_cpu->FormatAddress(addrStr, (*it)->addr, true, (m_analyseCode ? LFAll : LFNone));
			if (!(*it)->GetInfo(infoStr))
			{
				infoStr[0] = '-';
				infoStr[1] = '\0';
			}
			Print(" %-3u %-8s %-12s %-20s\n", (*it)->num, typeStr, addrStr, infoStr);
		}
	}

	void CConsoleDebugger::ListMonitors()
	{
		Print("%s Register Monitors:\n", m_cpu->name);
		if (m_cpu->regMons.size() == 0)
		{
			Print(" None\n");
			return;
		}

		char valStr[255];
		int num = 0;
		for (vector<CRegMonitor*>::iterator it = m_cpu->regMons.begin(); it != m_cpu->regMons.end(); it++)
		{
			(*it)->GetBeforeValue(valStr);
			Print(" %d - %s change from: %s\n", num++, (*it)->reg->name, valStr);
		}
	}

	UINT32 CConsoleDebugger::ListDisassembly(UINT32 start, UINT32 end, unsigned numInstrs)
	{
		UINT32 addr;
		char startStr[20];
		char endStr[20];
		char opCodes[50];
		char mnemonic[100];
		char operands[155];
		char addrStr[20];
		char labelStr[13];
		unsigned instr;

		UINT32 pc = m_cpu->pc;		
		CCodeAnalyser *analyser = (m_analyseCode ? m_cpu->GetCodeAnalyser() : NULL);

		// Align address to instruction boundary
		start -= start%m_cpu->minInstrLen;

		if (analyser != NULL)
		{
			// Use code analyser to find valid start address
			if (!analyser->analysis->GetNextValidAddr(start))
				return start;
		}
		else
		{
			// In the absence of code analyser, try to align code with current PC address
			if (m_cpu->instrCount > 0 && pc >= start && pc <= end)
			{
				unsigned count = m_cpu->instrCount;
				while (start < end && count-- > 0)
				{	
					bool okay = false;
					addr = start;
					instr = 0;
					while (addr < end && instr < numInstrs)
					{
						if (addr == pc)
						{
							okay = true;
							break;
						}
						int codesLen = m_cpu->GetOpLength(addr);
						addr += abs(codesLen);
						instr++;
					}
					if (okay)
						break;
					start += m_cpu->minInstrLen;
				}
			}
		}

		// Get true end address
		addr = start;
		instr = 0;
		while (addr < end && instr < numInstrs)
		{
			// Get instruction length
			int codesLen = m_cpu->Disassemble(addr, mnemonic, operands);
			// Move onto next valid instruction address
			addr += abs(codesLen);
			if (analyser != NULL && !analyser->analysis->GetNextValidAddr(addr))
				break;
			instr++;
		}
		end = addr;
				
		// Format start and end addresses and output title
		m_cpu->FormatAddress(startStr, start);
		m_cpu->FormatAddress(endStr, end);
		Print("%s Code %s - %s:\n", m_cpu->name, startStr, endStr);

		// Output the disassembly
		addr = start;
		instr = 0;
		while (addr < end)
		{
			// Add markers for current PC address and any breakpoints
			char ind[4];
			if (m_cpu->instrCount > 0 && addr == pc)
			{
				ind[0] = '>';
				ind[1] = '>';
			}
			else
			{
				ind[0] = ' ';
				ind[1] = ' ';
			}
			CBreakpoint *bp = m_cpu->GetBreakpoint(addr);
			ind[2] = (bp != NULL ? bp->symbol : ' ');
			ind[3] = '\0';
			
			// Format current address
			m_cpu->FormatAddress(addrStr, addr);

			// Get labels at address (if any)
			bool hasLabel = GetLabelText(labelStr, 12, addr);

			// Get mnemonic, operands and instruction length and print them
			int codesLen = m_cpu->Disassemble(addr, mnemonic, operands);		
			FormatOpCodes(opCodes, addr, abs(codesLen));
			
			// Get comment at address (if any)
			CComment *comment = m_cpu->GetComment(addr);
			
			// Output line
			Print("%s", ind);
			if (m_showLabels)
			{
				if (m_labelsOverAddr)
					Print("%-12s ", (hasLabel ? labelStr : addrStr));
				else
					Print("%s %-12s ", addrStr, labelStr);
			}
			else
				Print("%s ", addrStr);
			if (m_showOpCodes) 
				Print("[%s] ", opCodes);
			if (codesLen > 0)
				Print("%-*s %-20s", (int)m_cpu->maxMnemLen, mnemonic, operands);
			else
				Print("???");
			if (comment != NULL)
				Print(" ; %s", comment->text);
			Print("\n");

			// Move onto next valid instruction address
			addr += abs(codesLen);
			if (analyser != NULL && !analyser->analysis->GetNextValidAddr(addr))
				break;
		}
		return end;
	}

	UINT32 CConsoleDebugger::ListMemory(UINT32 start, UINT32 end, unsigned bytesPerRow)
	{
		char startStr[255];
		char endStr[255];
		char addrStr[20];
		char labelStr[13];
		char wChar, dChar;
		UINT8 data;
		
		// Adjust start/end points to be on row boundary
		start -= (start % bytesPerRow);
		end += bytesPerRow - (end % bytesPerRow);

		// Format start and end addresses and output title
		m_cpu->FormatAddress(startStr, start);
		m_cpu->FormatAddress(endStr, end);
		Print("%s Memory %s - %s:\n", m_cpu->name, startStr, endStr);

		UINT32 addr = start;
		while (addr < end)
		{
			// TODO - check address going out of region or out of range

			// Format current address
			m_cpu->FormatAddress(addrStr, addr);
			
			// Get labels at address (if any)
			bool hasLabel = GetLabelText(labelStr, 12, addr);

			// Output line
			if (m_showLabels)
			{
				if (m_labelsOverAddr)
					Print("   %-12s", (hasLabel ? labelStr : addrStr));
				else
					Print("   %s %-12s", addrStr, labelStr);
			}
			else
				Print("   %s%c", addrStr);
			UINT32 lAddr = addr;
			for (unsigned i = 0; i < bytesPerRow; i++)
			{
				CWatch *watch = m_cpu->GetMemWatch(lAddr, 1);
				// TODO - handling of mapped I/O
				//CMappedIO *io = m_cpu->GetMappedIO(lAddr);
				wChar = (watch != NULL ? watch->symbol : ' ');
				//if (io != NULL)
				//	data = (UINT8)io->last;
				//else
					data = (UINT8)m_cpu->ReadMem(lAddr, 1);
				Print("%c%02X", wChar, data);
				lAddr++;
			}
			Print(" ");
			lAddr = addr;
			for (unsigned i = 0; i < bytesPerRow; i++)
			{
				// TODO - handling of mapped I/O
				//CMappedIO *io = m_cpu->GetMappedIO(lAddr);
				//if (io != NULL)
				//	data = io->last;
				//else
					data = (UINT8)m_cpu->ReadMem(lAddr, 1);
				dChar = (data >= 32 && data <= 126 ? (char)data : '.');
				Print("%c", dChar);
				lAddr++;
			}
			Print("\n");
			addr += bytesPerRow;
		}
		return addr;
	}

	void CConsoleDebugger::AnalysisUpdated(CCodeAnalyser *analyser)
	{
		//
	}

	void CConsoleDebugger::ExceptionTrapped(CException *ex)
	{
		PrintEvent(ex->cpu, "Exception %s (%s) trapped.\n", ex->id, ex->name);
	}

	void CConsoleDebugger::InterruptTrapped(CInterrupt *in)
	{
		PrintEvent(in->cpu, "Interrupt %s (%s) trapped.\n", in->id, in->name);
	}

	void CConsoleDebugger::MemWatchTriggered(CWatch *watch, UINT32 addr, unsigned dataSize, UINT64 data, bool isRead)
	{
		const char *sizeStr = GetDataSizeStr(dataSize, true);
		const char *rwStr = (isRead ? "Read from" : "Write to");
		char dataStr[50];
		char addrStr[255];
		m_cpu->FormatData(dataStr, dataSize, data);
		watch->cpu->FormatAddress(addrStr, addr, true);
		int num = GetIndexOfMemWatch(watch);
		PrintEvent(watch->cpu, "%s %s (%s.%s) triggered memory watch #%d.\n", rwStr, addrStr, dataStr, sizeStr, num);
	}

	void CConsoleDebugger::IOWatchTriggered(CWatch *watch, CIO *io, UINT64 data, bool isInput)
	{
		const char *sizeStr = GetDataSizeStr(io->dataSize, true);
		const char *ioStr = (isInput ? "Input from" : "Output to");
		char dataStr[50];
		char locStr[255];
		m_cpu->FormatData(dataStr, io->dataSize, data);
		watch->io->GetLocation(locStr);
		int num = GetIndexOfPortWatch(watch);
		if (num >= 0)
			PrintEvent(watch->cpu, "%s %s (%s.%s) triggered port watch #%d.\n", ioStr, locStr, dataStr, sizeStr, num);
		else
		{
			num = GetIndexOfMemWatch(watch);
			PrintEvent(watch->cpu, "%s %s (%s.%s) triggered memory watch #%d.\n", ioStr, locStr, dataStr, sizeStr, num);
		}
	}

	void CConsoleDebugger::BreakpointReached(CBreakpoint *bp)
	{
		PrintEvent(bp->cpu, "Breakpoint #%d triggered.\n", bp->num);
	}

	void CConsoleDebugger::MonitorTriggered(CRegMonitor *regMon)
	{
		char valStr[255];
		CRegister *reg = regMon->reg;
		reg->GetValue(valStr);
		PrintEvent(reg->cpu, "Write to register %s (%s) triggered monitor.\n", reg->name, valStr);
	}

	void CConsoleDebugger::ExecutionHalted(CCPUDebug *cpu, EHaltReason reason)
	{
		if (reason&HaltUser)
			PrintEvent(cpu, "Execution halted.\n");
	}
	
	void CConsoleDebugger::Log(CCPUDebug *cpu, const char *typeStr, const char *fmtStr, va_list vl)
	{
		if (cpu != NULL)
		{
			char pcStr[255];
			cpu->FormatAddress(pcStr, cpu->pc, true, LFNone);
			Print("%s @ %s: ", cpu->name, pcStr);
		}
		if (typeStr != NULL)
			Print(" %s - ", typeStr);
		PrintVL(fmtStr, vl);
	}

	void CConsoleDebugger::Attach()
	{
		CDebugger::Attach();

		ApplyConfig();

		Attached();
	}

	void CConsoleDebugger::Detach()
	{
		Detaching();

		CDebugger::Detach();

		// Close redirected output file, if exists
		if (m_file != NULL)
		{
			fclose(m_file);
			m_file = NULL;
		}
	}

	void CConsoleDebugger::Poll()
	{
		CDebugger::Poll();

		if (m_nextFrame)
		{
			if (--m_nextFrameCount == 0)
				ForceBreak(true);
		}	
	}

	void CConsoleDebugger::ApplyConfig()
	{
		for (vector<CCPUDebug*>::iterator it = cpus.begin(); it != cpus.end(); it++)
		{
			(*it)->addrFmt = m_addrFmt;
			(*it)->portFmt = m_portFmt;
			(*it)->dataFmt = m_dataFmt;
		}
	}

	void CConsoleDebugger::Attached()
	{
		//
	}

	void CConsoleDebugger::Detaching()
	{
		//
	}
}

#endif  // SUPERMODEL_DEBUGGER
