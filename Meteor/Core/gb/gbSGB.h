#ifndef GBSGB_H
#define GBSGB_H

void gbSgbInit();
void gbSgbShutdown();
void gbSgbCommand();
void gbSgbResetPacketState();
void gbSgbReset();
void gbSgbDoBitTransfer(u8);
void gbSgbSaveGame(gzFile);
void gbSgbReadGame(gzFile, int version);
void gbSgbRenderBorder();

extern u8  gbSgbATF[20*18];
extern int gbSgbMode;
extern int gbSgbMask;
extern int gbSgbMultiplayer;
extern u8  gbSgbNextController;
extern int gbSgbPacketTimeout;
extern u8  gbSgbReadingController;
extern int gbSgbFourPlayers;

#endif // GBSGB_H
