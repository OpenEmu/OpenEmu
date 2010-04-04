/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "NESControlsPreference.h"
#import <OEGameControllerView.h>

@implementation NESControlsPreference

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    OEGameControllerView *view = (OEGameControllerView *)[self view];
    NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:@"nespad.png"];
    [view setGameController:[[[NSImage alloc] initWithContentsOfFile:path] autorelease]];
    
    [view addButtonWithName:@"PAD@_UP"     toolTip:@"D-Pad Up"      target:self startPosition:NSMakePoint(50,  166) endPosition:NSMakePoint(219, 145)];
    [view addButtonWithName:@"PAD@_LEFT"   toolTip:@"D-Pad Up"      target:self startPosition:NSMakePoint(50,  134) endPosition:NSMakePoint(197, 132)];
    [view addButtonWithName:@"PAD@_RIGHT"  toolTip:@"D-Pad Up"      target:self startPosition:NSMakePoint(50,  102) endPosition:NSMakePoint(231, 131)];
    [view addButtonWithName:@"PAD@_DOWN"   toolTip:@"D-Pad Up"      target:self startPosition:NSMakePoint(50,   70) endPosition:NSMakePoint(207, 118)];
    
    [view addButtonWithName:@"PAD@_A"      toolTip:@"A Button"      target:self startPosition:NSMakePoint(600, 150) endPosition:NSMakePoint(409, 118)];
    [view addButtonWithName:@"PAD@_B"      toolTip:@"B Button"      target:self startPosition:NSMakePoint(600,  86) endPosition:NSMakePoint(369, 118)];
    
    [view addButtonWithName:@"PAD@_START"  toolTip:@"Start Button"  target:self startPosition:NSMakePoint(368,  10) endPosition:NSMakePoint(316, 117)];
    [view addButtonWithName:@"PAD@_SELECT" toolTip:@"Select Button" target:self startPosition:NSMakePoint(272,  10) endPosition:NSMakePoint(276, 117)];
}

@end
