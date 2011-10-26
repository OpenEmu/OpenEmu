//
//  NSActionCell+OEAdditions.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 24.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>

@interface NSControl (NSControl_OEAdditions)

- (void)setTarget:(id)anObject andAction:(SEL)aSelector;
@end
