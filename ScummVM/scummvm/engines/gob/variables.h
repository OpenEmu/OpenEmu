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

#ifndef GOB_VARIABLES_H
#define GOB_VARIABLES_H

namespace Gob {

class Variables {
public:
	enum Type {
		kVariableType8,
		kVariableType16,
		kVariableType32
	};

	Variables(uint32 size);
	virtual ~Variables();

	uint32 getSize() const;

	void writeVar8(uint32 var, uint8 value);
	void writeVar16(uint32 var, uint16 value);
	void writeVar32(uint32 var, uint32 value);

	void writeVarString(uint32 var, const char *value);

	void writeOff8(uint32 offset, uint8 value);
	void writeOff16(uint32 offset, uint16 value);
	void writeOff32(uint32 offset, uint32 value);

	void writeOffString(uint32 offset, const char *value);

	uint8 readVar8(uint32 var) const;
	uint16 readVar16(uint32 var) const;
	uint32 readVar32(uint32 var) const;

	void readVarString(uint32 var, char *value, uint32 length);

	uint8 readOff8(uint32 offset) const;
	uint16 readOff16(uint32 offset) const;
	uint32 readOff32(uint32 offset) const;

	void readOffString(uint32 offset, char *value, uint32 length);


	const uint8 *getAddressVar8(uint32 var) const;
	uint8 *getAddressVar8(uint32 var);

	const char *getAddressVarString(uint32 var) const;
	char *getAddressVarString(uint32 var);

	const uint8 *getAddressOff8(uint32 offset) const;
	uint8 *getAddressOff8(uint32 offset);

	const char *getAddressOffString(uint32 offset) const;
	char *getAddressOffString(uint32 offset);


	bool copyTo(uint32 offset, byte *variables, uint32 n) const;
	bool copyFrom(uint32 offset, const byte *variables, uint32 n);

protected:
	virtual void write8(byte *buf, uint8 data) const = 0;
	virtual void write16(byte *buf, uint16 data) const = 0;
	virtual void write32(byte *buf, uint32 data) const = 0;

	virtual uint8 read8(const byte *buf) const = 0;
	virtual uint16 read16(const byte *buf) const = 0;
	virtual uint32 read32(const byte *buf) const = 0;

private:
	uint32 _size;
	byte *_vars;

	void clear();
};

class VariablesLE : public Variables {
public:
	VariablesLE(uint32 size);
	~VariablesLE();

protected:
	void write8(byte *buf, uint8 data) const;
	void write16(byte *buf, uint16 data) const;
	void write32(byte *buf, uint32 data) const;

	uint8 read8(const byte *buf) const;
	uint16 read16(const byte *buf) const;
	uint32 read32(const byte *buf) const;
};

class VariablesBE : public Variables {
public:
	VariablesBE(uint32 size);
	~VariablesBE();

protected:
	void write8(byte *buf, uint8 data) const;
	void write16(byte *buf, uint16 data) const;
	void write32(byte *buf, uint32 data) const;

	uint8 read8(const byte *buf) const;
	uint16 read16(const byte *buf) const;
	uint32 read32(const byte *buf) const;
};

class VariableReference {
public:
	VariableReference();
	VariableReference(Variables &vars, uint32 offset,
			Variables::Type type = Variables::kVariableType32);
	~VariableReference();

	void set(Variables &vars, uint32 offset, Variables::Type type = Variables::kVariableType32);

	VariableReference &operator=(uint32 value);
	VariableReference &operator+=(uint32 value);
	VariableReference &operator*=(uint32 value);
	operator uint32();

private:
	Variables *_vars;
	uint32 _offset;
	Variables::Type _type;
};

class VariableStack {
public:
	VariableStack(uint32 size);
	~VariableStack();

	void pushData(const Variables &vars, uint32 offset, uint32 size);
	void pushInt(uint32 value);

	void pop(Variables &vars, uint32 offset);

private:
	byte *_stack;

	uint32 _size;
	uint32 _position;
};

} // End of namespace Gob

#endif // GOB_VARIABLES_H
