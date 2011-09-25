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

#import "OESMSPreferenceView.h"


@implementation OESMSPreferenceView

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
    OEGameControllerView *view = (OEGameControllerView *)[self view];
    
	[view addButtonWithName:@"OESMSButtonUp[@]" label:@"Up:" target:self highlightPoint:NSMakePoint(113, 144)];
	[view addButtonWithName:@"OESMSButtonDown[@]" label:@"Down:" target:self highlightPoint:NSMakePoint(113, 59)];
	[view addButtonWithName:@"OESMSButtonLeft[@]" label:@"Left:" target:self highlightPoint:NSMakePoint(75, 106)];
	[view addButtonWithName:@"OESMSButtonRight[@]" label:@"Right:" target:self highlightPoint:NSMakePoint(152, 106)];
	
	[view addButtonWithName:@"OESMSButtonA[@]" label:@"Button 1 /Start:" target:self highlightPoint:NSMakePoint(301, 82)];
	[view addButtonWithName:@"OESMSButtonB[@]" label:@"Button 2:" target:self highlightPoint:NSMakePoint(371, 82)];
	
	return;
}

- (NSImage*)controllerImage{
	NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:@"controller_sms.png"];
	return [[[NSImage alloc] initWithContentsOfFile:path] autorelease];
}
@end
