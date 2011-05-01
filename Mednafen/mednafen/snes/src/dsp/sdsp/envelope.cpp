#ifdef SDSP_CPP

void sDSP::envelope_run(voice_t &v) {
  int env = v.env;

  if(v.env_mode == env_release) { //60%
    env -= 0x8;
    if(env < 0) env = 0;
    v.env = env;
    return;
  }

  int rate;
  int env_data = VREG(adsr1);
  if(state.t_adsr0 & 0x80) { //99% ADSR
    if(v.env_mode >= env_decay) { //99%
      env--;
      env -= env >> 8;
      rate = env_data & 0x1f;
      if(v.env_mode == env_decay) { //1%
        rate = ((state.t_adsr0 >> 3) & 0x0e) + 0x10;
      }
    } else { //env_attack
      rate = ((state.t_adsr0 & 0x0f) << 1) + 1;
      env += rate < 31 ? 0x20 : 0x400;
    }
  } else { //GAIN
    env_data = VREG(gain);
    int mode = env_data >> 5;
    if(mode < 4) { //direct
      env = env_data << 4;
      rate = 31;
    } else {
      rate = env_data & 0x1f;
      if(mode == 4) { //4: linear decrease
        env -= 0x20;
      } else if(mode < 6) { //5: exponential decrease
        env--;
        env -= env >> 8;
      } else { //6, 7: linear increase
        env += 0x20;
        if(mode > 6 && (unsigned)v.hidden_env >= 0x600) {
          env += 0x8 - 0x20; //7: two-slope linear increase
        }
      }
    }
  }

  //sustain level
  if((env >> 8) == (env_data >> 5) && v.env_mode == env_decay) v.env_mode = env_sustain;
  v.hidden_env = env;

  //unsigned cast because linear decrease underflowing also triggers this
  if((unsigned)env > 0x7ff) {
    env = (env < 0 ? 0 : 0x7ff);
    if(v.env_mode == env_attack) v.env_mode = env_decay;
  }

  if(counter_poll(rate) == true) v.env = env;
}

#endif
