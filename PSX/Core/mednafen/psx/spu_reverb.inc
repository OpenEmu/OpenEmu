static int16 ReverbSat(int32 samp) MDFN_WARN_UNUSED_RESULT;
static INLINE int16 ReverbSat(int32 samp)
{
 if(samp > 32767)
  samp = 32767;

 if(samp < -32768)
  samp = -32768;

 return(samp);
}


INLINE int32 PS_SPU::Get_Reverb_Offset(int32 in_offset)
{
 int32 offset = in_offset & 0x3FFFF;
 int32 wa_size = 0x40000 - ReverbWA;

 if(offset & 0x20000)
 {
  offset -= ReverbWA;

  if(offset < 0)
  {
   offset = 0;
   //PSX_WARNING("[SPU] A reverb offset is broken(-).");
  }
 }
 else
 {
  if(offset >= wa_size)
  {
   offset = wa_size - 1;
   //PSX_WARNING("[SPU] A reverb offset is broken(+): WASize=0x%04x, 0x%04x.", wa_size >> 2, in_offset >> 2);
  }
 }

 offset += ReverbCur;

 if(offset >= 0x40000)
  offset = (offset & 0x3FFFF) + ReverbWA;

 assert(offset >= ReverbWA && offset < 0x40000);

 return(offset);
}

int32 PS_SPU::RD_RVB(int16 raw_offs)
{
 //raw_offs = rand() & 0xFFFF;

 return((int16)SPURAM[Get_Reverb_Offset(raw_offs << 2)]);
}

void PS_SPU::WR_RVB(int16 raw_offs, int32 sample, int32 extra_offs)
{
 //raw_offs = rand() & 0xFFFF;

 SPURAM[Get_Reverb_Offset((raw_offs << 2) + extra_offs)] = ReverbSat(sample);
}

static INLINE int32 Reverb4422(const int16 *src)
{
 static const int16 ResampTable[40] =
 {
  (int16)0xffff,
  (int16)0x0000,
  (int16)0x0002,
  (int16)0x0000,
  (int16)0xfff6,
  (int16)0x0000,
  (int16)0x0023,
  (int16)0x0000,
  (int16)0xff99,
  (int16)0x0000,
  (int16)0x010a,
  (int16)0x0000,
  (int16)0xfd98,
  (int16)0x0000,
  (int16)0x0534,
  (int16)0x0000,
  (int16)0xf470,
  (int16)0x0000,
  (int16)0x2806,
  (int16)0x4000,
  (int16)0x2806,
  (int16)0x0000,
  (int16)0xf470,
  (int16)0x0000,
  (int16)0x0534,
  (int16)0x0000,
  (int16)0xfd98,
  (int16)0x0000,
  (int16)0x010a,
  (int16)0x0000,
  (int16)0xff99,
  (int16)0x0000,
  (int16)0x0023,
  (int16)0x0000,
  (int16)0xfff6,
  (int16)0x0000,
  (int16)0x0002,
  (int16)0x0000,
  (int16)0xffff,				     
  (int16)0x0000,
 };
 int32 out = 0;	// 32-bits is adequate(it won't overflow)

 for(int i = 0; i < 40; i += 2)
  out += ResampTable[i] * src[i];

 // Middle non-zero
 out += 0x4000 * src[19];

 out >>= 15;

 if(out < -32768)
  out = -32768;

 if(out > 32767)
  out = 32767;

 return(out);
}

