#ifdef DSP_CPP

void DSP::serialize(serializer &s) {
  Thread::serialize(s);

  s.array(state.regs, 128);
  state.echo_hist[0].serialize(s);
  state.echo_hist[1].serialize(s);
  s.integer(state.echo_hist_pos);

  s.integer(state.every_other_sample);
  s.integer(state.kon);
  s.integer(state.noise);
  s.integer(state.counter);
  s.integer(state.echo_offset);
  s.integer(state.echo_length);

  s.integer(state.new_kon);
  s.integer(state.endx_buf);
  s.integer(state.envx_buf);
  s.integer(state.outx_buf);

  s.integer(state.t_pmon);
  s.integer(state.t_non);
  s.integer(state.t_eon);
  s.integer(state.t_dir);
  s.integer(state.t_koff);

  s.integer(state.t_brr_next_addr);
  s.integer(state.t_adsr0);
  s.integer(state.t_brr_header);
  s.integer(state.t_brr_byte);
  s.integer(state.t_srcn);
  s.integer(state.t_esa);
  s.integer(state.t_echo_disabled);

  s.integer(state.t_dir_addr);
  s.integer(state.t_pitch);
  s.integer(state.t_output);
  s.integer(state.t_looped);
  s.integer(state.t_echo_ptr);

  s.integer(state.t_main_out[0]);
  s.integer(state.t_main_out[1]);
  s.integer(state.t_echo_out[0]);
  s.integer(state.t_echo_out[1]);
  s.integer(state.t_echo_in [0]);
  s.integer(state.t_echo_in [1]);

  for(unsigned n = 0; n < 8; n++) {
    voice[n].buffer.serialize(s);
    s.integer(voice[n].buf_pos);
    s.integer(voice[n].interp_pos);
    s.integer(voice[n].brr_addr);
    s.integer(voice[n].brr_offset);
    s.integer(voice[n].vbit);
    s.integer(voice[n].vidx);
    s.integer(voice[n].kon_delay);
    s.integer(voice[n].env_mode);
    s.integer(voice[n].env);
    s.integer(voice[n].t_envx_out);
    s.integer(voice[n].hidden_env);
  }
}

#endif
