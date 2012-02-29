 case 0x00:
   // NOP
   break;
 case 0x01:
   // LD BC, NNNN
   BC.B.B0=gbReadOpcode(PC.W++);
   BC.B.B1=gbReadOpcode(PC.W++);
   break;
 case 0x02:
   // LD (BC),A
   gbWriteMemory(BC.W,AF.B.B1);
   break;
 case 0x03:
   // INC BC
   BC.W++;
   break;
 case 0x04:
   // INC B
   BC.B.B1++;
   AF.B.B0= (AF.B.B0 & C_FLAG)|ZeroTable[BC.B.B1]| (BC.B.B1&0x0F? 0:H_FLAG);
   break;
 case 0x05:
   // DEC B
   BC.B.B1--;
   AF.B.B0= N_FLAG|(AF.B.B0 & C_FLAG)|ZeroTable[BC.B.B1]|
     ((BC.B.B1&0x0F)==0x0F? H_FLAG:0);
   break;
 case 0x06:
   // LD B, NN
   BC.B.B1=gbReadOpcode(PC.W++);
   break;
 case 0x07:
   // RLCA
   tempValue=AF.B.B1&0x80? C_FLAG:0;
   AF.B.B1=(AF.B.B1<<1)|(AF.B.B1>>7);
   AF.B.B0=tempValue;
   break;
 case 0x08:
   // LD (NNNN), SP
   tempRegister.B.B0=gbReadOpcode(PC.W++);
   tempRegister.B.B1=gbReadOpcode(PC.W++);
   gbWriteMemory(tempRegister.W++,SP.B.B0);
   gbWriteMemory(tempRegister.W,SP.B.B1);
   break;
 case 0x09:
   // ADD HL,BC
   tempRegister.W=(HL.W+BC.W)&0xFFFF;
   AF.B.B0= (AF.B.B0 & Z_FLAG)| ((HL.W^BC.W^tempRegister.W)&0x1000? H_FLAG:0)|
     (((long)HL.W+(long)BC.W)&0x10000? C_FLAG:0);
   HL.W=tempRegister.W;
   break;
 case 0x0a:
   // LD A,(BC)
   AF.B.B1=gbReadMemory(BC.W);
   break;
 case 0x0b:
   // DEC BC
   BC.W--;
   break;
 case 0x0c:
   // INC C
   BC.B.B0++;
   AF.B.B0= (AF.B.B0 & C_FLAG)|ZeroTable[BC.B.B0]| (BC.B.B0&0x0F? 0:H_FLAG);
   break;
 case 0x0d:
   // DEC C
   BC.B.B0--;
   AF.B.B0= N_FLAG|(AF.B.B0 & C_FLAG)|ZeroTable[BC.B.B0]|
     ((BC.B.B0&0x0F)==0x0F? H_FLAG:0);
   break;
 case 0x0e:
   // LD C, NN
   BC.B.B0=gbReadOpcode(PC.W++);
   break;
 case 0x0f:
   // RRCA
   tempValue=AF.B.B1&0x01;
   AF.B.B1=(AF.B.B1>>1)|(tempValue? 0x80:0);
   AF.B.B0=(tempValue<<4);
   break;
 case 0x10:
   // STOP
   opcode = gbReadOpcode(PC.W++);
   if(gbCgbMode) {
     if(gbMemory[0xff4d] & 1) {

       gbSpeedSwitch();
       //clockTicks += 228*144-(gbSpeed ? 62 : 63);

       if(gbSpeed == 0)
         gbMemory[0xff4d] = 0x00;
       else
         gbMemory[0xff4d] = 0x80;
     }
   }
   break;
 case 0x11:
   // LD DE, NNNN
   DE.B.B0=gbReadOpcode(PC.W++);
   DE.B.B1=gbReadOpcode(PC.W++);
   break;
 case 0x12:
   // LD (DE),A
   gbWriteMemory(DE.W,AF.B.B1);
   break;
 case 0x13:
   // INC DE
   DE.W++;
   break;
 case 0x14:
   // INC D
   DE.B.B1++;
   AF.B.B0= (AF.B.B0 & C_FLAG)|ZeroTable[DE.B.B1]| (DE.B.B1&0x0F? 0:H_FLAG);
   break;
 case 0x15:
   // DEC D
   DE.B.B1--;
   AF.B.B0= N_FLAG|(AF.B.B0 & C_FLAG)|ZeroTable[DE.B.B1]|
     ((DE.B.B1&0x0F)==0x0F? H_FLAG:0);
   break;
 case 0x16:
   //  LD D,NN
   DE.B.B1=gbReadOpcode(PC.W++);
   break;
 case 0x17:
   // RLA
   tempValue=AF.B.B1&0x80? C_FLAG:0;
   AF.B.B1=(AF.B.B1<<1)|((AF.B.B0&C_FLAG)>>4);
   AF.B.B0=tempValue;
   break;
 case 0x18:
   // JR NN
   PC.W+=(s8)gbReadOpcode(PC.W)+1;
   break;
 case 0x19:
   // ADD HL,DE
   tempRegister.W=(HL.W+DE.W)&0xFFFF;
   AF.B.B0= (AF.B.B0 & Z_FLAG)| ((HL.W^DE.W^tempRegister.W)&0x1000? H_FLAG:0)|
     (((long)HL.W+(long)DE.W)&0x10000? C_FLAG:0);
   HL.W=tempRegister.W;
   break;
 case 0x1a:
   // LD A,(DE)
   AF.B.B1=gbReadMemory(DE.W);
   break;
 case 0x1b:
   // DEC DE
   DE.W--;
   break;
 case 0x1c:
   // INC E
   DE.B.B0++;
   AF.B.B0= (AF.B.B0 & C_FLAG)|ZeroTable[DE.B.B0]| (DE.B.B0&0x0F? 0:H_FLAG);
   break;
 case 0x1d:
   // DEC E
   DE.B.B0--;
   AF.B.B0= N_FLAG|(AF.B.B0 & C_FLAG)|ZeroTable[DE.B.B0]|
     ((DE.B.B0&0x0F)==0x0F? H_FLAG:0);
   break;
 case 0x1e:
   // LD E,NN
   DE.B.B0=gbReadOpcode(PC.W++);
   break;
 case 0x1f:
   // RRA
   tempValue=AF.B.B1&0x01;
   AF.B.B1=(AF.B.B1>>1)|(AF.B.B0&C_FLAG? 0x80:0);
   AF.B.B0=(tempValue<<4);
   break;
 case 0x20:
   // JR NZ,NN
   if(AF.B.B0&Z_FLAG)
     PC.W++;
   else {
     PC.W+=(s8)gbReadOpcode(PC.W)+1;
     clockTicks++;
   }
   break;
 case 0x21:
   // LD HL,NNNN
   HL.B.B0=gbReadOpcode(PC.W++);
   HL.B.B1=gbReadOpcode(PC.W++);
   break;
 case 0x22:
   // LDI (HL),A
   gbWriteMemory(HL.W++,AF.B.B1);
   break;
 case 0x23:
   // INC HL
   HL.W++;
   break;
 case 0x24:
   // INC H
   HL.B.B1++;
   AF.B.B0= (AF.B.B0 & C_FLAG)|ZeroTable[HL.B.B1]| (HL.B.B1&0x0F? 0:H_FLAG);
   break;
 case 0x25:
   // DEC H
   HL.B.B1--;
   AF.B.B0= N_FLAG|(AF.B.B0 & C_FLAG)|ZeroTable[HL.B.B1]|
     ((HL.B.B1&0x0F)==0x0F? H_FLAG:0);
   break;
 case 0x26:
   // LD H,NN
   HL.B.B1=gbReadOpcode(PC.W++);
   break;
 case 0x27:
   // DAA
   tempRegister.W=AF.B.B1;
   tempRegister.W|=(AF.B.B0&(C_FLAG|H_FLAG|N_FLAG))<<4;
   AF.W=DAATable[tempRegister.W];
   break;
 case 0x28:
   // JR Z,NN
   if(AF.B.B0&Z_FLAG) {
     PC.W+=(s8)gbReadOpcode(PC.W)+1;
     clockTicks++;
   } else
     PC.W++;
   break;
 case 0x29:
   // ADD HL,HL
   tempRegister.W=(HL.W+HL.W)&0xFFFF; AF.B.B0= (AF.B.B0 & Z_FLAG)|
                             ((HL.W^HL.W^tempRegister.W)&0x1000? H_FLAG:0)|
                             (((long)HL.W+(long)HL.W)&0x10000? C_FLAG:0);
   HL.W=tempRegister.W;
   break;
 case 0x2a:
   // LDI A,(HL)
   AF.B.B1 = gbReadMemory(HL.W++);
   break;
 case 0x2b:
   // DEC HL
   HL.W--;
   break;
 case 0x2c:
   // INC L
   HL.B.B0++;
   AF.B.B0= (AF.B.B0 & C_FLAG)|ZeroTable[HL.B.B0]| (HL.B.B0&0x0F? 0:H_FLAG);
   break;
 case 0x2d:
   // DEC L
   HL.B.B0--;
   AF.B.B0= N_FLAG|(AF.B.B0 & C_FLAG)|ZeroTable[HL.B.B0]|
     ((HL.B.B0&0x0F)==0x0F? H_FLAG:0);
   break;
 case 0x2e:
   // LD L,NN
   HL.B.B0=gbReadOpcode(PC.W++);
   break;
 case 0x2f:
   // CPL
   AF.B.B1 ^= 255;
   AF.B.B0|=N_FLAG|H_FLAG;
   break;
 case 0x30:
   // JR NC,NN
   if(AF.B.B0&C_FLAG)
     PC.W++;
   else {
     PC.W+=(s8)gbReadOpcode(PC.W)+1;
     clockTicks++;
   }
   break;
 case 0x31:
   // LD SP,NNNN
   SP.B.B0=gbReadOpcode(PC.W++);
   SP.B.B1=gbReadOpcode(PC.W++);
   break;
 case 0x32:
   // LDD (HL),A
   gbWriteMemory(HL.W--,AF.B.B1);
   break;
 case 0x33:
   // INC SP
   SP.W++;
   break;
 case 0x34:
   // INC (HL)
   tempValue=gbReadMemory(HL.W)+1;
   AF.B.B0= (AF.B.B0 & C_FLAG)|ZeroTable[tempValue]| (tempValue&0x0F? 0:H_FLAG);
   gbWriteMemory(HL.W,tempValue);
   break;
 case 0x35:
   // DEC (HL)
   tempValue=gbReadMemory(HL.W)-1;
   AF.B.B0= N_FLAG|(AF.B.B0 & C_FLAG)|ZeroTable[tempValue]|
     ((tempValue&0x0F)==0x0F? H_FLAG:0);gbWriteMemory(HL.W,tempValue);
   break;
 case 0x36:
   // LD (HL),NN
   gbWriteMemory(HL.W,gbReadOpcode(PC.W++));
   break;
 case 0x37:
   // SCF
   AF.B.B0 = (AF.B.B0 & Z_FLAG) | C_FLAG;
   break;
