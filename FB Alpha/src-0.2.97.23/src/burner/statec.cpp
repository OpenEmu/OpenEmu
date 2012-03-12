// Driver State Compression module
#include "zlib.h"

#include "burnint.h"

static UINT8* Comp = NULL;		// Compressed data buffer
static INT32 nCompLen = 0;
static INT32 nCompFill = 0;				// How much of the buffer has been filled so far

static z_stream Zstr;					// Deflate stream

// -----------------------------------------------------------------------------
// Compression

static INT32 CompEnlarge(INT32 nAdd)
{
	void* NewMem = NULL;

	// Need to make more room in the compressed buffer
	NewMem = realloc(Comp, nCompLen + nAdd);
	if (NewMem == NULL) {
		return 1;
	}

	Comp = (UINT8*)NewMem;
	memset(Comp + nCompLen, 0, nAdd);
	nCompLen += nAdd;

	return 0;
}

static INT32 CompGo(INT32 bFinish)
{
	INT32 nResult = 0;
	INT32 nAvailOut = 0;

	bool bRetry, bOverflow;

	do {

		bRetry = false;

		// PoINT32 to the remainder of out buffer
		Zstr.next_out = Comp + nCompFill;
		nAvailOut = nCompLen - nCompFill;
		if (nAvailOut < 0) {
			nAvailOut = 0;
		}
		Zstr.avail_out = nAvailOut;

		// Try to deflate into the buffer (there may not be enough room though)
		if (bFinish) {
			nResult = deflate(&Zstr, Z_FINISH);					// deflate and finish
			if (nResult != Z_OK && nResult != Z_STREAM_END) {
				return 1;
			}
		} else {
			nResult = deflate(&Zstr, 0);						// deflate
			if (nResult != Z_OK) {
				return 1;
			}
		}

		nCompFill = Zstr.next_out - Comp;						// Update how much has been filled

		// Check for overflow
		bOverflow = bFinish ? (nResult == Z_OK) : (Zstr.avail_out <= 0);

		if (bOverflow) {
			if (CompEnlarge(4 * 1024)) {
				return 1;
			}

			bRetry = true;
		}
	} while (bRetry);

	return 0;
}

static INT32 __cdecl StateCompressAcb(struct BurnArea* pba)
{
	// Set the data as the next available input
	Zstr.next_in = (UINT8*)pba->Data;
	Zstr.avail_in = pba->nLen;

	CompGo(0);													// Compress this Area

	Zstr.avail_in = 0;
	Zstr.next_in = NULL;

	return 0;
}

// Compress a state using deflate
INT32 BurnStateCompress(UINT8** pDef, INT32* pnDefLen, INT32 bAll)
{
	void* NewMem = NULL;

	memset(&Zstr, 0, sizeof(Zstr));

	Comp = NULL; nCompLen = 0; nCompFill = 0;					// Begin with a zero-length buffer
	if (CompEnlarge(8 * 1024)) {
		return 1;
	}

	deflateInit(&Zstr, Z_DEFAULT_COMPRESSION);

	BurnAcb = StateCompressAcb;									// callback our function with each area

	if (bAll) BurnAreaScan(ACB_FULLSCAN | ACB_READ, NULL);		// scan all ram, read (from driver <- decompress)
	else      BurnAreaScan(ACB_NVRAM    | ACB_READ, NULL);		// scan nvram,   read (from driver <- decompress)

	// Finish off
	CompGo(1);

	deflateEnd(&Zstr);

	// Size down
	NewMem = realloc(Comp, nCompFill);
	if (NewMem) {
		Comp = (UINT8*)NewMem;
		nCompLen = nCompFill;
	}

	// Return the buffer
	if (pDef) {
		*pDef = Comp;
	}
	if (pnDefLen) {
		*pnDefLen = nCompFill;
	}

	return 0;
}

// -----------------------------------------------------------------------------
// Decompression

static INT32 __cdecl StateDecompressAcb(struct BurnArea* pba)
{
	Zstr.next_out =(UINT8*)pba->Data;
	Zstr.avail_out = pba->nLen;

	inflate(&Zstr, Z_SYNC_FLUSH);

	Zstr.avail_out = 0;
	Zstr.next_out = NULL;

	return 0;
}

INT32 BurnStateDecompress(UINT8* Def, INT32 nDefLen, INT32 bAll)
{
	memset(&Zstr, 0, sizeof(Zstr));
	inflateInit(&Zstr);

	// Set all of the buffer as available input
	Zstr.next_in = (UINT8*)Def;
	Zstr.avail_in = nDefLen;

	BurnAcb = StateDecompressAcb;								// callback our function with each area

	if (bAll) BurnAreaScan(ACB_FULLSCAN | ACB_WRITE, NULL);		// scan all ram, write (to driver <- decompress)
	else      BurnAreaScan(ACB_NVRAM    | ACB_WRITE, NULL);		// scan nvram,   write (to driver <- decompress)

	inflateEnd(&Zstr);
	memset(&Zstr, 0, sizeof(Zstr));

	return 0;
}
