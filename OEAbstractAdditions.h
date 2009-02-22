//
//  OEAbstractAdditions.h
//  OpenEmu
//
//  Created by Remy Demarest on 22/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSObject (OEAbstractAdditions)

- (void)doesNotImplementSelector:(SEL)aSel;
- (void)doesNotImplementOptionalSelector:(SEL)aSel;

@end
