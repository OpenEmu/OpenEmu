//
//  OESMSPreferenceView.m
//  OpenEmu
//
//  Created by Remy Demarest on 24/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import "OESMSPreferenceView.h"


@implementation OESMSPreferenceView

- (id)init
{
    self = [super init];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void)dealloc
{
    [super dealloc];
}


- (void)awakeFromNib
{
    OEGameControllerView *view = (OEGameControllerView *)[self view];
    
    NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:@"smscont.png"];
    [view setGameController:[[[NSImage alloc] initWithContentsOfFile:path] autorelease]];
    [view setControlZone:NSMakeRect(0, 0, 600, 230)];
    
    [view addButtonWithName:@"OESMSButtonUp[@]"    toolTip:@"D-Pad Up"    target:self startPosition:NSMakePoint(67, 163)  endPosition:NSMakePoint(230, 136)];
    [view addButtonWithName:@"OESMSButtonLeft[@]"  toolTip:@"D-Pad Left"  target:self startPosition:NSMakePoint(67, 131)  endPosition:NSMakePoint(204, 117)];
    [view addButtonWithName:@"OESMSButtonRight[@]" toolTip:@"D-Pad Right" target:self startPosition:NSMakePoint(67, 99)   endPosition:NSMakePoint(257, 117)];
    [view addButtonWithName:@"OESMSButtonDown[@]"  toolTip:@"D-Pad Down"  target:self startPosition:NSMakePoint(67, 67)   endPosition:NSMakePoint(230, 90)];
    
    [view addButtonWithName:@"OESMSButtonA[@]"     toolTip:@"A Button"    target:self startPosition:NSMakePoint(552, 131) endPosition:NSMakePoint(358, 100)];
    [view addButtonWithName:@"OESMSButtonB[@]"     toolTip:@"B Button"    target:self startPosition:NSMakePoint(552, 99)  endPosition:NSMakePoint(407, 100)];
}

@end
