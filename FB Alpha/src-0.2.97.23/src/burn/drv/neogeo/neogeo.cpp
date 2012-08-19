// Neo Geo -- misc. support functions

#include "neogeo.h"
#include "bitswap.h"

UINT8 nNeoProtectionXor;

// This function loads the 68K ROMs
INT32 NeoLoadCode(INT32 nOffset, INT32 nNum, UINT8* pDest)
{
    struct BurnRomInfo ri;

	for (INT32 i = 0; i < nNum; i++) {
		ri.nLen = 0;
		BurnDrvGetRomInfo(&ri, nOffset + i);

		if ((BurnDrvGetHardwareCode() & HARDWARE_SNK_P32) && (i == 0))
		{
			if (BurnLoadRom(pDest + 0, nOffset + i + 0, 2)) return 1;
			if (BurnLoadRom(pDest + 1, nOffset + i + 1, 2)) return 1;
 
			for (UINT32 j = 0; j < ri.nLen << 1; j+=4)
				BurnByteswap(pDest + j + 1, 2);

			i++;
			pDest += ri.nLen << 1;
			continue;
		}

		if (BurnLoadRom(pDest, nOffset + i, 1)) {
			return 1;
		}

		if ((BurnDrvGetHardwareCode() & HARDWARE_SNK_SWAPP) && (i == 0)) {
			for (UINT32 j = 0; j < (ri.nLen / 2); j++) {
				INT32 k = pDest[j];
				pDest[j] = pDest[j + (ri.nLen / 2)];
				pDest[j + (ri.nLen / 2)] = k;
			}
		}

		pDest += ri.nLen;
	}

	return 0;
}

static void NeoSVCAddressDecrypt(UINT8* src, UINT8* dst, INT32 start, INT32 end)
{
	for (INT32 i = start / 4; i < end / 4; i++) {
		((UINT32*)dst)[i] = ((UINT32*)src)[(i & 0xFFE00000) | (0x0C8923 ^ BITSWAP24((i & 0x1FFFFF), 0x17, 0x16, 0x15, 0x04, 0x0B, 0x0E, 0x08, 0x0C, 0x10, 0x00, 0x0a, 0x13, 0x03, 0x06, 0x02, 0x07, 0x0D, 0x01, 0x11, 0x09, 0x14, 0x0f, 0x12, 0x05))];
	}
}

static void NeoKOFAddressDecrypt(UINT8* src, UINT8* dst, INT32 start, INT32 end)
{
	for (INT32 i = start; i < end; i += 0x100) {
		memcpy(dst + i, src + ((i & 0xFF800000) | BURN_ENDIAN_SWAP_INT16((BITSWAP16((i >> 8) & 0x7FFF, 0x0F, 0x0A, 0x0E, 0x0C, 0x0B, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x0D, 0x01, 0x00)) << 8)), 0x100);
	}
}

static void NeoPCBDataDecrypt(UINT8* dst, INT32 size)
{
	for (INT32 i = 0; i < size / 4; i++) {
		((UINT32*)dst)[i] = BURN_ENDIAN_SWAP_INT32(BITSWAP32(0xE9C42134 ^ BURN_ENDIAN_SWAP_INT32(((UINT32*)dst)[i]), 0x09, 0x0D, 0x13, 0x00, 0x17, 0x0F, 0x03, 0x05, 0x04, 0x0C, 0x11, 0x1E, 0x12, 0x15, 0x0B, 0x06, 0x1B, 0x0A, 0x1A, 0x1C, 0x14, 0x02, 0x0e, 0x1D, 0x18, 0x08, 0x01, 0x10, 0x19, 0x1F, 0x07, 0x16));
	}
}

