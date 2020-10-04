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

#import <OpenEmuSystem/OpenEmuSystem.h>

#import "OpenEmu-Swift.h"

@import Quartz;

NSString * const OEControlsButtonHighlightRollsOver = @"ButtonHighlightRollsOver";

@interface _OEControlsSetupViewParser : NSObject
- (id)initWithTarget:(OEControlsButtonSetupView *)aTarget;
- (void)parseControlList:(NSArray *)controlList;
- (NSArray *)sections;
- (NSArray *)orderedKeys;
- (NSDictionary *)keyToButtonMap;
@end

@interface OEControlsButtonSetupView ()
{
    NSArray      *sections;
    NSArray      *orderedKeys;
    NSDictionary *keyToButtonMap;
}
NSComparisonResult headerSortingFunction(id obj1, id obj2, void *context);
@end

static void *const _OEControlsSetupViewFrameSizeContext = (void *)&_OEControlsSetupViewFrameSizeContext;

@implementation OEControlsButtonSetupView
@synthesize selectedKey, action, target, bindingsProvider;

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    return self;
}

- (void)setupWithControlList:(NSArray *)controlList;
{
    [[[self subviews] copy] makeObjectsPerformSelector:@selector(removeFromSuperview)];
    
    _OEControlsSetupViewParser *parser = [[_OEControlsSetupViewParser alloc] initWithTarget:self];
    [parser parseControlList:controlList];

    sections        = [parser sections];
    keyToButtonMap  = [parser keyToButtonMap];
    orderedKeys     = [parser orderedKeys];
    
    [keyToButtonMap enumerateKeysAndObjectsUsingBlock:
     ^(NSString *key, OEControlsKeyButton *obj, BOOL *stop)
     {
         [obj bind:@"title" toObject:self withKeyPath:[NSString stringWithFormat:@"bindingsProvider.%@", key] options:
          @{ NSNullPlaceholderBindingOption : @"" }];
     }];

    [self setupSubviews];
    [self layoutSubviews:NO];
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

        [previous setState:NSControlStateValueOff];
        [button   setState:NSControlStateValueOn];

        if(value && !NSWorkspace.sharedWorkspace.isVoiceOverEnabled)
        {
            // Scroll field into view
            NSClipView *clipView             = [[self enclosingScrollView] contentView];
            NSRect      convertedButtonFrame = [self convertRect:[button frame] toView:clipView];

            convertedButtonFrame.origin.x  = 0;
            convertedButtonFrame.origin.y -= [clipView frame].size.height / 2;
            convertedButtonFrame = [self backingAlignedRect:convertedButtonFrame options:NSAlignAllEdgesNearest];
            [[clipView animator] setBoundsOrigin:convertedButtonFrame.origin];
        }
    }
}

#pragma mark - Drawing and Layout
static const CGFloat width               = 227.0;
static const CGFloat topGap              =  16.0;
static const CGFloat bottomGap           =  16.0;
static const CGFloat leftGap             =  16.0;
static const CGFloat itemHeight          =  24.0;
static const CGFloat verticalItemSpacing =   9.0; // item bottom to top
static const CGFloat labelButtonSpacing  =   5.0;
static const CGFloat buttonWidth         = 130.0;
static const CGFloat minimumFrameHeight  = 259.0;

static const CGFloat sectionTitleHeight  =  25.0;

NSComparisonResult headerSortingFunction(id obj1, id obj2, void *context)
{
    BOOL obj1IsHeader = [obj1 isKindOfClass:[OEControlsSectionTitleView class]];
    BOOL obj2IsHeader = [obj2 isKindOfClass:[OEControlsSectionTitleView class]];

    if(obj1IsHeader ^ obj2IsHeader)
        return obj1IsHeader ?  NSOrderedDescending : NSOrderedAscending;
    return NSOrderedSame;
}

