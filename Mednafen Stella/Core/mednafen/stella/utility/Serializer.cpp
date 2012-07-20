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
// $Id: Serializer.cxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

//ROBO: Entire file is modified, diff with original for details...
//ROBO: Use StateMem as stream backend
//#include <mednafen/mednafen.h>

#include <fstream>
#include <sstream>

#include "Serializer.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Serializer::Serializer(StateMem* stream)
  : myStream(stream)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Serializer::~Serializer(void)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Serializer::isValid(void)
{
  return myStream != NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Serializer::reset(void)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
char Serializer::getByte(void)
{
  //char buf;
  //smem_read(myStream, &buf, 1);
  //return buf;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int Serializer::getInt(void)
{
  //int32 value;
  //smem_read32le(myStream, (uint32*)&value);
  //return value;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string Serializer::getString(void)
{
  int len = getInt();
  string str;
  str.resize((string::size_type)len);
  //smem_read(myStream, &str[0], len);
  return str;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Serializer::getBool(void)
{
  char b = getByte();
  if(b == (char)TruePattern)
    return true;
  else if(b == (char)FalsePattern)
    return false;
  else
    throw "Serializer::getBool() data corruption";

  return false;  // to stop compiler from complaining
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Serializer::putByte(char value)
{
  //smem_write(myStream, &value, 1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Serializer::putInt(int value)
{
  //smem_write32le(myStream, value);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Serializer::putString(const string& str)
{
  //uint32 len = str.length();
  //smem_write32le(myStream, len);
  //smem_write(myStream, (void*)str.data(), len);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Serializer::putBool(bool b)
{
  putByte(b ? TruePattern : FalsePattern);
}

