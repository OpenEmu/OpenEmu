//
//  NSArray+OEAdditions.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 8/31/12.
//
//

#import <Foundation/Foundation.h>
@interface NSArray (OEAdditions)
- (id)firstObjectMatchingBlock:(BOOL(^)(id))block;
@end
