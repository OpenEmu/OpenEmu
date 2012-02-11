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

#import "NSClipView+OEAnimatedScrolling.h"

#import "OEUIDrawingUtils.h"

#ifndef UDControlsButtonHighlightRollsOver
#define UDControlsButtonHighlightRollsOver @"ButtonHighlightRollsOver"
#endif
@implementation OEControlsSetupView
- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        elementPages = [[NSMutableArray alloc] initWithObjects:[NSMutableArray array], nil];
        NSMutableArray *currentPage = [elementPages lastObject];
        [currentPage addObject:[NSMutableArray array]];
        
        [self addObserver:self forKeyPath:@"frameSize" options:0 context:nil];
    }
    return self;
}
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    [self updateButtons];
}

- (void)dealloc
{
    [self removeObserver:self forKeyPath:@"frameSize"];
}

- (void)addButtonWithName:(NSString *)aName target:(id)aTarget startPosition:(NSPoint)start endPosition:(NSPoint)end
{
}

- (void)addButtonWithName:(NSString *)aName toolTip:(NSString *)aToolTip target:(id)aTarget startPosition:(NSPoint)start endPosition:(NSPoint)end
{
}
#pragma mark -
- (void)addButtonWithName:(NSString *)aName label:(NSString*)label target:(id)aTarget
{
    [self addButtonWithName:aName label:label target:aTarget highlightPoint:NSZeroPoint];
}

- (void)addButtonWithName:(NSString *)aName label:(NSString*)label target:(id)aTarget highlightPoint:(NSPoint)p
{
    NSMutableArray *currentPage = [elementPages lastObject];
    NSMutableArray *currentColumn = [currentPage lastObject];
    
    NSRect labelRect = NSMakeRect(0, 0, 0, 0);
    NSRect buttonRect = NSMakeRect(0, 0, 0, 0);
    
    OEControlsKeyButton *button = [[OEControlsKeyButton alloc] initWithFrame:buttonRect];
    button.highlightPoint = p;
    
    [button setTarget:aTarget];
    [button setAction:@selector(selectInputControl:)];
    [button bind:@"title" toObject:aTarget withKeyPath:aName options:nil];
    [currentColumn addObject:button];
    
    NSTextField *labelField = [[NSTextField alloc] initWithFrame:labelRect];
    NSTextFieldCell *labelFieldCell = [[OEControlsKeyLabelCell alloc] init];
    [labelField setCell:labelFieldCell];
    [labelField setStringValue:label];
    [currentColumn addObject:labelField];
}

- (void)nextColumn
{
    NSMutableArray *currentPage = [elementPages lastObject];
    [currentPage addObject:[NSMutableArray array]];
}

- (void)nextPage
{
    NSMutableArray *newPage = [[NSMutableArray alloc] init];
    [newPage addObject:[NSMutableArray array]];
    [elementPages addObject:newPage];
}

- (void)addColumnLabel:(NSString*)label
{
    NSMutableArray *currentPage = [elementPages lastObject];
    NSMutableArray *currentColumn = [currentPage lastObject];
    
    NSRect labelRect = NSMakeRect(0, 0, 0, 0);
    NSTextField *labelField = [[NSTextField alloc] initWithFrame:labelRect];
    NSTextFieldCell *labelFieldCell = [[OEControlsKeyHeadlineCell alloc] init];
    [labelField setCell:labelFieldCell];
    [labelField setStringValue:label];
    [currentColumn addObject:labelField];
}

- (void)addRowSeperator
{
    NSMutableArray *currentPage = [elementPages lastObject];
    NSMutableArray *currentColumn = [currentPage lastObject];
    
    OEControlsKeySeparatorView *view = [[OEControlsKeySeparatorView alloc] init];
    [currentColumn addObject:view];
}

