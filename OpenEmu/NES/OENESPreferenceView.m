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

#import "OENESPreferenceView.h"
#import "OELocalizationHelper.h"
@implementation OENESPreferenceView

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    BOOL famicom = [[OELocalizationHelper sharedHelper] isRegionJAP];
	
    OEControlsSetupView *view = (OEControlsSetupView *)[self view];
    
	[view addButtonWithName:@"OENESButtonUp[@]" label:@"Up:" target:self highlightPoint:NSMakePoint(159, famicom?137:147)];
	[view addButtonWithName:@"OENESButtonDown[@]" label:@"Down:" target:self highlightPoint:NSMakePoint(159, famicom?90:100)];
	[view addButtonWithName:@"OENESButtonLeft[@]" label:@"Left:" target:self highlightPoint:NSMakePoint(136, famicom?113:123)];
	[view addButtonWithName:@"OENESButtonRight[@]" label:@"Right:" target:self highlightPoint:NSMakePoint(183, famicom?113:123)];
    [view nextColumn];
	
	[view addButtonWithName:@"OENESButtonStart[@]" label:@"Start:" target:self highlightPoint:NSMakePoint(famicom?260:306, famicom?92:71+30)];
	[view addButtonWithName:@"OENESButtonSelect[@]" label:@"Select:" target:self highlightPoint:NSMakePoint(famicom?309:256, famicom?92:71+30)];	
	[view nextColumn];	
	
	[view addButtonWithName:@"OENESButtonA[@]" label:@"A:" target:self highlightPoint:NSMakePoint(famicom?449:441, famicom?95:100)];
	[view addButtonWithName:@"OENESButtonB[@]" label:@"B:" target:self highlightPoint:NSMakePoint(famicom?391:384, famicom?95:100)];
	
	[view updateButtons];
}

- (NSImage*)controllerImage
{
	NSString *controllerImageName = [[OELocalizationHelper sharedHelper] isRegionJAP]?@"controller_fc.png":@"controller_nes.png";
	
	NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:controllerImageName];
	return [[NSImage alloc] initWithContentsOfFile:path];
}

- (NSImage*)controllerImageMask
{
    NSString *controllerImageName = [[OELocalizationHelper sharedHelper] isRegionJAP]?@"controller_fc_mask.png":@"controller_nes_mask.png";
	
	NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:controllerImageName];
	return [[NSImage alloc] initWithContentsOfFile:path];
}
@end