- (void)setupSubviews
{
    // remove all subviews if any
    [[[self subviews] copy] makeObjectsPerformSelector:@selector(removeFromSuperview)];

    for(NSDictionary *section in sections)
    {
        OEControlsSectionTitleView *heading = [section objectForKey:@"header"];
        [self addSubview:heading];

        NSArray *groups = [section objectForKey:@"groups"];
        for(NSArray *group in groups)
            for(NSUInteger j = 0; j < [group count]; j += 2)
            {
                id item = [group objectAtIndex:j];
                [self addSubview:item];
                
                // handle headline cell
                if([item isKindOfClass:[OEControlsKeyHeadline class]])
                {
                    j--;
                    continue;
                }

                // handle separator
                if([item isKindOfClass:[OEControlsKeySeparatorView class]])
                {
                    j--;
                    continue;
                }

                // handle buttons + label
                NSTextField *label = [group objectAtIndex:j + 1];                
                [self addSubview:label];
            }
    }

    // Sort so that section headers are on top
    [self sortSubviewsUsingFunction:headerSortingFunction context:nil];
}

- (void)layoutSubviews:(BOOL)animated
{
#define animated(_X_) animated?[_X_ animator]:_X_
    if(animated) [CATransaction begin];

    CGFloat rightGap = 14 + (([NSScroller preferredScrollerStyle] == NSScrollerStyleLegacy) ? 14.0 : 0.0);

    // determine required height
    NSRect frame = [self frame];
    CGFloat viewHeight = [self OE_calculateViewHeight];
    if(NSHeight(frame) != viewHeight)
    {
        frame.size.height = viewHeight > minimumFrameHeight ? viewHeight : minimumFrameHeight;
        [animated(self) setFrame:frame];
    }

    CGFloat y = NSHeight(frame);
    for(NSDictionary *section in sections)
    {
        OEControlsSectionTitleView *heading = animated([section objectForKey:@"header"]);
        NSArray *groups       = [section objectForKey:@"groups"];

        // layout section header
        NSPoint headerOrigin = {0, y - sectionTitleHeight };
        NSRect  headerFrame  = (NSRect){headerOrigin, { width+2.0, sectionTitleHeight }};
        [heading setFrame:headerFrame];

        y -= headerFrame.size.height + topGap;

        for(NSArray *group in groups)
        {
            for(NSUInteger j = 0; j < [group count]; j += 2)
            {
                id item = animated([group objectAtIndex:j]);

                // handle headline cell
                if([item isKindOfClass:[OEControlsKeyHeadline class]])
                {
                    j--;

                    NSRect headlineFrame = (NSRect){{leftGap, y - itemHeight }, { width - leftGap - rightGap, itemHeight }};
                    [item setFrame:NSIntegralRect(headlineFrame)];
                    y -= itemHeight + verticalItemSpacing;

                    continue;
                }

                // handle separator
                if([item isKindOfClass:[OEControlsKeySeparatorView class]])
                {
                    j--;

                    NSRect seperatorLineRect = (NSRect){{ leftGap, y - itemHeight }, { width - leftGap - rightGap, itemHeight }};
                    [item setFrame:NSIntegralRect(seperatorLineRect)];
                    y -= itemHeight + verticalItemSpacing;

                    continue;
                }

                // handle buttons + label
                NSRect buttonRect = (NSRect){{ width - buttonWidth, y - itemHeight },{ buttonWidth - rightGap, itemHeight }};
                [item setFrame:NSIntegralRect(buttonRect)];

                NSTextField *label = animated([group objectAtIndex:j + 1]);
                NSRect labelRect = NSIntegralRect(NSMakeRect(leftGap, buttonRect.origin.y + buttonRect.size.height/2 + 1, width - leftGap - labelButtonSpacing - buttonWidth, 100000));
                
                NSSize labelFitSize = [label.cell cellSizeForBounds:labelRect];
                if (labelFitSize.height > 30)
                {
                    /* If the label size returned is too tall, enlarge the
                     * label to attempt fitting in 2 lines anyway */
                    labelRect.origin.x -= 5;
                    labelRect.size.width += 5;
                    labelFitSize = [label.cell cellSizeForBounds:labelRect];
                }
                labelRect.origin.y -= labelFitSize.height / 2;
                labelRect.size.height = labelFitSize.height;
                
                [label setFrame:labelRect];
                
                y -= itemHeight + verticalItemSpacing;
            }
        }
        y += verticalItemSpacing;
        y -= bottomGap;
    }

    [self layoutSectionHeadings:nil];

    if(animated) [CATransaction commit];
#undef animated
}

