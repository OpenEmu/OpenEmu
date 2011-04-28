 const void * const IOReadHandlers[0x20] =
 {
  &&VDC_00, &&VDC_01, &&VDC_02, &&VDC_03,
  &&VCE_00, &&VCE_01, &&VCE_02, &&VCE_03,
  &&PSG, &&PSG, &&PSG, &&PSG,
  &&TIMER_00, &&TIMER_01, &&TIMER_02, &&TIMER_03,
  &&INPUT_00, &&INPUT_01, &&INPUT_02, &&INPUT_03,
  &&IRQ_00, &&IRQ_01, &&IRQ_02, &&IRQ_03,
  &&CDROM, &&CDROM, &&CDROM, &&CDROM,
  &&EXP, &&EXP, &&EXP, &&EXP
 };

 A &= 0x1FFF;

 goto *IOReadHandlers[((A & 0x1C00) >> 8) | (A & 0x3)];

#if IOREAD_SGX==1
  VDC_00:
  VDC_01:
  VDC_02:
  VDC_03: HuC6280_StealCycle();
	  return(VDC_Read(A&0x1F, TRUE));
#else

 VDC_00:
        HuC6280_StealCycle();
        return(VDC_Read(0, FALSE));

 VDC_01:
        HuC6280_StealCycle();
        return(VDC_Read(1, FALSE));

 VDC_02:
        HuC6280_StealCycle();
        return(VDC_Read(2, FALSE));

 VDC_03:
        HuC6280_StealCycle();
        return(VDC_Read(3, FALSE));
#endif

 VCE_00:
 VCE_01:
 VCE_02:
 VCE_03:
        HuC6280_StealCycle();
        return(VCE_Read(A));

 PSG:
        if(HuCPU.in_block_move)
         return(0);
        return(PCEIODataBuffer);


 TIMER_00:
 TIMER_01:
 TIMER_02:
 TIMER_03:

        if(HuCPU.in_block_move)
         return(0);
        {
         uint8 ret = HuC6280_TimerRead(A);
         PCEIODataBuffer = ret;
         return(ret);
        }

 INPUT_00:
 INPUT_01:
 INPUT_02:
 INPUT_03:
               if(HuCPU.in_block_move)
                return(0);
               {
                uint8 ret = INPUT_Read(A);
                PCEIODataBuffer = ret;
                return(ret);
               }

 IRQ_00:
 IRQ_01:
 IRQ_02:
 IRQ_03:
               if(HuCPU.in_block_move)
                return(0);
               {
                uint8 ret = HuC6280_IRQStatusRead(A);
                PCEIODataBuffer = ret;
                return(ret);
               }

 CDROM:
               if(IsTsushin)
                return(PCE_TsushinRead(A));

               if(!PCE_IsCD)
                return(0xFF);

               if((A & 0x1E00) == 0x1A00)
               {
                if(arcade_card)
                 return(arcade_card->Read(A & 0x1FFF));
                else
                 return(0);
               }
               else
	       {
		int32 next_cd_event;

                return(PCECD_Read(HuCPU.timestamp * 3, A, next_cd_event));
	       }


 EXP:
               if(IsHES)
                return(ReadIBP(A));

 //printf("Meow: %08x, %02x:%04x\n", A, A >> 13, A & 0x1FFF);
 return(0xFF);
