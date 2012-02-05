/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - cop1.c                                                  *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Hacktarux                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ops.h"
#include "r4300.h"
#include "macros.h"
#include "exception.h"

#include "memory/memory.h"

void MFC1(void)
{
   if (check_cop1_unusable()) return;
   rrt32 = *((int*)reg_cop1_simple[rfs]);
   sign_extended(rrt);
   PC++;
}

void DMFC1(void)
{
   if (check_cop1_unusable()) return;
   rrt = *((long long*)reg_cop1_double[rfs]);
   PC++;
}

void CFC1(void)
{  
   if (check_cop1_unusable()) return;
   if (rfs==31)
     {
    rrt32 = FCR31;
    sign_extended(rrt);
     }
   if (rfs==0)
     {
    rrt32 = FCR0;
    sign_extended(rrt);
     }
   PC++;
}

void MTC1(void)
{
   if (check_cop1_unusable()) return;
   *((int*)reg_cop1_simple[rfs]) = rrt32;
   PC++;
}

void DMTC1(void)
{
   if (check_cop1_unusable()) return;
   *((long long*)reg_cop1_double[rfs]) = rrt;
   PC++;
}

void CTC1(void)
{
   if (check_cop1_unusable()) return;
   if (rfs==31)
     FCR31 = rrt32;
   switch((FCR31 & 3))
     {
      case 0:
    rounding_mode = 0x33F; // Round to nearest, or to even if equidistant
    break;
      case 1:
    rounding_mode = 0xF3F; // Truncate (toward 0)
    break;
      case 2:
    rounding_mode = 0xB3F; // Round up (toward +infinity) 
    break;
      case 3:
    rounding_mode = 0x73F; // Round down (toward -infinity) 
    break;
     }
   PC++;
}

