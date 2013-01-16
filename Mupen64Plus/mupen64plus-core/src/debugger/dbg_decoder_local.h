/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - dbg_debugger_local.h                                    *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2010 Marshall B. Rogers <mbr@64.vg>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef __DECODER_LOCAL_H__
#define __DECODER_LOCAL_H__

#include <stdio.h>

/*	$NetBSD: cpuregs.h,v 1.77 2009/12/14 00:46:04 matt Exp $	*/

/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell and Rick Macklem.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)machConst.h 8.1 (Berkeley) 6/10/93
 *
 * machConst.h --
 *
 *	Machine dependent constants.
 *
 *	Copyright (C) 1989 Digital Equipment Corporation.
 *	Permission to use, copy, modify, and distribute this software and
 *	its documentation for any purpose and without fee is hereby granted,
 *	provided that the above copyright notice appears in all copies.
 *	Digital Equipment Corporation makes no representations about the
 *	suitability of this software for any purpose.  It is provided "as is"
 *	without express or implied warranty.
 *
 * from: Header: /sprite/src/kernel/mach/ds3100.md/RCS/machConst.h,
 *	v 9.2 89/10/21 15:55:22 jhh Exp	 SPRITE (DECWRL)
 * from: Header: /sprite/src/kernel/mach/ds3100.md/RCS/machAddrs.h,
 *	v 1.2 89/08/15 18:28:21 rab Exp	 SPRITE (DECWRL)
 * from: Header: /sprite/src/kernel/vm/ds3100.md/RCS/vmPmaxConst.h,
 *	v 9.1 89/09/18 17:33:00 shirriff Exp  SPRITE (DECWRL)
 */

#ifndef _MIPS_CPUREGS_H_
#define	_MIPS_CPUREGS_H_


/*
 * Address space.
 * 32-bit mips CPUS partition their 32-bit address space into four segments:
 *
 * kuseg   0x00000000 - 0x7fffffff  User virtual mem,  mapped
 * kseg0   0x80000000 - 0x9fffffff  Physical memory, cached, unmapped
 * kseg1   0xa0000000 - 0xbfffffff  Physical memory, uncached, unmapped
 * kseg2   0xc0000000 - 0xffffffff  kernel-virtual,  mapped
 *
 * mips1 physical memory is limited to 512Mbytes, which is
 * doubly mapped in kseg0 (cached) and kseg1 (uncached.)
 * Caching of mapped addresses is controlled by bits in the TLB entry.
 */

#ifdef _LP64
#define	MIPS_XUSEG_START		(0L << 62)
#define	MIPS_XUSEG_P(x)			(((uint64_t)(x) >> 62) == 0)
#define	MIPS_USEG_P(x)			((uintptr_t)(x) < 0x80000000L)
#define	MIPS_XSSEG_START		(1L << 62)
#define	MIPS_XSSEG_P(x)			(((uint64_t)(x) >> 62) == 1)
#endif

/*
 * MIPS addresses are signed and we defining as negative so that
 * in LP64 kern they get sign-extended correctly.
 */
#ifndef _LOCORE
#define	MIPS_KSEG0_START		(-0x7fffffffL-1) /* 0x80000000 */
#define	MIPS_KSEG1_START		-0x60000000L	/* 0xa0000000 */
#define	MIPS_KSEG2_START		-0x40000000L	/* 0xc0000000 */
#define	MIPS_MAX_MEM_ADDR		-0x42000000L	/* 0xbe000000 */
#define	MIPS_RESERVED_ADDR		-0x40380000L	/* 0xbfc80000 */
#endif

#define	MIPS_PHYS_MASK			0x1fffffff

#define	MIPS_KSEG0_TO_PHYS(x)	((uintptr_t)(x) & MIPS_PHYS_MASK)
#define	MIPS_PHYS_TO_KSEG0(x)	((uintptr_t)(x) | (intptr_t)MIPS_KSEG0_START)
#define	MIPS_KSEG1_TO_PHYS(x)	((uintptr_t)(x) & MIPS_PHYS_MASK)
#define	MIPS_PHYS_TO_KSEG1(x)	((uintptr_t)(x) | (intptr_t)MIPS_KSEG1_START)

#define	MIPS_KSEG0_P(x)		(((intptr_t)(x) & ~MIPS_PHYS_MASK) == MIPS_KSEG0_START)
#define	MIPS_KSEG1_P(x)		(((intptr_t)(x) & ~MIPS_PHYS_MASK) == MIPS_KSEG1_START)
#define	MIPS_KSEG2_P(x)		((uintptr_t)MIPS_KSEG2_START <= (uintptr_t)(x))

/* Map virtual address to index in mips3 r4k virtually-indexed cache */
#define	MIPS3_VA_TO_CINDEX(x) \
		(((intptr_t)(x) & 0xffffff) | MIPS_KSEG0_START) 

#ifndef _LOCORE
#define	MIPS_XSEG_MASK		(0x3fffffffffffffffLL)
#define	MIPS_XKSEG_START	(0x3ULL << 62)
#define	MIPS_XKSEG_P(x)		(((uint64_t)(x) >> 62) == 3)

#define	MIPS_XKPHYS_START	(0x2ULL << 62)
#define	MIPS_PHYS_TO_XKPHYS_UNCACHED(x) \
	(MIPS_XKPHYS_START | ((uint64_t)(CCA_UNCACHED) << 59) | (x))
#define	MIPS_PHYS_TO_XKPHYS_CACHED(x) \
	(mips3_xkphys_cached | (x))
#define	MIPS_PHYS_TO_XKPHYS(cca,x) \
	(MIPS_XKPHYS_START | ((uint64_t)(cca) << 59) | (x))
#define	MIPS_XKPHYS_TO_PHYS(x)	((uint64_t)(x) & 0x07ffffffffffffffLL)
#define	MIPS_XKPHYS_TO_CCA(x)	(((uint64_t)(x) >> 59) & 7)
#define	MIPS_XKPHYS_P(x)	(((uint64_t)(x) >> 62) == 2)
#endif	/* _LOCORE */

#define	CCA_UNCACHED		2
#define	CCA_CACHEABLE		3	/* cacheable non-coherent */

/* CPU dependent mtc0 hazard hook */
#define	COP0_SYNC		/* nothing */
#define	COP0_HAZARD_FPUENABLE	nop; nop; nop; nop;

/*
 * The bits in the cause register.
 *
 * Bits common to r3000 and r4000:
 *
 *	MIPS_CR_BR_DELAY	Exception happened in branch delay slot.
 *	MIPS_CR_COP_ERR		Coprocessor error.
 *	MIPS_CR_IP		Interrupt pending bits defined below.
 *				(same meaning as in CAUSE register).
 *	MIPS_CR_EXC_CODE	The exception type (see exception codes below).
 *
 * Differences:
 *  r3k has 4 bits of execption type, r4k has 5 bits.
 */
#define	MIPS_CR_BR_DELAY	0x80000000
#define	MIPS_CR_COP_ERR		0x30000000
#define	MIPS1_CR_EXC_CODE	0x0000003C	/* four bits */
#define	MIPS3_CR_EXC_CODE	0x0000007C	/* five bits */
#define	MIPS_CR_IP		0x0000FF00
#define	MIPS_CR_EXC_CODE_SHIFT	2

/*
 * The bits in the status register.  All bits are active when set to 1.
 *
 *	R3000 status register fields:
 *	MIPS_SR_COP_USABILITY	Control the usability of the four coprocessors.
 *	MIPS_SR_TS		TLB shutdown.
 *
 *	MIPS_SR_INT_IE		Master (current) interrupt enable bit.
 *
 * Differences:
 *	r3k has cache control is via frobbing SR register bits, whereas the
 *	r4k cache control is via explicit instructions.
 *	r3k has a 3-entry stack of kernel/user bits, whereas the
 *	r4k has kernel/supervisor/user.
 */
#define	MIPS_SR_COP_USABILITY	0xf0000000
#define	MIPS_SR_COP_0_BIT	0x10000000
#define	MIPS_SR_COP_1_BIT	0x20000000

	/* r4k and r3k differences, see below */

#define	MIPS_SR_MX		0x01000000	/* MIPS64 */
#define	MIPS_SR_PX		0x00800000	/* MIPS64 */
#define	MIPS_SR_BEV		0x00400000	/* Use boot exception vector */
#define	MIPS_SR_TS		0x00200000

	/* r4k and r3k differences, see below */

#define	MIPS_SR_INT_IE		0x00000001
/*#define MIPS_SR_MBZ		0x0f8000c0*/	/* Never used, true for r3k */
/*#define MIPS_SR_INT_MASK	0x0000ff00*/


