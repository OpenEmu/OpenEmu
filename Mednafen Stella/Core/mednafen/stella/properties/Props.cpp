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
// $Id: Props.cxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#include <cctype>
#include <algorithm>
#include <sstream>

#include "bspf.hxx"
#include "Props.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Properties::Properties()
{
  setDefaults();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Properties::Properties(const Properties& properties)
{
  copy(properties);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Properties::~Properties()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const string& Properties::get(PropertyType key) const
{
  if(key >= 0 && key < LastPropType)
    return myProperties[key];
  else
    return EmptyString;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Properties::set(PropertyType key, const string& value)
{
  if(key >= 0 && key < LastPropType)
  {
    myProperties[key] = value;

    switch(key)
    {
      case Cartridge_Sound:
      case Cartridge_Type:
      case Console_LeftDifficulty:
      case Console_RightDifficulty:
      case Console_TelevisionType:
      case Console_SwapPorts:
      case Controller_Left:
      case Controller_Right:
      case Controller_SwapPaddles:
      case Display_Format:
      case Display_Phosphor:
      {
        transform(myProperties[key].begin(), myProperties[key].end(),
                  myProperties[key].begin(), (int(*)(int)) toupper);
        break;
      }

      case Display_PPBlend:
      {
        int blend = atoi(myProperties[key].c_str());
        if(blend < 0 || blend > 100) blend = 77;
        ostringstream buf;
        buf << blend;
        myProperties[key] = buf.str();
        break;
      }

      default:
        break;
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Properties::load(istream& in)
{
  setDefaults();

  // Loop reading properties
  string key, value;
  for(;;)
  {
    // Get the key associated with this property
    key = readQuotedString(in);

    // Make sure the stream is still okay
    if(!in)
      return;

    // A null key signifies the end of the property list
    if(key == "")
      break;

    // Get the value associated with this property
    value = readQuotedString(in);

    // Make sure the stream is still okay
    if(!in)
      return;

    // Set the property 
    PropertyType type = getPropertyType(key);
    set(type, value);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Properties::save(ostream& out) const
{
  // Write out each of the key and value pairs
  bool changed = false;
  for(int i = 0; i < LastPropType; ++i)
  {
    // Try to save some space by only saving the items that differ from default
    if(myProperties[i] != ourDefaultProperties[i])
    {
      writeQuotedString(out, ourPropertyNames[i]);
      out.put(' ');
      writeQuotedString(out, myProperties[i]);
      out.put('\n');
      changed = true;
    }
  }

  if(changed)
  {
    // Put a trailing null string so we know when to stop reading
    writeQuotedString(out, "");
    out.put('\n');
    out.put('\n');
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string Properties::readQuotedString(istream& in)
{
  char c;

  // Read characters until we see a quote
  while(in.get(c))
  {
    if(c == '"')
      break;
  }

  // Read characters until we see the close quote
  string s;
  while(in.get(c))
  {
    if((c == '\\') && (in.peek() == '"'))
      in.get(c);
    else if((c == '\\') && (in.peek() == '\\'))
      in.get(c);
    else if(c == '"')
      break;
    else if(c == '\r')
      continue;

    s += c;
  }

  return s;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Properties::writeQuotedString(ostream& out, const string& s)
{
  out.put('"');
  for(uInt32 i = 0; i < s.length(); ++i)
  {
    if(s[i] == '\\')
    {
      out.put('\\');
      out.put('\\');
    }
    else if(s[i] == '\"')
    {
      out.put('\\');
      out.put('"');
    }
    else
      out.put(s[i]);
  }
  out.put('"');
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Properties& Properties::operator = (const Properties& properties)
{
  // Do the assignment only if this isn't a self assignment
  if(this != &properties)
  {
    // Now, make myself a copy of the given object
    copy(properties);
  }

  return *this;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Properties::copy(const Properties& properties)
{
  // Now, copy each property from properties
  for(int i = 0; i < LastPropType; ++i)
    myProperties[i] = properties.myProperties[i];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Properties::print() const
{
  cout << get(Cartridge_MD5)          << "|"
       << get(Cartridge_Name)         << "|"
       << get(Cartridge_Manufacturer) << "|"
       << get(Cartridge_ModelNo)      << "|"
       << get(Cartridge_Note)         << "|"
       << get(Cartridge_Rarity)       << "|"
       << get(Cartridge_Sound)        << "|"
       << get(Cartridge_Type)         << "|"
       << get(Console_LeftDifficulty) << "|"
       << get(Console_RightDifficulty)<< "|"
       << get(Console_TelevisionType) << "|"
       << get(Console_SwapPorts)      << "|"
       << get(Controller_Left)        << "|"
       << get(Controller_Right)       << "|"
       << get(Controller_SwapPaddles) << "|"
       << get(Display_Format)         << "|"
       << get(Display_YStart)         << "|"
       << get(Display_Height)         << "|"
       << get(Display_Phosphor)       << "|"
       << get(Display_PPBlend)
       << endl;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Properties::setDefaults()
{
  for(int i = 0; i < LastPropType; ++i)
    myProperties[i] = ourDefaultProperties[i];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
PropertyType Properties::getPropertyType(const string& name)
{
  for(int i = 0; i < LastPropType; ++i)
    if(ourPropertyNames[i] == name)
      return (PropertyType)i;

  // Otherwise, indicate that the item wasn't found
  return LastPropType;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Properties::printHeader()
{
  cout << "Cartridge_MD5|"
       << "Cartridge_Name|"
       << "Cartridge_Manufacturer|"
       << "Cartridge_ModelNo|"
       << "Cartridge_Note|"
       << "Cartridge_Rarity|"
       << "Cartridge_Sound|"
       << "Cartridge_Type|"
       << "Console_LeftDifficulty|"
       << "Console_RightDifficulty|"
       << "Console_TelevisionType|"
       << "Console_SwapPorts|"
       << "Controller_Left|"
       << "Controller_Right|"
       << "Controller_SwapPaddles|"
       << "Display_Format|"
       << "Display_YStart|"
       << "Display_Height|"
       << "Display_Phosphor|"
       << "Display_PPBlend"
       << endl;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* Properties::ourDefaultProperties[LastPropType] = {
  "",            // Cartridge.MD5
  "",            // Cartridge.Manufacturer
  "",            // Cartridge.ModelNo
  "Untitled",    // Cartridge.Name
  "",            // Cartridge.Note
  "",            // Cartridge.Rarity
  "MONO",        // Cartridge.Sound
  "AUTO-DETECT", // Cartridge.Type
  "B",           // Console.LeftDifficulty
  "B",           // Console.RightDifficulty
  "COLOR",       // Console.TelevisionType
  "NO",          // Console.SwapPorts
  "JOYSTICK",    // Controller.Left
  "JOYSTICK",    // Controller.Right
  "NO",          // Controller.SwapPaddles
  "AUTO-DETECT", // Display.Format
  "34",          // Display.YStart
  "210",         // Display.Height
  "NO",          // Display.Phosphor
  "77"           // Display.PPBlend
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* Properties::ourPropertyNames[LastPropType] = {
  "Cartridge.MD5",
  "Cartridge.Manufacturer",
  "Cartridge.ModelNo",
  "Cartridge.Name",
  "Cartridge.Note",
  "Cartridge.Rarity",
  "Cartridge.Sound",
  "Cartridge.Type",
  "Console.LeftDifficulty",
  "Console.RightDifficulty",
  "Console.TelevisionType",
  "Console.SwapPorts",
  "Controller.Left",
  "Controller.Right",
  "Controller.SwapPaddles",
  "Display.Format",
  "Display.YStart",
  "Display.Height",
  "Display.Phosphor",
  "Display.PPBlend"
};
