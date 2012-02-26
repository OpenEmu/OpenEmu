#ifdef DSP_CPP

//counter_rate = number of samples per counter event
//all rates are evenly divisible by counter_range (0x7800, 30720, or 2048 * 5 * 3)
//note that rate[0] is a special case, which never triggers

const uint16 DSP::counter_rate[32] = {
     0, 2048, 1536,
  1280, 1024,  768,
   640,  512,  384,
   320,  256,  192,
   160,  128,   96,
    80,   64,   48,
    40,   32,   24,
    20,   16,   12,
    10,    8,    6,
     5,    4,    3,
           2,
           1,
};

//counter_offset = counter offset from zero
//counters do not appear to be aligned at zero for all rates

const uint16 DSP::counter_offset[32] = {
    0, 0, 1040,
  536, 0, 1040,
  536, 0, 1040,
  536, 0, 1040,
  536, 0, 1040,
  536, 0, 1040,
  536, 0, 1040,
  536, 0, 1040,
  536, 0, 1040,
  536, 0, 1040,
       0,
       0,
};

inline void DSP::counter_tick() {
  state.counter--;
  if(state.counter < 0) state.counter = counter_range - 1;
}

//return true if counter event should trigger

inline bool DSP::counter_poll(unsigned rate) {
  if(rate == 0) return false;
  return (((unsigned)state.counter + counter_offset[rate]) % counter_rate[rate]) == 0;
}

#endif