/*
 * The R2000/R3000-specific status register bit definitions.
 * all bits are active when set to 1.
 *
 *	MIPS_SR_PARITY_ERR	Parity error.
 *	MIPS_SR_CACHE_MISS	Most recent D-cache load resulted in a miss.
 *	MIPS_SR_PARITY_ZERO	Zero replaces outgoing parity bits.
 *	MIPS_SR_SWAP_CACHES	Swap I-cache and D-cache.
 *	MIPS_SR_ISOL_CACHES	Isolate D-cache from main memory.
 *				Interrupt enable bits defined below.
 *	MIPS_SR_KU_OLD		Old kernel/user mode bit. 1 => user mode.
 *	MIPS_SR_INT_ENA_OLD	Old interrupt enable bit.
 *	MIPS_SR_KU_PREV		Previous kernel/user mode bit. 1 => user mode.
 *	MIPS_SR_INT_ENA_PREV	Previous interrupt enable bit.
 *	MIPS_SR_KU_CUR		Current kernel/user mode bit. 1 => user mode.
 */

#define	MIPS1_PARITY_ERR	0x00100000
#define	MIPS1_CACHE_MISS	0x00080000
#define	MIPS1_PARITY_ZERO	0x00040000
#define	MIPS1_SWAP_CACHES	0x00020000
#define	MIPS1_ISOL_CACHES	0x00010000

#define	MIPS1_SR_KU_OLD		0x00000020	/* 2nd stacked KU/IE*/
#define	MIPS1_SR_INT_ENA_OLD	0x00000010	/* 2nd stacked KU/IE*/
#define	MIPS1_SR_KU_PREV	0x00000008	/* 1st stacked KU/IE*/
#define	MIPS1_SR_INT_ENA_PREV	0x00000004	/* 1st stacked KU/IE*/
#define	MIPS1_SR_KU_CUR		0x00000002	/* current KU */

/* backwards compatibility */
#define	MIPS_SR_PARITY_ERR	MIPS1_PARITY_ERR
#define	MIPS_SR_CACHE_MISS	MIPS1_CACHE_MISS
#define	MIPS_SR_PARITY_ZERO	MIPS1_PARITY_ZERO
#define	MIPS_SR_SWAP_CACHES	MIPS1_SWAP_CACHES
#define	MIPS_SR_ISOL_CACHES	MIPS1_ISOL_CACHES

#define	MIPS_SR_KU_OLD		MIPS1_SR_KU_OLD
#define	MIPS_SR_INT_ENA_OLD	MIPS1_SR_INT_ENA_OLD
#define	MIPS_SR_KU_PREV		MIPS1_SR_KU_PREV
#define	MIPS_SR_KU_CUR		MIPS1_SR_KU_CUR
#define	MIPS_SR_INT_ENA_PREV	MIPS1_SR_INT_ENA_PREV

/*
 * R4000 status register bit definitons,
 * where different from r2000/r3000.
 */
#define	MIPS3_SR_XX		0x80000000
#define	MIPS3_SR_RP		0x08000000
#define	MIPS3_SR_FR		0x04000000
#define	MIPS3_SR_RE		0x02000000

#define	MIPS3_SR_DIAG_DL	0x01000000		/* QED 52xx */
#define	MIPS3_SR_DIAG_IL	0x00800000		/* QED 52xx */
#define	MIPS3_SR_PX		0x00800000		/* MIPS64 */
#define	MIPS3_SR_SR		0x00100000
#define	MIPS3_SR_NMI		0x00080000		/* MIPS32/64 */
#define	MIPS3_SR_DIAG_CH	0x00040000
#define	MIPS3_SR_DIAG_CE	0x00020000
#define	MIPS3_SR_DIAG_PE	0x00010000
#define	MIPS3_SR_EIE		0x00010000		/* TX79/R5900 */
#define	MIPS3_SR_KX		0x00000080
#define	MIPS3_SR_SX		0x00000040
#define	MIPS3_SR_UX		0x00000020
#define	MIPS3_SR_KSU_MASK	0x00000018
#define	MIPS3_SR_KSU_USER	0x00000010
#define	MIPS3_SR_KSU_SUPER	0x00000008
#define	MIPS3_SR_KSU_KERNEL	0x00000000
#define	MIPS3_SR_ERL		0x00000004
#define	MIPS3_SR_EXL		0x00000002

#ifdef MIPS3_5900
#undef MIPS_SR_INT_IE
#define	MIPS_SR_INT_IE		0x00010001		/* XXX */
#endif

#define	MIPS_SR_SOFT_RESET	MIPS3_SR_SOFT_RESET
#define	MIPS_SR_DIAG_CH		MIPS3_SR_DIAG_CH
#define	MIPS_SR_DIAG_CE		MIPS3_SR_DIAG_CE
#define	MIPS_SR_DIAG_PE		MIPS3_SR_DIAG_PE
#define	MIPS_SR_KX		MIPS3_SR_KX
#define	MIPS_SR_SX		MIPS3_SR_SX
#define	MIPS_SR_UX		MIPS3_SR_UX

#define	MIPS_SR_KSU_MASK	MIPS3_SR_KSU_MASK
#define	MIPS_SR_KSU_USER	MIPS3_SR_KSU_USER
#define	MIPS_SR_KSU_SUPER	MIPS3_SR_KSU_SUPER
#define	MIPS_SR_KSU_KERNEL	MIPS3_SR_KSU_KERNEL
#define	MIPS_SR_ERL		MIPS3_SR_ERL
#define	MIPS_SR_EXL		MIPS3_SR_EXL


/*
 * The interrupt masks.
 * If a bit in the mask is 1 then the interrupt is enabled (or pending).
 */
#define	MIPS_INT_MASK		0xff00
#define	MIPS_INT_MASK_5		0x8000
#define	MIPS_INT_MASK_4		0x4000
#define	MIPS_INT_MASK_3		0x2000
#define	MIPS_INT_MASK_2		0x1000
#define	MIPS_INT_MASK_1		0x0800
#define	MIPS_INT_MASK_0		0x0400
#define	MIPS_HARD_INT_MASK	0xfc00
#define	MIPS_SOFT_INT_MASK_1	0x0200
#define	MIPS_SOFT_INT_MASK_0	0x0100

/*
 * mips3 CPUs have on-chip timer at INT_MASK_5.  Each platform can
 * choose to enable this interrupt.
 */
#if defined(MIPS3_ENABLE_CLOCK_INTR)
#define	MIPS3_INT_MASK			MIPS_INT_MASK
#define	MIPS3_HARD_INT_MASK		MIPS_HARD_INT_MASK
#else
#define	MIPS3_INT_MASK			(MIPS_INT_MASK &  ~MIPS_INT_MASK_5)
#define	MIPS3_HARD_INT_MASK		(MIPS_HARD_INT_MASK & ~MIPS_INT_MASK_5)
#endif

/*
 * The bits in the context register.
 */
#define	MIPS1_CNTXT_PTE_BASE	0xFFE00000
#define	MIPS1_CNTXT_BAD_VPN	0x001FFFFC

#define	MIPS3_CNTXT_PTE_BASE	0xFF800000
#define	MIPS3_CNTXT_BAD_VPN2	0x007FFFF0

/*
 * The bits in the MIPS3 config register.
 *
 *	bit 0..5: R/W, Bit 6..31: R/O
 */

/* kseg0 coherency algorithm - see MIPS3_TLB_ATTR values */
#define	MIPS3_CONFIG_K0_MASK	0x00000007

/*
 * R/W Update on Store Conditional
 *	0: Store Conditional uses coherency algorithm specified by TLB
 *	1: Store Conditional uses cacheable coherent update on write
 */
#define	MIPS3_CONFIG_CU		0x00000008

#define	MIPS3_CONFIG_DB		0x00000010	/* Primary D-cache line size */
#define	MIPS3_CONFIG_IB		0x00000020	/* Primary I-cache line size */
#define	MIPS3_CONFIG_CACHE_L1_LSIZE(config, bit) \
	(((config) & (bit)) ? 32 : 16)

#define	MIPS3_CONFIG_DC_MASK	0x000001c0	/* Primary D-cache size */
#define	MIPS3_CONFIG_DC_SHIFT	6
#define	MIPS3_CONFIG_IC_MASK	0x00000e00	/* Primary I-cache size */
#define	MIPS3_CONFIG_IC_SHIFT	9
#define	MIPS3_CONFIG_C_DEFBASE	0x1000		/* default base 2^12 */

