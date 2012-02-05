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
#include "fpu.h"

void ADD_S(void)
{
   if (check_cop1_unusable()) return;
   add_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft], reg_cop1_simple[cffd]);
   PC++;
}

void SUB_S(void)
{
   if (check_cop1_unusable()) return;
   sub_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft], reg_cop1_simple[cffd]);
   PC++;
}

void MUL_S(void)
{
   if (check_cop1_unusable()) return;
   mul_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft], reg_cop1_simple[cffd]);
   PC++;
}

void DIV_S(void)
{  
   if (check_cop1_unusable()) return;
   div_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft], reg_cop1_simple[cffd]);
   PC++;
}

void SQRT_S(void)
{
   if (check_cop1_unusable()) return;
   sqrt_s(reg_cop1_simple[cffs], reg_cop1_simple[cffd]);
   PC++;
}

void ABS_S(void)
{
   if (check_cop1_unusable()) return;
   abs_s(reg_cop1_simple[cffs], reg_cop1_simple[cffd]);
   PC++;
}

void MOV_S(void)
{
   if (check_cop1_unusable()) return;
   mov_s(reg_cop1_simple[cffs], reg_cop1_simple[cffd]);
   PC++;
}

void NEG_S(void)
{
   if (check_cop1_unusable()) return;
   neg_s(reg_cop1_simple[cffs], reg_cop1_simple[cffd]);
   PC++;
}

void ROUND_L_S(void)
{
   if (check_cop1_unusable()) return;
   round_l_s(reg_cop1_simple[cffs], (long long*)(reg_cop1_double[cffd]));
   PC++;
}

void TRUNC_L_S(void)
{
   if (check_cop1_unusable()) return;
   trunc_l_s(reg_cop1_simple[cffs], (long long*)(reg_cop1_double[cffd]));
   PC++;
}

void CEIL_L_S(void)
{
   if (check_cop1_unusable()) return;
   ceil_l_s(reg_cop1_simple[cffs], (long long*)(reg_cop1_double[cffd]));
   PC++;
}

void FLOOR_L_S(void)
{
   if (check_cop1_unusable()) return;
   floor_l_s(reg_cop1_simple[cffs], (long long*)(reg_cop1_double[cffd]));
   PC++;
}

void ROUND_W_S(void)
{
   if (check_cop1_unusable()) return;
   round_w_s(reg_cop1_simple[cffs], (int*)reg_cop1_simple[cffd]);
   PC++;
}

void TRUNC_W_S(void)
{
   if (check_cop1_unusable()) return;
   trunc_w_s(reg_cop1_simple[cffs], (int*)reg_cop1_simple[cffd]);
   PC++;
}

void CEIL_W_S(void)
{
   if (check_cop1_unusable()) return;
   ceil_w_s(reg_cop1_simple[cffs], (int*)reg_cop1_simple[cffd]);
   PC++;
}

void FLOOR_W_S(void)
{
   if (check_cop1_unusable()) return;
   floor_w_s(reg_cop1_simple[cffs], (int*)reg_cop1_simple[cffd]);
   PC++;
}

void CVT_D_S(void)
{
   if (check_cop1_unusable()) return;
   cvt_d_s(reg_cop1_simple[cffs], reg_cop1_double[cffd]);
   PC++;
}

void CVT_W_S(void)
{
   if (check_cop1_unusable()) return;
   cvt_w_s(reg_cop1_simple[cffs], (int*)reg_cop1_simple[cffd]);
   PC++;
}

void CVT_L_S(void)
{
   if (check_cop1_unusable()) return;
   cvt_l_s(reg_cop1_simple[cffs], (long long*)(reg_cop1_double[cffd]));
   PC++;
}

void C_F_S(void)
{
   if (check_cop1_unusable()) return;
   c_f_s();
   PC++;
}

void C_UN_S(void)
{
   if (check_cop1_unusable()) return;
   c_un_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft]);
   PC++;
}

void C_EQ_S(void)
{
   if (check_cop1_unusable()) return;
   c_eq_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft]);
   PC++;
}

void C_UEQ_S(void)
{
   if (check_cop1_unusable()) return;
   c_ueq_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft]);
   PC++;
}

void C_OLT_S(void)
{
   if (check_cop1_unusable()) return;
   c_olt_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft]);
   PC++;
}

void C_ULT_S(void)
{
   if (check_cop1_unusable()) return;
   c_ult_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft]);
   PC++;
}

void C_OLE_S(void)
{
   if (check_cop1_unusable()) return;
   c_ole_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft]);
   PC++;
}

void C_ULE_S(void)
{
   if (check_cop1_unusable()) return;
   c_ule_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft]);
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
   c_sf_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft]);
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
   c_ngle_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft]);
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
   c_seq_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft]);
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
   c_ngl_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft]);
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
   c_lt_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft]);
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
   c_nge_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft]);
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
   c_le_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft]);
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
   c_ngt_s(reg_cop1_simple[cffs], reg_cop1_simple[cfft]);
   PC++;
}

