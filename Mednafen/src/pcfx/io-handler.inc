static uint8 MDFN_FASTCALL port_rbyte(v810_timestamp_t &timestamp, uint32 A)
{
 if(A >= 0x000 && A <= 0x0FF)
  return(FXINPUT_Read8(A, timestamp));
 else if(A >= 0x100 && A <= 0x1FF) // SOUNDBOX dummy
 {
  timestamp += 4;
 }
 else if(A >= 0x200 && A <= 0x2FF) // RAINBOW dummy
 {
  timestamp += 4;
 }
 else if(A >= 0x300 && A <= 0x3FF) // FXVCE
 {
  timestamp += 4;
  return(FXVCE_Read16(A));
 }
 else if(A >= 0x400 && A <= 0x5FF) // 0x400-0x4FF: VDC-A ; 0x500-0x5FF: VDC-B
 {
  timestamp += 4;
  return(fx_vdc_chips[(A >> 8) & 0x1]->Read16((A & 4) >> 2));
 }
 else if(A >= 0x600 && A <= 0x6FF)
 {
  timestamp += 4;
  return(KING_Read8(timestamp, A));
 }
 else if(A >= 0x700 && A <= 0x7FF)
 {
  if(!(A & 1))
  {
   FXDBG("ExBusReset B Read");
   return(ExBusReset);
  }
  return(0);
 }
 else if(A >= 0xc00 && A <= 0xCFF) // Backup memory control
 {
  switch(A & 0xC0)
  {
   case 0x80: return(BackupControl);
   case 0x00: return(Last_VDC_AR[0]);
   case 0x40: return(Last_VDC_AR[1]);
  }
 }
 else if(A >= 0xe00 && A <= 0xeff)
 {
  return(PCFXIRQ_Read8(A));
 }
 else if(A >= 0xf00 && A <= 0xfff)
 {
  return(FXTIMER_Read8(A, timestamp));
 }
 else if((A & 0x7FFFFFFF) >= 0x500000 && (A & 0x7FFFFFFF) <= 0x52ffff)
 {
  if(WantHuC6273)
   return(HuC6273_Read8(A));
 }
 else if(FXSCSIROM && A >= 0x780000 && A <= 0x7FFFFF)
 {
  return(FXSCSIROM[A & 0x7FFFF]);
 }
 else if(FXSCSIROM && A >= 0x600000 && A <= 0x6FFFFF)
 {
  return(FXSCSI_CtrlRead(A));
 }
 FXDBG("Unknown 8-bit port read: %08x", A);

 return(0x00);
}

static uint16 MDFN_FASTCALL port_rhword(v810_timestamp_t &timestamp, uint32 A)
{
 if(A >= 0x000 && A <= 0x0FF)
  return(FXINPUT_Read16(A, timestamp));
 else if(A >= 0x100 && A <= 0x1FF) // SOUNDBOX dummy
 {
  timestamp += 4;
 }
 else if(A >= 0x200 && A <= 0x2FF) // RAINBOW dummy
 {
  timestamp += 4;
 }
 else if(A >= 0x300 && A <= 0x3FF)
 {
  timestamp += 4;
  return(FXVCE_Read16(A));
 }
 else if(A >= 0x400 && A <= 0x5FF) // 0x400-0x4FF: VDC-A ; 0x500-0x5FF: VDC-B
 {
  timestamp += 4;
  return(fx_vdc_chips[(A >> 8) & 0x1]->Read16((A & 4) >> 2));
 }
 else if(A >= 0x600 && A <= 0x6FF)
 {
  timestamp += 4;
  return(KING_Read16(timestamp, A));
 }
 else if(A >= 0x700 && A <= 0x7FF)
 {
  FXDBG("ExBusReset H Read");

  return(ExBusReset);
 }
 else if(A >= 0xc00 && A <= 0xCFF) // Backup memory control
 {
  switch(A & 0xC0)
  {
   case 0x80: return(BackupControl);
   case 0x00: return(Last_VDC_AR[0]);
   case 0x40: return(Last_VDC_AR[1]);
  }
 }
 else if(A >= 0xe00 && A <= 0xeff)
 {
  return(PCFXIRQ_Read16(A));
 }
 else if(A >= 0xf00 && A <= 0xfff)
 {
  return(FXTIMER_Read16(A, timestamp));
 }
 else if((A & 0x7FFFFFFF) >= 0x500000 && (A & 0x7FFFFFFF) <= 0x52ffff)
 {
  if(WantHuC6273)
   return(HuC6273_Read16(A));
 }
 else if(FXSCSIROM && A >= 0x780000 && A <= 0x7FFFFF)
 {
  return(le16toh(*(uint16*)&FXSCSIROM[A & 0x7FFFF]));
 }
 else if(FXSCSIROM && A >= 0x600000 && A <= 0x6FFFFF)
 {
  puts("FXSCSI 16-bit:");
  return(FXSCSI_CtrlRead(A));
 }

 FXDBG("Unknown 16-bit port read: %08x", A);

 return(0x00);
}