/* Cache size mode indication: available only on Vr41xx CPUs */
#define	MIPS3_CONFIG_CS		0x00001000
#define	MIPS3_CONFIG_C_4100BASE	0x0400		/* base is 2^10 if CS=1 */
#define	MIPS3_CONFIG_CACHE_SIZE(config, mask, base, shift) \
	((base) << (((config) & (mask)) >> (shift)))

/* External cache enable: Controls L2 for R5000/Rm527x and L3 for Rm7000 */
#define	MIPS3_CONFIG_SE		0x00001000

/* Block ordering: 0: sequential, 1: sub-block */
#define	MIPS3_CONFIG_EB		0x00002000

/* ECC mode - 0: ECC mode, 1: parity mode */
#define	MIPS3_CONFIG_EM		0x00004000

/* BigEndianMem - 0: kernel and memory are little endian, 1: big endian */
#define	MIPS3_CONFIG_BE		0x00008000

/* Dirty Shared coherency state - 0: enabled, 1: disabled */
#define	MIPS3_CONFIG_SM		0x00010000

/* Secondary Cache - 0: present, 1: not present */
#define	MIPS3_CONFIG_SC		0x00020000

/* System Port width - 0: 64-bit, 1: 32-bit (QED RM523x), 2,3: reserved */
#define	MIPS3_CONFIG_EW_MASK	0x000c0000
#define	MIPS3_CONFIG_EW_SHIFT	18

/* Secondary Cache port width - 0: 128-bit data path to S-cache, 1: reserved */
#define	MIPS3_CONFIG_SW		0x00100000

/* Split Secondary Cache Mode - 0: I/D mixed, 1: I/D separated by SCAddr(17) */
#define	MIPS3_CONFIG_SS		0x00200000

/* Secondary Cache line size */
#define	MIPS3_CONFIG_SB_MASK	0x00c00000
#define	MIPS3_CONFIG_SB_SHIFT	22
#define	MIPS3_CONFIG_CACHE_L2_LSIZE(config) \
	(0x10 << (((config) & MIPS3_CONFIG_SB_MASK) >> MIPS3_CONFIG_SB_SHIFT))

/* Write back data rate */
#define	MIPS3_CONFIG_EP_MASK	0x0f000000
#define	MIPS3_CONFIG_EP_SHIFT	24

/* System clock ratio - this value is CPU dependent */
#define	MIPS3_CONFIG_EC_MASK	0x70000000
#define	MIPS3_CONFIG_EC_SHIFT	28

/* Master-Checker Mode - 1: enabled */
#define	MIPS3_CONFIG_CM		0x80000000

/*
 * The bits in the MIPS4 config register.
 */

/* kseg0 coherency algorithm - see MIPS3_TLB_ATTR values */
#define	MIPS4_CONFIG_K0_MASK	MIPS3_CONFIG_K0_MASK
#define	MIPS4_CONFIG_DN_MASK	0x00000018	/* Device number */
#define	MIPS4_CONFIG_CT		0x00000020	/* CohPrcReqTar */
#define	MIPS4_CONFIG_PE		0x00000040	/* PreElmReq */
#define	MIPS4_CONFIG_PM_MASK	0x00000180	/* PreReqMax */
#define	MIPS4_CONFIG_EC_MASK	0x00001e00	/* SysClkDiv */
#define	MIPS4_CONFIG_SB		0x00002000	/* SCBlkSize */
#define	MIPS4_CONFIG_SK		0x00004000	/* SCColEn */
#define	MIPS4_CONFIG_BE		0x00008000	/* MemEnd */
#define	MIPS4_CONFIG_SS_MASK	0x00070000	/* SCSize */
#define	MIPS4_CONFIG_SC_MASK	0x00380000	/* SCClkDiv */
#define	MIPS4_CONFIG_RESERVED	0x03c00000	/* Reserved wired 0 */
#define	MIPS4_CONFIG_DC_MASK	0x1c000000	/* Primary D-Cache size */
#define	MIPS4_CONFIG_IC_MASK	0xe0000000	/* Primary I-Cache size */

#define	MIPS4_CONFIG_DC_SHIFT	26
#define	MIPS4_CONFIG_IC_SHIFT	29

#define	MIPS4_CONFIG_CACHE_SIZE(config, mask, base, shift)		\
	((base) << (((config) & (mask)) >> (shift)))

#define	MIPS4_CONFIG_CACHE_L2_LSIZE(config)				\
	(((config) & MIPS4_CONFIG_SB) ? 128 : 64)

/*
 * Location of exception vectors.
 *
 * Common vectors:  reset and UTLB miss.
 */
#define	MIPS_RESET_EXC_VEC	MIPS_PHYS_TO_KSEG1(0x1FC00000)
#define	MIPS_UTLB_MISS_EXC_VEC	MIPS_PHYS_TO_KSEG0(0)

/*
 * MIPS-1 general exception vector (everything else)
 */
#define	MIPS1_GEN_EXC_VEC	MIPS_PHYS_TO_KSEG0(0x0080)

/*
 * MIPS-III exception vectors
 */
#define	MIPS3_XTLB_MISS_EXC_VEC MIPS_PHYS_TO_KSEG0(0x0080)
#define	MIPS3_CACHE_ERR_EXC_VEC MIPS_PHYS_TO_KSEG0(0x0100)
#define	MIPS3_GEN_EXC_VEC	MIPS_PHYS_TO_KSEG0(0x0180)

/*
 * TX79 (R5900) exception vectors
 */
#define MIPS_R5900_COUNTER_EXC_VEC	MIPS_PHYS_TO_KSEG0(0x0080)
#define MIPS_R5900_DEBUG_EXC_VEC	MIPS_PHYS_TO_KSEG0(0x0100)

/*
 * MIPS32/MIPS64 (and some MIPS3) dedicated interrupt vector.
 */
#define	MIPS3_INTR_EXC_VEC	MIPS_PHYS_TO_KSEG0(0x0200)

/*
 * Coprocessor 0 registers:
 *
 *				v--- width for mips I,III,32,64
 *				     (3=32bit, 6=64bit, i=impl dep)
 *  0	MIPS_COP_0_TLB_INDEX	3333 TLB Index.
 *  1	MIPS_COP_0_TLB_RANDOM	3333 TLB Random.
 *  2	MIPS_COP_0_TLB_LOW	3... r3k TLB entry low.
 *  2	MIPS_COP_0_TLB_LO0	.636 r4k TLB entry low.
 *  3	MIPS_COP_0_TLB_LO1	.636 r4k TLB entry low, extended.
 *  4	MIPS_COP_0_TLB_CONTEXT	3636 TLB Context.
 *  5	MIPS_COP_0_TLB_PG_MASK	.333 TLB Page Mask register.
 *  6	MIPS_COP_0_TLB_WIRED	.333 Wired TLB number.
 *  8	MIPS_COP_0_BAD_VADDR	3636 Bad virtual address.
 *  9	MIPS_COP_0_COUNT	.333 Count register.
 * 10	MIPS_COP_0_TLB_HI	3636 TLB entry high.
 * 11	MIPS_COP_0_COMPARE	.333 Compare (against Count).
 * 12	MIPS_COP_0_STATUS	3333 Status register.
 * 13	MIPS_COP_0_CAUSE	3333 Exception cause register.
 * 14	MIPS_COP_0_EXC_PC	3636 Exception PC.
 * 15	MIPS_COP_0_PRID		3333 Processor revision identifier.
 * 15/1	MIPS_COP_0_EBASE	..33 Exception Base
 * 16	MIPS_COP_0_CONFIG	3333 Configuration register.
 * 16/1	MIPS_COP_0_CONFIG1	..33 Configuration register 1.
 * 16/2	MIPS_COP_0_CONFIG2	..33 Configuration register 2.
 * 16/3	MIPS_COP_0_CONFIG3	..33 Configuration register 3.
 * 17	MIPS_COP_0_LLADDR	.336 Load Linked Address.
 * 18	MIPS_COP_0_WATCH_LO	.336 WatchLo register.
 * 19	MIPS_COP_0_WATCH_HI	.333 WatchHi register.
 * 20	MIPS_COP_0_TLB_XCONTEXT .6.6 TLB XContext register.
 * 23	MIPS_COP_0_DEBUG	.... Debug JTAG register.
 * 24	MIPS_COP_0_DEPC		.... DEPC JTAG register.
 * 25	MIPS_COP_0_PERFCNT	..36 Performance Counter register.
 * 26	MIPS_COP_0_ECC		.3ii ECC / Error Control register.
 * 27	MIPS_COP_0_CACHE_ERR	.3ii Cache Error register.
 * 28/0	MIPS_COP_0_TAG_LO	.3ii Cache TagLo register (instr).
 * 28/1	MIPS_COP_0_DATA_LO	..ii Cache DataLo register (instr).
 * 28/2	MIPS_COP_0_TAG_LO	..ii Cache TagLo register (data).
 * 28/3	MIPS_COP_0_DATA_LO	..ii Cache DataLo register (data).
 * 29/0	MIPS_COP_0_TAG_HI	.3ii Cache TagHi register (instr).
 * 29/1	MIPS_COP_0_DATA_HI	..ii Cache DataHi register (instr).
 * 29/2	MIPS_COP_0_TAG_HI	..ii Cache TagHi register (data).
 * 29/3	MIPS_COP_0_DATA_HI	..ii Cache DataHi register (data).
 * 30	MIPS_COP_0_ERROR_PC	.636 Error EPC register.
 * 31	MIPS_COP_0_DESAVE	.... DESAVE JTAG register.
 */
