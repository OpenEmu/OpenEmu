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
// $Id: Props.hxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#ifndef PROPERTIES_HXX
#define PROPERTIES_HXX

#include "bspf.hxx"

enum PropertyType {
  Cartridge_MD5,
  Cartridge_Manufacturer,
  Cartridge_ModelNo,
  Cartridge_Name,
  Cartridge_Note,
  Cartridge_Rarity,
  Cartridge_Sound,
  Cartridge_Type,
  Console_LeftDifficulty,
  Console_RightDifficulty,
  Console_TelevisionType,
  Console_SwapPorts,
  Controller_Left,
  Controller_Right,
  Controller_SwapPaddles,
  Display_Format,
  Display_YStart,
  Display_Height,
  Display_Phosphor,
  Display_PPBlend,
  LastPropType
};

/**
  This class represents objects which maintain a collection of 
  properties.  A property is a key and its corresponding value.

  A properties object can contain a reference to another properties
  object as its "defaults"; this second properties object is searched 
  if the property key is not found in the original property list.

  @author  Bradford W. Mott
  @version $Id: Props.hxx 2199 2011-01-01 16:04:32Z stephena $
*/
class Properties
{
  friend class PropertiesSet;

  public:
    /**
      Creates an empty properties object with the specified defaults.  The 
      new properties object does not claim ownership of the defaults.
    */
    Properties();

    /**
      Creates a properties list by copying another one

      @param properties The properties to copy
    */
    Properties(const Properties& properties);

    /**
      Destructor
    */
    virtual ~Properties();

  public:
    /**
      Get the value assigned to the specified key.  If the key does
      not exist then the empty string is returned.

      @param key  The key of the property to lookup
      @return     The value of the property 
    */
    const string& get(PropertyType key) const;

    /**
      Set the value associated with key to the given value.

      @param key      The key of the property to set
      @param value    The value to assign to the property
    */
    void set(PropertyType key, const string& value);

    /**
      Load properties from the specified input stream

      @param in The input stream to use
    */
    void load(istream& in);
 
    /**
      Save properties to the specified output stream

      @param out The output stream to use
    */
    void save(ostream& out) const;

    /**
      Print the attributes of this properties object
    */
    void print() const;

    /**
      Resets all properties to their defaults
    */
    void setDefaults();

  public:
    /**
      Overloaded assignment operator

      @param properties The properties object to set myself equal to
      @return Myself after assignment has taken place
    */
    Properties& operator = (const Properties& properties);

  private:
    /**
      Helper function to perform a deep copy of the specified
      properties.  Assumes that old properties have already been 
      freed.

      @param properties The properties object to copy myself from
    */
    void copy(const Properties& properties);

    /**
      Read the next quoted string from the specified input stream
      and returns it.

      @param in The input stream to use
      @return The string inside the quotes
    */ 
    static string readQuotedString(istream& in);
     
    /**
      Write the specified string to the given output stream as a 
      quoted string.

      @param out The output stream to use
      @param s The string to output
    */ 
    static void writeQuotedString(ostream& out, const string& s);

    /**
      Get the property type associated with the named property

      @param name  The PropertyType key associated with the given string
    */ 
    static PropertyType getPropertyType(const string& name);

    /**
      When printing each collection of ROM properties, it is useful to
      see which columns correspond to the output fields; this method
      provides that output.
    */ 
    static void printHeader();

  private:
    // The array of properties
    string myProperties[LastPropType];

    // List of default properties to use when none have been provided
    static const char* ourDefaultProperties[LastPropType];

    // The text strings associated with each property type
    static const char* ourPropertyNames[LastPropType];
};

#endif
