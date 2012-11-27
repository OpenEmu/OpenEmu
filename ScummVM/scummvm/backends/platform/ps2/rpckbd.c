/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# USB Keyboard Driver for PS2 using RPC instead of FIO
*/

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <string.h>
#include "backends/platform/ps2/rpckbd.h"

static int curr_readmode = PS2KBD_READMODE_NORMAL;
static int kbdRpcSema = -1;
static int kbdInitialized = 0;

static SifRpcClientData_t cd0;
static unsigned char rpcBuf[3 * PS2KBD_KEYMAP_SIZE] __attribute__((aligned (16)));
static unsigned int  rpcKey __attribute__((aligned (16)));

int PS2KbdInit(void)
/* Initialise the keyboard library */
{
	int res;
	ee_sema_t kbdSema;

	while ((res = SifBindRpc(&cd0, PS2KBD_RPC_ID, 0)) < 0)
		nopdelay();

	memset(rpcBuf, 0, 3 * PS2KBD_KEYMAP_SIZE);
	rpcKey = 0;

	kbdSema.init_count = 1;
	kbdSema.max_count = 1;

	kbdRpcSema = CreateSema(&kbdSema);
	if (kbdRpcSema >= 0) {
		kbdInitialized = 1;
		return 0;
	} else
		return -1;
}

static void rpcCompleteIntr(void *param) {
    iSignalSema(kbdRpcSema);
}

int PS2KbdRead(char *key)
/* Reads 1 character from the keyboard */
{
	int res;
	if ((!kbdInitialized) || (curr_readmode != PS2KBD_READMODE_NORMAL))
		return -1;

	if (PollSema(kbdRpcSema) >= 0) {
		// last rpc call completed
		res = (rpcKey != 0);
		*key = *(char *)UNCACHED_SEG(&rpcKey);
		SifCallRpc(&cd0, KBD_RPC_READKEY, SIF_RPC_M_NOWAIT, rpcBuf, 0, &rpcKey, 4, rpcCompleteIntr, NULL);
		return res;
	} else // rpc still running
		return 0;
}

int PS2KbdReadRaw(PS2KbdRawKey *key)
/* Reads 1 raw character from the keyboard */
{
	int res;
	if ((!kbdInitialized) || (curr_readmode != PS2KBD_READMODE_RAW))
		return -1;

	if (PollSema(kbdRpcSema) >= 0) {
		// last rpc call completed
		res = (rpcKey != 0);
		*key = *(PS2KbdRawKey *)UNCACHED_SEG(&rpcKey);
		SifCallRpc(&cd0, KBD_RPC_READRAW, SIF_RPC_M_NOWAIT, rpcBuf, 0, &rpcKey, 4, rpcCompleteIntr, NULL);
		return res;
	} else // rpc still running
		return 0;
}

int PS2KbdSetReadmode(u32 readmode)
/* Sets the read mode to normal or raw */
{
	if (kbdInitialized) {
		if (curr_readmode == readmode)
			return 0;
		WaitSema(kbdRpcSema);
		*(unsigned int *)rpcBuf = curr_readmode = readmode;
		return SifCallRpc(&cd0, KBD_RPC_SETREADMODE, SIF_RPC_M_NOWAIT, rpcBuf, 4, rpcBuf, 0, rpcCompleteIntr, NULL);
	} else
		return -1;
}

int PS2KbdSetLeds(u8 leds)
/* Sets all connected keyboards leds */
{
	if (kbdInitialized) {
		WaitSema(kbdRpcSema);
		*(unsigned char *)rpcBuf = leds;
		return SifCallRpc(&cd0, KBD_RPC_SETLEDS, SIF_RPC_M_NOWAIT, rpcBuf, 4, rpcBuf, 0, rpcCompleteIntr, NULL);
	} else
		return -1;
}

int PS2KbdSetKeymap(PS2KbdKeyMap *keymaps)
/* Sets the current keymap */
{
	if (kbdInitialized) {
		WaitSema(kbdRpcSema);
		memcpy(rpcBuf + 0 * PS2KBD_KEYMAP_SIZE, keymaps->keymap,      PS2KBD_KEYMAP_SIZE);
		memcpy(rpcBuf + 1 * PS2KBD_KEYMAP_SIZE, keymaps->shiftkeymap, PS2KBD_KEYMAP_SIZE);
		memcpy(rpcBuf + 2 * PS2KBD_KEYMAP_SIZE, keymaps->keycap,      PS2KBD_KEYMAP_SIZE);
		return SifCallRpc(&cd0, KBD_RPC_SETKEYMAP, SIF_RPC_M_NOWAIT, rpcBuf, 3 * PS2KBD_KEYMAP_SIZE, rpcBuf, 0, rpcCompleteIntr, NULL);
	} else
		return -1;
}

int PS2KbdSetCtrlmap(u8 *ctrlmap)
/* Sets the control key mappings */
{
	if (kbdInitialized) {
		WaitSema(kbdRpcSema);
		memcpy(rpcBuf, ctrlmap, PS2KBD_KEYMAP_SIZE);
		return SifCallRpc(&cd0, KBD_RPC_SETCTRLMAP, SIF_RPC_M_NOWAIT, rpcBuf, PS2KBD_KEYMAP_SIZE, rpcBuf, 0, rpcCompleteIntr, NULL);
	} else
		return -1;
}

int PS2KbdSetAltmap(u8 *altmap)
/* Sets the alt key mappings */
{
	if (kbdInitialized) {
		WaitSema(kbdRpcSema);
		memcpy(rpcBuf, altmap, PS2KBD_KEYMAP_SIZE);
		return SifCallRpc(&cd0, KBD_RPC_SETALTMAP, SIF_RPC_M_NOWAIT, rpcBuf, PS2KBD_KEYMAP_SIZE, rpcBuf, 0, rpcCompleteIntr, NULL);
	} else
		return -1;
}

int PS2KbdSetSpecialmap(u8 *special)
/* Sets the special key mappings */
{
	if (kbdInitialized) {
		WaitSema(kbdRpcSema);
		memcpy(rpcBuf, special, PS2KBD_KEYMAP_SIZE);
		return SifCallRpc(&cd0, KBD_RPC_SETSPECIALMAP, SIF_RPC_M_NOWAIT, rpcBuf, PS2KBD_KEYMAP_SIZE, rpcBuf, 0, rpcCompleteIntr, NULL);
	} else
		return -1;
}

int PS2KbdFlushBuffer(void)
/* Flushes the keyboard buffer */
{
	if (kbdInitialized) {
		WaitSema(kbdRpcSema);
		return SifCallRpc(&cd0, KBD_RPC_FLUSHBUFFER, SIF_RPC_M_NOWAIT, rpcBuf, 0, rpcBuf, 0, rpcCompleteIntr, NULL);
	} else
		return -1;
}

int PS2KbdResetKeymap(void)
/* Resets the keymap to the default US mapping */
{
	if (kbdInitialized) {
		WaitSema(kbdRpcSema);
		return SifCallRpc(&cd0, KBD_RPC_RESETKEYMAP, SIF_RPC_M_NOWAIT, rpcBuf, 0, rpcBuf, 0, rpcCompleteIntr, NULL);
	} else
		return -1;
}
