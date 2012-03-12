#include "burner.h"

#define SSHOT_NOERROR 0
#define SSHOT_ERROR_BPP_NOTSUPPORTED 1
#define SSHOT_LIBPNG_ERROR 2
#define SSHOT_OTHER_ERROR 3

#define SSHOT_DIRECTORY "screenshots/"

static UINT8* pSShot = NULL;
static UINT8* pConvertedImage = NULL;
static png_bytep* pSShotImageRows = NULL;
static FILE* ff;

INT32 MakeScreenShot()
{
	char szAuthor[256]; char szDescription[256]; char szCopyright[256];	char szSoftware[256]; char szSource[256];
	png_text text_ptr[8] = { { 0, 0, 0, 0, 0, 0, 0 }, };
	INT32 num_text = 8;

    time_t currentTime;
    tm* tmTime;
    png_time_struct png_time_now;

    char szSShotName[MAX_PATH];
    INT32 w, h;

    // do our PNG construct things
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
		return SSHOT_LIBPNG_ERROR;
	}

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

		return SSHOT_LIBPNG_ERROR;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
		if (pConvertedImage) {
			free(pConvertedImage);
			pConvertedImage = NULL;
		}

		if (pSShotImageRows) {
			free(pSShotImageRows);
			pSShotImageRows = NULL;
		}

		fclose(ff);
        remove(szSShotName);

		return SSHOT_LIBPNG_ERROR;
    }

	if (pVidImage == NULL) {
		return SSHOT_OTHER_ERROR;
	}

    if (nVidImageBPP < 2 || nVidImageBPP > 4) {
        return SSHOT_ERROR_BPP_NOTSUPPORTED;
    }

	BurnDrvGetVisibleSize(&w, &h);

	pSShot = pVidImage;

	// Convert the image to 32-bit
	if (nVidImageBPP < 4) {
		UINT8* pTemp = (UINT8*)malloc(w * h * sizeof(INT32));

		if (nVidImageBPP == 2) {
			for (INT32 i = 0; i < h * w; i++) {
				UINT16 nColour = ((UINT16*)pSShot)[i];

				// Red
		        *(pTemp + i * 4 + 0) = (UINT8)((nColour & 0x1F) << 3);
			    *(pTemp + i * 4 + 0) |= *(pTemp + 4 * i + 0) >> 5;

				if (nVidImageDepth == 15) {
					// Green
					*(pTemp + i * 4 + 1) = (UINT8)(((nColour >> 5) & 0x1F) << 3);
					*(pTemp + i * 4 + 1) |= *(pTemp + i * 4 + 1) >> 5;
					// Blue
					*(pTemp + i * 4 + 2) = (UINT8)(((nColour >> 10)& 0x1F) << 3);
					*(pTemp + i * 4 + 2) |= *(pTemp + i * 4 + 2) >> 5;
				}

				if (nVidImageDepth == 16) {
					// Green
					*(pTemp + i * 4 + 1) = (UINT8)(((nColour >> 5) & 0x3F) << 2);
					*(pTemp + i * 4 + 1) |= *(pTemp + i * 4 + 1) >> 6;
					// Blue
					*(pTemp + i * 4 + 2) = (UINT8)(((nColour >> 11) & 0x1F) << 3);
					*(pTemp + i * 4 + 2) |= *(pTemp + i * 4 + 2) >> 5;
				}
			}
        } else {
			memset(pTemp, 0, w * h * sizeof(INT32));
			for (INT32 i = 0; i < h * w; i++) {
		        *(pTemp + i * 4 + 0) = *(pSShot + i * 3 + 0);
		        *(pTemp + i * 4 + 1) = *(pSShot + i * 3 + 1);
		        *(pTemp + i * 4 + 2) = *(pSShot + i * 3 + 2);
			}
        }

		pConvertedImage = pTemp;

        pSShot = pConvertedImage;
	}

	// Rotate and flip the image
	if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
		UINT8* pTemp = (UINT8*)malloc(w * h * sizeof(INT32));

		for (INT32 x = 0; x < h; x++) {
			if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED) {
				for (INT32 y = 0; y < w; y++) {
					((UINT32*)pTemp)[(w - y - 1) + x * w] = ((UINT32*)pSShot)[x + y * h];
				}
			} else {
				for (INT32 y = 0; y < w; y++) {
					((UINT32*)pTemp)[y + (h - x - 1) * w] = ((UINT32*)pSShot)[x + y * h];
				}
			}
		}

        pSShot = pTemp;
	}
	else if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED) { // fixed rotation by regret
		UINT8* pTemp = (UINT8*)malloc(w * h * sizeof(INT32));

		for (INT32 y = h - 1; y >= 0; y--) {
			for (INT32 x = w - 1; x >= 0; x--) {
				((UINT32*)pTemp)[(w - x - 1) + (h - y - 1) * w] = ((UINT32*)pSShot)[x + y * w];
			}
		}

        pSShot = pTemp;
	}

	// Get the time
	time(&currentTime);
    tmTime = localtime(&currentTime);
	png_convert_from_time_t(&png_time_now, currentTime);

	// construct our filename -> "romname-mm-dd-hms.png"
    sprintf(szSShotName,"%s%s-%.2d-%.2d-%.2d%.2d%.2d.png", SSHOT_DIRECTORY, BurnDrvGetTextA(DRV_NAME), tmTime->tm_mon + 1, tmTime->tm_mday, tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);

	ff = fopen(szSShotName, "wb");
	if (ff == NULL) {
		png_destroy_write_struct(&png_ptr, &info_ptr);

		if (pConvertedImage) {
			free(pConvertedImage);
			pConvertedImage = NULL;
		}

		return SSHOT_OTHER_ERROR;
	}

	// Fill the PNG text fields
