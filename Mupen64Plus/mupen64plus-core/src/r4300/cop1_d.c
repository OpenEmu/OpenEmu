/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - cop1_d.c                                                *
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

void ADD_D(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_double[cffd] = *reg_cop1_double[cffs] +
     *reg_cop1_double[cfft];
   PC++;
}

void SUB_D(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_double[cffd] = *reg_cop1_double[cffs] -
     *reg_cop1_double[cfft];
   PC++;
}

void MUL_D(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_double[cffd] = *reg_cop1_double[cffs] *
     *reg_cop1_double[cfft];
   PC++;
}

void DIV_D(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_double[cffd] = *reg_cop1_double[cffs] /
     *reg_cop1_double[cfft];
   PC++;
}

void SQRT_D(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_double[cffd] = sqrt(*reg_cop1_double[cffs]);
   PC++;
}

void ABS_D(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_double[cffd] = fabs(*reg_cop1_double[cffs]);
   PC++;
}

void MOV_D(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_double[cffd] = *reg_cop1_double[cffs];
   PC++;
}

void NEG_D(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_double[cffd] = -(*reg_cop1_double[cffs]);
   PC++;
}

void ROUND_L_D(void)
{
   if (check_cop1_unusable()) return;
   set_round();
   *((long long*)(reg_cop1_double[cffd])) = (long long) *reg_cop1_double[cffs];
   PC++;
}

void TRUNC_L_D(void)
{
   if (check_cop1_unusable()) return;
   set_trunc();
   *((long long*)(reg_cop1_double[cffd])) = (long long) *reg_cop1_double[cffs];
   PC++;
}

void CEIL_L_D(void)
{
   if (check_cop1_unusable()) return;
   set_ceil();
   *((long long*)(reg_cop1_double[cffd])) = (long long) *reg_cop1_double[cffs];
   PC++;
}

void FLOOR_L_D(void)
{
   if (check_cop1_unusable()) return;
   set_floor();
   *((long long*)(reg_cop1_double[cffd])) = (long long) *reg_cop1_double[cffs];
   PC++;
}

void ROUND_W_D(void)
{
   if (check_cop1_unusable()) return;
   set_round();
   *((int*)reg_cop1_simple[cffd]) = (int) *reg_cop1_double[cffs];
   PC++;
}

void TRUNC_W_D(void)
{
   if (check_cop1_unusable()) return;
   set_trunc();
   *((int*)reg_cop1_simple[cffd]) = (int) *reg_cop1_double[cffs];
   PC++;
}

void CEIL_W_D(void)
{
   if (check_cop1_unusable()) return;
   set_ceil();
   *((int*)reg_cop1_simple[cffd]) = (int) *reg_cop1_double[cffs];
   PC++;
}

void FLOOR_W_D(void)
{
   if (check_cop1_unusable()) return;
   set_floor();
   *((int*)reg_cop1_simple[cffd]) = (int) *reg_cop1_double[cffs];
   PC++;
}

void CVT_S_D(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *reg_cop1_simple[cffd] = (float) *reg_cop1_double[cffs];
   PC++;
}

void CVT_W_D(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *((int*)reg_cop1_simple[cffd]) = (int) *reg_cop1_double[cffs];
   PC++;
}

void CVT_L_D(void)
{
   if (check_cop1_unusable()) return;
   set_rounding();
   *((long long*)(reg_cop1_double[cffd])) = (long long) *reg_cop1_double[cffs];
   PC++;
}

void C_F_D(void)
{
   if (check_cop1_unusable()) return;
   FCR31 &= ~0x800000;
   PC++;
}

void C_UN_D(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_double[cffs]) || isnan(*reg_cop1_double[cfft]))
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_EQ_D(void)
{
   if (check_cop1_unusable()) return;
   if (!isnan(*reg_cop1_double[cffs]) && !isnan(*reg_cop1_double[cfft]) &&
       *reg_cop1_double[cffs] == *reg_cop1_double[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_UEQ_D(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_double[cffs]) || isnan(*reg_cop1_double[cfft]) ||
       *reg_cop1_double[cffs] == *reg_cop1_double[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_OLT_D(void)
{
   if (check_cop1_unusable()) return;
   if (!isnan(*reg_cop1_double[cffs]) && !isnan(*reg_cop1_double[cfft]) &&
       *reg_cop1_double[cffs] < *reg_cop1_double[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_ULT_D(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_double[cffs]) || isnan(*reg_cop1_double[cfft]) ||
       *reg_cop1_double[cffs] < *reg_cop1_double[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_OLE_D(void)
{
   if (check_cop1_unusable()) return;
   if (!isnan(*reg_cop1_double[cffs]) && !isnan(*reg_cop1_double[cfft]) &&
       *reg_cop1_double[cffs] <= *reg_cop1_double[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_ULE_D(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_double[cffs]) || isnan(*reg_cop1_double[cfft]) ||
       *reg_cop1_double[cffs] <= *reg_cop1_double[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_SF_D(void)
{
   if (isnan(*reg_cop1_double[cffs]) || isnan(*reg_cop1_double[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   FCR31 &= ~0x800000;
   PC++;
}

void C_NGLE_D(void)
{
   if (isnan(*reg_cop1_double[cffs]) || isnan(*reg_cop1_double[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   FCR31 &= ~0x800000;
   PC++;
}

void C_SEQ_D(void)
{
   if (isnan(*reg_cop1_double[cffs]) || isnan(*reg_cop1_double[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   if (*reg_cop1_double[cffs] == *reg_cop1_double[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_NGL_D(void)
{
   if (isnan(*reg_cop1_double[cffs]) || isnan(*reg_cop1_double[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   if (*reg_cop1_double[cffs] == *reg_cop1_double[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_LT_D(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_double[cffs]) || isnan(*reg_cop1_double[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   if (*reg_cop1_double[cffs] < *reg_cop1_double[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_NGE_D(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_double[cffs]) || isnan(*reg_cop1_double[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   if (*reg_cop1_double[cffs] < *reg_cop1_double[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_LE_D(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_double[cffs]) || isnan(*reg_cop1_double[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   if (*reg_cop1_double[cffs] <= *reg_cop1_double[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

void C_NGT_D(void)
{
   if (check_cop1_unusable()) return;
   if (isnan(*reg_cop1_double[cffs]) || isnan(*reg_cop1_double[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   if (*reg_cop1_double[cffs] <= *reg_cop1_double[cfft])
     FCR31 |= 0x800000;
   else FCR31 &= ~0x800000;
   PC++;
}

