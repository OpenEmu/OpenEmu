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

#import "OEControlsSetupView.h"
#import "OEGameCore.h"

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
- (id)initWithTarget:(OEControlsSetupView *)aTarget;
- (void)parseControlList:(NSArray *)controlList;
- (NSArray *)elementPages;
- (NSDictionary *)keyToButtonMap;
@end

@interface OEControlsSetupView ()
{
	NSArray      *elementPages;
    NSDictionary *keyToButtonMap;
    CGFloat       lastWidth;
}

- (void)OE_layoutSubviews;

@end

static void *const _OEControlsSetupViewFrameSizeContext = (void *)&_OEControlsSetupViewFrameSizeContext;

@implementation OEControlsSetupView
@synthesize selectedKey, action, target;

- (id)initWithFrame:(NSRect)frame
{
    if((self = [super initWithFrame:frame]))
    {
        elementPages = [[NSMutableArray alloc] initWithObjects:[NSMutableArray array], nil];
        NSMutableArray *currentPage = [elementPages lastObject];
        [currentPage addObject:[NSMutableArray array]];
        
        [self addObserver:self forKeyPath:@"frameSize" options:0 context:_OEControlsSetupViewFrameSizeContext];
    }
    
    return self;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if(context == _OEControlsSetupViewFrameSizeContext)
        [self updateButtons];
    else
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

- (void)dealloc
{
    [self removeObserver:self forKeyPath:@"frameSize" context:_OEControlsSetupViewFrameSizeContext];
}

- (void)setupWithControlList:(NSArray *)controlList;
{
    [[[self subviews] copy] makeObjectsPerformSelector:@selector(removeFromSuperview)];
    
    _OEControlsSetupViewParser *parser = [[_OEControlsSetupViewParser alloc] initWithTarget:self];
    [parser parseControlList:controlList];
    
    elementPages   = [parser elementPages];
    keyToButtonMap = [parser keyToButtonMap];
    
    [keyToButtonMap enumerateKeysAndObjectsUsingBlock:
     ^(NSString *key, OEControlsKeyButton *obj, BOOL *stop)
     {
         [obj bind:@"title" toObject:self withKeyPath:[NSString stringWithFormat:@"target.%@", key] options:nil];
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
    }
}

- (void)updateButtons
{
    if(lastWidth == [self frame].size.width)
        return;
    
    [self OE_layoutSubviews];
}

- (void)OE_layoutSubviews;
{
    lastWidth = [self frame].size.width;
    
    CGFloat pageSpacing = 28.0;
    
    // determine required height
    CGFloat viewHeight = MAX([elementPages count] * 119.0 + ([elementPages count] - 1) * pageSpacing + 34.0, 187.0);
    if([self frame].size.height != viewHeight)
    {
        NSRect frame = [self frame];
        frame.size.height = viewHeight;
        [self setFrame:frame];
        
        // return here. KVO will call updateButtons again
        return;
    }
    
    // remove all subviews if any
    [[[self subviews] copy] makeObjectsPerformSelector:@selector(removeFromSuperview)];
    
    // set up some sizes
    const CGFloat topBorder   = 35.0;
    const CGFloat leftBorder  = 61.0;
    const CGFloat rightBorder = 21.0;
    
    const CGFloat verticalItemSpacing  = 10.0; // item bottom to top
    const CGFloat labelHeight          = 24.0;
    const CGFloat labelButtonSpacing   = 8.0;
    
    const CGFloat groupXIndent = 10.0;
    //const CGFloat groupYIndent = -10.0;
    
    __block CGFloat pageY = [self frame].size.height - topBorder;
    
    // iterate through pages
    for(NSMutableArray *aPage in elementPages)
    {
        // iterate through columns
        NSUInteger columns = [aPage count];
        __block CGFloat x = leftBorder;
        
        [aPage enumerateObjectsUsingBlock:
         ^(id aColumn, NSUInteger idx, BOOL *stop)
         {
            CGFloat horizontalItemSpacing = columns == 2 ? 120 : 68.0; // item right to item left
            CGFloat labelWidth            = columns == 2 ? 112 : 60.0; // max value!!!
            
            CGFloat buttonHeight = 24.0;
            CGFloat buttonWidth  = ([self frame].size.width - leftBorder - rightBorder - ((columns - 1) * horizontalItemSpacing)) / columns;
            
            BOOL inGroup = NO;
            CGFloat y = pageY;
            for(NSUInteger j = 0; j < [aColumn count]; j += 2)
            {
                id item = [aColumn objectAtIndex:j];
                
                // handle headline cell
                if([item isKindOfClass:[NSTextField class]] && [[item cell] isKindOfClass:[OEControlsKeyHeadlineCell class]])
                {
                    j--;
                    inGroup = YES;
                    
                    CGFloat columnWidth = buttonWidth + labelWidth + labelButtonSpacing;
                    NSRect headlineFrame = (NSRect){{x - columnWidth, y }, { columnWidth, labelHeight }};
                    [item setFrame:NSIntegralRect(headlineFrame)];
                    [self addSubview:item];
                    
                    continue;
                }
                
                // handle separator
                if([item isKindOfClass:[OEControlsKeySeparatorView class]])
                {
                    j--;
                    
                    CGFloat columnWidth = buttonWidth+labelWidth+labelButtonSpacing;
                    
                    NSRect seperatorLineRect = (NSRect){{ x - labelWidth + 8.0, y - buttonHeight }, { columnWidth - 6.0, buttonHeight }};
                    [item setFrame:NSIntegralRect(seperatorLineRect)];
                    [self addSubview:item];
                    
                    inGroup = NO;
                    y -= buttonHeight+verticalItemSpacing;
                    
                    continue;
                }
                
                // handle buttons + label
                NSRect buttonRect = (NSRect){{ x, y - buttonHeight },{ buttonWidth, buttonHeight }};
                if(inGroup)
                {
                    buttonRect.origin.x   += groupXIndent;
                    buttonRect.size.width -= groupXIndent / 2;
                }
                [item setFrame:NSIntegralRect(buttonRect)];
                
                NSTextField *label = [aColumn objectAtIndex:j + 1];
                NSRect labelRect = NSIntegralRect(NSMakeRect(buttonRect.origin.x - labelWidth - labelButtonSpacing, buttonRect.origin.y - 4, labelWidth, labelHeight));
                
                BOOL multiline = [label attributedStringValue].size.width >= labelRect.size.width;
                if(multiline)
                {
                    labelRect.size.height += 10;
                    labelRect.origin.y    -= 3;
                }
                if(inGroup) labelRect.size.width -= groupXIndent/2;
                [label setFrame:labelRect];
                
                [self addSubview:item];
                [self addSubview:label];
                
                y -= buttonHeight+verticalItemSpacing;
            }
            x += horizontalItemSpacing+buttonWidth;
        }];      
        
        NSView *lastObj = [[self subviews] lastObject];
        pageY -= lastObj.frame.origin.y + lastObj.frame.size.height - 13.0;
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
    else if(p == [elementPages count] - 1)
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
    OEControlsKeyButton *currentButton = [keyToButtonMap objectForKey:[self selectedKey]];
    
    __block OEControlsKeyButton *firstButton       = nil;
    __block OEControlsKeyButton *nextButton        = nil;
    __block BOOL                 selectNext        = NO;
    __block NSInteger            currentButtonPage = 0;
    __block NSInteger            nextButtonPage    = 0;
    
    [elementPages enumerateObjectsUsingBlock:
     ^(NSArray *page, NSUInteger pageIdx, BOOL *stop)
     {
         [page enumerateObjectsUsingBlock:
          ^(NSArray *column, NSUInteger columnIdx, BOOL *stop)
          {
              [column enumerateObjectsUsingBlock:
               ^(OEControlsKeyButton *item, NSUInteger controlIdx, BOOL *stop)
               {
                   if(firstButton == nil && [item isKindOfClass:[OEControlsKeyButton class]])
                       firstButton = item;
                   
                   if(item == currentButton)
                   {
                       currentButtonPage = pageIdx;
                       selectNext        = YES;
                   }
                   else if(selectNext && [item isKindOfClass:[OEControlsKeyButton class]])
                   {
                       nextButton     = item;
                       nextButtonPage = pageIdx;
                       *stop          = YES;
                   }
               }];
              
              if(nextButton != nil) *stop = YES;
          }];
         
         if(nextButton != nil) *stop = YES;
     }];
    
    if(nextButton == nil && [[NSUserDefaults standardUserDefaults] boolForKey:UDControlsButtonHighlightRollsOver])
        nextButton = firstButton;
    
    [self setSelectedKey:[[keyToButtonMap allKeysForObject:nextButton] lastObject]];
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
    OEControlsSetupView *target;
    NSMutableArray      *elementPages;
    NSMutableDictionary *keyToButtonMap;
    
    NSMutableArray      *currentPage;
    NSMutableArray      *currentColumn;
}

- (void)OE_addButtonWithName:(NSString *)aName label:(NSString *)label;
- (void)OE_addColumnLabel:(NSString*)label;
- (void)OE_addRowSeperator;
- (void)OE_addColumn;
- (void)OE_addPage;

@end

@implementation _OEControlsSetupViewParser

- (id)init
{
    return nil;
}

- (id)initWithTarget:(OEControlsSetupView *)aTarget;
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
    elementPages   = [[NSMutableArray      alloc] initWithObjects:[[NSMutableArray alloc] initWithObjects:[[NSMutableArray alloc] init], nil], nil];
    currentPage    = [elementPages lastObject];
    currentColumn  = [currentPage lastObject];
    
    BOOL isFirstPage = YES;
    for(NSArray *page in controlList)
    {
        if(isFirstPage) isFirstPage = NO;
        else            [self OE_addPage];
        
        BOOL isFirstColumn = YES;
        for(NSArray *column in page)
        {
            if(isFirstColumn) isFirstColumn = NO;
            else              [self OE_addColumn];
            
            for(id obj in column)
            {
                if([obj isKindOfClass:[NSString class]])
                {
                    if([obj isEqualToString:@"-"])
                        [self OE_addRowSeperator];
                    else
                        [self OE_addColumnLabel:obj];
                }
                else if([obj isKindOfClass:[NSDictionary class]])
                    [self OE_addButtonWithName:[obj objectForKey:OEControlListKeyNameKey]
                                         label:[[obj objectForKey:OEControlListKeyLabelKey] stringByAppendingString:@":"]];
            }
        }
    }
}

- (NSArray *)elementPages;
{
    return [elementPages copy];
}

- (NSArray *)keyToButtonMap;
{
    return [keyToButtonMap copy];
}

- (void)OE_addButtonWithName:(NSString *)aName label:(NSString *)aLabel;
{
    OEControlsKeyButton *button = [[OEControlsKeyButton alloc] initWithFrame:NSZeroRect];
    
    [button setTarget:target];
    [button setAction:@selector(OE_selectInputControl:)];
    
    [keyToButtonMap setObject:button forKey:aName];
    
    [currentColumn addObject:button];
    
    NSTextField     *labelField     = [[NSTextField alloc] initWithFrame:NSZeroRect];
    NSTextFieldCell *labelFieldCell = [[OEControlsKeyLabelCell alloc] init];
    
    [labelField setCell:labelFieldCell];
    [labelField setStringValue:aLabel];
    
    [currentColumn addObject:labelField];
}

- (void)OE_addColumnLabel:(NSString *)label;
{
    NSTextField *labelField = [[NSTextField alloc] initWithFrame:NSZeroRect];
    NSTextFieldCell *labelFieldCell = [[OEControlsKeyHeadlineCell alloc] init];
    [labelField setCell:labelFieldCell];
    [labelField setStringValue:label];
    [currentColumn addObject:labelField];
}

- (void)OE_addRowSeperator;
{
    OEControlsKeySeparatorView *view = [[OEControlsKeySeparatorView alloc] init];
    [currentColumn addObject:view];
}

- (void)OE_addColumn;
{
    currentColumn = [[NSMutableArray alloc] init];
    [currentPage addObject:currentColumn];
}

- (void)OE_addPage;
{
    currentPage = [[NSMutableArray alloc] init];
    [elementPages addObject:currentPage];
    
    [self OE_addColumn];
}

@end