- (CGFloat)OE_calculateViewHeight
{
    CGFloat height = 0;
    for(NSDictionary *section in sections)
    {
        height += [self OE_calculateHeightOfSection:section];
    }
    return height;
}

- (CGFloat)OE_calculateHeightOfSection:(NSDictionary*)section;
{
    CGFloat height = sectionTitleHeight;
    
    height += topGap+bottomGap;
    NSUInteger numberOfRows = [[section objectForKey:@"numberOfRows"] unsignedIntegerValue];
    height += (numberOfRows - 1) * verticalItemSpacing + numberOfRows * itemHeight;
    
    return height;
}

- (void)viewWillMoveToSuperview:(NSView *)newSuperview
{
    NSClipView *oldClipView  = [[self enclosingScrollView] contentView];
    
    if(oldClipView != nil)
    {
        NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
        [nc removeObserver:self name:NSViewBoundsDidChangeNotification object:oldClipView];
    }
}

- (void)viewDidMoveToSuperview
{
    NSClipView *clipView  = [[self enclosingScrollView] contentView];
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];

    [clipView setPostsBoundsChangedNotifications:YES];
    [nc addObserver:self selector:@selector(layoutSectionHeadings:) name:NSViewBoundsDidChangeNotification object:clipView];

    [self layoutSectionHeadings:nil];
}

- (void)layoutSectionHeadings:(id)sender
{
    NSInteger i;
    CGFloat minY = 0;
    for(i=[sections count]-1; i>=0; i--)
    {
        NSDictionary *section = [sections objectAtIndex:i];
        OEControlsSectionTitleView *sectionHeader = [section objectForKey:@"header"];

        CGFloat sectionStart  = [self OE_headerPositionOfSectionAtIndex:i];
        CGFloat sectionHeight = [self OE_calculateHeightOfSection:section];

        NSRect sectionRect = (NSRect){{0, sectionStart-sectionHeight}, {width, sectionHeight}};
        NSRect visibleRect = [self visibleRect];
        NSRect visibleSectionRect = NSIntersectionRect(visibleRect, sectionRect);

        if(!NSEqualRects(visibleSectionRect, NSZeroRect))
        {
            [sectionHeader setFrameOrigin:(NSPoint){0,MAX(minY, NSMaxY(visibleSectionRect)-sectionTitleHeight)}];
            minY = NSMaxY(visibleSectionRect);

            if(minY>=NSMaxY(visibleRect)-sectionTitleHeight-1.0)
                [sectionHeader setPinned:YES];
            else
                [sectionHeader setPinned:NO];
        }
        else
            [sectionHeader setPinned:NO];
    }
    ;
}

- (CGFloat)OE_headerPositionOfSectionAtIndex:(NSInteger)i
{
    CGFloat y= NSHeight([self bounds]);
    for(NSUInteger j=0; j < i; j++)
    {
        y -= [self OE_calculateHeightOfSection:[sections objectAtIndex:j]];
    }
    return y;
}
#pragma mark -
- (void)selectNextKeyButton;
{
    if([orderedKeys count] <= 1) return;
    
    NSUInteger i = [orderedKeys indexOfObject:[self selectedKey]];
    
    NSString *newKey = nil;
    
    if(i + 1 >= [orderedKeys count])
    {
        if([[NSUserDefaults standardUserDefaults] boolForKey:OEControlsButtonHighlightRollsOver])
            newKey = [orderedKeys objectAtIndex:0];
    }
    else
    {
        newKey = [orderedKeys objectAtIndex:i + 1];
    }

    [CATransaction begin];
    [self setSelectedKey:newKey];
    [CATransaction commit];
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
        if([[NSUserDefaults standardUserDefaults] boolForKey:OEControlsButtonHighlightRollsOver])
            newKey = [orderedKeys objectAtIndex:0];
    }
    else {
        newKey = [orderedKeys objectAtIndex:i + 1];
    }

    [CATransaction begin];
    [self setSelectedKey:newKey];
    [CATransaction commit];
}

