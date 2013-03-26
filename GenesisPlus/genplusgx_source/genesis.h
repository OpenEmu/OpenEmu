/***************************************************************************************
 *  Genesis Plus
 *  Internal hardware & Bus controllers
 *
 *  Support for SG-1000, Mark-III, Master System, Game Gear, Mega Drive & Mega CD hardware
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  Copyright (C) 2007-2013  Eke-Eke (Genesis Plus GX)
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

#ifndef _GENESIS_H_
#define _GENESIS_H_

#include "md_cart.h"
#include "sms_cart.h"
#include "scd.h"

/* External Hardware */
typedef union
{
  md_cart_t md_cart;
  cd_hw_t cd_hw;
} external_t;

/* Global variables */
extern external_t ext;
extern uint8 boot_rom[0x800];
extern uint8 work_ram[0x10000];
extern uint8 zram[0x2000];
extern uint32 zbank;
extern uint8 zstate;
extern uint8 pico_current;

/* Function prototypes */
extern void gen_init(void);
extern void gen_reset(int hard_reset);
extern void gen_tmss_w(unsigned int offset, unsigned int data);
extern void gen_bankswitch_w(unsigned int data);
extern unsigned int gen_bankswitch_r(void);
extern void gen_zbusreq_w(unsigned int state, unsigned int cycles);
extern void gen_zreset_w(unsigned int state, unsigned int cycles);
extern void gen_zbank_w(unsigned int state);
extern int z80_irq_callback(int param);

#endif /* _GEN_H_ */