case 0x38:
  // JR C,NN
  if(AF.B.B0&C_FLAG) {
    PC.W+=(s8)gbReadOpcode(PC.W)+1;
    clockTicks ++;
  } else
    PC.W++;
  break;
 case 0x39:
   // ADD HL,SP
   tempRegister.W=(HL.W+SP.W)&0xFFFF;
   AF.B.B0= (AF.B.B0 & Z_FLAG)| ((HL.W^SP.W^tempRegister.W)&0x1000? H_FLAG:0)|
     (((long)HL.W+(long)SP.W)&0x10000? C_FLAG:0);
   HL.W=tempRegister.W;
   break;
 case 0x3a:
   // LDD A,(HL)
   AF.B.B1 = gbReadMemory(HL.W--);
   break;
 case 0x3b:
   // DEC SP
   SP.W--;
   break;
 case 0x3c:
   // INC A
   AF.B.B1++;
   AF.B.B0= (AF.B.B0 & C_FLAG)|ZeroTable[AF.B.B1]| (AF.B.B1&0x0F? 0:H_FLAG);
   break;
 case 0x3d:
   // DEC A
   AF.B.B1--;
   AF.B.B0= N_FLAG|(AF.B.B0 & C_FLAG)|ZeroTable[AF.B.B1]|
     ((AF.B.B1&0x0F)==0x0F? H_FLAG:0);
   break;
 case 0x3e:
   // LD A,NN
   AF.B.B1=gbReadOpcode(PC.W++);
   break;
 case 0x3f:
   // CCF
   AF.B.B0^=C_FLAG;AF.B.B0&=~(N_FLAG|H_FLAG);
   break;
 case 0x40:
   // LD B,B
   BC.B.B1=BC.B.B1;
   break;
 case 0x41:
   // LD B,C
   BC.B.B1=BC.B.B0;
   break;
 case 0x42:
   // LD B,D
   BC.B.B1=DE.B.B1;
   break;
 case 0x43:
   // LD B,E
   BC.B.B1=DE.B.B0;
   break;
 case 0x44:
   // LD B,H
   BC.B.B1=HL.B.B1;
   break;
 case 0x45:
   // LD B,L
   BC.B.B1=HL.B.B0;
   break;
 case 0x46:
   // LD B,(HL)
   BC.B.B1=gbReadMemory(HL.W);
   break;
 case 0x47:
   // LD B,A
   BC.B.B1=AF.B.B1;
   break;
 case 0x48:
   // LD C,B
   BC.B.B0=BC.B.B1;
   break;
 case 0x49:
   // LD C,C
   BC.B.B0=BC.B.B0;
   break;
 case 0x4a:
   // LD C,D
   BC.B.B0=DE.B.B1;
   break;
 case 0x4b:
   // LD C,E
   BC.B.B0=DE.B.B0;
   break;
 case 0x4c:
   // LD C,H
   BC.B.B0=HL.B.B1;
   break;
 case 0x4d:
   // LD C,L
   BC.B.B0=HL.B.B0;
   break;
 case 0x4e:
   // LD C,(HL)
   BC.B.B0=gbReadMemory(HL.W);
   break;
 case 0x4f:
   // LD C,A
   BC.B.B0=AF.B.B1;
   break;
 case 0x50:
   // LD D,B
   DE.B.B1=BC.B.B1;
   break;
 case 0x51:
   // LD D,C
   DE.B.B1=BC.B.B0;
   break;
 case 0x52:
   // LD D,D
   DE.B.B1=DE.B.B1;
   break;
 case 0x53:
   // LD D,E
   DE.B.B1=DE.B.B0;
   break;
 case 0x54:
   // LD D,H
   DE.B.B1=HL.B.B1;
   break;
 case 0x55:
   // LD D,L
   DE.B.B1=HL.B.B0;
   break;
 case 0x56:
   // LD D,(HL)
   DE.B.B1=gbReadMemory(HL.W);
   break;
 case 0x57:
   // LD D,A
   DE.B.B1=AF.B.B1;
   break;
 case 0x58:
   // LD E,B
   DE.B.B0=BC.B.B1;
   break;
 case 0x59:
   // LD E,C
   DE.B.B0=BC.B.B0;
   break;
 case 0x5a:
   // LD E,D
   DE.B.B0=DE.B.B1;
   break;
 case 0x5b:
   // LD E,E
   DE.B.B0=DE.B.B0;
   break;
 case 0x5c:
   // LD E,H
   DE.B.B0=HL.B.B1;
   break;
 case 0x5d:
   // LD E,L
   DE.B.B0=HL.B.B0;
   break;
 case 0x5e:
   // LD E,(HL)
   DE.B.B0=gbReadMemory(HL.W);
   break;
 case 0x5f:
   // LD E,A
   DE.B.B0=AF.B.B1;
   break;
 case 0x60:
   // LD H,B
   HL.B.B1=BC.B.B1;
   break;
 case 0x61:
   // LD H,C
   HL.B.B1=BC.B.B0;
   break;
 case 0x62:
   // LD H,D
   HL.B.B1=DE.B.B1;
   break;
 case 0x63:
   // LD H,E
   HL.B.B1=DE.B.B0;
   break;
 case 0x64:
   // LD H,H
   HL.B.B1=HL.B.B1;
   break;
 case 0x65:
   // LD H,L
   HL.B.B1=HL.B.B0;
   break;
 case 0x66:
   // LD H,(HL)
   HL.B.B1=gbReadMemory(HL.W);
   break;
 case 0x67:
   // LD H,A
   HL.B.B1=AF.B.B1;
   break;
 case 0x68:
   // LD L,B
   HL.B.B0=BC.B.B1;
   break;
 case 0x69:
   // LD L,C
   HL.B.B0=BC.B.B0;
   break;
 case 0x6a:
   // LD L,D
   HL.B.B0=DE.B.B1;
   break;
 case 0x6b:
   // LD L,E
   HL.B.B0=DE.B.B0;
   break;
 case 0x6c:
   // LD L,H
   HL.B.B0=HL.B.B1;
   break;
 case 0x6d:
   // LD L,L
   HL.B.B0=HL.B.B0;
   break;
 case 0x6e:
   // LD L,(HL)
   HL.B.B0=gbReadMemory(HL.W);
   break;
 case 0x6f:
   // LD L,A
   HL.B.B0=AF.B.B1;
   break;
 case 0x70:
   // LD (HL),B
   gbWriteMemory(HL.W,BC.B.B1);
   break;
 case 0x71:
   // LD (HL),C
   gbWriteMemory(HL.W,BC.B.B0);
   break;
 case 0x72:
   // LD (HL),D
   gbWriteMemory(HL.W,DE.B.B1);
   break;
 case 0x73:
   // LD (HL),E
   gbWriteMemory(HL.W,DE.B.B0);
   break;
 case 0x74:
   // LD (HL),H
   gbWriteMemory(HL.W,HL.B.B1);
   break;
 case 0x75:
   // LD (HL),L
   gbWriteMemory(HL.W,HL.B.B0);
   break;
 case 0x76:
   // HALT
   // If an EI is pending, the interrupts are triggered before Halt state !!
   // Fix Torpedo Range's intro.
   if (IFF & 0x40)
   {
     IFF &= ~0x70;
     IFF |=1;
     PC.W--;
   }
   else
   {
     // if (IE & IF) and interrupts are disabeld,
     // Halt is cancelled.
     if ((register_IE & register_IF & 0x1f) && !(IFF & 1))
     {
       IFF|=2;
     }
     else
       IFF |= 0x80;
   }
   break;
 case 0x77:
   // LD (HL),A
   gbWriteMemory(HL.W,AF.B.B1);
   break;
 case 0x78:
   // LD A,B
   AF.B.B1=BC.B.B1;
   break;
 case 0x79:
   // LD A,C
   AF.B.B1=BC.B.B0;
   break;
 case 0x7a:
   // LD A,D
   AF.B.B1=DE.B.B1;
   break;
 case 0x7b:
   // LD A,E
   AF.B.B1=DE.B.B0;
   break;
 case 0x7c:
   // LD A,H
   AF.B.B1=HL.B.B1;
   break;
 case 0x7d:
   // LD A,L
   AF.B.B1=HL.B.B0;
   break;
 case 0x7e:
   // LD A,(HL)
   AF.B.B1=gbReadMemory(HL.W);
   break;
 case 0x7f:
   // LD A,A
   AF.B.B1=AF.B.B1;
   break;
 case 0x80:
   // ADD B
   tempRegister.W=AF.B.B1+BC.B.B1;
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^BC.B.B1^tempRegister.B.B0)&0x10 ? H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x81:
   // ADD C
   tempRegister.W=AF.B.B1+BC.B.B0;
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^BC.B.B0^tempRegister.B.B0)&0x10 ? H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x82:
   // ADD D
   tempRegister.W=AF.B.B1+DE.B.B1;
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^DE.B.B1^tempRegister.B.B0)&0x10 ? H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x83:
   // ADD E
   tempRegister.W=AF.B.B1+DE.B.B0;
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^DE.B.B0^tempRegister.B.B0)&0x10 ? H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x84:
   // ADD H
   tempRegister.W=AF.B.B1+HL.B.B1;
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^HL.B.B1^tempRegister.B.B0)&0x10 ? H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x85:
   // ADD L
   tempRegister.W=AF.B.B1+HL.B.B0;
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^HL.B.B0^tempRegister.B.B0)&0x10 ? H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x86:
   // ADD (HL)
   tempValue=gbReadMemory(HL.W);
   tempRegister.W=AF.B.B1+tempValue;
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^tempValue^tempRegister.B.B0)&0x10 ? H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x87:
   // ADD A
   tempRegister.W=AF.B.B1+AF.B.B1;
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^AF.B.B1^tempRegister.B.B0)&0x10 ? H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x88:
   // ADC B:
   tempRegister.W=AF.B.B1+BC.B.B1+(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^BC.B.B1^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x89:
   // ADC C
   tempRegister.W=AF.B.B1+BC.B.B0+(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^BC.B.B0^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x8a:
   // ADC D
   tempRegister.W=AF.B.B1+DE.B.B1+(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^DE.B.B1^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x8b:
   // ADC E
   tempRegister.W=AF.B.B1+DE.B.B0+(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^DE.B.B0^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x8c:
   // ADC H
   tempRegister.W=AF.B.B1+HL.B.B1+(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^HL.B.B1^tempRegister.B.B0)&0x10?H_FLAG:0); AF.B.B1=tempRegister.B.B0;
   break;
 case 0x8d:
   // ADC L
   tempRegister.W=AF.B.B1+HL.B.B0+(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^HL.B.B0^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x8e:
   // ADC (HL)
   tempValue=gbReadMemory(HL.W);
   tempRegister.W=AF.B.B1+tempValue+(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^tempValue^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x8f:
   // ADC A
   tempRegister.W=AF.B.B1+AF.B.B1+(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^AF.B.B1^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x90:
   // SUB B
   tempRegister.W=AF.B.B1-BC.B.B1;
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^BC.B.B1^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x91:
   // SUB C
   tempRegister.W=AF.B.B1-BC.B.B0;
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^BC.B.B0^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x92:
   // SUB D
   tempRegister.W=AF.B.B1-DE.B.B1;
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^DE.B.B1^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x93:
   // SUB E
   tempRegister.W=AF.B.B1-DE.B.B0;
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^DE.B.B0^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x94:
   // SUB H
   tempRegister.W=AF.B.B1-HL.B.B1;
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^HL.B.B1^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x95:
   // SUB L
   tempRegister.W=AF.B.B1-HL.B.B0;
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^HL.B.B0^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x96:
   // SUB (HL)
   tempValue=gbReadMemory(HL.W);
   tempRegister.W=AF.B.B1-tempValue;
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^tempValue^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x97:
   // SUB A
   AF.B.B1=0;
   AF.B.B0=N_FLAG|Z_FLAG;
   break;
 case 0x98:
   // SBC B
   tempRegister.W=AF.B.B1-BC.B.B1-(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^BC.B.B1^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x99:
   // SBC C
   tempRegister.W=AF.B.B1-BC.B.B0-(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^BC.B.B0^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x9a:
   // SBC D
   tempRegister.W=AF.B.B1-DE.B.B1-(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^DE.B.B1^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x9b:
   // SBC E
   tempRegister.W=AF.B.B1-DE.B.B0-(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^DE.B.B0^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x9c:
   // SBC H
   tempRegister.W=AF.B.B1-HL.B.B1-(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^HL.B.B1^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x9d:
   // SBC L
   tempRegister.W=AF.B.B1-HL.B.B0-(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^HL.B.B0^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x9e:
   // SBC (HL)
   tempValue=gbReadMemory(HL.W);
   tempRegister.W=AF.B.B1-tempValue-(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^tempValue^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0x9f:
   // SBC A
   tempRegister.W=AF.B.B1-AF.B.B1-(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^AF.B.B1^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0xa0:
   // AND B
   AF.B.B1&=BC.B.B1;
   AF.B.B0=H_FLAG|ZeroTable[AF.B.B1];
   break;
 case 0xa1:
   // AND C
   AF.B.B1&=BC.B.B0;
   AF.B.B0=H_FLAG|ZeroTable[AF.B.B1];
   break;
 case 0xa2:
   // AND_D
   AF.B.B1&=DE.B.B1;
   AF.B.B0=H_FLAG|ZeroTable[AF.B.B1];
   break;
 case 0xa3:
   // AND E
   AF.B.B1&=DE.B.B0;
   AF.B.B0=H_FLAG|ZeroTable[AF.B.B1];
   break;
 case 0xa4:
   // AND H
   AF.B.B1&=HL.B.B1;
   AF.B.B0=H_FLAG|ZeroTable[AF.B.B1];
   break;
 case 0xa5:
   // AND L
   AF.B.B1&=HL.B.B0;
   AF.B.B0=H_FLAG|ZeroTable[AF.B.B1];
   break;
 case 0xa6:
   // AND (HL)
   tempValue=gbReadMemory(HL.W);
   AF.B.B1&=tempValue;
   AF.B.B0=H_FLAG|ZeroTable[AF.B.B1];
   break;
 case 0xa7:
   // AND A
   AF.B.B1&=AF.B.B1;
   AF.B.B0=H_FLAG|ZeroTable[AF.B.B1];
   break;
 case 0xa8:
   // XOR B
   AF.B.B1^=BC.B.B1;
   AF.B.B0=ZeroTable[AF.B.B1];
   break;
 case 0xa9:
   // XOR C
   AF.B.B1^=BC.B.B0;
   AF.B.B0=ZeroTable[AF.B.B1];
   break;
 case 0xaa:
   // XOR D
   AF.B.B1^=DE.B.B1;
   AF.B.B0=ZeroTable[AF.B.B1];
   break;
 case 0xab:
   // XOR E
   AF.B.B1^=DE.B.B0;
   AF.B.B0=ZeroTable[AF.B.B1];
   break;
 case 0xac:
   // XOR H
   AF.B.B1^=HL.B.B1;
   AF.B.B0=ZeroTable[AF.B.B1];
   break;
 case 0xad:
   // XOR L
   AF.B.B1^=HL.B.B0;
   AF.B.B0=ZeroTable[AF.B.B1];
   break;
 case 0xae:
   // XOR (HL)
   tempValue=gbReadMemory(HL.W);
   AF.B.B1^=tempValue;
   AF.B.B0=ZeroTable[AF.B.B1];
   break;
 case 0xaf:
   // XOR A
   AF.B.B1=0;
   AF.B.B0=Z_FLAG;
   break;
 case 0xb0:
   // OR B
   AF.B.B1|=BC.B.B1;
   AF.B.B0=ZeroTable[AF.B.B1];
   break;
 case 0xb1:
   // OR C
   AF.B.B1|=BC.B.B0;
   AF.B.B0=ZeroTable[AF.B.B1];
   break;
 case 0xb2:
   // OR D
   AF.B.B1|=DE.B.B1;
   AF.B.B0=ZeroTable[AF.B.B1];
   break;
 case 0xb3:
   // OR E
   AF.B.B1|=DE.B.B0;
   AF.B.B0=ZeroTable[AF.B.B1];
   break;
 case 0xb4:
   // OR H
   AF.B.B1|=HL.B.B1;
   AF.B.B0=ZeroTable[AF.B.B1];
   break;
 case 0xb5:
   // OR L
   AF.B.B1|=HL.B.B0;
   AF.B.B0=ZeroTable[AF.B.B1];
   break;
 case 0xb6:
   // OR (HL)
   tempValue=gbReadMemory(HL.W);
   AF.B.B1|=tempValue;
   AF.B.B0=ZeroTable[AF.B.B1];
   break;
 case 0xb7:
   // OR A
   AF.B.B1|=AF.B.B1;
   AF.B.B0=ZeroTable[AF.B.B1];
   break;
 case 0xb8:
   // CP B:
   tempRegister.W=AF.B.B1-BC.B.B1;
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^BC.B.B1^tempRegister.B.B0)&0x10?H_FLAG:0);
   break;
 case 0xb9:
   // CP C
   tempRegister.W=AF.B.B1-BC.B.B0;
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^BC.B.B0^tempRegister.B.B0)&0x10?H_FLAG:0);
   break;
 case 0xba:
   // CP D
   tempRegister.W=AF.B.B1-DE.B.B1;
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^DE.B.B1^tempRegister.B.B0)&0x10?H_FLAG:0);
   break;
 case 0xbb:
   // CP E
   tempRegister.W=AF.B.B1-DE.B.B0;
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^DE.B.B0^tempRegister.B.B0)&0x10?H_FLAG:0);
   break;
 case 0xbc:
   // CP H
   tempRegister.W=AF.B.B1-HL.B.B1;
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^HL.B.B1^tempRegister.B.B0)&0x10?H_FLAG:0);
   break;
 case 0xbd:
   // CP L
   tempRegister.W=AF.B.B1-HL.B.B0;
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^HL.B.B0^tempRegister.B.B0)&0x10?H_FLAG:0);
   break;
 case 0xbe:
   // CP (HL)
   tempValue=gbReadMemory(HL.W);
   tempRegister.W=AF.B.B1-tempValue;
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^tempValue^tempRegister.B.B0)&0x10?H_FLAG:0);
   break;
 case 0xbf:
   // CP A
   AF.B.B0=N_FLAG|Z_FLAG;
   break;
 case 0xc0:
   // RET NZ
   if(!(AF.B.B0&Z_FLAG)) {
     PC.B.B0=gbReadMemory(SP.W++);
     PC.B.B1=gbReadMemory(SP.W++);
     clockTicks += 3;
   }
   break;
 case 0xc1:
   // POP BC
   BC.B.B0=gbReadMemory(SP.W++);
   BC.B.B1=gbReadMemory(SP.W++);
   break;
 case 0xc2:
   // JP NZ,NNNN
   if(AF.B.B0&Z_FLAG)
     PC.W+=2;
   else {
     tempRegister.B.B0=gbReadOpcode(PC.W++);
     tempRegister.B.B1=gbReadOpcode(PC.W);
     PC.W=tempRegister.W;
     clockTicks++;
   }
   break;
 case 0xc3:
   // JP NNNN
   tempRegister.B.B0=gbReadOpcode(PC.W++);
   tempRegister.B.B1=gbReadOpcode(PC.W);
   PC.W=tempRegister.W;
   break;
 case 0xc4:
   // CALL NZ,NNNN
   if(AF.B.B0&Z_FLAG)
     PC.W+=2;
   else {
     tempRegister.B.B0=gbReadOpcode(PC.W++);
     tempRegister.B.B1=gbReadOpcode(PC.W++);
     gbWriteMemory(--SP.W,PC.B.B1);
     gbWriteMemory(--SP.W,PC.B.B0);
     PC.W=tempRegister.W;
     clockTicks += 3;
   }
   break;
 case 0xc5:
   // PUSH BC
   gbWriteMemory(--SP.W,BC.B.B1);
   gbWriteMemory(--SP.W,BC.B.B0);
   break;
 case 0xc6:
   // ADD NN
   tempValue=gbReadOpcode(PC.W++);
   tempRegister.W=AF.B.B1+tempValue;
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^tempValue^tempRegister.B.B0)&0x10 ? H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0xc7:
   // RST 00
   gbWriteMemory(--SP.W,PC.B.B1);
   gbWriteMemory(--SP.W,PC.B.B0);
   PC.W=0x0000;
   break;
 case 0xc8:
   // RET Z
   if(AF.B.B0&Z_FLAG) {
     PC.B.B0=gbReadMemory(SP.W++);
     PC.B.B1=gbReadMemory(SP.W++);
     clockTicks += 3;
   }
   break;
 case 0xc9:
   // RET
   PC.B.B0=gbReadMemory(SP.W++);
   PC.B.B1=gbReadMemory(SP.W++);
   break;
 case 0xca:
   // JP Z,NNNN
   if(AF.B.B0&Z_FLAG) {
     tempRegister.B.B0=gbReadOpcode(PC.W++);
     tempRegister.B.B1=gbReadOpcode(PC.W);
     PC.W=tempRegister.W;
     clockTicks++;
   } else
     PC.W+=2;
   break;
   // CB done outside
 case 0xcc:
   // CALL Z,NNNN
   if(AF.B.B0&Z_FLAG) {
     tempRegister.B.B0=gbReadOpcode(PC.W++);
     tempRegister.B.B1=gbReadOpcode(PC.W++);
     gbWriteMemory(--SP.W,PC.B.B1);
     gbWriteMemory(--SP.W,PC.B.B0);
     PC.W=tempRegister.W;
     clockTicks += 3;
   } else
     PC.W+=2;
   break;
 case 0xcd:
   // CALL NNNN
   tempRegister.B.B0=gbReadOpcode(PC.W++);
   tempRegister.B.B1=gbReadOpcode(PC.W++);
   gbWriteMemory(--SP.W,PC.B.B1);
   gbWriteMemory(--SP.W,PC.B.B0);
   PC.W=tempRegister.W;
   break;
 case 0xce:
   // ADC NN
   tempValue=gbReadOpcode(PC.W++);
   tempRegister.W=AF.B.B1+tempValue+(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= (tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^tempValue^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0xcf:
   // RST 08
   gbWriteMemory(--SP.W,PC.B.B1);
   gbWriteMemory(--SP.W,PC.B.B0);
   PC.W=0x0008;
   break;
 case 0xd0:
   // RET NC
   if(!(AF.B.B0&C_FLAG)) {
     PC.B.B0=gbReadMemory(SP.W++);
     PC.B.B1=gbReadMemory(SP.W++);
     clockTicks += 3;
   }
   break;
 case 0xd1:
   // POP DE
   DE.B.B0=gbReadMemory(SP.W++);
   DE.B.B1=gbReadMemory(SP.W++);
   break;
 case 0xd2:
   // JP NC,NNNN
   if(AF.B.B0&C_FLAG)
     PC.W+=2;
   else {
     tempRegister.B.B0=gbReadOpcode(PC.W++);
     tempRegister.B.B1=gbReadOpcode(PC.W);
     PC.W=tempRegister.W;
     clockTicks++;
   }
   break;
   // D3 illegal
 case 0xd3:
     PC.W--;
     IFF = 0;
   break;
 case 0xd4:
   // CALL NC,NNNN
   if(AF.B.B0&C_FLAG)
     PC.W+=2;
   else {
     tempRegister.B.B0=gbReadOpcode(PC.W++);
     tempRegister.B.B1=gbReadOpcode(PC.W++);
     gbWriteMemory(--SP.W,PC.B.B1);
     gbWriteMemory(--SP.W,PC.B.B0);
     PC.W=tempRegister.W;
     clockTicks += 3;
   }
   break;
 case 0xd5:
   // PUSH DE
   gbWriteMemory(--SP.W,DE.B.B1);
   gbWriteMemory(--SP.W,DE.B.B0);
   break;
 case 0xd6:
   // SUB NN
   tempValue=gbReadOpcode(PC.W++);
   tempRegister.W=AF.B.B1-tempValue;
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^tempValue^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0xd7:
   // RST 10
   gbWriteMemory(--SP.W,PC.B.B1);
   gbWriteMemory(--SP.W,PC.B.B0);
   PC.W=0x0010;
   break;
 case 0xd8:
   // RET C
   if(AF.B.B0&C_FLAG) {
     PC.B.B0=gbReadMemory(SP.W++);
     PC.B.B1=gbReadMemory(SP.W++);
     clockTicks += 3;
   }
   break;
 case 0xd9:
   // RETI
   PC.B.B0=gbReadMemory(SP.W++);
   PC.B.B1=gbReadMemory(SP.W++);
   IFF |= 0x01;
   break;
 case 0xda:
   // JP C,NNNN
   if(AF.B.B0&C_FLAG) {
     tempRegister.B.B0=gbReadOpcode(PC.W++);
     tempRegister.B.B1=gbReadOpcode(PC.W);
     PC.W=tempRegister.W;
     clockTicks++;
   } else
     PC.W+=2;
   break;
   // DB illegal
 case 0xdb:
     PC.W--;
     IFF = 0;
   break;
 case 0xdc:
   // CALL C,NNNN
   if(AF.B.B0&C_FLAG) {
     tempRegister.B.B0=gbReadOpcode(PC.W++);
     tempRegister.B.B1=gbReadOpcode(PC.W++);
     gbWriteMemory(--SP.W,PC.B.B1);
     gbWriteMemory(--SP.W,PC.B.B0);
     PC.W=tempRegister.W;
     clockTicks += 3;
   } else
     PC.W+=2;
   break;
   // DD illegal
 case 0xdd:
     PC.W--;
     IFF = 0;
   break;
 case 0xde:
   // SBC NN
   tempValue=gbReadOpcode(PC.W++);
   tempRegister.W=AF.B.B1-tempValue-(AF.B.B0&C_FLAG ? 1 : 0);
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^tempValue^tempRegister.B.B0)&0x10?H_FLAG:0);
   AF.B.B1=tempRegister.B.B0;
   break;
 case 0xdf:
   // RST 18
   gbWriteMemory(--SP.W,PC.B.B1);
   gbWriteMemory(--SP.W,PC.B.B0);
   PC.W=0x0018;
   break;
 case 0xe0:
   // LD (FF00+NN),A
   gbWriteMemory(0xff00 + gbReadOpcode(PC.W++),AF.B.B1);
   break;
 case 0xe1:
   // POP HL
   HL.B.B0=gbReadMemory(SP.W++);
   HL.B.B1=gbReadMemory(SP.W++);
   break;
 case 0xe2:
   // LD (FF00+C),A
   gbWriteMemory(0xff00 + BC.B.B0,AF.B.B1);
   break;
   // E3 illegal
   // E4 illegal
 case 0xe3:
 case 0xe4:
     PC.W--;
     IFF = 0;
   break;
 case 0xe5:
   // PUSH HL
   gbWriteMemory(--SP.W,HL.B.B1);
   gbWriteMemory(--SP.W,HL.B.B0);
   break;
 case 0xe6:
   // AND NN
   tempValue=gbReadOpcode(PC.W++);
   AF.B.B1&=tempValue;
   AF.B.B0=H_FLAG|ZeroTable[AF.B.B1];
   break;
 case 0xe7:
   // RST 20
   gbWriteMemory(--SP.W,PC.B.B1);
   gbWriteMemory(--SP.W,PC.B.B0);
   PC.W=0x0020;
   break;
 case 0xe8:
   // ADD SP,NN
   offset = (s8)gbReadOpcode(PC.W++);
   tempRegister.W = SP.W + offset;
   AF.B.B0 = ((SP.W^offset^tempRegister.W)&0x100? C_FLAG : 0) |
             ((SP.W^offset^tempRegister.W)& 0x10? H_FLAG : 0);
   SP.W = tempRegister.W;
   break;
 case 0xe9:
   // LD PC,HL
   PC.W=HL.W;
   break;
 case 0xea:
   // LD (NNNN),A
   tempRegister.B.B0=gbReadOpcode(PC.W++);
   tempRegister.B.B1=gbReadOpcode(PC.W++);
   gbWriteMemory(tempRegister.W,AF.B.B1);
   break;
   // EB illegal
   // EC illegal
   // ED illegal
 case 0xeb:
 case 0xec:
 case 0xed:
     PC.W--;
     IFF = 0;
   break;
 case 0xee:
   // XOR NN
   tempValue=gbReadOpcode(PC.W++);
   AF.B.B1^=tempValue;
   AF.B.B0=ZeroTable[AF.B.B1];
   break;
 case 0xef:
   // RST 28
   gbWriteMemory(--SP.W,PC.B.B1);
   gbWriteMemory(--SP.W,PC.B.B0);
   PC.W=0x0028;
   break;
 case 0xf0:
   // LD A,(FF00+NN)
   AF.B.B1 = gbReadMemory(0xff00+gbReadOpcode(PC.W++));
   break;
 case 0xf1:
   // POP AF
   AF.B.B0=gbReadMemory(SP.W++)&0xF0;
   AF.B.B1=gbReadMemory(SP.W++);
   break;
 case 0xf2:
   // LD A,(FF00+C)
   AF.B.B1 = gbReadMemory(0xff00+BC.B.B0);
   break;
 case 0xf3:
   // DI
 //   IFF&=0xFE;
     IFF|=0x08;
   break;
   // F4 illegal
 case 0xf4:
     PC.W--;
     IFF = 0;
   break;
 case 0xf5:
   // PUSH AF
   gbWriteMemory(--SP.W,AF.B.B1);
   gbWriteMemory(--SP.W,AF.B.B0);
   break;
 case 0xf6:
   // OR NN
   tempValue=gbReadOpcode(PC.W++);
   AF.B.B1|=tempValue;
   AF.B.B0=ZeroTable[AF.B.B1];
   break;
 case 0xf7:
   // RST 30
   gbWriteMemory(--SP.W,PC.B.B1);
   gbWriteMemory(--SP.W,PC.B.B0);
   PC.W=0x0030;
   break;
 case 0xf8:
   // LD HL,SP+NN
   offset = (s8)gbReadOpcode(PC.W++);
   tempRegister.W = SP.W + offset;
   AF.B.B0 = ((SP.W^offset^tempRegister.W)&0x100? C_FLAG : 0) |
             ((SP.W^offset^tempRegister.W)& 0x10? H_FLAG : 0);
   HL.W = tempRegister.W;
   break;
 case 0xf9:
   // LD SP,HL
   SP.W=HL.W;
   break;
 case 0xfa:
   // LD A,(NNNN)
   tempRegister.B.B0=gbReadOpcode(PC.W++);
   tempRegister.B.B1=gbReadOpcode(PC.W++);
   AF.B.B1=gbReadMemory(tempRegister.W);
   break;
 case 0xfb:
   // EI
   if (!(IFF & 0x30))
     // If an EI is executed right before HALT,
     // the interrupts are triggered before the Halt state !!
     // Fix Torpedo Range Intro.
     // IFF |= 0x10 : 1 ticks before the EI enables the interrupts
     // IFF |= 0x40 : marks that an EI is being executed.
     IFF|=0x50;
   break;
   // FC illegal (FC = breakpoint)
 case 0xfc:
    breakpoint = true;
  break;
   // FD illegal
 case 0xfd:
     PC.W--;
     IFF = 0;
   break;
 case 0xfe:
   // CP NN
   tempValue=gbReadOpcode(PC.W++);
   tempRegister.W=AF.B.B1-tempValue;
   AF.B.B0= N_FLAG|(tempRegister.B.B1?C_FLAG:0)|ZeroTable[tempRegister.B.B0]|
     ((AF.B.B1^tempValue^tempRegister.B.B0)&0x10?H_FLAG:0);
   break;
 case 0xff:
   // RST 38
   gbWriteMemory(--SP.W,PC.B.B1);
   gbWriteMemory(--SP.W,PC.B.B0);
   PC.W=0x0038;
   break;
 default:
   if (gbSystemMessage == false)
   {
     systemMessage(0, N_("Unknown opcode %02x at %04x"),
                   gbReadOpcode(PC.W-1),PC.W-1);
     gbSystemMessage =true;
   }
   return;
