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
	
	[view addButtonWithName:@"OEGenesisButtonUp[@]" label:@"Up:" target:self];
	[view addButtonWithName:@"OEGenesisButtonDown[@]" label:@"Down:" target:self];
	[view addButtonWithName:@"OEGenesisButtonLeft[@]" label:@"Left:" target:self];
	[view addButtonWithName:@"OEGenesisButtonRight[@]" label:@"Right:" target:self];
    [view nextColumn];
	
	[view addButtonWithName:@"OEGenesisButtonStart[@]" label:@"Start:" target:self];
	[view addButtonWithName:@"OEGenesisButtonMode[@]" label:@"Mode:" target:self];
	[view nextColumn];
	
	[view addButtonWithName:@"OEGenesisButtonA[@]" label:@"A:" target:self];
	[view addButtonWithName:@"OEGenesisButtonB[@]" label:@"B:" target:self];
	[view addButtonWithName:@"OEGenesisButtonC[@]" label:@"C:" target:self];
	[view nextColumn];
	
	[view addButtonWithName:@"OEGenesisButtonX[@]" label:@"X:" target:self];
	[view addButtonWithName:@"OEGenesisButtonY[@]" label:@"Y:" target:self];
	[view addButtonWithName:@"OEGenesisButtonZ[@]" label:@"Z:" target:self];
    
    [view updateButtons];
}

- (NSImage*)controllerImage{
	return nil;
}
@end
