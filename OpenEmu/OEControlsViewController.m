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

#import "OEControlsViewController.h"
#import "OEHIDEvent.h"
#import "OEGameCoreController.h"
#import "OEControlsSetupView.h"
#import "OESystemController.h"

@implementation OEControlsViewController
{
    OEHIDEventAxis readingAxis;
}

@synthesize selectedControl, delegate;


- (void)awakeFromNib
{
    [super awakeFromNib];
    
    selectedPlayer = 1;
    selectedBindingType = 0;
    
    OEControlsSetupView *view = (OEControlsSetupView *)[self view];
    
    NSArray *pages = [[self delegate] controlPageListInControlsViewController:self];
    
    BOOL isFirstPage = YES;
    for(NSArray *page in pages)
    {
        if(isFirstPage) isFirstPage = NO;
        else            [view nextPage];
        
        BOOL isFirstColumn = YES;
        for(NSArray *column in page)
        {
            if(isFirstColumn) isFirstColumn = NO;
            else              [view nextColumn];
            
            for(id obj in column)
            {
                if([obj isKindOfClass:[NSString class]])
                {
                    if([obj isEqualToString:@"-"])
                        [view addRowSeperator];
                    else
                        [view addColumnLabel:obj];
                }
                else if([obj isKindOfClass:[NSDictionary class]])
                    [view addButtonWithName:[obj objectForKey:OEControlListKeyNameKey]
                                      label:[[obj objectForKey:OEControlListKeyLabelKey] stringByAppendingString:@":"]
                                     target:self
                             highlightPoint:NSPointFromString([obj objectForKey:OEControlListKeyPositionKey] ? : @"{0, 0}")];
            }
        }
    }
    
    [view updateButtons];
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (IBAction)selectInputControl:(id)sender
{
    id lastControl = selectedControl;
    if(sender == nil || [sender respondsToSelector:@selector(state)])
    {
        NSInteger state = [sender state];
        
        [selectedControl setState:NSOffState];
        [[sender window] makeFirstResponder:(state == NSOnState ? [self view] : lastControl)];
        selectedControl = (state == NSOnState ? sender : nil);
    }
    
    if(!selectedControl || ![selectedControl state])
    {
        [[NSNotificationCenter defaultCenter] postNotificationName:@"OEControlsPreferencesSelectedButtonDidChange" object:nil];
    } 
    else 
    {
        [[NSNotificationCenter defaultCenter] postNotificationName:@"OEControlsPreferencesSelectedButtonDidChange" object:selectedControl];
    }
}

- (void)resetBindingsWithKeys:(NSArray *)keys
{
    for(NSString *key in keys)
    {
        [self willChangeValueForKey:key];
        [self didChangeValueForKey:key];
    }
}

- (void)resetKeyBindings
{
    [self resetBindingsWithKeys:[[self delegate] genericControlNamesInControlsViewController:self]];
}

- (BOOL)isKeyboardEventSelected
{
    return selectedBindingType == 0;
}

- (NSString *)selectedKey
{
    return [[selectedControl infoForBinding:@"title"] objectForKey:NSObservedKeyPathKey];
}

- (NSUInteger)selectedPlayer
{
    return selectedPlayer;
}
- (void)selectPlayer:(NSUInteger)_player
{
    selectedPlayer = _player;
    [self resetKeyBindings];
}

- (NSString *)keyPathForKey:(NSString *)aKey
{
    NSUInteger player = [self selectedPlayer];
    if(player != NSNotFound)
    {
        NSString *keyPathForKey = [[self delegate] controlsViewController:self playerKeyForKey:aKey player:player];
        return keyPathForKey;
    }  
    else
        return aKey;
}

- (void)registerEvent:(id)anEvent
{
    if(selectedControl != nil)
    {
        id button = selectedControl;
        id gameControllerView = [selectedControl superview];
        [self setValue:anEvent forKey:[self selectedKey]];
        
        [self selectInputControl:nil];
        
        
        if([gameControllerView respondsToSelector:@selector(selectNextKeyButton:)]) [gameControllerView performSelector:@selector(selectNextKeyButton:) withObject:button];
    }
}

- (void)selectBindingType:(NSInteger)newType
{
    if(selectedBindingType != newType)
    {
        selectedBindingType = newType;
        [self resetKeyBindings];
        [[NSNotificationCenter defaultCenter] postNotificationName:@"OEControlsViewControllerChangedBindingType" object:self];
    }
}

- (NSInteger)selectedBindingType
{
    return selectedBindingType;
}

- (void)keyDown:(NSEvent *)theEvent
{
    
}

- (void)keyUp:(NSEvent *)theEvent
{
    
}

- (void)axisMoved:(OEHIDEvent *)anEvent
{
    OEHIDEventAxis axis = [anEvent axis];
    OEHIDDirection dir  = [anEvent direction];
    
    if(readingAxis == OEHIDAxisNone && axis != OEHIDAxisNone && dir != OEHIDDirectionNull)
    {
        readingAxis = axis;
        
        [self registerEvent:anEvent];
        [self selectBindingType:1];
    }
    else if(readingAxis == axis && dir == OEHIDDirectionNull)
        readingAxis = OEHIDAxisNone;
}

- (void)buttonDown:(OEHIDEvent *)anEvent
{
    [self registerEvent:anEvent];
    [self selectBindingType:1];
}

- (void)hatSwitchChanged:(OEHIDEvent *)anEvent;
{
    if([anEvent position] != 0)
    {
        [self registerEvent:anEvent];
        [self selectBindingType:1];
    }
}

- (void)HIDKeyDown:(OEHIDEvent *)anEvent
{
    [self registerEvent:anEvent];
    [self selectBindingType:0];
}

- (id)valueForKey:(NSString *)key
{
    if([[[self delegate] genericControlNamesInControlsViewController:self] containsObject:key])
    {
        id anEvent = nil;
        if([self isKeyboardEventSelected])
            anEvent = [[self delegate] controlsViewController:self keyboardEventForKey:[self keyPathForKey:key]];
        else
            anEvent = [[self delegate] controlsViewController:self HIDEventForKey:[self keyPathForKey:key]];
        
        return (anEvent != nil ? [anEvent displayDescription] : @"<empty>");
    }
    return [super valueForKey:key];
}

- (void)setValue:(id)value forKey:(NSString *)key
{
    // should be mutually exclusive
    if([[[self delegate] genericControlNamesInControlsViewController:self] containsObject:key])
    {
        [self willChangeValueForKey:key];
        [[self delegate] controlsViewController:self registerEvent:value forKey:[self keyPathForKey:key]];
        [self didChangeValueForKey:key];
    }
    else [super setValue:value forKey:key];
}


- (NSImage*)controllerImage
{
    return nil;
}

- (NSImage *)controllerImageMask
{
    return nil;
}

@end
