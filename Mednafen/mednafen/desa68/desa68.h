/**
 * @ingroup   desa68_devel
 * @file      desa68/desa68.h
 * @author    Benjamin Gerard <ben@sashipa.com>
 * @date      17/03/1999
 * @brief     Motorola 68K disassembler
 *
 * $Id: desa68.h,v 2.3 2003/08/26 23:14:02 benjihan Exp $
 */

/*
 *                        sc68 - 68000 disassembler
 *         Copyright (C) 2001-2003 Benjamin Gerard <ben@sashipa.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

/* Modified for usage in Mednafen */

#ifndef _DESA68_H_
#define _DESA68_H_

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EMU68DEBUG
#define EMU68DEBUG
#endif

/** @defgroup desa68_devel desa68 library documentation.
 *
 *  @author Benjamin Gerard <ben@sashipa.com>
 *
 *  desa68 library is a 68000 disassembler with enhanced features that help to
 *  trace branch instructions.
 *
 *  Optionnally the disassembler may disassemble with symbol instead of 
 *  absolute address or long immediat. A supplemental control is available
 *  to choose the range of address that must be disassembled as symbol.
 *
 *  A good sample is may be found in the sourcer68 directory. This library
 *  is also used by debug68 programs.
 *
 *  @{
 */

/** @name Disassembly option flags.
 *  @anchor desa68_opt_flags
 *
 *  Use bitwise OR operation with these values to set the
 *  the DESA68parm_t::flags in order to configure the disassembler.
 *
 *  @{
 */

/** Disassemble with symbol.
 *
 *   If the DESA68_SYMBOL_FLAG is set in the DESA68parm_t::flags 
 *   and the value of absolute long addressing mode or an immediat long
 *   is in greater or equal to DESA68parm_t::immsym_min and less than
 *   DESA68parm_t::immsym_max
 *   then the disassembler replaces the value by a named symbol.
 *   The named symbol constist on the value transformed to an 6 hexadecimal
 *   digit number with a prefixed 'L'.
 */
#define DESA68_SYMBOL_FLAG  (1<<0)

/**@}*/

/** @name Instruction type flags.
 *  @anchor desa68_inst_flags
 *
 *  These flags are setted in the DESA68parm_t::status field by desa68()
 *  function. It allow to determine the type of the dissassembled instruction.
 *
 *  @{
 */

/** Valid instruction. */
#define DESA68_INST (1<<0)

/** Branch always instruction (bra/jmp/dbcc). */
#define DESA68_BRA  (1<<1)

/** Subroutine (bsr/jsr)/ Conditionnal branch instruction (bcc/dbcc). */
#define DESA68_BSR  (1<<2)

/** Return from subroutine/Interruption instruction (rts/rte). */
#define DESA68_RTS  (1<<3)

/** Software interrupt instruction (trap/chk). */ 
#define DESA68_INT  (1<<4)

/** nop instruction. */ 
#define DESA68_NOP  (1<<5) 

/**@}*/


/** 68K disassemble pass parameters.
 *
 *    The DESA68parm_t data structure contains the information necessary to
 *    disassemble 68K instructions.
 *
 *    There are 3 categories of fields in this structure.
 *     - Input parameters; Must be set before calling the desa68() function.
 *     - Output parameters; Information on disassembled instruction
 *                          filled by desa68() function.
 *     - Miscellaneous internal fields.
 *
 * @note The DESA68parm_t::pc field is both input and output since it is use
 *       to set the address of the instruction to decode and returns with
 *       the value of the next one.
 */
typedef struct
{

  /** @name Input parameters.
   *
   *  These parameters must be set before calling the desa68() functions.
   *
   *  @{
   */

  uint16_t (*mem_callb)(uint32_t address, void *private_data);
  void *private_data;

  /*unsigned char *mem;  */  /**< Base of 68K memory.                         */

  unsigned int   memmsk; /**< Size of memory - 1 (mask).                  */
  /** Address (Offset in mem) of instruction to disassemble; Returns
   *  with the address of the next instruction.
   */
  unsigned int   pc;
  int            flags;  /**< @ref desa68_opt_flags "Disassemble options" */
  char          *str;    /**< Destination string.                         */
  int            strmax; /**< Destination string buffer size.
			      @warning Unused                             */
  /** Minimum value to interpret long immediat or absolute long as symbol.
   * @see DESA68_SYMBOL_FLAG for more details
   * @see immsym_max
   */
  unsigned int   immsym_min;
  /** Maximum value to interpret long immediat or absolute long as symbol.
   * @see DESA68_SYMBOL_FLAG for more details
   * @see immsym_min
   */
  unsigned int   immsym_max;

  /**@}*/


  /** @name Output parameters.
   *
   *  These parameters are setted by the desa68() functions.
   *  @{
   */

  /** Effective address of source operand (-1:not a memory operand). */
  unsigned int ea_src;
  /** Effective address of destiantion operand (-1:not a memory operand). */
  unsigned int ea_dst; /**< Effective address of destination operand */
  /** @ref desa68_inst_flags "disassembly instruction flags" */
  unsigned int status;
  /** Branch or interrupt vector address.
   *
   *  If the dissassembled instruction was a branch a call or a sotfware
   *  interrupt the DESA68parm_t::branch is set to the jump address or
   *  the interrupt vector involved.
   *
   * @see status for more information on instruction type.
   */
  unsigned int branch;
  /** Last decoded word (16 bit sign extended). */
  int w;                  
  /** Pointer to current destination char. */
  char *s;

  /**@}*/


  /** @name Miscellaneous internal variables.
   * @internal
   * @{
   */

  /** Intermediat opcode decoding. */
  int	reg0;
  int	reg9;
  int	mode3;
  int	mode6;
  int	opsz;
  int	line;
  int	adrmode0;
  int	adrmode6;
  int	szchar;
  unsigned int ea;

  /**@}*/

} DESA68parm_t;


/** Disassemble a single 68000 instruction.
 *
 *    param  d  Pointer to disassemble pass parameter structure.
 */
void desa68(DESA68parm_t *d);

/**
 *  @}
 */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _DESA68_H_ */
