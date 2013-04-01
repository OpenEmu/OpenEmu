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

#import "OEControlsButtonSetupView.h"

#import "OEControlsKeyButton.h"
#import "OEControlsKeyLabelCell.h"
#import "OEControlsKeyHeadlineCell.h"
#import "OEControlsKeySeparatorView.h"

#import "OESystemController.h"

#import "NSClipView+OEAnimatedScrolling.h"

#import "OEUIDrawingUtils.h"

#ifndef UDControlsButtonHighlightRollsOver
#define UDControlsButtonHighlightRollsOver @"ButtonHighlightRollsOver"
#endif

@interface _OEControlsSetupViewParser : NSObject
- (id)initWithTarget:(OEControlsButtonSetupView *)aTarget;
- (void)parseControlList:(NSArray *)controlList;
- (NSArray *)elementGroups;
- (NSArray *)orderedKeys;
- (NSDictionary *)keyToButtonMap;
- (NSUInteger)numberOfRows;
@end

@interface OEControlsButtonSetupView ()
{
	NSArray      *elementGroups;
    NSArray      *orderedKeys;
    NSDictionary *keyToButtonMap;
    NSUInteger    numberOfRows;
}

- (void)OE_layoutSubviews;

@end

static void *const _OEControlsSetupViewFrameSizeContext = (void *)&_OEControlsSetupViewFrameSizeContext;

@implementation OEControlsButtonSetupView
@synthesize selectedKey, action, target, bindingsProvider;

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    
    return self;
}

- (void)setFrame:(NSRect)frameRect
{
    NSScrollView* enclosingScrollView = [self enclosingScrollView];
    if(enclosingScrollView && [enclosingScrollView hasVerticalScroller] && [enclosingScrollView scrollerStyle] == NSScrollerStyleLegacy)
        frameRect.size.width = MIN(frameRect.size.width, [self visibleRect].size.width);
    
    [super setFrame:frameRect];
}

- (void)setupWithControlList:(NSArray *)controlList;
{
    [[[self subviews] copy] makeObjectsPerformSelector:@selector(removeFromSuperview)];
    
    _OEControlsSetupViewParser *parser = [[_OEControlsSetupViewParser alloc] initWithTarget:self];
    [parser parseControlList:controlList];
    
    elementGroups   = [parser elementGroups];
    keyToButtonMap  = [parser keyToButtonMap];
    orderedKeys     = [parser orderedKeys];
    numberOfRows    = [parser numberOfRows];
    
    [keyToButtonMap enumerateKeysAndObjectsUsingBlock:
     ^(NSString *key, OEControlsKeyButton *obj, BOOL *stop)
     {
         [obj bind:@"title" toObject:self withKeyPath:[NSString stringWithFormat:@"bindingsProvider.%@", key] options:
          @{ NSNullPlaceholderBindingOption : @"" }];
     }];

    [self OE_layoutSubviews];
}

- (IBAction)OE_selectInputControl:(id)sender
{
    // Only one object should be there
    NSString *key = [[keyToButtonMap allKeysForObject:sender] lastObject];
    
    if(key == nil) return;
    
    [self setSelectedKey:key];
    
    if([self action] != NULL) [NSApp sendAction:[self action] to:[self target] from:self];
}

- (void)setSelectedKey:(NSString *)value
{
    OEControlsKeyButton *button = [keyToButtonMap objectForKey:value];
    if(button == nil) value = nil;
    
    if(selectedKey != value)
    {
        OEControlsKeyButton *previous = [keyToButtonMap objectForKey:selectedKey];
        
        selectedKey = [value copy];

        [previous setState:NSOffState];
        [button   setState:NSOnState];

        NSClipView *clipView = [[self enclosingScrollView] contentView];
        [clipView scrollToPoint:[button frame].origin animated:YES];
    }
}

