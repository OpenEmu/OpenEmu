/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

namespace Agi {

/**
 * WagProperty represents a single property from WinAGI's *.wag file.
 * A property consists of a header and of data.
 * The header consists of the following:
 * - Property code (Integer/Enumeration, 1 byte)
 * - Property type (Integer/Enumeration, 1 byte)
 * - Property number (Integer, 1 byte)
 * - Property size (Little endian integer, 2 bytes)
 * And then there's the data with as many bytes as defined in the header's property size variable.
 */
class WagProperty {
// Constants, enumerations etc
public:
	/**
	 * Property codes taken from WinAGI 1.1.21's source code file WinAGI/AGIObjects.bas.
	 */
	enum WagPropertyCode {
		PC_GAMEDESC = 129, ///< Game description (WinAGI 1.1.21 limits these to 4096 bytes)
		PC_GAMEAUTHOR,     ///< Game author (WinAGI 1.1.21 limits these to 256 bytes)
		PC_GAMEID,         ///< Game ID
		PC_INTVERSION,     ///< Interpreter version (WinAGI 1.1.21 defaults to version 2.917)
		PC_GAMELAST,       ///< Last edit date
		PC_GAMEVERSION,    ///< Game version (WinAGI 1.1.21 limits these to 256 bytes)
		PC_GAMEABOUT,      ///< About game (WinAGI 1.1.21 limits these to 4096 bytes)
		PC_GAMEEXEC,       ///< Game executable
		PC_RESDIR,         ///< Resource directory name
		PC_DEFSYNTAX,      ///< Default syntax
		PC_INVOBJDESC = 144,
		PC_VOCABWORDDESC = 160,
		PC_PALETTE = 172,
		PC_USERESNAMES = 180,
		PC_LOGIC = 192,
		PC_PICTURE = 208,
		PC_SOUND = 224,
		PC_VIEW = 240,
		PC_UNDEFINED = 0x100 ///< An undefined property code (Added for ScummVM).
	};

	/**
	 * Property types taken from WinAGI 1.1.21's source code file WinAGI/AGIObjects.bas.
	 * At the moment these aren't really at all needed by ScummVM. Just here if anyone decides to use them.
	 */
	enum WagPropertyType {
		PT_ID,
		PT_DESC,
		PT_SYNTAX,
		PT_CRC32,
		PT_KEY,
		PT_INST0,
		PT_INST1,
		PT_INST2,
		PT_MUTE0,
		PT_MUTE1,
		PT_MUTE2,
		PT_MUTE3,
		PT_TPQN,
		PT_ROOM,
		PT_VIS0,
		PT_VIS1,
		PT_VIS2,
		PT_VIS3,
		PT_ALL = 0xff,
		PT_UNDEFINED = 0x100 ///< An undefined property type (Added for ScummVM).
	};

// Constructors, destructors, operators etc
public:
	/**
	 * Creates an empty WagProperty object.
	 * No property header or property data in it.
	 */
	WagProperty();

	/**
	 * Destructor. Releases allocated memory if any etc. The usual.
	 */
	~WagProperty();

	/**
	 * Copy constructor. Deep copies the variables.
	 */
	WagProperty(const WagProperty &other);

	/**
	 * Assignment operator. Deep copies the variables.
	 */
	WagProperty &operator=(const WagProperty &other);

// Non-public helper methods
protected:
	/**
	 * Sets the default values for member variables.
	 */
	void setDefaults();

	/**
	 * Delete's the property's data from memory if we have it, otherwise does nothing.
	 */
	void deleteData();

	/**
	 * Deep copies the parameter object to this object.
	 * @param other The object to be deep copied to this object.
	 */
	void deepCopy(const WagProperty &other);

// Public methods that have side-effects
public:
	/**
	 * Read in a property (Header and data).
	 * @return True if reading was a success, false otherwise.
	 */
	bool read(Common::SeekableReadStream &stream);

	/**
	 * Clears the property.
	 * After this the property is empty. No header or data.
	 */
	void clear();

// Public access functions
public:
	/**
	 * Was the property read ok from the source stream?
	 */
	bool readOk() const { return _readOk; }

	/**
	 * Return the property's code.
	 * @return The property's code if readOk(), PC_UNDEFINED otherwise.
	 */
	enum WagPropertyCode getCode() const { return _propCode; }

	/**
	 * Return the property's type.
	 * @return The property's type if readOk(), PT_UNDEFINED otherwise.
	 */
	enum WagPropertyType getType() const { return _propType; }

