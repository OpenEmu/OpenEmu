/*
 Copyright (c) 2010 OpenEmu Team
 
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

#import "MupenPreferenceViewController.h"
#import <OEGameControllerView.h>

@implementation MupenPreferenceViewController

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    OEGameControllerView *view = (OEGameControllerView*)[self view];
    NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:@"controller.gif"];
    [view setGameController:[[[NSImage alloc] initWithContentsOfFile:path] autorelease]];
    [view setControlZone:NSMakeRect(0, 0, 476, 461)];
    
    [view addButtonWithName:@"N64_DPadR"   toolTip:@"Left Trigger"  target:self startPosition:NSMakePoint(105, 134)  endPosition:NSMakePoint(127, 158)];
    [view addButtonWithName:@"N64_DPadL"   toolTip:@"Left Trigger"  target:self startPosition:NSMakePoint(52, 134)   endPosition:NSMakePoint(76, 158)];
    [view addButtonWithName:@"N64_DPadD"   toolTip:@"Left Trigger"  target:self startPosition:NSMakePoint(78, 159)   endPosition:NSMakePoint(102, 184)];
    [view addButtonWithName:@"N64_DPadU"   toolTip:@"Left Trigger"  target:self startPosition:NSMakePoint(78, 108)   endPosition:NSMakePoint(102, 104)];
    
    [view addButtonWithName:@"N64_Start"   toolTip:@"Left Trigger"  target:self startPosition:NSMakePoint(213, 141)  endPosition:NSMakePoint(256, 183)];
    [view addButtonWithName:@"N64_Z"       toolTip:@"Left Trigger"  target:self startPosition:NSMakePoint(217, 322)  endPosition:NSMakePoint(247, 362)];
    
    [view addButtonWithName:@"N64_B"       toolTip:@"Left Trigger"  target:self startPosition:NSMakePoint(307, 130)  endPosition:NSMakePoint(347, 171)];
    [view addButtonWithName:@"N64_A"       toolTip:@"Left Trigger"  target:self startPosition:NSMakePoint(339, 167)  endPosition:NSMakePoint(382, 210)];
    
    [view addButtonWithName:@"N64_CR"      toolTip:@"Left Trigger"  target:self startPosition:NSMakePoint(412, 112)  endPosition:NSMakePoint(450, 150)];
    [view addButtonWithName:@"N64_CL"      toolTip:@"Left Trigger"  target:self startPosition:NSMakePoint(345, 105)  endPosition:NSMakePoint(385, 145)];
    [view addButtonWithName:@"N64_CD"      toolTip:@"Left Trigger"  target:self startPosition:NSMakePoint(380, 140)  endPosition:NSMakePoint(420, 180)];
    [view addButtonWithName:@"N64_CU"      toolTip:@"Left Trigger"  target:self startPosition:NSMakePoint(380, 80)   endPosition:NSMakePoint(420, 115)];
    
    [view addButtonWithName:@"N64_R"       toolTip:@"Left Trigger"  target:self startPosition:NSMakePoint(337, 62)   endPosition:NSMakePoint(437, 69)];
    [view addButtonWithName:@"N64_L"       toolTip:@"Left Trigger"  target:self startPosition:NSMakePoint(37, 34)    endPosition:NSMakePoint(142, 61)];
    
    //[view addButtonWithName:@""   toolTip:@"Left Trigger"  target:self startPosition:NSMakePoint(50, 180)  endPosition:NSMakePoint(217, 178)];
}

@end
