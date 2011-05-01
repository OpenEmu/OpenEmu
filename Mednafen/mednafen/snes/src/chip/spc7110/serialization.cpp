#ifdef SPC7110_CPP

void SPC7110Decomp::serialize(serializer &s) {
  s.integer(decomp_mode);
  s.integer(decomp_offset);

  s.array(decomp_buffer, decomp_buffer_size);
  s.integer(decomp_buffer_rdoffset);
  s.integer(decomp_buffer_wroffset);
  s.integer(decomp_buffer_length);

  for(unsigned n = 0; n < 32; n++) {
    s.integer(context[n].index);
    s.integer(context[n].invert);
  }
}

void SPC7110::serialize(serializer &s) {
  s.integer(r4801);
  s.integer(r4802);
  s.integer(r4803);
  s.integer(r4804);
  s.integer(r4805);
  s.integer(r4806);
  s.integer(r4807);
  s.integer(r4808);
  s.integer(r4809);
  s.integer(r480a);
  s.integer(r480b);
  s.integer(r480c);
  decomp.serialize(s);

  s.integer(r4811);
  s.integer(r4812);
  s.integer(r4813);
  s.integer(r4814);
  s.integer(r4815);
  s.integer(r4816);
  s.integer(r4817);
  s.integer(r4818);
  s.integer(r481x);
  s.integer(r4814_latch);
  s.integer(r4815_latch);

  s.integer(r4820);
  s.integer(r4821);
  s.integer(r4822);
  s.integer(r4823);
  s.integer(r4824);
  s.integer(r4825);
  s.integer(r4826);
  s.integer(r4827);
  s.integer(r4828);
  s.integer(r4829);
  s.integer(r482a);
  s.integer(r482b);
  s.integer(r482c);
  s.integer(r482d);
  s.integer(r482e);
  s.integer(r482f);

  s.integer(r4830);
  s.integer(r4831);
  s.integer(r4832);
  s.integer(r4833);
  s.integer(r4834);

  s.integer(dx_offset);
  s.integer(ex_offset);
  s.integer(fx_offset);

  s.integer(r4840);
  s.integer(r4841);
  s.integer(r4842);

  s.integer(rtc_state);
  s.integer(rtc_mode);
  s.integer(rtc_index);
}

#endif