#ifdef _LOCORE
#define	_(n)	__CONCAT($,n)
#else
#define	_(n)	n
#endif
#define	MIPS_COP_0_TLB_INDEX	_(0)
#define	MIPS_COP_0_TLB_RANDOM	_(1)
	/* Name and meaning of	TLB bits for $2 differ on r3k and r4k. */

#define	MIPS_COP_0_TLB_CONTEXT	_(4)
					/* $5 and $6 new with MIPS-III */
#define	MIPS_COP_0_BAD_VADDR	_(8)
#define	MIPS_COP_0_TLB_HI	_(10)
#define	MIPS_COP_0_STATUS	_(12)
#define	MIPS_COP_0_CAUSE	_(13)
#define	MIPS_COP_0_EXC_PC	_(14)
#define	MIPS_COP_0_PRID		_(15)


/* MIPS-I */
#define	MIPS_COP_0_TLB_LOW	_(2)

/* MIPS-III */
#define	MIPS_COP_0_TLB_LO0	_(2)
#define	MIPS_COP_0_TLB_LO1	_(3)

#define	MIPS_COP_0_TLB_PG_MASK	_(5)
#define	MIPS_COP_0_TLB_WIRED	_(6)

#define	MIPS_COP_0_COUNT	_(9)
#define	MIPS_COP_0_COMPARE	_(11)

#define	MIPS_COP_0_CONFIG	_(16)
#define	MIPS_COP_0_LLADDR	_(17)
#define	MIPS_COP_0_WATCH_LO	_(18)
#define	MIPS_COP_0_WATCH_HI	_(19)
#define	MIPS_COP_0_TLB_XCONTEXT _(20)
#define	MIPS_COP_0_ECC		_(26)
#define	MIPS_COP_0_CACHE_ERR	_(27)
#define	MIPS_COP_0_TAG_LO	_(28)
#define	MIPS_COP_0_TAG_HI	_(29)
#define	MIPS_COP_0_ERROR_PC	_(30)

/* MIPS32/64 */
#define	MIPS_COP_0_DEBUG	_(23)
#define	MIPS_COP_0_DEPC		_(24)
#define	MIPS_COP_0_PERFCNT	_(25)
#define	MIPS_COP_0_DATA_LO	_(28)
#define	MIPS_COP_0_DATA_HI	_(29)
#define	MIPS_COP_0_DESAVE	_(31)

/*
 * Values for the code field in a break instruction.
 */
#define	MIPS_BREAK_INSTR	0x0000000d
#define	MIPS_BREAK_VAL_MASK	0x03ff0000
#define	MIPS_BREAK_VAL_SHIFT	16
#define	MIPS_BREAK_KDB_VAL	512
#define	MIPS_BREAK_SSTEP_VAL	513
#define	MIPS_BREAK_BRKPT_VAL	514
#define	MIPS_BREAK_SOVER_VAL	515
#define	MIPS_BREAK_KDB		(MIPS_BREAK_INSTR | \
				(MIPS_BREAK_KDB_VAL << MIPS_BREAK_VAL_SHIFT))
#define	MIPS_BREAK_SSTEP	(MIPS_BREAK_INSTR | \
				(MIPS_BREAK_SSTEP_VAL << MIPS_BREAK_VAL_SHIFT))
#define	MIPS_BREAK_BRKPT	(MIPS_BREAK_INSTR | \
				(MIPS_BREAK_BRKPT_VAL << MIPS_BREAK_VAL_SHIFT))
#define	MIPS_BREAK_SOVER	(MIPS_BREAK_INSTR | \
				(MIPS_BREAK_SOVER_VAL << MIPS_BREAK_VAL_SHIFT))

/*
 * Mininum and maximum cache sizes.
 */
#define	MIPS_MIN_CACHE_SIZE	(16 * 1024)
#define	MIPS_MAX_CACHE_SIZE	(256 * 1024)
#define	MIPS3_MAX_PCACHE_SIZE	(32 * 1024)	/* max. primary cache size */

/*
 * The floating point version and status registers.
 */
#define	MIPS_FPU_ID	$0
#define	MIPS_FPU_CSR	$31

/*
 * The floating point coprocessor status register bits.
 */
#define	MIPS_FPU_ROUNDING_BITS		0x00000003
#define	MIPS_FPU_ROUND_RN		0x00000000
#define	MIPS_FPU_ROUND_RZ		0x00000001
#define	MIPS_FPU_ROUND_RP		0x00000002
#define	MIPS_FPU_ROUND_RM		0x00000003
#define	MIPS_FPU_STICKY_BITS		0x0000007c
#define	MIPS_FPU_STICKY_INEXACT		0x00000004
#define	MIPS_FPU_STICKY_UNDERFLOW	0x00000008
#define	MIPS_FPU_STICKY_OVERFLOW	0x00000010
#define	MIPS_FPU_STICKY_DIV0		0x00000020
#define	MIPS_FPU_STICKY_INVALID		0x00000040
#define	MIPS_FPU_ENABLE_BITS		0x00000f80
#define	MIPS_FPU_ENABLE_INEXACT		0x00000080
#define	MIPS_FPU_ENABLE_UNDERFLOW	0x00000100
#define	MIPS_FPU_ENABLE_OVERFLOW	0x00000200
#define	MIPS_FPU_ENABLE_DIV0		0x00000400
#define	MIPS_FPU_ENABLE_INVALID		0x00000800
#define	MIPS_FPU_EXCEPTION_BITS		0x0003f000
#define	MIPS_FPU_EXCEPTION_INEXACT	0x00001000
#define	MIPS_FPU_EXCEPTION_UNDERFLOW	0x00002000
#define	MIPS_FPU_EXCEPTION_OVERFLOW	0x00004000
#define	MIPS_FPU_EXCEPTION_DIV0		0x00008000
#define	MIPS_FPU_EXCEPTION_INVALID	0x00010000
#define	MIPS_FPU_EXCEPTION_UNIMPL	0x00020000
#define	MIPS_FPU_COND_BIT		0x00800000
#define	MIPS_FPU_FLUSH_BIT		0x01000000	/* r4k,	 MBZ on r3k */
#define	MIPS1_FPC_MBZ_BITS		0xff7c0000
#define	MIPS3_FPC_MBZ_BITS		0xfe7c0000


/*
 * Constants to determine if have a floating point instruction.
 */
#define	MIPS_OPCODE_SHIFT	26
#define	MIPS_OPCODE_C1		0x11


/*
 * The low part of the TLB entry.
 */
#define	MIPS1_TLB_PFN			0xfffff000
#define	MIPS1_TLB_NON_CACHEABLE_BIT	0x00000800
#define	MIPS1_TLB_DIRTY_BIT		0x00000400
#define	MIPS1_TLB_VALID_BIT		0x00000200
#define	MIPS1_TLB_GLOBAL_BIT		0x00000100

#define	MIPS3_TLB_PFN			0x3fffffc0
#define	MIPS3_TLB_ATTR_MASK		0x00000038
#define	MIPS3_TLB_ATTR_SHIFT		3
#define	MIPS3_TLB_DIRTY_BIT		0x00000004
#define	MIPS3_TLB_VALID_BIT		0x00000002
#define	MIPS3_TLB_GLOBAL_BIT		0x00000001

