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

#ifndef COMMON_STREAM_H
#define COMMON_STREAM_H

#include "common/endian.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Common {

class SeekableReadStream;

/**
 * Virtual base class for both ReadStream and WriteStream.
 */
class Stream {
public:
	virtual ~Stream() {}

	/**
	 * Returns true if an I/O failure occurred.
	 * This flag is never cleared automatically. In order to clear it,
	 * client code has to call clearErr() explicitly.
	 *
	 * @note The semantics of any implementation of this method are
	 * supposed to match those of ISO C ferror().
	 */
	virtual bool err() const { return false; }

	/**
	 * Reset the I/O error status as returned by err().
	 * For a ReadStream, also reset the end-of-stream status returned by eos().
	 *
	 * @note The semantics of any implementation of this method are
	 * supposed to match those of ISO C clearerr().
	 */
	virtual void clearErr() {}
};

/**
 * Generic interface for a writable data stream.
 */
class WriteStream : virtual public Stream {
public:
	/**
	 * Write data into the stream. Subclasses must implement this
	 * method; all other write methods are implemented using it.
	 *
	 * @note The semantics of any implementation of this method are
	 * supposed to match those of ISO C fwrite().
	 *
	 * @param dataPtr	pointer to the data to be written
	 * @param dataSize	number of bytes to be written
	 * @return the number of bytes which were actually written.
	 */
	virtual uint32 write(const void *dataPtr, uint32 dataSize) = 0;

	/**
	 * Commit any buffered data to the underlying channel or
	 * storage medium; unbuffered streams can use the default
	 * implementation.
	 *
	 * @note The semantics of any implementation of this method are
	 * supposed to match those of ISO C fflush().
	 *
	 * @return true on success, false in case of a failure
	 */
	virtual bool flush() { return true; }

	/**
	 * Finalize and close this stream. To be called right before this
	 * stream instance is deleted. The goal here is to enable calling
	 * code to detect and handle I/O errors which might occur when
	 * closing (and this flushing, if buffered) the stream.
	 *
	 * After this method has been called, no further writes may be
	 * performed on the stream. Calling err() is allowed.
	 *
	 * By default, this just flushes the stream.
	 */
	virtual void finalize() {
		flush();
	}


	// The remaining methods all have default implementations; subclasses
	// need not (and should not) overload them.

	void writeByte(byte value) {
		write(&value, 1);
	}

	void writeSByte(int8 value) {
		write(&value, 1);
	}

	void writeUint16LE(uint16 value) {
		value = TO_LE_16(value);
		write(&value, 2);
	}

	void writeUint32LE(uint32 value) {
		value = TO_LE_32(value);
		write(&value, 4);
	}

	void writeUint16BE(uint16 value) {
		value = TO_BE_16(value);
		write(&value, 2);
	}

	void writeUint32BE(uint32 value) {
		value = TO_BE_32(value);
		write(&value, 4);
	}

	FORCEINLINE void writeSint16LE(int16 value) {
		writeUint16LE((uint16)value);
	}

	FORCEINLINE void writeSint32LE(int32 value) {
		writeUint32LE((uint32)value);
	}

	FORCEINLINE void writeSint16BE(int16 value) {
		writeUint16BE((uint16)value);
	}

	FORCEINLINE void writeSint32BE(int32 value) {
		writeUint32BE((uint32)value);
	}

	/**
	 * Write the given string to the stream.
	 * This writes str.size() characters, but no terminating zero byte.
	 */
	void writeString(const String &str);
};

/**
 * Generic interface for a readable data stream.
 */
class ReadStream : virtual public Stream {
public:
	/**
	 * Returns true if a read failed because the stream end has been reached.
	 * This flag is cleared by clearErr().
	 * For a SeekableReadStream, it is also cleared by a successful seek.
	 *
	 * @note The semantics of any implementation of this method are
	 * supposed to match those of ISO C feof(). In particular, in a stream
	 * with N bytes, reading exactly N bytes from the start should *not*
	 * set eos; only reading *beyond* the available data should set it.
	 */
	virtual bool eos() const = 0;

