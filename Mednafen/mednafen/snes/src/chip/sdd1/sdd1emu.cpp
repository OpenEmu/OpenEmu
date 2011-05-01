#ifdef SDD1_CPP

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

typedef uint8 bool8;
#define SDD1_read(__addr) (sdd1.read(__addr))

////////////////////////////////////////////////////


void SDD1_IM::prepareDecomp(uint32 in_buf) {

  byte_ptr=in_buf;
  bit_count=4;

}

////////////////////////////////////////////////////


uint8 SDD1_IM::getCodeword(uint8 code_len) {

  uint8 codeword;
  uint8 comp_count;

  codeword = (SDD1_read(byte_ptr))<<bit_count;

  ++bit_count;

  if (codeword & 0x80) {
    codeword |= SDD1_read(byte_ptr+1)>>(9-bit_count);
    bit_count+=code_len;
  }
  
  if (bit_count & 0x08) {
    byte_ptr++;
    bit_count&=0x07;
  }
  
  return codeword;

}

//////////////////////////////////////////////////////


SDD1_GCD::SDD1_GCD(SDD1_IM *associatedIM) :
  IM(associatedIM)
{

}

//////////////////////////////////////////////////////


void SDD1_GCD::getRunCount(uint8 code_num, uint8 *MPScount, bool8 *LPSind) {

  const uint8 run_count[] = {
    0x00, 0x00, 0x01, 0x00, 0x03, 0x01, 0x02, 0x00,
    0x07, 0x03, 0x05, 0x01, 0x06, 0x02, 0x04, 0x00,
    0x0f, 0x07, 0x0b, 0x03, 0x0d, 0x05, 0x09, 0x01,
    0x0e, 0x06, 0x0a, 0x02, 0x0c, 0x04, 0x08, 0x00,
    0x1f, 0x0f, 0x17, 0x07, 0x1b, 0x0b, 0x13, 0x03,
    0x1d, 0x0d, 0x15, 0x05, 0x19, 0x09, 0x11, 0x01,
    0x1e, 0x0e, 0x16, 0x06, 0x1a, 0x0a, 0x12, 0x02,
    0x1c, 0x0c, 0x14, 0x04, 0x18, 0x08, 0x10, 0x00,
    0x3f, 0x1f, 0x2f, 0x0f, 0x37, 0x17, 0x27, 0x07,
    0x3b, 0x1b, 0x2b, 0x0b, 0x33, 0x13, 0x23, 0x03,
    0x3d, 0x1d, 0x2d, 0x0d, 0x35, 0x15, 0x25, 0x05,
    0x39, 0x19, 0x29, 0x09, 0x31, 0x11, 0x21, 0x01,
    0x3e, 0x1e, 0x2e, 0x0e, 0x36, 0x16, 0x26, 0x06,
    0x3a, 0x1a, 0x2a, 0x0a, 0x32, 0x12, 0x22, 0x02,
    0x3c, 0x1c, 0x2c, 0x0c, 0x34, 0x14, 0x24, 0x04,
    0x38, 0x18, 0x28, 0x08, 0x30, 0x10, 0x20, 0x00,
    0x7f, 0x3f, 0x5f, 0x1f, 0x6f, 0x2f, 0x4f, 0x0f,
    0x77, 0x37, 0x57, 0x17, 0x67, 0x27, 0x47, 0x07,
    0x7b, 0x3b, 0x5b, 0x1b, 0x6b, 0x2b, 0x4b, 0x0b,
    0x73, 0x33, 0x53, 0x13, 0x63, 0x23, 0x43, 0x03,
    0x7d, 0x3d, 0x5d, 0x1d, 0x6d, 0x2d, 0x4d, 0x0d,
    0x75, 0x35, 0x55, 0x15, 0x65, 0x25, 0x45, 0x05,
    0x79, 0x39, 0x59, 0x19, 0x69, 0x29, 0x49, 0x09,
    0x71, 0x31, 0x51, 0x11, 0x61, 0x21, 0x41, 0x01,
    0x7e, 0x3e, 0x5e, 0x1e, 0x6e, 0x2e, 0x4e, 0x0e,
    0x76, 0x36, 0x56, 0x16, 0x66, 0x26, 0x46, 0x06,
    0x7a, 0x3a, 0x5a, 0x1a, 0x6a, 0x2a, 0x4a, 0x0a,
    0x72, 0x32, 0x52, 0x12, 0x62, 0x22, 0x42, 0x02,
    0x7c, 0x3c, 0x5c, 0x1c, 0x6c, 0x2c, 0x4c, 0x0c,
    0x74, 0x34, 0x54, 0x14, 0x64, 0x24, 0x44, 0x04,
    0x78, 0x38, 0x58, 0x18, 0x68, 0x28, 0x48, 0x08,
    0x70, 0x30, 0x50, 0x10, 0x60, 0x20, 0x40, 0x00,
  };

  uint8 codeword=IM->getCodeword(code_num);

  if (codeword & 0x80) {
    *LPSind=1;
    *MPScount=run_count[codeword>>(code_num^0x07)];
  }
  else {
    *MPScount=(1<<code_num);
  }

}

