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

- (NSUInteger)selectedPlayer
{
    if(playerStepper  != nil) return [playerStepper intValue];
    if(playerSelector != nil) return [playerSelector selectedTag];
    return NSNotFound;
}

- (NSString *)keyPathForKey:(NSString *)aKey
{
    OEGameCoreController *controller = [self controller];
    NSUInteger player = [self selectedPlayer];
    if(player != NSNotFound)
        return [aKey stringByReplacingOccurrencesOfString:[controller playerString]
                                               withString:[NSString stringWithFormat:[controller replacePlayerFormat], player]];
    else
        return aKey;
}

- (IBAction)closeWindow:(id)sender
{
    [[[self view] window] close];
}

- (void)registerEvent:(id)anEvent
{
    if(selectedControl != nil)
    {
        [self setValue:anEvent forKey:[self selectedKey]];
        [selectedControl setState:NSOffState];
        selectedControl = nil;
    }
}

- (void)setSelectedBindingType:(NSInteger)aTag
{
    if([bindingType selectedTag] != aTag)
    {
        [bindingType selectCellWithTag:aTag];
        [self resetKeyBindings];
    }
}

- (void)axisMoved:(OEHIDEvent *)anEvent
{
    if([anEvent isPushed])
    {
        [self registerEvent:anEvent];
        [self setSelectedBindingType:1];
    }
}

- (void)buttonDown:(OEHIDEvent *)anEvent
{
    [self registerEvent:anEvent];
    [self setSelectedBindingType:1];
}

- (void)hatSwitchDown:(OEHIDEvent *)anEvent
{
    [self registerEvent:anEvent];
    [self setSelectedBindingType:1];
}

- (void)keyDown:(NSEvent *)anEvent
{
    [self registerEvent:anEvent];
    [self setSelectedBindingType:0];
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
