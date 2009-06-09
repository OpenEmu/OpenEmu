//
//  OERingBuffer.h
//  OpenEmu
//
//  Created by Joshua Weinberg on 6/8/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface OERingBuffer : NSObject {
	uint8_t * _buffer;
	NSUInteger bufferSize;
	NSUInteger _writePosition;
	NSUInteger _readPosition;
	NSUInteger _bufferUsed;
	
	NSLock *bufferLock;

}

- (id) initWithLength:(NSUInteger) length;

- (NSUInteger) bytesAvailable;
- (NSUInteger) bytesUsed;
- (NSUInteger)read:(uint8_t *)buffer maxLength:(NSUInteger)len;
- (NSUInteger)write:(const uint8_t *)buffer maxLength:(NSUInteger)length;


@end
