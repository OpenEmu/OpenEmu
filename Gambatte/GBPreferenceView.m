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

#import "GBPreferenceView.h"
#import "GBGameController.h"
#import "GBGameEmu.h"
#import <OEGameControllerView.h>
@implementation GBPreferenceView

- (void)awakeFromNib
{
    OEGameControllerView *view = (OEGameControllerView *)[self view];
    
    NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:@"gameboy-1.png"];
    [view setGameController:[[[NSImage alloc] initWithContentsOfFile:path] autorelease]];

    [view addButtonWithName:@"GB_PAD_UP"    toolTip:@"D-Pad Up"      target:self startPosition:NSMakePoint(115, 246) endPosition:NSMakePoint(222, 215)];
    [view addButtonWithName:@"GB_PAD_LEFT"  toolTip:@"D-Pad Left"    target:self startPosition:NSMakePoint(115, 213) endPosition:NSMakePoint(195, 196)];
    [view addButtonWithName:@"GB_PAD_RIGHT" toolTip:@"D-Pad Right"   target:self startPosition:NSMakePoint(115, 180) endPosition:NSMakePoint(245, 196)];
    [view addButtonWithName:@"GB_PAD_DOWN"  toolTip:@"D-Pad Down"    target:self startPosition:NSMakePoint(115, 147) endPosition:NSMakePoint(222, 173)];
    
    [view addButtonWithName:@"GB_PAD_A"     toolTip:@"A Button"      target:self startPosition:NSMakePoint(552, 213) endPosition:NSMakePoint(411, 208)];
    [view addButtonWithName:@"GB_PAD_B"     toolTip:@"B Button"      target:self startPosition:NSMakePoint(552, 180) endPosition:NSMakePoint(365, 185)];

    [view addButtonWithName:@"GB_SELECT"    toolTip:@"Select Button" target:self startPosition:NSMakePoint(245, 27)  endPosition:NSMakePoint(265, 130)];
    [view addButtonWithName:@"GB_START"     toolTip:@"Start Button"  target:self startPosition:NSMakePoint(360, 27)  endPosition:NSMakePoint(315, 130)];

}

@end
