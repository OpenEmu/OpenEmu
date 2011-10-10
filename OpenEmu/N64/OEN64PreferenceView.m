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

#import "OEN64PreferenceView.h"


@implementation OEN64PreferenceView

- (void)awakeFromNib
{
    OEGameControllerView *view = (OEGameControllerView *)[self view];
    
    [view addButtonWithName:@"OEN64ButtonDPadUp[@]" label:@"D-Pad Up" target:self highlightPoint:NSZeroPoint];
    [view addButtonWithName:@"OEN64ButtonDPadDown[@]" label:@"D-Pad Down" target:self highlightPoint:NSZeroPoint];
    [view addButtonWithName:@"OEN64ButtonDPadLeft[@]" label:@"D-Pad Left" target:self highlightPoint:NSZeroPoint];
    [view addButtonWithName:@"OEN64ButtonDPadRight[@]" label:@"D-Pad Right" target:self highlightPoint:NSZeroPoint];

    [view addButtonWithName:@"OEN64ButtonCUp[@]" label:@"C Up" target:self highlightPoint:NSZeroPoint];
    [view addButtonWithName:@"OEN64ButtonCDown[@]" label:@"C Down" target:self highlightPoint:NSZeroPoint];
    [view addButtonWithName:@"OEN64ButtonCLeft[@]" label:@"C Left" target:self highlightPoint:NSZeroPoint];
    [view addButtonWithName:@"OEN64ButtonCRight[@]" label:@"C Right" target:self highlightPoint:NSZeroPoint];
    
    [view addButtonWithName:@"OEN64ButtonL[@]" label:@"L" target:self highlightPoint:NSZeroPoint];
    [view addButtonWithName:@"OEN64ButtonR[@]" label:@"R" target:self highlightPoint:NSZeroPoint];

    [view addButtonWithName:@"OEN64ButtonA[@]" label:@"A" target:self highlightPoint:NSZeroPoint];
    [view addButtonWithName:@"OEN64ButtonB[@]" label:@"B" target:self highlightPoint:NSZeroPoint];
    [view addButtonWithName:@"OEN64ButtonStart[@]" label:@"Start" target:self highlightPoint:NSZeroPoint];
    [view addButtonWithName:@"OEN64ButtonZ[@]" label:@"Z" target:self highlightPoint:NSZeroPoint];
	
	return;
}

- (NSImage*)controllerImage{
	NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:@"controller_n64.png"];
	return [[[NSImage alloc] initWithContentsOfFile:path] autorelease];
}
@end
