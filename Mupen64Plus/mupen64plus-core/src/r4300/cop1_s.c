/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - cop1_s.c                                                *
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

#include <math.h>

#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "osal/preproc.h"

#include "r4300.h"
#include "ops.h"
#include "macros.h"

void ADD_S(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_simple[cffd] = *reg_cop1_simple[cffs] +
     *reg_cop1_simple[cfft];
   PC++;
}

void SUB_S(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_simple[cffd] = *reg_cop1_simple[cffs] -
     *reg_cop1_simple[cfft];
   PC++;
}

void MUL_S(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_simple[cffd] = *reg_cop1_simple[cffs] *
     *reg_cop1_simple[cfft];
   PC++;
}

void DIV_S(void)
{  
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_simple[cffd] = *reg_cop1_simple[cffs] /
     *reg_cop1_simple[cfft];
   PC++;
}

void SQRT_S(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_simple[cffd] = (float) sqrt(*reg_cop1_simple[cffs]);
   PC++;
}

void ABS_S(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_simple[cffd] = (float) fabs(*reg_cop1_simple[cffs]);
   PC++;
}

void MOV_S(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_simple[cffd] = *reg_cop1_simple[cffs];
   PC++;
}

void NEG_S(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_simple[cffd] = -(*reg_cop1_simple[cffs]);
   PC++;
}

void ROUND_L_S(void)
{
   if (check_cop1_unusable()) return;
   set_round();
   *((long long*)(reg_cop1_double[cffd])) = (long long) *reg_cop1_simple[cffs];
   PC++;
}

void TRUNC_L_S(void)
{
   if (check_cop1_unusable()) return;
   set_trunc();
   *((long long*)(reg_cop1_double[cffd])) = (long long) *reg_cop1_simple[cffs];
   PC++;
}

void CEIL_L_S(void)
{
   if (check_cop1_unusable()) return;
   set_ceil();
   *((long long*)(reg_cop1_double[cffd])) = (long long) *reg_cop1_simple[cffs];
   PC++;
}

void FLOOR_L_S(void)
{
   if (check_cop1_unusable()) return;
   set_floor();
   *((long long*)(reg_cop1_double[cffd])) = (long long) *reg_cop1_simple[cffs];
   PC++;
}

void ROUND_W_S(void)
{
   if (check_cop1_unusable()) return;
   set_round();
   *((int*)reg_cop1_simple[cffd]) = (int) *reg_cop1_simple[cffs];
   PC++;
}

void TRUNC_W_S(void)
{
   if (check_cop1_unusable()) return;
   set_trunc();
   *((int*)reg_cop1_simple[cffd]) = (int) *reg_cop1_simple[cffs];
   PC++;
}

void CEIL_W_S(void)
{
   if (check_cop1_unusable()) return;
   set_ceil();
   *((int*)reg_cop1_simple[cffd]) = (int) *reg_cop1_simple[cffs];
   PC++;
}

void FLOOR_W_S(void)
{
   if (check_cop1_unusable()) return;
   set_floor();
   *((int*)reg_cop1_simple[cffd]) = (int) *reg_cop1_simple[cffs];
   PC++;
}

void CVT_D_S(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_double[cffd] = *reg_cop1_simple[cffs];
   PC++;
}

void CVT_W_S(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *((int*)reg_cop1_simple[cffd]) = (int) *reg_cop1_simple[cffs];
   PC++;
}

void CVT_L_S(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *((long long*)(reg_cop1_double[cffd])) = (long long) *reg_cop1_simple[cffs];
   PC++;
}

void C_F_S(void)
{
   if (check_cop1_unusable()) return;
   FCR31 &= ~0x800000;
   PC++;
}

void C_UN_S(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_simple[cffs]) || isnan(*reg_cop1_simple[cfft]))
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_EQ_S(void)
{
   if (check_cop1_unusable()) return;
   if (!isnan(*reg_cop1_simple[cffs]) && !isnan(*reg_cop1_simple[cfft]) &&
       *reg_cop1_simple[cffs] == *reg_cop1_simple[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_UEQ_S(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_simple[cffs]) || isnan(*reg_cop1_simple[cfft]) ||
       *reg_cop1_simple[cffs] == *reg_cop1_simple[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_OLT_S(void)
{
   if (check_cop1_unusable()) return;
   if (!isnan(*reg_cop1_simple[cffs]) && !isnan(*reg_cop1_simple[cfft]) &&
       *reg_cop1_simple[cffs] < *reg_cop1_simple[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_ULT_S(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_simple[cffs]) || isnan(*reg_cop1_simple[cfft]) ||
       *reg_cop1_simple[cffs] < *reg_cop1_simple[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_OLE_S(void)
{
   if (check_cop1_unusable()) return;
   if (!isnan(*reg_cop1_simple[cffs]) && !isnan(*reg_cop1_simple[cfft]) &&
       *reg_cop1_simple[cffs] <= *reg_cop1_simple[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_ULE_S(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_simple[cffs]) || isnan(*reg_cop1_simple[cfft]) ||
       *reg_cop1_simple[cffs] <= *reg_cop1_simple[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_SF_S(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_simple[cffs]) || isnan(*reg_cop1_simple[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   FCR31 &= ~0x800000;
   PC++;
}

void C_NGLE_S(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_simple[cffs]) || isnan(*reg_cop1_simple[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   FCR31 &= ~0x800000;
   PC++;
}

void C_SEQ_S(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_simple[cffs]) || isnan(*reg_cop1_simple[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   if (*reg_cop1_simple[cffs] == *reg_cop1_simple[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_NGL_S(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_simple[cffs]) || isnan(*reg_cop1_simple[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   if (*reg_cop1_simple[cffs] == *reg_cop1_simple[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_LT_S(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_simple[cffs]) || isnan(*reg_cop1_simple[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   if (*reg_cop1_simple[cffs] < *reg_cop1_simple[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_NGE_S(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_simple[cffs]) || isnan(*reg_cop1_simple[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   if (*reg_cop1_simple[cffs] < *reg_cop1_simple[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_LE_S(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_simple[cffs]) || isnan(*reg_cop1_simple[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   if (*reg_cop1_simple[cffs] <= *reg_cop1_simple[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_NGT_S(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_simple[cffs]) || isnan(*reg_cop1_simple[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   if (*reg_cop1_simple[cffs] <= *reg_cop1_simple[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

