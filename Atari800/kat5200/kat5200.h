/******************************************************************************
*
* FILENAME: kat5200.h
*
* DESCRIPTION:  This contains console struct and functions used in main
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.1     07/18/04  bberlin      Creation
* 0.2     12/09/04  bberlin      Added 4K cart, mapping parameter
* 0.5.0   09/01/06  bberlin      Added video_group,sound_group to 'cart_5200'
******************************************************************************/
#ifndef kat5200_h
#define kat5200_h

#include "core/console.h"

#define NTSC_SCREEN_TIME  29773
#define NTSC_V_BLANK_TIME 22*114
#define NTSC_SCAN_LINES   240

#define VERSION "0.6.2"

int run_system ( void );
int load_os ( char *filename, e_machine_type type );

#endif
