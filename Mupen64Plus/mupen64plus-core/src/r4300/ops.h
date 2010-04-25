/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - ops.h                                                   *
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

void NI(void);
void LW(void);
void LUI(void);
void ADDIU(void);
void BNE(void);
void SLL(void);
void SW(void);
void ORI(void);
void ADDI(void);
void OR(void);
void JAL(void);
void SLTI(void);
void BEQL(void);
void ANDI(void);
void XORI(void);
void JR(void);
void SRL(void);
void BNEL(void);
void BEQ(void);
void BLEZL(void);
void SUBU(void);
void MULTU(void);
void MFLO(void);
void ADDU(void);
void SB(void);
void AND(void);
void LBU(void);
void BGEZL(void);
void SLT(void);
void ADD(void);
void CACHE(void);
void SLTU(void);
void SRLV(void);
void SLLV(void);
void XOR(void);
void BGEZAL(void);
void CFC1(void);
void MTC0(void);
void MFC0(void);
void CTC1(void);
void BLEZ(void);
void TLBWI(void);
void LD(void);
void DMULTU(void);
void DSLL32(void);
void DSRA32(void);
void DDIVU(void);
void SRA(void);
void SLTIU(void);
void SH(void);
void LHU(void);
void MTLO(void);
void MTHI(void);
void ERET(void);
void SD(void);
void J(void);
void DIV(void);
void MFHI(void);
void BGEZ(void);
void MULT(void);
void LWC1(void);
void MTC1(void);
void CVT_S_W(void);
void DIV_S(void);
void MUL_S(void);
void ADD_S(void);
void CVT_D_S(void);
void ADD_D(void);
void TRUNC_W_D(void);
void MFC1(void);
void NOP(void);
void RESERVED(void);

void TLBP(void);
void TLBR(void);
void SWL(void);
void SWR(void);
void LWL(void);
void LWR(void);
void SRAV(void);
void BLTZ(void);

void BGTZ(void);
void LB(void);

void SWC1(void);
void CVT_D_W(void);
void MUL_D(void);
void DIV_D(void);
void CVT_S_D(void);
void MOV_S(void);
void C_LE_S(void);
void BC1T(void);
void TRUNC_W_S(void);
void C_LT_S(void);
void BC1FL(void);
void NEG_S(void);
void LDC1(void);
void SUB_D(void);
void C_LE_D(void);
void BC1TL(void);
void BGEZAL_IDLE(void);
void J_IDLE(void);
void BLEZ_IDLE(void);
void BEQ_IDLE(void);

void LH(void);
void NOR(void);
void NEG_D(void);
void MOV_D(void);
void C_LT_D(void);
void BC1F(void);

void SUB(void);

void CVT_W_S(void);
void DIVU(void);

void JALR(void);
void SDC1(void);
void C_EQ_S(void);
void SUB_S(void);
void BLTZL(void);

void CVT_W_D(void);
void SQRT_S(void);
void C_EQ_D(void);
void FIN_BLOCK(void);
void DDIV(void);
void DADDIU(void);
void ABS_S(void);
void BGTZL(void);
void DSRAV(void);
void DSLLV(void);
void CVT_S_L(void);
void DMTC1(void);
void DSRLV(void);
void DSRA(void);
void DMULT(void);
void DSLL(void);
void SC(void);

void SYSCALL(void);
void DADD(void);
void DADDU(void);
void DSUB(void);
void DSUBU(void);
void TEQ(void);
void DSRL(void);
void DSRL32(void);
void BLTZ_IDLE(void);
void BGEZ_IDLE(void);
void BLTZL_IDLE(void);
void BGEZL_IDLE(void);
void BLTZAL(void);
void BLTZAL_IDLE(void);
void BLTZALL(void);
void BLTZALL_IDLE(void);
void BGEZALL(void);
void BGEZALL_IDLE(void);
void TLBWR(void);
void BC1F_IDLE(void);
void BC1T_IDLE(void);
void BC1FL_IDLE(void);
void BC1TL_IDLE(void);
void ROUND_L_S(void);
void TRUNC_L_S(void);
void CEIL_L_S(void);
void FLOOR_L_S(void);
void ROUND_W_S(void);
void CEIL_W_S(void);
void FLOOR_W_S(void);
void CVT_L_S(void);
void C_F_S(void);
void C_UN_S(void);
void C_UEQ_S(void);
void C_OLT_S(void);
void C_ULT_S(void);
void C_OLE_S(void);
void C_ULE_S(void);
void C_SF_S(void);
void C_NGLE_S(void);
void C_SEQ_S(void);
void C_NGL_S(void);
void C_NGE_S(void);
void C_NGT_S(void);
void SQRT_D(void);
void ABS_D(void);
void ROUND_L_D(void);
void TRUNC_L_D(void);
void CEIL_L_D(void);
void FLOOR_L_D(void);
void ROUND_W_D(void);
void CEIL_W_D(void);
void FLOOR_W_D(void);
void CVT_L_D(void);
void C_F_D(void);
void C_UN_D(void);
void C_UEQ_D(void);
void C_OLT_D(void);
void C_ULT_D(void);
void C_OLE_D(void);
void C_ULE_D(void);
void C_SF_D(void);
void C_NGLE_D(void);
void C_SEQ_D(void);
void C_NGL_D(void);
void C_NGE_D(void);
void C_NGT_D(void);
void CVT_D_L(void);
void DMFC1(void);
void JAL_IDLE(void);
void BNE_IDLE(void);
void BGTZ_IDLE(void);
void BEQL_IDLE(void);
void BNEL_IDLE(void);
void BLEZL_IDLE(void);
void BGTZL_IDLE(void);
void DADDI(void);
void LDL(void);
void LDR(void);
void LWU(void);
void SDL(void);
void SDR(void);
void SYNC(void);
void BLTZ_OUT(void);
void BGEZ_OUT(void);
void BLTZL_OUT(void);
void BGEZL_OUT(void);
void BLTZAL_OUT(void);
void BGEZAL_OUT(void);
void BLTZALL_OUT(void);
void BGEZALL_OUT(void);
void BC1F_OUT(void);
void BC1T_OUT(void);
void BC1FL_OUT(void);
void BC1TL_OUT(void);
void J_OUT(void);
void JAL_OUT(void);
void BEQ_OUT(void);
void BNE_OUT(void);
void BLEZ_OUT(void);
void BGTZ_OUT(void);
void BEQL_OUT(void);
void BNEL_OUT(void);
void BLEZL_OUT(void);
void BGTZL_OUT(void);
void NOTCOMPILED(void);
void LL(void);
void NOTCOMPILED2(void);

