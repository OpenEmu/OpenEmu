//
//  OEGridViewLayoutManager.m
//  OEGridViewDemo
//
//  Created by Faustino Osuna on 2/9/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEGridViewLayoutManager.h"
#import <QuartzCore/QuartzCore.h>

static OEGridViewLayoutManager *theLayoutManager = nil;

@implementation OEGridViewLayoutManager

+ (void)initialize
{
    if (self == [OEGridViewLayoutManager class]) {
        theLayoutManager = [[self alloc] init];
    }
}

+ (OEGridViewLayoutManager *)layoutManager
{
    return theLayoutManager;
}

- (void)layoutSublayersOfLayer:(CALayer *)theLayer
{
    if([theLayer conformsToProtocol:@protocol(OEGridViewLayoutManagerProtocol)])
        [theLayer layoutSublayers];
    else if([[theLayer delegate] conformsToProtocol:@protocol(OEGridViewLayoutManagerProtocol)])
        [[theLayer delegate] layoutSublayers];
}

@end
