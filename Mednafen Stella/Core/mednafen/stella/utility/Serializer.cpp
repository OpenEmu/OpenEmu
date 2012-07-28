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

Int32 smem_read(StateMem *st, void *buffer, uInt32 len)
{
    if((len + st->loc) > st->len)
        return(0);
    
    memcpy(buffer, st->data + st->loc, len);
    st->loc += len;
    
    return(len);
}

int smem_read32le(StateMem *st, uInt32 *b)
{
    uInt8 s[4];
    
    if(smem_read(st, s, 4) < 4)
        return(0);
    
    *b = s[0] | (s[1] << 8) | (s[2] << 16) | (s[3] << 24);
    
    return(4);
}

Int32 smem_write(StateMem *st, void *buffer, uInt32 len)
{
    if((len + st->loc) > st->malloced)
    {
        uInt32 newsize = (st->malloced >= 32768) ? st->malloced : (st->initial_malloc ? st->initial_malloc : 32768);
        
        while(newsize < (len + st->loc))
            newsize *= 2;
        st->data = (uInt8 *)realloc(st->data, newsize);
        st->malloced = newsize;
    }
    memcpy(st->data + st->loc, buffer, len);
    st->loc += len;
    
    if(st->loc > st->len) st->len = st->loc;
    
    return(len);
}

int smem_write32le(StateMem *st, uInt32 b)
{
    uInt8 s[4];
    s[0]=b;
    s[1]=b>>8;
    s[2]=b>>16;
    s[3]=b>>24;
    return((smem_write(st, s, 4)<4)?0:4);
}

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
  char buf;
  smem_read(myStream, &buf, 1);
  return buf;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int Serializer::getInt(void)
{
  Int32 value;
  smem_read32le(myStream, (uInt32*)&value);
  return value;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string Serializer::getString(void)
{
  int len = getInt();
  string str;
  str.resize((string::size_type)len);
  smem_read(myStream, &str[0], len);
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
  smem_write(myStream, &value, 1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Serializer::putInt(int value)
{
  smem_write32le(myStream, value);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Serializer::putString(const string& str)
{
  uInt32 len = str.length();
  smem_write32le(myStream, len);
  smem_write(myStream, (void*)str.data(), len);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Serializer::putBool(bool b)
{
  putByte(b ? TruePattern : FalsePattern);
}

