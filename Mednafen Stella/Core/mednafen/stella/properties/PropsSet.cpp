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
// $Id: PropsSet.cxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#include <fstream>
#include <sstream>
#include <map>

#include "bspf.hxx"

#include "DefProps.hxx"
//ROBO: No OSystem
//#include "OSystem.hxx"
#include "Props.hxx"
#include "Settings.hxx"

#include "PropsSet.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//ROBO: No OSystem
PropertiesSet::PropertiesSet(void* osystem)
//  : myOSystem(osystem),
  :  mySize(0)
{
//ROBO: No OSystem
//  const string& props = myOSystem->propertiesFile();
  //TODO: LOAD FROM MEDNAFEN FIRMWARE FOLDER?
//  load(props);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
PropertiesSet::~PropertiesSet()
{
  myExternalProps.clear();
  myTempProps.clear();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PropertiesSet::load(const string& filename)
{
  ifstream in(filename.c_str(), ios::in);

  // Loop reading properties
  for(;;)
  {
    // Make sure the stream is still good or we're done 
    if(!in)
      break;

    // Get the property list associated with this profile
    Properties prop;
    prop.load(in);

    // If the stream is still good then insert the properties
    if(in)
      insert(prop);
  }
  if(in)
    in.close();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool PropertiesSet::save(const string& filename) const
{
  ofstream out(filename.c_str(), ios::out);
  if(!out)
    return false;

  // Only save those entries in the external list
  for(PropsList::const_iterator i = myExternalProps.begin();
      i != myExternalProps.end(); ++i)
    i->second.save(out);

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool PropertiesSet::getMD5(const string& md5, Properties& properties,
                           bool useDefaults) const
{
  properties.setDefaults();
  bool found = false;

  // There are three lists to search when looking for a properties entry,
  // which must be done in the following order
  // If 'useDefaults' is specified, only use the built-in list
  //
  //  'save': entries previously inserted that are saved on program exit
  //  'temp': entries previously inserted that are discarded
  //  'builtin': the defaults compiled into the program

  // First check properties from external file
  if(!useDefaults)
  {
    // Check external list
    PropsList::const_iterator iter = myExternalProps.find(md5);
    if(iter != myExternalProps.end())
    {
      properties = iter->second;
      found = true;
    }
    else  // Search temp list
    {
      iter = myTempProps.find(md5);
      if(iter != myTempProps.end())
      {
        properties = iter->second;
        found = true;
      }
    }
  }

  // Otherwise, search the internal database using binary search
  if(!found)
  {
    int low = 0, high = DEF_PROPS_SIZE - 1;
    while(low <= high)
    {
      int i = (low + high) / 2;
      int cmp = strncmp(md5.c_str(), DefProps[i][Cartridge_MD5], 32);

      if(cmp == 0)  // found it
      {
        for(int p = 0; p < LastPropType; ++p)
          if(DefProps[i][p][0] != 0)
            properties.set((PropertyType)p, DefProps[i][p]);

        found = true;
        break;
      }
      else if(cmp < 0)
        high = i - 1; // look at lower range
      else
        low = i + 1;  // look at upper range
    }
  }

  return found;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PropertiesSet::insert(const Properties& properties, bool save)
{
  // Note that the following code is optimized for insertion when an item
  // doesn't already exist, and when the external properties file is
  // relatively small (which is the case with current versions of Stella,
  // as the properties are built-in)
  // If an item does exist, it will be removed and insertion done again
  // This shouldn't be a speed issue, as insertions will only fail with
  // duplicates when you're changing the current ROM properties, which
  // most people tend not to do

  // Since the PropSet is keyed by md5, we can't insert without a valid one
  const string& md5 = properties.get(Cartridge_MD5);
  if(md5 == "")
    return;

  // The status of 'save' determines which list to save to
  PropsList& list = save ? myExternalProps : myTempProps;

  pair<PropsList::iterator,bool> ret;
  ret = list.insert(make_pair(md5, properties));
  if(ret.second == false)
  {
    // Remove old item and insert again
    list.erase(ret.first);
    list.insert(make_pair(md5, properties));
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PropertiesSet::removeMD5(const string& md5)
{
  // We only remove from the external list
  myExternalProps.erase(md5);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PropertiesSet::print() const
{
  // We only look at the external properties and the built-in ones;
  // the temp properties are ignored
  // Also, any properties entries in the external file override the built-in
  // ones
  // The easiest way to merge the lists is to create another temporary one
  // This isn't fast, but I suspect this method isn't used too often (or at all)

  PropsList list;

  // First insert all external props
  list = myExternalProps;

  // Now insert all the built-in ones
  // Note that if we try to insert a duplicate, the insertion will fail
  // This is fine, since a duplicate in the built-in list means it should
  // be overrided anyway (and insertion shouldn't be done)
  Properties properties;
  for(int i = 0; i < DEF_PROPS_SIZE; ++i)
  {
    properties.setDefaults();
    for(int p = 0; p < LastPropType; ++p)
      if(DefProps[i][p][0] != 0)
        properties.set((PropertyType)p, DefProps[i][p]);

    list.insert(make_pair(DefProps[i][Cartridge_MD5], properties));
  }

  // Now, print the resulting list
  Properties::printHeader();
  for(PropsList::const_iterator i = list.begin(); i != list.end(); ++i)
    i->second.print();
}
