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

#import "OESNESPreferenceViewController.h"

@implementation OESNESPreferenceViewController

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    OEGameControllerView *view = (OEGameControllerView *)[self view];
	
	[view addButtonWithName:@"OESNESButtonUp[@]" label:@"Up:" target:self];
	[view addButtonWithName:@"OESNESButtonDown[@]" label:@"Down:" target:self];
	[view addButtonWithName:@"OESNESButtonLeft[@]" label:@"Left:" target:self];
	[view addButtonWithName:@"OESNESButtonRight[@]" label:@"Right:" target:self];

	[view addButtonWithName:@"OESNESButtonSelect[@]" label:@"Start:" target:self];
	[view addButtonWithName:@"OESNESButtonStart[@]" label:@"Select:" target:self];
	[view nextColumn];

	[view addButtonWithName:@"OESNESButtonA[@]" label:@"A:" target:self];
	[view addButtonWithName:@"OESNESButtonB[@]" label:@"B:" target:self];
	[view addButtonWithName:@"OESNESButtonX[@]" label:@"X:" target:self];
	[view addButtonWithName:@"OESNESButtonY[@]" label:@"Y:" target:self];
	
	
	[view addButtonWithName:@"OESNESButtonTriggerLeft[@]" label:@"Trigger Left:" target:self];
	[view addButtonWithName:@"OESNESButtonTriggerRight[@]" label:@"Trigger Right:" target:self];	
	
	[view updateButtons];
}

- (NSImage*)controllerImage{
	// TODO: localize controller image
	NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:@"controller_snes_usa.png"];
	return [[[NSImage alloc] initWithContentsOfFile:path] autorelease];
}
@end
