typedef struct {
	UTF8 *GameName, *Artist, *Copyright, *Ripper;

        UTF8 **SongNames;
        int32 *SongLengths;
        int32 *SongFades;

        int TotalSongs;
        int StartingSong;
        int CurrentSong;
        int TotalChannels;
        int VideoSystem;

        uint16 PlayAddr,InitAddr,LoadAddr;
        uint8 BankSwitch[8];
        int SoundChip;

        /* NOTE:  NSFRawData does not necessarily point to the value returned by malloc() or
           calloc(), so it should not be passed to free()!
        */
        uint8 *NSFRawData;
        uint32 NSFRawDataSize;

        /* Currently used only by the NSFE code.  All unrecognized chunks will be stuck in
           here.
        */
        uint8 *NSFExtra;
        uint32 NSFExtraSize;

        uint8 *NSFDATA;
        int NSFMaxBank;
        int NSFSize;
} NSFINFO;

typedef struct {
                char ID[5]; /*NESM^Z*/
                uint8 Version;
                uint8 TotalSongs;
                uint8 StartingSong;
                uint8 LoadAddressLow;
                uint8 LoadAddressHigh;
                uint8 InitAddressLow;
                uint8 InitAddressHigh;
                uint8 PlayAddressLow;
                uint8 PlayAddressHigh;
                UTF8 GameName[32];
                UTF8 Artist[32];
                UTF8 Copyright[32];
                uint8 NTSCspeed[2];              // Unused
                uint8 BankSwitch[8];
                uint8 PALspeed[2];               // Unused
                uint8 VideoSystem;
                uint8 SoundChip;
                uint8 Expansion[4];
                uint8 reserve[8];
        } NSF_HEADER;

void NSF_init(void);
void NSFDealloc(void);
void NSFDodo(void);
void DoNSFFrame(void);
void MDFNNES_DrawNSF(MDFN_Surface *surface, MDFN_Rect *DisplayRect, int16 *samples, int32 scount);

// NSF Expansion Chip Set Write Handler
void NSFECSetWriteHandler(int32 start, int32 end, writefunc func);
