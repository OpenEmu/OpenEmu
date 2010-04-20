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

#import "SNESPreferenceViewController.h"
#import <OEGameControllerView.h>

@implementation SNESPreferenceViewController

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    OEGameControllerView *view = (OEGameControllerView *)[self view];
    NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:@"snes_pad.png"];
    [view setGameController:[[[NSImage alloc] initWithContentsOfFile:path] autorelease]];
    [view setControlZone:NSMakeRect(0, 0, 600, 230)];
    
    [view addButtonWithName:@"Joypad@ L"      toolTip:@"Left Trigger"  target:self startPosition:NSMakePoint(50, 180)  endPosition:NSMakePoint(217, 178)];
    [view addButtonWithName:@"Joypad@ Up"     toolTip:@"D-Pad Up"      target:self startPosition:NSMakePoint(50, 148)  endPosition:NSMakePoint(220, 121)];
    [view addButtonWithName:@"Joypad@ Left"   toolTip:@"D-Pad Left"    target:self startPosition:NSMakePoint(50, 116)  endPosition:NSMakePoint(202, 107)];
    [view addButtonWithName:@"Joypad@ Right"  toolTip:@"D-Pad Right"   target:self startPosition:NSMakePoint(50, 84)   endPosition:NSMakePoint(234, 106)];
    [view addButtonWithName:@"Joypad@ Down"   toolTip:@"D-Pad Down"    target:self startPosition:NSMakePoint(50, 52)   endPosition:NSMakePoint(218, 91)];
    
    [view addButtonWithName:@"Joypad@ Select" toolTip:@"Select Button" target:self startPosition:NSMakePoint(252, 25)  endPosition:NSMakePoint(276, 98)];
    [view addButtonWithName:@"Joypad@ Start"  toolTip:@"Start Button"  target:self startPosition:NSMakePoint(348, 25)  endPosition:NSMakePoint(308, 98)];
    
    [view addButtonWithName:@"Joypad@ R"      toolTip:@"Right Trigger" target:self startPosition:NSMakePoint(550, 180) endPosition:NSMakePoint(389, 177)];
    [view addButtonWithName:@"Joypad@ X"      toolTip:@"X Button"      target:self startPosition:NSMakePoint(550, 148) endPosition:NSMakePoint(389, 128)];
    [view addButtonWithName:@"Joypad@ A"      toolTip:@"A Button"      target:self startPosition:NSMakePoint(550, 116) endPosition:NSMakePoint(415, 107)];
    [view addButtonWithName:@"Joypad@ Y"      toolTip:@"Y Button"      target:self startPosition:NSMakePoint(550, 84)  endPosition:NSMakePoint(361, 108)];
    [view addButtonWithName:@"Joypad@ B"      toolTip:@"B Button"      target:self startPosition:NSMakePoint(550, 52)  endPosition:NSMakePoint(386, 86)];
}

@end
