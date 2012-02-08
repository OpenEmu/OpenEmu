#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "System.h"
#include "NLS.h"
#include "Util.h"

#include "gba/Flash.h"
#include "gba/GBA.h"
#include "gba/Globals.h"
#include "gba/RTC.h"
#include "common/Port.h"

#include "gb/gbGlobals.h"
#include "common/memgzio.h"

extern int systemColorDepth;
extern int systemRedShift;
extern int systemGreenShift;
extern int systemBlueShift;

extern uint16_t systemColorMap16[0x10000];
extern uint32_t systemColorMap32[0x10000];

static int (ZEXPORT *utilGzWriteFunc)(gzFile, const voidp, unsigned int) = NULL;
static int (ZEXPORT *utilGzReadFunc)(gzFile, voidp, unsigned int) = NULL;
static int (ZEXPORT *utilGzCloseFunc)(gzFile) = NULL;
static z_off_t (ZEXPORT *utilGzSeekFunc)(gzFile, z_off_t, int) = NULL;

void utilPutDword(uint8_t *p, uint32_t value)
{
	*p++ = value & 255;
	*p++ = (value >> 8) & 255;
	*p++ = (value >> 16) & 255;
	*p = (value >> 24) & 255;
}

void utilPutWord(uint8_t *p, uint16_t value)
{
	*p++ = value & 255;
	*p = (value >> 8) & 255;
}


extern bool cpuIsMultiBoot;

bool utilIsGBAImage(const char * file)
{
	cpuIsMultiBoot = false;
	if(strlen(file) > 4)
	{
		const char * p = strrchr(file,'.');

		if(p != NULL)
		{
			if((strcasecmp(p, ".agb") == 0) ||
					(strcasecmp(p, ".gba") == 0) ||
					(strcasecmp(p, ".bin") == 0) ||
					(strcasecmp(p, ".elf") == 0))
				return true;
			if(strcasecmp(p, ".mb") == 0)
			{
				cpuIsMultiBoot = true;
				return true;
			}
		}
	}

	return false;
}

bool utilIsGBImage(const char * file)
{
	if(strlen(file) > 4)
	{
		const char * p = strrchr(file,'.');

		if(p != NULL)
		{
			if((strcasecmp(p, ".dmg") == 0) ||
					(strcasecmp(p, ".gb") == 0) ||
					(strcasecmp(p, ".gbc") == 0) ||
					(strcasecmp(p, ".cgb") == 0) ||
					(strcasecmp(p, ".sgb") == 0))
				return true;
		}
	}

	return false;
}

bool utilIsGzipFile(const char *file)
{
	if(strlen(file) > 3)
	{
		const char * p = strrchr(file,'.');

		if(p != NULL)
		{
			if(strcasecmp(p, ".gz") == 0)
				return true;
			if(strcasecmp(p, ".z") == 0)
				return true;
		}
	}

	return false;
}

bool utilIsZipFile(const char * file)
{
	if(strlen(file) > 4)
	{
		const char * p = strrchr(file,'.');

		if(p != NULL)
		{
			if(strcasecmp(p, ".zip") == 0)
				return true;
		}
	}

	return false;
}

/* strip .gz or .z off end*/
void utilStripDoubleExtension(const char *file, char *buffer)
{
	if(buffer != file) /* allows conversion in place*/
		strcpy(buffer, file);

	if(utilIsGzipFile(file))
	{
		char *p = strrchr(buffer, '.');

		if(p)
			*p = 0;
	}
}

static bool utilIsImage(const char *file)
{
	return utilIsGBAImage(file) || utilIsGBImage(file);
}

uint32_t utilFindType(const char *file)
{
	char buffer [2048];
	if ( !utilIsImage( file ) ) /* TODO: utilIsArchive() instead?*/
		return IMAGE_UNKNOWN;

	return utilIsGBAImage(file) ? IMAGE_GBA : IMAGE_GB;
}

static int utilGetSize(int size)
{
	int res;

	res = 1;

	while(res < size)
		res <<= 1;

	return res;
}

uint8_t *utilLoad(const char *file, bool (*accept)(const char *), uint8_t *data, int &size)
{
	FILE *fp = NULL;
	char *buf = NULL;

	fp = fopen(file,"rb");
	fseek(fp, 0, SEEK_END); /*go to end*/
	size = ftell(fp); /* get position at end (length)*/
	rewind(fp);

	uint8_t *image = data;
	if(image == NULL)
	{
		/*allocate buffer memory if none was passed to the function*/
		image = (uint8_t *)malloc(utilGetSize(size));
		if(image == NULL)
		{
			systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"data");
			return NULL;
		}
	}

	if(utilIsZipFile(file))
	{
		FILE *gd = NULL;
		uint8_t * buf = (uint8_t *)malloc(64*1024*1024);
		printf("ZIP file detected: %s\n", file);

		/* Open file */
		gd = fopen(file, "rb");
		if(!gd)
			return 0;

		size = UnZipBuffer(buf, gd);

		memcpy(image, buf, size); /* read into buffer*/
		/* Close file */
		fclose(gd);
	}
	else
	{
		printf("Non-ZIP file detected: %s\n",file);
		fread(image, 1, size, fp); /* read into buffer*/
	}
	fclose(fp);
	return image;
}

void utilWriteInt(gzFile gzFile, int i)
{
  utilGzWrite(gzFile, &i, sizeof(int));
}

