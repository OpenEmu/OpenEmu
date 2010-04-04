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

#import "SMSPreferenceView.h"
#import "SMSGameController.h"
#import "SMSGameCore.h"
#import <OEGameControllerView.h>
@implementation SMSPreferenceView

- (void)awakeFromNib
{
    OEGameControllerView *view = (OEGameControllerView *)[self view];
    
    NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:@"smscont.png"];
    [view setGameController:[[[NSImage alloc] initWithContentsOfFile:path] autorelease]];
    [view setControlZone:NSMakeRect(0, 0, 600, 230)];

    [view addButtonWithName:@"SMS_PAD@_UP"    toolTip:@"D-Pad Up"    target:self startPosition:NSMakePoint(67, 163)  endPosition:NSMakePoint(230, 136)];
    [view addButtonWithName:@"SMS_PAD@_LEFT"  toolTip:@"D-Pad Left"  target:self startPosition:NSMakePoint(67, 131)  endPosition:NSMakePoint(204, 117)];
    [view addButtonWithName:@"SMS_PAD@_RIGHT" toolTip:@"D-Pad Right" target:self startPosition:NSMakePoint(67, 99)   endPosition:NSMakePoint(257, 117)];
    [view addButtonWithName:@"SMS_PAD@_DOWN"  toolTip:@"D-Pad Down"  target:self startPosition:NSMakePoint(67, 67)   endPosition:NSMakePoint(230, 90)];
    
    [view addButtonWithName:@"SMS_PAD@_A"     toolTip:@"A Button"    target:self startPosition:NSMakePoint(552, 131) endPosition:NSMakePoint(358, 100)];
    [view addButtonWithName:@"SMS_PAD@_B"     toolTip:@"B Button"    target:self startPosition:NSMakePoint(552, 99)  endPosition:NSMakePoint(407, 100)];
}

@end
