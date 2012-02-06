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
#import "OELocalizationHelper.h"
@implementation OESNESPreferenceViewController

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    OEControlsSetupView *view = (OEControlsSetupView *)[self view];

    [view addButtonWithName:@"OESNESButtonUp[@]" label:@"Up:" target:self highlightPoint:NSMakePoint(169, 154)];
    [view addButtonWithName:@"OESNESButtonDown[@]" label:@"Down:" target:self highlightPoint:NSMakePoint(169, 110)];
    [view addButtonWithName:@"OESNESButtonLeft[@]" label:@"Left:" target:self highlightPoint:NSMakePoint(147, 132)];
    [view addButtonWithName:@"OESNESButtonRight[@]" label:@"Right:" target:self highlightPoint:NSMakePoint(192, 132)];
    [view nextColumn];
    
    [view addButtonWithName:@"OESNESButtonSelect[@]" label:@"Select:" target:self highlightPoint:NSMakePoint(252, 117)];
    [view addButtonWithName:@"OESNESButtonStart[@]" label:@"Start:" target:self highlightPoint:NSMakePoint(299, 117)];
    [view nextColumn];
    
    [view addButtonWithName:@"OESNESButtonA[@]" label:@"A:" target:self highlightPoint:NSMakePoint(456, 130)];
    [view addButtonWithName:@"OESNESButtonB[@]" label:@"B:" target:self highlightPoint:NSMakePoint(413, 95)];
    [view addButtonWithName:@"OESNESButtonX[@]" label:@"X:" target:self highlightPoint:NSMakePoint(416, 164)];
    [view addButtonWithName:@"OESNESButtonY[@]" label:@"Y:" target:self highlightPoint:NSMakePoint(373, 130)];
    [view nextColumn];
    
    [view addButtonWithName:@"OESNESButtonTriggerLeft[@]" label:@"Trigger Left:" target:self highlightPoint:NSMakePoint(166, 220)];
    [view addButtonWithName:@"OESNESButtonTriggerRight[@]" label:@"Trigger Right:" target:self highlightPoint:NSMakePoint(423, 220)];
    
    [view updateButtons];
}

- (NSImage*)controllerImage
{
    NSString *controllerImageName = nil;
    if([[OELocalizationHelper sharedHelper] isRegionJAP])
        controllerImageName = @"controller_snes_jap.png";
    else if([[OELocalizationHelper sharedHelper] isRegionNA])
        controllerImageName = @"controller_snes_usa.png";
    else
        controllerImageName = @"controller_snes_eu.png";
    
    NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:controllerImageName];
    return [[[NSImage alloc] initWithContentsOfFile:path] autorelease];
}

- (NSImage*)controllerImageMask
{
    NSString *path = [[NSBundle bundleForClass:[self class]] pathForImageResource:@"controller_snes_mask.png"];
    return [[[NSImage alloc] initWithContentsOfFile:path] autorelease];
}
@end
