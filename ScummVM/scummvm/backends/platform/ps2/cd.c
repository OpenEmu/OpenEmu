/*********************************************************************
 * Copyright (C) 2003 Tord Lindstrom (pukko@home.se)
 * This file is subject to the terms and conditions of the PS2Link License.
 * See the file LICENSE in the main directory of this distribution for more
 * details.
 */

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>

#define CDVD_INIT_BIND_RPC 0x80000592

static SifRpcClientData_t cdvdCd __attribute__((aligned(64)));
static char sendBuffer[256] __attribute__((aligned(16)));

int cdvdInitialised = 0;


void cdvdExit(void)
{
    cdvdInitialised = 0;
}


int cdvdInit(int mode)
{
	int i=0,len=0,ret=0;
    u8  *pkt;

    cdvdCd.server = NULL;

    do {
        if ((ret = SifBindRpc(&cdvdCd, CDVD_INIT_BIND_RPC, 0)) < 0) {
            return -1;
        }
        if (!cdvdCd.server) {
            nopdelay();
        }
    }
	while(!cdvdCd.server);

    pkt = sendBuffer;
    PUSHDATA( int, pkt, mode, i);
    pkt += i; len += i;

	if ((ret = SifCallRpc(&cdvdCd, 0, 0, sendBuffer, len, NULL, 0, 0, 0)) < 0)
        return -1;

    cdvdInitialised = 1;

    return 0;
}
