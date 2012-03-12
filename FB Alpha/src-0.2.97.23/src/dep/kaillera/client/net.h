#include "kailleraclient.h"

extern int Kaillera_Initialised;

extern int (WINAPI *Kaillera_Get_Version) (char *version);
extern int (WINAPI *Kaillera_Init) ();
extern int (WINAPI *Kaillera_Shutdown) ();
extern int (WINAPI *Kaillera_Set_Infos) (kailleraInfos *infos);
extern int (WINAPI *Kaillera_Select_Server_Dialog) (HWND parent);
extern int (WINAPI *Kaillera_Modify_Play_Values) (void *values, int size);
extern int (WINAPI *Kaillera_Chat_Send) (char *text);
extern int (WINAPI *Kaillera_End_Game) ();

int Init_Network(void);
void End_Network(void);