int utilReadInt(gzFile gzFile)
{
	int i = 0;
	utilGzRead(gzFile, &i, sizeof(int));
	return i;
}

void utilReadData(gzFile gzFile, variable_desc* data)
{
	while(data->address)
	{
		utilGzRead(gzFile, data->address, data->size);
		data++;
	}
}

void utilReadDataSkip(gzFile gzFile, variable_desc* data)
{
	while(data->address)
	{
		utilGzSeek(gzFile, data->size, SEEK_CUR);
		data++;
	}
}

void utilWriteData(gzFile gzFile, variable_desc *data)
{
	while(data->address)
	{
		utilGzWrite(gzFile, data->address, data->size);
		data++;
	}
}

gzFile utilGzOpen(const char *file, const char *mode)
{
	utilGzWriteFunc = (int (ZEXPORT *)(void *,void * const, unsigned int))gzwrite;
	utilGzReadFunc = gzread;
	utilGzCloseFunc = gzclose;
	utilGzSeekFunc = gzseek;

	return gzopen(file, mode);
}

gzFile utilMemGzOpen(char *memory, int available, const char *mode)
{
	utilGzWriteFunc = memgzwrite;
	utilGzReadFunc = memgzread;
	utilGzCloseFunc = memgzclose;

	return memgzopen(memory, available, mode);
}

int utilGzWrite(gzFile file, const voidp buffer, unsigned int len)
{
	return utilGzWriteFunc(file, buffer, len);
}

int utilGzRead(gzFile file, voidp buffer, unsigned int len)
{
	return utilGzReadFunc(file, buffer, len);
}

int utilGzClose(gzFile file)
{
	return utilGzCloseFunc(file);
}

z_off_t utilGzSeek(gzFile file, z_off_t offset, int whence)
{
	return utilGzSeekFunc(file, offset, whence);
}

long utilGzMemTell(gzFile file)
{
	return memtell(file);
}

void utilGBAFindSave(const uint8_t *data, const int size)
{
	uint32_t *p = (uint32_t *)data;
	uint32_t *end = (uint32_t *)(data + size);
	int saveType = 0;
	int flashSize = 0x10000;
	bool rtcFound = false;

	while(p  < end) {
		uint32_t d = READ32LE(p);

		if(d == 0x52504545) {
			if(memcmp(p, "EEPROM_", 7) == 0) {
				if(saveType == 0)
					saveType = 3;
			}
		} else if (d == 0x4D415253) {
			if(memcmp(p, "SRAM_", 5) == 0) {
				if(saveType == 0)
					saveType = 1;
			}
		} else if (d == 0x53414C46) {
			if(memcmp(p, "FLASH1M_", 8) == 0) {
				if(saveType == 0) {
					saveType = 2;
					flashSize = 0x20000;
				}
			} else if(memcmp(p, "FLASH", 5) == 0) {
				if(saveType == 0) {
					saveType = 2;
					flashSize = 0x10000;
				}
			}
		} else if (d == 0x52494953) {
			if(memcmp(p, "SIIRTC_V", 8) == 0)
				rtcFound = true;
		}
		p++;
	}
	/* if no matches found, then set it to NONE*/
	if(saveType == 0)
		saveType = 5;

	rtcEnable(rtcFound);
	cpuSaveType = saveType;
	flashSetSize(flashSize);
}

void utilUpdateSystemColorMaps()
{
	int i;

	switch(systemColorDepth)
	{
		case 16:
			for( i = 0; i < 0x10000; i++)
			{
				systemColorMap16[i] = ((i & 0x1f) << systemRedShift) |
					(((i & 0x3e0) >> 5) << systemGreenShift) |
					(((i & 0x7c00) >> 10) << systemBlueShift);
			}
			break;
		case 24:
		case 32:
			for( i = 0; i < 0x10000; i++)
			{
				systemColorMap32[i] = ((i & 0x1f) << systemRedShift) |
					(((i & 0x3e0) >> 5) << systemGreenShift) |
					(((i & 0x7c00) >> 10) << systemBlueShift);
			}
			break;
	}
}

/* Check for existence of file.*/
bool utilFileExists( const char *filename )
{
	FILE *f = fopen( filename, "r" );
	if( f == NULL )
		return false;
	else
	{
		fclose( f );
		return true;
	}
}

/* Not endian safe, but VBA itself doesn't seem to care */
#ifdef __LIBSNES__
void utilWriteIntMem(uint8_t *& data, int val)
{
	memcpy(data, &val, sizeof(int));
	data += sizeof(int);
}

void utilWriteMem(uint8_t *& data, const void *in_data, unsigned size)
{
	memcpy(data, in_data, size);
	data += size;
}

void utilWriteDataMem(uint8_t *& data, variable_desc *desc)
{
	while (desc->address) 
	{
		utilWriteMem(data, desc->address, desc->size);
		desc++;
	}
}

int utilReadIntMem(const uint8_t *& data)
{
	int res;

	memcpy(&res, data, sizeof(int));
	data += sizeof(int);
	return res;
}

void utilReadMem(void *buf, const uint8_t *& data, unsigned size)
{
	memcpy(buf, data, size);
	data += size;
}

void utilReadDataMem(const uint8_t *& data, variable_desc *desc)
{
	while (desc->address)
	{
		utilReadMem(desc->address, data, desc->size);
		desc++;
	}
}
#endif
