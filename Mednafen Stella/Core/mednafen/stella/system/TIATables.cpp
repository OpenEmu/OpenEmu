//============================================================================
//
//   SSSS    tt          lll  lll       
//  SS  SS   tt           ll   ll        
//  SS     tttttt  eeee   ll   ll   aaaa 
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2011 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: TIATables.cxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#include <cassert>

#include "bspf.hxx"
#include "TIATables.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIATables::computeAllTables()
{
  for(uInt32 i = 0; i < 640; ++i)
    DisabledMask[i] = 0;

  buildCollisionMaskTable();
  buildPxMaskTable();
  buildMxMaskTable();
  buildBLMaskTable();
  buildPFMaskTable();
  buildGRPReflectTable();
  buildPxPosResetWhenTable();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIATables::buildCollisionMaskTable()
{
  for(uInt8 i = 0; i < 64; ++i)
  { 
    CollisionMask[i] = 0;

    if((i & M0Bit) && (i & P1Bit))    // M0-P1
      CollisionMask[i] |= Cx_M0P1;

    if((i & M0Bit) && (i & P0Bit))    // M0-P0
      CollisionMask[i] |= Cx_M0P0;

    if((i & M1Bit) && (i & P0Bit))    // M1-P0
      CollisionMask[i] |= Cx_M1P0;

    if((i & M1Bit) && (i & P1Bit))    // M1-P1
      CollisionMask[i] |= Cx_M1P1;

    if((i & P0Bit) && (i & PFBit))    // P0-PF
      CollisionMask[i] |= Cx_P0PF;

    if((i & P0Bit) && (i & BLBit))    // P0-BL
      CollisionMask[i] |= Cx_P0BL;

    if((i & P1Bit) && (i & PFBit))    // P1-PF
      CollisionMask[i] |= Cx_P1PF;

    if((i & P1Bit) && (i & BLBit))    // P1-BL
      CollisionMask[i] |= Cx_P1BL;

    if((i & M0Bit) && (i & PFBit))    // M0-PF
      CollisionMask[i] |= Cx_M0PF;

    if((i & M0Bit) && (i & BLBit))    // M0-BL
      CollisionMask[i] |= Cx_M0BL;

    if((i & M1Bit) && (i & PFBit))    // M1-PF
      CollisionMask[i] |= Cx_M1PF;

    if((i & M1Bit) && (i & BLBit))    // M1-BL
      CollisionMask[i] |= Cx_M1BL;

    if((i & BLBit) && (i & PFBit))    // BL-PF
      CollisionMask[i] |= Cx_BLPF;

    if((i & P0Bit) && (i & P1Bit))    // P0-P1
      CollisionMask[i] |= Cx_P0P1;

    if((i & M0Bit) && (i & M1Bit))    // M0-M1
      CollisionMask[i] |= Cx_M0M1;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// [4][2][8][320]
// [alignment][suppress mode][nusiz][pixel]
// suppress=1: suppress on
// suppress=0: suppress off
void TIATables::buildPxMaskTable()
{
  // First, calculate masks for alignment 0
  Int32 x, suppress, nusiz;

  // Set the player mask table to all zeros
  for(suppress = 0; suppress < 2; ++suppress)
    for(nusiz = 0; nusiz < 8; ++nusiz)
      for(x = 0; x < 160; ++x)
        PxMask[0][suppress][nusiz][x] = 0x00;

  // Now, compute the player mask table
  for(suppress = 0; suppress < 2; ++suppress)
  {
    for(nusiz = 0; nusiz < 8; ++nusiz)
    {
      for(x = 0; x < 160 + 72; ++x)
      {
        // nusiz:
        // 0: one copy
        // 1: two copies-close
        // 2: two copies-med
        // 3: three copies-close
        // 4: two copies-wide
        // 5: double size player
        // 6: 3 copies medium
        // 7: quad sized player
        switch(nusiz)
        {
          case 0x00:
            if((suppress == 0) && (x >= 0) && (x < 8))
              PxMask[0][suppress][nusiz][x % 160] = 0x80 >> x;
            break;

          case 0x01:
            if((suppress == 0) && (x >= 0) && (x < 8))
              PxMask[0][suppress][nusiz][x % 160] = 0x80 >> x;
            else if(((x - 16) >= 0) && ((x - 16) < 8))
              PxMask[0][suppress][nusiz][x % 160] = 0x80 >> (x - 16);
            break;

          case 0x02:
            if((suppress == 0) && (x >= 0) && (x < 8))
              PxMask[0][suppress][nusiz][x % 160] = 0x80 >> x;
            else if(((x - 32) >= 0) && ((x - 32) < 8))
              PxMask[0][suppress][nusiz][x % 160] = 0x80 >> (x - 32);
            break;

          case 0x03:
            if((suppress == 0) && (x >= 0) && (x < 8))
              PxMask[0][suppress][nusiz][x % 160] = 0x80 >> x;
            else if(((x - 16) >= 0) && ((x - 16) < 8))
              PxMask[0][suppress][nusiz][x % 160] = 0x80 >> (x - 16);
            else if(((x - 32) >= 0) && ((x - 32) < 8))
              PxMask[0][suppress][nusiz][x % 160] = 0x80 >> (x - 32);
            break;

          case 0x04:
            if((suppress == 0) && (x >= 0) && (x < 8))
              PxMask[0][suppress][nusiz][x % 160] = 0x80 >> x;
            else if(((x - 64) >= 0) && ((x - 64) < 8))
              PxMask[0][suppress][nusiz][x % 160] = 0x80 >> (x - 64);
            break;

          case 0x05:
            // For some reason in double size nusiz the player's output
            // is delayed by one pixel thus we use > instead of >=
            if((suppress == 0) && (x > 0) && (x <= 16))
              PxMask[0][suppress][nusiz][x % 160] = 0x80 >> ((x - 1)/2);
            break;

          case 0x06:
            if((suppress == 0) && (x >= 0) && (x < 8))
              PxMask[0][suppress][nusiz][x % 160] = 0x80 >> x;
            else if(((x - 32) >= 0) && ((x - 32) < 8))
              PxMask[0][suppress][nusiz][x % 160] = 0x80 >> (x - 32);
            else if(((x - 64) >= 0) && ((x - 64) < 8))
              PxMask[0][suppress][nusiz][x % 160] = 0x80 >> (x - 64);
            break;

          case 0x07:
            // For some reason in quad size nusiz the player's output
            // is delayed by one pixel thus we use > instead of >=
            if((suppress == 0) && (x > 0) && (x <= 32))
              PxMask[0][suppress][nusiz][x % 160] = 0x80 >> ((x - 1)/4);
            break;
        }
      }
  
      // Copy data into wrap-around area
      for(x = 0; x < 160; ++x)
        PxMask[0][suppress][nusiz][x + 160] = PxMask[0][suppress][nusiz][x];
    }
  }

  // Now, copy data for alignments of 1, 2 and 3
  for(uInt32 align = 1; align < 4; ++align)
  {
    for(suppress = 0; suppress < 2; ++suppress)
    {
      for(nusiz = 0; nusiz < 8; ++nusiz)
      {
        for(x = 0; x < 320; ++x)
        {
          PxMask[align][suppress][nusiz][x] =
              PxMask[0][suppress][nusiz][(x + 320 - align) % 320];
        }
      }
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// [4][8][5][320]
// [alignment][number][size][pixel]
void TIATables::buildMxMaskTable()
{
  // First, calculate masks for alignment 0
  Int32 x, size, number;

  // Clear the missle table to start with
  for(number = 0; number < 8; ++number)
    for(size = 0; size < 5; ++size)
      for(x = 0; x < 160; ++x)
        MxMask[0][number][size][x] = false;

  for(number = 0; number < 8; ++number)
  {
    for(size = 0; size < 5; ++size)
    {
      for(x = 0; x < 160 + 72; ++x)
      {
        // For the following, size index = 4 is almost exactly the same as
        // index = 2; that is, 1 << 2, or 4 colour clocks wide
        // To simulate the weirdness in the Cosmic Ark starfield effect,
        // each group of 4 pixels has its 3rd pixel blanked
        switch(number)
        {
          // Only one copy of the missle
          case 0x00:
          case 0x05:
          case 0x07:
            if(size != 4)
            {
              if((x >= 0) && (x < (1 << size)))
                MxMask[0][number][size][x % 160] = true;
            }
            else
            {
              if((x >= 0) && (x < (1 << 2)))
                MxMask[0][number][4][x % 160] = ((x - 2) % 4 == 0 ? false : true);
            }
          break;

          // Two copies - close
          case 0x01:
            if(size != 4)
            {
              if((x >= 0) && (x < (1 << size)))
                MxMask[0][number][size][x % 160] = true;
              else if(((x - 16) >= 0) && ((x - 16) < (1 << size)))
                MxMask[0][number][size][x % 160] = true;
            }
            else
            {
              if((x >= 0) && (x < (1 << 2)))
                MxMask[0][number][4][x % 160] = ((x - 2) % 4 == 0 ? false : true);
              else if(((x - 16) >= 0) && ((x - 16) < (1 << 2)))
                MxMask[0][number][4][x % 160] = ((x - 2) % 4 == 0 ? false : true);
            }
            break;

          // Two copies - medium
          case 0x02:
            if(size != 4)
            {
              if((x >= 0) && (x < (1 << size)))
                MxMask[0][number][size][x % 160] = true;
              else if(((x - 32) >= 0) && ((x - 32) < (1 << size)))
                MxMask[0][number][size][x % 160] = true;
            }
            else
            {
              if((x >= 0) && (x < (1 << 2)))
                MxMask[0][number][4][x % 160] = ((x - 2) % 4 == 0 ? false : true);
              else if(((x - 32) >= 0) && ((x - 32) < (1 << 2)))
                MxMask[0][number][4][x % 160] = ((x - 2) % 4 == 0 ? false : true);
            }
            break;

          // Three copies - close
          case 0x03:
            if(size != 4)
            {
              if((x >= 0) && (x < (1 << size)))
                MxMask[0][number][size][x % 160] = true;
              else if(((x - 16) >= 0) && ((x - 16) < (1 << size)))
                MxMask[0][number][size][x % 160] = true;
              else if(((x - 32) >= 0) && ((x - 32) < (1 << size)))
                MxMask[0][number][size][x % 160] = true;
            }
            else
            {
              if((x >= 0) && (x < (1 << 2)))
                MxMask[0][number][4][x % 160] = ((x - 2) % 4 == 0 ? false : true);
              else if(((x - 16) >= 0) && ((x - 16) < (1 << 2)))
                MxMask[0][number][4][x % 160] = ((x - 2) % 4 == 0 ? false : true);
              else if(((x - 32) >= 0) && ((x - 32) < (1 << 2)))
                MxMask[0][number][4][x % 160] = ((x - 2) % 4 == 0 ? false : true);
            }
            break;

          // Two copies - wide
          case 0x04:
            if(size != 4)
            {
              if((x >= 0) && (x < (1 << size)))
                MxMask[0][number][size][x % 160] = true;
              else if(((x - 64) >= 0) && ((x - 64) < (1 << size)))
                MxMask[0][number][size][x % 160] = true;
            }
            else
            {
              if((x >= 0) && (x < (1 << 2)))
                MxMask[0][number][4][x % 160] = ((x - 2) % 4 == 0 ? false : true);
              else if(((x - 64) >= 0) && ((x - 64) < (1 << 2)))
                MxMask[0][number][4][x % 160] = ((x - 2) % 4 == 0 ? false : true);
            }
            break;

          // Three copies - medium
          case 0x06:
            if(size != 4)
            {
              if((x >= 0) && (x < (1 << size)))
                MxMask[0][number][size][x % 160] = true;
              else if(((x - 32) >= 0) && ((x - 32) < (1 << size)))
                MxMask[0][number][size][x % 160] = true;
              else if(((x - 64) >= 0) && ((x - 64) < (1 << size)))
                MxMask[0][number][size][x % 160] = true;
            }
            else
            {
              if((x >= 0) && (x < (1 << 2)))
                MxMask[0][number][4][x % 160] = ((x - 2) % 4 == 0 ? false : true);
              else if(((x - 32) >= 0) && ((x - 32) < (1 << 2)))
                MxMask[0][number][4][x % 160] = ((x - 2) % 4 == 0 ? false : true);
              else if(((x - 64) >= 0) && ((x - 64) < (1 << 2)))
                MxMask[0][number][4][x % 160] = ((x - 2) % 4 == 0 ? false : true);
            }
            break;
        }
      }

      // Copy data into wrap-around area
      for(x = 0; x < 160; ++x)
        MxMask[0][number][size][x + 160] = 
          MxMask[0][number][size][x];
    }
  }

  // Now, copy data for alignments of 1, 2 and 3
  for(uInt32 align = 1; align < 4; ++align)
  {
    for(number = 0; number < 8; ++number)
    {
      for(size = 0; size < 5; ++size)
      {
        for(x = 0; x < 320; ++x)
        {
          MxMask[align][number][size][x] = 
            MxMask[0][number][size][(x + 320 - align) % 320];
        }
      }
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// [4][4][320]
// [alignment][size][pixel]
void TIATables::buildBLMaskTable()
{
  // First, calculate masks for alignment 0
  for(Int32 size = 0; size < 4; ++size)
  {
    Int32 x;

    // Set all of the masks to false to start with
    for(x = 0; x < 160; ++x)
      BLMask[0][size][x] = false;

    // Set the necessary fields true
    for(x = 0; x < 160 + 8; ++x)
      if((x >= 0) && (x < (1 << size)))
        BLMask[0][size][x % 160] = true;

    // Copy fields into the wrap-around area of the mask
    for(x = 0; x < 160; ++x)
      BLMask[0][size][x + 160] = BLMask[0][size][x];
  }

  // Now, copy data for alignments of 1, 2 and 3
  for(uInt32 align = 1; align < 4; ++align)
    for(uInt32 size = 0; size < 4; ++size)
      for(uInt32 x = 0; x < 320; ++x)
        BLMask[align][size][x] = BLMask[0][size][(x + 320 - align) % 320];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// [2][160]
// [reflect, pixel]
// reflect=1: reflection on
// reflect=0: reflection off
void TIATables::buildPFMaskTable()
{
  Int32 x;

  // Compute playfield mask table for non-reflected mode
  for(x = 0; x < 160; ++x)
  {
    if(x < 16)
      PFMask[0][x] = 0x00001 << (x >> 2);
    else if(x < 48)
      PFMask[0][x] = 0x00800 >> ((x - 16) >> 2);
    else if(x < 80) 
      PFMask[0][x] = 0x01000 << ((x - 48) >> 2);
    else if(x < 96) 
      PFMask[0][x] = 0x00001 << ((x - 80) >> 2);
    else if(x < 128)
      PFMask[0][x] = 0x00800 >> ((x - 96) >> 2);
    else if(x < 160) 
      PFMask[0][x] = 0x01000 << ((x - 128) >> 2);
  }

  // Compute playfield mask table for reflected mode
  for(x = 0; x < 160; ++x)
  {
    if(x < 16)
      PFMask[1][x] = 0x00001 << (x >> 2);
    else if(x < 48)
      PFMask[1][x] = 0x00800 >> ((x - 16) >> 2);
    else if(x < 80) 
      PFMask[1][x] = 0x01000 << ((x - 48) >> 2);
    else if(x < 112) 
      PFMask[1][x] = 0x80000 >> ((x - 80) >> 2);
    else if(x < 144) 
      PFMask[1][x] = 0x00010 << ((x - 112) >> 2);
    else if(x < 160) 
      PFMask[1][x] = 0x00008 >> ((x - 144) >> 2);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIATables::buildGRPReflectTable()
{
  for(uInt16 i = 0; i < 256; ++i)
  {
    uInt8 r = 0;

    for(uInt16 t = 1; t <= 128; t <<= 1)
      r = (r << 1) | ((i & t) ? 0x01 : 0x00);

    GRPReflect[i] = r;
  } 
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// [8][160][160]
// [nusiz][old pixel][new pixel]
void TIATables::buildPxPosResetWhenTable()
{
  uInt32 nusiz, oldx, newx;

  // Loop through all player nusizs, all old player positions, and all new
  // player positions and determine where the new position is located:
  // 1 means the new position is within the display of an old copy of the
  // player, -1 means the new position is within the delay portion of an
  // old copy of the player, and 0 means it's neither of these two
  for(nusiz = 0; nusiz < 8; ++nusiz)
  {
    for(oldx = 0; oldx < 160; ++oldx)
    {
      // Set everything to 0 for non-delay/non-display section
      for(newx = 0; newx < 160; ++newx)
        PxPosResetWhen[nusiz][oldx][newx] = 0;

      // Now, we'll set the entries for non-delay/non-display section
      for(newx = 0; newx < 160 + 72 + 5; ++newx)
      {
        // nusiz:
        // 0: one copy
        // 1: two copies-close
        // 2: two copies-med
        // 3: three copies-close
        // 4: two copies-wide
        // 5: double size player
        // 6: 3 copies medium
        // 7: quad sized player
        switch(nusiz)
        {
          case 0x00:
            if((newx >= oldx) && (newx < (oldx + 4)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = -1;

            else if((newx >= oldx + 4) && (newx < (oldx + 4 + 8)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = 1;
            break;

          case 0x01:
            if((newx >= oldx) && (newx < (oldx + 4)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = -1;
            else if((newx >= (oldx + 16)) && (newx < (oldx + 16 + 4)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = -1;

            else if((newx >= oldx + 4) && (newx < (oldx + 4 + 8)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = 1;
            else if((newx >= oldx + 16 + 4) && (newx < (oldx + 16 + 4 + 8)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = 1;
            break;

          case 0x02:
            if((newx >= oldx) && (newx < (oldx + 4)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = -1;
            else if((newx >= (oldx + 32)) && (newx < (oldx + 32 + 4)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = -1;

            else if((newx >= oldx + 4) && (newx < (oldx + 4 + 8)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = 1;
            else if((newx >= oldx + 32 + 4) && (newx < (oldx + 32 + 4 + 8)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = 1;
            break;

          case 0x03:
            if((newx >= oldx) && (newx < (oldx + 4)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = -1;
            else if((newx >= (oldx + 16)) && (newx < (oldx + 16 + 4)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = -1;
            else if((newx >= (oldx + 32)) && (newx < (oldx + 32 + 4)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = -1;

            else if((newx >= oldx + 4) && (newx < (oldx + 4 + 8)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = 1;
            else if((newx >= oldx + 16 + 4) && (newx < (oldx + 16 + 4 + 8)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = 1;
            else if((newx >= oldx + 32 + 4) && (newx < (oldx + 32 + 4 + 8)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = 1;
            break;

          case 0x04:
            if((newx >= oldx) && (newx < (oldx + 4)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = -1;
            else if((newx >= (oldx + 64)) && (newx < (oldx + 64 + 4)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = -1;

            else if((newx >= oldx + 4) && (newx < (oldx + 4 + 8)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = 1;
            else if((newx >= oldx + 64 + 4) && (newx < (oldx + 64 + 4 + 8)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = 1;
            break;

          case 0x05:
            if((newx >= oldx) && (newx < (oldx + 4)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = -1;

            else if((newx >= oldx + 4) && (newx < (oldx + 4 + 16)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = 1;
            break;

          case 0x06:
            if((newx >= oldx) && (newx < (oldx + 4)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = -1;
            else if((newx >= (oldx + 32)) && (newx < (oldx + 32 + 4)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = -1;
            else if((newx >= (oldx + 64)) && (newx < (oldx + 64 + 4)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = -1;

            else if((newx >= oldx + 4) && (newx < (oldx + 4 + 8)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = 1;
            else if((newx >= oldx + 32 + 4) && (newx < (oldx + 32 + 4 + 8)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = 1;
            else if((newx >= oldx + 64 + 4) && (newx < (oldx + 64 + 4 + 8)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = 1;
            break;

          case 0x07:
            if((newx >= oldx) && (newx < (oldx + 4)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = -1;

            else if((newx >= oldx + 4) && (newx < (oldx + 4 + 32)))
              PxPosResetWhen[nusiz][oldx][newx % 160] = 1;
            break;
        }
      }

      // Let's do a sanity check on table entries
      uInt32 s1 = 0, s2 = 0;
      for(newx = 0; newx < 160; ++newx)
      {
        if(PxPosResetWhen[nusiz][oldx][newx] == -1)
          ++s1;
        if(PxPosResetWhen[nusiz][oldx][newx] == 1)
          ++s2;
      }
      assert((s1 % 4 == 0) && (s2 % 8 == 0));
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 TIATables::BLMask[4][4][320];

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 TIATables::CollisionMask[64];

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 TIATables::DisabledMask[640];

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const Int16 TIATables::PokeDelay[64] = {
   0,  1,  0,  0,  8,  8,  0,  0,  0,  0,  0,  1,  1, -1, -1, -1,
   0,  0,  8,  8,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 TIATables::MxMask[4][8][5][320];

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const bool TIATables::HMOVEBlankEnableCycles[76] = {
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,   // 00
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,   // 10
  true,  false, false, false, false, false, false, false, false, false,  // 20
  false, false, false, false, false, false, false, false, false, false,  // 30
  false, false, false, false, false, false, false, false, false, false,  // 40
  false, false, false, false, false, false, false, false, false, false,  // 50
  false, false, false, false, false, false, false, false, false, false,  // 60
  false, false, false, false, false, true                                // 70
};

#if 0
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const Int32 TIATables::CompleteMotion[76][16] = {
  { 0, -1, -2, -3, -4, -5, -6, -7,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -6, -7,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -6, -7,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -6, -7,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -6, -6,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -5, -5,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -5, -5,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -4, -4, -4,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -3, -3, -3, -3,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -2, -2, -2, -2, -2,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -2, -2, -2, -2, -2,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -1, -1, -1, -1, -1, -1,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0,  0,  0,  0,  0,  0,  0,  0,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 1,  1,  1,  1,  1,  1,  1,  1,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 1,  1,  1,  1,  1,  1,  1,  1,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 2,  2,  2,  2,  2,  2,  2,  2,  8,  7,  6,  5,  4,  3,  2,  2}, // HBLANK
  { 3,  3,  3,  3,  3,  3,  3,  3,  8,  7,  6,  5,  4,  3,  3,  3}, // HBLANK
  { 4,  4,  4,  4,  4,  4,  4,  4,  8,  7,  6,  5,  4,  4,  4,  4}, // HBLANK
  { 4,  4,  4,  4,  4,  4,  4,  4,  8,  7,  6,  5,  4,  4,  4,  4}, // HBLANK
  { 5,  5,  5,  5,  5,  5,  5,  5,  8,  7,  6,  5,  5,  5,  5,  5}, // HBLANK
  { 6,  6,  6,  6,  6,  6,  6,  6,  8,  7,  6,  6,  6,  6,  6,  6}, // HBLANK
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0,  0, -1, -2,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0, -1, -2, -3,  0,  0,  0,  0,  0,  0,  0,  0},    
  { 0,  0,  0,  0,  0, -1, -2, -3,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0, -1, -2, -3, -4,  0,  0,  0,  0,  0,  0,  0,  0}, 
  { 0,  0,  0, -1, -2, -3, -4, -5,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0, -1, -2, -3, -4, -5, -6,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0, -1, -2, -3, -4, -5, -6,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0, -1, -2, -3, -4, -5, -6, -7,  0,  0,  0,  0,  0,  0,  0,  0},
  {-1, -2, -3, -4, -5, -6, -7, -8,  0,  0,  0,  0,  0,  0,  0,  0},
  {-2, -3, -4, -5, -6, -7, -8, -9,  0,  0,  0,  0,  0,  0,  0, -1},
  {-2, -3, -4, -5, -6, -7, -8, -9,  0,  0,  0,  0,  0,  0,  0, -1},
  {-3, -4, -5, -6, -7, -8, -9,-10,  0,  0,  0,  0,  0,  0, -1, -2}, 
  {-4, -5, -6, -7, -8, -9,-10,-11,  0,  0,  0,  0,  0, -1, -2, -3},
  {-5, -6, -7, -8, -9,-10,-11,-12,  0,  0,  0,  0, -1, -2, -3, -4},
  {-5, -6, -7, -8, -9,-10,-11,-12,  0,  0,  0,  0, -1, -2, -3, -4},
  {-6, -7, -8, -9,-10,-11,-12,-13,  0,  0,  0, -1, -2, -3, -4, -5},
  {-7, -8, -9,-10,-11,-12,-13,-14,  0,  0, -1, -2, -3, -4, -5, -6},
  {-8, -9,-10,-11,-12,-13,-14,-15,  0, -1, -2, -3, -4, -5, -6, -7},
  {-8, -9,-10,-11,-12,-13,-14,-15,  0, -1, -2, -3, -4, -5, -6, -7},
  { 0, -1, -2, -3, -4, -5, -6, -7,  8,  7,  6,  5,  4,  3,  2,  1}  // HBLANK
};
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 TIATables::PxMask[4][2][8][320];

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Int8 TIATables::PxPosResetWhen[8][160][160];

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 TIATables::GRPReflect[256];

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 TIATables::PFMask[2][160];
