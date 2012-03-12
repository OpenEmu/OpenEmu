#include "sys16.h"
#include "fd1094.h"

#define S16_NUMCACHE 8

static UINT8 *fd1094_key; // the memory region containing key
static UINT16 *fd1094_cpuregion; // the CPU region with encrypted code
static UINT32  fd1094_cpuregionsize; // the size of this region in bytes

static UINT16* fd1094_userregion; // a user region where the current decrypted state is put and executed from
static UINT16* fd1094_cacheregion[S16_NUMCACHE]; // a cache region where S16_NUMCACHE states are stored to improve performance
static INT32 fd1094_cached_states[S16_NUMCACHE]; // array of cached state numbers
static INT32 fd1094_current_cacheposition; // current position in cache array

static INT32 fd1094_state;
static INT32 fd1094_selected_state;

static INT32 nFD1094CPU = 0;

bool System18Banking;
/*
static void *fd1094_get_decrypted_base(void)
{
	if (!fd1094_key)
		return NULL;
	return fd1094_userregion;
}*/

/* this function checks the cache to see if the current state is cached,
   if it is then it copies the cached data to the user region where code is
   executed from, if its not cached then it gets decrypted to the current
   cache position using the functions in fd1094.c */
static void fd1094_setstate_and_decrypt(INT32 state)
{
	INT32 i;
	UINT32 addr;
	
	switch (state & 0x300) {
		case 0x000:
		case FD1094_STATE_RESET:
			fd1094_selected_state = state & 0xff;
		break;
	}

	fd1094_state = state;

	// force a flush of the prefetch cache
	m68k_set_reg(M68K_REG_PREF_ADDR, 0x1000);
	
	/* set the FD1094 state ready to decrypt.. */
	state = fd1094_set_state(fd1094_key,state);

	/* first check the cache, if its cached we don't need to decrypt it, just copy */
	for (i=0;i<S16_NUMCACHE;i++)
	{
		if (fd1094_cached_states[i] == state)
		{
			/* copy cached state */
			fd1094_userregion=fd1094_cacheregion[i];
			INT32 nActiveCPU = SekGetActive();
			if (nActiveCPU == -1) {
				SekOpen(nFD1094CPU);
				SekMapMemory((UINT8*)fd1094_userregion, 0x000000, 0x0fffff, SM_FETCH);
				if (System18Banking) SekMapMemory((UINT8*)fd1094_userregion + 0x200000, 0x200000, 0x27ffff, SM_FETCH);
				SekClose();
			} else {
				if (nActiveCPU == nFD1094CPU) {
					SekMapMemory((UINT8*)fd1094_userregion, 0x000000, 0x0fffff, SM_FETCH);
					if (System18Banking) SekMapMemory((UINT8*)fd1094_userregion + 0x200000, 0x200000, 0x27ffff, SM_FETCH);
				} else {
					SekClose();
					SekOpen(nFD1094CPU);
					SekMapMemory((UINT8*)fd1094_userregion, 0x000000, 0x0fffff, SM_FETCH);
					if (System18Banking) SekMapMemory((UINT8*)fd1094_userregion + 0x200000, 0x200000, 0x27ffff, SM_FETCH);
					SekClose();
					SekOpen(nActiveCPU);
				}
			}

			return;
		}
	}

	/* mark it as cached (because it will be once we decrypt it) */
	fd1094_cached_states[fd1094_current_cacheposition]=state;

	for (addr=0;addr<fd1094_cpuregionsize/2;addr++)
	{
		UINT16 dat;
		dat = fd1094_decode(addr,fd1094_cpuregion[addr],fd1094_key,0);
		fd1094_cacheregion[fd1094_current_cacheposition][addr]=dat;
	}

	/* copy newly decrypted data to user region */
	fd1094_userregion=fd1094_cacheregion[fd1094_current_cacheposition];
	INT32 nActiveCPU = SekGetActive();
	if (nActiveCPU == -1) {
		SekOpen(nFD1094CPU);
		SekMapMemory((UINT8*)fd1094_userregion, 0x000000, 0x0fffff, SM_FETCH);
		if (System18Banking) SekMapMemory((UINT8*)fd1094_userregion + 0x200000, 0x200000, 0x27ffff, SM_FETCH);
		SekClose();
	} else {
		if (nActiveCPU == nFD1094CPU) {
			SekMapMemory((UINT8*)fd1094_userregion, 0x000000, 0x0fffff, SM_FETCH);
			if (System18Banking) SekMapMemory((UINT8*)fd1094_userregion + 0x200000, 0x200000, 0x27ffff, SM_FETCH);
		} else {
			SekClose();
			SekOpen(nFD1094CPU);
			SekMapMemory((UINT8*)fd1094_userregion, 0x000000, 0x0fffff, SM_FETCH);
			if (System18Banking) SekMapMemory((UINT8*)fd1094_userregion + 0x200000, 0x200000, 0x27ffff, SM_FETCH);
			SekClose();
			SekOpen(nActiveCPU);
		}
	}
	
	fd1094_current_cacheposition++;

	if (fd1094_current_cacheposition>=S16_NUMCACHE)
	{
#if 1 && defined FBA_DEBUG
		bprintf(PRINT_NORMAL, _T("out of cache, performance may suffer, increase S16_NUMCACHE!\n"));
#endif
		fd1094_current_cacheposition=0;
	}
}

