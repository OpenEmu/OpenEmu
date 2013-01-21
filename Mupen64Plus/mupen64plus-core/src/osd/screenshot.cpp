/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - screenshot.c                                            *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Richard42                                          *
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <SDL_opengl.h>
#include <SDL.h>
#include <png.h>

#include "osd.h"

extern "C" {
#define M64P_CORE_PROTOTYPES 1
#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "api/m64p_config.h"
#include "api/config.h"
#include "main/main.h"
#include "main/util.h"
#include "main/rom.h"
#include "osal/files.h"
#include "osal/preproc.h"
#include "plugin/plugin.h"
}

/*********************************************************************************************************
* PNG support functions for writing screenshot files
*/

static void mupen_png_error(png_structp png_write, const char *message)
{
    DebugMessage(M64MSG_ERROR, "PNG Error: %s", message);
}

static void mupen_png_warn(png_structp png_write, const char *message)
{
    DebugMessage(M64MSG_WARNING, "PNG Warning: %s", message);
}

static void user_write_data(png_structp png_write, png_bytep data, png_size_t length)
{
    FILE *fPtr = (FILE *) png_get_io_ptr(png_write);
    if (fwrite(data, 1, length, fPtr) != length)
        DebugMessage(M64MSG_ERROR, "Failed to write %zi bytes to screenshot file.", length);
}

static void user_flush_data(png_structp png_write)
{
    FILE *fPtr = (FILE *) png_get_io_ptr(png_write);
    fflush(fPtr);
}

/*********************************************************************************************************
* Other Local (static) functions
*/

static int SaveRGBBufferToFile(const char *filename, const unsigned char *buf, int width, int height, int pitch)
{
    int i;

    // allocate PNG structures
    png_structp png_write = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, mupen_png_error, mupen_png_warn);
    if (!png_write)
    {
        DebugMessage(M64MSG_ERROR, "Error creating PNG write struct.");
        return 1;
    }
    png_infop png_info = png_create_info_struct(png_write);
    if (!png_info)
    {
        png_destroy_write_struct(&png_write, (png_infopp)NULL);
        DebugMessage(M64MSG_ERROR, "Error creating PNG info struct.");
        return 2;
    }
    // Set the jumpback
    if (setjmp(png_jmpbuf(png_write)))
    {
        png_destroy_write_struct(&png_write, &png_info);
        DebugMessage(M64MSG_ERROR, "Error calling setjmp()");
        return 3;
    }
    // open the file to write
    FILE *savefile = fopen(filename, "wb");
    if (savefile == NULL)
    {
        DebugMessage(M64MSG_ERROR, "Error opening '%s' to save screenshot.", filename);
        return 4;
    }
    // set function pointers in the PNG library, for write callbacks
    png_set_write_fn(png_write, (png_voidp) savefile, user_write_data, user_flush_data);
    // set the info
    png_set_IHDR(png_write, png_info, width, height, 8, PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    // allocate row pointers and scale each row to 24-bit color
    png_byte **row_pointers;
    row_pointers = (png_byte **) malloc(height * sizeof(png_bytep));
    for (i = 0; i < height; i++)
    {
        row_pointers[i] = (png_byte *) (buf + (height - 1 - i) * pitch);
    }
    // set the row pointers
    png_set_rows(png_write, png_info, row_pointers);
    // write the picture to disk
    png_write_png(png_write, png_info, 0, NULL);
    // free memory
    free(row_pointers);
    png_destroy_write_struct(&png_write, &png_info);
    // close file
    fclose(savefile);
    // all done
    return 0;
}

static int CurrentShotIndex;

static char *GetNextScreenshotPath(void)
{
    char *ScreenshotPath;
    char ScreenshotFileName[20 + 8 + 1];

    // generate the base name of the screenshot
    // add the ROM name, convert to lowercase, convert spaces to underscores
    strcpy(ScreenshotFileName, ROM_PARAMS.headername);
    for (char *pch = ScreenshotFileName; *pch != '\0'; pch++)
        *pch = (*pch == ' ') ? '_' : tolower(*pch);
    strcat(ScreenshotFileName, "-###.png");
    
    // add the base path to the screenshot file name
    const char *SshotDir = ConfigGetParamString(g_CoreConfig, "ScreenshotPath");
    if (SshotDir == NULL || *SshotDir == '\0')
    {
        // note the trick to avoid an allocation. we add a NUL character
        // instead of the separator, call mkdir, then add the separator
        ScreenshotPath = formatstr("%sscreenshot%c%s", ConfigGetUserDataPath(), '\0', ScreenshotFileName);
        if (ScreenshotPath == NULL)
            return NULL;
        osal_mkdirp(ScreenshotPath, 0700);
        ScreenshotPath[strlen(ScreenshotPath)] = OSAL_DIR_SEPARATORS[0];
    }
    else
    {
        ScreenshotPath = combinepath(SshotDir, ScreenshotFileName);
        if (ScreenshotPath == NULL)
            return NULL;
    }

    // patch the number part of the name (the '###' part) until we find a free spot
    char *NumberPtr = ScreenshotPath + strlen(ScreenshotPath) - 7;
    for (; CurrentShotIndex < 1000; CurrentShotIndex++)
    {
        sprintf(NumberPtr, "%03i.png", CurrentShotIndex);
        FILE *pFile = fopen(ScreenshotPath, "r");
        if (pFile == NULL)
            break;
        fclose(pFile);
    }

    if (CurrentShotIndex >= 1000)
    {
        DebugMessage(M64MSG_ERROR, "Can't save screenshot; folder already contains 1000 screenshots for this ROM");
        return NULL;
    }
    CurrentShotIndex++;

    return ScreenshotPath;
}

/*********************************************************************************************************
* Global screenshot functions
*/

extern "C" void ScreenshotRomOpen(void)
{
    CurrentShotIndex = 0;
}

extern "C" void TakeScreenshot(int iFrameNumber)
{
    char *filename;

    // look for an unused screenshot filename
    filename = GetNextScreenshotPath();
    if (filename == NULL)
        return;

    // get the width and height
    int width = 640;
    int height = 480;
    gfx.readScreen(NULL, &width, &height, 0);

    // allocate memory for the image
    unsigned char *pucFrame = (unsigned char *) malloc(width * height * 3);
    if (pucFrame == NULL)
    {
        free(filename);
        return;
    }

    // grab the back image from OpenGL by calling the video plugin
    gfx.readScreen(pucFrame, &width, &height, 0);

    // write the image to a PNG
    SaveRGBBufferToFile(filename, pucFrame, width, height, width * 3);
    // free the memory
    free(pucFrame);
    free(filename);
    // print message -- this allows developers to capture frames and use them in the regression test
    main_message(M64MSG_INFO, OSD_BOTTOM_LEFT, "Captured screenshot for frame %i.", iFrameNumber);
}

