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
 * PPCDisasm.h
 * 
 * Header file for PowerPC disassembler.
 */
 
#ifndef INCLUDED_PPCDISASM_H
#define INCLUDED_PPCDISASM_H

/*
 * DisassemblePowerPC(op, vpc, mnem, oprs, simplify):
 *
 * Disassembles one PowerPC 603e instruction. 
 *
 * A non-zero return code indicates that the instruction could not be
 * recognized or that the operands to an instruction were invalid. To
 * determine which case occured, check if mnem[0] == '\0'. If it does not,
 * then the latter case happened.
 *
 * Arguments:
 *      op         Instruction word to disassemble.
 *      vpc        Current instruction address.
 *      mnem       Buffer to write instruction mnemonic to. If no
 *                 instruction was decoded, mnem[0] and oprs[0] will be set
 *                 to '\0'.
 *      oprs       Buffer to write any operands to.
 *      simplify   If non-zero, simplified forms of instructions will be
 *                 printed in certain cases.
 *
 * Returns:
 *      OKAY if successful, FAIL if the instruction was unrecognized or had an
 *      invalid form (see note above in function description.)
 */ 
extern bool DisassemblePowerPC(UINT32 op, UINT32 vpc, char *mnem, char *oprs,
                        	   bool simplify);

#endif	// INCLUDED_PPCDISASM_H
