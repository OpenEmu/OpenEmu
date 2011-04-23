//
//  NSString+OEAdditions.h
//  OpenEmu
//
//  Created by Remy Demarest on 17/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface NSString (OEAdditions)
- (BOOL)isEqualToString:(NSString *)aString excludingRange:(NSRange)aRange;
@end