#define	MIPS1_TLB_PHYS_PAGE_SHIFT	12
#define	MIPS3_TLB_PHYS_PAGE_SHIFT	6
#define	MIPS1_TLB_PF_NUM		MIPS1_TLB_PFN
#define	MIPS3_TLB_PF_NUM		MIPS3_TLB_PFN
#define	MIPS1_TLB_MOD_BIT		MIPS1_TLB_DIRTY_BIT
#define	MIPS3_TLB_MOD_BIT		MIPS3_TLB_DIRTY_BIT

/*
 * MIPS3_TLB_ATTR values - coherency algorithm:
 * 0: cacheable, noncoherent, write-through, no write allocate
 * 1: cacheable, noncoherent, write-through, write allocate
 * 2: uncached
 * 3: cacheable, noncoherent, write-back (noncoherent)
 * 4: cacheable, coherent, write-back, exclusive (exclusive)
 * 5: cacheable, coherent, write-back, exclusive on write (sharable)
 * 6: cacheable, coherent, write-back, update on write (update)
 * 7: uncached, accelerated (gather STORE operations)
 */
#define	MIPS3_TLB_ATTR_WT		0 /* IDT */
#define	MIPS3_TLB_ATTR_WT_WRITEALLOCATE 1 /* IDT */
#define	MIPS3_TLB_ATTR_UNCACHED		2 /* R4000/R4400, IDT */
#define	MIPS3_TLB_ATTR_WB_NONCOHERENT	3 /* R4000/R4400, IDT */
#define	MIPS3_TLB_ATTR_WB_EXCLUSIVE	4 /* R4000/R4400 */
#define	MIPS3_TLB_ATTR_WB_SHARABLE	5 /* R4000/R4400 */
#define	MIPS3_TLB_ATTR_WB_UPDATE	6 /* R4000/R4400 */
#define	MIPS4_TLB_ATTR_UNCACHED_ACCELERATED 7 /* R10000 */


/*
 * The high part of the TLB entry.
 */
#define	MIPS1_TLB_VPN			0xfffff000
#define	MIPS1_TLB_PID			0x00000fc0
#define	MIPS1_TLB_PID_SHIFT		6

#define	MIPS3_TLB_VPN2			0xffffe000
#define	MIPS3_TLB_ASID			0x000000ff

#define	MIPS1_TLB_VIRT_PAGE_NUM		MIPS1_TLB_VPN
#define	MIPS3_TLB_VIRT_PAGE_NUM		MIPS3_TLB_VPN2
#define	MIPS3_TLB_PID			MIPS3_TLB_ASID
#define	MIPS_TLB_VIRT_PAGE_SHIFT	12

/*
 * r3000: shift count to put the index in the right spot.
 */
#define	MIPS1_TLB_INDEX_SHIFT		8

/*
 * The first TLB that write random hits.
 */
#define	MIPS1_TLB_FIRST_RAND_ENTRY	8
#define	MIPS3_TLB_WIRED_UPAGES		1

/*
 * The number of process id entries.
 */
#define	MIPS1_TLB_NUM_PIDS		64
#define	MIPS3_TLB_NUM_ASIDS		256

/*
 * Patch codes to hide CPU design differences between MIPS1 and MIPS3.
 */

/* XXX simonb: this is before MIPS3_PLUS is defined (and is ugly!) */

#if !(defined(MIPS3) || defined(MIPS4) || defined(MIPS32) || defined(MIPS64)) \
    && defined(MIPS1)				/* XXX simonb must be neater! */
#define	MIPS_TLB_PID_SHIFT		MIPS1_TLB_PID_SHIFT
#define	MIPS_TLB_NUM_PIDS		MIPS1_TLB_NUM_PIDS
#endif

#if (defined(MIPS3) || defined(MIPS4) || defined(MIPS32) || defined(MIPS64)) \
    && !defined(MIPS1)				/* XXX simonb must be neater! */
#define	MIPS_TLB_PID_SHIFT		0
#define	MIPS_TLB_NUM_PIDS		MIPS3_TLB_NUM_ASIDS
#endif


#if !defined(MIPS_TLB_PID_SHIFT)
#define	MIPS_TLB_PID_SHIFT \
    ((MIPS_HAS_R4K_MMU) ? 0 : MIPS1_TLB_PID_SHIFT)

#define	MIPS_TLB_NUM_PIDS \
    ((MIPS_HAS_R4K_MMU) ? MIPS3_TLB_NUM_ASIDS : MIPS1_TLB_NUM_PIDS)
#endif

/*
 * CPU processor revision IDs for company ID == 0 (non mips32/64 chips)
 */
#define	MIPS_R2000	0x01	/* MIPS R2000 			ISA I	*/
#define	MIPS_R3000	0x02	/* MIPS R3000 			ISA I	*/
#define	MIPS_R6000	0x03	/* MIPS R6000 			ISA II	*/
#define	MIPS_R4000	0x04	/* MIPS R4000/R4400 		ISA III */
#define	MIPS_R3LSI	0x05	/* LSI Logic R3000 derivative	ISA I	*/
#define	MIPS_R6000A	0x06	/* MIPS R6000A 			ISA II	*/
#define	MIPS_R3IDT	0x07	/* IDT R3041 or RC36100 	ISA I	*/
#define	MIPS_R10000	0x09	/* MIPS R10000			ISA IV	*/
#define	MIPS_R4200	0x0a	/* NEC VR4200 			ISA III */
#define	MIPS_R4300	0x0b	/* NEC VR4300 			ISA III */
#define	MIPS_R4100	0x0c	/* NEC VR4100 			ISA III */
#define	MIPS_R12000	0x0e	/* MIPS R12000			ISA IV	*/
#define	MIPS_R14000	0x0f	/* MIPS R14000			ISA IV	*/
#define	MIPS_R8000	0x10	/* MIPS R8000 Blackbird/TFP	ISA IV	*/
#define	MIPS_RC32300	0x18	/* IDT RC32334,332,355		ISA 32  */
#define	MIPS_R4600	0x20	/* QED R4600 Orion		ISA III */
#define	MIPS_R4700	0x21	/* QED R4700 Orion		ISA III */
#define	MIPS_R3SONY	0x21	/* Sony R3000 based 		ISA I	*/
#define	MIPS_R4650	0x22	/* QED R4650 			ISA III */
#define	MIPS_TX3900	0x22	/* Toshiba TX39 family		ISA I	*/
#define	MIPS_R5000	0x23	/* MIPS R5000 			ISA IV	*/
#define	MIPS_R3NKK	0x23	/* NKK R3000 based 		ISA I	*/
#define	MIPS_RC32364	0x26	/* IDT RC32364 			ISA 32	*/
#define	MIPS_RM7000	0x27	/* QED RM7000			ISA IV  */
#define	MIPS_RM5200	0x28	/* QED RM5200s 			ISA IV	*/
#define	MIPS_TX4900	0x2d	/* Toshiba TX49 family		ISA III */
#define	MIPS_R5900	0x2e	/* Toshiba R5900 (EECore)	ISA --- */
#define	MIPS_RC64470	0x30	/* IDT RC64474/RC64475 		ISA III */
#define	MIPS_TX7900	0x38	/* Toshiba TX79			ISA III+*/
#define	MIPS_R5400	0x54	/* NEC VR5400 			ISA IV	*/
#define	MIPS_R5500	0x55	/* NEC VR5500 			ISA IV	*/
#define	MIPS_LOONGSON2	0x63	/* ICT Loongson-2		ISA III	*/

/*
 * CPU revision IDs for some prehistoric processors.
 */

/* For MIPS_R3000 */
#define	MIPS_REV_R2000A		0x16	/* R2000A uses R3000 proc revision */
#define	MIPS_REV_R3000		0x20
#define	MIPS_REV_R3000A		0x30

/* For MIPS_TX3900 */
#define	MIPS_REV_TX3912		0x10
#define	MIPS_REV_TX3922		0x30
#define	MIPS_REV_TX3927		0x40

/* For MIPS_R4000 */
#define	MIPS_REV_R4000_A	0x00
#define	MIPS_REV_R4000_B	0x22
#define	MIPS_REV_R4000_C	0x30
#define	MIPS_REV_R4400_A	0x40
#define	MIPS_REV_R4400_B	0x50
#define	MIPS_REV_R4400_C	0x60

/* For MIPS_TX4900 */
#define	MIPS_REV_TX4927		0x22

/* For MIPS_LOONGSON2 */
#define	MIPS_REV_LOONGSON2E	0x02
#define	MIPS_REV_LOONGSON2F	0x03

/*
 * CPU processor revision IDs for company ID == 1 (MIPS)
 */
