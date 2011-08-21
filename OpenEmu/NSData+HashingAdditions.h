//
//  NSData+HashingAdditions.h
//  Archive
//
//  Created by Carl Leimbrock on 31.07.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSData (HashingAdditions)
- (NSString*)MD5HashString;
- (NSString*)CRC32HashString;
@end