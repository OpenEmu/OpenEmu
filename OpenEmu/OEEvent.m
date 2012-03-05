//
//  OEEvent.m
//  OpenEmu
//
//  Created by Remy Demarest on 04/03/2012.
//  Copyright (c) 2012 NuLayer Inc. All rights reserved.
//

#import "OEEvent.h"
#import <QuartzCore/QuartzCore.h>

@implementation OEEvent
{
    NSEvent    *realEvent;
    OEIntPoint  location;
}

+ (id)eventWithMouseEvent:(NSEvent *)anEvent withLocationInGameView:(OEIntPoint)aLocation;
{
    return [[self alloc] initWithMouseEvent:anEvent withLocationInGameView:aLocation];
}

- (id)init
{
    return nil;
}

- (id)initWithMouseEvent:(NSEvent *)anEvent withLocationInGameView:(OEIntPoint)aLocation;
{
    if((self = [super init]))
    {
        realEvent = anEvent;
        location  = aLocation;
    }
    
    return self;
}

- (OEIntPoint)locationInGameView;
{
    return location;
}

- (id)forwardingTargetForSelector:(SEL)aSelector
{
    return [realEvent respondsToSelector:aSelector] ? realEvent : nil;
}

@end

@implementation NSEvent (OEEventAdditions)

- (OEIntPoint)locationInGameView;
{
    CGPoint p = [self locationInWindow];
    
    return (OEIntPoint){ .x = p.x, .y = p.y };
}

@end
