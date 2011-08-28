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

#import "OEGenesisPreferenceView.h"

@implementation OEGenesisPreferenceView

- (void)awakeFromNib
{
    [super awakeFromNib];

    OEGameControllerView *view = (OEGameControllerView *)[self view];
    NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:@"gen6btn.png"];
    
    [view addButtonWithName:@"OEGenesisButtonUp[@]"    toolTip:@"D-Pad Up"     target:self startPosition:NSMakePoint(88, 196)  endPosition:NSMakePoint(230, 165)];
    [view addButtonWithName:@"OEGenesisButtonLeft[@]"  toolTip:@"D-Pad Left"   target:self startPosition:NSMakePoint(88, 164)  endPosition:NSMakePoint(207, 148)];
    [view addButtonWithName:@"OEGenesisButtonRight[@]" toolTip:@"D-Pad Right"  target:self startPosition:NSMakePoint(88, 132)  endPosition:NSMakePoint(249, 143)];
    [view addButtonWithName:@"OEGenesisButtonDown[@]"  toolTip:@"D-Pad Down"   target:self startPosition:NSMakePoint(88, 100)  endPosition:NSMakePoint(222, 120)];
    [view addButtonWithName:@"OEGenesisButtonMode[@]"  toolTip:@"Mode Button"  target:self startPosition:NSMakePoint(252, 25)  endPosition:NSMakePoint(293, 114)];
    [view addButtonWithName:@"OEGenesisButtonStart[@]" toolTip:@"Start Button" target:self startPosition:NSMakePoint(348, 25)  endPosition:NSMakePoint(328, 114)];
    [view addButtonWithName:@"OEGenesisButtonA[@]"     toolTip:@"A Button"     target:self startPosition:NSMakePoint(510, 68)  endPosition:NSMakePoint(367, 122)];
    [view addButtonWithName:@"OEGenesisButtonB[@]"     toolTip:@"B Button"     target:self startPosition:NSMakePoint(510, 100) endPosition:NSMakePoint(399, 132)];
    [view addButtonWithName:@"OEGenesisButtonC[@]"     toolTip:@"C Button"     target:self startPosition:NSMakePoint(510, 132) endPosition:NSMakePoint(429, 142)];
    [view addButtonWithName:@"OEGenesisButtonZ[@]"     toolTip:@"Z Button"     target:self startPosition:NSMakePoint(510, 164) endPosition:NSMakePoint(413, 172)];
    [view addButtonWithName:@"OEGenesisButtonY[@]"     toolTip:@"Y Button"     target:self startPosition:NSMakePoint(510, 196) endPosition:NSMakePoint(381, 161)];
    [view addButtonWithName:@"OEGenesisButtonX[@]"     toolTip:@"X Button"     target:self startPosition:NSMakePoint(510, 228) endPosition:NSMakePoint(355, 153)];
}

- (NSImage*)controllerImage{
	return nil;
}
@end
