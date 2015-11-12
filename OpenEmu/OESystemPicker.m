/*
 Copyright (c) 2012, OpenEmu Team
 
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
#import "OESystemPicker.h"
#import "OESystemPlugin.h"

@interface OESystemPicker ()
- (IBAction)confirmSelection:(id)sender;
@end

@implementation OESystemPicker

+ (OESystemPlugin*)pickSystemFromArray:(NSArray*)plugins
{
    OESystemPicker *instance = [[OESystemPicker alloc] initWithWindowNibName:@"OESystemPicker"];
    NSMenu *systemsMenu = [[NSMenu alloc] init];
    [plugins enumerateObjectsUsingBlock:^(OESystemPlugin *obj, NSUInteger idx, BOOL *stop) {
        [systemsMenu addItemWithTitle:[obj systemName] action:NULL keyEquivalent:@""];
        [[[systemsMenu itemArray] lastObject] setImage:[obj systemIcon]];
    }];

    [instance window];
    
    [[instance systemSelector] setMenu:systemsMenu];
    NSInteger result = [NSApp runModalForWindow:[instance window]];
    NSAssert(result!=-1, @"PopUpButton should not allow to make no selection");
    return [plugins objectAtIndex:result];
}

- (IBAction)confirmSelection:(id)sender
{
    NSPopUpButton *button = [[[sender superview] subviews] lastObject];
    NSInteger index = [button indexOfSelectedItem];
    [NSApp stopModalWithCode:index];
}
@synthesize systemSelector;
@end
