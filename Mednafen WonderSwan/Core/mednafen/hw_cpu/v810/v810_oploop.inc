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


    // Macro test taken from http://gcc.gnu.org/viewcvs/trunk/gcc/testsuite/gcc.dg/20020919-1.c?view=markup&pathrev=142696
    //#if defined (__powerpc__) || defined (__PPC__) || defined (__ppc__)	|| defined (__POWERPC__) || defined (PPC) || defined (_IBMR2) 
    //	register v810_timestamp_t timestamp_rl asm("15") = v810_timestamp;
    //#elif defined(__x86_64__)
    //	register v810_timestamp_t timestamp_rl asm("r11") = v810_timestamp;
    //#else
	register v810_timestamp_t timestamp_rl = v810_timestamp;
    //#endif

    uint32 opcode;
    uint32 tmp2;
    int val = 0;


    #define ADDCLOCK(__n) { timestamp += __n; }

    #define CHECK_HALTED();	{ if(Halted && timestamp < next_event_ts) { timestamp = next_event_ts; } }

    while(Running)
    {
     #ifdef RB_DEBUGMODE
     uint32 old_PC = RB_GETPC();
     #endif
     uint32 tmpop;

     assert(timestamp_rl <= next_event_ts);

     if(!IPendingCache)
     {
      if(Halted)
      {
       timestamp_rl = next_event_ts;
      }
      else if(in_bstr)
      {
       tmpop = in_bstr_to;
       opcode = tmpop >> 9;
       goto op_BSTR;
      }
     }

     while(timestamp_rl < next_event_ts)
     {
	#ifdef RB_DEBUGMODE
	old_PC = RB_GETPC();
	#endif

        P_REG[0] = 0; //Zero the Zero Reg!!!

	//v810_timestamp = timestamp_rl;
	RB_CPUHOOK(RB_GETPC());
	//timestamp_rl = v810_timestamp;

	{
	 //printf("%08x\n", RB_GETPC());
	 {
	  v810_timestamp_t timestamp = timestamp_rl;

	  tmpop = RB_RDOP(0, 0);

	  timestamp_rl = timestamp;
	 }

       	 opcode = (tmpop >> 9) | IPendingCache;

	 //printf("%02x\n", opcode >> 1);

	 static const void *const op_goto_table[256] =
	 {
	  #include "v810_op_table.inc"
	 };

	 goto *op_goto_table[opcode];

	// Bit string subopcodes
        #define DO_AM_BSTR()							\
            const uint32 arg1 = (tmpop >> 5) & 0x1F;				\
            const uint32 arg2 = (tmpop & 0x1F);					\
            RB_INCPCBY2();


        #define DO_AM_FPP()							\
            const uint32 arg1 = (tmpop >> 5) & 0x1F;				\
            const uint32 arg2 = (tmpop & 0x1F);					\
            const uint32 arg3 = ((RB_RDOP(2) >> 10)&0x3F);			\
	    RB_INCPCBY4();


        #define DO_AM_UDEF()					\
            RB_INCPCBY2();

        #define DO_AM_I()					\
            const uint32 arg1 = tmpop & 0x1F;			\
            const uint32 arg2 = (tmpop >> 5) & 0x1F;		\
            RB_INCPCBY2();
						
	#define DO_AM_II() DO_AM_I();


        #define DO_AM_IV()					\
	    const uint32 arg1 = ((tmpop & 0x000003FF) << 16) | RB_RDOP(2);	\


        #define DO_AM_V()					\
            const uint32 arg3 = (tmpop >> 5) & 0x1F;		\
            const uint32 arg2 = tmpop & 0x1F;			\
            const uint32 arg1 = RB_RDOP(2);	\
            RB_INCPCBY4();						


        #define DO_AM_VIa()					\
            const uint32 arg1 = RB_RDOP(2);	\
            const uint32 arg2 = tmpop & 0x1F;			\
            const uint32 arg3 = (tmpop >> 5) & 0x1F;		\
            RB_INCPCBY4();						\


        #define DO_AM_VIb()					\
            const uint32 arg1 = (tmpop >> 5) & 0x1F;		\
            const uint32 arg2 = RB_RDOP(2);	\
            const uint32 arg3 = (tmpop & 0x1F);			\
            RB_INCPCBY4();					\

        #define DO_AM_IX()					\
            const uint32 arg1 = (tmpop & 0x1);			\
            RB_INCPCBY2();					\

        #define DO_AM_III()					\
            const uint32 arg1 = tmpop & 0x1FE;

	#include "v810_do_am.h"

	 #define BEGIN_OP(meowtmpop) { op_##meowtmpop: v810_timestamp_t timestamp = timestamp_rl; DO_##meowtmpop ##_AM();
	 #define END_OP()		timestamp_rl = timestamp; goto OpFinished; }
	 #define END_OP_SKIPLO()       	timestamp_rl = timestamp; goto OpFinishedSkipLO; }

	BEGIN_OP(MOV);
	    ADDCLOCK(1);
            SetPREG(arg2, P_REG[arg1]);
	END_OP();


	BEGIN_OP(ADD);
             ADDCLOCK(1);
             uint32 temp = P_REG[arg2] + P_REG[arg1];

             SetFlag(PSW_OV, ((P_REG[arg2]^(~P_REG[arg1]))&(P_REG[arg2]^temp))&0x80000000);
             SetFlag(PSW_CY, temp < P_REG[arg2]);

             SetPREG(arg2, temp);
	     SetSZ(P_REG[arg2]);
	END_OP();


	BEGIN_OP(SUB);
             ADDCLOCK(1);
	     uint32 temp = P_REG[arg2] - P_REG[arg1];

             SetFlag(PSW_OV, ((P_REG[arg2]^P_REG[arg1])&(P_REG[arg2]^temp))&0x80000000);
             SetFlag(PSW_CY, temp > P_REG[arg2]);

	     SetPREG(arg2, temp);
	     SetSZ(P_REG[arg2]);
	END_OP();


	BEGIN_OP(CMP);
             ADDCLOCK(1);
 	     uint32 temp = P_REG[arg2] - P_REG[arg1];

	     SetSZ(temp);
             SetFlag(PSW_OV, ((P_REG[arg2]^P_REG[arg1])&(P_REG[arg2]^temp))&0x80000000);
	     SetFlag(PSW_CY, temp > P_REG[arg2]);
	END_OP();


	BEGIN_OP(SHL);
            ADDCLOCK(1);
            val = P_REG[arg1] & 0x1F;

            // set CY before we destroy the regisrer info....
            SetFlag(PSW_CY, (val != 0) && ((P_REG[arg2] >> (32 - val))&0x01) );
	    SetFlag(PSW_OV, FALSE);
            SetPREG(arg2, P_REG[arg2] << val);
	    SetSZ(P_REG[arg2]);            
	END_OP();
			
	BEGIN_OP(SHR);
            ADDCLOCK(1);
            val = P_REG[arg1] & 0x1F;
            // set CY before we destroy the regisrer info....
            SetFlag(PSW_CY, (val) && ((P_REG[arg2] >> (val-1))&0x01));
	    SetFlag(PSW_OV, FALSE);
	    SetPREG(arg2, P_REG[arg2] >> val);
	    SetSZ(P_REG[arg2]);
	END_OP();

	BEGIN_OP(JMP);

	    (void)arg2;		// arg2 is unused.

            ADDCLOCK(3);
            RB_SETPC((P_REG[arg1] & 0xFFFFFFFE));
	    if(RB_AccurateMode)
	    {
	     BRANCH_ALIGN_CHECK(PC);
	    }
	    RB_ADDBT(old_PC, RB_GETPC(), 0);
	END_OP();

	BEGIN_OP(SAR);
            ADDCLOCK(1);
            val = P_REG[arg1] & 0x1F;
			
	    SetFlag(PSW_CY, (val) && ((P_REG[arg2]>>(val-1))&0x01) );
	    SetFlag(PSW_OV, FALSE);

	    SetPREG(arg2, (uint32) ((int32)P_REG[arg2] >> val));
            
	    SetSZ(P_REG[arg2]);
	END_OP();

	BEGIN_OP(OR);
            ADDCLOCK(1);
            SetPREG(arg2, P_REG[arg1] | P_REG[arg2]);
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg2]);
	END_OP();

	BEGIN_OP(AND);
            ADDCLOCK(1);
            SetPREG(arg2, P_REG[arg1] & P_REG[arg2]);
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg2]);
	END_OP();

	BEGIN_OP(XOR);
            ADDCLOCK(1);
	    SetPREG(arg2, P_REG[arg1] ^ P_REG[arg2]);
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg2]);
	END_OP();

	BEGIN_OP(NOT);
            ADDCLOCK(1);
	    SetPREG(arg2, ~P_REG[arg1]);
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg2]);
	END_OP();

	BEGIN_OP(MOV_I);
            ADDCLOCK(1);
            SetPREG(arg2,sign_5(arg1));
	END_OP();

	BEGIN_OP(ADD_I);
             ADDCLOCK(1);
             uint32 temp = P_REG[arg2] + sign_5(arg1);

             SetFlag(PSW_OV, ((P_REG[arg2]^(~sign_5(arg1)))&(P_REG[arg2]^temp))&0x80000000);
	     SetFlag(PSW_CY, (uint32)temp < P_REG[arg2]);

             SetPREG(arg2, (uint32)temp);
	     SetSZ(P_REG[arg2]);
	END_OP();


	BEGIN_OP(SETF);
		ADDCLOCK(1);

		//SETF may contain bugs
		P_REG[arg2] = 0;

		//if(arg1 != 0xe)
		//printf("SETF: %02x\n", arg1);
		//snortus();
		switch (arg1 & 0x0F) 
		{
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
	END_OP();

	BEGIN_OP(CMP_I);
             ADDCLOCK(1);
	     uint32 temp = P_REG[arg2] - sign_5(arg1);

	     SetSZ(temp);
             SetFlag(PSW_OV, ((P_REG[arg2]^(sign_5(arg1)))&(P_REG[arg2]^temp))&0x80000000);
	     SetFlag(PSW_CY, temp > P_REG[arg2]);
	END_OP();

	BEGIN_OP(SHR_I);
            ADDCLOCK(1);
	    SetFlag(PSW_CY, arg1 && ((P_REG[arg2] >> (arg1-1))&0x01) );
            // set CY before we destroy the regisrer info....
            SetPREG(arg2, P_REG[arg2] >> arg1);
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg2]);
	END_OP();

	BEGIN_OP(SHL_I);
            ADDCLOCK(1);
            SetFlag(PSW_CY, arg1 && ((P_REG[arg2] >> (32 - arg1))&0x01) );
            // set CY before we destroy the regisrer info....

            SetPREG(arg2, P_REG[arg2] << arg1);
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg2]);
	END_OP();

	BEGIN_OP(SAR_I);
            ADDCLOCK(1);
 	    SetFlag(PSW_CY, arg1 && ((P_REG[arg2]>>(arg1-1))&0x01) );

            SetPREG(arg2, (uint32) ((int32)P_REG[arg2] >> arg1));

	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg2]);
	END_OP();

	BEGIN_OP(LDSR);		// Loads a Sys Reg with the value in specified PR
             ADDCLOCK(1);	// ?

	     SetSREG(timestamp, arg1 & 0x1F, P_REG[arg2 & 0x1F]);
	END_OP();

	BEGIN_OP(STSR);		// Loads a PR with the value in specified Sys Reg
             ADDCLOCK(1);	// ?
             P_REG[arg2 & 0x1F] = GetSREG(arg1 & 0x1F);
	END_OP();

        BEGIN_OP(EI);
	    (void)arg1;         // arg1 is unused.
	    (void)arg2;         // arg2 is unused.

	    if(VBMode)
	    {
             ADDCLOCK(1);
             S_REG[PSW] = S_REG[PSW] &~ PSW_ID;
	     RecalcIPendingCache();
	    }
	    else
	    {
	     ADDCLOCK(1);
	     RB_DECPCBY2();
             Exception(INVALID_OP_HANDLER_ADDR, ECODE_INVALID_OP);
             CHECK_HALTED();
	    }
        END_OP();

	BEGIN_OP(DI);
            (void)arg1;         // arg1 is unused.
            (void)arg2;         // arg2 is unused.

            if(VBMode)
            {
             ADDCLOCK(1);
             S_REG[PSW] |= PSW_ID;
	     IPendingCache = 0;
	    }
	    else
            {
             ADDCLOCK(1);
	     RB_DECPCBY2();
             Exception(INVALID_OP_HANDLER_ADDR, ECODE_INVALID_OP);
             CHECK_HALTED();
            }
	END_OP();


	#define COND_BRANCH(cond)			\
		if(cond) 				\
		{ 					\
		 ADDCLOCK(3);				\
		 RB_PCRELCHANGE(sign_9(arg1) & 0xFFFFFFFE);	\
		 if(RB_AccurateMode)			\
		 {					\
		  BRANCH_ALIGN_CHECK(PC);		\
		 }					\
		 RB_ADDBT(old_PC, RB_GETPC(), 0);			\
		}					\
		else					\
		{					\
		 ADDCLOCK(1);				\
		 RB_INCPCBY2();				\
		}

	BEGIN_OP(BV);
		COND_BRANCH(TESTCOND_V);
	END_OP();


	BEGIN_OP(BL);
        	COND_BRANCH(TESTCOND_L);
	END_OP();

	BEGIN_OP(BE);
        	COND_BRANCH(TESTCOND_E);
	END_OP();

	BEGIN_OP(BNH);
          	COND_BRANCH(TESTCOND_NH);
	END_OP();

	BEGIN_OP(BN);
          	COND_BRANCH(TESTCOND_N);
	END_OP();

	BEGIN_OP(BR);
          	COND_BRANCH(TRUE);
	END_OP();

	BEGIN_OP(BLT);
          	COND_BRANCH(TESTCOND_LT);
	END_OP();

	BEGIN_OP(BLE);
          	COND_BRANCH(TESTCOND_LE);
	END_OP();

	BEGIN_OP(BNV);
          	COND_BRANCH(TESTCOND_NV);
	END_OP();

	BEGIN_OP(BNL);
          	COND_BRANCH(TESTCOND_NL);
	END_OP();

	BEGIN_OP(BNE);
          	COND_BRANCH(TESTCOND_NE);
	END_OP();

	BEGIN_OP(BH);
          	COND_BRANCH(TESTCOND_H);
	END_OP();

	BEGIN_OP(BP);
          	COND_BRANCH(TESTCOND_P);
	END_OP();

	BEGIN_OP(BGE);
          	COND_BRANCH(TESTCOND_GE);
	END_OP();

	BEGIN_OP(BGT);
          	COND_BRANCH(TESTCOND_GT);
	END_OP();

	BEGIN_OP(JR);
            ADDCLOCK(3);
            RB_PCRELCHANGE(sign_26(arg1) & 0xFFFFFFFE);
            if(RB_AccurateMode)
            {
             BRANCH_ALIGN_CHECK(PC);
            }
            RB_ADDBT(old_PC, RB_GETPC(), 0);
	END_OP();

	BEGIN_OP(JAL);
            ADDCLOCK(3);
	    P_REG[31] = RB_GETPC() + 4;
            RB_PCRELCHANGE(sign_26(arg1) & 0xFFFFFFFE);
            if(RB_AccurateMode)
            {
             BRANCH_ALIGN_CHECK(PC);
            }
            RB_ADDBT(old_PC, RB_GETPC(), 0);
	END_OP();

	BEGIN_OP(MOVEA);
            ADDCLOCK(1);
	    SetPREG(arg3, P_REG[arg2] + sign_16(arg1));
	END_OP();

	BEGIN_OP(ADDI);
             ADDCLOCK(1);
             uint32 temp = P_REG[arg2] + sign_16(arg1);

             SetFlag(PSW_OV, ((P_REG[arg2]^(~sign_16(arg1)))&(P_REG[arg2]^temp))&0x80000000);
	     SetFlag(PSW_CY, (uint32)temp < P_REG[arg2]);

             SetPREG(arg3, (uint32)temp);
	     SetSZ(P_REG[arg3]);
	END_OP();

	BEGIN_OP(ORI);
            ADDCLOCK(1);
            SetPREG(arg3, arg1 | P_REG[arg2]);
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg3]);
	END_OP();

	BEGIN_OP(ANDI);
            ADDCLOCK(1);
            SetPREG(arg3, (arg1 & P_REG[arg2]));
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg3]);
	END_OP();

	BEGIN_OP(XORI);
            ADDCLOCK(1);
	    SetPREG(arg3, arg1 ^ P_REG[arg2]);
	    SetFlag(PSW_OV, FALSE);
	    SetSZ(P_REG[arg3]);
	END_OP();

	BEGIN_OP(MOVHI);
            ADDCLOCK(1);
            SetPREG(arg3, (arg1 << 16) + P_REG[arg2]);
	END_OP();

	// LD.B
	BEGIN_OP(LD_B);
		        ADDCLOCK(1);
			tmp2 = (sign_16(arg1)+P_REG[arg2])&0xFFFFFFFF;
			
			SetPREG(arg3, sign_8(MemRead8(timestamp, tmp2)));

			//should be 3 clocks when executed alone, 2 when precedes another LD, or 1
			//when precedes an instruction with many clocks (I'm guessing FP, MUL, DIV, etc)
			if(lastop >= 0)
			{
				if(lastop == LASTOP_LD)
				{ 
				 ADDCLOCK(1);
				}
				else
				{
				 ADDCLOCK(2);
				}
			}
			lastop = LASTOP_LD;
	END_OP_SKIPLO();

	// LD.H
	BEGIN_OP(LD_H);
                        ADDCLOCK(1);
			tmp2 = (sign_16(arg1)+P_REG[arg2]) & 0xFFFFFFFE;
		        SetPREG(arg3, sign_16(MemRead16(timestamp, tmp2)));

		        if(lastop >= 0)
			{
                                if(lastop == LASTOP_LD)
				{
				 ADDCLOCK(1);
				}
                                else
				{
				 ADDCLOCK(2);
				}
                        }
			lastop = LASTOP_LD;
	END_OP_SKIPLO();


	// LD.W
	BEGIN_OP(LD_W);
                        ADDCLOCK(1);

                        tmp2 = (sign_16(arg1)+P_REG[arg2]) & 0xFFFFFFFC;

	                if(MemReadBus32[tmp2 >> 24])
			{
			 SetPREG(arg3, MemRead32(timestamp, tmp2));
			
			 if(lastop >= 0)
			 {
				if(lastop == LASTOP_LD)
				{
				 ADDCLOCK(1);
				}
				else
				{
				 ADDCLOCK(2);
				}
			 }
			}
			else
			{
                         SetPREG(arg3, MemRead16(timestamp, tmp2) | (MemRead16(timestamp, tmp2 | 2) << 16));

                         if(lastop >= 0)
                         {
                                if(lastop == LASTOP_LD)
				{
				 ADDCLOCK(3);
				}
                                else
				{
				 ADDCLOCK(4);
				}
                         }
			}
			lastop = LASTOP_LD;
	END_OP_SKIPLO();

	// ST.B
	BEGIN_OP(ST_B);
             ADDCLOCK(1);
             MemWrite8(timestamp, sign_16(arg2)+P_REG[arg3], P_REG[arg1] & 0xFF);

             if(lastop == LASTOP_ST)
	     {
	      ADDCLOCK(1); 
	     }
	     lastop = LASTOP_ST;
	END_OP_SKIPLO();

	// ST.H
	BEGIN_OP(ST_H);
             ADDCLOCK(1);

             MemWrite16(timestamp, (sign_16(arg2)+P_REG[arg3])&0xFFFFFFFE, P_REG[arg1] & 0xFFFF);

             if(lastop == LASTOP_ST)
	     {
	      ADDCLOCK(1);
	     }
	     lastop = LASTOP_ST;
	END_OP_SKIPLO();

	// ST.W
	BEGIN_OP(ST_W);
             ADDCLOCK(1);
  	     tmp2 = (sign_16(arg2)+P_REG[arg3]) & 0xFFFFFFFC;

	     if(MemWriteBus32[tmp2 >> 24])
	     {
	      MemWrite32(timestamp, tmp2, P_REG[arg1]);

              if(lastop == LASTOP_ST)
	      {
	       ADDCLOCK(1);
	      }
	     }
	     else
	     {
              MemWrite16(timestamp, tmp2, P_REG[arg1] & 0xFFFF);
              MemWrite16(timestamp, tmp2 | 2, P_REG[arg1] >> 16);

              if(lastop == LASTOP_ST)
	      {
	       ADDCLOCK(3);
	      }
	     }
	     lastop = LASTOP_ST;
	END_OP_SKIPLO();

	// IN.B
	BEGIN_OP(IN_B);
	    {
             ADDCLOCK(3);
             SetPREG(arg3, IORead8(timestamp, sign_16(arg1)+P_REG[arg2]));
	    }
	    lastop = LASTOP_IN;
	END_OP_SKIPLO();


	// IN.H
	BEGIN_OP(IN_H);
	    {
             ADDCLOCK(3);
             SetPREG(arg3, IORead16(timestamp, (sign_16(arg1)+P_REG[arg2]) & 0xFFFFFFFE));
	    }
	    lastop = LASTOP_IN;
	END_OP_SKIPLO();


	// IN.W
	BEGIN_OP(IN_W);
	     if(IORead32)
	     {
              ADDCLOCK(3);
              SetPREG(arg3, IORead32(timestamp, (sign_16(arg1)+P_REG[arg2]) & 0xFFFFFFFC));
	     }
	     else
	     {
	      uint32 eff_addr = (sign_16(arg1) + P_REG[arg2]) & 0xFFFFFFFC;

	      ADDCLOCK(5);
              SetPREG(arg3, IORead16(timestamp, eff_addr) | ((IORead16(timestamp, eff_addr | 2) << 16)));
	     }
	     lastop = LASTOP_IN;
	END_OP_SKIPLO();


	// OUT.B
	BEGIN_OP(OUT_B);
             ADDCLOCK(1);
             IOWrite8(timestamp, sign_16(arg2)+P_REG[arg3],P_REG[arg1]&0xFF);

	     if(lastop == LASTOP_OUT)
	     {
	      ADDCLOCK(1); 
	     }
	     lastop = LASTOP_OUT;
	END_OP_SKIPLO();


	// OUT.H
	BEGIN_OP(OUT_H);
             ADDCLOCK(1);
             IOWrite16(timestamp, (sign_16(arg2)+P_REG[arg3])&0xFFFFFFFE,P_REG[arg1]&0xFFFF);

             if(lastop == LASTOP_OUT)
             {
              ADDCLOCK(1);
             }
	     lastop = LASTOP_OUT;
	END_OP_SKIPLO();


	// OUT.W
	BEGIN_OP(OUT_W);
             ADDCLOCK(1);

	     if(IOWrite32)
              IOWrite32(timestamp, (sign_16(arg2)+P_REG[arg3])&0xFFFFFFFC,P_REG[arg1]);
	     else
	     {
	      uint32 eff_addr = (sign_16(arg2)+P_REG[arg3])&0xFFFFFFFC;
              IOWrite16(timestamp, eff_addr, P_REG[arg1] & 0xFFFF);
              IOWrite16(timestamp, eff_addr | 2, P_REG[arg1] >> 16);
	     }

             if(lastop == LASTOP_OUT)
             {
	      if(IOWrite32)
              {
	       ADDCLOCK(1);
	      }
	      else
	      {
	       ADDCLOCK(3);
	      }	      
             }
	     lastop = LASTOP_OUT;
	END_OP_SKIPLO();

	BEGIN_OP(NOP);
            (void)arg1;         // arg1 is unused.

            ADDCLOCK(1);
	    RB_INCPCBY2();
	END_OP();

	BEGIN_OP(RETI);
            (void)arg1;         // arg1 is unused.

            ADDCLOCK(10);

            //Return from Trap/Interupt
            if(S_REG[PSW] & PSW_NP) { // Read the FE Reg
                RB_SETPC(S_REG[FEPC] & 0xFFFFFFFE);
                S_REG[PSW] = S_REG[FEPSW];
            } else { 	//Read the EI Reg Interupt
                RB_SETPC(S_REG[EIPC] & 0xFFFFFFFE);
                S_REG[PSW] = S_REG[EIPSW];
            }
	    RecalcIPendingCache();

            RB_ADDBT(old_PC, RB_GETPC(), 0);
	END_OP();

	BEGIN_OP(MUL);
             ADDCLOCK(13);

             uint64 temp = (int64)(int32)P_REG[arg1] * (int32)P_REG[arg2];

             SetPREG(30, (uint32)(temp >> 32));
             SetPREG(arg2, temp);
	     SetSZ(P_REG[arg2]);
	     SetFlag(PSW_OV, temp != (uint32)temp);
	     lastop = -1;
	END_OP_SKIPLO();

	BEGIN_OP(MULU);
             ADDCLOCK(13);
             uint64 temp = (uint64)P_REG[arg1] * (uint64)P_REG[arg2];

             SetPREG(30, (uint32)(temp >> 32));
 	     SetPREG(arg2, (uint32)temp);

	     SetSZ(P_REG[arg2]);
	     SetFlag(PSW_OV, temp != (uint32)temp);
	     lastop = -1;
	END_OP_SKIPLO();

	BEGIN_OP(DIVU);
            ADDCLOCK(36);
            if(P_REG[arg1] == 0) // Divide by zero!
	    {
	     RB_DECPCBY2();
	     Exception(ZERO_DIV_HANDLER_ADDR, ECODE_ZERO_DIV);
	     CHECK_HALTED();
            } 
	    else 
	    {
	     // Careful here, since arg2 can be == 30
	     uint32 quotient = (uint32)P_REG[arg2] / (uint32)P_REG[arg1];
	     uint32 remainder = (uint32)P_REG[arg2] % (uint32)P_REG[arg1];

	     SetPREG(30, remainder);
             SetPREG(arg2, quotient);

	     SetFlag(PSW_OV, FALSE);
	     SetSZ(quotient);
            }
	    lastop = -1;
	END_OP_SKIPLO();

	BEGIN_OP(DIV);
             //if(P_REG[arg1] & P_REG[arg2] & 0x80000000)
             //{
             // printf("Div: %08x %08x\n", P_REG[arg1], P_REG[arg2]);
             //}

            ADDCLOCK(38);
            if((uint32)P_REG[arg1] == 0) // Divide by zero!
	    { 
	     RB_DECPCBY2();
	     Exception(ZERO_DIV_HANDLER_ADDR, ECODE_ZERO_DIV);
	     CHECK_HALTED();
            } 
	    else 
	    {
                if((P_REG[arg2]==0x80000000)&&(P_REG[arg1]==0xFFFFFFFF)) 
		{
			SetFlag(PSW_OV, TRUE);
			P_REG[30]=0;
	                SetPREG(arg2, 0x80000000);
	                SetSZ(P_REG[arg2]);
		}
		else
		{
		     // Careful here, since arg2 can be == 30
        	     uint32 quotient = (int32)P_REG[arg2] / (int32)P_REG[arg1];
	             uint32 remainder = (int32)P_REG[arg2] % (int32)P_REG[arg1];

	             SetPREG(30, remainder);
	             SetPREG(arg2, quotient);

	             SetFlag(PSW_OV, FALSE);
	             SetSZ(quotient);
		}
	    }
	    lastop = -1;
	END_OP_SKIPLO();

	BEGIN_OP(FPP);
            ADDCLOCK(1);
	    fpu_subop(timestamp, arg3, arg1, arg2);
	    lastop = -1;
	    CHECK_HALTED();
	END_OP_SKIPLO();

	BEGIN_OP(BSTR);
	    if(!in_bstr)
	    {
             ADDCLOCK(1);
	    }

            if(bstr_subop(timestamp, arg2, arg1))
	    {
	     RB_DECPCBY2();
             in_bstr = TRUE;
             in_bstr_to = tmpop;
	    }
	    else
	    {
	     in_bstr = FALSE;
	     have_src_cache = have_dst_cache = FALSE;
	    }
	END_OP();

	BEGIN_OP(HALT);
            (void)arg1;         // arg1 is unused.

            ADDCLOCK(1);
	    Halted = HALT_HALT;
            //printf("Untested opcode: HALT\n");
	END_OP();

	BEGIN_OP(TRAP);
            (void)arg2;         // arg2 is unused.

            ADDCLOCK(15);

	    Exception(TRAP_HANDLER_BASE + (arg1 & 0x10), ECODE_TRAP_BASE + (arg1 & 0x1F));
	    CHECK_HALTED();
	END_OP();

	BEGIN_OP(CAXI);
            //printf("Untested opcode: caxi\n");

	    // Lock bus(N/A)

            ADDCLOCK(26);

	    {
	     uint32 addr, tmp, compare_temp;
	     uint32 to_write;

             addr = sign_16(arg1) + P_REG[arg2];
	     addr &= ~3;

	     if(MemReadBus32[addr >> 24])
	      tmp = MemRead32(timestamp, addr);
	     else
	      tmp = MemRead16(timestamp, addr) | (MemRead16(timestamp, addr | 2) << 16);

             compare_temp = P_REG[arg3] - tmp;

             SetSZ(compare_temp);
             SetFlag(PSW_OV, ((P_REG[arg3]^tmp)&(P_REG[arg3]^compare_temp))&0x80000000);
             SetFlag(PSW_CY, compare_temp > P_REG[arg3]);

	     if(!compare_temp) // If they're equal...
	      to_write = P_REG[30];
	     else
	      to_write = tmp;

	     if(MemWriteBus32[addr >> 24])
	      MemWrite32(timestamp, addr, to_write);
	     else
	     {
              MemWrite16(timestamp, addr, to_write & 0xFFFF);
              MemWrite16(timestamp, addr | 2, to_write >> 16);
	     }
	     P_REG[arg3] = tmp;
	    }

	    // Unlock bus(N/A)

	END_OP();



	op_INT_HANDLER:
	{
	 int iNum = ilevel;

	 S_REG[EIPC]  = GetPC();
	 S_REG[EIPSW] = S_REG[PSW];

	 SetPC(0xFFFFFE00 | (iNum << 4));

	 RB_ADDBT(old_PC, RB_GETPC(), 0xFE00 | (iNum << 4));
    
	 S_REG[ECR] = 0xFE00 | (iNum << 4);

	 S_REG[PSW] |= PSW_EP;
	 S_REG[PSW] |= PSW_ID;
	 S_REG[PSW] &= ~PSW_AE;

	 // Now, set need to set the interrupt enable level to he level that is being processed + 1,
	 // saturating at 15.
	 iNum++;

	 if(iNum > 0x0F) 
	  iNum = 0x0F;

	 S_REG[PSW] &= ~PSW_IA;
	 S_REG[PSW] |= iNum << 16;

	 // Accepting an interrupt takes us out of normal HALT status, of course!
	 Halted = HALT_NONE;

	 // Invalidate our bitstring state(forces the instruction to be re-read, and the r/w buffers reloaded).
	 in_bstr = FALSE;
	 have_src_cache = FALSE;
	 have_dst_cache = FALSE;

	 IPendingCache = 0;

 	 goto OpFinished;
	}


	BEGIN_OP(INVALID);
	    RB_DECPCBY2();
	    if(!RB_AccurateMode)
	    {
	     RB_SETPC(RB_GETPC());
	     if((uint32)(RB_RDOP(0, 0) >> 9) != opcode)
	     {
	      //printf("Trampoline: %08x %02x\n", RB_GETPC(), opcode >> 1);
	     }
	     else
	     {
              ADDCLOCK(1);
              Exception(INVALID_OP_HANDLER_ADDR, ECODE_INVALID_OP);
              CHECK_HALTED();
	     }
	    }
	    else
	    {
	     ADDCLOCK(1);
	     Exception(INVALID_OP_HANDLER_ADDR, ECODE_INVALID_OP);
	     CHECK_HALTED();
	    }
	END_OP();

	}

	OpFinished:	;
	lastop = opcode;
	OpFinishedSkipLO: ;
     }	// end  while(timestamp_rl < next_event_ts)
     next_event_ts = event_handler(timestamp_rl);
     //printf("Next: %d, Cur: %d\n", next_event_ts, timestamp);
    }

v810_timestamp = timestamp_rl;
