//
//  OESetupAssistantBackgroundView.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 26.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OESetupAssistantBackgroundView.h"
#import "OESetupAssistantQCOpenGLLayer.h"

@interface OESetupAssistantBackgroundView (Private)
- (void)setupLayer;
@end
@implementation OESetupAssistantBackgroundView
- (void)setupLayer
{
    [self setWantsLayer:YES];
    
    OESetupAssistantQCOpenGLLayer* _backgroundAnimation = [OESetupAssistantQCOpenGLLayer layer];
    
    [_backgroundAnimation setFrame:self.frame];
    [_backgroundAnimation setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
    [_backgroundAnimation setAsynchronous:YES];
    
    self.layer = _backgroundAnimation;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if(self)
    {
        [self setupLayer];
    }
    return self;
}
- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if(self)
    {
        [self setupLayer];
    }
    
    return self;
}

- (void) dealloc
{
    self.layer = nil;
    [super dealloc];
}

- (void)viewDidMoveToWindow
{
    [((OESetupAssistantQCOpenGLLayer*)self.layer) setContinaingWindow:[self window]];
}
@end
