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
// $Id: StringList.hxx 2234 2011-05-26 16:14:46Z stephena $
//============================================================================

#ifndef STRING_LIST_HXX
#define STRING_LIST_HXX

#include "Array.hxx"
#include "bspf.hxx"


class StringList : public Common::Array<string>
{
  public:
    void push_back(const char *str)
    {
      ensureCapacity(_size + 1);
      _data[_size++] = str;
    }

    void push_back(const string& str)
    {
      ensureCapacity(_size + 1);
      _data[_size++] = str;
    }

    static string removePattern(const string& str, const string& pattern)
    {
      // This can probably be made more efficient ...
      string tmp;
      for(unsigned int i = 0; i < str.length(); ++i)
      {
        bool match = false;
        for(unsigned int j = 0; j < pattern.length(); ++j)
        {
          if(str[i] == pattern[j])
          {
            match = true;
            break;
          }
        }
        if(!match) tmp += str[i];
      }
      return tmp;
    }
};

class StringMap : public Common::Array< pair<string,string> >
{
  public:
    void push_back(const string& name, const string& tag)
    {
      ensureCapacity(_size + 1);
      _data[_size++] = make_pair(name, tag);
    }
};

#endif
