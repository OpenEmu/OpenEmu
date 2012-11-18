struct Decomp {
  struct IM {  //input manager
    Decomp &self;
    void init(unsigned offset);
    uint8 get_codeword(uint8 code_length);
    IM(SDD1::Decomp &self) : self(self) {}
  private:
    unsigned offset;
    unsigned bit_count;
  };

  struct GCD {  //golomb-code decoder
    Decomp &self;
    static const uint8 run_count[256];
    void get_run_count(uint8 code_number, uint8 &mps_count, bool &lps_index);
    GCD(SDD1::Decomp &self) : self(self) {}
  };

  struct BG {  //bits generator
    Decomp &self;
    void init();
    uint8 get_bit(bool &end_of_run);
    BG(SDD1::Decomp &self, uint8 code_number) : self(self), code_number(code_number) {}
  private:
    const uint8 code_number;
    uint8 mps_count;
    bool lps_index;
  };

  struct PEM {  //probability estimation module
    Decomp &self;
    void init();
    uint8 get_bit(uint8 context);
    PEM(SDD1::Decomp &self) : self(self) {}
  private:
    struct State {
      uint8 code_number;
      uint8 next_if_mps;
      uint8 next_if_lps;
    };
    static const State evolution_table[33];
    struct ContextInfo {
      uint8 status;
      uint8 mps;
    } context_info[32];
  };

  struct CM {  //context model
    Decomp &self;
    void init(unsigned offset);
    uint8 get_bit();
    CM(SDD1::Decomp &self) : self(self) {}
  private:
    uint8 bitplanes_info;
    uint8 context_bits_info;
    uint8 bit_number;
    uint8 current_bitplane;
    uint16 previous_bitplane_bits[8];
  };

  struct OL {  //output logic
    Decomp &self;
    void init(unsigned offset);
    uint8 decompress();
    OL(SDD1::Decomp &self) : self(self) {}
  private:
    uint8 bitplanes_info;
    uint8 r0, r1, r2;
  };

  void init(unsigned offset);
  uint8 read();
  uint8 rom_read(unsigned offset);
  Decomp();

  IM  im;
  GCD gcd;
  BG  bg0, bg1, bg2, bg3, bg4, bg5, bg6, bg7;
  PEM pem;
  CM  cm;
  OL  ol;
};
