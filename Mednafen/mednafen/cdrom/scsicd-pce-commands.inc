/********************************************************
*							*
*	PC Engine CD Command 0xD8 - SAPSP		*
*							*
********************************************************/
static void DoNEC_PCE_SAPSP(const uint8 *cdb)
{
 uint32 new_read_sec_start;

 //printf("Set audio start: %02x %02x %02x %02x %02x %02x %02x\n", cdb[9], cdb[1], cdb[2], cdb[3], cdb[4], cdb[5], cdb[6]);
 switch (cdb[9] & 0xc0)
 {
  default:  SCSIDBG("Unknown SAPSP 9: %02x\n", cdb[9]);
  case 0x00:
   new_read_sec_start = (cdb[3] << 16) | (cdb[4] << 8) | cdb[5];
   break;

  case 0x40:
   new_read_sec_start = msf2lba(BCD_TO_INT(cdb[2]), BCD_TO_INT(cdb[3]), BCD_TO_INT(cdb[4]));
   break;

  case 0x80:
   {
    int track = BCD_TO_INT(cdb[2]);

    if(!track)
     track = 1;
    else if(track == toc.last_track + 1)
     track = 100;
    else if(track > toc.last_track)
    {
     CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_END_OF_VOLUME);
     return;
    }
    new_read_sec_start = toc.tracks[track].lba;
   }
   break;
 }

 //printf("%lld\n", (long long)(monotonic_timestamp - pce_lastsapsp_timestamp) * 1000 / System_Clock);
 if(cdda.CDDAStatus == CDDASTATUS_PLAYING && new_read_sec_start == read_sec_start && ((int64)(monotonic_timestamp - pce_lastsapsp_timestamp) * 1000 / System_Clock) < 190)
 {
  pce_lastsapsp_timestamp = monotonic_timestamp;

  SendStatusAndMessage(STATUS_GOOD, 0x00);
  CDIRQCallback(SCSICD_IRQ_DATA_TRANSFER_DONE);
  return;
 }

 pce_lastsapsp_timestamp = monotonic_timestamp;

 read_sec = read_sec_start = new_read_sec_start;
 read_sec_end = toc.tracks[100].lba;


 cdda.CDDAReadPos = 588;

 cdda.CDDAStatus = CDDASTATUS_PAUSED;
 cdda.PlayMode = PLAYMODE_SILENT;

 if(cdb[1])
 {
  cdda.PlayMode = PLAYMODE_NORMAL;
  cdda.CDDAStatus = CDDASTATUS_PLAYING;
 }

 if(read_sec < toc.tracks[100].lba)
  CDIF_HintReadSector(read_sec);

 SendStatusAndMessage(STATUS_GOOD, 0x00);
 CDIRQCallback(SCSICD_IRQ_DATA_TRANSFER_DONE);
}



/********************************************************
*							*
*	PC Engine CD Command 0xD9 - SAPEP		*
*							*
********************************************************/
static void DoNEC_PCE_SAPEP(const uint8 *cdb)
{
 uint32 new_read_sec_end;

 //printf("Set audio end: %02x %02x %02x %02x %02x %02x %02x\n", cdb[9], cdb[1], cdb[2], cdb[3], cdb[4], cdb[5], cdb[6]);

 switch (cdb[9] & 0xc0)
 {
  default: SCSIDBG("Unknown SAPEP 9: %02x\n", cdb[9]);

  case 0x00:
   new_read_sec_end = (cdb[3] << 16) | (cdb[4] << 8) | cdb[5];
   break;

  case 0x40:
   new_read_sec_end = BCD_TO_INT(cdb[4]) + 75 * (BCD_TO_INT(cdb[3]) + 60 * BCD_TO_INT(cdb[2]));
   new_read_sec_end -= 150;
   break;

  case 0x80:
   {
    int track = BCD_TO_INT(cdb[2]);

    if(!track)
     track = 1;
    else if(track == toc.last_track + 1)
     track = 100;
    else if(track > toc.last_track)
    {
     CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_END_OF_VOLUME);
     return;
    }
    new_read_sec_end = toc.tracks[track].lba;
   }
   break;
 }

 read_sec_end = new_read_sec_end;

 switch(cdb[1])	// PCE CD(TODO: Confirm these, and check the mode mask):
 {
	default:
	case 0x03: cdda.PlayMode = PLAYMODE_NORMAL;
		   cdda.CDDAStatus = CDDASTATUS_PLAYING;
		   break;

	case 0x02: cdda.PlayMode = PLAYMODE_INTERRUPT;
		   cdda.CDDAStatus = CDDASTATUS_PLAYING;
		   break;

	case 0x01: cdda.PlayMode = PLAYMODE_LOOP;
		   cdda.CDDAStatus = CDDASTATUS_PLAYING;
		   break;

	case 0x00: cdda.PlayMode = PLAYMODE_SILENT;
		   cdda.CDDAStatus = CDDASTATUS_STOPPED;
		   break;
 }

 SendStatusAndMessage(STATUS_GOOD, 0x00);
}