	/**
	 * Read data from the stream. Subclasses must implement this
	 * method; all other read methods are implemented using it.
	 *
	 * @note The semantics of any implementation of this method are
	 * supposed to match those of ISO C fread(), in particular where
	 * it concerns setting error and end of file/stream flags.
	 *
	 * @param dataPtr	pointer to a buffer into which the data is read
	 * @param dataSize	number of bytes to be read
	 * @return the number of bytes which were actually read.
	 */
	virtual uint32 read(void *dataPtr, uint32 dataSize) = 0;


	// The remaining methods all have default implementations; subclasses
	// in general should not overload them.

	/**
	 * Read an unsigned byte from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling err() and eos() ).
	 */
	byte readByte() {
		byte b = 0; // FIXME: remove initialisation
		read(&b, 1);
		return b;
	}

	/**
	 * Read a signed byte from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling err() and eos() ).
	 */
	FORCEINLINE int8 readSByte() {
		return (int8)readByte();
	}

	/**
	 * Read an unsigned 16-bit word stored in little endian (LSB first) order
	 * from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling err() and eos() ).
	 */
	uint16 readUint16LE() {
		uint16 val;
		read(&val, 2);
		return FROM_LE_16(val);
	}

	/**
	 * Read an unsigned 32-bit word stored in little endian (LSB first) order
	 * from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling err() and eos() ).
	 */
	uint32 readUint32LE() {
		uint32 val;
		read(&val, 4);
		return FROM_LE_32(val);
	}

	/**
	 * Read an unsigned 16-bit word stored in big endian (MSB first) order
	 * from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling err() and eos() ).
	 */
	uint16 readUint16BE() {
		uint16 val;
		read(&val, 2);
		return FROM_BE_16(val);
	}

	/**
	 * Read an unsigned 32-bit word stored in big endian (MSB first) order
	 * from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling err() and eos() ).
	 */
	uint32 readUint32BE() {
		uint32 val;
		read(&val, 4);
		return FROM_BE_32(val);
	}

	/**
	 * Read a signed 16-bit word stored in little endian (LSB first) order
	 * from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling err() and eos() ).
	 */
	FORCEINLINE int16 readSint16LE() {
		return (int16)readUint16LE();
	}

	/**
	 * Read a signed 32-bit word stored in little endian (LSB first) order
	 * from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling err() and eos() ).
	 */
	FORCEINLINE int32 readSint32LE() {
		return (int32)readUint32LE();
	}

	/**
	 * Read a signed 16-bit word stored in big endian (MSB first) order
	 * from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling err() and eos() ).
	 */
	FORCEINLINE int16 readSint16BE() {
		return (int16)readUint16BE();
	}

	/**
	 * Read a signed 32-bit word stored in big endian (MSB first) order
	 * from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling err() and eos() ).
	 */
	FORCEINLINE int32 readSint32BE() {
		return (int32)readUint32BE();
	}

	/**
	 * Read the specified amount of data into a malloc'ed buffer
	 * which then is wrapped into a MemoryReadStream.
	 * The returned stream might contain less data than requested,
	 * if reading more failed, because of an I/O error or because
	 * the end of the stream was reached. Which can be determined by
	 * calling err() and eos().
	 */
	SeekableReadStream *readStream(uint32 dataSize);

};


/**
 * Interface for a seekable & readable data stream.
 *
 * @todo Get rid of SEEK_SET, SEEK_CUR, or SEEK_END, use our own constants
 */
class SeekableReadStream : virtual public ReadStream {
public:

	/**
	 * Obtains the current value of the stream position indicator of the
	 * stream.
	 *
	 * @return the current position indicator, or -1 if an error occurred.
	 */
	virtual int32 pos() const = 0;

