//
//  OERingBuffer.m
//  OpenEmu
//
//  Created by Joshua Weinberg on 6/8/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "OERingBuffer.h"


@implementation OERingBuffer

- (id) initWithLength: (NSUInteger) length
{
	if( self = [super init] )
	{
		_buffer = calloc(length, sizeof(uint8_t));
		bufferLock = [NSLock new];
		_writePosition = 0;
		_readPosition = 0;
		bufferSize = length;
		_bufferUsed = 0;
	}
	return self;
}

- (NSUInteger)write:(const uint8_t *)buffer maxLength:(NSUInteger)length
{
	[bufferLock lock];
	if( length > bufferSize - _bufferUsed )
	{
		length = bufferSize - _bufferUsed;
	}
	
	if( length )
	{
		//Means theres enough room to just write it right in
		if( _writePosition + length < bufferSize )
		{
			memcpy(_buffer + _writePosition, buffer, length);
			_writePosition += length;
		} 
		else
		{
			NSUInteger partialLength = bufferSize - _writePosition;
			memcpy(_buffer + _writePosition, buffer, partialLength);
			_writePosition = 0;
			memcpy(_buffer, buffer + partialLength, length - partialLength);
			_writePosition = length - partialLength;
		}
	}
	_bufferUsed += length;
	[bufferLock unlock];
	
	return length;
}

- (NSUInteger)read:(uint8_t *)buffer maxLength:(NSUInteger)len
{
	[bufferLock lock];	
	memset(buffer, 0, len);
	len = MIN(_bufferUsed, len);	
	if( len )
	{
		if( _readPosition + len <  bufferSize )
		{
			memcpy(buffer, _buffer + _readPosition, len);
			_readPosition += len;
		}
		else
		{
			NSUInteger partialLength = bufferSize - _readPosition;
			memcpy(buffer, _buffer + _readPosition, partialLength);
			_readPosition = 0;
			memcpy(buffer + partialLength, _buffer + _readPosition, len - partialLength);
			_readPosition = len - partialLength;
		}
	}
	_bufferUsed -= len;
	[bufferLock unlock];
	return len;
}

- (NSUInteger) bytesAvailable
{
	return bufferSize - _bufferUsed;
}

- (NSUInteger) bytesUsed
{
	return _bufferUsed;
}

- (NSString*) description
{
	return [NSString stringWithFormat:@"Buffer with length %d.\nUsed: %d\nFree: %d\nRead position: %d\nWrite position: %d", bufferSize, [self bytesUsed], [self bytesAvailable], _readPosition, _writePosition];
}

- (void) dealloc
{
	[bufferLock release];
	free(_buffer);
	[super dealloc];
}

@end
