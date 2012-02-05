/***************************************************************************************
 *  Genesis Plus
 *  Input peripherals support
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  Eke-Eke (2007-2011), additional code & fixes for the GCN/Wii port
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************************/

#include "shared.h"
#include "gamepad.h"
#include "lightgun.h"
#include "mouse.h"
#include "activator.h"
#include "xe_a1p.h"
#include "teamplayer.h"
#include "paddle.h"
#include "sportspad.h"

t_input input;
int old_system[2] = {-1,-1};


void input_init(void)
{
  int i;
  int player = 0;

  for (i=0; i<MAX_DEVICES; i++)
  {
    input.dev[i] = NO_DEVICE;
    input.pad[i] = 0;
  }

  /* PICO tablet & pen */
  if (system_hw == SYSTEM_PICO)
  {
    input.dev[0] = DEVICE_TABLET;
    return;
  }

  switch (input.system[0])
  {
    case SYSTEM_MS_GAMEPAD:
    {
      if (player == MAX_INPUTS) return;
      input.dev[0] = DEVICE_PAD2B;
      player++;
      break;
    }

    case SYSTEM_MD_GAMEPAD:
    {
      if (player == MAX_INPUTS) return;
      input.dev[0] = config.input[player].padtype;
      player++;
      break;
    }

    case SYSTEM_MOUSE:
    {
      if (player == MAX_INPUTS) return;
      input.dev[0] = DEVICE_MOUSE;
      player++;
      break;
    }

    case SYSTEM_ACTIVATOR:
    {
      if (player == MAX_INPUTS) return;
      input.dev[0] = DEVICE_ACTIVATOR;
      player++;
      break;
    }

    case SYSTEM_XE_A1P:
    {
      if (player == MAX_INPUTS) return;
      input.dev[0] = DEVICE_XE_A1P;
      player++;
      break;
    }

    case SYSTEM_WAYPLAY:
    {
      for (i=0; i< 4; i++)
      {
        if (player == MAX_INPUTS) return;
        input.dev[i] = config.input[player].padtype;
        player++;
      }
      break;
    }

    case SYSTEM_TEAMPLAYER:
    {
      for (i=0; i<4; i++)
      {
        if (player == MAX_INPUTS) return;
        input.dev[i] = config.input[player].padtype;
        player++;
      }
      teamplayer_init(0);
      break;
    }

    case SYSTEM_LIGHTPHASER:
    {
      if (player == MAX_INPUTS) return;
      input.dev[0] = DEVICE_LIGHTGUN;
      player++;
      break;
    }

    case SYSTEM_PADDLE:
    {
      if (player == MAX_INPUTS) return;
      input.dev[0] = DEVICE_PADDLE;
      player++;
      break;
    }

    case SYSTEM_SPORTSPAD:
    {
      if (player == MAX_INPUTS) return;
      input.dev[0] = DEVICE_SPORTSPAD;
      player++;
      break;
    }
  }

  switch (input.system[1])
  {
    case SYSTEM_MS_GAMEPAD:
    {
      if (player == MAX_INPUTS) return;
      input.dev[4] = DEVICE_PAD2B;
      player++;
      break;
    }

    case SYSTEM_MD_GAMEPAD:
    {
      if (player == MAX_INPUTS) return;
      input.dev[4] = config.input[player].padtype;
      player++;
      break;
    }

    case SYSTEM_MOUSE:
    {
      if (player == MAX_INPUTS) return;
      input.dev[4] = DEVICE_MOUSE;
      player++;
      break;
    }

    case SYSTEM_ACTIVATOR:
    {
      if (player == MAX_INPUTS) return;
      input.dev[4] = DEVICE_ACTIVATOR;
      player++;
      break;
    }

    case SYSTEM_MENACER:
    {
      if (player == MAX_INPUTS) return;
      input.dev[4] = DEVICE_LIGHTGUN;
      player++;
      break;
    }

    case SYSTEM_JUSTIFIER:
    {
      for (i=4; i<6; i++)
      {
        if (player == MAX_INPUTS) return;
        input.dev[i] = DEVICE_LIGHTGUN;
        player++;
      }
      break;
    }

    case SYSTEM_TEAMPLAYER:
    {
      for (i=4; i<8; i++)
      {
        if (player == MAX_INPUTS) return;
        input.dev[i] = config.input[player].padtype;
        player++;
      }
      teamplayer_init(1);
      break;
    }

    case SYSTEM_LIGHTPHASER:
    {
      if (player == MAX_INPUTS) return;
      input.dev[4] = DEVICE_LIGHTGUN;
      player++;
      break;
    }

    case SYSTEM_PADDLE:
    {
      if (player == MAX_INPUTS) return;
      input.dev[4] = DEVICE_PADDLE;
      player++;
      break;
    }

    case SYSTEM_SPORTSPAD:
    {
      if (player == MAX_INPUTS) return;
      input.dev[4] = DEVICE_SPORTSPAD;
      player++;
      break;
    }
  }

  /* J-CART */
  if (cart.special & HW_J_CART)
  {
    /* two additional gamepads */
    for (i=5; i<7; i++)
    {
      if (player == MAX_INPUTS) return;
      input.dev[i] = config.input[player].padtype;
      player ++;
    }
  }
}

void input_reset(void)
{
  /* Reset input devices */
  int i;
  for (i=0; i<MAX_DEVICES; i++)
  {
    switch (input.dev[i])
    {
      case DEVICE_PAD2B:
      case DEVICE_PAD3B:
      case DEVICE_PAD6B:
      {
        gamepad_reset(i);
        break;
      }

      case DEVICE_LIGHTGUN:
      {
        lightgun_reset(i);
        break;
      }

      case DEVICE_MOUSE:
      {
        mouse_reset(i);
        break;
      }

      case DEVICE_ACTIVATOR:
      {
        activator_reset(i >> 2);
        break;
      }

      case DEVICE_XE_A1P:
      {
        xe_a1p_reset();
        break;
      }

      case DEVICE_PADDLE:
      {
        paddle_reset(i >> 2);
        break;
      }

      case DEVICE_SPORTSPAD:
      {
        sportspad_reset(i >> 2);
        break;
      }

      default:
      {
        break;
      }
    }
  }

  /* Team Player */
  for (i=0; i<2; i++)
  {
    if (input.system[i] == SYSTEM_TEAMPLAYER)
    {
      teamplayer_reset(i);
    }
  }
}

void input_refresh(void)
{
  int i;
  for (i=0; i<MAX_DEVICES; i++)
  {
    switch (input.dev[i])
    {
      case DEVICE_PAD6B:
      {
        gamepad_refresh(i);
        break;
      }

      case DEVICE_LIGHTGUN:
      {
        lightgun_refresh(i);
        break;
      }
    }
  }
}
