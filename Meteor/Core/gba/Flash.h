#ifndef FLASH_H
#define FLASH_H

#define FLASH_128K_SZ 0x20000

extern void flashSaveGame(gzFile _gzFile);
extern void flashReadGame(gzFile _gzFile, int version);
extern void flashReadGameSkip(gzFile _gzFile, int version);
extern u8 flashRead(u32 address);
extern void flashWrite(u32 address, u8 byte);
extern void flashDelayedWrite(u32 address, u8 byte);
extern u8 flashSaveMemory[FLASH_128K_SZ];
extern void flashSaveDecide(u32 address, u8 byte);
extern void flashReset();
extern void flashSetSize(int size);
extern void flashInit();

extern int flashSize;

#endif // FLASH_H
