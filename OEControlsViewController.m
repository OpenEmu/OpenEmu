//
//  OEControlsViewController.m
//  OpenEmu
//
//  Created by Remy Demarest on 28/05/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEControlsViewController.h"
#import "OEHIDEvent.h"
#import "OEGameCoreController.h"

@implementation OEControlsViewController
@synthesize selectedControl;

- (void)awakeFromNib
{
    [playerField setIntegerValue:1];
    [playerStepper setIntegerValue:1];
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (IBAction)selectInputControl:(id)sender
{
    if([sender respondsToSelector:@selector(state)])
    {
        NSInteger state = [sender state];
        
        [selectedControl setState:NSOffState];
        [[sender window] makeFirstResponder:(state == NSOnState ? self : nil)];
        selectedControl = (state == NSOnState ? sender : nil);
    }
}

- (IBAction)showedBindingsChanged:(id)sender
{
    if(sender == playerField)
        [playerStepper setIntegerValue:[playerField integerValue]];
    else if(sender == playerStepper)
        [playerField setIntegerValue:[playerStepper integerValue]];
    
    [self resetKeyBindings];
}

- (void)resetKeyBindings
{
    [self resetBindingsWithKeys:[[self controller] genericControlNames]];
}

- (BOOL)isKeyboardEventSelected
{
    return [bindingType selectedTag] == 0;
}

- (NSString *)selectedKey
{
    return [[selectedControl infoForBinding:@"title"] objectForKey:NSObservedKeyPathKey];
}

- (NSString *)keyPathForKey:(NSString *)aKey
{
    if(playerStepper != nil)
        return [NSString stringWithFormat:@"%d.%@", [playerStepper intValue] - 1, aKey];
    else
        return aKey;
}

- (void)registerEvent:(id)anEvent
{
    if(selectedControl != nil) [self setValue:anEvent forKey:[self selectedKey]];
}

- (void)axisMoved:(OEHIDEvent *)anEvent
{
    if([anEvent isPushed]) [self registerEvent:anEvent];
}

- (void)buttonDown:(OEHIDEvent *)anEvent
{
    [self registerEvent:anEvent];
}

- (void)hatSwitchDown:(OEHIDEvent *)anEvent
{
    [self registerEvent:anEvent];
}

- (void)keyDown:(NSEvent *)anEvent
{
    [self registerEvent:anEvent];
}

- (id)valueForKey:(NSString *)key
{
    OEGameCoreController *controller = [self controller];
    if([[controller genericControlNames] containsObject:key])
    {
        id anEvent = nil;
        if([self isKeyboardEventSelected])
            anEvent = [controller keyboardEventForKey:[self keyPathForKey:key]];
        else
            anEvent = [controller HIDEventForKey:[self keyPathForKey:key]];
        
        return (anEvent != nil ? [anEvent displayDescription] : @"<empty>");
    }
    
    return [super valueForKey:key];
}

- (void)setValue:(id)value forKey:(NSString *)key
{
    OEGameCoreController *controller = [self controller];
    if([[controller genericControlNames] containsObject:key]) // should be mutually exclusive
    {
        [self willChangeValueForKey:key];
        [controller registerEvent:value forKey:[self keyPathForKey:key]];
        [self didChangeValueForKey:key];
    }
    else [super setValue:value forKey:key];
}

@end
