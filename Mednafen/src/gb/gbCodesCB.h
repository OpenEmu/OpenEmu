// -*- C++ -*-
// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2004 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

 default:
 case 0x00:
   // RLC B
   AF.B.B0 = (BC.B.B1 & 0x80)?C_FLAG:0;
   BC.B.B1 = (BC.B.B1<<1) | (BC.B.B1>>7);
   AF.B.B0 |= ZeroTable[BC.B.B1];
   break;
 case 0x01:
   // RLC C
   AF.B.B0 = (BC.B.B0 & 0x80)?C_FLAG:0;
   BC.B.B0 = (BC.B.B0<<1) | (BC.B.B0>>7);
   AF.B.B0 |= ZeroTable[BC.B.B0];
   break;
 case 0x02:
   // RLC D
   AF.B.B0 = (DE.B.B1 & 0x80)?C_FLAG:0;
   DE.B.B1 = (DE.B.B1<<1) | (DE.B.B1>>7);
   AF.B.B0 |= ZeroTable[DE.B.B1];
   break;
 case 0x03:
   // RLC E
   AF.B.B0 = (DE.B.B0 & 0x80)?C_FLAG:0;
   DE.B.B0 = (DE.B.B0<<1) | (DE.B.B0>>7);
   AF.B.B0 |= ZeroTable[DE.B.B0];
   break;
 case 0x04:
   // RLC H
   AF.B.B0 = (HL.B.B1 & 0x80)?C_FLAG:0;
   HL.B.B1 = (HL.B.B1<<1) | (HL.B.B1>>7);
   AF.B.B0 |= ZeroTable[HL.B.B1];
   break;
 case 0x05:
   // RLC L
   AF.B.B0 = (HL.B.B0 & 0x80)?C_FLAG:0;
   HL.B.B0 = (HL.B.B0<<1) | (HL.B.B0>>7);
   AF.B.B0 |= ZeroTable[HL.B.B0];
   break;
 case 0x06:
   // RLC (HL)
   tempValue=gbReadMemory(HL.W);
   AF.B.B0 = (tempValue & 0x80)?C_FLAG:0;
   tempValue = (tempValue<<1) | (tempValue>>7);
   AF.B.B0 |= ZeroTable[tempValue];
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0x07:
   // RLC A
   AF.B.B0 = (AF.B.B1 & 0x80)?C_FLAG:0;
   AF.B.B1 = (AF.B.B1<<1) | (AF.B.B1>>7);
   AF.B.B0 |= ZeroTable[AF.B.B1];
   break;
 case 0x08:
   // RRC B
   AF.B.B0=(BC.B.B1&0x01 ? C_FLAG : 0);
   BC.B.B1=(BC.B.B1>>1)|(BC.B.B1<<7);
   AF.B.B0|=ZeroTable[BC.B.B1];
   break;
 case 0x09:
   // RRC C
   AF.B.B0=(BC.B.B0&0x01 ? C_FLAG : 0);
   BC.B.B0=(BC.B.B0>>1)|(BC.B.B0<<7);
   AF.B.B0|=ZeroTable[BC.B.B0];
   break;
 case 0x0a:
   // RRC D
   AF.B.B0=(DE.B.B1&0x01 ? C_FLAG : 0);
   DE.B.B1=(DE.B.B1>>1)|(DE.B.B1<<7);
   AF.B.B0|=ZeroTable[DE.B.B1];
   break;
 case 0x0b:
   // RRC E
   AF.B.B0=(DE.B.B0&0x01 ? C_FLAG : 0);
   DE.B.B0=(DE.B.B0>>1)|(DE.B.B0<<7);
   AF.B.B0|=ZeroTable[DE.B.B0];
   break;
 case 0x0c:
   // RRC H
   AF.B.B0=(HL.B.B1&0x01 ? C_FLAG : 0);
   HL.B.B1=(HL.B.B1>>1)|(HL.B.B1<<7);
   AF.B.B0|=ZeroTable[HL.B.B1];
   break;
 case 0x0d:
   // RRC L
   AF.B.B0=(HL.B.B0&0x01 ? C_FLAG : 0);
   HL.B.B0=(HL.B.B0>>1)|(HL.B.B0<<7);
   AF.B.B0|=ZeroTable[HL.B.B0];
   break;
 case 0x0e:
   // RRC (HL)
   tempValue=gbReadMemory(HL.W);
   AF.B.B0=(tempValue&0x01 ? C_FLAG : 0);
   tempValue=(tempValue>>1)|(tempValue<<7);
   AF.B.B0|=ZeroTable[tempValue];
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0x0f:
   // RRC A
   AF.B.B0=(AF.B.B1&0x01 ? C_FLAG : 0);
   AF.B.B1=(AF.B.B1>>1)|(AF.B.B1<<7);
   AF.B.B0|=ZeroTable[AF.B.B1];
   break;
 case 0x10:
   // RL B
   if(BC.B.B1&0x80) {
     BC.B.B1=(BC.B.B1<<1)|(AF.B.B0&C_FLAG ? 1 : 0);
     AF.B.B0=ZeroTable[BC.B.B1]|C_FLAG;
   } else {
     BC.B.B1=(BC.B.B1<<1)|(AF.B.B0&C_FLAG ? 1 : 0);
     AF.B.B0=ZeroTable[BC.B.B1];
   }
   break;
 case 0x11:
   // RL C
   if(BC.B.B0&0x80) {
     BC.B.B0=(BC.B.B0<<1)|(AF.B.B0&C_FLAG ? 1 : 0);
     AF.B.B0=ZeroTable[BC.B.B0]|C_FLAG;
   } else {
     BC.B.B0=(BC.B.B0<<1)|(AF.B.B0&C_FLAG ? 1 : 0);
     AF.B.B0=ZeroTable[BC.B.B0];
   }
   break;
 case 0x12:
   // RL D
   if(DE.B.B1&0x80) {
     DE.B.B1=(DE.B.B1<<1)|(AF.B.B0&C_FLAG ? 1 : 0);
     AF.B.B0=ZeroTable[DE.B.B1]|C_FLAG;
   } else {
     DE.B.B1=(DE.B.B1<<1)|(AF.B.B0&C_FLAG ? 1 : 0);
     AF.B.B0=ZeroTable[DE.B.B1];
   }
   break;
 case 0x13:
   // RL E
   if(DE.B.B0&0x80) {
     DE.B.B0=(DE.B.B0<<1)|(AF.B.B0&C_FLAG ? 1 : 0);
     AF.B.B0=ZeroTable[DE.B.B0]|C_FLAG;
   } else {
     DE.B.B0=(DE.B.B0<<1)|(AF.B.B0&C_FLAG ? 1 : 0);
     AF.B.B0=ZeroTable[DE.B.B0];
   }
   break;
 case 0x14:
   // RL H
   if(HL.B.B1&0x80) {
     HL.B.B1=(HL.B.B1<<1)|(AF.B.B0&C_FLAG ? 1 : 0);
     AF.B.B0=ZeroTable[HL.B.B1]|C_FLAG;
   } else {
     HL.B.B1=(HL.B.B1<<1)|(AF.B.B0&C_FLAG ? 1 : 0);
     AF.B.B0=ZeroTable[HL.B.B1];
   }
   break;
 case 0x15:
   // RL L
   if(HL.B.B0&0x80) {
     HL.B.B0=(HL.B.B0<<1)|(AF.B.B0&C_FLAG ? 1 : 0);
     AF.B.B0=ZeroTable[HL.B.B0]|C_FLAG;
   } else {
     HL.B.B0=(HL.B.B0<<1)|(AF.B.B0&C_FLAG ? 1 : 0);
     AF.B.B0=ZeroTable[HL.B.B0];
   }
   break;
 case 0x16:
   // RL (HL)
   tempValue=gbReadMemory(HL.W);
   if(tempValue&0x80) {
     tempValue=(tempValue<<1)|(AF.B.B0&C_FLAG ? 1 : 0);
     AF.B.B0=ZeroTable[tempValue]|C_FLAG;
   } else {
     tempValue=(tempValue<<1)|(AF.B.B0&C_FLAG ? 1 : 0);
     AF.B.B0=ZeroTable[tempValue];
   }
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0x17:
   // RL A
   if(AF.B.B1&0x80) {
     AF.B.B1=(AF.B.B1<<1)|(AF.B.B0&C_FLAG ? 1 : 0);
     AF.B.B0=ZeroTable[AF.B.B1]|C_FLAG;
   } else {
     AF.B.B1=(AF.B.B1<<1)|(AF.B.B0&C_FLAG ? 1 : 0);
     AF.B.B0=ZeroTable[AF.B.B1];
   }
   break;
 case 0x18:
   // RR B
   if(BC.B.B1&0x01) {
     BC.B.B1=(BC.B.B1>>1)|(AF.B.B0 & C_FLAG ? 0x80:0);
     AF.B.B0=ZeroTable[BC.B.B1]|C_FLAG;
   } else {
     BC.B.B1=(BC.B.B1>>1)|(AF.B.B0 & C_FLAG ? 0x80:0);
     AF.B.B0=ZeroTable[BC.B.B1];
   }
   break;
 case 0x19:
   // RR C
   if(BC.B.B0&0x01) {
     BC.B.B0=(BC.B.B0>>1)|(AF.B.B0 & C_FLAG ? 0x80:0);
     AF.B.B0=ZeroTable[BC.B.B0]|C_FLAG;
   } else {
     BC.B.B0=(BC.B.B0>>1)|(AF.B.B0 & C_FLAG ? 0x80:0);
     AF.B.B0=ZeroTable[BC.B.B0];
   }
   break;
 case 0x1a:
   // RR D
   if(DE.B.B1&0x01) {
     DE.B.B1=(DE.B.B1>>1)|(AF.B.B0 & C_FLAG ? 0x80:0);
     AF.B.B0=ZeroTable[DE.B.B1]|C_FLAG;
   } else {
     DE.B.B1=(DE.B.B1>>1)|(AF.B.B0 & C_FLAG ? 0x80:0);
     AF.B.B0=ZeroTable[DE.B.B1];
   }
   break;
 case 0x1b:
   // RR E
   if(DE.B.B0&0x01) {
     DE.B.B0=(DE.B.B0>>1)|(AF.B.B0 & C_FLAG ? 0x80:0);
     AF.B.B0=ZeroTable[DE.B.B0]|C_FLAG;
   } else {
     DE.B.B0=(DE.B.B0>>1)|(AF.B.B0 & C_FLAG ? 0x80:0);
     AF.B.B0=ZeroTable[DE.B.B0];
   }
   break;
 case 0x1c:
   // RR H
   if(HL.B.B1&0x01) {
     HL.B.B1=(HL.B.B1>>1)|(AF.B.B0 & C_FLAG ? 0x80:0);
     AF.B.B0=ZeroTable[HL.B.B1]|C_FLAG;
   } else {
     HL.B.B1=(HL.B.B1>>1)|(AF.B.B0 & C_FLAG ? 0x80:0);
     AF.B.B0=ZeroTable[HL.B.B1];
   }
   break;
 case 0x1d:
   // RR L
   if(HL.B.B0&0x01) {
     HL.B.B0=(HL.B.B0>>1)|(AF.B.B0 & C_FLAG ? 0x80:0);
     AF.B.B0=ZeroTable[HL.B.B0]|C_FLAG;
   } else {
     HL.B.B0=(HL.B.B0>>1)|(AF.B.B0 & C_FLAG ? 0x80:0);
     AF.B.B0=ZeroTable[HL.B.B0];
   }
   break;
 case 0x1e:
   // RR (HL)
   tempValue=gbReadMemory(HL.W);
   if(tempValue&0x01) {
     tempValue=(tempValue>>1)|(AF.B.B0 & C_FLAG ? 0x80:0);
     AF.B.B0=ZeroTable[tempValue]|C_FLAG;
   } else {
     tempValue=(tempValue>>1)|(AF.B.B0 & C_FLAG ? 0x80:0);
     AF.B.B0=ZeroTable[tempValue];
   }
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0x1f:
   // RR A
   if(AF.B.B1&0x01) {
     AF.B.B1=(AF.B.B1>>1)|(AF.B.B0 & C_FLAG ? 0x80:0);
     AF.B.B0=ZeroTable[AF.B.B1]|C_FLAG;
   } else {
     AF.B.B1=(AF.B.B1>>1)|(AF.B.B0 & C_FLAG ? 0x80:0);
     AF.B.B0=ZeroTable[AF.B.B1];
   }
   break;
 case 0x20:
   // SLA B
   AF.B.B0=(BC.B.B1&0x80?C_FLAG : 0);
   BC.B.B1<<=1;
   AF.B.B0|=ZeroTable[BC.B.B1];
   break;
 case 0x21:
   // SLA C
   AF.B.B0=(BC.B.B0&0x80?C_FLAG : 0);
   BC.B.B0<<=1;
   AF.B.B0|=ZeroTable[BC.B.B0];
   break;
 case 0x22:
   // SLA D
   AF.B.B0=(DE.B.B1&0x80?C_FLAG : 0);
   DE.B.B1<<=1;
   AF.B.B0|=ZeroTable[DE.B.B1];
   break;
 case 0x23:
   // SLA E
   AF.B.B0=(DE.B.B0&0x80?C_FLAG : 0);
   DE.B.B0<<=1;
   AF.B.B0|=ZeroTable[DE.B.B0];
   break;
 case 0x24:
   // SLA H
   AF.B.B0=(HL.B.B1&0x80?C_FLAG : 0);
   HL.B.B1<<=1;
   AF.B.B0|=ZeroTable[HL.B.B1];
   break;
 case 0x25:
   // SLA L
   AF.B.B0=(HL.B.B0&0x80?C_FLAG : 0);
   HL.B.B0<<=1;
   AF.B.B0|=ZeroTable[HL.B.B0];
   break;
 case 0x26:
   // SLA (HL)
   tempValue=gbReadMemory(HL.W);
   AF.B.B0=(tempValue&0x80?C_FLAG : 0);
   tempValue<<=1;
   AF.B.B0|=ZeroTable[tempValue];
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0x27:
   // SLA A
   AF.B.B0=(AF.B.B1&0x80?C_FLAG : 0);
   AF.B.B1<<=1;
   AF.B.B0|=ZeroTable[AF.B.B1];
   break;
 case 0x28:
   // SRA B
   AF.B.B0=(BC.B.B1&0x01 ? C_FLAG: 0);
   BC.B.B1=(BC.B.B1>>1)|(BC.B.B1&0x80);
   AF.B.B0|=ZeroTable[BC.B.B1];
   break;
 case 0x29:
   // SRA C
   AF.B.B0=(BC.B.B0&0x01 ? C_FLAG: 0);
   BC.B.B0=(BC.B.B0>>1)|(BC.B.B0&0x80);
   AF.B.B0|=ZeroTable[BC.B.B0];
   break;
 case 0x2a:
   // SRA D
   AF.B.B0=(DE.B.B1&0x01 ? C_FLAG: 0);
   DE.B.B1=(DE.B.B1>>1)|(DE.B.B1&0x80);
   AF.B.B0|=ZeroTable[DE.B.B1];
   break;
 case 0x2b:
   // SRA E
   AF.B.B0=(DE.B.B0&0x01 ? C_FLAG: 0);
   DE.B.B0=(DE.B.B0>>1)|(DE.B.B0&0x80);
   AF.B.B0|=ZeroTable[DE.B.B0];
   break;
 case 0x2c:
   // SRA H
   AF.B.B0=(HL.B.B1&0x01 ? C_FLAG: 0);
   HL.B.B1=(HL.B.B1>>1)|(HL.B.B1&0x80);
   AF.B.B0|=ZeroTable[HL.B.B1];
   break;
 case 0x2d:
   // SRA L
   AF.B.B0=(HL.B.B0&0x01 ? C_FLAG: 0);
   HL.B.B0=(HL.B.B0>>1)|(HL.B.B0&0x80);
   AF.B.B0|=ZeroTable[HL.B.B0];
   break;
 case 0x2e:
   // SRA (HL)
   tempValue=gbReadMemory(HL.W);
   AF.B.B0=(tempValue&0x01 ? C_FLAG: 0);
   tempValue=(tempValue>>1)|(tempValue&0x80);
   AF.B.B0|=ZeroTable[tempValue];
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0x2f:
   // SRA A
   AF.B.B0=(AF.B.B1&0x01 ? C_FLAG: 0);
   AF.B.B1=(AF.B.B1>>1)|(AF.B.B1&0x80);
   AF.B.B0|=ZeroTable[AF.B.B1];
   break;
 case 0x30:
   // SWAP B
   BC.B.B1 = (BC.B.B1&0xf0)>>4 | (BC.B.B1&0x0f)<<4;
   AF.B.B0 = ZeroTable[BC.B.B1];
   break;
 case 0x31:
   // SWAP C
   BC.B.B0 = (BC.B.B0&0xf0)>>4 | (BC.B.B0&0x0f)<<4;
   AF.B.B0 = ZeroTable[BC.B.B0];
   break;
 case 0x32:
  // SWAP D
  DE.B.B1 = (DE.B.B1&0xf0)>>4 | (DE.B.B1&0x0f)<<4;
  AF.B.B0 = ZeroTable[DE.B.B1];
  break;
 case 0x33:
   // SWAP E
   DE.B.B0 = (DE.B.B0&0xf0)>>4 | (DE.B.B0&0x0f)<<4;
   AF.B.B0 = ZeroTable[DE.B.B0];
   break;
 case 0x34:
   // SWAP H
   HL.B.B1 = (HL.B.B1&0xf0)>>4 | (HL.B.B1&0x0f)<<4;
   AF.B.B0 = ZeroTable[HL.B.B1];
   break;
 case 0x35:
   // SWAP L
   HL.B.B0 = (HL.B.B0&0xf0)>>4 | (HL.B.B0&0x0f)<<4;
   AF.B.B0 = ZeroTable[HL.B.B0];
   break;
 case 0x36:
   // SWAP (HL)
   tempValue=gbReadMemory(HL.W);
   tempValue = (tempValue&0xf0)>>4 | (tempValue&0x0f)<<4;
   AF.B.B0 = ZeroTable[tempValue];
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0x37:
   // SWAP A
   AF.B.B1 = (AF.B.B1&0xf0)>>4 | (AF.B.B1&0x0f)<<4;
   AF.B.B0 = ZeroTable[AF.B.B1];
   break;
 case 0x38:
   // SRL B
   AF.B.B0=(BC.B.B1&0x01)?C_FLAG:0;
   BC.B.B1>>=1;
   AF.B.B0|=ZeroTable[BC.B.B1];
   break;
 case 0x39:
   // SRL C
   AF.B.B0=(BC.B.B0&0x01)?C_FLAG:0;
   BC.B.B0>>=1;
   AF.B.B0|=ZeroTable[BC.B.B0];
   break;
 case 0x3a:
   // SRL D
   AF.B.B0=(DE.B.B1&0x01)?C_FLAG:0;
   DE.B.B1>>=1;
   AF.B.B0|=ZeroTable[DE.B.B1];
   break;
 case 0x3b:
   // SRL E
   AF.B.B0=(DE.B.B0&0x01)?C_FLAG:0;
   DE.B.B0>>=1;
   AF.B.B0|=ZeroTable[DE.B.B0];
   break;
 case 0x3c:
   // SRL H
   AF.B.B0=(HL.B.B1&0x01)?C_FLAG:0;
   HL.B.B1>>=1;
   AF.B.B0|=ZeroTable[HL.B.B1];
   break;
 case 0x3d:
   // SRL L
   AF.B.B0=(HL.B.B0&0x01)?C_FLAG:0;
   HL.B.B0>>=1;
   AF.B.B0|=ZeroTable[HL.B.B0];
   break;
 case 0x3e:
   // SRL (HL)
   tempValue=gbReadMemory(HL.W);
   AF.B.B0=(tempValue&0x01)?C_FLAG:0;
   tempValue>>=1;
   AF.B.B0|=ZeroTable[tempValue];
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0x3f:
   // SRL A
   AF.B.B0=(AF.B.B1&0x01)?C_FLAG:0;
   AF.B.B1>>=1;
   AF.B.B0|=ZeroTable[AF.B.B1];
   break;
 case 0x40:
   // BIT 0,B
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(BC.B.B1&(1<<0)? 0:Z_FLAG);
   break;
 case 0x41:
   // BIT 0,C
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(BC.B.B0&(1<<0)? 0:Z_FLAG);
   break;
 case 0x42:
   // BIT 0,D
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(DE.B.B1&(1<<0)? 0:Z_FLAG);
   break;
 case 0x43:
   // BIT 0,E
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(DE.B.B0&(1<<0)? 0:Z_FLAG);
   break;
 case 0x44:
   // BIT 0,H
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(HL.B.B1&(1<<0)? 0:Z_FLAG);
   break;
 case 0x45:
   // BIT 0,L
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(HL.B.B0&(1<<0)? 0:Z_FLAG);
   break;
 case 0x46:
   // BIT 0,(HL)
   tempValue=gbReadMemory(HL.W);
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(tempValue&(1<<0)? 0:Z_FLAG);
   break;
 case 0x47:
   // BIT 0,A
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(AF.B.B1&(1<<0)? 0:Z_FLAG);
   break;
 case 0x48:
   // BIT 1,B
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(BC.B.B1&(1<<1)? 0:Z_FLAG);
   break;
 case 0x49:
   // BIT 1,C
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(BC.B.B0&(1<<1)? 0:Z_FLAG);
   break;
 case 0x4a:
   // BIT 1,D
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(DE.B.B1&(1<<1)? 0:Z_FLAG);
   break;
 case 0x4b:
   // BIT 1,E
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(DE.B.B0&(1<<1)? 0:Z_FLAG);
   break;
 case 0x4c:
   // BIT 1,H
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(HL.B.B1&(1<<1)? 0:Z_FLAG);
   break;
 case 0x4d:
   // BIT 1,L
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(HL.B.B0&(1<<1)? 0:Z_FLAG);
   break;
 case 0x4e:
   // BIT 1,(HL)
   tempValue=gbReadMemory(HL.W);
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(tempValue&(1<<1)? 0:Z_FLAG);
   break;
 case 0x4f:
   // BIT 1,A
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(AF.B.B1&(1<<1)? 0:Z_FLAG);
   break;
 case 0x50:
   // BIT 2,B
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(BC.B.B1&(1<<2)? 0:Z_FLAG);
   break;
 case 0x51:
   // BIT 2,C
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(BC.B.B0&(1<<2)? 0:Z_FLAG);
   break;
 case 0x52:
   // BIT 2,D
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(DE.B.B1&(1<<2)? 0:Z_FLAG);
   break;
 case 0x53:
   // BIT 2,E
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(DE.B.B0&(1<<2)? 0:Z_FLAG);
   break;
 case 0x54:
   // BIT 2,H
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(HL.B.B1&(1<<2)? 0:Z_FLAG);
   break;
 case 0x55:
   // BIT 2,L
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(HL.B.B0&(1<<2)? 0:Z_FLAG);
   break;
 case 0x56:
   // BIT 2,(HL)
   tempValue=gbReadMemory(HL.W);
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(tempValue&(1<<2)? 0:Z_FLAG);
   break;
 case 0x57:
   // BIT 2,A
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(AF.B.B1&(1<<2)? 0:Z_FLAG);
   break;
 case 0x58:
   // BIT 3,B
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(BC.B.B1&(1<<3)? 0:Z_FLAG);
   break;
 case 0x59:
   // BIT 3,C
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(BC.B.B0&(1<<3)? 0:Z_FLAG);
   break;
 case 0x5a:
   // BIT 3,D
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(DE.B.B1&(1<<3)? 0:Z_FLAG);
   break;
 case 0x5b:
   // BIT 3,E
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(DE.B.B0&(1<<3)? 0:Z_FLAG);
   break;
 case 0x5c:
   // BIT 3,H
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(HL.B.B1&(1<<3)? 0:Z_FLAG);
   break;
 case 0x5d:
   // BIT 3,L
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(HL.B.B0&(1<<3)? 0:Z_FLAG);
   break;
 case 0x5e:
   // BIT 3,(HL)
   tempValue=gbReadMemory(HL.W);
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(tempValue&(1<<3)? 0:Z_FLAG);
   break;
 case 0x5f:
   // BIT 3,A
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(AF.B.B1&(1<<3)? 0:Z_FLAG);
   break;
 case 0x60:
   // BIT 4,B
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(BC.B.B1&(1<<4)? 0:Z_FLAG);
   break;
 case 0x61:
   // BIT 4,C
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(BC.B.B0&(1<<4)? 0:Z_FLAG);
   break;
 case 0x62:
   // BIT 4,D
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(DE.B.B1&(1<<4)? 0:Z_FLAG);
   break;
 case 0x63:
   // BIT 4,E
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(DE.B.B0&(1<<4)? 0:Z_FLAG);
   break;
 case 0x64:
   // BIT 4,H
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(HL.B.B1&(1<<4)? 0:Z_FLAG);
   break;
 case 0x65:
   // BIT 4,L
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(HL.B.B0&(1<<4)? 0:Z_FLAG);
   break;
 case 0x66:
   // BIT 4,(HL)
   tempValue=gbReadMemory(HL.W);
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(tempValue&(1<<4)? 0:Z_FLAG);
   break;
 case 0x67:
   // BIT 4,A
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(AF.B.B1&(1<<4)? 0:Z_FLAG);
   break;
 case 0x68:
   // BIT 5,B
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(BC.B.B1&(1<<5)? 0:Z_FLAG);
   break;
 case 0x69:
   // BIT 5,C
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(BC.B.B0&(1<<5)? 0:Z_FLAG);
   break;
 case 0x6a:
   // BIT 5,D
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(DE.B.B1&(1<<5)? 0:Z_FLAG);
   break;
 case 0x6b:
   // BIT 5,E
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(DE.B.B0&(1<<5)? 0:Z_FLAG);
   break;
 case 0x6c:
   // BIT 5,H
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(HL.B.B1&(1<<5)? 0:Z_FLAG);
   break;
 case 0x6d:
   // BIT 5,L
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(HL.B.B0&(1<<5)? 0:Z_FLAG);
   break;
 case 0x6e:
   // BIT 5,(HL)
   tempValue=gbReadMemory(HL.W);
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(tempValue&(1<<5)? 0:Z_FLAG);
   break;
 case 0x6f:
   // BIT 5,A
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(AF.B.B1&(1<<5)? 0:Z_FLAG);
   break;
 case 0x70:
   // BIT 6,B
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(BC.B.B1&(1<<6)? 0:Z_FLAG);
   break;
 case 0x71:
   // BIT 6,C
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(BC.B.B0&(1<<6)? 0:Z_FLAG);
   break;
 case 0x72:
   // BIT 6,D
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(DE.B.B1&(1<<6)? 0:Z_FLAG);
   break;
 case 0x73:
   // BIT 6,E
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(DE.B.B0&(1<<6)? 0:Z_FLAG);
   break;
 case 0x74:
   // BIT 6,H
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(HL.B.B1&(1<<6)? 0:Z_FLAG);
   break;
 case 0x75:
   // BIT 6,L
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(HL.B.B0&(1<<6)? 0:Z_FLAG);
   break;
 case 0x76:
   // BIT 6,(HL)
   tempValue=gbReadMemory(HL.W);
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(tempValue&(1<<6)? 0:Z_FLAG);
   break;
 case 0x77:
   // BIT 6,A
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(AF.B.B1&(1<<6)? 0:Z_FLAG);
   break;
 case 0x78:
   // BIT 7,B
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(BC.B.B1&(1<<7)? 0:Z_FLAG);
   break;
 case 0x79:
   // BIT 7,C
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(BC.B.B0&(1<<7)? 0:Z_FLAG);
   break;
 case 0x7a:
   // BIT 7,D
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(DE.B.B1&(1<<7)? 0:Z_FLAG);
   break;
 case 0x7b:
   // BIT 7,E
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(DE.B.B0&(1<<7)? 0:Z_FLAG);
   break;
 case 0x7c:
   // BIT 7,H
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(HL.B.B1&(1<<7)? 0:Z_FLAG);
   break;
 case 0x7d:
   // BIT 7,L
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(HL.B.B0&(1<<7)? 0:Z_FLAG);
   break;
 case 0x7e:
   // BIT 7,(HL)
   tempValue=gbReadMemory(HL.W);
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(tempValue&(1<<7)? 0:Z_FLAG);
   break;
 case 0x7f:
   // BIT 7,A
   AF.B.B0=(AF.B.B0&C_FLAG)|H_FLAG|(AF.B.B1&(1<<7)? 0:Z_FLAG);
   break;
 case 0x80:
   // RES 0,B
   BC.B.B1&=~(1<<0);
   break;
 case 0x81:
   // RES 0,C
   BC.B.B0&=~(1<<0);
   break;
 case 0x82:
   // RES 0,D
   DE.B.B1&=~(1<<0);
   break;
 case 0x83:
   // RES 0,E
   DE.B.B0&=~(1<<0);
   break;
 case 0x84:
   // RES 0,H
   HL.B.B1&=~(1<<0);
   break;
 case 0x85:
   // RES 0,L
   HL.B.B0&=~(1<<0);
   break;
 case 0x86:
   // RES 0,(HL)
   tempValue=gbReadMemory(HL.W);
   tempValue&=~(1<<0);
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0x87:
   // RES 0,A
   AF.B.B1&=~(1<<0);
   break;
 case 0x88:
   // RES 1,B
   BC.B.B1&=~(1<<1);
   break;
 case 0x89:
   // RES 1,C
   BC.B.B0&=~(1<<1);
   break;
 case 0x8a:
   // RES 1,D
   DE.B.B1&=~(1<<1);
   break;
 case 0x8b:
   // RES 1,E
   DE.B.B0&=~(1<<1);
   break;
 case 0x8c:
   // RES 1,H
   HL.B.B1&=~(1<<1);
   break;
 case 0x8d:
   // RES 1,L
   HL.B.B0&=~(1<<1);
   break;
 case 0x8e:
   // RES 1,(HL)
   tempValue=gbReadMemory(HL.W);
   tempValue&=~(1<<1);
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0x8f:
   // RES 1,A
   AF.B.B1&=~(1<<1);
   break;
 case 0x90:
   // RES 2,B
   BC.B.B1&=~(1<<2);
   break;
 case 0x91:
   // RES 2,C
   BC.B.B0&=~(1<<2);
   break;
 case 0x92:
   // RES 2,D
   DE.B.B1&=~(1<<2);
   break;
 case 0x93:
   // RES 2,E
   DE.B.B0&=~(1<<2);
   break;
 case 0x94:
   // RES 2,H
   HL.B.B1&=~(1<<2);
   break;
 case 0x95:
   // RES 2,L
   HL.B.B0&=~(1<<2);
   break;
 case 0x96:
   // RES 2,(HL)
   tempValue=gbReadMemory(HL.W);
   tempValue&=~(1<<2);
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0x97:
   // RES 2,A
   AF.B.B1&=~(1<<2);
   break;
 case 0x98:
   // RES 3,B
   BC.B.B1&=~(1<<3);
   break;
 case 0x99:
   // RES 3,C
   BC.B.B0&=~(1<<3);
   break;
 case 0x9a:
   // RES 3,D
   DE.B.B1&=~(1<<3);
   break;
 case 0x9b:
   // RES 3,E
   DE.B.B0&=~(1<<3);
   break;
 case 0x9c:
   // RES 3,H
   HL.B.B1&=~(1<<3);
   break;
 case 0x9d:
   // RES 3,L
   HL.B.B0&=~(1<<3);
   break;
 case 0x9e:
   // RES 3,(HL)
   tempValue=gbReadMemory(HL.W);
   tempValue&=~(1<<3);
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0x9f:
   // RES 3,A
   AF.B.B1&=~(1<<3);
   break;
 case 0xa0:
   // RES 4,B
   BC.B.B1&=~(1<<4);
   break;
 case 0xa1:
   // RES 4,C
   BC.B.B0&=~(1<<4);
   break;
 case 0xa2:
   // RES 4,D
   DE.B.B1&=~(1<<4);
   break;
 case 0xa3:
   // RES 4,E
   DE.B.B0&=~(1<<4);
   break;
 case 0xa4:
   // RES 4,H
   HL.B.B1&=~(1<<4);
   break;
 case 0xa5:
   // RES 4,L
   HL.B.B0&=~(1<<4);
   break;
 case 0xa6:
   // RES 4,(HL)
   tempValue=gbReadMemory(HL.W);
   tempValue&=~(1<<4);
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0xa7:
   // RES 4,A
   AF.B.B1&=~(1<<4);
   break;
 case 0xa8:
   // RES 5,B
   BC.B.B1&=~(1<<5);
   break;
 case 0xa9:
   // RES 5,C
   BC.B.B0&=~(1<<5);
   break;
 case 0xaa:
   // RES 5,D
   DE.B.B1&=~(1<<5);
   break;
 case 0xab:
   // RES 5,E
   DE.B.B0&=~(1<<5);
   break;
 case 0xac:
   // RES 5,H
   HL.B.B1&=~(1<<5);
   break;
 case 0xad:
   // RES 5,L
   HL.B.B0&=~(1<<5);
   break;
 case 0xae:
   // RES 5,(HL)
   tempValue=gbReadMemory(HL.W);
   tempValue&=~(1<<5);
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0xaf:
   // RES 5,A
   AF.B.B1&=~(1<<5);
   break;
 case 0xb0:
   // RES 6,B
   BC.B.B1&=~(1<<6);
   break;
 case 0xb1:
   // RES 6,C
   BC.B.B0&=~(1<<6);
   break;
 case 0xb2:
   // RES 6,D
   DE.B.B1&=~(1<<6);
   break;
 case 0xb3:
   // RES 6,E
   DE.B.B0&=~(1<<6);
   break;
 case 0xb4:
   // RES 6,H
   HL.B.B1&=~(1<<6);
   break;
 case 0xb5:
   // RES 6,L
   HL.B.B0&=~(1<<6);
   break;
 case 0xb6:
   // RES 6,(HL)
   tempValue=gbReadMemory(HL.W);
   tempValue&=~(1<<6);
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0xb7:
   // RES 6,A
   AF.B.B1&=~(1<<6);
   break;
 case 0xb8:
   // RES 7,B
   BC.B.B1&=~(1<<7);
   break;
 case 0xb9:
   // RES 7,C
   BC.B.B0&=~(1<<7);
   break;
 case 0xba:
   // RES 7,D
   DE.B.B1&=~(1<<7);
   break;
 case 0xbb:
   // RES 7,E
   DE.B.B0&=~(1<<7);
   break;
 case 0xbc:
   // RES 7,H
   HL.B.B1&=~(1<<7);
   break;
 case 0xbd:
   // RES 7,L
   HL.B.B0&=~(1<<7);
   break;
 case 0xbe:
   // RES 7,(HL)
   tempValue=gbReadMemory(HL.W);
   tempValue&=~(1<<7);
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0xbf:
   // RES 7,A
   AF.B.B1&=~(1<<7);
   break;
 case 0xc0:
   // SET 0,B
   BC.B.B1|=1<<0;
   break;
 case 0xc1:
   // SET 0,C
   BC.B.B0|=1<<0;
   break;
 case 0xc2:
   // SET 0,D
   DE.B.B1|=1<<0;
   break;
 case 0xc3:
   // SET 0,E
   DE.B.B0|=1<<0;
   break;
 case 0xc4:
   // SET 0,H
   HL.B.B1|=1<<0;
   break;
 case 0xc5:
   // SET 0,L
   HL.B.B0|=1<<0;
   break;
 case 0xc6:
   // SET 0,(HL)
   tempValue=gbReadMemory(HL.W);
   tempValue|=1<<0;
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0xc7:
   // SET 0,A
   AF.B.B1|=1<<0;
   break;
 case 0xc8:
   // SET 1,B
   BC.B.B1|=1<<1;
   break;
 case 0xc9:
   // SET 1,C
   BC.B.B0|=1<<1;
   break;
 case 0xca:
   // SET 1,D
   DE.B.B1|=1<<1;
   break;
 case 0xcb:
   // SET 1,E
   DE.B.B0|=1<<1;
   break;
 case 0xcc:
   // SET 1,H
   HL.B.B1|=1<<1;
   break;
 case 0xcd:
   // SET 1,L
   HL.B.B0|=1<<1;
   break;
 case 0xce:
   // SET 1,(HL)
   tempValue=gbReadMemory(HL.W);
   tempValue|=1<<1;
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0xcf:
   // SET 1,A
   AF.B.B1|=1<<1;
   break;
 case 0xd0:
   // SET 2,B
   BC.B.B1|=1<<2;
   break;
 case 0xd1:
   // SET 2,C
   BC.B.B0|=1<<2;
   break;
 case 0xd2:
   // SET 2,D
   DE.B.B1|=1<<2;
   break;
 case 0xd3:
   // SET 2,E
   DE.B.B0|=1<<2;
   break;
 case 0xd4:
   // SET 2,H
   HL.B.B1|=1<<2;
   break;
 case 0xd5:
   // SET 2,L
   HL.B.B0|=1<<2;
   break;
 case 0xd6:
   // SET 2,(HL)
   tempValue=gbReadMemory(HL.W);
   tempValue|=1<<2;
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0xd7:
   // SET 2,A
   AF.B.B1|=1<<2;
   break;
 case 0xd8:
   // SET 3,B
   BC.B.B1|=1<<3;
   break;
 case 0xd9:
   // SET 3,C
   BC.B.B0|=1<<3;
   break;
 case 0xda:
   // SET 3,D
   DE.B.B1|=1<<3;
   break;
 case 0xdb:
   // SET 3,E
   DE.B.B0|=1<<3;
   break;
 case 0xdc:
   // SET 3,H
   HL.B.B1|=1<<3;
   break;
 case 0xdd:
   // SET 3,L
   HL.B.B0|=1<<3;
   break;
 case 0xde:
   // SET 3,(HL)
   tempValue=gbReadMemory(HL.W);
   tempValue|=1<<3;
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0xdf:
   // SET 3,A
   AF.B.B1|=1<<3;
   break;
 case 0xe0:
   // SET 4,B
   BC.B.B1|=1<<4;
   break;
 case 0xe1:
   // SET 4,C
   BC.B.B0|=1<<4;
   break;
 case 0xe2:
   // SET 4,D
   DE.B.B1|=1<<4;
   break;
 case 0xe3:
   // SET 4,E
   DE.B.B0|=1<<4;
   break;
 case 0xe4:
   // SET 4,H
   HL.B.B1|=1<<4;
   break;
 case 0xe5:
   // SET 4,L
   HL.B.B0|=1<<4;
   break;
 case 0xe6:
   // SET 4,(HL)
   tempValue=gbReadMemory(HL.W);
   tempValue|=1<<4;
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0xe7:
   // SET 4,A
   AF.B.B1|=1<<4;
   break;
 case 0xe8:
   // SET 5,B
   BC.B.B1|=1<<5;
   break;
 case 0xe9:
   // SET 5,C
   BC.B.B0|=1<<5;
   break;
 case 0xea:
   // SET 5,D
   DE.B.B1|=1<<5;
   break;
 case 0xeb:
   // SET 5,E
   DE.B.B0|=1<<5;
   break;
 case 0xec:
   // SET 5,H
   HL.B.B1|=1<<5;
   break;
 case 0xed:
   // SET 5,L
   HL.B.B0|=1<<5;
   break;
 case 0xee:
   // SET 5,(HL)
   tempValue=gbReadMemory(HL.W);
   tempValue|=1<<5;
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0xef:
   // SET 5,A
   AF.B.B1|=1<<5;
   break;
 case 0xf0:
   // SET 6,B
   BC.B.B1|=1<<6;
   break;
 case 0xf1:
   // SET 6,C
   BC.B.B0|=1<<6;
   break;
 case 0xf2:
   // SET 6,D
   DE.B.B1|=1<<6;
   break;
 case 0xf3:
   // SET 6,E
   DE.B.B0|=1<<6;
   break;
 case 0xf4:
   // SET 6,H
   HL.B.B1|=1<<6;
   break;
 case 0xf5:
   // SET 6,L
   HL.B.B0|=1<<6;
   break;
 case 0xf6:
   // SET 6,(HL)
   tempValue=gbReadMemory(HL.W);
   tempValue|=1<<6;
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0xf7:
   // SET 6,A
   AF.B.B1|=1<<6;
   break;
 case 0xf8:
   // SET 7,B
   BC.B.B1|=1<<7;
   break;
 case 0xf9:
   // SET 7,C
   BC.B.B0|=1<<7;
   break;
 case 0xfa:
   // SET 7,D
   DE.B.B1|=1<<7;
   break;
 case 0xfb:
   // SET 7,E
   DE.B.B0|=1<<7;
   break;
 case 0xfc:
   // SET 7,H
   HL.B.B1|=1<<7;
   break;
 case 0xfd:
   // SET 7,L
   HL.B.B0|=1<<7;
   break;
 case 0xfe:
   // SET 7,(HL)
   tempValue=gbReadMemory(HL.W);
   tempValue|=1<<7;
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0xff:
   // SET 7,A
   AF.B.B1|=1<<7;
   break;