///////////////////////////////////////////////////////

SDD1_BG::SDD1_BG(SDD1_GCD *associatedGCD, uint8 code) :
  GCD(associatedGCD), code_num(code)
{

}

///////////////////////////////////////////////


void SDD1_BG::prepareDecomp(void) {

  MPScount=0;
  LPSind=0;

}

//////////////////////////////////////////////


uint8 SDD1_BG::getBit(bool8 *endOfRun) {

  uint8 bit;

  if (!(MPScount || LPSind)) GCD->getRunCount(code_num, &MPScount, &LPSind);

  if (MPScount) {
    bit=0;
    MPScount--;
  }
  else {
    bit=1;
    LPSind=0;
  }

  if (MPScount || LPSind) (*endOfRun)=0;
  else (*endOfRun)=1;

  return bit;

}

/////////////////////////////////////////////////


SDD1_PEM::SDD1_PEM(SDD1_BG *associatedBG0, SDD1_BG *associatedBG1,
		   SDD1_BG *associatedBG2, SDD1_BG *associatedBG3,
		   SDD1_BG *associatedBG4, SDD1_BG *associatedBG5,
		   SDD1_BG *associatedBG6, SDD1_BG *associatedBG7) {

  BG[0]=associatedBG0;
  BG[1]=associatedBG1;
  BG[2]=associatedBG2;
  BG[3]=associatedBG3;
  BG[4]=associatedBG4;
  BG[5]=associatedBG5;
  BG[6]=associatedBG6;
  BG[7]=associatedBG7;

}

/////////////////////////////////////////////////////////


const SDD1_PEM::state SDD1_PEM::evolution_table[]={
    { 0,25,25},
    { 0, 2, 1},
    { 0, 3, 1},
    { 0, 4, 2},
    { 0, 5, 3},
    { 1, 6, 4},
    { 1, 7, 5},
    { 1, 8, 6},
    { 1, 9, 7},
    { 2,10, 8},
    { 2,11, 9},
    { 2,12,10},
    { 2,13,11},
    { 3,14,12},
    { 3,15,13},
    { 3,16,14},
    { 3,17,15},
    { 4,18,16},
    { 4,19,17},
    { 5,20,18},
    { 5,21,19},
    { 6,22,20},
    { 6,23,21},
    { 7,24,22},
    { 7,24,23},
    { 0,26, 1},
    { 1,27, 2},
    { 2,28, 4},
    { 3,29, 8},
    { 4,30,12},
    { 5,31,16},
    { 6,32,18},
    { 7,24,22}
  };

//////////////////////////////////////////////////////


void SDD1_PEM::prepareDecomp(void) {

  for (uint8 i=0; i<32; i++) {
    contextInfo[i].status=0;
    contextInfo[i].MPS=0;
  }

}

/////////////////////////////////////////////////////////


uint8 SDD1_PEM::getBit(uint8 context) {

  bool8 endOfRun;
  uint8 bit;

  SDD1_ContextInfo *pContInfo=&contextInfo[context];
  uint8 currStatus = pContInfo->status;
  const state *pState=&SDD1_PEM::evolution_table[currStatus];
  uint8 currentMPS=pContInfo->MPS;

  bit=(BG[pState->code_num])->getBit(&endOfRun);

  if (endOfRun)
    if (bit) {
      if (!(currStatus & 0xfe)) (pContInfo->MPS)^=0x01;
      (pContInfo->status)=pState->nextIfLPS;
    }
    else
      (pContInfo->status)=pState->nextIfMPS;

  return bit^currentMPS;

}

//////////////////////////////////////////////////////////////


SDD1_CM::SDD1_CM(SDD1_PEM *associatedPEM) :
  PEM(associatedPEM)
{

}

//////////////////////////////////////////////////////////////


