/*
 Copyright (c) 2009, OpenEmu Team
 
 
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

#import "GBAControlsPreference.h"
#import <OEGameControllerView.h>

@implementation GBAControlsPreference

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    OEGameControllerView *view = (OEGameControllerView *)[self view];
    NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:@"gba.png"];
    [view setGameController:[[[NSImage alloc] initWithContentsOfFile:path] autorelease]];
    
    [view addButtonWithName:@"KEY@_BUTTON_L"      toolTip:@"Left Trigger"  target:self startPosition:NSMakePoint(50, 220)  endPosition:NSMakePoint(203, 208)];
    [view addButtonWithName:@"KEY@_UP"            toolTip:@"D-Pad Up"      target:self startPosition:NSMakePoint(50, 188)  endPosition:NSMakePoint(210, 172)];
    [view addButtonWithName:@"KEY@_LEFT"          toolTip:@"D-Pad Left"    target:self startPosition:NSMakePoint(50, 156)  endPosition:NSMakePoint(199, 161)];
    [view addButtonWithName:@"KEY@_RIGHT"         toolTip:@"D-Pad Right"   target:self startPosition:NSMakePoint(50, 124)  endPosition:NSMakePoint(221, 163)];
    [view addButtonWithName:@"KEY@_DOWN"          toolTip:@"D-Pad Down"    target:self startPosition:NSMakePoint(50, 92)   endPosition:NSMakePoint(210, 151)];
    
    [view addButtonWithName:@"KEY@_BUTTON_START"  toolTip:@"Start Button"  target:self startPosition:NSMakePoint(348, 20)  endPosition:NSMakePoint(227, 127)];
    [view addButtonWithName:@"KEY@_BUTTON_SELECT" toolTip:@"Select Button" target:self startPosition:NSMakePoint(252, 20)  endPosition:NSMakePoint(227, 114)];
    
    [view addButtonWithName:@"KEY@_BUTTON_R"      toolTip:@"Right Trigger" target:self startPosition:NSMakePoint(550, 220) endPosition:NSMakePoint(393, 210)];
    [view addButtonWithName:@"KEY@_BUTTON_A"      toolTip:@"A Button"      target:self startPosition:NSMakePoint(550, 124) endPosition:NSMakePoint(403, 167)];
    [view addButtonWithName:@"KEY@_BUTTON_B"      toolTip:@"B Button"      target:self startPosition:NSMakePoint(550, 92)  endPosition:NSMakePoint(379, 159)];
}

@end
