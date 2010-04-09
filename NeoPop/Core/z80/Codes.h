/** Z80: portable Z80 emulator *******************************/
/**                                                         **/
/**                          Codes.h                        **/
/**                                                         **/
/** This file contains implementation for the main table of **/
/** Z80 commands. It is included from Z80.c.                **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994-2002                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

case JR_NZ:   if(R->AF.B.l&Z_FLAG) R->PC.W++; else { R->ICount-=5;M_JR; } break;
case JR_NC:   if(R->AF.B.l&C_FLAG) R->PC.W++; else { R->ICount-=5;M_JR; } break;
case JR_Z:    if(R->AF.B.l&Z_FLAG) { R->ICount-=5;M_JR; } else R->PC.W++; break;
case JR_C:    if(R->AF.B.l&C_FLAG) { R->ICount-=5;M_JR; } else R->PC.W++; break;

case JP_NZ:   if(R->AF.B.l&Z_FLAG) R->PC.W+=2; else { M_JP; } break;
case JP_NC:   if(R->AF.B.l&C_FLAG) R->PC.W+=2; else { M_JP; } break;
case JP_PO:   if(R->AF.B.l&P_FLAG) R->PC.W+=2; else { M_JP; } break;
case JP_P:    if(R->AF.B.l&S_FLAG) R->PC.W+=2; else { M_JP; } break;
case JP_Z:    if(R->AF.B.l&Z_FLAG) { M_JP; } else R->PC.W+=2; break;
case JP_C:    if(R->AF.B.l&C_FLAG) { M_JP; } else R->PC.W+=2; break;
case JP_PE:   if(R->AF.B.l&P_FLAG) { M_JP; } else R->PC.W+=2; break;
case JP_M:    if(R->AF.B.l&S_FLAG) { M_JP; } else R->PC.W+=2; break;

case RET_NZ:  if(!(R->AF.B.l&Z_FLAG)) { R->ICount-=6;M_RET; } break;
case RET_NC:  if(!(R->AF.B.l&C_FLAG)) { R->ICount-=6;M_RET; } break;
case RET_PO:  if(!(R->AF.B.l&P_FLAG)) { R->ICount-=6;M_RET; } break;
case RET_P:   if(!(R->AF.B.l&S_FLAG)) { R->ICount-=6;M_RET; } break;
case RET_Z:   if(R->AF.B.l&Z_FLAG)    { R->ICount-=6;M_RET; } break;
case RET_C:   if(R->AF.B.l&C_FLAG)    { R->ICount-=6;M_RET; } break;
case RET_PE:  if(R->AF.B.l&P_FLAG)    { R->ICount-=6;M_RET; } break;
case RET_M:   if(R->AF.B.l&S_FLAG)    { R->ICount-=6;M_RET; } break;

case CALL_NZ: if(R->AF.B.l&Z_FLAG) R->PC.W+=2; else { R->ICount-=7;M_CALL; } break;
case CALL_NC: if(R->AF.B.l&C_FLAG) R->PC.W+=2; else { R->ICount-=7;M_CALL; } break;
case CALL_PO: if(R->AF.B.l&P_FLAG) R->PC.W+=2; else { R->ICount-=7;M_CALL; } break;
case CALL_P:  if(R->AF.B.l&S_FLAG) R->PC.W+=2; else { R->ICount-=7;M_CALL; } break;
case CALL_Z:  if(R->AF.B.l&Z_FLAG) { R->ICount-=7;M_CALL; } else R->PC.W+=2; break;
case CALL_C:  if(R->AF.B.l&C_FLAG) { R->ICount-=7;M_CALL; } else R->PC.W+=2; break;
case CALL_PE: if(R->AF.B.l&P_FLAG) { R->ICount-=7;M_CALL; } else R->PC.W+=2; break;
case CALL_M:  if(R->AF.B.l&S_FLAG) { R->ICount-=7;M_CALL; } else R->PC.W+=2; break;

case ADD_B:    M_ADD(R->BC.B.h);break;
case ADD_C:    M_ADD(R->BC.B.l);break;
case ADD_D:    M_ADD(R->DE.B.h);break;
case ADD_E:    M_ADD(R->DE.B.l);break;
case ADD_H:    M_ADD(R->HL.B.h);break;
case ADD_L:    M_ADD(R->HL.B.l);break;
case ADD_A:    M_ADD(R->AF.B.h);break;
case ADD_xHL:  I=RdZ80(R->HL.W);M_ADD(I);break;
case ADD_BYTE: I=RdZ80(R->PC.W++);M_ADD(I);break;

case SUB_B:    M_SUB(R->BC.B.h);break;
case SUB_C:    M_SUB(R->BC.B.l);break;
case SUB_D:    M_SUB(R->DE.B.h);break;
case SUB_E:    M_SUB(R->DE.B.l);break;
case SUB_H:    M_SUB(R->HL.B.h);break;
case SUB_L:    M_SUB(R->HL.B.l);break;
case SUB_A:    R->AF.B.h=0;R->AF.B.l=N_FLAG|Z_FLAG;break;
case SUB_xHL:  I=RdZ80(R->HL.W);M_SUB(I);break;
case SUB_BYTE: I=RdZ80(R->PC.W++);M_SUB(I);break;

case AND_B:    M_AND(R->BC.B.h);break;
case AND_C:    M_AND(R->BC.B.l);break;
case AND_D:    M_AND(R->DE.B.h);break;
case AND_E:    M_AND(R->DE.B.l);break;
case AND_H:    M_AND(R->HL.B.h);break;
case AND_L:    M_AND(R->HL.B.l);break;
case AND_A:    M_AND(R->AF.B.h);break;
case AND_xHL:  I=RdZ80(R->HL.W);M_AND(I);break;
case AND_BYTE: I=RdZ80(R->PC.W++);M_AND(I);break;

case OR_B:     M_OR(R->BC.B.h);break;
case OR_C:     M_OR(R->BC.B.l);break;
case OR_D:     M_OR(R->DE.B.h);break;
case OR_E:     M_OR(R->DE.B.l);break;
case OR_H:     M_OR(R->HL.B.h);break;
case OR_L:     M_OR(R->HL.B.l);break;
case OR_A:     M_OR(R->AF.B.h);break;
case OR_xHL:   I=RdZ80(R->HL.W);M_OR(I);break;
case OR_BYTE:  I=RdZ80(R->PC.W++);M_OR(I);break;

case ADC_B:    M_ADC(R->BC.B.h);break;
case ADC_C:    M_ADC(R->BC.B.l);break;
case ADC_D:    M_ADC(R->DE.B.h);break;
case ADC_E:    M_ADC(R->DE.B.l);break;
case ADC_H:    M_ADC(R->HL.B.h);break;
case ADC_L:    M_ADC(R->HL.B.l);break;
case ADC_A:    M_ADC(R->AF.B.h);break;
case ADC_xHL:  I=RdZ80(R->HL.W);M_ADC(I);break;
case ADC_BYTE: I=RdZ80(R->PC.W++);M_ADC(I);break;

case SBC_B:    M_SBC(R->BC.B.h);break;
case SBC_C:    M_SBC(R->BC.B.l);break;
case SBC_D:    M_SBC(R->DE.B.h);break;
case SBC_E:    M_SBC(R->DE.B.l);break;
case SBC_H:    M_SBC(R->HL.B.h);break;
case SBC_L:    M_SBC(R->HL.B.l);break;
case SBC_A:    M_SBC(R->AF.B.h);break;
case SBC_xHL:  I=RdZ80(R->HL.W);M_SBC(I);break;
case SBC_BYTE: I=RdZ80(R->PC.W++);M_SBC(I);break;

case XOR_B:    M_XOR(R->BC.B.h);break;
case XOR_C:    M_XOR(R->BC.B.l);break;
case XOR_D:    M_XOR(R->DE.B.h);break;
case XOR_E:    M_XOR(R->DE.B.l);break;
case XOR_H:    M_XOR(R->HL.B.h);break;
case XOR_L:    M_XOR(R->HL.B.l);break;
case XOR_A:    R->AF.B.h=0;R->AF.B.l=P_FLAG|Z_FLAG;break;
case XOR_xHL:  I=RdZ80(R->HL.W);M_XOR(I);break;
case XOR_BYTE: I=RdZ80(R->PC.W++);M_XOR(I);break;

case CP_B:     M_CP(R->BC.B.h);break;
case CP_C:     M_CP(R->BC.B.l);break;
case CP_D:     M_CP(R->DE.B.h);break;
case CP_E:     M_CP(R->DE.B.l);break;
case CP_H:     M_CP(R->HL.B.h);break;
case CP_L:     M_CP(R->HL.B.l);break;
case CP_A:     R->AF.B.l=N_FLAG|Z_FLAG;break;
case CP_xHL:   I=RdZ80(R->HL.W);M_CP(I);break;
case CP_BYTE:  I=RdZ80(R->PC.W++);M_CP(I);break;
               
case LD_BC_WORD: M_LDWORD(BC);break;
case LD_DE_WORD: M_LDWORD(DE);break;
case LD_HL_WORD: M_LDWORD(HL);break;
case LD_SP_WORD: M_LDWORD(SP);break;

case LD_PC_HL: R->PC.W=R->HL.W;break;
case LD_SP_HL: R->SP.W=R->HL.W;break;
case LD_A_xBC: R->AF.B.h=RdZ80(R->BC.W);break;
case LD_A_xDE: R->AF.B.h=RdZ80(R->DE.W);break;

case ADD_HL_BC:  M_ADDW(HL,BC);break;
case ADD_HL_DE:  M_ADDW(HL,DE);break;
case ADD_HL_HL:  M_ADDW(HL,HL);break;
case ADD_HL_SP:  M_ADDW(HL,SP);break;

case DEC_BC:   R->BC.W--;break;
case DEC_DE:   R->DE.W--;break;
case DEC_HL:   R->HL.W--;break;
case DEC_SP:   R->SP.W--;break;

case INC_BC:   R->BC.W++;break;
case INC_DE:   R->DE.W++;break;
case INC_HL:   R->HL.W++;break;
case INC_SP:   R->SP.W++;break;

case DEC_B:    M_DEC(R->BC.B.h);break;
case DEC_C:    M_DEC(R->BC.B.l);break;
case DEC_D:    M_DEC(R->DE.B.h);break;
case DEC_E:    M_DEC(R->DE.B.l);break;
case DEC_H:    M_DEC(R->HL.B.h);break;
case DEC_L:    M_DEC(R->HL.B.l);break;
case DEC_A:    M_DEC(R->AF.B.h);break;
case DEC_xHL:  I=RdZ80(R->HL.W);M_DEC(I);WrZ80(R->HL.W,I);break;

case INC_B:    M_INC(R->BC.B.h);break;
case INC_C:    M_INC(R->BC.B.l);break;
case INC_D:    M_INC(R->DE.B.h);break;
case INC_E:    M_INC(R->DE.B.l);break;
case INC_H:    M_INC(R->HL.B.h);break;
case INC_L:    M_INC(R->HL.B.l);break;
case INC_A:    M_INC(R->AF.B.h);break;
case INC_xHL:  I=RdZ80(R->HL.W);M_INC(I);WrZ80(R->HL.W,I);break;

case RLCA:
  I=R->AF.B.h&0x80? C_FLAG:0;
  R->AF.B.h=(R->AF.B.h<<1)|I;
  R->AF.B.l=(R->AF.B.l&~(C_FLAG|N_FLAG|H_FLAG))|I;
  break;
case RLA:
  I=R->AF.B.h&0x80? C_FLAG:0;
  R->AF.B.h=(R->AF.B.h<<1)|(R->AF.B.l&C_FLAG);
  R->AF.B.l=(R->AF.B.l&~(C_FLAG|N_FLAG|H_FLAG))|I;
  break;
case RRCA:
  I=R->AF.B.h&0x01;
  R->AF.B.h=(R->AF.B.h>>1)|(I? 0x80:0);
  R->AF.B.l=(R->AF.B.l&~(C_FLAG|N_FLAG|H_FLAG))|I; 
  break;
case RRA:
  I=R->AF.B.h&0x01;
  R->AF.B.h=(R->AF.B.h>>1)|(R->AF.B.l&C_FLAG? 0x80:0);
  R->AF.B.l=(R->AF.B.l&~(C_FLAG|N_FLAG|H_FLAG))|I;
  break;

case RST00:    M_RST(0x0000);break;
case RST08:    M_RST(0x0008);break;
case RST10:    M_RST(0x0010);break;
case RST18:    M_RST(0x0018);break;
case RST20:    M_RST(0x0020);break;
case RST28:    M_RST(0x0028);break;
case RST30:    M_RST(0x0030);break;
case RST38:    M_RST(0x0038);break;

case PUSH_BC:  M_PUSH(BC);break;
case PUSH_DE:  M_PUSH(DE);break;
case PUSH_HL:  M_PUSH(HL);break;
case PUSH_AF:  M_PUSH(AF);break;

case POP_BC:   M_POP(BC);break;
case POP_DE:   M_POP(DE);break;
case POP_HL:   M_POP(HL);break;
case POP_AF:   M_POP(AF);break;

case DJNZ: if(--R->BC.B.h) { R->ICount-=5;M_JR; } else R->PC.W++;break;
case JP:   M_JP;break;
case JR:   M_JR;break;
case CALL: M_CALL;break;
case RET:  M_RET;break;
case SCF:  S(C_FLAG);R(N_FLAG|H_FLAG);break;
case CPL:  R->AF.B.h=~R->AF.B.h;S(N_FLAG|H_FLAG);break;
case NOP:  break;
case OUTA: I=RdZ80(R->PC.W++);OutZ80(I,R->AF.B.h);break;
case INA:  I=RdZ80(R->PC.W++);R->AF.B.h=InZ80(I);break;

case HALT:
  R->PC.W--;
  R->IFF|=IFF_HALT;
  R->IBackup=0;
  R->ICount=0;
  break;

case DI:
  if(R->IFF&IFF_EI) R->ICount+=R->IBackup-1;
  R->IFF&=~(IFF_1|IFF_2|IFF_EI);
  break;

case EI:
  if(!(R->IFF&(IFF_1|IFF_EI)))
  {
    R->IFF|=IFF_2|IFF_EI;
    R->IBackup=R->ICount;
    R->ICount=1;
  }
  break;

case CCF:
  R->AF.B.l^=C_FLAG;R(N_FLAG|H_FLAG);
  R->AF.B.l|=R->AF.B.l&C_FLAG? 0:H_FLAG;
  break;

case EXX:
  J.W=R->BC.W;R->BC.W=R->BC1.W;R->BC1.W=J.W;
  J.W=R->DE.W;R->DE.W=R->DE1.W;R->DE1.W=J.W;
  J.W=R->HL.W;R->HL.W=R->HL1.W;R->HL1.W=J.W;
  break;

case EX_DE_HL: J.W=R->DE.W;R->DE.W=R->HL.W;R->HL.W=J.W;break;
case EX_AF_AF: J.W=R->AF.W;R->AF.W=R->AF1.W;R->AF1.W=J.W;break;  
  
case LD_B_B:   R->BC.B.h=R->BC.B.h;break;
case LD_C_B:   R->BC.B.l=R->BC.B.h;break;
case LD_D_B:   R->DE.B.h=R->BC.B.h;break;
case LD_E_B:   R->DE.B.l=R->BC.B.h;break;
case LD_H_B:   R->HL.B.h=R->BC.B.h;break;
case LD_L_B:   R->HL.B.l=R->BC.B.h;break;
case LD_A_B:   R->AF.B.h=R->BC.B.h;break;
case LD_xHL_B: WrZ80(R->HL.W,R->BC.B.h);break;

case LD_B_C:   R->BC.B.h=R->BC.B.l;break;
case LD_C_C:   R->BC.B.l=R->BC.B.l;break;
case LD_D_C:   R->DE.B.h=R->BC.B.l;break;
case LD_E_C:   R->DE.B.l=R->BC.B.l;break;
case LD_H_C:   R->HL.B.h=R->BC.B.l;break;
case LD_L_C:   R->HL.B.l=R->BC.B.l;break;
case LD_A_C:   R->AF.B.h=R->BC.B.l;break;
case LD_xHL_C: WrZ80(R->HL.W,R->BC.B.l);break;

case LD_B_D:   R->BC.B.h=R->DE.B.h;break;
case LD_C_D:   R->BC.B.l=R->DE.B.h;break;
case LD_D_D:   R->DE.B.h=R->DE.B.h;break;
case LD_E_D:   R->DE.B.l=R->DE.B.h;break;
case LD_H_D:   R->HL.B.h=R->DE.B.h;break;
case LD_L_D:   R->HL.B.l=R->DE.B.h;break;
case LD_A_D:   R->AF.B.h=R->DE.B.h;break;
case LD_xHL_D: WrZ80(R->HL.W,R->DE.B.h);break;

case LD_B_E:   R->BC.B.h=R->DE.B.l;break;
case LD_C_E:   R->BC.B.l=R->DE.B.l;break;
case LD_D_E:   R->DE.B.h=R->DE.B.l;break;
case LD_E_E:   R->DE.B.l=R->DE.B.l;break;
case LD_H_E:   R->HL.B.h=R->DE.B.l;break;
case LD_L_E:   R->HL.B.l=R->DE.B.l;break;
case LD_A_E:   R->AF.B.h=R->DE.B.l;break;
case LD_xHL_E: WrZ80(R->HL.W,R->DE.B.l);break;

case LD_B_H:   R->BC.B.h=R->HL.B.h;break;
case LD_C_H:   R->BC.B.l=R->HL.B.h;break;
case LD_D_H:   R->DE.B.h=R->HL.B.h;break;
case LD_E_H:   R->DE.B.l=R->HL.B.h;break;
case LD_H_H:   R->HL.B.h=R->HL.B.h;break;
case LD_L_H:   R->HL.B.l=R->HL.B.h;break;
case LD_A_H:   R->AF.B.h=R->HL.B.h;break;
case LD_xHL_H: WrZ80(R->HL.W,R->HL.B.h);break;

case LD_B_L:   R->BC.B.h=R->HL.B.l;break;
case LD_C_L:   R->BC.B.l=R->HL.B.l;break;
case LD_D_L:   R->DE.B.h=R->HL.B.l;break;
case LD_E_L:   R->DE.B.l=R->HL.B.l;break;
case LD_H_L:   R->HL.B.h=R->HL.B.l;break;
case LD_L_L:   R->HL.B.l=R->HL.B.l;break;
case LD_A_L:   R->AF.B.h=R->HL.B.l;break;
case LD_xHL_L: WrZ80(R->HL.W,R->HL.B.l);break;

case LD_B_A:   R->BC.B.h=R->AF.B.h;break;
case LD_C_A:   R->BC.B.l=R->AF.B.h;break;
case LD_D_A:   R->DE.B.h=R->AF.B.h;break;
case LD_E_A:   R->DE.B.l=R->AF.B.h;break;
case LD_H_A:   R->HL.B.h=R->AF.B.h;break;
case LD_L_A:   R->HL.B.l=R->AF.B.h;break;
case LD_A_A:   R->AF.B.h=R->AF.B.h;break;
case LD_xHL_A: WrZ80(R->HL.W,R->AF.B.h);break;

case LD_xBC_A: WrZ80(R->BC.W,R->AF.B.h);break;
case LD_xDE_A: WrZ80(R->DE.W,R->AF.B.h);break;

case LD_B_xHL:    R->BC.B.h=RdZ80(R->HL.W);break;
case LD_C_xHL:    R->BC.B.l=RdZ80(R->HL.W);break;
case LD_D_xHL:    R->DE.B.h=RdZ80(R->HL.W);break;
case LD_E_xHL:    R->DE.B.l=RdZ80(R->HL.W);break;
case LD_H_xHL:    R->HL.B.h=RdZ80(R->HL.W);break;
case LD_L_xHL:    R->HL.B.l=RdZ80(R->HL.W);break;
case LD_A_xHL:    R->AF.B.h=RdZ80(R->HL.W);break;

case LD_B_BYTE:   R->BC.B.h=RdZ80(R->PC.W++);break;
case LD_C_BYTE:   R->BC.B.l=RdZ80(R->PC.W++);break;
case LD_D_BYTE:   R->DE.B.h=RdZ80(R->PC.W++);break;
case LD_E_BYTE:   R->DE.B.l=RdZ80(R->PC.W++);break;
case LD_H_BYTE:   R->HL.B.h=RdZ80(R->PC.W++);break;
case LD_L_BYTE:   R->HL.B.l=RdZ80(R->PC.W++);break;
case LD_A_BYTE:   R->AF.B.h=RdZ80(R->PC.W++);break;
case LD_xHL_BYTE: WrZ80(R->HL.W,RdZ80(R->PC.W++));break;

case LD_xWORD_HL:
  J.B.l=RdZ80(R->PC.W++);
  J.B.h=RdZ80(R->PC.W++);
  WrZ80(J.W++,R->HL.B.l);
  WrZ80(J.W,R->HL.B.h);
  break;

case LD_HL_xWORD:
  J.B.l=RdZ80(R->PC.W++);
  J.B.h=RdZ80(R->PC.W++);
  R->HL.B.l=RdZ80(J.W++);
  R->HL.B.h=RdZ80(J.W);
  break;

case LD_A_xWORD:
  J.B.l=RdZ80(R->PC.W++);
  J.B.h=RdZ80(R->PC.W++); 
  R->AF.B.h=RdZ80(J.W);
  break;

case LD_xWORD_A:
  J.B.l=RdZ80(R->PC.W++);
  J.B.h=RdZ80(R->PC.W++);
  WrZ80(J.W,R->AF.B.h);
  break;

case EX_HL_xSP:
  J.B.l=RdZ80(R->SP.W);WrZ80(R->SP.W++,R->HL.B.l);
  J.B.h=RdZ80(R->SP.W);WrZ80(R->SP.W--,R->HL.B.h);
  R->HL.W=J.W;
  break;

case DAA:
  J.W=R->AF.B.h;
  if(R->AF.B.l&C_FLAG) J.W|=256;
  if(R->AF.B.l&H_FLAG) J.W|=512;
  if(R->AF.B.l&N_FLAG) J.W|=1024;
  R->AF.W=DAATable[J.W];
  break;

default:
  if(R->TrapBadOps)
    printf
    (
      "[Z80 %lX] Unrecognized instruction: %02X at PC=%04X\n",
      (long)R->User,RdZ80(R->PC.W-1),R->PC.W-1
    );
  break;
