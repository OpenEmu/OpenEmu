  if(ch->control & 0x40) // DDA mode
  {
    #ifdef LFO_ON
    just_do_dda_lfo:
    WaveSynth.offset(timestamp, ch->dda_cache[0] - ch->last_dda_cache[0], &FXsbuf[0]);
    WaveSynth.offset(timestamp, ch->dda_cache[1] - ch->last_dda_cache[1], &FXsbuf[1]);
    ch->last_dda_cache[0] = ch->dda_cache[0];
    ch->last_dda_cache[1] = ch->dda_cache[1];
    #else
    just_do_dda:
    WaveSynth.offset(timestamp, ch->dda_cache[0] - ch->last_dda_cache[0], &FXsbuf[0]);
    WaveSynth.offset(timestamp, ch->dda_cache[1] - ch->last_dda_cache[1], &FXsbuf[1]);
    ch->last_dda_cache[0] = ch->dda_cache[0];
    ch->last_dda_cache[1] = ch->dda_cache[1];
    #endif
  }
  else if(ch->noisectrl & 0x80)
  {
   int32 freq = 0x1F - (ch->noisectrl & 0x1F);

   if(!ch->lfsr) ch->lfsr = 1;
   if(!freq)
    freq = 0x20;
   else
    freq <<= 6;

   freq <<= 1;

   ch->noisecount -= run_time;
   while(ch->noisecount <= 0)
   {
    ch->lfsr = (ch->lfsr << 1) | ((((ch->lfsr >> 15) ^ (ch->lfsr >> 14) ^ (ch->lfsr >> 12) ^ (ch->lfsr >> 3)))&1);
    ch->dda = (ch->lfsr & 0x1) ? 0x18 : 0; //ch->lfsr & 0x1F; //(ch->lfsr&1)?0x10: 0;
    redo_ddacache(ch);
    NoiseSynth.offset_inline(timestamp + ch->noisecount, ch->dda_cache[0] - ch->last_dda_cache[0], &FXsbuf[0]);
    NoiseSynth.offset_inline(timestamp + ch->noisecount, ch->dda_cache[1] - ch->last_dda_cache[1], &FXsbuf[1]);
    ch->last_dda_cache[0] = ch->dda_cache[0];
    ch->last_dda_cache[1] = ch->dda_cache[1];
    ch->noisecount += freq;
   }
  }
  else
  {
   int32 freq = ch->frequency ? ch->frequency : 4096;

   if(freq <= 5) 
   { 
    int32 samp_acc[2] = {0, 0};
    for(int x = 0; x < 0x20; x++)
    {
     ch->dda = ch->waveform[x];
     redo_ddacache(ch);
     samp_acc[0] += ch->dda_cache[0];
     samp_acc[1] += ch->dda_cache[1];
    }
    ch->dda_cache[0] = samp_acc[0] >> 5;
    ch->dda_cache[1] = samp_acc[1] >> 5;
    #ifdef LFO_ON
    goto just_do_dda_lfo;
    #else
    goto just_do_dda; 
    #endif
   }

   freq <<= 1;

   #ifdef LFO_ON
   int32 lfo_freq =  psg.channel[1].frequency;
   if(!lfo_freq) lfo_freq = 4096;
   lfo_freq *= ((psg.lfofreq & 0xFF) ? (psg.lfofreq & 0xFF) : 256);
   lfo_freq <<= 1;

   while(run_time > 0)
   #else
   ch->counter -= run_time;
   while(ch->counter <= 0)
   #endif
   {
    #ifdef LFO_ON
//	printf("Weehee %d %d %d %d\n", run_time, psg.channel[1].counter, ch->counter, ch->frequency);
    int32 tmprt = psg.channel[1].counter;
    if(tmprt > ch->counter) tmprt = ch->counter;
    if(tmprt > run_time) tmprt = run_time;
    run_time -= tmprt;
    psg.channel[1].counter -= tmprt;
    ch->counter -= tmprt;

    if(psg.channel[1].counter <= 0)
    {
     psg.channel[1].waveform_index = (psg.channel[1].waveform_index + 1) & 0x1F;

     uint32 shift = (((psg.lfoctrl & 0x3) - 1) << 1);
     uint8 la = psg.channel[1].waveform[psg.channel[1].waveform_index];

     freq = ch->base_frequency + ((la - 0x10) << shift);

     if(freq < 0) freq = 0;
     if(freq > 4095) freq = 4095;

     ch->frequency = freq;
     if(!freq)
      freq = 4096;
     freq <<= 1;

     psg.channel[1].counter += lfo_freq;
    }
    if(ch->counter > 0) continue;
    #endif

    ch->waveform_index = (ch->waveform_index + 1) & 0x1F;

    ch->dda = ch->waveform[ch->waveform_index];

    redo_ddacache(ch);
    #ifdef LFO_ON
    WaveSynth.offset(timestamp + ch->counter - run_time, ch->dda_cache[0] - ch->last_dda_cache[0], &FXsbuf[0]);
    WaveSynth.offset(timestamp + ch->counter - run_time, ch->dda_cache[1] - ch->last_dda_cache[1], &FXsbuf[1]);
    #else
    WaveSynth.offset_inline(timestamp + ch->counter, ch->dda_cache[0] - ch->last_dda_cache[0], &FXsbuf[0]);
    WaveSynth.offset_inline(timestamp + ch->counter, ch->dda_cache[1] - ch->last_dda_cache[1], &FXsbuf[1]);
    #endif
    ch->last_dda_cache[0] = ch->dda_cache[0];
    ch->last_dda_cache[1] = ch->dda_cache[1];
    ch->counter += freq;
   }
  }
