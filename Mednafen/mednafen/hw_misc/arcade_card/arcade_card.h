#ifndef __MDFN_PCE_ARCADE_CARD_H
#define __MDFN_PCE_ARCADE_CARD_H

typedef struct
{
 uint32 base;           // 24 bits
 uint16 offset;         // 16 bits
 uint16 increment;      // 16 bits
 uint8 control;         //  7 bits
} ACPort_t;

typedef struct
{
 ACPort_t ports[4];
 uint32 shift_latch;    // 32 bits
 uint8 shift_bits;      // signed 4-bit value
 uint8 rotate_bits;	// same
} ArcadeCard_t;

class ArcadeCard
{
 public:

 ArcadeCard(void);
 ~ArcadeCard();

 void Power(void);
 int StateAction(StateMem *sm, int load, int data_only);

 uint8 Read(uint32 A, bool peek = false);	// Pass peek as true if you don't want side-effects from this read(IE in a debugger).
 void Write(uint32 A, uint8 V);

 INLINE void PhysWrite(uint32 A, uint8 V)
 {
  Write(0x1a00 | ((A >> 9) & 0x30), V);
 }

 INLINE uint8 PhysRead(uint32 A, bool peek = false)
 {
  return(Read(0x1a00 | ((A >> 9) & 0x30), peek));
 }


 void PeekRAM(uint32 Address, uint32 Length, uint8 *Buffer);
 void PokeRAM(uint32 Address, uint32 Length, const uint8 *Buffer);

 private:

 ArcadeCard_t AC;

 bool ACRAMUsed;
 uint8 ACRAM[0x200000];
};

#endif
