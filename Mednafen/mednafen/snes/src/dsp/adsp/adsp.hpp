class aDSP : public DSP {
private:
uint8 dspram[128];
uint8 *spcram;

uint32 dsp_counter;

enum { BRR_END = 1, BRR_LOOP = 2 };

uint8  readb (uint16 addr);
uint16 readw (uint16 addr);
void   writeb(uint16 addr, uint8  data);
void   writew(uint16 addr, uint16 data);

public:
static const uint16 rate_table[32];
static const int16  gaussian_table[512];

enum EnvelopeStates {
  ATTACK,
  DECAY,
  SUSTAIN,
  RELEASE,
  SILENCE
};

enum EnvelopeModes {
  DIRECT,
  LINEAR_DEC,
  EXP_DEC,
  LINEAR_INC,
  BENT_INC,

  FAST_ATTACK,
  RELEASE_DEC
};

private:
struct Status {
//$0c,$1c
  int8   MVOLL, MVOLR;
//$2c,$3c
  int8   EVOLL, EVOLR;
//$4c,$5c
  uint8  KON, KOFF;
//$6c
  uint8  FLG;
//$7c
  uint8  ENDX;
//$0d
  int8   EFB;
//$2d,$3d,$4d
  uint8  PMON, NON, EON;
//$5d
  uint8  DIR;
//$6d,$7d
  uint8  ESA, EDL;

//$xf
  int8   FIR[8];

//internal variables
  uint8  kon, esa;

  int16  noise_ctr, noise_rate;
  uint16 noise_sample;

  uint16 echo_index, echo_length;
  int16  fir_buffer[2][8];
  uint8  fir_buffer_index;

//functions
  bool soft_reset() { return !!(FLG & 0x80); }
  bool mute()       { return !!(FLG & 0x40); }
  bool echo_write() { return  !(FLG & 0x20); }
} status;

struct Voice {
//$x0-$x1
  int8   VOLL, VOLR;
//$x2-$x3
  int16  PITCH;
//$x4
  uint8  SRCN;
//$x5-$x7
  uint8  ADSR1, ADSR2, GAIN;
//$x8-$x9
  uint8  ENVX, OUTX;

//internal variables
  int16  pitch_ctr;

  int8   brr_index;
  uint16 brr_ptr;
  uint8  brr_header;
  bool   brr_looped;

  int16  brr_data[4];
  uint8  brr_data_index;

  int16  envx;
  uint16 env_ctr, env_rate, env_sustain;
  enum   EnvelopeStates env_state;
  enum   EnvelopeModes  env_mode;

  int16  outx;

//functions
  int16 pitch_rate()        { return PITCH & 0x3fff; }

  uint8 brr_header_shift()  { return brr_header >> 4; }
  uint8 brr_header_filter() { return (brr_header >> 2) & 3; }
  uint8 brr_header_flags()  { return brr_header & 3; }

  bool  ADSR_enabled()      { return !!(ADSR1 & 0x80); }
  uint8 ADSR_decay()        { return (ADSR1 >> 4) & 7; }
  uint8 ADSR_attack()       { return ADSR1 & 15; }
  uint8 ADSR_sus_level()    { return ADSR2 >> 5; }
  uint8 ADSR_sus_rate()     { return ADSR2 & 31; }

  void  AdjustEnvelope() {
    if(env_state == SILENCE) {
      env_mode = DIRECT;
      env_rate = 0;
      envx     = 0;
    } else if(env_state == RELEASE) {
      env_mode = RELEASE_DEC;
      env_rate = 0x7800;
    } else if(ADSR_enabled()) {
      switch(env_state) {
      case ATTACK:
        env_rate = rate_table[(ADSR_attack() << 1) + 1];
        env_mode = (env_rate == 0x7800) ? FAST_ATTACK : LINEAR_INC;
        break;
      case DECAY:
        env_rate = rate_table[(ADSR_decay() << 1) + 0x10];
        env_mode = EXP_DEC;
        break;
      case SUSTAIN:
        env_rate = rate_table[ADSR_sus_rate()];
        env_mode = (env_rate == 0) ? DIRECT : EXP_DEC;
        break;
      }
    } else if(GAIN & 0x80) {
      switch(GAIN & 0x60) {
      case 0x00: env_mode = LINEAR_DEC; break;
      case 0x20: env_mode = EXP_DEC;    break;
      case 0x40: env_mode = LINEAR_INC; break;
      case 0x60: env_mode = BENT_INC;   break;
      }
      env_rate = rate_table[GAIN & 0x1f];
    } else {
      env_mode = DIRECT;
      env_rate = 0;
      envx     = (GAIN & 0x7f) << 4;
    }
  }
} voice[8];

public:
  void   enter();
  void   run();

  uint8  read (uint8 addr);
  void   write(uint8 addr, uint8 data);

  void   power();
  void   reset();

  aDSP();
  ~aDSP();
};

extern aDSP dsp;