void PS_SPU::RunReverb(int32 in_l, int32 in_r, int32 &out_l, int32 &out_r)
{
 int32 upsampled[2] = { 0, 0 };

 RDSB[0][RDSB_WP] = in_l;
 RDSB[1][RDSB_WP] = in_r;
 RDSB[0][RDSB_WP | 0x40] = in_l;	// So we don't have to &/bounds check in our MAC loop
 RDSB[1][RDSB_WP | 0x40] = in_r;

 RDSB_WP = (RDSB_WP + 1) & 0x3F;

 if(!(RDSB_WP & 1))
 {
  int32 downsampled[2];

  for(int lr = 0; lr < 2; lr++)
   downsampled[lr] = Reverb4422(&RDSB[lr][(RDSB_WP - 40) & 0x3F]);

  //
  // Run algorithm
  ///
 if(SPUControl & 0x80)
 {
  int32 IIR_INPUT_A0;
  int32 IIR_INPUT_A1;
  int32 IIR_INPUT_B0;
  int32 IIR_INPUT_B1;
  int32 IIR_A0, IIR_A1, IIR_B0, IIR_B1;
  int32 ACC0, ACC1;
  int32 FB_A0, FB_A1, FB_B0, FB_B1;

  IIR_INPUT_A0 = ((RD_RVB(IIR_SRC_A0) * IIR_COEF) >> 15) + ((downsampled[0] * IN_COEF_L) >> 15);
  IIR_INPUT_A1 = ((RD_RVB(IIR_SRC_A1) * IIR_COEF) >> 15) + ((downsampled[1] * IN_COEF_R) >> 15);
  IIR_INPUT_B0 = ((RD_RVB(IIR_SRC_B0) * IIR_COEF) >> 15) + ((downsampled[0] * IN_COEF_L) >> 15);
  IIR_INPUT_B1 = ((RD_RVB(IIR_SRC_B1) * IIR_COEF) >> 15) + ((downsampled[1] * IN_COEF_R) >> 15);


  IIR_A0 = (((int64)IIR_INPUT_A0 * IIR_ALPHA) >> 15) + ((RD_RVB(IIR_DEST_A0) * (32768 - IIR_ALPHA)) >> 15);
  IIR_A1 = (((int64)IIR_INPUT_A1 * IIR_ALPHA) >> 15) + ((RD_RVB(IIR_DEST_A1) * (32768 - IIR_ALPHA)) >> 15);
  IIR_B0 = (((int64)IIR_INPUT_B0 * IIR_ALPHA) >> 15) + ((RD_RVB(IIR_DEST_B0) * (32768 - IIR_ALPHA)) >> 15);
  IIR_B1 = (((int64)IIR_INPUT_B1 * IIR_ALPHA) >> 15) + ((RD_RVB(IIR_DEST_B1) * (32768 - IIR_ALPHA)) >> 15);

  WR_RVB(IIR_DEST_A0, IIR_A0, 1);
  WR_RVB(IIR_DEST_A1, IIR_A1, 1);
  WR_RVB(IIR_DEST_B0, IIR_B0, 1);
  WR_RVB(IIR_DEST_B1, IIR_B1, 1);

#if 0
  ACC0 = ((RD_RVB(ACC_SRC_A0) * ACC_COEF_A) >> 15) +
         ((RD_RVB(ACC_SRC_B0) * ACC_COEF_B) >> 15) +
         ((RD_RVB(ACC_SRC_C0) * ACC_COEF_C) >> 15) +
         ((RD_RVB(ACC_SRC_D0) * ACC_COEF_D) >> 15);

  ACC1 = ((RD_RVB(ACC_SRC_A1) * ACC_COEF_A) >> 15) +
         ((RD_RVB(ACC_SRC_B1) * ACC_COEF_B) >> 15) +
         ((RD_RVB(ACC_SRC_C1) * ACC_COEF_C) >> 15) +
         ((RD_RVB(ACC_SRC_D1) * ACC_COEF_D) >> 15);
#endif

  ACC0 = ((int64)(RD_RVB(ACC_SRC_A0) * ACC_COEF_A) +
	        (RD_RVB(ACC_SRC_B0) * ACC_COEF_B) +
	        (RD_RVB(ACC_SRC_C0) * ACC_COEF_C) +
	        (RD_RVB(ACC_SRC_D0) * ACC_COEF_D)) >> 15;


  ACC1 = ((int64)(RD_RVB(ACC_SRC_A1) * ACC_COEF_A) +
	        (RD_RVB(ACC_SRC_B1) * ACC_COEF_B) +
	        (RD_RVB(ACC_SRC_C1) * ACC_COEF_C) +
	        (RD_RVB(ACC_SRC_D1) * ACC_COEF_D)) >> 15;


  FB_A0 = RD_RVB(MIX_DEST_A0 - FB_SRC_A);
  FB_A1 = RD_RVB(MIX_DEST_A1 - FB_SRC_A);
  FB_B0 = RD_RVB(MIX_DEST_B0 - FB_SRC_B);
  FB_B1 = RD_RVB(MIX_DEST_B1 - FB_SRC_B);

  WR_RVB(MIX_DEST_A0, ACC0 - ((FB_A0 * FB_ALPHA) >> 15));
  WR_RVB(MIX_DEST_A1, ACC1 - ((FB_A1 * FB_ALPHA) >> 15));

  WR_RVB(MIX_DEST_B0, (((int64)FB_ALPHA * ACC0) >> 15) - ((FB_A0 * (int16)(0x8000 ^ FB_ALPHA)) >> 15) - ((FB_B0 * FB_X) >> 15));
  WR_RVB(MIX_DEST_B1, (((int64)FB_ALPHA * ACC1) >> 15) - ((FB_A1 * (int16)(0x8000 ^ FB_ALPHA)) >> 15) - ((FB_B1 * FB_X) >> 15));
 }

  // 
  // Get output samples
  //
//  RUSB[0][RUSB_WP | 0x40] = RUSB[0][RUSB_WP] = (short)rand();
//  RUSB[1][RUSB_WP | 0x40] = RUSB[1][RUSB_WP] = (short)rand();
  RUSB[0][RUSB_WP | 0x40] = RUSB[0][RUSB_WP] = (RD_RVB(MIX_DEST_A0) + RD_RVB(MIX_DEST_B0)) >> 1;
  RUSB[1][RUSB_WP | 0x40] = RUSB[1][RUSB_WP] = (RD_RVB(MIX_DEST_A1) + RD_RVB(MIX_DEST_B1)) >> 1;

  RUSB_WP = (RUSB_WP + 1) & 0x3F;

  ReverbCur = (ReverbCur + 1) & 0x3FFFF;
  if(!ReverbCur)
   ReverbCur = ReverbWA;
 } 
 else
 {
  RUSB[0][RUSB_WP | 0x40] = RUSB[0][RUSB_WP] = 0;
  RUSB[1][RUSB_WP | 0x40] = RUSB[1][RUSB_WP] = 0;

  RUSB_WP = (RUSB_WP + 1) & 0x3F;
 }

 for(int lr = 0; lr < 2; lr++)
  upsampled[lr] = Reverb4422(&RUSB[lr][(RUSB_WP - 40) & 0x3F]);

 out_l = upsampled[0];
 out_r = upsampled[1];
}