	/**
	 * Obtains the total size of the stream, measured in bytes.
	 * If this value is unknown or can not be computed, -1 is returned.
	 *
	 * @return the size of the stream, or -1 if an error occurred
	 */
	virtual int32 size() const = 0;

	/**
	 * Sets the stream position indicator for the stream. The new position,
	 * measured in bytes, is obtained by adding offset bytes to the position
	 * specified by whence. If whence is set to SEEK_SET, SEEK_CUR, or
	 * SEEK_END, the offset is relative to the start of the file, the current
	 * position indicator, or end-of-file, respectively. A successful call
	 * to the seek() method clears the end-of-file indicator for the stream.
	 *
	 * @note The semantics of any implementation of this method are
	 * supposed to match those of ISO C fseek().
	 *
	 * @param offset	the relative offset in bytes
	 * @param whence	the seek reference: SEEK_SET, SEEK_CUR, or SEEK_END
	 * @return true on success, false in case of a failure
	 */
	virtual bool seek(int32 offset, int whence = SEEK_SET) = 0;

	/**
	 * TODO: Get rid of this??? Or keep it and document it
	 * @return true on success, false in case of a failure
	 */
	virtual bool skip(uint32 offset) { return seek(offset, SEEK_CUR); }

	/**
	 * Reads at most one less than the number of characters specified
	 * by bufSize from the and stores them in the string buf. Reading
	 * stops when the end of a line is reached (CR, CR/LF or LF), and
	 * at end-of-file or error. The newline, if any, is retained (CR
	 * and CR/LF are translated to LF = 0xA = '\n'). If any characters
	 * are read and there is no error, a `\0' character is appended
	 * to end the string.
	 *
	 * Upon successful completion, return a pointer to the string. If
	 * end-of-file occurs before any characters are read, returns NULL
	 * and the buffer contents remain unchanged.  If an error occurs,
	 * returns NULL and the buffer contents are indeterminate.
	 * This method does not distinguish between end-of-file and error;
	 * callers must use err() or eos() to determine which occurred.
	 *
	 * @note This methods is closely modeled after the standard fgets()
	 *       function from stdio.h.
	 *
	 * @param buf	the buffer to store into
	 * @param bufSize	the size of the buffer
	 * @return a pointer to the read string, or NULL if an error occurred
	 */
	virtual char *readLine(char *s, size_t bufSize);


	/**
	 * Reads a full line and returns it as a Common::String. Reading
	 * stops when the end of a line is reached (CR, CR/LF or LF), and
	 * at end-of-file or error.
	 *
	 * Upon successful completion, return a string with the content
	 * of the line, *without* the end of a line marker. This method
	 * does not indicate whether an error occurred. Callers must use
	 * err() or eos() to determine whether an exception occurred.
	 */
	virtual String readLine();
};

/**
 * This is a ReadStream mixin subclass which adds non-endian read
 * methods whose endianness is set during the stream creation.
 */
class ReadStreamEndian : virtual public ReadStream {
private:
	const bool _bigEndian;

public:
	ReadStreamEndian(bool bigEndian) : _bigEndian(bigEndian) {}

	bool isBE() const { return _bigEndian; }

	uint16 readUint16() {
		uint16 val;
		read(&val, 2);
		return (_bigEndian) ? TO_BE_16(val) : TO_LE_16(val);
	}

	uint32 readUint32() {
		uint32 val;
		read(&val, 4);
		return (_bigEndian) ? TO_BE_32(val) : TO_LE_32(val);
	}

	FORCEINLINE int16 readSint16() {
		return (int16)readUint16();
	}

	FORCEINLINE int32 readSint32() {
		return (int32)readUint32();
	}
};

/**
 * This is a SeekableReadStream subclass which adds non-endian read
 * methods whose endianness is set during the stream creation.
 */
class SeekableReadStreamEndian : public SeekableReadStream, public ReadStreamEndian {
public:
	SeekableReadStreamEndian(bool bigEndian) : ReadStreamEndian(bigEndian) {}
};


}	// End of namespace Common

#endif
