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

#import "OEGBPreferenceView.h"

@implementation OEGBPreferenceView

- (void)awakeFromNib
{    
    [super awakeFromNib];

    OEControlsSetupView *view = (OEControlsSetupView *)[self view];
    
	[view addButtonWithName:@"OEGBButtonUp" label:@"Up:" target:self highlightPoint:NSMakePoint(58+170, 165+7)];
	[view addButtonWithName:@"OEGBButtonDown" label:@"Down:" target:self highlightPoint:NSMakePoint(58+170, 127+7)];
	[view addButtonWithName:@"OEGBButtonLeft" label:@"Left:" target:self highlightPoint:NSMakePoint(36+170, 146+7)];
	[view addButtonWithName:@"OEGBButtonRight" label:@"Right:" target:self highlightPoint:NSMakePoint(77+170, 147+7)];
    [view nextColumn];

	[view addButtonWithName:@"OEGBButtonStart" label:@"Start:" target:self highlightPoint:NSMakePoint(138+170, 87+7)];
	[view addButtonWithName:@"OEGBButtonSelect" label:@"Select:" target:self highlightPoint:NSMakePoint(97+170, 87+7)];
	[view nextColumn];
	
	[view addButtonWithName:@"OEGBButtonA" label:@"A:" target:self highlightPoint:NSMakePoint(220+170, 159+7)];
	[view addButtonWithName:@"OEGBButtonB" label:@"B:" target:self highlightPoint:NSMakePoint(180+170, 142+7)];
	
	[view updateButtons];
}

- (NSImage*)controllerImage
{
	 NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:@"controller_gb.png"];
	return [[[NSImage alloc] initWithContentsOfFile:path] autorelease];
}

- (NSImage*)controllerImageMask
{
    NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:@"controller_gb_mask.png"];
	return [[[NSImage alloc] initWithContentsOfFile:path] autorelease];
}
@end
