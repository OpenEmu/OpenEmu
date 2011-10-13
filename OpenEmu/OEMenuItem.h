//
//  OEMenuItem.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 13.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>

@interface OEMenuItem : NSMenuItem
{
    BOOL hasAlternate;
    id alternateTarget;
    SEL alternateAction;
    
    BOOL isAlternate;
}

@property BOOL hasAlternate;
@property (assign) id alternateTarget;
@property SEL alternateAction;

@property BOOL isAlternate;
@end
