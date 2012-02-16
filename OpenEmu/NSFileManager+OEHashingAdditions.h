//
//  NSFileManager+OEHashingAdditions.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 12.02.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSFileManager (OEHashingAdditions)
- (NSString*)md5DigestForFileAtPath:(NSString*)path error:(NSError**)error;
- (NSString*)md5DigestForFileAtURL:(NSURL*)url error:(NSError**)error;

- (NSString*)crc32ForFileAtPath:(NSString*)path error:(NSError**)error;
- (NSString*)crc32ForFileAtURL:(NSURL*)url error:(NSError**)error;
@end
