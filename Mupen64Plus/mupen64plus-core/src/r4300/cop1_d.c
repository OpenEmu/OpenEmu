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
#include "fpu.h"

void ADD_D(void)
{
   if (check_cop1_unusable()) return;
   add_d(reg_cop1_double[cffs], reg_cop1_double[cfft], reg_cop1_double[cffd]);
   PC++;
}

void SUB_D(void)
{
   if (check_cop1_unusable()) return;
   sub_d(reg_cop1_double[cffs], reg_cop1_double[cfft], reg_cop1_double[cffd]);
   PC++;
}

void MUL_D(void)
{
   if (check_cop1_unusable()) return;
   mul_d(reg_cop1_double[cffs], reg_cop1_double[cfft], reg_cop1_double[cffd]);
   PC++;
}

void DIV_D(void)
{
   if (check_cop1_unusable()) return;
   div_d(reg_cop1_double[cffs], reg_cop1_double[cfft], reg_cop1_double[cffd]);
   PC++;
}

void SQRT_D(void)
{
   if (check_cop1_unusable()) return;
   sqrt_d(reg_cop1_double[cffs], reg_cop1_double[cffd]);
   PC++;
}

void ABS_D(void)
{
   if (check_cop1_unusable()) return;
   abs_d(reg_cop1_double[cffs], reg_cop1_double[cffd]);
   PC++;
}

void MOV_D(void)
{
   if (check_cop1_unusable()) return;
   mov_d(reg_cop1_double[cffs], reg_cop1_double[cffd]);
   PC++;
}

void NEG_D(void)
{
   if (check_cop1_unusable()) return;
   neg_d(reg_cop1_double[cffs], reg_cop1_double[cffd]);
   PC++;
}

void ROUND_L_D(void)
{
   if (check_cop1_unusable()) return;
   round_l_d(reg_cop1_double[cffs], (long long*)(reg_cop1_double[cffd]));
   PC++;
}

void TRUNC_L_D(void)
{
   if (check_cop1_unusable()) return;
   trunc_l_d(reg_cop1_double[cffs], (long long*)(reg_cop1_double[cffd]));
   PC++;
}

void CEIL_L_D(void)
{
   if (check_cop1_unusable()) return;
   ceil_l_d(reg_cop1_double[cffs], (long long*)(reg_cop1_double[cffd]));
   PC++;
}

void FLOOR_L_D(void)
{
   if (check_cop1_unusable()) return;
   floor_l_d(reg_cop1_double[cffs], (long long*)(reg_cop1_double[cffd]));
   PC++;
}

void ROUND_W_D(void)
{
   if (check_cop1_unusable()) return;
   round_w_d(reg_cop1_double[cffs], (int*)reg_cop1_simple[cffd]);
   PC++;
}

void TRUNC_W_D(void)
{
   if (check_cop1_unusable()) return;
   trunc_w_d(reg_cop1_double[cffs], (int*)reg_cop1_simple[cffd]);
   PC++;
}

void CEIL_W_D(void)
{
   if (check_cop1_unusable()) return;
   ceil_w_d(reg_cop1_double[cffs], (int*)reg_cop1_simple[cffd]);
   PC++;
}

void FLOOR_W_D(void)
{
   if (check_cop1_unusable()) return;
   floor_w_d(reg_cop1_double[cffs], (int*)reg_cop1_simple[cffd]);
   PC++;
}

void CVT_S_D(void)
{
   if (check_cop1_unusable()) return;
   cvt_s_d(reg_cop1_double[cffs], reg_cop1_simple[cffd]);
   PC++;
}

void CVT_W_D(void)
{
   if (check_cop1_unusable()) return;
   cvt_w_d(reg_cop1_double[cffs], (int*)reg_cop1_simple[cffd]);
   PC++;
}

void CVT_L_D(void)
{
   if (check_cop1_unusable()) return;
   cvt_l_d(reg_cop1_double[cffs], (long long*)(reg_cop1_double[cffd]));
   PC++;
}

void C_F_D(void)
{
   if (check_cop1_unusable()) return;
   c_f_d();
   PC++;
}

void C_UN_D(void)
{
   if (check_cop1_unusable()) return;
   c_un_d(reg_cop1_double[cffs], reg_cop1_double[cfft]);
   PC++;
}

void C_EQ_D(void)
{
   if (check_cop1_unusable()) return;
   c_eq_d(reg_cop1_double[cffs], reg_cop1_double[cfft]);
   PC++;
}

void C_UEQ_D(void)
{
   if (check_cop1_unusable()) return;
   c_ueq_d(reg_cop1_double[cffs], reg_cop1_double[cfft]);
   PC++;
}

void C_OLT_D(void)
{
   if (check_cop1_unusable()) return;
   c_olt_d(reg_cop1_double[cffs], reg_cop1_double[cfft]);
   PC++;
}

void C_ULT_D(void)
{
   if (check_cop1_unusable()) return;
   c_ult_d(reg_cop1_double[cffs], reg_cop1_double[cfft]);
   PC++;
}

void C_OLE_D(void)
{
   if (check_cop1_unusable()) return;
   c_ole_d(reg_cop1_double[cffs], reg_cop1_double[cfft]);
   PC++;
}

void C_ULE_D(void)
{
   if (check_cop1_unusable()) return;
   c_ule_d(reg_cop1_double[cffs], reg_cop1_double[cfft]);
   PC++;
}

void C_SF_D(void)
{
   if (isnan(*reg_cop1_double[cffs]) || isnan(*reg_cop1_double[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   c_sf_d(reg_cop1_double[cffs], reg_cop1_double[cfft]);
   PC++;
}

void C_NGLE_D(void)
{
   if (isnan(*reg_cop1_double[cffs]) || isnan(*reg_cop1_double[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   c_ngle_d(reg_cop1_double[cffs], reg_cop1_double[cfft]);
   PC++;
}

void C_SEQ_D(void)
{
   if (isnan(*reg_cop1_double[cffs]) || isnan(*reg_cop1_double[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   c_seq_d(reg_cop1_double[cffs], reg_cop1_double[cfft]);
   PC++;
}

void C_NGL_D(void)
{
   if (isnan(*reg_cop1_double[cffs]) || isnan(*reg_cop1_double[cfft]))
   {
     DebugMessage(M64MSG_ERROR, "Invalid operation exception in C opcode");
     stop=1;
   }
   c_ngl_d(reg_cop1_double[cffs], reg_cop1_double[cfft]);
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
   c_lt_d(reg_cop1_double[cffs], reg_cop1_double[cfft]);
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
   c_nge_d(reg_cop1_double[cffs], reg_cop1_double[cfft]);
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
   c_le_d(reg_cop1_double[cffs], reg_cop1_double[cfft]);
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
   c_ngt_d(reg_cop1_double[cffs], reg_cop1_double[cfft]);
   PC++;
}

