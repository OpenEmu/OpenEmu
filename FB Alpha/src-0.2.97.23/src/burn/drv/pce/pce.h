extern UINT8 PCEPaletteRecalc;

extern UINT8 PCEReset;
extern UINT8 PCEJoy1[12];
extern UINT8 PCEJoy2[12];
extern UINT8 PCEJoy3[12];
extern UINT8 PCEJoy4[12];
extern UINT8 PCEJoy5[12];
extern UINT8 PCEDips[3];

INT32 PceGetZipName(char** pszName, UINT32 i);
INT32 TgGetZipName(char** pszName, UINT32 i);
INT32 SgxGetZipName(char** pszName, UINT32 i);

INT32 PCEInit();
INT32 TG16Init();
INT32 SGXInit();
INT32 populousInit();

INT32 PCEExit();
INT32 PCEDraw();
INT32 PCEFrame();
INT32 PCEScan(INT32 nAction, INT32 *pnMin);
