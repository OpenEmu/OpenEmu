/* V810 Emulator
 *
 * Copyright (C) 2006 David Tucker
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

    uint8 opcode;
    unsigned int arg1 = 0, arg2 = 0, arg3 = 0; // Set to zero to stop compiler whining
    uint32 tmp2;
    int val = 0;

    uint32 clocks;

    CycleCounter += RunCycles;

    #define ADDCLOCK(__n) { int n = __n; clocks +=n; v810_timestamp += n; }

    // HALT emulation not complete(we lose some clock cycles)!!
    #define TESTHALT() { if(Halted) goto HaltHandler; }

    // Begin IN HALT code
    HaltHandler:
    while(Halted && CycleCounter)
    {
     clocks = 1;

     if(ilevel)
     {
      int temp_clocks = v810_int(ilevel);
      ADDCLOCK(temp_clocks);
     }

     CycleCounter -= clocks;
     if(event_king > 0)
     {
      event_king -= clocks;
      if(event_king <= 0)
      {
       KING_DoMagic();
      }
     }
     if(event_pad > 0)
     {
      event_pad -= clocks;
      if(event_pad <= 0)
       FXINPUT_Update();
     }
     if(event_timer > 0)
     {
      event_timer -= clocks;
      if(event_timer <= 0)
       FXTIMER_Update();
     }
     if(event_scsi > 0)
     {
      event_scsi -= clocks;
      if(event_scsi <= 0)
      {
       v810_setevent(V810_EVENT_SCSI, SCSICD_Run(v810_timestamp));
      }
     }
     if(event_adpcm > 0)
     {
      event_adpcm -= clocks;
      if(event_adpcm <= 0)
      {
       DoADPCMUpdate();
      }
     }
    }
    // end IN HALT Code

    while(CycleCounter > 0)
    {
     int32 next_event;
     int32 event_clocks;

     event_clocks = 0;
     next_event = CycleCounter;

     if(event_pad < next_event && event_pad > 0)
      next_event = event_pad;

     if(event_timer < next_event && event_timer > 0)
      next_event = event_timer;

     if(event_scsi < next_event && event_scsi > 0)
      next_event = event_scsi;

     if(event_adpcm < next_event && event_adpcm > 0)
      next_event = event_adpcm;

     if(event_king < next_event && event_king > 0)
      next_event = event_king;

     while(next_event > 0)
     {     
	clocks = 0;

	if(ilevel)
	{ 
	 int temp_clocks = v810_int(ilevel);

	 clocks += temp_clocks;
	 v810_timestamp += temp_clocks;
	}

        P_REG[0]=0; //Zero the Zero Reg!!!

	CHECKBPS();
	CPUHOOKTEST;

	{
	 uint32 tmpop = RDOP(PC); // Only sets the lower 16 bits of tmpop, so we don't have to mask to get our opcode.

       	 opcode = tmpop >> 10;

	 #if 0
	 op_usage[((tmpop & 0xE000) == 0x8000) ? tmpop >> 9 : tmpop >> 10]++;
	 #endif

	static const void* addr_mode_goto_table[64] =
	{
	  &&lab_AM_I,  &&lab_AM_I,  &&lab_AM_I,  &&lab_AM_I,  &&lab_AM_I,  &&lab_AM_I,  &&lab_AM_I,  &&lab_AM_I, // 0x00
	  &&lab_AM_I,  &&lab_AM_I,  &&lab_AM_I,  &&lab_AM_I,  &&lab_AM_I,  &&lab_AM_I,  &&lab_AM_I,  &&lab_AM_I, // 0x08

	  &&lab_AM_II, &&lab_AM_II, &&lab_AM_II, &&lab_AM_II, &&lab_AM_II, &&lab_AM_II, &&lab_AM_II, &&lab_AM_II, // 0x10

	  // TRAP	  RETI		HALT	                 LDSR	         STSR        DI		 BSTR
	  &&lab_AM_II, &&lab_AM_IX, &&lab_AM_IX, &&lab_AM_UDEF, &&lab_AM_II, &&lab_AM_II, &&lab_AM_II, &&lab_AM_BSTR, // 0x18

	  &&lab_AM_III, &&lab_AM_III, &&lab_AM_III, &&lab_AM_III, &&lab_AM_III, &&lab_AM_III, &&lab_AM_III, &&lab_AM_III, // 0x20

	  &&lab_AM_V,  &&lab_AM_V,  &&lab_AM_IV, &&lab_AM_IV, &&lab_AM_V,  &&lab_AM_V,  &&lab_AM_V,  &&lab_AM_V, // 0x28


	  // LD_B	 LD_H                         LD_W           ST_B         ST_H                          ST_W
	  &&lab_AM_VIa, &&lab_AM_VIa, &&lab_AM_UDEF, &&lab_AM_VIa, &&lab_AM_VIb, &&lab_AM_VIb, &&lab_AM_UDEF, &&lab_AM_VIb, // 0x30


	  &&lab_AM_VIa, &&lab_AM_VIa, &&lab_AM_VIa, &&lab_AM_VIa, &&lab_AM_VIb, &&lab_AM_VIb, &&lab_AM_FPP, &&lab_AM_VIb,  // 0x38
	};

	goto *addr_mode_goto_table[opcode];

	static const void* op_goto_table[80] =
	{
	 &&op_MOV, &&op_ADD, &&op_SUB, &&op_CMP, &&op_SHL, &&op_SHR, &&op_JMP, &&op_SAR,
	 &&op_MUL, &&op_DIV, &&op_MULU, &&op_DIVU, &&op_OR, &&op_AND, &&op_XOR, &&op_NOT,

	 &&op_MOV_I, &&op_ADD_I, &&op_SETF, &&op_CMP_I, &&op_SHL_I, &&op_SHR_I, &&op_EI, &&op_SAR_I,

	 &&op_TRAP, &&op_RETI, &&op_HALT, &&op_INVALID, &&op_LDSR, &&op_STSR, &&op_DI, &&op_BSTR,

	 &&op_INVALID,&&op_INVALID,&&op_INVALID,&&op_INVALID,&&op_INVALID,&&op_INVALID,&&op_INVALID,&&op_INVALID,  //0x20 - 0x27  // Lost to Branch Instructions

	 &&op_MOVEA, &&op_ADDI, &&op_JR, &&op_JAL, &&op_ORI, &&op_ANDI, &&op_XORI, &&op_MOVHI,

	 &&op_LD_B, &&op_LD_H, &&op_INVALID, &&op_LD_W, &&op_ST_B, &&op_ST_H, &&op_INVALID, &&op_ST_W,

	 &&op_IN_B, &&op_IN_H, &&op_CAXI, &&op_IN_W, &&op_OUT_B, &&op_OUT_H, &&op_FPP, &&op_OUT_W,

	 &&op_BV, &&op_BL, &&op_BE, &&op_BNH, &&op_BN, &&op_BR, &&op_BLT, &&op_BLE,

	 &&op_BNV, &&op_BNL, &&op_BNE, &&op_BH, &&op_BP, &&op_NOP, &&op_BGE, &&op_BGT,
	};

	#define END_AM }

        lab_AM_BSTR:  // Bit String Subopcodes
            //printf("BitStr: dest: %08x\n", (P_REG[29] & 0xFFFFFFFC));
            arg1 = (tmpop >> 5) & 0x1F;
            arg2 = (tmpop & 0x1F);
            PC += 2;   // 16 bit instruction
	    goto *op_goto_table[opcode];

        lab_AM_FPP:   // Floating Point Subcode
            //printf("FPU");
            arg1 = (tmpop >> 5) & 0x1F;
            arg2 = (tmpop & 0x1F);
            arg3 = ((RDOP(PC + 2) >> 10)&0x3F);
            PC += 4;   // 32 bit instruction
            goto *op_goto_table[opcode];

        lab_AM_UDEF:  // Invalid opcode.
            //printf("\n%08lx\t\t%2x %2x  ;Invalid Opcode", PC, lowB, highB);
            PC += 2;
            goto *op_goto_table[opcode];

        lab_AM_I:       // Do the same Ither way =)
        lab_AM_II:
            arg1 = tmpop & 0x1F;
            arg2 = (tmpop >> 5) & 0x1F;
            PC += 2;   // 16 bit instruction
            goto *op_goto_table[opcode];

        lab_AM_IV:
	    // 32 bit instruction
            //arg1 = ((highB & 0x3) << 24) + (lowB << 16) + (highB2 << 8) + lowB2;
	    arg1 = ((tmpop & 0x000003FF) << 16) | RDOP(PC + 2);
            goto *op_goto_table[opcode];

        lab_AM_V:       
            arg3 = (tmpop >> 5) & 0x1F;
            arg2 = tmpop & 0x1F;
            arg1 = RDOP(PC + 2);
            PC += 4;   // 32 bit instruction
            goto *op_goto_table[opcode];

        lab_AM_VIa:  // Mode6 form1
            arg1 = RDOP(PC + 2);
            arg2 = tmpop & 0x1F;
            arg3 = (tmpop >> 5) & 0x1F;
            PC += 4;   // 32 bit instruction
            goto *op_goto_table[opcode];

        lab_AM_VIb:  // Mode6 form2
            arg1 = (tmpop >> 5) & 0x1F;

            arg2 = RDOP(PC + 2);
            arg3 = (tmpop & 0x1F);

            PC += 4;   // 32 bit instruction
            goto *op_goto_table[opcode];

        lab_AM_IX:
            arg1 = (tmpop & 0x1); // Mode ID, Ignore for now
            PC += 2;   // 16 bit instruction
            goto *op_goto_table[opcode];

        lab_AM_III:
            arg1 = tmpop & 0x1FE;
	    opcode = tmpop >> 9;
            goto *op_goto_table[opcode];


         op_MOV:
	    ADDCLOCK(1);
            SetPREG(arg2, P_REG[arg1]);
            goto OpDone;

          op_ADD:
	    {
             ADDCLOCK(1);
             uint32 temp = P_REG[arg2] + P_REG[arg1];

             SetFlag(PSW_OV, ((P_REG[arg2]^(~P_REG[arg1]))&(P_REG[arg2]^temp))&0x80000000);
             SetFlag(PSW_CY, temp < P_REG[arg2]);

             SetPREG(arg2, temp);
	     SetSZ(P_REG[arg2]);
	    }
            goto OpDone;

          op_SUB:
	    {
             ADDCLOCK(1);
	     uint32 temp = P_REG[arg2] - P_REG[arg1];

             SetFlag(PSW_OV, ((P_REG[arg2]^P_REG[arg1])&(P_REG[arg2]^temp))&0x80000000);
             SetFlag(PSW_CY, temp > P_REG[arg2]);

	     SetPREG(arg2, temp);
	     SetSZ(P_REG[arg2]);
	    }
            goto OpDone;

          op_CMP:
	    {
             ADDCLOCK(1);
 	     uint32 temp = P_REG[arg2] - P_REG[arg1];

	     SetSZ(temp);
             SetFlag(PSW_OV, ((P_REG[arg2]^P_REG[arg1])&(P_REG[arg2]^temp))&0x80000000);
	     SetFlag(PSW_CY, temp > P_REG[arg2]);
	    }
            goto OpDone;

          op_SHL:
            ADDCLOCK(1);
            val = P_REG[arg1] & 0x1F;

            // set CY before we destroy the regisrer info....
            SetFlag(PSW_CY, (val != 0) && ((P_REG[arg2] >> (32 - val))&0x01) );
	    SetFlag(PSW_OV, FALSE);
            SetPREG(arg2, P_REG[arg2] << val);
	    SetSZ(P_REG[arg2]);            
            goto OpDone;
			
          op_SHR:
            ADDCLOCK(1);
            val = P_REG[arg1] & 0x1F;
            // set CY before we destroy the regisrer info....
            SetFlag(PSW_CY, (val) && ((P_REG[arg2] >> (val-1))&0x01));
	    SetFlag(PSW_OV, FALSE);
	    SetPREG(arg2, P_REG[arg2] >> val);
	    SetSZ(P_REG[arg2]);
            goto OpDone;

          op_JMP:
            ADDCLOCK(3);
            PC = (P_REG[arg1] & 0xFFFFFFFE);
            ADDBT(PC);
            goto OpDone;

          op_SAR:
            ADDCLOCK(1);
            val = P_REG[arg1] & 0x1F;
			
	    SetFlag(PSW_CY, (val) && ((P_REG[arg2]>>(val-1))&0x01) );
	    SetFlag(PSW_OV, FALSE);

	    SetPREG(arg2, (uint32) ((int32)P_REG[arg2] >> val));
            
	    SetSZ(P_REG[arg2]);
            goto OpDone;

          op_MUL:
	    {
             ADDCLOCK(13);

             uint64 temp = (int64)(int32)P_REG[arg1] * (int32)P_REG[arg2];

             SetPREG(30, (uint32)(temp >> 32));
             SetPREG(arg2, temp);
	     SetSZ(P_REG[arg2]);
	     SetFlag(PSW_OV, temp != (uint32)temp);
	    }
            goto OpDone;

          op_DIV:
             //if(P_REG[arg1] & P_REG[arg2] & 0x80000000)
             //{
             // printf("Div: %08x %08x\n", P_REG[arg1], P_REG[arg2]);
             //}

            ADDCLOCK(38);
            if((uint32)P_REG[arg1] == 0) // Divide by zero!
	    { 
	     PC -= 2;
	     v810_exp(ZERO_DIV_HANDLER_ADDR, ECODE_ZERO_DIV);
	     TESTHALT();
            } 
	    else 
	    {

                if((P_REG[arg2]==0x80000000)&&(P_REG[arg1]==0xFFFFFFFF)) 
		{
			SetFlag(PSW_OV, TRUE);
			P_REG[30]=0;
	                SetPREG(arg2, 0x80000000);
		} else {
			SetFlag(PSW_OV, FALSE);
			uint32 temp        = (int32)P_REG[arg2] % (int32)P_REG[arg1];
			SetPREG(30, temp);
			SetPREG(arg2, (int32)P_REG[arg2] / (int32)P_REG[arg1]);
		}
		SetSZ(P_REG[arg2]);
            }
            goto OpDone;

          op_MULU:
	    {
             ADDCLOCK(13);
             uint64 temp = (uint64)P_REG[arg1] * (uint64)P_REG[arg2];

             SetPREG(30, (uint32)(temp >> 32));
 	     SetPREG(arg2, (uint32)temp);

	     SetSZ(P_REG[arg2]);
	     SetFlag(PSW_OV, temp != (uint32)temp);
	    }
            goto OpDone;

          op_DIVU:
            ADDCLOCK(36);
            if(P_REG[arg1] == 0) // Divide by zero!
	    {
	     PC -= 2;
	     v810_exp(ZERO_DIV_HANDLER_ADDR, ECODE_ZERO_DIV);
	     TESTHALT();
            } 
	    else 
	    {
		uint32 temp        = (uint32)P_REG[arg2] % (uint32)P_REG[arg1];
		SetPREG(30, temp);
                SetPREG(arg2, (uint32)P_REG[arg2] / (uint32)P_REG[arg1]);
		SetFlag(PSW_OV, FALSE);
		SetSZ(P_REG[arg2]);
            }
            goto OpDone;

          op_OR:
            ADDCLOCK(1);
            SetPREG(arg2, P_REG[arg1] | P_REG[arg2]);
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg2]);
            goto OpDone;

          op_AND:
            ADDCLOCK(1);
            SetPREG(arg2, P_REG[arg1] & P_REG[arg2]);
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg2]);
            goto OpDone;

          op_XOR:
            ADDCLOCK(1);
	    SetPREG(arg2, P_REG[arg1] ^ P_REG[arg2]);
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg2]);
            goto OpDone;

          op_NOT:
            ADDCLOCK(1);
	    SetPREG(arg2, ~P_REG[arg1]);
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg2]);
            goto OpDone;

          op_MOV_I:
            ADDCLOCK(1);
            SetPREG(arg2,sign_5(arg1));
            goto OpDone;

          op_ADD_I:
	    {
             ADDCLOCK(1);
             uint32 temp = P_REG[arg2] + sign_5(arg1);

             SetFlag(PSW_OV, ((P_REG[arg2]^(~sign_5(arg1)))&(P_REG[arg2]^temp))&0x80000000);
	     SetFlag(PSW_CY, (uint32)temp < P_REG[arg2]);

             SetPREG(arg2, (uint32)temp);
	     SetSZ(P_REG[arg2]);
	    }
            goto OpDone;

          op_SETF:
            ADDCLOCK(1);

			//SETF may contain bugs
			P_REG[arg2] = 0;

			//if(arg1 != 0xe)
			//printf("SETF: %02x\n", arg1);
			//snortus();
			switch (arg1 & 0x0F) {
				case COND_V:
					if (TESTCOND_V) P_REG[arg2] = 1;
					break;
				case COND_C:
					if (TESTCOND_C) P_REG[arg2] = 1;
					break;
				case COND_Z:
					if (TESTCOND_Z) P_REG[arg2] = 1;
					break;
				case COND_NH:
					if (TESTCOND_NH) P_REG[arg2] = 1;
					break;
				case COND_S:
					if (TESTCOND_S) P_REG[arg2] = 1;
					break;
				case COND_T:
					P_REG[arg2] = 1;
					break;
				case COND_LT:
					if (TESTCOND_LT) P_REG[arg2] = 1;
					break;
				case COND_LE:
					if (TESTCOND_LE) P_REG[arg2] = 1;
					break;
				case COND_NV:
					if (TESTCOND_NV) P_REG[arg2] = 1;
					break;
				case COND_NC:
					if (TESTCOND_NC) P_REG[arg2] = 1;
					break;
				case COND_NZ:
					if (TESTCOND_NZ) P_REG[arg2] = 1;
					break;
				case COND_H:
					if (TESTCOND_H) P_REG[arg2] = 1;
					break;
				case COND_NS:
					if (TESTCOND_NS) P_REG[arg2] = 1;
					break;
				case COND_F:
					//always false! do nothing more
					break;
				case COND_GE:
					if (TESTCOND_GE) P_REG[arg2] = 1;
					break;
				case COND_GT:
					if (TESTCOND_GT) P_REG[arg2] = 1;
					break;
			}
            goto OpDone;

          op_CMP_I:
	    {
             ADDCLOCK(1);
	     uint32 temp = P_REG[arg2] - sign_5(arg1);

	     SetSZ(temp);
             SetFlag(PSW_OV, ((P_REG[arg2]^(sign_5(arg1)))&(P_REG[arg2]^temp))&0x80000000);
	     SetFlag(PSW_CY, temp > P_REG[arg2]);
	    }
            goto OpDone;

          op_SHL_I:
            ADDCLOCK(1);
            SetFlag(PSW_CY, arg1 && ((P_REG[arg2] >> (32 - arg1))&0x01) );
            // set CY before we destroy the regisrer info....

            SetPREG(arg2, P_REG[arg2] << arg1);
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg2]);
            goto OpDone;

          op_SHR_I:
            ADDCLOCK(1);
	    SetFlag(PSW_CY, arg1 && ((P_REG[arg2] >> (arg1-1))&0x01) );
            // set CY before we destroy the regisrer info....
            SetPREG(arg2, P_REG[arg2] >> arg1);
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg2]);
            goto OpDone;

	  op_EI:
            ADDCLOCK(1);
            S_REG[PSW] = S_REG[PSW] &~ PSW_ID;
            goto OpDone;

          op_SAR_I:
            ADDCLOCK(1);
 	    SetFlag(PSW_CY, arg1 && ((P_REG[arg2]>>(arg1-1))&0x01) );

            SetPREG(arg2, (uint32) ((int32)P_REG[arg2] >> arg1));

	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg2]);
            goto OpDone;

          op_TRAP:
            ADDCLOCK(15);

	    v810_exp(TRAP_HANDLER_BASE + (arg1 & 0x10), ECODE_TRAP_BASE + (arg1 & 0x1F));
	    TESTHALT();
            goto OpDone;

          op_LDSR:
	    {
	     int whichsr = arg1 & 0x1F;
	     uint32 loadval = P_REG[arg2 & 0x1F];

             ADDCLOCK(1); // ?

	     if(whichsr == PSW)
	     {
              S_REG[PSW] = loadval & 0xFF3FF;
	      //printf("%08x\n", loadval & 0xFF3FF);
	     }
             else if(whichsr == ADDTRE)
	     {
              S_REG[ADDTRE] = loadval & 0xFFFFFFFE;
	      printf("Address trap(unemulated): %08x\n", loadval);
	     }
	     else if(whichsr == CHCW)
	     {
	      S_REG[CHCW] = loadval & 0x2;
	      //printf("Cache control: %08x\n", loadval);
	     }
	     else if(whichsr != PIR && whichsr != TKCW && whichsr != ECR)
              S_REG[whichsr] = loadval;
	    }
            goto OpDone;

          op_STSR:
	    {
	     int whichsr = arg1 & 0x1F;

	     if(whichsr != 24 && whichsr != 25 && whichsr >= 8)
	     {
	      printf("STSR Reserved!  %08x %02x\n", PC, arg1);
	     }
	     //else if(whichsr == PIR)
	     // printf("Yarr: %08x\n", PC);

             ADDCLOCK(1);
             P_REG[(arg2 & 0x1F)] = S_REG[whichsr];
	    }
            goto OpDone;

          op_DI:
            ADDCLOCK(1);
            S_REG[PSW] |= PSW_ID;
            goto OpDone;

	#define COND_BRANCH(cond)	if(cond) { ADDCLOCK(3); PC += (sign_9(arg1) & 0xFFFFFFFE);  ADDBT(PC); } else { ADDCLOCK(1); PC += 2; }

          op_BV: COND_BRANCH(TESTCOND_V);
            goto OpDone;

          op_BL: COND_BRANCH(TESTCOND_L);
            goto OpDone;

          op_BE: COND_BRANCH(TESTCOND_E);
            goto OpDone;

          op_BNH: COND_BRANCH(TESTCOND_NH);
            goto OpDone;

          op_BN: COND_BRANCH(TESTCOND_N);
            goto OpDone;

          op_BR: COND_BRANCH(TRUE);
            goto OpDone;

          op_BLT: COND_BRANCH(TESTCOND_LT);
            goto OpDone;

          op_BLE: COND_BRANCH(TESTCOND_LE);
            goto OpDone;

          op_BNV: COND_BRANCH(TESTCOND_NV);
            goto OpDone;

          op_BNL: COND_BRANCH(TESTCOND_NL);
            goto OpDone;

          op_BNE: COND_BRANCH(TESTCOND_NE);
            goto OpDone;

          op_BH: COND_BRANCH(TESTCOND_H);
            goto OpDone;

          op_BP: COND_BRANCH(TESTCOND_P);
            goto OpDone;

          op_BGE: COND_BRANCH(TESTCOND_GE);
            goto OpDone;

          op_BGT: COND_BRANCH(TESTCOND_GT);
            goto OpDone;

          op_JR:
            ADDCLOCK(3);
            PC += (sign_26(arg1) & 0xFFFFFFFE);
	    ADDBT(PC);
            goto OpDone;

          op_JAL:
            ADDCLOCK(3);
	    P_REG[31] = PC + 4;
            PC += (sign_26(arg1) & 0xFFFFFFFE);
            ADDBT(PC);
            goto OpDone;

          op_MOVEA:
            ADDCLOCK(1);
	    SetPREG(arg3, P_REG[arg2] + sign_16(arg1));
            goto OpDone;

          op_ADDI:
	    {
             ADDCLOCK(1);
             uint32 temp = P_REG[arg2] + sign_16(arg1);

             SetFlag(PSW_OV, ((P_REG[arg2]^(~sign_16(arg1)))&(P_REG[arg2]^temp))&0x80000000);
	     SetFlag(PSW_CY, (uint32)temp < P_REG[arg2]);

             SetPREG(arg3, (uint32)temp);
	     SetSZ(P_REG[arg3]);
	    }
            goto OpDone;

          op_ORI:
            ADDCLOCK(1);
            SetPREG(arg3, arg1 | P_REG[arg2]);
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg3]);
            goto OpDone;

          op_ANDI:
            ADDCLOCK(1);
            SetPREG(arg3, (arg1 & P_REG[arg2]));
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg3]);
            goto OpDone;

          op_XORI:
            ADDCLOCK(1);
	    SetPREG(arg3, arg1 ^ P_REG[arg2]);
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg3]);
            goto OpDone;

          op_MOVHI:
            ADDCLOCK(1);
            SetPREG(arg3, (arg1 << 16) + P_REG[arg2]);
            goto OpDone;

          op_RETI:
            ADDCLOCK(10);

            //Return from Trap/Interupt
            if(S_REG[PSW] & PSW_NP) { // Read the FE Reg
                PC = S_REG[FEPC] & 0xFFFFFFFE;
                S_REG[PSW] = S_REG[FEPSW];
            } else { 	//Read the EI Reg Interupt
                PC = S_REG[EIPC] & 0xFFFFFFFE;
                S_REG[PSW] = S_REG[EIPSW];
            }
            ADDBT(PC);
            goto OpDone;

          op_HALT:
            ADDCLOCK(1);
	    Halted = HALT_HALT;
            printf("Unhandled opcode: HALT\n");
	    TESTHALT();
            goto OpDone;

          op_LD_B:
		        ADDCLOCK(1);
			tmp2 = (sign_16(arg1)+P_REG[arg2])&0xFFFFFFFF;
			
			SetPREG(arg3, sign_8(mem_rbyte(tmp2)));

			//should be 3 clocks when executed alone, 2 when precedes another LD, or 1
			//when precedes an instruction with many clocks (I'm guessing FP, MUL, DIV, etc)
			if (lastclock < 6) {
				if ((lastop == LD_B) || (lastop == LD_H) || (lastop == LD_W)) { ADDCLOCK(1); }
				else { ADDCLOCK(2); }
			}
            goto OpDone;

          op_LD_H:
                        ADDCLOCK(1);
			tmp2 = (sign_16(arg1)+P_REG[arg2]) & 0xFFFFFFFE;
		        SetPREG(arg3, sign_16(mem_rhword(tmp2)));
		        if(lastclock < 6)
			{
                                if ((lastop == LD_B) || (lastop == LD_H) || (lastop == LD_W)) { ADDCLOCK(1); }
                                else { ADDCLOCK(2); }
                        }
            goto OpDone;

          op_LD_W:
                        ADDCLOCK(1);
			tmp2 = (sign_16(arg1)+P_REG[arg2]) & 0xFFFFFFFC;
			SetPREG(arg3, mem_rword(tmp2));
			
			if (lastclock < 6) 
			{
				if ((lastop == LD_B) || (lastop == LD_H) || (lastop == LD_W)) { ADDCLOCK(3); }
				else { ADDCLOCK(4); }
			}
            goto OpDone;

          op_ST_B:
            ADDCLOCK(1);
            mem_wbyte(sign_16(arg2)+P_REG[arg3],P_REG[arg1]&0xFF);
   	    //clocks should be 2 clocks when follows another ST
	    if (lastop == ST_B) { ADDCLOCK(1); }
            goto OpDone;

          op_ST_H:
            ADDCLOCK(1);
            mem_whword((sign_16(arg2)+P_REG[arg3])&0xFFFFFFFE,P_REG[arg1]&0xFFFF);
  	    if (lastop == ST_H) { ADDCLOCK(1); }
            goto OpDone;

          op_ST_W:
            ADDCLOCK(1);
  	    tmp2 = (sign_16(arg2)+P_REG[arg3]) & 0xFFFFFFFC;
	    mem_wword(tmp2,P_REG[arg1]);
	    if (lastop == ST_W) { ADDCLOCK(3); }
            goto OpDone;

          op_IN_B:
            ADDCLOCK(3);
            SetPREG(arg3, port_rbyte(sign_16(arg1)+P_REG[arg2]));
            goto OpDone;

          op_IN_H:
            ADDCLOCK(3);
            SetPREG(arg3, port_rhword((sign_16(arg1)+P_REG[arg2]) & 0xFFFFFFFE));
            goto OpDone;

          op_CAXI:
             printf("Untested opcode: caxi\n");

	    // Lock bus(N/A)

            ADDCLOCK(26);

	    {
	     uint32 addr = sign_16(arg1) + P_REG[arg2];
	     uint32 tmp = mem_rword(addr);
             uint32 compare_temp = P_REG[arg3] - tmp;

             SetSZ(compare_temp);
             SetFlag(PSW_OV, ((P_REG[arg3]^tmp)&(P_REG[arg3]^compare_temp))&0x80000000);
             SetFlag(PSW_CY, compare_temp > P_REG[arg3]);

	     if(!compare_temp) // If they're equal...
	     {
	      mem_wword(addr, P_REG[30]);
	      P_REG[arg3] = tmp;
	     }
	     else
	     {
	      mem_wword(addr, tmp);
	      P_REG[arg3] = tmp;
	     } 

	    }

	    // Unlock bus(N/A)

            goto OpDone;

          op_IN_W:
            ADDCLOCK(5);
            SetPREG(arg3, port_rword((sign_16(arg1)+P_REG[arg2]) & 0xFFFFFFFC));
            goto OpDone;

          op_OUT_B:
            ADDCLOCK(1);
            port_wbyte(sign_16(arg2)+P_REG[arg3],P_REG[arg1]&0xFF);
	    //clocks should be 2 when follows another OUT
	    if (lastop == OUT_B) { ADDCLOCK(1); }
            goto OpDone;

          op_OUT_H:
            ADDCLOCK(1);
            port_whword((sign_16(arg2)+P_REG[arg3])&0xFFFFFFFE,P_REG[arg1]&0xFFFF);
	    if (lastop == OUT_H) { ADDCLOCK(1); }
            goto OpDone;

          op_OUT_W:
            ADDCLOCK(1);
            port_wword((sign_16(arg2)+P_REG[arg3])&0xFFFFFFFC,P_REG[arg1]);
 	    if (lastop == OUT_W) { ADDCLOCK(3); }
            goto OpDone;

          op_FPP:
            ADDCLOCK(1);
	    {
		 int temp_clocks = fpu_subop(arg3, arg1, arg2);
		 ADDCLOCK(temp_clocks);
		 TESTHALT();
   	    }
            goto OpDone;

          op_BSTR:
            ADDCLOCK(1);
            bstr_subop(arg2,arg1);    
	    TESTHALT();
            goto OpDone;

         op_NOP:
            ADDCLOCK(1);
            PC +=2;
            goto OpDone;

          op_INVALID:
            ADDCLOCK(1);
	    PC -= 2;
	    v810_exp(INVALID_OP_HANDLER_ADDR, ECODE_INVALID_OP);
	    TESTHALT();
	    goto OpDone;
	}

	OpDone:

        lastop = opcode;
        lastclock = clocks;
        next_event -= clocks;
	event_clocks += clocks;
     }
     CycleCounter -= event_clocks;

     if(event_king > 0)
     {
      event_king -= event_clocks;
      if(event_king <= 0)
      {
       KING_DoMagic();
      }
     }

     if(event_pad > 0)
     {
      event_pad -= event_clocks;
      if(event_pad <= 0)
       FXINPUT_Update();
     }
     if(event_timer > 0)
     {
      event_timer -= event_clocks;
      if(event_timer <= 0)
       FXTIMER_Update();
     }
     if(event_scsi > 0)
     {
      event_scsi -= event_clocks;
      if(event_scsi <= 0)
      {
       v810_setevent(V810_EVENT_SCSI, SCSICD_Run(v810_timestamp));
      }
     }
     if(event_adpcm > 0)
     {
      event_adpcm -= event_clocks;
      if(event_adpcm <= 0)
      {
       DoADPCMUpdate();
      }
     }
    }