- (void)OE_layoutSubviews;
{
    // remove all subviews if any
    [[[self subviews] copy] makeObjectsPerformSelector:@selector(removeFromSuperview)];
    
    // set up some sizes
    const CGFloat width               = 213.0;
    const CGFloat leftGap             =  16.0;
    const CGFloat rightGap            =  16.0;
    const CGFloat itemHeight          =  24.0;
    const CGFloat verticalItemSpacing =   9.0; // item bottom to top
    const CGFloat labelButtonSpacing  =   8.0;
    const CGFloat buttonWidth         = 118.0;

    // determine required height
    CGFloat viewHeight = (numberOfRows + 1) * verticalItemSpacing + numberOfRows * itemHeight;
    if([self frame].size.height != viewHeight)
    {
        NSRect frame = [self frame];
        frame.size.height = viewHeight;
        [self setFrame:frame];
    }

    CGFloat y = self.frame.size.height - verticalItemSpacing;

    for(NSArray *group in elementGroups)
    {
        for(NSUInteger j = 0; j < [group count]; j += 2)
        {
            id item = [group objectAtIndex:j];

            // handle headline cell
            if([item isKindOfClass:[NSTextField class]] && [[item cell] isKindOfClass:[OEControlsKeyHeadlineCell class]])
            {
                j--;

                NSRect headlineFrame = (NSRect){{leftGap, y - itemHeight }, { width - leftGap - rightGap, itemHeight }};
                [item setFrame:NSIntegralRect(headlineFrame)];
                [self addSubview:item];

                y -= itemHeight + verticalItemSpacing;

                continue;
            }

            // handle separator
            if([item isKindOfClass:[OEControlsKeySeparatorView class]])
            {
                j--;

                NSRect seperatorLineRect = (NSRect){{ leftGap, y - itemHeight }, { width - leftGap - rightGap, itemHeight }};
                [item setFrame:NSIntegralRect(seperatorLineRect)];
                [self addSubview:item];

                y -= itemHeight + verticalItemSpacing;

                continue;
            }

            // handle buttons + label
            NSRect buttonRect = (NSRect){{ width - rightGap - buttonWidth, y - itemHeight },{ buttonWidth, itemHeight }};
            [item setFrame:NSIntegralRect(buttonRect)];

            NSTextField *label = [group objectAtIndex:j + 1];
            NSRect labelRect = NSIntegralRect(NSMakeRect(leftGap, buttonRect.origin.y - 4, width - leftGap - labelButtonSpacing - buttonWidth - rightGap, itemHeight));

            
            BOOL multiline = [label attributedStringValue].size.width >= labelRect.size.width;
            if(multiline)
            {
                labelRect.size.height += 10;
                labelRect.origin.y    -= 3;
            }
            [label setFrame:labelRect];

            [self addSubview:item];
            [self addSubview:label];

            y -= itemHeight + verticalItemSpacing;
        }
    }
}

#pragma mark -

- (void)scrollToPage:(NSUInteger)p
{
    NSClipView *clipView = [[self enclosingScrollView] contentView];
    CGFloat y = 0;
    if(p == 0)
    {
        y = [self frame].size.height;
    }
    else if(p == [elementGroups count] - 1)
    {
        y = 0;
    }
    else
    {
        // TODO: i was lazy... calculate something here if we need that
    }
    [clipView scrollToPoint:(NSPoint){0,y} animated:YES];
}

- (void)selectNextKeyButton;
{
    if([orderedKeys count] <= 1) return;
    
    NSUInteger i = [orderedKeys indexOfObject:[self selectedKey]];
    
    NSString *newKey = nil;
    
    if(i + 1 >= [orderedKeys count])
    {
        if([[NSUserDefaults standardUserDefaults] boolForKey:UDControlsButtonHighlightRollsOver])
            newKey = [orderedKeys objectAtIndex:0];
    }
    else newKey = [orderedKeys objectAtIndex:i + 1];
    
    [self setSelectedKey:newKey];
}

- (void)selectNextKeyAfterKeys:(NSArray *)keys;
{
    NSIndexSet *indexes = [orderedKeys indexesOfObjectsPassingTest:
                           ^ BOOL (id obj, NSUInteger idx, BOOL *stop)
                           {
                               return [keys containsObject:obj];
                           }];
    NSUInteger i = [indexes lastIndex];
    
    NSString *newKey = nil;
    
    if(i + 1 >= [orderedKeys count])
    {
        if([[NSUserDefaults standardUserDefaults] boolForKey:UDControlsButtonHighlightRollsOver])
            newKey = [orderedKeys objectAtIndex:0];
    }
    else newKey = [orderedKeys objectAtIndex:i + 1];
    
    [self setSelectedKey:newKey];
}