- (void)updateButtons
{
    if(lastWidth == [self frame].size.width)
        return;
    lastWidth = [self frame].size.width;
    
    float pageSpacing = 28.0;
    
    // determine required height
    float viewHeight = [elementPages count]*119.0+([elementPages count]-1)*pageSpacing+34.0;  
    viewHeight = viewHeight<187.0?187.0:viewHeight;
    if([self frame].size.height != viewHeight)
    {
        NSRect frame = [self frame];
        frame.size.height = viewHeight;
        [self setFrame:frame];
        
        // return here. KVO will call updateButtons again
        return;
    }
    
    // remove all subviews if any
    while([[self subviews] count])
    {
        [[[self subviews] lastObject] removeFromSuperview];
    } 
    
    // set up some sizes
    float topBorder   = 35.0;
    const float leftBorder  = 61.0;
    const float rightBorder = 21.0;
    
    const float verticalItemSpacing  = 10.0;// item bottom to top
    const float labelHeight  = 24.0;
    const float labelButtonSpacing  = 8.0;
    
    const float groupXIndent = 10.0;
    //    const float groupYIndent = -10.0;
    
    __block float pageY=[self frame].size.height-topBorder;
    // iterate through pages
    for(NSMutableArray *aPage in elementPages)
    {
        // iterate through columns
        NSUInteger columns = [aPage count];
        __block float x = leftBorder;
        [aPage enumerateObjectsUsingBlock:
         ^(id aColumn, NSUInteger idx, BOOL *stop)
         {
            float horizontalItemSpacing = columns == 2 ? 120 : 68.0;// item right to item left
            float labelWidth            = columns == 2 ? 112 : 60.0;// max value!!!
            
            float buttonHeight = 24.0;
            float buttonWidth = ([self frame].size.width-leftBorder-rightBorder-((columns-1)*horizontalItemSpacing))/columns;            
            
            
            BOOL inGroup = NO;
            float y = pageY;
            for(NSUInteger j=0; j<[aColumn count]; j+=2) {
                id item = [aColumn objectAtIndex:j];
                
                // handle headline cell
                if([item isKindOfClass:[NSTextField class]] && [[item cell] isKindOfClass:[OEControlsKeyHeadlineCell class]])
                {
                    j--;
                    inGroup = YES;
                    
                    float columnWidth = buttonWidth+labelWidth+labelButtonSpacing;
                    NSRect headlineFrame = (NSRect){{x-columnWidth,y},{columnWidth, labelHeight}};
                    [item setFrame:NSIntegralRect(headlineFrame)];
                    [self addSubview:item];
                    
                    continue;
                }
                
                // handle separator
                if([item isKindOfClass:[OEControlsKeySeparatorView class]])
                {
                    j--;
                    
                    float columnWidth = buttonWidth+labelWidth+labelButtonSpacing;
                    
                    NSRect seperatorLineRect = (NSRect){{x-labelWidth+8.0, y-buttonHeight},{columnWidth-6.0, buttonHeight}};
                    [item setFrame:NSIntegralRect(seperatorLineRect)];
                    [self addSubview:item];
                    
                    inGroup = NO;
                    y -= buttonHeight+verticalItemSpacing;
                    
                    continue;
                }
                
                // handle buttons + label
                NSRect buttonRect = (NSRect){{x, y-buttonHeight},{buttonWidth, buttonHeight}};
                if(inGroup)
                {
                    buttonRect.origin.x += groupXIndent;
                    buttonRect.size.width -= groupXIndent/2;
                }
                [item setFrame:NSIntegralRect(buttonRect)];
                
                NSTextField *label = [aColumn objectAtIndex:j+1];
                NSRect labelRect = NSIntegralRect(NSMakeRect(buttonRect.origin.x-labelWidth-labelButtonSpacing, buttonRect.origin.y-4, labelWidth, labelHeight));
                BOOL multiline = [label attributedStringValue].size.width >= labelRect.size.width;
                if(multiline)
                {
                    labelRect.size.height += 10;
                    labelRect.origin.y -= 3;
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
        pageY -= lastObj.frame.origin.y + lastObj.frame.size.height -13.0;
    }
}
#pragma mark -

- (void)scrollToPage:(NSUInteger)p
{
    NSClipView *clipView = [[self enclosingScrollView] contentView];
    float y = 0;
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

- (void)selectNextKeyButton:(id)currentButton
{
    if(!currentButton) return;
    
    __block BOOL _selectNext = NO;
    __block BOOL _nextButtonIsOnDifferentPage = NO;
    [elementPages enumerateObjectsUsingBlock:
     ^(id page, NSUInteger pidx, BOOL *stop)
     {
         [page enumerateObjectsUsingBlock:
          ^(id column, NSUInteger idx, BOOL *stop)
          {
              [column enumerateObjectsUsingBlock:
               ^(id item, NSUInteger idx, BOOL *stop)
               {
                   if(item == currentButton)
                   {
                       
                       _nextButtonIsOnDifferentPage = NO;
                       _selectNext = YES;
                   }
                   else if(_selectNext && [item isKindOfClass:[OEControlsKeyButton class]])
                   {
                       if(item && [item target] && [item action] && [[item target] respondsToSelector:[item action]])
                       {
                           if(_nextButtonIsOnDifferentPage)
                           {
                               [self scrollToPage:pidx];
                           }
                           [item setState:NSOnState];
                           [[item target] performSelector:[item action] withObject:item];
                           *stop = YES;
                           _selectNext = NO;
                       }
                   }
               }];
              _nextButtonIsOnDifferentPage = YES;
          }];
     }];
    
    if(_selectNext && [[NSUserDefaults standardUserDefaults] boolForKey:UDControlsButtonHighlightRollsOver])
    {
        NSMutableArray *firstPage = [elementPages objectAtIndex:0];
        if([firstPage count])
        {
            NSMutableArray *firstColumn = [firstPage objectAtIndex:0];
            for(id aElement in firstColumn)
            {
                if([aElement isKindOfClass:[OEControlsKeyButton class]] && [aElement target] && [aElement action] && [[aElement target] respondsToSelector:[aElement action]])
                {
                    [self scrollToPage:0];
                    
                    [aElement setState:NSOnState];
                    [[aElement target] performSelector:[aElement action] withObject:aElement];
                    
                    break;
                }
            }
        }
    }
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

#pragma mark -
#ifndef NSDistanceBetweenPoints
#define NSDistanceBetweenPoints(p1, p2) sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p2.y-p1.y)*(p2.y-p1.y))
#endif

- (id)controllerButtonClosestTo:(NSPoint)point
{
    id item = nil;
    float distance = 0;
    
    for(NSView *aSubview in [self subviews])
    {
        if([aSubview respondsToSelector:@selector(highlightPoint)])
        {
            NSPoint highlightPoint = [(OEControlsKeyButton *)aSubview highlightPoint];
            float currentDistance = NSDistanceBetweenPoints(point, highlightPoint);
            if(item == nil || currentDistance < distance)
            {
                item = aSubview;
                distance = currentDistance;
            }
            
        }
    }
    
    return item;
}
@end
