/******************************************************************************
*
* FILENAME: states.h
*
* DESCRIPTION:  This contains function and struct declarations for save state 
*               functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.2     12/10/05  bberlin      Creation
******************************************************************************/
#ifndef states_h
#define states_h

#include "../core/console.h"

int save_state ( const char *name, struct atari *atari );
int recall_state ( const char *name, struct atari *atari );
int save_quick_state ( int slot, struct atari *atari );
int recall_quick_state ( int slot, struct atari *atari );
int states_get_qs_existance ( int *exists );

#endif