#pragma mark -

- (void)scrollWheel:(NSEvent *)theEvent
{
    [[self superview] scrollWheel:theEvent];
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)keyUp:(NSEvent *)theEvent
{
}

- (void)keyDown:(NSEvent *)theEvent
{
}

@end

@interface _OEControlsSetupViewParser ()
{
    OEControlsButtonSetupView *target;
    NSMutableArray      *elementGroups;
    NSMutableArray      *orderedKeys;
    NSMutableDictionary *keyToButtonMap;
    NSUInteger           numberOfRows;
    
    NSMutableArray      *currentGroup;
}

- (void)OE_addButtonWithName:(NSString *)aName label:(NSString *)label;
- (void)OE_addGroupLabel:(NSString*)label;
- (void)OE_addRowSeperator;
- (void)OE_addGroup;

@end

@implementation _OEControlsSetupViewParser

- (id)init
{
    return nil;
}

- (id)initWithTarget:(OEControlsButtonSetupView *)aTarget;
{
    if((self = [super init]))
    {
        target = aTarget;
    }
    return self;
}

- (void)parseControlList:(NSArray *)controlList;
{
    keyToButtonMap = [[NSMutableDictionary alloc] init];
    orderedKeys    = [[NSMutableArray      alloc] init];
    elementGroups  = [[NSMutableArray      alloc] init];
    currentGroup   = nil;
    numberOfRows   = 0;

    for(NSArray *group in controlList)
    {
        [self OE_addGroup];

        for(id row in group)
        {
            ++numberOfRows;
            if([row isKindOfClass:[NSString class]])
            {
                if([row isEqualToString:@"-"])
                    [self OE_addRowSeperator];
                else
                    [self OE_addGroupLabel:row];
            }
            else if([row isKindOfClass:[NSDictionary class]])
                [self OE_addButtonWithName:[row objectForKey:OEControlListKeyNameKey]
                                     label:[[row objectForKey:OEControlListKeyLabelKey] stringByAppendingString:@":"]];
        }
    }

    [elementGroups addObject:currentGroup];
    currentGroup = nil;
}

- (NSArray *)elementGroups;
{
    return [elementGroups copy];
}

- (NSArray *)keyToButtonMap;
{
    return [keyToButtonMap copy];
}

- (NSArray *)orderedKeys;
{
    return [orderedKeys copy];
}

- (NSUInteger)numberOfRows;
{
    return numberOfRows;
}

- (void)OE_addButtonWithName:(NSString *)aName label:(NSString *)aLabel;
{
    OEControlsKeyButton *button = [[OEControlsKeyButton alloc] initWithFrame:NSZeroRect];
    
    [button setTarget:target];
    [button setAction:@selector(OE_selectInputControl:)];
    
    [orderedKeys    addObject:aName];
    [keyToButtonMap setObject:button forKey:aName];
    
    [currentGroup addObject:button];
    
    NSTextField     *labelField     = [[NSTextField alloc] initWithFrame:NSZeroRect];
    NSTextFieldCell *labelFieldCell = [[OEControlsKeyLabelCell alloc] init];
    
    [labelField setCell:labelFieldCell];
    [labelField setStringValue:aLabel];
    
    [currentGroup addObject:labelField];
}

- (void)OE_addGroupLabel:(NSString *)label;
{
    NSTextField *labelField = [[NSTextField alloc] initWithFrame:NSZeroRect];
    NSTextFieldCell *labelFieldCell = [[OEControlsKeyHeadlineCell alloc] init];
    [labelField setCell:labelFieldCell];
    [labelField setStringValue:label];
    [currentGroup addObject:labelField];
}

- (void)OE_addRowSeperator;
{
    OEControlsKeySeparatorView *view = [[OEControlsKeySeparatorView alloc] init];
    [currentGroup addObject:view];
}

- (void)OE_addGroup;
{
    if(currentGroup)
        [elementGroups addObject:currentGroup];
    
    currentGroup = [[NSMutableArray alloc] init];
}

@end
