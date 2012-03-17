/***************************************************************************
 * Gens: Input Class - DirectInput 5                                       *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "input_osx.hpp"
#include "io.h"

Input_OSX::Input_OSX()
{
	for(int i = 0; i < 8; i++)
	{
        playerMasks[i] = 0;
        m_keyMap[i].Start = CONTROLLER_START | (1 << (16 + i));
        m_keyMap[i].Mode  = CONTROLLER_MODE  | (1 << (16 + i));
        m_keyMap[i].A     = CONTROLLER_A     | (1 << (16 + i));
        m_keyMap[i].B     = CONTROLLER_B     | (1 << (16 + i));
        m_keyMap[i].C     = CONTROLLER_C     | (1 << (16 + i));
        m_keyMap[i].X     = CONTROLLER_X     | (1 << (16 + i));
        m_keyMap[i].Y     = CONTROLLER_Y     | (1 << (16 + i));
        m_keyMap[i].Z     = CONTROLLER_Z     | (1 << (16 + i));
        m_keyMap[i].Up    = CONTROLLER_UP    | (1 << (16 + i));
        m_keyMap[i].Down  = CONTROLLER_DOWN  | (1 << (16 + i));
        m_keyMap[i].Left  = CONTROLLER_LEFT  | (1 << (16 + i));
        m_keyMap[i].Right = CONTROLLER_RIGHT | (1 << (16 + i));
	}
}


Input_OSX::~Input_OSX()
{

}


/**
 * joyExists(): Check if the specified joystick exists.
 * @param joyNum Joystick number.
 * @return true if the joystick exists; false if it does not exist.
 */
bool Input_OSX::joyExists(int joyNum)
{    
    return 0 < joyNum && joyNum < 8;
}


/**
 * getKey(): Get a key. (Used for controller configuration.)
 * @return Key value.
 */
unsigned int Input_OSX::getKey(void)
{
    return 0;
}


/**
 * update(): Update the input subsystem.
 */
void Input_OSX::update(void)
{
}


void Input_OSX::pressKey(unsigned player, unsigned key)
{
    if(player < 8) playerMasks[player] |= key;
}
void Input_OSX::releaseKey(unsigned player, unsigned key)
{
    if(player < 8) playerMasks[player] &= ~key;
}

#define GET_PLAYER(mask) ((unsigned)log2((mask) >> 16))

/**
 * checkKeyPressed(): Checks if the specified key is pressed.
 * @param key Key to check.
 * @return True if the key is pressed.
 */
bool Input_OSX::checkKeyPressed(unsigned int key)
{
    unsigned player = GET_PLAYER(key);
	return (player < 8 ? playerMasks[player] & key : false);
}