#ifdef _UNICODE
	sprintf(szAuthor, APP_TITLE " v%.20ls", szAppBurnVer);
#else
	sprintf(szAuthor, APP_TITLE " v%.20s", szAppBurnVer);
#endif
	sprintf(szDescription, "Screenshot of %s", DecorateGameName(nBurnDrvActive));
	sprintf(szCopyright, "%s %s", BurnDrvGetTextA(DRV_DATE), BurnDrvGetTextA(DRV_MANUFACTURER));
#ifdef _UNICODE
	sprintf(szSoftware, APP_TITLE " v%.20ls using LibPNG " PNG_LIBPNG_VER_STRING, szAppBurnVer);
#else
	sprintf(szSoftware, APP_TITLE " v%.20s using LibPNG " PNG_LIBPNG_VER_STRING, szAppBurnVer);
#endif
	sprintf(szSource, "%s video game hardware", BurnDrvGetTextA(DRV_SYSTEM));

	text_ptr[0].key = "Title";			text_ptr[0].text = BurnDrvGetTextA(DRV_FULLNAME);
	text_ptr[1].key = "Author";			text_ptr[1].text = szAuthor;
	text_ptr[2].key = "Description";	text_ptr[2].text = szDescription;
	text_ptr[3].key = "Copyright";		text_ptr[3].text = szCopyright;
	text_ptr[4].key = "Creation Time";	text_ptr[4].text = (char*)png_convert_to_rfc1123(png_ptr, &png_time_now);
	text_ptr[5].key = "Software";		text_ptr[5].text = szSoftware;
	text_ptr[6].key = "Source";			text_ptr[6].text = szSource;
	text_ptr[7].key = "Comment";		text_ptr[7].text = "This screenshot was created by running the game in an emulator; it might not accurately reflect the actual hardware the game was designed to run on.";

	for (INT32 i = 0; i < num_text; i++) {
		text_ptr[i].compression = PNG_TEXT_COMPRESSION_NONE;
	}

	png_set_text(png_ptr, info_ptr, text_ptr, num_text);

	png_init_io(png_ptr, ff);

    png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);

	png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);

    png_set_bgr(png_ptr);

	pSShotImageRows = (png_bytep*)malloc(h * sizeof(png_bytep));
    for (INT32 y = 0; y < h; y++) {
        pSShotImageRows[y] = pSShot + (y * w * sizeof(INT32));
    }

	png_write_image(png_ptr, pSShotImageRows);
	png_write_end(png_ptr, info_ptr);

	if (pSShotImageRows) {
		free(pSShotImageRows);
		pSShotImageRows = NULL;
	}

	fclose(ff);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	if (pConvertedImage) {
		free(pConvertedImage);
		pConvertedImage = NULL;
	}

	return SSHOT_NOERROR;
}
