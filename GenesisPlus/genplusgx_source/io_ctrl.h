/***************************************************************************************
 *  Genesis Plus
 *  I/O controller
 *
 *  Support for Master System (315-5216, 315-5237 & 315-5297), Game Gear & Mega Drive I/O chips
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  Copyright (C) 2007-2012  Eke-Eke (Genesis Plus GX)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/

#ifndef _IO_CTRL_H_
#define _IO_CTRL_H_

#define IO_RESET_HI 0x10
#define IO_CONT1_HI 0x20

#define REGION_JAPAN_NTSC 0x00
#define REGION_JAPAN_PAL  0x40
#define REGION_USA        0x80
#define REGION_EUROPE     0xC0

/* Global variables */
extern uint8 io_reg[0x10];
extern uint8 region_code;

/* Function prototypes */
extern void io_init(void);
extern void io_reset(void);
extern void io_68k_write(unsigned int offset, unsigned int data);
extern unsigned int io_68k_read(unsigned int offset);
extern void io_z80_write(unsigned int offset, unsigned int data, unsigned int cycles);
extern unsigned int io_z80_read(unsigned int offset);
extern void io_gg_write(unsigned int offset, unsigned int data);
extern unsigned int io_gg_read(unsigned int offset);

#endif /* _IO_CTRL_H_ */