/********************************************************
*							*
*	PC Engine CD Command 0xDA - Pause		*
*							*
********************************************************/
static void DoNEC_PCE_PAUSE(const uint8 *cdb)
{
 if(cdda.CDDAStatus != CDDASTATUS_STOPPED) // Hmm, should we give an error if it tries to pause and it's already paused?
 {
  cdda.CDDAStatus = CDDASTATUS_PAUSED;
  SendStatusAndMessage(STATUS_GOOD, 0x00);
 }
 else // Definitely give an error if it tries to pause when no track is playing!
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_AUDIO_NOT_PLAYING);
 }
}



/********************************************************
*							*
*	PC Engine CD Command 0xDD - Read Subchannel Q	*
*							*
********************************************************/
static void DoNEC_PCE_READSUBQ(const uint8 *cdb)
{
 uint8 *SubQBuf = cd.SubQBuf[QMode_Time];
 uint8 data_in[8192];

 memset(data_in, 0x00, 10);

 data_in[2] = SubQBuf[1];     // Track
 data_in[3] = SubQBuf[2];     // Index
 data_in[4] = SubQBuf[3];     // M(rel)
 data_in[5] = SubQBuf[4];     // S(rel)
 data_in[6] = SubQBuf[5];     // F(rel)
 data_in[7] = SubQBuf[7];     // M(abs)
 data_in[8] = SubQBuf[8];     // S(abs)
 data_in[9] = SubQBuf[9];     // F(abs)

 if(cdda.CDDAStatus == CDDASTATUS_PAUSED)
  data_in[0] = 2;		// Pause
 else if(cdda.CDDAStatus == CDDASTATUS_PLAYING || cdda.CDDAStatus == CDDASTATUS_SCANNING) // FIXME:  Is this the correct status code for scanning playback?
  data_in[0] = 0;		// Playing
 else
  data_in[0] = 3;		// Stopped

 DoSimpleDataIn(data_in, 10);
}



/********************************************************
*							*
*	PC Engine CD Command 0xDE - Get Directory Info	*
*							*
********************************************************/
static void DoNEC_PCE_GETDIRINFO(const uint8 *cdb)
{
 // Problems:
 //	Returned data lengths on real PCE are not confirmed.
 //	Mode 0x03 behavior not tested on real PCE

 uint8 data_in[2048];
 uint32 data_in_size = 0;

 memset(data_in, 0, sizeof(data_in));

 switch(cdb[1])
 {
  default: MDFN_DispMessage("Unknown GETDIRINFO Mode: %02x", cdb[1]);
	   printf("Unknown GETDIRINFO Mode: %02x", cdb[1]);
  case 0x0:
   data_in[0] = INT_TO_BCD(toc.first_track);
   data_in[1] = INT_TO_BCD(toc.last_track);

   data_in_size = 2;
   break;

  case 0x1:
   {
    uint8 m, s, f;

    lba2msf(toc.tracks[100].lba, &m, &s, &f);

    data_in[0] = INT_TO_BCD(m);
    data_in[1] = INT_TO_BCD(s);
    data_in[2] = INT_TO_BCD(f);

    data_in_size = 3;
   }
   break;

  case 0x2:
   {
    uint8 m, s, f;
    int track = BCD_TO_INT(cdb[2]);

    if(!track)
     track = 1;
    else if(cdb[2] == 0xAA)
    {
     track = 100;
    }
    else if(track > 99)
    {
     CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
     return;
    }

    lba2msf(toc.tracks[track].lba, &m, &s, &f);

    data_in[0] = INT_TO_BCD(m);
    data_in[1] = INT_TO_BCD(s);
    data_in[2] = INT_TO_BCD(f);
    data_in[3] = toc.tracks[track].control;
    data_in_size = 4;
   }
   break;
 }

 DoSimpleDataIn(data_in, data_in_size);
}

