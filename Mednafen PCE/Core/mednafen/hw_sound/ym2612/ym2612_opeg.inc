class OpEG
{
 public:

 OpEG();
 ~OpEG();

 void serialize(MDFN::LEPacker &slizer, bool load);

 void Reset(void);

 void SetKONOFF(bool);
 void SetCSMKONOFF(bool);
 void InstrParamChanged(void);

 bool Run(bool div3_run_eg, unsigned GlobalEGCycleCounter, unsigned ksr);

 void SetTL(unsigned);
 void SetSL(unsigned);
 void SetAR(unsigned);
 void SetDR(unsigned);
 void SetSR(unsigned);
 void SetRR(unsigned);
 void SetSSG_EG(unsigned);

 uint32 GetOutAttenuation(void);

 //
 //
 //
 private:

 void CheckPhaseAdvance(void);
 unsigned CalcRate(unsigned ksr);

 bool NeedFullKON;
 bool KONOFF;
 bool CSMKONOFF;
 bool LA_OT_KONOFF;
 uint8 TL, SL;
 uint8 SSG_EG;

 union
 {
  struct
  {
   uint8 AR, DR, SR, RR;	// Note: RR is stored (<< 1) | 1 here
  };
  uint8 RawRates[4];
 };

 unsigned CurPhase;
 signed Attenuation;
 bool SSGInvert;

 enum
 {
  ADSR_ATTACK = 0,
  ADSR_DECAY = 1,
  ADSR_SUSTAIN = 2,
  ADSR_RELEASE = 3
 };
 #define SSGEG_MASK_HOLD	0x01
 #define SSGEG_MASK_ALTERNATE	0x02
 #define SSGEG_MASK_ATTACK	0x04
 #define SSGEG_MASK_ENABLE	0x08
};

OpEG::OpEG()
{
 Reset();
}

OpEG::~OpEG()
{


}

void OpEG::serialize(MDFN::LEPacker &slizer, bool load)
{
 slizer ^ KONOFF;
 slizer ^ CSMKONOFF;
 slizer ^ LA_OT_KONOFF;
 slizer ^ NeedFullKON;
 slizer ^ TL;
 slizer ^ SL;
 slizer ^ SSG_EG;

 slizer ^ AR;
 slizer ^ DR;
 slizer ^ SR;
 slizer ^ RR;

 slizer ^ CurPhase;
 slizer ^ Attenuation;
 slizer ^ SSGInvert;
}

void OpEG::Reset(void)
{
 TL = 0;
 SL = 0;
 SSG_EG = 0;

 AR = 0;
 DR = 0;
 SR = 0;
 RR = 1;

 CurPhase = ADSR_RELEASE;
 Attenuation = 0x3FF;
 SSGInvert = false;
 KONOFF = false;
 CSMKONOFF = false;
 LA_OT_KONOFF = false;

 NeedFullKON = false;
}


void OpEG::InstrParamChanged(void)
{
 NeedFullKON = true;
}

void OpEG::SetCSMKONOFF(bool ns)
{
 CSMKONOFF = ns;
}

void OpEG::SetKONOFF(bool ns)
{
 // printf("NS: %d\n", ns);
 //if(LA_OT_KONOFF != (KONOFF | CSMKONOFF))
 // puts("IYEE :(\n\n");

 KONOFF = ns;
}


void OpEG::SetTL(unsigned v)
{
 TL = v & 0x7F;
}

void OpEG::SetSL(unsigned v)
{
 SL = v & 0x0F;
}

void OpEG::SetAR(unsigned v)
{
 AR = v & 0x1F;
 //if(LA_OT_KONOFF != (KONOFF | CSMKONOFF) || CurPhase != ADSR_RELEASE)
 // puts("AR Evil");
}

void OpEG::SetDR(unsigned v)
{
 DR = v & 0x1F;
 //if(LA_OT_KONOFF != (KONOFF | CSMKONOFF) || CurPhase != ADSR_RELEASE)
 // puts("DR Evil");
}

void OpEG::SetSR(unsigned v)
{
 SR = v & 0x1F;
 //if(LA_OT_KONOFF != (KONOFF | CSMKONOFF) || CurPhase != ADSR_RELEASE)
 // puts("SR Evil");
}

void OpEG::SetRR(unsigned v)
{
 RR = ((v & 0x0F) << 1) | 1;
 //if(LA_OT_KONOFF != (KONOFF | CSMKONOFF) || CurPhase != ADSR_RELEASE)
 // puts("RR Evil");
}

void OpEG::SetSSG_EG(unsigned v)
{
 SSG_EG = v & 0xF;
}

unsigned OpEG::CalcRate(unsigned ksr)
{
 unsigned int rate = ((RawRates[CurPhase] << 1) + (RawRates[CurPhase] ? ksr : 0));

 if(rate > 0x3F)
  rate = 0x3F; 

 return(rate);
}


