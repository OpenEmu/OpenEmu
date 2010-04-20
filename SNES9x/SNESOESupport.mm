/*
 Copyright (c) 2009, OpenEmu Team
 
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "memmap.h"
#include "gfx.h"
#include "display.h"
#include "ppu.h"
#include "soundux.h"
#include "apu.h"
#include "controls.h"
#include "snes9x.h"
#include "movie.h"
#include "screenshot.h"

const char *S9xBasename (const char *filename)
{
    NSLog(@"basename");
    return NULL;
}
void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
    short    x;
    
    drive[0] = '\0';
    fname[0] = '\0';
    ext[0]   = '\0';
    dir[0]   = '\0';
    
    if (strlen(path) < 1)
        return;
    
    x = strlen(path) - 1;
    while (x && (path[x] != '/'))
        x--;
    
    if (x)
    {
        strcpy(dir, path);
        dir[x + 1] = '\0';
        
        strcpy(fname, path + x + 1);
    }
    else
        strcpy(fname, path);
    
    x = strlen(fname);
    while (x && (fname[x] != '.'))
        x--;
    
    if (x)
    {
        strcpy(ext, fname + x);
        fname[x] = '\0';
    }
}

void _makepath(char *path, const char *drive, const char *dir, const char *fname, const char *ext)
{
#pragma unused (drive)
    
    path[0] = '\0';
    
    if (dir && dir[0])
        strcat(path, dir);
    
    if (fname && fname[0])
        strcat(path, fname);
    
    if (ext && ext[0])
    {
        if (ext[0] != '.')
            strcat(path, ".");
        strcat(path, ext);
    }
}

void S9xExit ()
{
    NSLog(@"exit");
}

void S9xHandlePortCommand(s9xcommand_t cmd, int16 data1, int16 data2)
{
    
    NSLog(@"Port command");
}


void S9xGenerateSound ()
{
    NSLog(@"Gen sound");
}


void S9xSetPalette ()
{
    //NSLog(@"Set pallette");
}
const char * S9xGetSPC7110Directory(void)
{
    NSLog(@"7110 dir");
    return NULL;
}
const char *S9xGetDirectory (enum s9x_getdirtype dirtype)
{
    NSLog(@"Get dir");
    return NULL;
}

const char *S9xChooseFilename (bool8 read_only)
{
    NSLog(@"Choose fname");
    return NULL;
}
void S9xLoadSDD1Data ()
{
    NSLog(@"Load SDD1");
}

void S9xAutoSaveSRAM (void)
{
    NSLog(@"Auto save SRAM");
}

void S9xToggleSoundChannel (int channel)
{
    NSLog(@"Toggle channel");
}

extern "C" char *osd_GetPackDir()
{
    NSLog(@"Get pack dir");
    return NULL;
}

void S9xSyncSpeed ()
{
    IPPU.RenderThisFrame = true;
    ///NSLog(@"Sync");
}

const char *S9xStringInput(const char *message)
{
    NSLog(@"String input");
    return NULL;
}

bool8 S9xInitUpdate (void)
{
    //NSLog(@"Init update");
    return true;
}

bool8 S9xDeinitUpdate(int width, int height)
{
    //NSLog(@"de init");
    return true;
}

unsigned char S9xContinueUpdate(int width, int height)
{
    return true;
}


const char *S9xGetFilename (const char *extension, enum s9x_getdirtype dirtype)
{
    NSLog(@"Get filename");
    return NULL;
}

void SetInfoDlgColor(unsigned char, unsigned char, unsigned char)
{
    NSLog(@"Set info dlg");
}
const char *S9xGetFilenameInc (const char *, enum s9x_getdirtype)
{
    NSLog(@"Get filenameinc");
    return NULL;
}


bool8 S9xDoScreenshot(int width, int height)
{
    NSLog(@"Do screenshot");
    return false;
}

bool8 S9xOpenSnapshotFile (const char *base, bool8 read_only, STREAM *file)
{
    FILE* stream;
    stream = fopen(base, (read_only ? "r" : "w"));
    *file = stream;
    return true;
}

void S9xCloseSnapshotFile (STREAM file)
{
    fclose(file);
}

bool S9xPollButton(uint32 id, bool *pressed)
{
    NSLog(@"Poll button");
    return true;
}
bool S9xPollPointer(uint32 id, int16 *x, int16 *y)
{
    NSLog(@"Poll Pointer");
    return true;
}

bool S9xPollAxis(uint32 id, int16 *value)
{
    NSLog(@"Poll axis");
    return true;
}

void S9xMessage (int type, int number, const char *message)
{
    NSLog(@"%s", message);
}

const char *S9xChooseMovieFilename(bool8 read_only)
{
    NSLog(@"Movie filename");
    return NULL;
}

