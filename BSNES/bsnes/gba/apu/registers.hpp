struct Registers {
  struct SoundBias {
    uint10 level;
    uint2 amplitude;

    operator uint16() const;
    uint16 operator=(uint16 source);
    SoundBias& operator=(const SoundBias&) = delete;
  } bias;

  unsigned clock;
} regs;

struct Sweep {
  uint3 shift;
  uint1 direction;
  uint3 frequency;

  uint1 enable;
  uint1 negate;
  uint3 period;
};

struct Envelope {
  uint3 frequency;
  uint1 direction;
  uint4 volume;

  uint3 period;

  inline bool dacenable() const { return volume || direction; }
};

struct Square {
  Envelope envelope;
  uint1 enable;
  uint6 length;
  uint2 duty;
  uint11 frequency;
  uint1 counter;
  uint1 initialize;

  signed shadowfrequency;
  uint1 signal;
  uint4 output;
  unsigned period;
  uint3 phase;
  uint4 volume;

  void run();
  void clocklength();
  void clockenvelope();
};

struct Square1 : Square {
  Sweep sweep;

  void runsweep(bool update);
  void clocksweep();
  uint8 read(unsigned addr) const;
  void write(unsigned addr, uint8 byte);
  void power();
} square1;

struct Square2 : Square {
  uint8 read(unsigned addr) const;
  void write(unsigned addr, uint8 byte);
  void power();
} square2;

struct Wave {
  uint1 mode;
  uint1 bank;
  uint1 dacenable;
  uint8 length;
  uint3 volume;
  uint11 frequency;
  uint1 counter;
  uint1 initialize;
  uint4 pattern[32];

  uint1 enable;
  uint4 output;
  uint4 patternaddr;
  uint1 patternbank;
  uint4 patternsample;
  unsigned period;

  void run();
  void clocklength();
  uint8 read(unsigned addr) const;
  void write(unsigned addr, uint8 byte);
  uint8 readram(unsigned addr) const;
  void writeram(unsigned addr, uint8 byte);
  void power();
} wave;

struct Noise {
  Envelope envelope;
  uint6 length;
  uint3 divisor;
  uint1 narrowlfsr;
  uint4 frequency;
  uint1 counter;
  uint1 initialize;

  uint1 enable;
  uint15 lfsr;
  uint4 output;
  unsigned period;
  uint4 volume;

  unsigned divider() const;
  void run();
  void clocklength();
  void clockenvelope();
  uint8 read(unsigned addr) const;
  void write(unsigned addr, uint8 byte);
  void power();
} noise;

struct Sequencer {
  uint2 volume;
  uint3 lvolume;
  uint3 rvolume;
  uint1 lenable[4];
  uint1 renable[4];
  uint1 enable[4];
  uint1 masterenable;

  uint13 base;
  uint3 step;
  int16 lsample;
  int16 rsample;

  uint8 read(unsigned addr) const;
  void write(unsigned addr, uint8 byte);
  void power();
} sequencer;

struct FIFO {
  int8 sample[32];
  int8 output;

  uint5 rdoffset;
  uint5 wroffset;
  uint6 size;

  uint1 volume;  //0 = 50%, 1 = 100%
  uint1 lenable;
  uint1 renable;
  uint1 timer;

  void read();
  void write(int8 byte);
  void reset();
  void power();
} fifo[2];
