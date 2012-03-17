#include <stdio.h>
#include <windows.h>
#include "net.h"

HINSTANCE Kaillera_HDLL;
int Kaillera_Initialised;

int (WINAPI * Kaillera_Get_Version) (char *version);
int (WINAPI * Kaillera_Init) ();
int (WINAPI * Kaillera_Shutdown) ();
int (WINAPI * Kaillera_Set_Infos) (kailleraInfos * infos);
int (WINAPI * Kaillera_Select_Server_Dialog) (HWND parent);
int (WINAPI * Kaillera_Modify_Play_Values) (void *values, int size);
int (WINAPI * Kaillera_Chat_Send) (char *text);
int (WINAPI * Kaillera_End_Game) ();


int WINAPI
Empty_Kaillera_Get_Version (char *version)
{
  return 0;
}

int WINAPI
Empty_Kaillera_Init ()
{
  return 0;
}

int WINAPI
Empty_Kaillera_Shutdown ()
{
  return 0;
}

int WINAPI
Empty_Kaillera_Set_Infos (kailleraInfos * infos)
{
  return 0;
}

int WINAPI
Empty_Kaillera_Select_Server_Dialog (HWND parent)
{
  return 0;
}

int WINAPI
Empty_Kaillera_Modify_Play_Values (void *values, int size)
{
  return 0;
}

int WINAPI
Empty_Kaillera_Chat_Send (char *text)
{
  return 0;
}

int WINAPI
Empty_Kaillera_End_Game ()
{
  return 0;
}


int
Init_Network (void)
{
  Kaillera_HDLL = LoadLibrary ("kailleraclient.dll");

  if (Kaillera_HDLL != NULL)
    {
      Kaillera_Get_Version =
	(int (WINAPI *) (char *version)) GetProcAddress (Kaillera_HDLL,
							 "_kailleraGetVersion@4");
      Kaillera_Init =
	(int (WINAPI *) ()) GetProcAddress (Kaillera_HDLL, "_kailleraInit@0");
      Kaillera_Shutdown =
	(int (WINAPI *) ()) GetProcAddress (Kaillera_HDLL,
					    "_kailleraShutdown@0");
      Kaillera_Set_Infos =
	(int (WINAPI *) (kailleraInfos * infos))
	GetProcAddress (Kaillera_HDLL, "_kailleraSetInfos@4");
      Kaillera_Select_Server_Dialog =
	(int (WINAPI *) (HWND parent)) GetProcAddress (Kaillera_HDLL,
						       "_kailleraSelectServerDialog@4");
      Kaillera_Modify_Play_Values =
	(int (WINAPI *) (void *values, int size))
	GetProcAddress (Kaillera_HDLL, "_kailleraModifyPlayValues@8");
      Kaillera_Chat_Send =
	(int (WINAPI *) (char *text)) GetProcAddress (Kaillera_HDLL,
						      "_kailleraChatSend@4");
      Kaillera_End_Game =
	(int (WINAPI *) ()) GetProcAddress (Kaillera_HDLL,
					    "_kailleraEndGame@0");

      if ((Kaillera_Get_Version != NULL) && (Kaillera_Init != NULL)
	  && (Kaillera_Shutdown != NULL) && (Kaillera_Set_Infos != NULL)
	  && (Kaillera_Select_Server_Dialog != NULL)
	  && (Kaillera_Modify_Play_Values != NULL)
	  && (Kaillera_Chat_Send != NULL) && (Kaillera_End_Game != NULL))
	{
	  Kaillera_Init ();
	  Kaillera_Initialised = 1;
	  return 0;
	}

      FreeLibrary (Kaillera_HDLL);
    }

  Kaillera_Get_Version = Empty_Kaillera_Get_Version;
  Kaillera_Init = Empty_Kaillera_Init;
  Kaillera_Shutdown = Empty_Kaillera_Shutdown;
  Kaillera_Set_Infos = Empty_Kaillera_Set_Infos;
  Kaillera_Select_Server_Dialog = Empty_Kaillera_Select_Server_Dialog;
  Kaillera_Modify_Play_Values = Empty_Kaillera_Modify_Play_Values;
  Kaillera_Chat_Send = Empty_Kaillera_Chat_Send;
  Kaillera_End_Game = Empty_Kaillera_End_Game;

  Kaillera_Initialised = 0;
  return 1;
}


void
End_Network (void)
{
  if (Kaillera_Initialised)
    {
      Kaillera_Shutdown ();
      FreeLibrary (Kaillera_HDLL);
    }
}
