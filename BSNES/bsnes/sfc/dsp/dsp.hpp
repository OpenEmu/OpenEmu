struct DSP : Thread {
  enum : bool { Threaded = true };
  alwaysinline void step(unsigned clocks);
  alwaysinline void synchronize_smp();

  uint8 read(uint8 addr);
  void write(uint8 addr, uint8 data);

  void enter();
  void power();
  void reset();

  void serialize(serializer&);
  DSP();
  ~DSP();

privileged:
  #include "moduloarray.hpp"

  //global registers
  enum global_reg_t {
    r_mvoll = 0x0c, r_mvolr = 0x1c,
    r_evoll = 0x2c, r_evolr = 0x3c,
    r_kon   = 0x4c, r_koff  = 0x5c,
    r_flg   = 0x6c, r_endx  = 0x7c,
    r_efb   = 0x0d, r_pmon  = 0x2d,
    r_non   = 0x3d, r_eon   = 0x4d,
    r_dir   = 0x5d, r_esa   = 0x6d,
    r_edl   = 0x7d, r_fir   = 0x0f,  //8 coefficients at 0x0f, 0x1f, ... 0x7f
  };

  //voice registers
  enum voice_reg_t {
    v_voll   = 0x00, v_volr   = 0x01,
    v_pitchl = 0x02, v_pitchh = 0x03,
    v_srcn   = 0x04, v_adsr0  = 0x05,
    v_adsr1  = 0x06, v_gain   = 0x07,
    v_envx   = 0x08, v_outx   = 0x09,
  };

  //internal envelope modes
  enum env_mode_t { env_release, env_attack, env_decay, env_sustain };

  //internal constants
  enum { echo_hist_size =  8 };
  enum { brr_buf_size   = 12 };
  enum { brr_block_size =  9 };

  //global state
  struct state_t {
    uint8 regs[128];

    moduloarray<int, echo_hist_size> echo_hist[2];  //echo history keeps most recent 8 samples
    int echo_hist_pos;

    bool every_other_sample;  //toggles every sample
    int kon;                  //KON value when last checked
    int noise;
    int counter;
    int echo_offset;          //offset from ESA in echo buffer
    int echo_length;          //number of bytes that echo_offset will stop at

    //hidden registers also written to when main register is written to
    int new_kon;
    int endx_buf;
    int envx_buf;
    int outx_buf;

    //temporary state between clocks

    //read once per sample
    int t_pmon;
    int t_non;
    int t_eon;
    int t_dir;
    int t_koff;

    //read a few clocks ahead before used
    int t_brr_next_addr;
    int t_adsr0;
    int t_brr_header;
    int t_brr_byte;
    int t_srcn;
    int t_esa;
    int t_echo_disabled;

    //internal state that is recalculated every sample
    int t_dir_addr;
    int t_pitch;
    int t_output;
    int t_looped;
    int t_echo_ptr;

    //left/right sums
    int t_main_out[2];
    int t_echo_out[2];
    int t_echo_in [2];
  } state;

  //voice state
  struct voice_t {
    moduloarray<int, brr_buf_size> buffer;  //decoded samples
    int buf_pos;     //place in buffer where next samples will be decoded
    int interp_pos;  //relative fractional position in sample (0x1000 = 1.0)
    int brr_addr;    //address of current BRR block
    int brr_offset;  //current decoding offset in BRR block
    int vbit;        //bitmask for voice: 0x01 for voice 0, 0x02 for voice 1, etc
    int vidx;        //voice channel register index: 0x00 for voice 0, 0x10 for voice 1, etc
    int kon_delay;   //KON delay/current setup phase
    int env_mode;
    int env;         //current envelope level
    int t_envx_out;
    int hidden_env;  //used by GAIN mode 7, very obscure quirk
  } voice[8];

  //gaussian
  static const int16 gaussian_table[512];
  int gaussian_interpolate(const voice_t &v);

  //counter
  enum { counter_range = 2048 * 5 * 3 };  //30720 (0x7800)
  static const uint16 counter_rate[32];
  static const uint16 counter_offset[32];
  void counter_tick();
  bool counter_poll(unsigned rate);

  //envelope
  void envelope_run(voice_t &v);

  //brr
  void brr_decode(voice_t &v);

  //misc
  void misc_27();
  void misc_28();
  void misc_29();
  void misc_30();

  //voice
  void voice_output(voice_t &v, bool channel);
  void voice_1 (voice_t &v);
  void voice_2 (voice_t &v);
  void voice_3 (voice_t &v);
  void voice_3a(voice_t &v);
  void voice_3b(voice_t &v);
  void voice_3c(voice_t &v);
  void voice_4 (voice_t &v);
  void voice_5 (voice_t &v);
  void voice_6 (voice_t &v);
  void voice_7 (voice_t &v);
  void voice_8 (voice_t &v);
  void voice_9 (voice_t &v);

  //echo
  int calc_fir(int i, bool channel);
  int echo_output(bool channel);
  void echo_read(bool channel);
  void echo_write(bool channel);
  void echo_22();
  void echo_23();
  void echo_24();
  void echo_25();
  void echo_26();
  void echo_27();
  void echo_28();
  void echo_29();
  void echo_30();

  //dsp
  static void Enter();
  void tick();
};

extern DSP dsp;
