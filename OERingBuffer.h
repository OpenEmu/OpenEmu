//
//  OERingBuffer.h
//  OpenEmu
//
//  Created by Joshua Weinberg on 6/8/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface OERingBuffer : NSObject
{
@private
	NSLock     *_bufferLock;
	void       *_buffer;
	NSUInteger  _bufferSize;
	NSUInteger  _writePosition;
	NSUInteger  _readPosition;
	NSUInteger  _bufferUsed;
}

- (id)initWithLength:(NSUInteger)length;

@property NSUInteger length;
@property(readonly) NSUInteger availableBytes;
@property(readonly) NSUInteger usedBytes;

- (NSUInteger) bytesAvailable;
- (NSUInteger) bytesUsed;
- (NSUInteger)read:(void *)buffer maxLength:(NSUInteger)len;
- (NSUInteger)write:(const void *)buffer maxLength:(NSUInteger)length;


@end