static void MDFN_FASTCALL port_wbyte(v810_timestamp_t &timestamp, uint32 A, uint8 V)
{
  if(A >= 0x000 && A <= 0x0FF)
   FXINPUT_Write8(A, V, timestamp);
  else if(A >= 0x100 && A <= 0x1FF)
  {
   timestamp += 2;
   SoundBox_Write(A, V, timestamp);
  }
  else if(A >= 0x200 && A <= 0x2FF)
  {
   timestamp += 2;
   RAINBOW_Write8(A, V);
  }
  else if(A >= 0x300 && A <= 0x3FF) // FXVCE
  {
   timestamp += 2;
   FXVCE_Write16(A, V);
  }
  else if(A >= 0x400 && A <= 0x5FF) // 0x400-0x4FF: VDC-A ; 0x500-0x5FF: VDC-B
  {
   timestamp += 2;

   if(!(A & 4))
    Last_VDC_AR[(A >> 8) & 0x1] = V;

   fx_vdc_chips[(A >> 8) & 0x1]->Write16((A & 4) >> 2, V);
  }
  else if(A >= 0x600 && A <= 0x6FF)
  {
   timestamp += 2;
   KING_Write8(timestamp, A, V);
  }
  else if(A >= 0x700 && A <= 0x7FF)
  {
   if(!(A & 1))
   {
    FXDBG("ExBusReset B Write: %02x", V & 1);
    ExBusReset = V & 1;
   }
  }
  else if(A >= 0xc00 && A <= 0xCFF)
  {
   switch(A & 0xC1)
   {
    case 0x80: BackupControl = V & 0x3;
               break;

    default:   FXDBG("Port 8-bit write: %08x %02x", A, V);
               break;
   }
  }
  else if(A >= 0xe00 && A <= 0xeff)
  {
   FXDBG("IRQ write8: %08x %02x", A, V);
   PCFXIRQ_Write16(A, V);
  }
  else if((A & 0x7FFFFFFF) >= 0x500000 && (A & 0x7FFFFFFF) <= 0x52ffff)
  {
   if(WantHuC6273)
    HuC6273_Write16(A, V);
  }
  else if(FXSCSIROM && A >= 0x600000 && A <= 0x6FFFFF)
  {
   FXSCSI_CtrlWrite(A, V);
  }
  else
  {
   FXDBG("Port 8-bit write: %08x %02x", A, V);
  }
}

static void MDFN_FASTCALL port_whword(v810_timestamp_t &timestamp, uint32 A, uint16 V)
{
  if(A >= 0x000 && A <= 0x0FF)
   FXINPUT_Write16(A, V, timestamp);
  else if(A >= 0x100 && A <= 0x1FF)
  {
   timestamp += 2;
   SoundBox_Write(A, V, timestamp);
  }
  else if(A >= 0x200 && A <= 0x2FF)
  {
   timestamp += 2;
   RAINBOW_Write16(A, V);
  }
  else if(A >= 0x300 && A <= 0x3FF)
  {
   timestamp += 2;
   FXVCE_Write16(A, V);
  }
  else if(A >= 0x400 && A <= 0x5FF) // 0x400-0x4FF: VDC-A ; 0x500-0x5FF: VDC-B
  {
   timestamp += 2;

   if(!(A & 4))
    Last_VDC_AR[(A >> 8) & 0x1] = V;

   fx_vdc_chips[(A >> 8) & 0x1]->Write16((A & 4) >> 2, V);
  }
  else if(A >= 0x600 && A <= 0x6FF)
  {
   timestamp += 2;
   KING_Write16(timestamp, A, V);
  }
  else if(A >= 0x700 && A <= 0x7FF)
  {
   ExBusReset = V & 1;
   FXDBG("ExBusReset H Write: %04x", V);
  }
  else if(A >= 0x800 && A <= 0x8FF) // ?? LIP writes here
  {
   FXDBG("Port 16-bit write: %08x %04x", A, V);
  }
  else if(A >= 0xc00 && A <= 0xCFF)
  {
   switch(A & 0xC0)
   {
    case 0x80: BackupControl = V & 0x3; 
	       break;

    default:   FXDBG("Port 16-bit write: %08x %04x", A, V);
	       break;
   }
  }
  else if(A >= 0xe00 && A <= 0xeff)
  { 
   PCFXIRQ_Write16(A, V);
  }
  else if(A >= 0xF00 && A <= 0xFFF)
  {
   FXTIMER_Write16(A, V, timestamp);
  }
  else if((A & 0x7FFFFFFF) >= 0x500000 && (A & 0x7FFFFFFF) <= 0x52ffff)
  {
   if(WantHuC6273)
    HuC6273_Write16(A, V);
  }
  else if(FXSCSIROM && A >= 0x600000 && A <= 0x6FFFFF)
  {
   puts("FXSCSI 16-bit:");
   FXSCSI_CtrlWrite(A, V);
  }
  else
  {
   FXDBG("Port 16-bit write: %08x %04x", A, V);
  }
}