	/**
	 * Return the property's number.
	 * @return The property's number if readOk(), 0 otherwise.
	 */
	byte getNumber() const { return _propNum; }

	/**
	 * Return the property's data's length.
	 * @return The property's data's length if readOk(), 0 otherwise.
	 */
	uint16 getSize() const { return _propSize; }

	/**
	 * Return property's data. Constant access version.
	 * Can be used as a C-style string (i.e. this is guaranteed to have a trailing zero).
	 * @return The property's data if readOk(), NULL otherwise.
	 */
	const char *getData() const { return _propData; }

// Member variables
protected:
	bool _readOk;                   ///< Was the property read ok from the source stream?
	enum WagPropertyCode _propCode; ///< Property code (Part of the property's header)
	enum WagPropertyType _propType; ///< Property type (Part of the property's header)
	byte _propNum;                  ///< Property number (Part of the property's header)
	uint16 _propSize;               ///< Property's size (Part of the property's header)
	char *_propData;                ///< The property's data (Plus a trailing zero for C-style string access)
};


/**
 * Class for parsing *.wag files created by WinAGI.
 * Using this class you can get information about fanmade AGI games if they have provided a *.wag file with them.
 */
class WagFileParser {
// Constants, type definitions, enumerations etc.
public:
	enum {
		WINAGI_VERSION_LENGTH = 16 ///< WinAGI's version string's length (Always 16)
	};
	typedef Common::Array<WagProperty> PropertyList; ///< A type definition for an array of *.wag file properties

public:
	/**
	 * Constructor. Creates a WagFileParser object in a default state.
	 */
	WagFileParser();

	/**
	 * Destructor.
	 */
	~WagFileParser();

	/**
	 * Loads a *.wag file and parses it.
	 * @note After this you can access the loaded properties using getProperty() and getProperties() etc.
	 * @param filename Name of the file to be parsed.
	 * @return True if parsed successfully, false otherwise.
	 */
	bool parse(const Common::FSNode &node);

	/**
	 * Get list of the loaded properties.
	 * @note Use only after a call to parse() first.
	 * @return The list of loaded properties.
	 */
	const PropertyList &getProperties() const { return _propList; }

	/**
	 * Get property with the given property code.
	 * @note Use only after a call to parse() first.
	 * @return Pointer to the property if its found in memory, NULL otherwise.
	 *
	 * TODO/FIXME: Handle cases where several properties with the given property code are found.
	 *             At the moment we don't need this functionality because the properties we use
	 *             for fallback detection probably don't have multiples in the WAG-file.
	 * TODO: Make this faster than linear time if desired/needed.
	 */
	const WagProperty *getProperty(const WagProperty::WagPropertyCode code) const;

	/**
	 * Tests if the given property contains a valid AGI interpreter version string.
	 * A valid AGI interpreter version string is of the form "X.Y" or "X,Y" where
	 * X is a single decimal digit and Y is a string of decimal digits (At least one digit).
	 * @param version The property to be tested.
	 * @return True if the given property contains a valid AGI interpreter version string, false otherwise.
	 */
	bool checkAgiVersionProperty(const WagProperty &version) const;

	/**
	 * Convert property's data to an AGI interpreter version number.
	 * @param version The property to be converted (Property code should be PC_INTVERSION).
	 * @return AGI interpreter version number if successful, 0 otherwise.
	 */
	uint16 convertToAgiVersionNumber(const WagProperty &version);

	/**
	 * Was the file parsed successfully?
	 * @return True if file was parsed successfully, false otherwise.
	 */
	bool parsedOk() const { return _parsedOk; }

protected:
	/**
	 * Checks if stream has a valid WinAGI version string in its end.
	 * @param stream The stream to be checked.
	 * @return True if reading was successful and stream contains a valid WinAGI version string, false otherwise.
	 */
	bool checkWagVersion(Common::SeekableReadStream &stream);

	/**
	 * Checks if we're at or past the end of the properties stored in the stream.
	 * @param stream The stream whose seeking position is to be checked.
	 * @return True if stream's seeking position is at or past the end of the properties, false otherwise.
	 */
	bool endOfProperties(const Common::SeekableReadStream &stream) const;

// Member variables
protected:
	PropertyList _propList; ///< List of loaded properties from the file.
	bool         _parsedOk; ///< Did the parsing of the file go ok?
};

} // End of namespace Agi