void SDD1_CM::prepareDecomp(uint32 first_byte) {

  bitplanesInfo = SDD1_read(first_byte) & 0xc0;
  contextBitsInfo = SDD1_read(first_byte) & 0x30;
  bit_number=0;
  for (int i=0; i<8; i++) prevBitplaneBits[i]=0;
  switch (bitplanesInfo) {
  case 0x00:
    currBitplane = 1;
    break;
  case 0x40:
    currBitplane = 7;
    break;
  case 0x80:
    currBitplane = 3;
  }

}

/////////////////////////////////////////////////////////////


uint8 SDD1_CM::getBit(void) {

  uint8 currContext;
  uint16 *context_bits;

  switch (bitplanesInfo) {
  case 0x00:
    currBitplane ^= 0x01;
    break;
  case 0x40:
    currBitplane ^= 0x01;
    if (!(bit_number & 0x7f)) currBitplane = ((currBitplane+2) & 0x07);
    break;
  case 0x80:
    currBitplane ^= 0x01;
    if (!(bit_number & 0x7f)) currBitplane ^= 0x02;
    break;
  case 0xc0:
    currBitplane = bit_number & 0x07;
  }

  context_bits = &prevBitplaneBits[currBitplane];

  currContext=(currBitplane & 0x01)<<4;
  switch (contextBitsInfo) {
  case 0x00:
    currContext|=((*context_bits & 0x01c0)>>5)|(*context_bits & 0x0001);
    break;
  case 0x10:
    currContext|=((*context_bits & 0x0180)>>5)|(*context_bits & 0x0001);
    break;
  case 0x20:
    currContext|=((*context_bits & 0x00c0)>>5)|(*context_bits & 0x0001);
    break;
  case 0x30:
    currContext|=((*context_bits & 0x0180)>>5)|(*context_bits & 0x0003);
  }

  uint8 bit=PEM->getBit(currContext);

  *context_bits <<= 1;
  *context_bits |= bit;

  bit_number++;

  return bit;

}

//////////////////////////////////////////////////


SDD1_OL::SDD1_OL(SDD1_CM *associatedCM) :
  CM(associatedCM)
{

}

///////////////////////////////////////////////////


void SDD1_OL::prepareDecomp(uint32 first_byte, uint16 out_len, uint8 *out_buf) {

  bitplanesInfo = SDD1_read(first_byte) & 0xc0;
  length=out_len;
  buffer=out_buf;

}

///////////////////////////////////////////////////


void SDD1_OL::launch(void) {

  uint8 i;
  uint8 register1, register2;

  switch (bitplanesInfo) {
  case 0x00:
  case 0x40:
  case 0x80:
    i=1;
    do {       //if length==0, we output 2^16 bytes
      if (!i) {
	*(buffer++)=register2;
	i=~i;
      }
      else {
	for (register1=register2=0, i=0x80; i; i>>=1) {
	  if (CM->getBit()) register1 |= i;
	  if (CM->getBit()) register2 |= i;
	}
	*(buffer++)=register1;
      }
    } while (--length);
    break;
  case 0xc0:
    do {
      for (register1=0, i=0x01; i; i<<=1) {
	if (CM->getBit()) register1 |= i;
      }
      *(buffer++)=register1;
    } while (--length);
  }

}

///////////////////////////////////////////////////////


void SDD1emu::decompress(uint32 in_buf, uint16 out_len, uint8 *out_buf) {

  IM.prepareDecomp(in_buf);
  BG0.prepareDecomp();
  BG1.prepareDecomp();
  BG2.prepareDecomp();
  BG3.prepareDecomp();
  BG4.prepareDecomp();
  BG5.prepareDecomp();
  BG6.prepareDecomp();
  BG7.prepareDecomp();
  PEM.prepareDecomp();
  CM.prepareDecomp(in_buf);
  OL.prepareDecomp(in_buf, out_len, out_buf);

  OL.launch();

}

////////////////////////////////////////////////////////////


SDD1emu::SDD1emu() : 
  GCD(&IM),
  BG0(&GCD, 0), BG1(&GCD, 1), BG2(&GCD, 2), BG3(&GCD, 3),
  BG4(&GCD, 4), BG5(&GCD, 5), BG6(&GCD, 6), BG7(&GCD, 7),
  PEM(&BG0, &BG1, &BG2, &BG3, &BG4, &BG5, &BG6, &BG7),
  CM(&PEM),
  OL(&CM)
{

}

///////////////////////////////////////////////////////////

#endif