#define	MIPS_4Kc	0x80	/* MIPS 4Kc			ISA 32  */
#define	MIPS_5Kc	0x81	/* MIPS 5Kc			ISA 64  */
#define	MIPS_20Kc	0x82	/* MIPS 20Kc			ISA 64  */
#define	MIPS_4Kmp	0x83	/* MIPS 4Km/4Kp			ISA 32  */
#define	MIPS_4KEc	0x84	/* MIPS 4KEc			ISA 32  */
#define	MIPS_4KEmp	0x85	/* MIPS 4KEm/4KEp		ISA 32  */
#define	MIPS_4KSc	0x86	/* MIPS 4KSc			ISA 32  */
#define	MIPS_M4K	0x87	/* MIPS M4K			ISA 32  Rel 2 */
#define	MIPS_25Kf	0x88	/* MIPS 25Kf			ISA 64  */
#define	MIPS_5KE	0x89	/* MIPS 5KE			ISA 64  Rel 2 */
#define	MIPS_4KEc_R2	0x90	/* MIPS 4KEc_R2			ISA 32  Rel 2 */
#define	MIPS_4KEmp_R2	0x91	/* MIPS 4KEm/4KEp_R2		ISA 32  Rel 2 */
#define	MIPS_4KSd	0x92	/* MIPS 4KSd			ISA 32  Rel 2 */
#define	MIPS_24K	0x93	/* MIPS 24Kc/24Kf		ISA 32  Rel 2 */
#define	MIPS_34K	0x95	/* MIPS 34K			ISA 32  R2 MT */
#define	MIPS_24KE	0x96	/* MIPS 24KEc			ISA 32  Rel 2 */
#define	MIPS_74K	0x97	/* MIPS 74Kc/74Kf		ISA 32  Rel 2 */

/*
 * Alchemy (company ID 3) use the processor ID field to donote the CPU core
 * revision and the company options field do donate the SOC chip type.
 */
/* CPU processor revision IDs */
#define	MIPS_AU_REV1	0x01	/* Alchemy Au1000 (Rev 1)	ISA 32  */
#define	MIPS_AU_REV2	0x02	/* Alchemy Au1000 (Rev 2)	ISA 32  */
/* CPU company options IDs */
#define	MIPS_AU1000	0x00
#define	MIPS_AU1500	0x01
#define	MIPS_AU1100	0x02
#define	MIPS_AU1550	0x03

/*
 * CPU processor revision IDs for company ID == 4 (SiByte)
 */
#define	MIPS_SB1	0x01	/* SiByte SB1	 		ISA 64  */

/*
 * CPU processor revision IDs for company ID == 5 (SandCraft)
 */
#define	MIPS_SR7100	0x04	/* SandCraft SR7100 		ISA 64  */

/*
 * CPU processor revision IDs for company ID == 12 (RMI)
 */
#define	MIPS_XLR732	0x00	/* RMI XLR732-C	 		ISA 64  */
#define	MIPS_XLR716	0x02	/* RMI XLR716-C	 		ISA 64  */
#define	MIPS_XLR532	0x08	/* RMI XLR532-C	 		ISA 64  */
#define	MIPS_XLR516	0x0a	/* RMI XLR516-C	 		ISA 64  */
#define	MIPS_XLR508	0x0b	/* RMI XLR508-C	 		ISA 64  */
#define	MIPS_XLR308	0x0f	/* RMI XLR308-C	 		ISA 64  */
#define	MIPS_XLS616	0x40	/* RMI XLS616	 		ISA 64  */
#define	MIPS_XLS416	0x44	/* RMI XLS416	 		ISA 64  */
#define	MIPS_XLS608	0x4A	/* RMI XLS608	 		ISA 64  */
#define	MIPS_XLS408	0x4E	/* RMI XLS406	 		ISA 64  */
#define	MIPS_XLS404	0x4F	/* RMI XLS404	 		ISA 64  */
#define	MIPS_XLS408LITE	0x88	/* RMI XLS408-Lite		ISA 64  */
#define	MIPS_XLS404LITE	0x8C	/* RMI XLS404-Lite	 	ISA 64  */
#define	MIPS_XLS208	0x8E	/* RMI XLS208	 		ISA 64  */
#define	MIPS_XLS204	0x8F	/* RMI XLS204	 		ISA 64  */
#define	MIPS_XLS108	0xCE	/* RMI XLS108	 		ISA 64  */
#define	MIPS_XLS104	0xCF	/* RMI XLS104	 		ISA 64  */

/*
 * FPU processor revision ID
 */
#define	MIPS_SOFT	0x00	/* Software emulation		ISA I	*/
#define	MIPS_R2360	0x01	/* MIPS R2360 FPC		ISA I	*/
#define	MIPS_R2010	0x02	/* MIPS R2010 FPC		ISA I	*/
#define	MIPS_R3010	0x03	/* MIPS R3010 FPC		ISA I	*/
#define	MIPS_R6010	0x04	/* MIPS R6010 FPC		ISA II	*/
#define	MIPS_R4010	0x05	/* MIPS R4010 FPC		ISA II	*/
#define	MIPS_R31LSI	0x06	/* LSI Logic derivate		ISA I	*/
#define	MIPS_R3TOSH	0x22	/* Toshiba R3000 based FPU	ISA I	*/

#endif /* _MIPS_CPUREGS_H_ */

/*	$NetBSD: cpu.h,v 1.94 2009/12/14 00:46:04 matt Exp $	*/

/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell and Rick Macklem.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)cpu.h	8.4 (Berkeley) 1/4/94
 */

#ifndef _CPU_H_
#define _CPU_H_


/*
 * bitfield defines for cpu_cp0flags
 */
#define  MIPS_CP0FL_USE		__BIT(0)	/* use these flags */
#define  MIPS_CP0FL_ECC		__BIT(1)
#define  MIPS_CP0FL_CACHE_ERR	__BIT(2)
#define  MIPS_CP0FL_EIRR	__BIT(3)
#define  MIPS_CP0FL_EIMR	__BIT(4)
#define  MIPS_CP0FL_EBASE	__BIT(5)
#define  MIPS_CP0FL_CONFIG	__BIT(6)
#define  MIPS_CP0FL_CONFIGn(n)	(__BIT(7) << ((n) & 7))

/*
 * cpu_cidflags defines, by company
 */
/*
 * RMI company-specific cpu_cidflags
 */
#define MIPS_CIDFL_RMI_TYPE     __BITS(0,2)
#define  CIDFL_RMI_TYPE_XLR     0
#define  CIDFL_RMI_TYPE_XLS     1
#define  CIDFL_RMI_TYPE_XLP     2


#define	CPU_INFO_ITERATOR		int
#define	CPU_INFO_FOREACH(cii, ci)	\
    (void)(cii), ci = &cpu_info_store; ci != NULL; ci = ci->ci_next


/*
 * CTL_MACHDEP definitions.
 */
#define CPU_CONSDEV		1	/* dev_t: console terminal device */
#define CPU_BOOTED_KERNEL	2	/* string: booted kernel name */
#define CPU_ROOT_DEVICE		3	/* string: root device name */
#define CPU_LLSC		4	/* OS/CPU supports LL/SC instruction */

/*
 * Platform can override, but note this breaks userland compatibility
 * with other mips platforms.
 */
#ifndef CPU_MAXID
#define CPU_MAXID		5	/* number of valid machdep ids */

#endif

#ifdef _KERNEL
#if defined(_LKM) || defined(_STANDALONE)
/* Assume all CPU architectures are valid for LKM's and standlone progs */
#define	MIPS1	1
#define	MIPS3	1
#define	MIPS4	1
#define	MIPS32	1
#define	MIPS64	1
#endif

#if (MIPS1 + MIPS3 + MIPS4 + MIPS32 + MIPS64) == 0
#error at least one of MIPS1, MIPS3, MIPS4, MIPS32 or MIPS64 must be specified
#endif

/* Shortcut for MIPS3 or above defined */
#if defined(MIPS3) || defined(MIPS4) || defined(MIPS32) || defined(MIPS64)
#define	MIPS3_PLUS	1
#else
#undef MIPS3_PLUS
#endif

/*
 * Macros to find the CPU architecture we're on at run-time,
 * or if possible, at compile-time.
 */

#define	CPU_ARCH_MIPSx	0		/* XXX unknown */
#define	CPU_ARCH_MIPS1	(1 << 0)
#define	CPU_ARCH_MIPS2	(1 << 1)
#define	CPU_ARCH_MIPS3	(1 << 2)
#define	CPU_ARCH_MIPS4	(1 << 3)
#define	CPU_ARCH_MIPS5	(1 << 4)
#define	CPU_ARCH_MIPS32	(1 << 5)
#define	CPU_ARCH_MIPS64	(1 << 6)

