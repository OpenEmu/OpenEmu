/*
  Kaillera client API v0.84
  (c) 2001 Christophe Thibault

  History:
  0.84: added moreInfos callback
  0.83: added clientDropped callback
  0.8 : added in-game chat function/callback
  0.72: added kailleraEndGame() method
        added kailleraGetVersion() method
  0.7:  modified kailleraModifyPlayValues behaviour
*/

#ifndef KAILLERA_CLIENT_H
#define KAILLERA_CLIENT_H

#ifdef _WIN32
#include <windows.h>
#endif

#define KAILLERA_CLIENT_API_VERSION "0.9"

#ifdef KAILLERA_DLL
#define DLLEXP __declspec(dllexport) int WINAPI
#else
#define DLLEXP __declspec(dllimport) int WINAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct {
    char *appName;
    char *gameList;

    int (WINAPI *gameCallback)(char *game, int player, int numplayers);

    void (WINAPI *chatReceivedCallback)(char *nick, char *text);
    void (WINAPI *clientDroppedCallback)(char *nick, int playernb);

    void (WINAPI *moreInfosCallback)(char *gamename);
  } kailleraInfos;

  /*
     kailleraGetVersion
     Call this method to retrieve kailleraclient.dll's version
     - version must point to a char[16] buffer
  */
  DLLEXP kailleraGetVersion(char *version);
  /*
     kailleraInit:
     Call this method when your program starts
  */
  DLLEXP kailleraInit();
  /*
     kailleraShutdown:
     Call this method when your program ends
  */
  DLLEXP kailleraShutdown();
  /*
     kailleraSetInfos:
     Use this method for setting up various infos:
     Required:
     - appName must be 128 chars max.
     - gameList is composed of all the game names separated by a NULL char (\0).
       The list ends with 2 NULL chars (\0\0).
       Be sure to only put available games there.
     - gameCallback will be the function called when a new game starts
       game -> name of the selected game
       player -> player number (1-8). 0 for spectator.
       numplayers -> number of players in the game (1-8)

     Optional:
     - chatReceivedCallback will be the function called when a chat line text
       has been received. Set it to NULL if you don't need/want this callback.
     - clientDroppedCallback will be the function called when a client drops
       from the current game. Set it to NULL if you don't need/want this callback.
     - moreInfosCallback will be the function called when the user selects
       "More infos about this game..." in the game list context menu.
       Set it to NULL if you don't need/want this feature.
  */
  DLLEXP kailleraSetInfos(kailleraInfos *infos);
  /*
     kailleraSelectServerDialog:
     Use this method for launching the Kaillera server dialog
  */
  DLLEXP kailleraSelectServerDialog(HWND parent);

  /*
     kailleraModifyPlayValues:
     You must call this method at every frame after you retrieved values from your input devices.
     This method will record/bufferize the values you send in and will manage to mix them with
     the other players.

     Basically, each players sends his values and receive all players values concatened.

     e.g:
     t   - player 1 sends "867F"            - player 2 sends "964F"
     t+1 - player 1 receives "867F964F"     - player 2 receives "867F964F"

     you may want to look at the modified MAME source code available on kaillera's home page
     to have a working example out of this (look in inptport.c)

     Ideally, your input values should be defined like 1 bit for a given key, which has to be
     unset when the key is not pressed and set when the key is pressed.

     The size of the values you pass to this function is the size for ONE player's values.
     Also, it must be the SAME for a given game and should be kept to a minimum (for network
     speed and latency issues).

     Be sure that the values parameters has enough space for receiving all inputs from all players
     (8 players max.)

     returns: length received or -1 on network error (player no more in the game)
  */
  DLLEXP kailleraModifyPlayValues(void *values, int size);

  /*
      kailleraChatSend
      Use this function to send a line of chat text during a game
  */
  DLLEXP kailleraChatSend(char *text);

  /*
      kailleraEndGame:
      Your emulation thread must call this method when the user stops the emulation
  */
  DLLEXP kailleraEndGame();

#ifdef __cplusplus
}
#endif

#endif
