/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string.h>

#include "main.h"
#include "input.h"
#include "input-config.h"
#include "video.h"
#include "joystick.h"

int DTestButton(std::vector<ButtConfig> &bc, char *KeyState, const uint32 *MouseData)
{
 unsigned int x;

 for(x=0; x<bc.size(); x++)
 {
  if(bc[x].ButtType==BUTTC_KEYBOARD)
  {
   if(KeyState[bc[x].ButtonNum])
    return(1);
  }
  else if(bc[x].ButtType==BUTTC_JOYSTICK)
  {
   if(DTestButtonJoy(bc[x]))
    return(1);
  }
  else if(bc[x].ButtType == BUTTC_MOUSE)
  {
   if(bc[x].ButtonNum & 0x8000)
   {
    if(bc[x].ButtonNum&1) // Y
    {
     if(bc[x].ButtonNum & 0x4000)
     {
      if(MouseData[1] < 128)
       return(1);
     }
     else
     {
      if(MouseData[1] >= 128)
       return(1);
     }
    }
    else			// X
    {
     if(bc[x].ButtonNum & 0x4000)
     {
      if(MouseData[0] < 128)
       return(1);
     }
     else
     {
      if(MouseData[0] >= 128)
       return(1);
     }
    }
    
   }
   else 
   {
    if(MouseData[2] & (1 << SDL_BUTTON(bc[x].ButtonNum)))
     return(1);
   }
  }
 }
 return(0);
}

int DTestButton(ButtConfig &bc, char *KeyState, const uint32 *MouseData)
{
 std::vector<ButtConfig> neobc;
 neobc.push_back(bc);

 return(DTestButton(neobc, KeyState, MouseData));
}


#define ICSS_ALT	1
#define ICSS_SHIFT	2
#define ICSS_CTRL	4

/* Used for command keys */
int DTestButtonCombo(std::vector<ButtConfig> &bc, char *KeyState)
{
 unsigned int x;
 unsigned int ss = 0;

 if(KeyState[MKK(LALT)] || KeyState[MKK(RALT)]) ss |= ICSS_ALT;
 if(KeyState[MKK(LSHIFT)] || KeyState[MKK(RSHIFT)]) ss |= ICSS_SHIFT;
 if(KeyState[MKK(LCTRL)] || KeyState[MKK(RCTRL)]) ss |= ICSS_CTRL;

 for(x = 0; x < bc.size(); x++)
 {
  if(bc[x].ButtType == BUTTC_KEYBOARD)
  {
   uint32 b = bc[x].ButtonNum;
   
   if(KeyState[b & 0xFFFF] && ((b >> 24) == ss))
    return(1);
  }
  else if(bc[x].ButtType == BUTTC_JOYSTICK)
  {
   if(DTestButtonJoy(bc[x]))
    return(1);
  }
 }
 return(0);
}

int DTestButtonCombo(ButtConfig &bc, char *KeyState)
{
 std::vector<ButtConfig> neobc;
 neobc.push_back(bc);

 return(DTestButtonCombo(neobc, KeyState));

}


static ButtConfig efbc;
static volatile int efbcdone;
static volatile int efck;
static int32 LastAx[64][64];
static int LastMouseX;
static int LastMouseY;

static int EventFilter(const SDL_Event *event)
{
 if(efbcdone)
  return(1);
  switch(event->type)
  {
   case SDL_KEYDOWN:    if(!efck || (event->key.keysym.sym != MKK(LALT) && event->key.keysym.sym != MKK(RALT) &&
                         event->key.keysym.sym != MKK(LSHIFT) && event->key.keysym.sym != MKK(RSHIFT)))
                        {
                                efbc.ButtType = BUTTC_KEYBOARD;
                                efbc.DeviceNum = 0;
                                efbc.ButtonNum = event->key.keysym.sym;
                                if(efck)
                                        efbc.ButtonNum |= ((event->key.keysym.mod & KMOD_ALT) ? (ICSS_ALT<<24):0) | ((event->key.keysym.mod & KMOD_SHIFT) ? (ICSS_SHIFT<<24):0);
				efbcdone = 1;
                                return(1);
                        }
                        break;
   case SDL_MOUSEBUTTONDOWN:efbc.ButtType = BUTTC_MOUSE;
			    efbc.DeviceNum = 0;
			    efbc.ButtonNum = event->button.button - SDL_BUTTON_LEFT;
  		   	    efbcdone = 1;
			    printf("%d\n", efbc.ButtonNum);
			    return(1);

  case SDL_JOYBUTTONDOWN:efbc.ButtType = BUTTC_JOYSTICK;
                          efbc.DeviceNum = event->jbutton.which;
                          efbc.ButtonNum = event->jbutton.button;
			  efbc.DeviceID = GetJoystickUniqueID(event->jbutton.which);
			  efbcdone = 1;
                          return(1);
   case SDL_JOYHATMOTION:                         
			//printf("Raw: %d\n", SDL_JoystickGetHat(Joysticks[event->jhat.which], 0xFF)); //(bc->ButtonNum[x]>>8)&0x1F) & (bc->ButtonNum[x]&0xFF)));
			if(event->jhat.value != SDL_HAT_CENTERED && event->jhat.value != SDL_HAT_RIGHTUP && event->jhat.value != SDL_HAT_RIGHTDOWN &&
				event->jhat.value != SDL_HAT_LEFTUP && event->jhat.value != SDL_HAT_LEFTDOWN)
                         {
				//printf("Real: %d\n",event->jhat.value);
				//break;
                          efbc.ButtType = BUTTC_JOYSTICK;
                          efbc.DeviceNum = event->jhat.which;
                          efbc.ButtonNum = 0x2000|((event->jhat.hat&0x1F)<<8)|event->jhat.value;
			  efbc.DeviceID = GetJoystickUniqueID(event->jhat.which);
			  efbcdone = 1;
                          return(1);
                         }
                         break;
   case SDL_JOYAXISMOTION:
	//printf("%d %d %d\n", event->jaxis.which, event->jaxis.axis, event->jaxis.value);
        if(LastAx[event->jaxis.which][event->jaxis.axis]==0x100000)
        {
         if(abs(event->jaxis.value)<1000)
          LastAx[event->jaxis.which][event->jaxis.axis]=event->jaxis.value;
        }
        else
        {
         if(abs(LastAx[event->jaxis.which][event->jaxis.axis]-event->jaxis.value)>=24000)
         {
          efbc.ButtType = BUTTC_JOYSTICK;
          efbc.DeviceNum = event->jaxis.which;
          efbc.ButtonNum = 0x8000|(event->jaxis.axis)|((event->jaxis.value<0)?0x4000:0);
	  efbc.DeviceID = GetJoystickUniqueID(event->jaxis.which);
	  efbcdone = 1;

	  //puts("axis");
	  return(1);
         }
        }
        break;
  }

 return(1);
}


int DTryButtonBegin(ButtConfig *bc, int commandkey)
{
 memcpy(&efbc, bc, sizeof(ButtConfig));
 efck = commandkey;
 efbcdone = 0;

 //SDL_MDFN_ShowCursor(1);
 SDL_GetMouseState(&LastMouseX, &LastMouseY);

 MainSetEventHook(EventFilter);
 //SDL_SetEventFilter(EventFilter);
 return(1);
}

int DTryButton(void)
{
 return(efbcdone);
}

int DTryButtonEnd(ButtConfig *bc)
{
 MainSetEventHook(EventFilter);
 memcpy(bc, &efbc, sizeof(ButtConfig));

 return(1);
}