/* Note: must be kept in sync with -ffixed-?? Makefile.mips. */
#define MIPS_CURLWP             $23
#define MIPS_CURLWP_QUOTED      "$23"
#define MIPS_CURLWP_CARD	23
#define	MIPS_CURLWP_FRAME(x)	FRAME_S7(x)

#ifndef _LOCORE

#define	curlwp			mips_curlwp
#define	curcpu()		(curlwp->l_cpu)
#define	curpcb			((struct pcb *)lwp_getpcb(curlwp))
#define	fpcurlwp		(curcpu()->ci_fpcurlwp)
#define	cpu_number()		(0)
#define	cpu_proc_fork(p1, p2)	((void)((p2)->p_md.md_abi = (p1)->p_md.md_abi))

/* XXX simonb
 * Should the following be in a cpu_info type structure?
 * And how many of these are per-cpu vs. per-system?  (Ie,
 * we can assume that all cpus have the same mmu-type, but
 * maybe not that all cpus run at the same clock speed.
 * Some SGI's apparently support R12k and R14k in the same
 * box.)
 */

#define	CPU_MIPS_R4K_MMU		0x0001
#define	CPU_MIPS_NO_LLSC		0x0002
#define	CPU_MIPS_CAUSE_IV		0x0004
#define	CPU_MIPS_HAVE_SPECIAL_CCA	0x0008	/* Defaults to '3' if not set. */
#define	CPU_MIPS_CACHED_CCA_MASK	0x0070
#define	CPU_MIPS_CACHED_CCA_SHIFT	 4
#define	CPU_MIPS_DOUBLE_COUNT		0x0080	/* 1 cp0 count == 2 clock cycles */
#define	CPU_MIPS_USE_WAIT		0x0100	/* Use "wait"-based cpu_idle() */
#define	CPU_MIPS_NO_WAIT		0x0200	/* Inverse of previous, for mips32/64 */
#define	CPU_MIPS_D_CACHE_COHERENT	0x0400	/* D-cache is fully coherent */
#define	CPU_MIPS_I_D_CACHE_COHERENT	0x0800	/* I-cache funcs don't need to flush the D-cache */
#define	CPU_MIPS_NO_LLADDR		0x1000
#define	CPU_MIPS_HAVE_MxCR		0x2000	/* have mfcr, mtcr insns */
#define	MIPS_NOT_SUPP			0x8000

#endif	/* !_LOCORE */

#if ((MIPS1 + MIPS3 + MIPS4 + MIPS32 + MIPS64) == 1) || defined(_LOCORE)

#if defined(MIPS1)

# define CPUISMIPS3		0
# define CPUIS64BITS		0
# define CPUISMIPS32		0
# define CPUISMIPS64		0
# define CPUISMIPSNN		0
# define MIPS_HAS_R4K_MMU	0
# define MIPS_HAS_CLOCK		0
# define MIPS_HAS_LLSC		0
# define MIPS_HAS_LLADDR	0

#elif defined(MIPS3) || defined(MIPS4)

# define CPUISMIPS3		1
# define CPUIS64BITS		1
# define CPUISMIPS32		0
# define CPUISMIPS64		0
# define CPUISMIPSNN		0
# define MIPS_HAS_R4K_MMU	1
# define MIPS_HAS_CLOCK		1
# if defined(_LOCORE)
#  if !defined(MIPS3_5900) && !defined(MIPS3_4100)
#   define MIPS_HAS_LLSC	1
#  else
#   define MIPS_HAS_LLSC	0
#  endif
# else	/* _LOCORE */
#  define MIPS_HAS_LLSC		(mips_has_llsc)
# endif	/* _LOCORE */
# define MIPS_HAS_LLADDR	((mips_cpu_flags & CPU_MIPS_NO_LLADDR) == 0)

#elif defined(MIPS32)

# define CPUISMIPS3		1
# define CPUIS64BITS		0
# define CPUISMIPS32		1
# define CPUISMIPS64		0
# define CPUISMIPSNN		1
# define MIPS_HAS_R4K_MMU	1
# define MIPS_HAS_CLOCK		1
# define MIPS_HAS_LLSC		1
# define MIPS_HAS_LLADDR	((mips_cpu_flags & CPU_MIPS_NO_LLADDR) == 0)

#elif defined(MIPS64)

# define CPUISMIPS3		1
# define CPUIS64BITS		1
# define CPUISMIPS32		0
# define CPUISMIPS64		1
# define CPUISMIPSNN		1
# define MIPS_HAS_R4K_MMU	1
# define MIPS_HAS_CLOCK		1
# define MIPS_HAS_LLSC		1
# define MIPS_HAS_LLADDR	((mips_cpu_flags & CPU_MIPS_NO_LLADDR) == 0)

#endif

#else /* run-time test */

#ifndef	_LOCORE

#define	MIPS_HAS_R4K_MMU	(mips_has_r4k_mmu)
#define	MIPS_HAS_LLSC		(mips_has_llsc)
#define	MIPS_HAS_LLADDR		((mips_cpu_flags & CPU_MIPS_NO_LLADDR) == 0)

/* This test is ... rather bogus */
#define	CPUISMIPS3	((cpu_arch & \
	(CPU_ARCH_MIPS3 | CPU_ARCH_MIPS4 | CPU_ARCH_MIPS32 | CPU_ARCH_MIPS64)) != 0)

/* And these aren't much better while the previous test exists as is... */
#define	CPUISMIPS32	((cpu_arch & CPU_ARCH_MIPS32) != 0)
#define	CPUISMIPS64	((cpu_arch & CPU_ARCH_MIPS64) != 0)
#define	CPUISMIPSNN	((cpu_arch & (CPU_ARCH_MIPS32 | CPU_ARCH_MIPS64)) != 0)
#define	CPUIS64BITS	((cpu_arch & \
	(CPU_ARCH_MIPS3 | CPU_ARCH_MIPS4 | CPU_ARCH_MIPS64)) != 0)

#define	MIPS_HAS_CLOCK	(cpu_arch >= CPU_ARCH_MIPS3)

#else	/* !_LOCORE */

#define	MIPS_HAS_LLSC	0

#endif	/* !_LOCORE */

#endif /* run-time test */

#ifndef	_LOCORE

/*
 * A port must provde CLKF_USERMODE() for use in machine-independent code.
 * These differ on r4000 and r3000 systems; provide them in the
 * port-dependent file that includes this one, using the macros below.
 */

/* mips1 versions */
#define	MIPS1_CLKF_USERMODE(framep)	((framep)->sr & MIPS_SR_KU_PREV)

/* mips3 versions */
#define	MIPS3_CLKF_USERMODE(framep)	((framep)->sr & MIPS_SR_KSU_USER)

#define	CLKF_PC(framep)		((framep)->pc)
#define	CLKF_INTR(framep)	(0)

#if defined(MIPS3_PLUS) && !defined(MIPS1)		/* XXX bogus! */
#define	CLKF_USERMODE(framep)	MIPS3_CLKF_USERMODE(framep)
#endif

#if !defined(MIPS3_PLUS) && defined(MIPS1)		/* XXX bogus! */
#define	CLKF_USERMODE(framep)	MIPS1_CLKF_USERMODE(framep)
#endif

#if defined(MIPS3_PLUS) && defined(MIPS1)		/* XXX bogus! */
#define CLKF_USERMODE(framep) \
    ((CPUISMIPS3) ? MIPS3_CLKF_USERMODE(framep):  MIPS1_CLKF_USERMODE(framep))
#endif

/*
 * This is used during profiling to integrate system time.  It can safely
 * assume that the process is resident.
 */
#define	PROC_PC(p)							\
	(((struct frame *)(p)->p_md.md_regs)->f_regs[37])	/* XXX PC */

/*
 * Preempt the current process if in interrupt from user mode,
 * or after the current trap/syscall if in system mode.
 */

/*
 * Give a profiling tick to the current process when the user profiling
 * buffer pages are invalid.  On the MIPS, request an ast to send us
 * through trap, marking the proc as needing a profiling tick.
 */
#define	cpu_need_proftick(l)						\
do {									\
	(l)->l_pflag |= LP_OWEUPC;					\
	aston(l);							\
} while (/*CONSTCOND*/0)

/*
 * Notify the current lwp (l) that it has a signal pending,
 * process as soon as possible.
 */
#define	cpu_signotify(l)	aston(l)

#define aston(l)		((l)->l_md.md_astpending = 1)


