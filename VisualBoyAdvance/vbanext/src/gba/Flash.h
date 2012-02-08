#ifndef FLASH_H
#define FLASH_H

#define FLASH_128K_SZ 0x20000

extern void flashSaveGame(gzFile _gzFile);
extern void flashReadGame(gzFile _gzFile, int version);

#ifdef __LIBSNES__
extern void flashSaveGameMem(uint8_t *& data);
extern void flashReadGameMem(const uint8_t *& data, int version);
#endif

extern void flashReadGameSkip(gzFile _gzFile, int version);
extern uint8_t flashRead(uint32_t address);
extern void flashWrite(uint32_t address, uint8_t byte);
extern void flashDelayedWrite(uint32_t address, uint8_t byte);
extern void flashSaveDecide(uint32_t address, uint8_t byte);
extern void flashReset();
extern void flashSetSize(int size);
extern void flashInit();

extern int flashSize;

#ifdef __LIBSNES__
extern uint8_t *flashSaveMemory;
#else
extern uint8_t flashSaveMemory[FLASH_128K_SZ];
#endif

#endif // FLASH_H
