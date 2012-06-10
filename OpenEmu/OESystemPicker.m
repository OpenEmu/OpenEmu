//
//  OESystemPicker.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 10.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OESystemPicker.h"

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
