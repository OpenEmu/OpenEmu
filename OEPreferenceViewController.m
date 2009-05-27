//
//  OEPreferenceViewController.m
//  OpenEmu
//
//  Created by Remy Demarest on 21/05/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEPreferenceViewController.h"
#import "OEHIDEvent.h"
#import "OEGameCoreController.h"

@interface OEPreferenceViewController ()
- (void)OE_resetBindingsWithKeys:(NSArray *)keys;
@end


@implementation OEPreferenceViewController
@synthesize selectedControl;

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (OEGameCoreController *)controller
{
    return (OEGameCoreController *) [self nextResponder];
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

- (void)OE_resetBindingsWithKeys:(NSArray *)keys
{
    for(NSString *key in keys)
    {
        [self willChangeValueForKey:key];
        [self didChangeValueForKey:key];
    }
}

- (void)resetKeyBindings
{
    [self OE_resetBindingsWithKeys:[[self controller] genericControlNames]];
}

- (void)resetSettingBindings
{
    [self OE_resetBindingsWithKeys:[[self controller] usedSettingNames]];
}

- (BOOL)isKeyboardEventSelected
{
    return YES;
}

- (NSString *)selectedKey
{
    return [[selectedControl infoForBinding:@"title"] objectForKey:NSObservedKeyPathKey];
}

- (NSString *)keyPathForKey:(NSString *)aKey
{
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
    else if([[controller usedSettingNames] containsObject:key])
        return [controller settingForKey:key];
    return [super valueForKey:key];
}

- (void)setValue:(id)value forKey:(NSString *)key
{
    OEGameCoreController *controller = [self controller];
    BOOL isControl = NO, isSetting = NO;
    isControl = [[controller genericControlNames] containsObject:key];
    isSetting = [[controller usedSettingNames] containsObject:key];
    
    NSAssert1(!(isControl && isSetting), @"A key shouldn't be found in both usedControlNames and userSettingNames in controller %@", [self controller]);
        
    if(isControl || isSetting) // should be mutually exclusive
    {
        [self willChangeValueForKey:key];
        if(isControl) [controller registerEvent:value forKey:[self keyPathForKey:key]];
        else if(isSetting) [controller registerSetting:value forKey:[self keyPathForKey:key]];
        [self didChangeValueForKey:key];
    }
    else [super setValue:value forKey:key];
}

@end
