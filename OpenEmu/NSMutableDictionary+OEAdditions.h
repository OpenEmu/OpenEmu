//
//  NSMutableDictionary+OEAdditions.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 27/05/14.
//
//

#import <Foundation/Foundation.h>

@interface NSMutableDictionary (OEAdditions)
- (id)popObjectForKey:(NSString*)key;
@end
