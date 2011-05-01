/* Saves/loads most user-level PowerPC registers */

#ifndef PPC_REGS_H
#define PPC_REGS_H

#include <stdint.h>

typedef __attribute__((aligned(16))) struct ppc_regs_t
{
	uint64_t	vr  [32] [2];
	double		fpr [32];
	uint64_t	gpr [32];
	uint32_t	cr;
	uint32_t	vrsave;
	uint32_t	ctr;
	uint32_t	xer;
	uint32_t	garbage;
	uint32_t	fpscr;
} ppc_regs_t;

#ifdef __cplusplus
	extern "C" {
#endif

void save_ppc_regs( ppc_regs_t* );
void load_ppc_regs( ppc_regs_t const* );

#ifdef __cplusplus
	}
#endif

#endif
