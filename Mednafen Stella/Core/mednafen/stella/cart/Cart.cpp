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
// $Id: Cart.cxx 2249 2011-06-07 13:40:59Z stephena $
//============================================================================

#include <cassert>
#include <cstring>
#include <sstream>

#include "bspf.hxx"
#include "Cart.hxx"
#include "Cart0840.hxx"
#include "Cart2K.hxx"
#include "Cart3E.hxx"
#include "Cart3F.hxx"
#include "Cart4A50.hxx"
#include "Cart4K.hxx"
#include "CartAR.hxx"
#include "CartDPC.hxx"
#include "CartDPCPlus.hxx"
#include "CartE0.hxx"
#include "CartE7.hxx"
#include "CartEF.hxx"
#include "CartEFSC.hxx"
#include "CartF0.hxx"
#include "CartF4.hxx"
#include "CartF4SC.hxx"
#include "CartF6.hxx"
#include "CartF6SC.hxx"
#include "CartF8.hxx"
#include "CartF8SC.hxx"
#include "CartFA.hxx"
#include "CartFE.hxx"
#include "CartMC.hxx"
#include "CartCV.hxx"
#include "CartUA.hxx"
#include "CartSB.hxx"
#include "CartX07.hxx"
#include "MD5.hxx"
#include "Props.hxx"
#include "Settings.hxx"
#ifdef DEBUGGER_SUPPORT
  #include "Debugger.hxx"
  #include "CartDebug.hxx"
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge* Cartridge::create(const uInt8* image, uInt32 size, string& md5,
     string& dtype, string& id, Settings& settings)
{
  Cartridge* cartridge = 0;
  string type = dtype;

  // Collect some info about the ROM
  ostringstream buf;

  // See if we should try to auto-detect the cartridge type
  // If we ask for extended info, always do an autodetect
  string autodetect = "";
  if(type == "AUTO-DETECT" || settings.getBool("rominfo"))
  {
    const string& detected = autodetectType(image, size);
    autodetect = "*";
    if(type != "AUTO-DETECT" && type != detected)
      cerr << "Auto-detection not consistent: " << type << ", " << detected << endl;

    type = detected;
  }
  buf << type << autodetect;

  // Check for multicart first; if found, get the correct part of the image
  if(type == "2IN1")
  {
    // Make sure we have a valid sized image
    if(size == 2*2048 || size == 2*4096 || size == 2*8192 || size == 2*16384)
    {
      dtype = type;
      type = createFromMultiCart(image, size, 2, md5, id, settings);
      buf << id;
    }

  }
  else if(type == "4IN1")
  {
    // Make sure we have a valid sized image
    if(size == 4*2048 || size == 4*4096 || size == 4*8192)
    {
      dtype = type;
      type = createFromMultiCart(image, size, 4, md5, id, settings);
      buf << id;
    }
  }
  else if(type == "8IN1")
  {
    // Make sure we have a valid sized image
    if(size == 8*2048 || size == 8*4096 || size == 8*8192)
    {
      dtype = type;
      type = createFromMultiCart(image, size, 8, md5, id, settings);
      buf << id;
    }
  }
  else if(type == "16IN1")
  {
    // Make sure we have a valid sized image
    if(size == 16*2048 || size == 16*4096 || size == 16*8192)
    {
      dtype = type;
      type = createFromMultiCart(image, size, 16, md5, id, settings);
      buf << id;
    }
  }
  else if(type == "32IN1")
  {
    // Make sure we have a valid sized image
    if(size == 32*2048 || size == 32*4096)
    {
      dtype = type;
      type = createFromMultiCart(image, size, 32, md5, id, settings);
      buf << id;
    }
  }

  // We should know the cart's type by now so let's create it
  if(type == "2K")
    cartridge = new Cartridge2K(image, size, settings);
  else if(type == "3E")
    cartridge = new Cartridge3E(image, size, settings);
  else if(type == "3F")
    cartridge = new Cartridge3F(image, size, settings);
  else if(type == "4A50")
    cartridge = new Cartridge4A50(image, size, settings);
  else if(type == "4K")
    cartridge = new Cartridge4K(image, settings);
  else if(type == "AR")
    cartridge = new CartridgeAR(image, size, settings);
  else if(type == "DPC")
    cartridge = new CartridgeDPC(image, size, settings);
  else if(type == "DPC+")
    cartridge = new CartridgeDPCPlus(image, size, settings);
  else if(type == "E0")
    cartridge = new CartridgeE0(image, settings);
  else if(type == "E7")
    cartridge = new CartridgeE7(image, settings);
  else if(type == "EF")
    cartridge = new CartridgeEF(image, settings);
  else if(type == "EFSC")
    cartridge = new CartridgeEFSC(image, settings);
  else if(type == "F4")
    cartridge = new CartridgeF4(image, settings);
  else if(type == "F4SC")
    cartridge = new CartridgeF4SC(image, settings);
  else if(type == "F6")
    cartridge = new CartridgeF6(image, settings);
  else if(type == "F6SC")
    cartridge = new CartridgeF6SC(image, settings);
  else if(type == "F8")
    cartridge = new CartridgeF8(image, md5, settings);
  else if(type == "F8SC")
    cartridge = new CartridgeF8SC(image, settings);
  else if(type == "FA" || type == "FASC")
    cartridge = new CartridgeFA(image, settings);
  else if(type == "FE")
    cartridge = new CartridgeFE(image, settings);
  else if(type == "MC")
    cartridge = new CartridgeMC(image, size, settings);
  else if(type == "F0" || type == "MB")
    cartridge = new CartridgeF0(image, settings);
  else if(type == "CV")
    cartridge = new CartridgeCV(image, size, settings);
  else if(type == "UA")
    cartridge = new CartridgeUA(image, settings);
  else if(type == "0840")
    cartridge = new Cartridge0840(image, settings);
  else if(type == "SB")
    cartridge = new CartridgeSB(image, size, settings);
  else if(type == "X07")
    cartridge = new CartridgeX07(image, settings);
  else
    cerr << "ERROR: Invalid cartridge type " << type << " ..." << endl;

  if(size < 1024)
    buf << " (" << size << "B) ";
  else
    buf << " (" << (size/1024) << "K) ";
  myAboutString = buf.str();

  return cartridge;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string Cartridge::createFromMultiCart(const uInt8*& image, uInt32& size,
    uInt32 numroms, string& md5, string& id, Settings& settings)
{
  // Get a piece of the larger image
  uInt32 i = settings.getInt("romloadcount");
  size /= numroms;
  image += i*size;

  // We need a new md5 and name
  md5 = MD5(image, size);
  ostringstream buf;
  buf << " [G" << (i+1) << "]";
  id = buf.str();

  // Move to the next game the next time this ROM is loaded
  settings.setInt("romloadcount", (i+1)%numroms);

  if(size <= 2048)       return "2K";
  else if(size == 4096)  return "4K";
  else if(size == 8192)  return "F8";
  else  /* default */    return "4K";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge::Cartridge(const Settings& settings)
  : mySettings(settings),
    myStartBank(0),
    myBankChanged(true),
    myCodeAccessBase(NULL),
    myBankLocked(false)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge::~Cartridge()
{
  if(myCodeAccessBase)
    delete[] myCodeAccessBase;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::save(ofstream& out)
{
  int size = -1;

  const uInt8* image = getImage(size);
  if(image == 0 || size <= 0)
  {
    cerr << "save not supported" << endl;
    return false;
  }

  for(int i=0; i<size; i++)
    out << image[i];

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 Cartridge::startBank()
{
  return myStartBank;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::bankChanged()
{
  bool changed = myBankChanged;
  myBankChanged = false;
  return changed;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge::registerRamArea(uInt16 start, uInt16 size,
                                uInt16 roffset, uInt16 woffset)
{
#ifdef DEBUGGER_SUPPORT
  RamArea area;
  area.start   = start;
  area.size    = size;
  area.roffset = roffset;
  area.woffset = woffset;
  myRamAreaList.push_back(area);
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge::triggerReadFromWritePort(uInt16 address)
{
#ifdef DEBUGGER_SUPPORT
  Debugger::debugger().cartDebug().triggerReadFromWritePort(address);
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge::createCodeAccessBase(uInt32 size)
{
#ifdef DEBUGGER_SUPPORT
  myCodeAccessBase = new uInt8[size];
  memset(myCodeAccessBase, CartDebug::ROW, size);
#else
  myCodeAccessBase = NULL;
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string Cartridge::autodetectType(const uInt8* image, uInt32 size)
{
  // Guess type based on size
  const char* type = 0;

  if((size % 8448) == 0 || size == 6144)
  {
    type = "AR";
  }
  else if(size < 2048)  // Sub2K images
  {
    type = "2K";
  }
  else if((size == 2048) ||
          (size == 4096 && memcmp(image, image + 2048, 2048) == 0))
  {
    type = isProbablyCV(image, size) ? "CV" : "2K";
  }
  else if(size == 4096)
  {
    type = isProbablyCV(image, size) ? "CV" : "4K";
  }
  else if(size == 8*1024)  // 8K
  {
    if(isProbablySC(image, size))
      type = "F8SC";
    else if(memcmp(image, image + 4096, 4096) == 0)
      type = "4K";
    else if(isProbablyE0(image, size))
      type = "E0";
    else if(isProbably3E(image, size))
      type = "3E";
    else if(isProbably3F(image, size))
      type = "3F";
    else if(isProbablyUA(image, size))
      type = "UA";
    else if(isProbablyFE(image, size))
      type = "FE";
    else if(isProbably0840(image, size))
      type = "0840";
    else
      type = "F8";
  }
  else if(size >= 10240 && size <= 10496)  // ~10K - Pitfall2
  {
    type = "DPC";
  }
  else if(size == 12*1024)  // 12K
  {
    type = "FA";
  }
  else if(size == 16*1024)  // 16K
  {
    if(isProbablySC(image, size))
      type = "F6SC";
    else if(isProbablyE7(image, size))
      type = "E7";
    else if(isProbably3E(image, size))
      type = "3E";
    else if(isProbably3F(image, size))
      type = "3F";
    else
      type = "F6";
  }
  else if(size == 29*1024)  // 29K
  {
    type = "DPC+";
  }
  else if(size == 32*1024)  // 32K
  {
    if(isProbablySC(image, size))
      type = "F4SC";
    else if(isProbably3E(image, size))
      type = "3E";
    else if(isProbably3F(image, size))
      type = "3F";
    else if(isProbablyDPCplus(image, size))
      type = "DPC+";
    else
      type = "F4";
  }
  else if(size == 64*1024)  // 64K
  {
    if(isProbably3E(image, size))
      type = "3E";
    else if(isProbably3F(image, size))
      type = "3F";
    else if(isProbably4A50(image, size))
      type = "4A50";
    else if(isProbablyEF(image, size))
      type = isProbablySC(image, size) ? "EFSC" : "EF";
    else if(isProbablyX07(image, size))
      type = "X07";
    else
      type = "F0";
  }
  else if(size == 128*1024)  // 128K
  {
    if(isProbably3E(image, size))
      type = "3E";
    else if(isProbably3F(image, size))
      type = "3F";
    else if(isProbably4A50(image, size))
      type = "4A50";
    else if(isProbablySB(image, size))
      type = "SB";
    else
      type = "MC";
  }
  else if(size == 256*1024)  // 256K
  {
    if(isProbably3E(image, size))
      type = "3E";
    else if(isProbably3F(image, size))
      type = "3F";
    else /*if(isProbablySB(image, size))*/
      type = "SB";
  }
  else  // what else can we do?
  {
    if(isProbably3E(image, size))
      type = "3E";
    else if(isProbably3F(image, size))
      type = "3F";
    else
      type = "4K";  // Most common bankswitching type
  }

  return type;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::searchForBytes(const uInt8* image, uInt32 imagesize,
                               const uInt8* signature, uInt32 sigsize,
                               uInt32 minhits)
{
  uInt32 count = 0;
  for(uInt32 i = 0; i < imagesize - sigsize; ++i)
  {
    uInt32 matches = 0;
    for(uInt32 j = 0; j < sigsize; ++j)
    {
      if(image[i+j] == signature[j])
        ++matches;
      else
        break;
    }
    if(matches == sigsize)
    {
      ++count;
      i += sigsize;  // skip past this signature 'window' entirely
    }
    if(count >= minhits)
      break;
  }

  return (count >= minhits);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablySC(const uInt8* image, uInt32 size)
{
  // We assume a Superchip cart contains the same bytes for its entire
  // RAM area; obviously this test will fail if it doesn't
  // The RAM area will be the first 256 bytes of each 4K bank
  uInt32 banks = size / 4096;
  for(uInt32 i = 0; i < banks; ++i)
  {
    uInt8 first = image[i*4096];
    for(uInt32 j = 0; j < 256; ++j)
    {
      if(image[i*4096+j] != first)
        return false;
    }
  }
  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbably0840(const uInt8* image, uInt32 size)
{
  // 0840 cart bankswitching is triggered by accessing addresses 0x0800
  // or 0x0840
  uInt8 signature1[2][3] = {
    { 0xAD, 0x00, 0x08 },  // LDA $0800
    { 0xAD, 0x40, 0x08 }   // LDA $0840
  };
  for(uInt32 i = 0; i < 2; ++i)
    if(searchForBytes(image, size, signature1[i], 3, 1))
      return true;

  uInt8 signature2[2][4] = {
    { 0x0C, 0x00, 0x08, 0x4C },  // NOP $0800; JMP ...
    { 0x0C, 0xFF, 0x0F, 0x4C }   // NOP $0FFF; JMP ...
  };
  for(uInt32 i = 0; i < 2; ++i)
    if(searchForBytes(image, size, signature2[i], 4, 1))
      return true;

  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbably3E(const uInt8* image, uInt32 size)
{
  // 3E cart bankswitching is triggered by storing the bank number
  // in address 3E using 'STA $3E', commonly followed by an
  // immediate mode LDA
  uInt8 signature[] = { 0x85, 0x3E, 0xA9, 0x00 };  // STA $3E; LDA #$00
  return searchForBytes(image, size, signature, 4, 1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbably3F(const uInt8* image, uInt32 size)
{
  // 3F cart bankswitching is triggered by storing the bank number
  // in address 3F using 'STA $3F'
  // We expect it will be present at least 2 times, since there are
  // at least two banks
  uInt8 signature[] = { 0x85, 0x3F };  // STA $3F
  return searchForBytes(image, size, signature, 2, 2);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbably4A50(const uInt8* image, uInt32 size)
{
  // 4A50 carts store address $4A50 at the NMI vector, which
  // in this scheme is always in the last page of ROM at
  // $1FFA - $1FFB (at least this is true in rev 1 of the format)
  if(image[size-6] == 0x50 && image[size-5] == 0x4A)
    return true;

  // Program starts at $1Fxx with NOP $6Exx or NOP $6Fxx?
  if(((image[0xfffd] & 0x1f) == 0x1f) &&
      (image[image[0xfffd] * 256 + image[0xfffc]] == 0x0c) &&
      ((image[image[0xfffd] * 256 + image[0xfffc] + 2] & 0xfe) == 0x6e))
    return true;

  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyCV(const uInt8* image, uInt32 size)
{
  // CV RAM access occurs at addresses $f3ff and $f400
  // These signatures are attributed to the MESS project
  uInt8 signature[2][3] = {
    { 0x9D, 0xFF, 0xF3 },  // STA $F3FF
    { 0x99, 0x00, 0xF4 }   // STA $F400
  };
  if(searchForBytes(image, size, signature[0], 3, 1))
    return true;
  else
    return searchForBytes(image, size, signature[1], 3, 1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyDPCplus(const uInt8* image, uInt32 size)
{
  // DPC+ ARM code has 2 occurrences of the string DPC+
  uInt8 signature[] = { 0x44, 0x50, 0x43, 0x2B };  // DPC+
  return searchForBytes(image, size, signature, 4, 2);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyE0(const uInt8* image, uInt32 size)
{
  // E0 cart bankswitching is triggered by accessing addresses
  // $FE0 to $FF9 using absolute non-indexed addressing
  // To eliminate false positives (and speed up processing), we
  // search for only certain known signatures
  // Thanks to "stella@casperkitty.com" for this advice
  // These signatures are attributed to the MESS project
  uInt8 signature[8][3] = {
   { 0x8D, 0xE0, 0x1F },  // STA $1FE0
   { 0x8D, 0xE0, 0x5F },  // STA $5FE0
   { 0x8D, 0xE9, 0xFF },  // STA $FFE9
   { 0x0C, 0xE0, 0x1F },  // NOP $1FE0
   { 0xAD, 0xE0, 0x1F },  // LDA $1FE0
   { 0xAD, 0xE9, 0xFF },  // LDA $FFE9
   { 0xAD, 0xED, 0xFF },  // LDA $FFED
   { 0xAD, 0xF3, 0xBF }   // LDA $BFF3
  };
  for(uInt32 i = 0; i < 8; ++i)
    if(searchForBytes(image, size, signature[i], 3, 1))
      return true;

  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyE7(const uInt8* image, uInt32 size)
{
  // E7 cart bankswitching is triggered by accessing addresses
  // $FE0 to $FE6 using absolute non-indexed addressing
  // To eliminate false positives (and speed up processing), we
  // search for only certain known signatures
  // Thanks to "stella@casperkitty.com" for this advice
  // These signatures are attributed to the MESS project
  uInt8 signature[7][3] = {
   { 0xAD, 0xE2, 0xFF },  // LDA $FFE2
   { 0xAD, 0xE5, 0xFF },  // LDA $FFE5
   { 0xAD, 0xE5, 0x1F },  // LDA $1FE5
   { 0xAD, 0xE7, 0x1F },  // LDA $1FE7
   { 0x0C, 0xE7, 0x1F },  // NOP $1FE7
   { 0x8D, 0xE7, 0xFF },  // STA $FFE7
   { 0x8D, 0xE7, 0x1F }   // STA $1FE7
  };
  for(uInt32 i = 0; i < 7; ++i)
    if(searchForBytes(image, size, signature[i], 3, 1))
      return true;

  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyEF(const uInt8* image, uInt32 size)
{
  // EF cart bankswitching switches banks by accessing addresses 0xFE0
  // to 0xFEF, usually with either a NOP or LDA
  // It's likely that the code will switch to bank 0, so that's what is tested
  uInt8 signature[2][3] = {
    { 0x0C, 0xE0, 0xFF },  // NOP $FFE0
    { 0xAD, 0xE0, 0xFF }   // LDA $FFE0
  };
  if(searchForBytes(image, size, signature[0], 3, 1))
    return true;
  else
    return searchForBytes(image, size, signature[1], 3, 1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyFE(const uInt8* image, uInt32 size)
{
  // FE bankswitching is very weird, but always seems to include a
  // 'JSR $xxxx'
  // These signatures are attributed to the MESS project
  uInt8 signature[4][5] = {
    { 0x20, 0x00, 0xD0, 0xC6, 0xC5 },  // JSR $D000; DEC $C5
    { 0x20, 0xC3, 0xF8, 0xA5, 0x82 },  // JSR $F8C3; LDA $82
    { 0xD0, 0xFB, 0x20, 0x73, 0xFE },  // BNE $FB; JSR $FE73
    { 0x20, 0x00, 0xF0, 0x84, 0xD6 }   // JSR $F000; STY $D6
  };
  for(uInt32 i = 0; i < 4; ++i)
    if(searchForBytes(image, size, signature[i], 5, 1))
      return true;

  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablySB(const uInt8* image, uInt32 size)
{
  // SB cart bankswitching switches banks by accessing address 0x0800
  uInt8 signature[2][3] = {
    { 0xBD, 0x00, 0x08 },  // LDA $0800,x
    { 0xAD, 0x00, 0x08 }   // LDA $0800
  };
  if(searchForBytes(image, size, signature[0], 3, 1))
    return true;
  else
    return searchForBytes(image, size, signature[1], 3, 1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyUA(const uInt8* image, uInt32 size)
{
  // UA cart bankswitching switches to bank 1 by accessing address 0x240
  // using 'STA $240' or 'LDA $240'
  uInt8 signature[3][3] = {
    { 0x8D, 0x40, 0x02 },  // STA $240
    { 0xAD, 0x40, 0x02 },  // LDA $240
    { 0xBD, 0x1F, 0x02 }   // LDA $21F,X
  };
  for(uInt32 i = 0; i < 3; ++i)
    if(searchForBytes(image, size, signature[i], 3, 1))
      return true;

  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyX07(const uInt8* image, uInt32 size)
{
  // X07 bankswitching switches to bank 0, 1, 2, etc by accessing address 0x08xd
  uInt8 signature[3][3] = {
    { 0xAD, 0x0D, 0x08 },  // LDA $080D
    { 0xAD, 0x1D, 0x08 },  // LDA $081D
    { 0xAD, 0x2D, 0x08 }   // LDA $082D
  };
  for(uInt32 i = 0; i < 3; ++i)
    if(searchForBytes(image, size, signature[i], 3, 1))
      return true;

  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge::Cartridge(const Cartridge& cart)
  : mySettings(cart.mySettings)
{
  assert(false);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge& Cartridge::operator = (const Cartridge&)
{
  assert(false);
  return *this;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string Cartridge::myAboutString= "";