- (NSDictionary*)OE_sectionOfKey:(NSString*)key
{
    id button = [keyToButtonMap objectForKey:key];
    for(NSDictionary *section in sections)
        for(NSDictionary *group in [section objectForKey:@"groups"])
            for(id element in group)
                if(element == button)
                    return section;
    return nil;
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

- (void)toggleSection:(OEControlsSectionTitleView*)sectionHeader
{
    [self layoutSubviews:YES];   
}

@end

@interface _OEControlsSetupViewParser ()
{
    OEControlsButtonSetupView *target;
    NSMutableArray      *sections;
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
- (id)OE_createSectionHeadingWithName:(NSString*)name;
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
    sections       = [[NSMutableArray      alloc] init];
    elementGroups  = [[NSMutableArray      alloc] init];
    currentGroup   = nil;
    numberOfRows   = 0;

    NSAssert([controlList count]%2==0, @"control list has to have an even number of items (headline and group pairs)");
    for(NSUInteger i=0; i < [controlList count]; i+=2) // Sections
    {
        NSString *sectionTitle    = NSLocalizedStringFromTable([controlList objectAtIndex:i], @"ControlLabels", @"Section Title");
        NSArray  *sectionContents = [controlList objectAtIndex:i+1];

        for(NSArray *group in sectionContents)
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
                        [self OE_addGroupLabel:NSLocalizedStringFromTable(row, @"ControlLabels", @"Group Label")];
                }
                else if([row isKindOfClass:[NSDictionary class]]) {
                    NSString *fontFamily = nil;

                    if([row objectForKey:OEControlListKeyFontFamilyKey] != nil) {
                        fontFamily = [row objectForKey:OEControlListKeyFontFamilyKey];
                    }

                    NSString *label = NSLocalizedStringFromTable([row objectForKey:OEControlListKeyLabelKey], @"ControlLabels", @"Button Label");

                    [self OE_addButtonWithName:[row objectForKey:OEControlListKeyNameKey]
                                         label:[label stringByAppendingString:@":"]];
                }
            }
        }
        if(currentGroup != nil)
            [elementGroups addObject:currentGroup];

        [sections addObject:@{
         @"numberOfRows" : @(numberOfRows),
               @"groups" : elementGroups,
              @"header" : [self OE_createSectionHeadingWithName:sectionTitle]
         }];
        
        elementGroups = [[NSMutableArray alloc] init];
        currentGroup = nil;
        numberOfRows = 0;
    }
}

- (NSArray *)sections;
{
    return [sections copy];
}

- (NSArray *)keyToButtonMap;
{
    return [keyToButtonMap copy];
}

- (NSArray *)orderedKeys;
{
    return [orderedKeys copy];
}

- (void)OE_addButtonWithName:(NSString *)aName label:(NSString *)aLabel;
{
    OEControlsKeyButton *button = [[OEControlsKeyButton alloc] initWithFrame:NSZeroRect];
    
    [button setTarget:target];
    [button setAction:@selector(OE_selectInputControl:)];
    button.label = aLabel;
    
    [orderedKeys    addObject:aName];
    [keyToButtonMap setObject:button forKey:aName];
    
    [currentGroup addObject:button];
    
    OEControlsKeyLabel *labelField = [[OEControlsKeyLabel alloc] initWithFrame:NSZeroRect];

    [labelField setStringValue:aLabel];

    [currentGroup addObject:labelField];
}

- (void)OE_addGroupLabel:(NSString *)label;
{
    OEControlsKeyHeadline *labelField = [[OEControlsKeyHeadline alloc] initWithFrame:NSZeroRect];
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

- (id)OE_createSectionHeadingWithName:(NSString*)name
{
    OEControlsSectionTitleView *labelField = [[OEControlsSectionTitleView alloc] initWithFrame:NSZeroRect];
    [labelField setStringValue:name];

    return labelField;
}


@end
