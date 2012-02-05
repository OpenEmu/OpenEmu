//
//  OESetupAssistantBackgroundView.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 26.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OESetupAssistantBackgroundView.h"
#import "OESetupAssistantQCOpenGLLayer.h"

@implementation OESetupAssistantBackgroundView

- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if(self)
    {
        [self fuckYouLoadTheGoddamnedQCCompositionYouFuckingTwat];
    }
    
    return self;
}

- (void) dealloc
{
    self.layer = nil;
    [super dealloc];
}

- (void) awakeFromNib
{
    [self fuckYouLoadTheGoddamnedQCCompositionYouFuckingTwat];
}

- (void) fuckYouLoadTheGoddamnedQCCompositionYouFuckingTwat
{
    [self setWantsLayer:YES];
    
    OESetupAssistantQCOpenGLLayer* _backgroundAnimation = [OESetupAssistantQCOpenGLLayer layer];
    
    [_backgroundAnimation setFrame:self.frame];
    [_backgroundAnimation setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
    [_backgroundAnimation setAsynchronous:YES];
    [_backgroundAnimation setContinaingWindow:self.window];
    
    self.layer = _backgroundAnimation;
}

@end
