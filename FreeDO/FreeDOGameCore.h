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

#import <Cocoa/Cocoa.h>
#import <OpenEmuBase/OEGameCore.h>

@class OERingBuffer;

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480


struct VolumeHeader             // 132 bytes
{
    Byte recordType;               // 1 byte
    Byte syncBytes[5];       // 5 bytes
    Byte recordVersion;            // 1 byte
    Byte flags;                    // 1 byte
    Byte comment[32];        // 32 bytes
    Byte label[32];          // 32 bytes
    UInt32 id;                     // 4 bytes
    UInt32 blockSize;              // 4 bytes
    UInt32 blockCount;             // 4 bytes
    UInt32 rootDirId;              // 4 bytes
    UInt32 rootDirBlocks;          // 4 bytes
    UInt32 rootDirBlockSize;       // 4 bytes
    UInt32 lastRootDirCopy;        // 4 bytes
    UInt32 rootDirCopies[8]; // 32 bytes
};


typedef enum 
{
    InterfaceFunction_FDP_FREEDOCORE_VERSION = 0,
    InterfaceFunction_FDP_INIT = 1,  //set ext_interface
    InterfaceFunction_FDP_DESTROY = 2,
    InterfaceFunction_FDP_DO_EXECFRAME = 3,  //execute 1/60 of second
    InterfaceFunction_FDP_DO_FRAME_MT = 4,  //multitasking
    InterfaceFunction_FDP_DO_EXECFRAME_MT = 5,  //multitasking
    InterfaceFunction_FDP_DO_LOAD = 6,  //load state from buffer, returns !NULL if everything went smooth
    InterfaceFunction_FDP_GET_SAVE_SIZE = 7,  //return size of savestatemachine
    InterfaceFunction_FDP_DO_SAVE = 8,  //save state to buffer
    InterfaceFunction_FDP_GETP_NVRAM = 9,  //returns ptr to NVRAM 32K
    InterfaceFunction_FDP_GETP_RAMS = 10, //returns ptr to RAM 3M
    InterfaceFunction_FDP_GETP_ROMS = 11, //returns ptr to ROM 2M
    InterfaceFunction_FDP_GETP_PROFILE = 12, //returns profile pointer, sizeof = 3M/4
    InterfaceFunction_FDP_BUGTEMPORALFIX = 13, // JMK NOTE: Unused?
    InterfaceFunction_FDP_SET_ARMCLOCK = 14,
    InterfaceFunction_FDP_SET_TEXQUALITY = 15,
    InterfaceFunction_FDP_GETP_WRCOUNT = 16, // JMK NOTE: Unused?
    InterfaceFunction_FDP_SET_FIX_MODE = 17,
    InterfaceFunction_FDP_GET_FRAME_BITMAP = 18
} InterfaceFunction;

enum
{
    ExternalFunction_EXT_READ_ROMS = 1,
    ExternalFunction_EXT_READ_NVRAM = 2,
    ExternalFunction_EXT_WRITE_NVRAM = 3,
    ExternalFunction_EXT_SWAPFRAME = 5, //frame swap (in mutlithreaded) or frame draw(single treaded)
    ExternalFunction_EXT_PUSH_SAMPLE = 6, //sends sample to the buffer
    ExternalFunction_EXT_GET_PBUSLEN = 7,
    ExternalFunction_EXT_GETP_PBUSDATA = 8,
    ExternalFunction_EXT_KPRINT = 9,
    ExternalFunction_EXT_DEBUG_PRINT = 10,
    ExternalFunction_EXT_FRAMETRIGGER_MT = 12, //multitasking
    ExternalFunction_EXT_READ2048 = 14, //for XBUS Plugin
    ExternalFunction_EXT_GET_DISC_SIZE = 15,
    ExternalFunction_EXT_ON_SECTOR = 16,
    ExternalFunction_EXT_ARM_SYNC = 17,
    ExternalFunction_FDP_GET_FRAME_BITMAP = 18
} ExternalFunction;

enum {
    PhysicalParameters_ROM1_SIZE = 1 * 1024 * 1024,
    PhysicalParameters_ROM2_SIZE = 1 * 1024 * 1024,
    PhysicalParameters_NVRAM_SIZE = 32 * 1024,
    PhysicalParameters_PBUS_DATA_MAX_SIZE = 20,
    
    PhysicalParameters_TARGET_FRAMES_PER_SECOND = 60,
} PhysicalParameters;

enum {
  /*  PbusButtonAddress_L     = 0x0400,
    PbusButtonAddress_R     = 0x0800,
    PbusButtonAddress_X     = 0x1000,
    PbusButtonAddress_P     = 0x2000,
    PbusButtonAddress_C     = 0x4000,
    PbusButtonAddress_B     = 0x8000,
    PbusButtonAddress_A     = 0x0001,
    PbusButtonAddress_Left  = 0x0002,
    PbusButtonAddress_Right = 0x0004,
    PbusButtonAddress_Up    = 0x0008,
    PbusButtonAddress_Down  = 0x0010,
    
    */PbusButtonAddress_L     = 0x800,
    PbusButtonAddress_R     = 0x1000,
    PbusButtonAddress_X     = 0x200,
    PbusButtonAddress_P     = 0x400,
    PbusButtonAddress_C     = 0x4,
    PbusButtonAddress_B     = 0x8,
    PbusButtonAddress_A     = 0x10,
    PbusButtonAddress_Left  = 0x20,
    PbusButtonAddress_Right = 0x100,
    PbusButtonAddress_Up    = 0x80,
    PbusButtonAddress_Down  = 0x40,
} PbusButtonsAddresses;



unsigned char nvramhead[]=
{
    0x01,0x5a,0x5a,0x5a,0x5a,0x5a,0x02,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0x6e,0x76,0x72,0x61,0x6d,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0xff,0xff,0xff,0xff,0,0,0,1,
    0,0,0x80,0,0xff,0xff,0xff,0xfe,0,0,0,0,0,0,0,1,
    0,0,0,0,0,0,0,0x84,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0x85,0x5a,2,0xb6,0,0,0,0x98,0,0,0,0x98,
    0,0,0,0x14,0,0,0,0x14,0x7A,0xa5,0x65,0xbd,0,0,0,0x84,
    0,0,0,0x84,0,0,0x76,0x68,0,0,0,0x14
};

OE_EXPORTED_CLASS
@interface FreeDOGameCore : OEGameCore
{
    @public
    char *videoBuffer;
    int videoWidth, videoHeight;
    NSString *romName;
    double sampleRate;
}


@end
