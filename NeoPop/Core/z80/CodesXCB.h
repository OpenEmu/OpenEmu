/** Z80: portable Z80 emulator *******************************/
/**                                                         **/
/**                         CodesXCB.h                      **/
/**                                                         **/
/** This file contains implementation for FD/DD-CB tables   **/
/** of Z80 commands. It is included from Z80.c.             **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994-2002                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

case RLC_xHL: I=RdZ80(J.W);M_RLC(I);WrZ80(J.W,I);break;
case RRC_xHL: I=RdZ80(J.W);M_RRC(I);WrZ80(J.W,I);break;
case RL_xHL:  I=RdZ80(J.W);M_RL(I);WrZ80(J.W,I);break;
case RR_xHL:  I=RdZ80(J.W);M_RR(I);WrZ80(J.W,I);break;
case SLA_xHL: I=RdZ80(J.W);M_SLA(I);WrZ80(J.W,I);break;
case SRA_xHL: I=RdZ80(J.W);M_SRA(I);WrZ80(J.W,I);break;
case SLL_xHL: I=RdZ80(J.W);M_SLL(I);WrZ80(J.W,I);break;
case SRL_xHL: I=RdZ80(J.W);M_SRL(I);WrZ80(J.W,I);break;

case BIT0_B: case BIT0_C: case BIT0_D: case BIT0_E:
case BIT0_H: case BIT0_L: case BIT0_A:
case BIT0_xHL: I=RdZ80(J.W);M_BIT(0,I);break;
case BIT1_B: case BIT1_C: case BIT1_D: case BIT1_E:
case BIT1_H: case BIT1_L: case BIT1_A:
case BIT1_xHL: I=RdZ80(J.W);M_BIT(1,I);break;
case BIT2_B: case BIT2_C: case BIT2_D: case BIT2_E:
case BIT2_H: case BIT2_L: case BIT2_A:
case BIT2_xHL: I=RdZ80(J.W);M_BIT(2,I);break;
case BIT3_B: case BIT3_C: case BIT3_D: case BIT3_E:
case BIT3_H: case BIT3_L: case BIT3_A:
case BIT3_xHL: I=RdZ80(J.W);M_BIT(3,I);break;
case BIT4_B: case BIT4_C: case BIT4_D: case BIT4_E:
case BIT4_H: case BIT4_L: case BIT4_A:
case BIT4_xHL: I=RdZ80(J.W);M_BIT(4,I);break;
case BIT5_B: case BIT5_C: case BIT5_D: case BIT5_E:
case BIT5_H: case BIT5_L: case BIT5_A:
case BIT5_xHL: I=RdZ80(J.W);M_BIT(5,I);break;
case BIT6_B: case BIT6_C: case BIT6_D: case BIT6_E:
case BIT6_H: case BIT6_L: case BIT6_A:
case BIT6_xHL: I=RdZ80(J.W);M_BIT(6,I);break;
case BIT7_B: case BIT7_C: case BIT7_D: case BIT7_E:
case BIT7_H: case BIT7_L: case BIT7_A:
case BIT7_xHL: I=RdZ80(J.W);M_BIT(7,I);break;

case RES0_xHL: I=RdZ80(J.W);M_RES(0,I);WrZ80(J.W,I);break;
case RES1_xHL: I=RdZ80(J.W);M_RES(1,I);WrZ80(J.W,I);break;   
case RES2_xHL: I=RdZ80(J.W);M_RES(2,I);WrZ80(J.W,I);break;   
case RES3_xHL: I=RdZ80(J.W);M_RES(3,I);WrZ80(J.W,I);break;   
case RES4_xHL: I=RdZ80(J.W);M_RES(4,I);WrZ80(J.W,I);break;   
case RES5_xHL: I=RdZ80(J.W);M_RES(5,I);WrZ80(J.W,I);break;   
case RES6_xHL: I=RdZ80(J.W);M_RES(6,I);WrZ80(J.W,I);break;   
case RES7_xHL: I=RdZ80(J.W);M_RES(7,I);WrZ80(J.W,I);break;   

case SET0_xHL: I=RdZ80(J.W);M_SET(0,I);WrZ80(J.W,I);break;   
case SET1_xHL: I=RdZ80(J.W);M_SET(1,I);WrZ80(J.W,I);break; 
case SET2_xHL: I=RdZ80(J.W);M_SET(2,I);WrZ80(J.W,I);break; 
case SET3_xHL: I=RdZ80(J.W);M_SET(3,I);WrZ80(J.W,I);break; 
case SET4_xHL: I=RdZ80(J.W);M_SET(4,I);WrZ80(J.W,I);break; 
case SET5_xHL: I=RdZ80(J.W);M_SET(5,I);WrZ80(J.W,I);break; 
case SET6_xHL: I=RdZ80(J.W);M_SET(6,I);WrZ80(J.W,I);break; 
case SET7_xHL: I=RdZ80(J.W);M_SET(7,I);WrZ80(J.W,I);break; 
