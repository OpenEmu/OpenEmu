void MDFN_VSUniPower(void);
void MDFN_VSUniCheck(uint64 md5partial, int *, int *);
void MDFN_VSUniDraw(MDFN_Surface *surface);

void MDFN_VSUniToggleDIPView(void);
void MDFN_VSUniToggleDIP(int);	/* For movies and netplay */
void MDFN_VSUniCoin(void);
void MDFN_VSUniSwap(uint8 *j0, uint8 *j1);

int MDFNNES_VSUNIStateAction(StateMem *sm, int load, int state_only);

void MDFN_VSUniInstallRWHooks(void);

unsigned int MDFN_VSUniGetPaletteNum(void);

