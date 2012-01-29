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

#import "OEGBAControlsPreference.h"

@implementation OEGBAControlsPreference

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    OEControlsSetupView *view = (OEControlsSetupView *)[self view];
    
	[view addButtonWithName:@"OEGBAButtonUp[@]" label:@"Up:" target:self highlightPoint:(NSPoint){146, 173}];
	[view addButtonWithName:@"OEGBAButtonDown[@]" label:@"Down:" target:self highlightPoint:(NSPoint){146, 138}];
	[view addButtonWithName:@"OEGBAButtonLeft[@]" label:@"Left:" target:self highlightPoint:(NSPoint){128, 155}];
	[view addButtonWithName:@"OEGBAButtonRight[@]" label:@"Right:" target:self highlightPoint:(NSPoint){168, 155}];
    [view nextColumn];
	
	[view addButtonWithName:@"OEGBAButtonStart[@]" label:@"Start:" target:self highlightPoint:(NSPoint){173,97}];
	[view addButtonWithName:@"OEGBAButtonSelect[@]" label:@"Select:" target:self highlightPoint:(NSPoint){173, 73}];	
	[view nextColumn];
	
	[view addButtonWithName:@"OEGBAButtonA[@]" label:@"A:" target:self highlightPoint:(NSPoint){471, 161}];
	[view addButtonWithName:@"OEGBAButtonB[@]" label:@"B:" target:self highlightPoint:(NSPoint){437, 148}];
	
	[view addButtonWithName:@"OEGBAButtonL[@]" label:@"Left Trigger:" target:self highlightPoint:(NSPoint){131, 237}];
	[view addButtonWithName:@"OEGBAButtonR[@]" label:@"Right Trigger:" target:self highlightPoint:(NSPoint){469, 237}];

	[view updateButtons];
}
- (NSImage*)controllerImage{
    NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:@"controller_gba.png"];
	return [[[NSImage alloc] initWithContentsOfFile:path] autorelease];
}
@end