// This function loads and pre-processes the sprite data
INT32 NeoLoadSprites(INT32 nOffset, INT32 nNum, UINT8* pDest, UINT32 nSpriteSize)
{
	struct BurnRomInfo ri;

	UINT32 nRomSize = 0;

	if (BurnDrvGetHardwareCode() & (HARDWARE_SNK_CMC42 | HARDWARE_SNK_CMC50)) {

		UINT8* pBuf1 = NULL;
		UINT8* pBuf2 = NULL;

//		double dProgress = 1.0 / ((double)((nSpriteSize > 0x04000000) ? 0x05000000 : nSpriteSize) / 0x400000 * 1.5);

		BurnDrvGetRomInfo(&ri, nOffset);
		nRomSize = ri.nLen;

		if (BurnDrvGetHardwareCode() & HARDWARE_SNK_CMC42) {
			NeoCMC42Init();
		} else {
			if (BurnDrvGetHardwareCode() & HARDWARE_SNK_CMC50) {
				NeoCMC50Init();
			}
		}

		pBuf1 = (UINT8*)BurnMalloc(nRomSize * 2);
		if (pBuf1 == NULL) {
			return 1;
		}

		if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SNK_DEDICATED_PCB) {
//			dProgress *= 0.75;

			pBuf2 = (UINT8*)BurnMalloc(nRomSize * 2);
			if (pBuf2 == NULL) {
				return 1;
			}
		}

		for (INT32 i = 0; i < (nNum >> 1); i++) {
			if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SNK_DEDICATED_PCB) {
				if (nRomSize == 0x02000000) {
					
					// The ROM chips are 32-bit

					BurnLoadRom(pBuf2 + 0 * nRomSize, nOffset +     (i << 1), 1);
					BurnLoadRom(pBuf2 + 1 * nRomSize, nOffset + 1 + (i << 1), 1);
				} else {

					// The ROM chips are 16-bit and need to be interleaved

					BurnLoadRom(pBuf1, nOffset +     (i << 1), 1);
					for (UINT32 j = 0; j < nRomSize / 2; j++) {
						((UINT16*)pBuf2)[(j << 1) + 0] = ((UINT16*)pBuf1)[j];
					}
					BurnLoadRom(pBuf1, nOffset + 1 + (i << 1), 1);
					for (UINT32 j = 0; j < nRomSize / 2; j++) {
						((UINT16*)pBuf2)[(j << 1) + 1] = ((UINT16*)pBuf1)[j];
					}
				}
			} else {
				BurnLoadRom(pBuf1 + 0, nOffset +     (i << 1), 2);
				BurnLoadRom(pBuf1 + 1, nOffset + 1 + (i << 1), 2);
			}

//			BurnUpdateProgress(0.0, _T("Decrypting graphics...")/*, BST_DECRYPT_GRA*/ , 0);
			BurnUpdateProgress(1.0 / ((double)(nSpriteSize/0x800000) * 8.0 / (nRomSize / 0x400000) / 3.0), _T("Decrypting graphics..."), 0);

			if ((i * nRomSize * 2) < 0x04000000) {
				for (UINT32 j = 0; j < nRomSize * 2; j += 0x400000) {
					if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SNK_DEDICATED_PCB) {
//						BurnUpdateProgress(dProgress / 2.0, NULL/*, 0*/, 0);
						(BurnDrvGetHardwareCode() & HARDWARE_SNK_KOF2K3) ? NeoKOFAddressDecrypt(pBuf2, pBuf1, j, j + 0x400000) : NeoSVCAddressDecrypt(pBuf2, pBuf1, j, j + 0x400000);
						NeoPCBDataDecrypt(pBuf1 + j, 0x400000);
					}
//					BurnUpdateProgress(dProgress, NULL/*, 0*/, 0);
					NeoCMCDecrypt(nNeoProtectionXor, pDest, pBuf1 + j, i * (nRomSize * 2) + j, 0x400000, nSpriteSize);
				}
			} else {
				// The kof2k3 PCB has 96MB of graphics ROM, however the last 16MB are unused, and the protection/decryption hardware does not see them

				for (UINT32 j = 0; j < nRomSize; j += 0x400000) {
//					BurnUpdateProgress(dProgress / 2.0, NULL/*, 0*/, 0);
					NeoKOFAddressDecrypt(pBuf2, pBuf1, j, j + 0x400000);
					NeoPCBDataDecrypt(pBuf1 + j, 0x400000);
//					BurnUpdateProgress(dProgress, NULL, /*0,*/ 0);
					NeoCMCDecrypt(nNeoProtectionXor, pDest + 0x4000000, pBuf1 + j, j, 0x400000, 0x1000000);
				}
			}
		}

		BurnFree(pBuf2);
		BurnFree(pBuf1);
	} else {
		nSpriteSize = 0;

		// Compute correct size to gaps into account (Kizuna)
		for (INT32 i = 0; i < nNum - 2; i++) {
			BurnDrvGetRomInfo(&ri, nOffset + i);
			if (ri.nLen > nRomSize) {
				nRomSize = ri.nLen;
			}
		}
	
		if (BurnDrvGetHardwareCode() & HARDWARE_SNK_SPRITE32) {
			for (INT32 i = 0; i < (nNum >> 2); i++) {
				BurnLoadRom(pDest + nSpriteSize + 0, nOffset + 0 + (i << 2), 4);
				BurnLoadRom(pDest + nSpriteSize + 2, nOffset + 1 + (i << 2), 4);
				BurnLoadRom(pDest + nSpriteSize + 1, nOffset + 2 + (i << 2), 4);
				BurnLoadRom(pDest + nSpriteSize + 3, nOffset + 3 + (i << 2), 4);

				BurnDrvGetRomInfo(&ri, nOffset + (i << 2));

				if (i < (nNum >> 2) - 1) {
					nSpriteSize += nRomSize << 2;
				} else {
					nSpriteSize += ri.nLen << 2;
				}
			}
		} else {
			for (INT32 i = 0; i < (nNum >> 1); i++) {
				BurnLoadRom(pDest + nSpriteSize + 0, nOffset + (i << 1), 2);
				BurnLoadRom(pDest + nSpriteSize + 1, nOffset + 1 + (i << 1), 2);

				BurnDrvGetRomInfo(&ri, nOffset + (i << 1));

				if (i < (nNum >> 1) - 1) {
					nSpriteSize += nRomSize << 1;
				} else {
					nSpriteSize += ri.nLen << 1;
				}
			}
		}
	}

	// Swap data for viewpoin, aof, ssideki, kotm2, more
	if (BurnDrvGetHardwareCode() & HARDWARE_SNK_SWAPC) {
		UINT8* pBuf = (UINT8*)BurnMalloc(0x600000);

		if (pBuf) {
			for (INT32 i = 0x200000; i < 0x600000; i++) {
				pBuf[i] = pDest[i];
			}
			for (INT32 i = 0; i < 0x100000; i++) {
				((INT16*)(pDest + 0x200000))[i] = ((INT16*)(pBuf + 0x400000))[i];
				((INT16*)(pDest + 0x400000))[i] = ((INT16*)(pBuf + 0x200000))[i];
			}

			BurnFree(pBuf);
		} else {
			return 1;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------
// Graphics decoding for MVS/AES

void NeoDecodeSprites(UINT8* pDest, INT32 nSize)
{
//	double dProgress = 0.0;

//	if (nSize > 0x04000000) {
//		nSize = 0x04000000;
//	}

/*	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SNK_DEDICATED_PCB) {
		dProgress = 1.0 / 32.0;
	} else if (BurnDrvGetHardwareCode() & (HARDWARE_SNK_CMC42 | HARDWARE_SNK_CMC50)) {
		dProgress = 1.0 / 24.0;
	} else {
		dProgress = 1.0 /  8.0;
	}*/

	for (INT32 i = 0; i < 8; i++) {

		UINT8* pStart = pDest + i * (nSize >> 3);
		UINT8* pEnd = pStart + (nSize >> 3);

//		BurnUpdateProgress(dProgress, i ? NULL : _T("Preprocessing graphics...")/*, BST_PROCESS_GRA*/, 0);

		INT32 nStep = 8;
		if (BurnDrvGetHardwareCode() & (HARDWARE_SNK_CMC42 | HARDWARE_SNK_CMC50)) {
			nStep *= 4;
		}
		BurnUpdateProgress(1.0 / nStep, i ? NULL : _T("Preprocessing graphics..."), 0);

		// Pre-process the sprite graphics
		for (UINT8* pTile = pStart; pTile < pEnd; pTile += 128) {
			UINT32 data[32];

			for (INT32 y = 0; y < 16; y++) {
				UINT32 n = 0;
				for (INT32 x = 0; x < 8; x++) {
					UINT32 m = ((pTile[67 + (y << 2)] >> x) & 1) << 3;
					m |= ((pTile[65 + (y << 2)] >> x) & 1) << 2;
					m |= ((pTile[66 + (y << 2)] >> x) & 1) << 1;
					m |= ((pTile[64 + (y << 2)] >> x) & 1) << 0;

					n |= m << (x << 2);
				}
				data[(y << 1) + 0] = n;

				n = 0;
				for (INT32 x = 0; x < 8; x++) {
					UINT32 m = ((pTile[3 + (y << 2)] >> x) & 1) << 3;
					m |= ((pTile[1 + (y << 2)] >> x) & 1) << 2;
					m |= ((pTile[2 + (y << 2)] >> x) & 1) << 1;
					m |= ((pTile[0 + (y << 2)] >> x) & 1) << 0;

					n |= m << (x << 2);
				}
				data[(y << 1) + 1] = n;
			}
			for (INT32 n = 0; n < 32; n++) {
				((UINT32*)pTile)[n] = data[n];
			}
		}
	}
}

/*
void NeoDecodeText(UINT8* pDest, INT32 nSize)
{
	// Pre-process the text layer graphics
	for (UINT8* pTile = pDest; pTile < (pDest + nSize); pTile += 32) {
		UINT8 data[32];
		for (INT32 n = 0; n < 8; n++) {
			data[0 + n * 4] = pTile[16 + n];
			data[1 + n * 4] = pTile[24 + n];
			data[2 + n * 4] = pTile[ 0 + n];
			data[3 + n * 4] = pTile[ 8 + n];
		}

		for (INT32 n = 0; n < 32; n++) {
			pTile[n]  = data[n] << 4;
			pTile[n] |= data[n] >> 4;
		}
	}
}
*/

// ----------------------------------------------------------------------------
// Graphics decoding for Neo CD

void NeoDecodeSpritesCD(UINT8* pData, UINT8* pDest, INT32 nSize)
{
	UINT8* pEnd = pData + nSize;

	for (UINT8* pTile = pData; pTile < pEnd; pTile += 128, pDest += 128) {
		UINT32 data[32];

		for (INT32 y = 0; y < 16; y++) {
			UINT32 n = 0;

			for (INT32 x = 0; x < 8; x++) {
				UINT32 m = ((pTile[67 + (y << 2)] >> x) & 1) << 3;
				m |= ((pTile[66 + (y << 2)] >> x) & 1) << 2;
				m |= ((pTile[65 + (y << 2)] >> x) & 1) << 1;
				m |= ((pTile[64 + (y << 2)] >> x) & 1) << 0;

				n |= m << (x << 2);
			}
			data[(y << 1) + 0] = n;

			n = 0;
			for (INT32 x = 0; x < 8; x++) {
				UINT32 m = ((pTile[3 + (y << 2)] >> x) & 1) << 3;
				m |= ((pTile[2 + (y << 2)] >> x) & 1) << 2;
				m |= ((pTile[1 + (y << 2)] >> x) & 1) << 1;
				m |= ((pTile[0 + (y << 2)] >> x) & 1) << 0;

				n |= m << (x << 2);
			}
			data[(y << 1) + 1] = n;
		}

		for (INT32 n = 0; n < 32; n++) {
			((UINT32*)pDest)[n] = data[n];
		}
	}
}

// ----------------------------------------------------------------------------

INT32 NeoLoadADPCM(INT32 nOffset, INT32 nNum, UINT8* pDest)
{
	struct BurnRomInfo ri;
	ri.nType = 0;
	ri.nLen = 0;

	BurnDrvGetRomInfo(&ri, nOffset);

	for (INT32 i = 0; i < nNum; i++) {
		BurnLoadRom(pDest + ri.nLen * i, nOffset + i, 1);
	}

	return 0;
}

// This function fills the screen with the first palette entry
void NeoClearScreen()
{
	UINT32 nColour = NeoPalette[0x0FFF];

	if (nColour) {
		switch (nBurnBpp) {
			case 4: {
				UINT32* pClear = (UINT32*)pBurnDraw;
				for (INT32 i = 0; i < nNeoScreenWidth * 224 / 8; i++) {
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
				}
				break;
			}

			case 3: {
				UINT8* pClear = pBurnDraw;
				UINT8 r =  nColour;
				UINT8 g = (nColour >>  8) & 0xFF;
				UINT8 b = (nColour >> 16) & 0xFF;
				for (INT32 i = 0; i < nNeoScreenWidth * 224; i++) {
					*pClear++ = r;
					*pClear++ = g;
					*pClear++ = b;
				}
				break;
			}

			case 2: {
				UINT32* pClear = (UINT32*)pBurnDraw;
				nColour |= nColour << 16;
				for (INT32 i = 0; i < nNeoScreenWidth * 224 / 16; i++) {
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
				}
				break;
			}
		}
	} else {
		memset(pBurnDraw, 0, nNeoScreenWidth * 224 * nBurnBpp);
	}
}
