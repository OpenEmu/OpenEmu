/*
* Glide64 - Glide video plugin for Nintendo 64 emulators.
* Copyright (c) 2002  Dave2001
* Copyright (c) 2003-2009  Sergey 'Gonetz' Lipski
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* any later version.
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

//****************************************************************
//
// Glide64 - Glide Plugin for Nintendo 64 emulators
// Project started on December 29th, 2001
//
// Authors:
// Dave2001, original author, founded the project in 2001, left it in 2002
// Gugaman, joined the project in 2002, left it in 2002
// Sergey 'Gonetz' Lipski, joined the project in 2002, main author since fall of 2002
// Hiroshi 'KoolSmoky' Morii, joined the project in 2007
//
//****************************************************************
//
// To modify Glide64:
// * Write your name and (optional)email, commented by your work, so I know who did it, and so that you can find which parts you modified when it comes time to send it to me.
// * Do NOT send me the whole project or file that you modified.  Take out your modified code sections, and tell me where to put them.  If people sent the whole thing, I would have many different versions, but no idea how to combine them all.
//
//****************************************************************
//
// Keys, used by Glide64. 
// Since key codes are different for WinAPI and SDL, this difference is managed here
// Created by Sergey 'Gonetz' Lipski, July 2009
//
//****************************************************************

#ifndef Keys_H
#define Keys_H

#define G64_VK_CONTROL  0
#define G64_VK_ALT      1
#define G64_VK_INSERT   2
#define G64_VK_LBUTTON  3
#define G64_VK_UP       4
#define G64_VK_DOWN     5
#define G64_VK_LEFT     6
#define G64_VK_RIGHT    7
#define G64_VK_SPACE    8
#define G64_VK_BACK     9
#define G64_VK_SCROLL   10
#define G64_VK_1        11
#define G64_VK_2        12
#define G64_VK_3        13
#define G64_VK_4        14
#define G64_VK_5        15
#define G64_VK_6        16
#define G64_VK_7        17
#define G64_VK_8        18
#define G64_VK_9        19
#define G64_VK_0        20
#define G64_VK_A        21
#define G64_VK_B        22
#define G64_VK_D        23
#define G64_VK_G        24
#define G64_VK_Q        25
#define G64_VK_R        26
#define G64_VK_S        27
#define G64_VK_V        28
#define G64_VK_W        29

#define G64_NUM_KEYS    30

class Glide64Keys
{
  public:
    Glide64Keys();
    ~Glide64Keys(){}
    int operator[](unsigned int index){return _keys[index];}

  private:
    int _keys[G64_NUM_KEYS];
};

#endif //Keys_H
