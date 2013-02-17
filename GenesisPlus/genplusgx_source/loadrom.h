/***************************************************************************************
 *  Genesis Plus
 *  ROM Loading Support
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

#ifndef _LOADROM_H_
#define _LOADROM_H_

#define MAXROMSIZE 10485760

typedef struct
{
  char consoletype[18];         /* Genesis or Mega Drive */
  char copyright[18];           /* Copyright message */
  char domestic[50];            /* Domestic name of ROM */
  char international[50];       /* International name of ROM */
  char ROMType[4];              /* Boot ROM (BR), Educational (AL) or Game (GM) program */
  char product[14];             /* Product serial number */
  unsigned short checksum;      /* ROM Checksum (header) */
  unsigned short realchecksum;  /* ROM Checksum (calculated) */
  unsigned int romstart;        /* ROM start address */
  unsigned int romend;          /* ROM end address */
  char country[18];             /* Country flag */
  uint16 peripherals;           /* Supported peripherals */
} ROMINFO;


/* Global variables */
extern ROMINFO rominfo;
extern uint8 romtype;

/* Function prototypes */
extern int load_bios(void);
extern int load_rom(char *filename);
extern void get_region(char *romheader);
extern char *get_company(void);
extern char *get_peripheral(int index);
extern void getrominfo(char *romheader);

#endif /* _LOADROM_H_ */

