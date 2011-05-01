/************************************************************************

S-DD1'algorithm emulation code
------------------------------

Author:	     Andreas Naive
Date:        August 2003
Last update: October 2004

This code is Public Domain. There is no copyright holded by the author.
Said this, the author wish to explicitly emphasize his inalienable moral rights
over this piece of intelectual work and the previous research that made it
possible, as recognized by most of the copyright laws around the world.

This code is provided 'as-is', with no warranty, expressed or implied.
No responsability is assumed by the author in connection with it.

The author is greatly indebted with The Dumper, without whose help and
patience providing him with real S-DD1 data the research would have never been
possible. He also wish to note that in the very beggining of his research,
Neviksti had done some steps in the right direction. By last, the author is
indirectly indebted to all the people that worked and contributed in the
S-DD1 issue in the past.

An algorithm's documentation is available as a separate document.
The implementation is obvious when the algorithm is
understood.

************************************************************************/

#define bool8 uint8

class SDD1_IM {  //Input Manager

 public:
  SDD1_IM(void) {}
  void prepareDecomp(uint32 in_buf);
  uint8 getCodeword(const uint8 code_len);
  
 private:
  uint32 byte_ptr;
  uint8 bit_count;

};

////////////////////////////////////////////////////


class SDD1_GCD {  //Golomb-Code Decoder

 public:
  SDD1_GCD(SDD1_IM *associatedIM);
  void getRunCount(uint8 code_num, uint8 *MPScount, bool8 *LPSind);
  
 private:
  SDD1_IM *const IM;

};

//////////////////////////////////////////////////////


class SDD1_BG {  // Bits Generator

 public:
  SDD1_BG(SDD1_GCD *associatedGCD, uint8 code);
  void prepareDecomp(void);
  uint8 getBit(bool8 *endOfRun);

 private:
  const uint8 code_num;
  uint8 MPScount;
  bool8 LPSind;
  SDD1_GCD *const GCD;

};

////////////////////////////////////////////////


class SDD1_PEM {  //Probability Estimation Module

 public:
  SDD1_PEM(SDD1_BG *associatedBG0, SDD1_BG *associatedBG1,
	   SDD1_BG *associatedBG2, SDD1_BG *associatedBG3,
	   SDD1_BG *associatedBG4, SDD1_BG *associatedBG5,
	   SDD1_BG *associatedBG6, SDD1_BG *associatedBG7);
  void prepareDecomp(void);
  uint8 getBit(uint8 context);

 private:
  struct state {
    uint8 code_num;
    uint8 nextIfMPS;
    uint8 nextIfLPS;
  };
  static const state evolution_table[];
  struct SDD1_ContextInfo {
    uint8 status;
    uint8 MPS;
  } contextInfo[32];
  SDD1_BG * BG[8];

};

///////////////////////////////////////////////////


class SDD1_CM {  //Context Model

 public:
  SDD1_CM(SDD1_PEM *associatedPEM);
  void prepareDecomp(uint32 first_byte);
  uint8 getBit(void);

 private:
  uint8 bitplanesInfo;
  uint8 contextBitsInfo;
  uint8 bit_number;
  uint8 currBitplane;
  uint16 prevBitplaneBits[8];
  SDD1_PEM *const PEM;

};

///////////////////////////////////////////////////


class SDD1_OL {  //Output Logic

 public:
  SDD1_OL(SDD1_CM *associatedCM);
  void prepareDecomp(uint32 first_byte, uint16 out_len, uint8 *out_buf);
  void launch(void);

 private:
  uint8 bitplanesInfo;
  uint16 length;
  uint8 *buffer;
  SDD1_CM *const CM;

};

/////////////////////////////////////////////////////////


class SDD1emu {

 public:
  SDD1emu(void);
  void decompress(uint32 in_buf, uint16 out_len, uint8 *out_buf);

 private:
  SDD1_IM IM;
  SDD1_GCD GCD;
  SDD1_BG BG0;  SDD1_BG BG1;  SDD1_BG BG2;  SDD1_BG BG3;
  SDD1_BG BG4;  SDD1_BG BG5;  SDD1_BG BG6;  SDD1_BG BG7;
  SDD1_PEM PEM;
  SDD1_CM CM;
  SDD1_OL OL;

};

#undef bool8