/* Callback for CMP.L instructions (state change) */
INT32 __fastcall fd1094_cmp_callback(UINT32 val, INT32 reg)
{
	if (reg == 0 && (val & 0x0000ffff) == 0x0000ffff) // ?
	{
		fd1094_setstate_and_decrypt((val & 0xffff0000) >> 16);
	}
	
	return 0;
}

/* Callback when the FD1094 enters interrupt code */
INT32 __fastcall fd1094_int_callback (INT32 irq)
{
	fd1094_setstate_and_decrypt(FD1094_STATE_IRQ);
	return (0x60+irq*4)/4; // vector address
}

INT32 __fastcall fd1094_rte_callback (void)
{
	fd1094_setstate_and_decrypt(FD1094_STATE_RTE);
	
	return 0;
}

void fd1094_kludge_reset_values(void)
{
	INT32 i;

	for (i = 0;i < 4;i++) {
		fd1094_userregion[i] = fd1094_decode(i,fd1094_cpuregion[i],fd1094_key,1);
	}
		
	SekOpen(nFD1094CPU);
	SekMapMemory((UINT8*)fd1094_userregion, 0x000000, 0x0fffff, SM_FETCH);
	if (System18Banking) SekMapMemory((UINT8*)fd1094_userregion + 0x200000, 0x200000, 0x27ffff, SM_FETCH);
	SekClose();
}


/* function, to be called from MACHINE_RESET (every reset) */
void fd1094_machine_init(void)
{
	fd1094_setstate_and_decrypt(FD1094_STATE_RESET);
	fd1094_kludge_reset_values();

	SekOpen(nFD1094CPU);
	SekSetCmpCallback(fd1094_cmp_callback);
	SekSetRTECallback(fd1094_rte_callback);
	SekSetIrqCallback(fd1094_int_callback);
	SekClose();
}

/* startup function, to be called from DRIVER_INIT (once on startup) */
void fd1094_driver_init(INT32 nCPU)
{
	INT32 i;
	
	nFD1094CPU = nCPU;

	if (nFD1094CPU == 0) {
		fd1094_cpuregion = (UINT16*)System16Rom;
		fd1094_cpuregionsize = System16RomSize;
	}
	
	if (nFD1094CPU == 1) {
		fd1094_cpuregion = (UINT16*)System16Rom2;
		fd1094_cpuregionsize = System16Rom2Size;
	}
	
	if (nFD1094CPU >= 2) {
		bprintf(PRINT_ERROR, _T("Invalid CPU called for FD1094 Driver Init\n"));
	}
	
	fd1094_key = System16Key;

	/* punt if no key; this allows us to be called even for non-FD1094 games */
	if (!fd1094_key)
		return;
		
	for (i=0;i<S16_NUMCACHE;i++)
	{
		fd1094_cacheregion[i]=(UINT16*)BurnMalloc(fd1094_cpuregionsize);
	}

	/* flush the cached state array */
	for (i=0;i<S16_NUMCACHE;i++) fd1094_cached_states[i] = -1;
	
	fd1094_current_cacheposition = 0;
	fd1094_state = -1;
	
	if (System16RomSize > 0x0fffff) System18Banking = true;
}

void fd1094_exit()
{
	System18Banking = false;
	nFD1094CPU = 0;
	
	for (INT32 i = 0; i < S16_NUMCACHE; i++) {
		BurnFree(fd1094_cacheregion[i]);
	}
	
	fd1094_current_cacheposition = 0;
}

void fd1094_scan(INT32 nAction)
{
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(fd1094_selected_state);
		SCAN_VAR(fd1094_state);
		
		if (nAction & ACB_WRITE) {
			if (fd1094_state != -1)	{
				INT32 selected_state = fd1094_selected_state;
				INT32 state = fd1094_state;

				fd1094_machine_init();

				fd1094_setstate_and_decrypt(selected_state);
				fd1094_setstate_and_decrypt(state);
			}
		}
	}
}
