/*
 Copyright (c) 2009, OpenEmu Team
 
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "OERingBuffer.h"


@implementation OERingBuffer

- (id)init
{
    return [self initWithLength:1];
}

- (id)initWithLength:(NSUInteger)length
{
    if((self = [super init]))
    {
        _bufferLock = [[NSLock alloc] init];
        _bufferSize = length;
        _buffer     = NSZoneCalloc([self zone], _bufferSize, sizeof(char));
        if(_buffer == NULL)
        {
            [self release];
            return nil;
        }
    }
    return self;
}

- (NSUInteger)length
{
    return _bufferSize;
}

- (void)setLength:(NSUInteger)length
{
    [_bufferLock lock];
    if(length > _bufferSize)
    {
        void *temp = NSZoneCalloc([self zone], length, sizeof(char));
        // Checks whether the allocation went well
        if(temp != NULL)
        {
            if(_buffer != NULL) NSZoneFree([self zone], _buffer);
            _buffer = temp;
        }
        // If the allocation didn't work, we keep the old buffer
        else length = _bufferSize;
    }
    _bufferSize = length;
    [_bufferLock unlock];
}

- (NSUInteger)write:(const void *)buffer maxLength:(NSUInteger)length
{
    [_bufferLock lock];
    if(length > _bufferSize - _bufferUsed)
        length = _bufferSize - _bufferUsed;
    
    if(length > 0)
    {
        // Means theres enough room to just write it right in
        if(_writePosition + length < _bufferSize)
        {
            memcpy(_buffer + _writePosition, buffer, length);
            _writePosition += length;
        }
        else
        {
            NSUInteger partialLength = _bufferSize - _writePosition;
            memcpy(_buffer + _writePosition, buffer, partialLength);
            memcpy(_buffer, buffer + partialLength, length - partialLength);
            _writePosition = length - partialLength;
        }
    }
    _bufferUsed += length;
    [_bufferLock unlock];
    
    return length;
}

- (NSUInteger)read:(void *)buffer maxLength:(NSUInteger)len
{
    [_bufferLock lock];    
    memset(buffer, 0, len);
    len = MIN(_bufferUsed, len);
    if(len > 0)
    {
        if(_readPosition + len <  _bufferSize)
        {
            memcpy(buffer, _buffer + _readPosition, len);
            _readPosition += len;
        }
        else
        {
            NSUInteger partialLength = _bufferSize - _readPosition;
            memcpy(buffer, _buffer + _readPosition, partialLength);
            memcpy(buffer + partialLength, _buffer, len - partialLength);
            _readPosition = len - partialLength;
        }
    }
    _bufferUsed -= len;
    [_bufferLock unlock];
    return len;
}

- (NSUInteger)availableBytes
{
    return _bufferSize - _bufferUsed;
}

- (NSUInteger)usedBytes
{
    return _bufferUsed;
}

- (NSUInteger)bytesAvailable
{
    return [self availableBytes];
}

- (NSUInteger)bytesUsed
{
    return [self usedBytes];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"Buffer with length %d.\nUsed: %d\nFree: %d\nRead position: %d\nWrite position: %d", _bufferSize, [self usedBytes], [self availableBytes], _readPosition, _writePosition];
}

- (void)dealloc
{
    if(_buffer != NULL) NSZoneFree([self zone], _buffer);
    [_bufferLock release];
    [super dealloc];
}

@end