#endif /* ! _LOCORE */
#endif /* _KERNEL */
#endif /* _CPU_H_ */


/*	$NetBSD: mips_opcode.h,v 1.13 2009/08/06 04:34:50 msaitoh Exp $	*/

/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)mips_opcode.h	8.1 (Berkeley) 6/10/93
 */

/*
 * Define the instruction formats and opcode values for the
 * MIPS instruction set.
 */

/*
 * Define the instruction formats.
 */
 
 
typedef union {
	unsigned word;
	struct {
		unsigned imm: 16;
		unsigned rt: 5;
		unsigned rs: 5;
		unsigned op: 6;
	} IType;

	struct {
		unsigned target: 26;
		unsigned op: 6;
	} JType;

	struct {
		unsigned func: 6;
		unsigned shamt: 5;
		unsigned rd: 5;
		unsigned rt: 5;
		unsigned rs: 5;
		unsigned op: 6;
	} RType;

	struct {
		unsigned func: 6;
		unsigned fd: 5;
		unsigned fs: 5;
		unsigned ft: 5;
		unsigned fmt: 4;
		unsigned : 1;		/* always '1' */
		unsigned op: 6;		/* always '0x11' */
	} FRType;
} InstFmt;

/*
 * Values for the 'op' field.
 */
#define OP_SPECIAL	000
#define OP_BCOND	001
#define OP_J		002
#define	OP_JAL		003
#define OP_BEQ		004
#define OP_BNE		005
#define OP_BLEZ		006
#define OP_BGTZ		007

#define OP_ADDI		010
#define OP_ADDIU	011
#define OP_SLTI		012
#define OP_SLTIU	013
#define OP_ANDI		014
#define OP_ORI		015
#define OP_XORI		016
#define OP_LUI		017

#define OP_COP0		020
#define OP_COP1		021
#define OP_COP2		022
#define OP_COP3		023
#define OP_BEQL		024		/* MIPS-II, for r4000 port */
#define OP_BNEL		025		/* MIPS-II, for r4000 port */
#define OP_BLEZL	026		/* MIPS-II, for r4000 port */
#define OP_BGTZL	027		/* MIPS-II, for r4000 port */

#define OP_DADDI	030		/* MIPS-II, for r4000 port */
#define OP_DADDIU	031		/* MIPS-II, for r4000 port */
#define OP_LDL		032		/* MIPS-II, for r4000 port */
#define OP_LDR		033		/* MIPS-II, for r4000 port */

#define OP_SPECIAL2	034		/* QED opcodes */

#define OP_LB		040
#define OP_LH		041
#define OP_LWL		042
#define OP_LW		043
#define OP_LBU		044
#define OP_LHU		045
#define OP_LWR		046
#define OP_LHU		045
#define OP_LWR		046
#define OP_LWU		047		/* MIPS-II, for r4000 port */

#define OP_SB		050
#define OP_SH		051
#define OP_SWL		052
#define OP_SW		053
#define OP_SDL		054		/* MIPS-II, for r4000 port */
#define OP_SDR		055		/* MIPS-II, for r4000 port */
#define OP_SWR		056
#define OP_CACHE	057		/* MIPS-II, for r4000 port */

#define OP_LL		060
#define OP_LWC0		OP_LL	/* backwards source compatibility */
#define OP_LWC1		061
#define OP_LWC2		062
#define OP_LWC3		063
#define OP_LLD		064		/* MIPS-II, for r4000 port */
#define OP_LDC1		065
#define OP_LD		067		/* MIPS-II, for r4000 port */

#define OP_SC		070
#define OP_SWC0		OP_SC	/* backwards source compatibility */
#define OP_SWC1		071
#define OP_SWC2		072
#define OP_SWC3		073
#define OP_SCD		074		/* MIPS-II, for r4000 port */
#define OP_SDC1		075
#define OP_SD		077		/* MIPS-II, for r4000 port */

/*
 * Values for the 'func' field when 'op' == OP_SPECIAL.
 */
#define OP_SLL		000
#define OP_SRL		002
#define OP_SRA		003
#define OP_SLLV		004
#define OP_SRLV		006
#define OP_SRAV		007

#define OP_JR		010
#define OP_JALR		011
#define OP_SYSCALL	014
#define OP_BREAK	015
#define OP_SYNC		017		/* MIPS-II, for r4000 port */

#define OP_MFHI		020
#define OP_MTHI		021
#define OP_MFLO		022
#define OP_MTLO		023
#define OP_DSLLV	024		/* MIPS-II, for r4000 port */
#define OP_DSRLV	026		/* MIPS-II, for r4000 port */
#define OP_DSRAV	027		/* MIPS-II, for r4000 port */

#define OP_MULT		030
#define OP_MULTU	031
#define OP_DIV		032
#define OP_DIVU		033
#define OP_DMULT	034		/* MIPS-II, for r4000 port */
#define OP_DMULTU	035		/* MIPS-II, for r4000 port */
#define OP_DDIV		036		/* MIPS-II, for r4000 port */
#define OP_DDIVU	037		/* MIPS-II, for r4000 port */

#define OP_ADD		040
#define OP_ADDU		041
#define OP_SUB		042
#define OP_SUBU		043
#define OP_AND		044
#define OP_OR		045
#define OP_XOR		046
#define OP_NOR		047

#define OP_SLT		052
#define OP_SLTU		053
#define OP_DADD		054		/* MIPS-II, for r4000 port */
#define OP_DADDU	055		/* MIPS-II, for r4000 port */
#define OP_DSUB		056		/* MIPS-II, for r4000 port */
#define OP_DSUBU	057		/* MIPS-II, for r4000 port */

#define OP_TGE		060		/* MIPS-II, for r4000 port */
#define OP_TGEU		061		/* MIPS-II, for r4000 port */
#define OP_TLT		062		/* MIPS-II, for r4000 port */
#define OP_TLTU		063		/* MIPS-II, for r4000 port */
#define OP_TEQ		064		/* MIPS-II, for r4000 port */
#define OP_TNE		066		/* MIPS-II, for r4000 port */

#define OP_DSLL		070		/* MIPS-II, for r4000 port */
#define OP_DSRL		072		/* MIPS-II, for r4000 port */
#define OP_DSRA		073		/* MIPS-II, for r4000 port */
#define OP_DSLL32	074		/* MIPS-II, for r4000 port */
#define OP_DSRL32	076		/* MIPS-II, for r4000 port */
#define OP_DSRA32	077		/* MIPS-II, for r4000 port */

/*
 * Values for the 'func' field when 'op' == OP_SPECIAL2.
 */
#define OP_MAD		000		/* QED */
#define OP_MADU		001		/* QED */
#define OP_MUL		002		/* QED */

/*
 * Values for the 'func' field when 'op' == OP_BCOND.
 */
#define OP_BLTZ		000
#define OP_BGEZ		001
#define OP_BLTZL	002		/* MIPS-II, for r4000 port */
#define OP_BGEZL	003		/* MIPS-II, for r4000 port */

#define OP_TGEI		010		/* MIPS-II, for r4000 port */
#define OP_TGEIU	011		/* MIPS-II, for r4000 port */
#define OP_TLTI		012		/* MIPS-II, for r4000 port */
#define OP_TLTIU	013		/* MIPS-II, for r4000 port */
#define OP_TEQI		014		/* MIPS-II, for r4000 port */
#define OP_TNEI		016		/* MIPS-II, for r4000 port */

#define OP_BLTZAL	020		/* MIPS-II, for r4000 port */
#define OP_BGEZAL	021
#define OP_BLTZALL	022
#define OP_BGEZALL	023

/*
 * Values for the 'rs' field when 'op' == OP_COPz.
 */
#define OP_MF		000
#define OP_DMF		001		/* MIPS-II, for r4000 port */
#define OP_CF		002
#define OP_MFH		003
#define OP_MT		004
#define OP_DMT		005		/* MIPS-II, for r4000 port */
#define OP_CT		006
#define OP_MTH		007
#define OP_BCx		010
#define OP_BCy		014

/*
 * Values for the 'rt' field when 'op' == OP_COPz.
 */
#define COPz_BC_TF_MASK	0x01
#define COPz_BC_TRUE	0x01
#define COPz_BC_FALSE	0x00
#define COPz_BCL_TF_MASK	0x02		/* MIPS-II, for r4000 port */
#define COPz_BCL_TRUE	0x02		/* MIPS-II, for r4000 port */
#define COPz_BCL_FALSE	0x00		/* MIPS-II, for r4000 port */

#endif /* __DECODER_LOCAL_H__ */

