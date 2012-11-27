/**********************************
  Copyright (C) Rick Wong (Lick)
  http://licklick.wordpress.com/
***********************************/
#ifndef CARTRESET_H
#define CARTRESET_H

#include <nds.h>
//#include <fat.h>


#ifdef __cplusplus
extern "C" {
#endif

#ifdef ARM9
// Auto detect:
#define DEVICE_TYPE_AUTO        0x00000000 // doesn't work in libcartreset "nolibfat" version

// Not supported:
#define DEVICE_TYPE_FCSR        0x52534346
#define DEVICE_TYPE_MMCF        0x46434D4D
#define DEVICE_TYPE_NJSD        0x44534A4E
#define DEVICE_TYPE_NMMC        0x434D4D4E

// Supported:
#define DEVICE_TYPE_EFA2        0x32414645
#define DEVICE_TYPE_MPCF        0x4643504D
#define DEVICE_TYPE_M3CF        0x4643334D
#define DEVICE_TYPE_M3SD        0x4453334D
#define DEVICE_TYPE_SCCF        0x46434353
#define DEVICE_TYPE_SCSD        0x44534353

// Supported, but libfat doesn't detect the device:
#define DEVICE_TYPE_EZSD        0x44535A45


bool cartSetMenuMode(u32 _deviceType);
void passmeloopEnter();

#endif


#ifdef ARM7

bool passmeloopQuery();
void cartExecute();

#endif


#ifdef __cplusplus
}
#endif

#endif
