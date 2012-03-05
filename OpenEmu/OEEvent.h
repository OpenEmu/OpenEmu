//
//  OEEvent.h
//  OpenEmu
//
//  Created by Remy Demarest on 04/03/2012.
//  Copyright (c) 2012 NuLayer Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OEGameCore.h"

@class OEGameView;

@interface NSEvent (OEEventAdditions)
- (OEIntPoint)locationInGameView;
@end

@interface OEEvent : NSObject

+ (id)eventWithMouseEvent:(NSEvent *)anEvent withLocationInGameView:(OEIntPoint)aLocation;
- (id)initWithMouseEvent:(NSEvent *)anEvent withLocationInGameView:(OEIntPoint)aLocation;
- (OEIntPoint)locationInGameView;

@end