// See Mega Turrican track 3, and Flashback track 3.
void OpEG::CheckPhaseAdvance(void)
{
 if(CurPhase == ADSR_ATTACK)
 {
  if(Attenuation == 0)
  {
   CurPhase = ADSR_DECAY;
  }
 }


 if(CurPhase == ADSR_DECAY)
 {
  if(Attenuation >= ((SL == 0x0F) ? 0x3FF : (SL << 5)))
  {
   CurPhase = ADSR_SUSTAIN;
  }
 }
}

// 
bool OpEG::Run(bool div3_run_eg, unsigned GlobalEGCycleCounter, unsigned ksr)
{
 bool ret = true;
 bool combo_KONOFF = KONOFF | CSMKONOFF;

 if(combo_KONOFF != LA_OT_KONOFF)
 {
  if(combo_KONOFF)	// Key on
  {
   //printf("Key on\n");
   ret = false;

   CurPhase = ADSR_ATTACK;
   SSGInvert = false;

   if(NeedFullKON)
   {
    Attenuation = 0x3FF;
    NeedFullKON = false;
   }

   if(CalcRate(ksr) >= 0x3E)
   {
    Attenuation = 0;
   }
   CheckPhaseAdvance();
  }
  else	// Key off
  {
   //printf("Key off\n");
   CurPhase = ADSR_RELEASE;

   if((SSG_EG & 0x08) && (SSGInvert ^ (bool)(SSG_EG & SSGEG_MASK_ATTACK)))
   {
    Attenuation = (0x200 - Attenuation) & 0x3FF;
   }
  }
 }
 LA_OT_KONOFF = combo_KONOFF;

 if((SSG_EG & 0x08) && (Attenuation >= 0x200))
 {
  if((SSG_EG & SSGEG_MASK_ALTERNATE) && (!(SSG_EG & SSGEG_MASK_HOLD) || !SSGInvert))
  {
   SSGInvert = !SSGInvert;
  }

  if(!(SSG_EG & SSGEG_MASK_ALTERNATE) && !(SSG_EG & SSGEG_MASK_HOLD))
  {
   ret = false;
  }

  if(CurPhase != ADSR_ATTACK)
  {
   if((CurPhase != ADSR_RELEASE) && !(SSG_EG & SSGEG_MASK_HOLD))
   {
    ret = false;

    CurPhase = ADSR_ATTACK;

    //Attenuation = 0x3FF;
    if(NeedFullKON)
    {
     Attenuation = 0x3FF;
     NeedFullKON = false;
    }

    if(CalcRate(ksr) >= 0x3E)
    {
     Attenuation = 0;
    }
    CheckPhaseAdvance();
   }
   else if((CurPhase == ADSR_RELEASE) || !(SSGInvert ^ (bool)(SSG_EG & SSGEG_MASK_ATTACK)))
   {
    Attenuation = 0x3FF;
    CheckPhaseAdvance();
   }
  }
 }


 if(div3_run_eg)
 {
  static const uint8 counter_shift_table[0x40] = {
	11,   11,   11,   11,      // 0-3    (0x00-0x03)
	10,   10,   10,   10,      // 4-7    (0x04-0x07)
	 9,    9,    9,    9,      // 8-11   (0x08-0x0B)
	 8,    8,    8,    8,      // 12-15  (0x0C-0x0F)
	 7,    7,    7,    7,      // 16-19  (0x10-0x13)
	 6,    6,    6,    6,      // 20-23  (0x14-0x17)
	 5,    5,    5,    5,      // 24-27  (0x18-0x1B)
	 4,    4,    4,    4,      // 28-31  (0x1C-0x1F)
	 3,    3,    3,    3,      // 32-35  (0x20-0x23)
	 2,    2,    2,    2,      // 36-39  (0x24-0x27)
	 1,    1,    1,    1,      // 40-43  (0x28-0x2B)
	 0,    0,    0,    0,      // 44-47  (0x2C-0x2F)
	 0,    0,    0,    0,      // 48-51  (0x30-0x33)
	 0,    0,    0,    0,      // 52-55  (0x34-0x37)
	 0,    0,    0,    0,      // 56-59  (0x38-0x3B)
	 0,    0,    0,    0,      // 60-63  (0x3C-0x3F)
	};

  static const uint8 atten_inc_table[0x40][0x08] = {
	{ 0,0,0,0,0,0,0,0 }, { 0,0,0,0,0,0,0,0 }, { 0,1,0,1,0,1,0,1 }, { 0,1,0,1,0,1,0,1 }, //  0-3    (0x00-0x03)
 	{ 0,1,0,1,0,1,0,1 }, { 0,1,0,1,0,1,0,1 }, { 0,1,1,1,0,1,1,1 }, { 0,1,1,1,0,1,1,1 }, //  4-7    (0x04-0x07)
	{ 0,1,0,1,0,1,0,1 }, { 0,1,0,1,1,1,0,1 }, { 0,1,1,1,0,1,1,1 }, { 0,1,1,1,1,1,1,1 }, //  8-11   (0x08-0x0B)
	{ 0,1,0,1,0,1,0,1 }, { 0,1,0,1,1,1,0,1 }, { 0,1,1,1,0,1,1,1 }, { 0,1,1,1,1,1,1,1 }, //  12-15  (0x0C-0x0F)
	{ 0,1,0,1,0,1,0,1 }, { 0,1,0,1,1,1,0,1 }, { 0,1,1,1,0,1,1,1 }, { 0,1,1,1,1,1,1,1 }, //  16-19  (0x10-0x13)
	{ 0,1,0,1,0,1,0,1 }, { 0,1,0,1,1,1,0,1 }, { 0,1,1,1,0,1,1,1 }, { 0,1,1,1,1,1,1,1 }, //  20-23  (0x14-0x17)
	{ 0,1,0,1,0,1,0,1 }, { 0,1,0,1,1,1,0,1 }, { 0,1,1,1,0,1,1,1 }, { 0,1,1,1,1,1,1,1 }, //  24-27  (0x18-0x1B)
	{ 0,1,0,1,0,1,0,1 }, { 0,1,0,1,1,1,0,1 }, { 0,1,1,1,0,1,1,1 }, { 0,1,1,1,1,1,1,1 }, //  28-31  (0x1C-0x1F)
	{ 0,1,0,1,0,1,0,1 }, { 0,1,0,1,1,1,0,1 }, { 0,1,1,1,0,1,1,1 }, { 0,1,1,1,1,1,1,1 }, //  32-35  (0x20-0x23)
	{ 0,1,0,1,0,1,0,1 }, { 0,1,0,1,1,1,0,1 }, { 0,1,1,1,0,1,1,1 }, { 0,1,1,1,1,1,1,1 }, //  36-39  (0x24-0x27)
	{ 0,1,0,1,0,1,0,1 }, { 0,1,0,1,1,1,0,1 }, { 0,1,1,1,0,1,1,1 }, { 0,1,1,1,1,1,1,1 }, //  40-43  (0x28-0x2B)
	{ 0,1,0,1,0,1,0,1 }, { 0,1,0,1,1,1,0,1 }, { 0,1,1,1,0,1,1,1 }, { 0,1,1,1,1,1,1,1 }, //  44-47  (0x2C-0x2F)
	{ 1,1,1,1,1,1,1,1 }, { 1,1,1,2,1,1,1,2 }, { 1,2,1,2,1,2,1,2 }, { 1,2,2,2,1,2,2,2 }, //  48-51  (0x30-0x33)
	{ 2,2,2,2,2,2,2,2 }, { 2,2,2,4,2,2,2,4 }, { 2,4,2,4,2,4,2,4 }, { 2,4,4,4,2,4,4,4 }, //  52-55  (0x34-0x37)
	{ 4,4,4,4,4,4,4,4 }, { 4,4,4,8,4,4,4,8 }, { 4,8,4,8,4,8,4,8 }, { 4,8,8,8,4,8,8,8 }, //  56-59  (0x38-0x3B)
	{ 8,8,8,8,8,8,8,8 }, { 8,8,8,8,8,8,8,8 }, { 8,8,8,8,8,8,8,8 }, { 8,8,8,8,8,8,8,8 }, //  60-63  (0x3C-0x3F)
	};

  unsigned rate = CalcRate(ksr);

  if((GlobalEGCycleCounter & ((1 << counter_shift_table[rate]) - 1)) == 0)
  {
   unsigned sub_cycle = (GlobalEGCycleCounter >> counter_shift_table[rate]) & 0x07;
   signed inc_amount = atten_inc_table[rate][sub_cycle];

   if(CurPhase == ADSR_ATTACK)
   {
    if(rate < 0x3E)
    {    
     Attenuation += ((~Attenuation) * inc_amount) >> 4;
     if(Attenuation < 0)
     {
      //printf("%d\n", Attenuation);	
      Attenuation = 0;
     }
    }
   }
   else
   {
    if(SSG_EG & 0x8)
    {
     if(Attenuation < 0x200)
     {
      Attenuation += inc_amount * 4;
     }
    }
    else
     Attenuation += inc_amount;
   }

   if(Attenuation > 0x3FF)
    Attenuation = 0x3FF;

   CheckPhaseAdvance();
  }
 }

 return(ret);
}


uint32 OpEG::GetOutAttenuation(void)
{
 unsigned ret;

 if((SSG_EG & 0x08) && (CurPhase != ADSR_RELEASE) && (SSGInvert ^ (bool)(SSG_EG & SSGEG_MASK_ATTACK)))
 {
  ret = ((0x200 - Attenuation) & 0x3FF);
 }
 else
  ret = Attenuation & 0x3FF;

 ret += TL << 3;

 if(ret > 0x3FF)
  ret = 0x3FF;

 return(ret);
}
