/*
 Copyright (c) 2011, OpenEmu Team
 
 
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

#import "OEGGPreferenceView.h"


@implementation OEGGPreferenceView

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
    [super awakeFromNib];

    OEControlsSetupView *view = (OEControlsSetupView *)[self view];

	[view addButtonWithName:@"OEGGButtonUp" label:@"Up:" target:self highlightPoint:NSMakePoint(195, 176)];
	[view addButtonWithName:@"OEGGButtonDown" label:@"Down:" target:self highlightPoint:NSMakePoint(195, 93)];
	[view addButtonWithName:@"OEGGButtonLeft" label:@"Left:" target:self highlightPoint:NSMakePoint(157, 138)];
	[view addButtonWithName:@"OEGGButtonRight" label:@"Right:" target:self highlightPoint:NSMakePoint(234, 138)];
    [view nextColumn];
    
	[view addButtonWithName:@"OEGGButtonA" label:@"Button 1:" target:self highlightPoint:NSMakePoint(383, 114)];
	[view addButtonWithName:@"OEGGButtonB" label:@"Button 2:" target:self highlightPoint:NSMakePoint(453, 114)];
	[view addRowSeperator];

	[view addButtonWithName:@"OEGGButtonStart" label:@"Start" target:self];

    [view updateButtons];
	
    return;
}

- (NSImage*)controllerImage{
	NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:@"controller_sms.png"];
	return [[[NSImage alloc] initWithContentsOfFile:path] autorelease];
}

- (NSImage*)controllerImageMask
{
	NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:@"controller_sms_mask.png"];
	return [[[NSImage alloc] initWithContentsOfFile:path] autorelease];
}
@end
