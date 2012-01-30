//
//  IKSGridView.m
//  Sonora
//
//  Created by Indragie Karunaratne on 10-11-14.
//  Copyright 2010 PCWiz Computer. All rights reserved.
//

#import "IKSGridView.h"
#import "IKSGridItemLayer.h"
#import "NSColor+IKSAdditions.h"
#import "IKSGridViewProtocols.h"

#import "OEGridViewFieldEditor.h"
#import "OEGridBlankSlateView.h"
typedef enum 
{
    IKSLayerPositionUp = 0,
    IKSLayerPositionRight = 1,
    IKSLayerPositionDown = 2,
    IKSLayerPositionLeft = 3
} IKSLayerPosition;

@interface IKSGridView ()
- (void)_setupGridLayer;
- (void)_setupFieldEditor;
- (void)_setupSelectionLayer;
- (void)_setupDragIndicationLayer;
- (void)_registerClipViewNotifications;
- (CALayer*)_layerWithEvent:(NSEvent*)event;
- (CALayer*)_layerAtPoint:(NSPoint)point;
- (void)_moveSelectionToLayerAtPosition:(IKSLayerPosition)position;
- (void)_selectLayers:(NSArray*)layers exclusively:(BOOL)exclusive;
- (void)_deselectLayers:(NSArray*)layers;
- (NSImage*)_generateDraggedImage;
@end


@implementation IKSGridView
@synthesize delegate, trackingArea, eventLayer, gridLayer;
@synthesize selectionLayer, backgroundColor, sortDescriptors;
@synthesize selectable, allowsMultipleSelection;
@synthesize minimumItemSize, maximumItemSize, layoutEnabled;
@synthesize foregroundLayer;
@synthesize backgroundLayer, draggedImage, draggedLayer;
@synthesize dragIndicationLayer, selectedIndexes;
@synthesize autoscrollTimer, lastEvent, cellClass, dataSource;
@synthesize target, doubleAction;
#pragma mark -
#pragma mark Initialization
- (IBAction)copy:(id)sender
{
    NSPasteboard* pBoard = [NSPasteboard generalPasteboard];
    [pBoard clearContents];
    
    [self.selectedLayers enumerateObjectsUsingBlock:^(id layer, NSUInteger index, BOOL *stop){
#warning reimplement
        //[pBoard writeObjects:[NSArray arrayWithObject:((IKSGridItemLayer*)layer).representedObject]];
    }];
}

+ (void)initialize
{
}

- (id)initWithFrame:(NSRect)frame 
{
    self = [super initWithFrame:frame];
    if (self) 
    {
        self.cellClass = [IKSGridItemLayer class];
        //
        // Set default values for everything and run other setup routines
        //
        self.selectable = YES;
        self.allowsMultipleSelection = YES;
        self.minimumItemSize = NSMakeSize(125.0, 125.0);
        self.maximumItemSize = NSMakeSize(256.0, 256.0);
        [self _setupGridLayer];
        [self _setupSelectionLayer];
        [self _setupDragIndicationLayer];
        [self _registerClipViewNotifications];
        [self _setupFieldEditor];
        
        [self setAcceptsTouchEvents:YES];
        [[self window] setAcceptsMouseMovedEvents:YES];
    }
    return self;
}

- (void)awakeFromNib
{
    [super awakeFromNib];
    //
    // Set default values for everything and run other setup routines
    //
    self.selectable = YES;
    self.allowsMultipleSelection = YES;
    self.minimumItemSize = NSMakeSize(125.0, 125.0);
    self.maximumItemSize = NSMakeSize(256.0, 256.0);
    [self _setupGridLayer];
    [self _setupSelectionLayer];
    [self _setupDragIndicationLayer];
    [self _registerClipViewNotifications];
    [self _setupFieldEditor];
    
    [self setAcceptsTouchEvents:YES];
    [[self window] setAcceptsMouseMovedEvents:YES];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowChanged:) name:NSWindowDidBecomeMainNotification object:[self window]];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowChanged:) name:NSWindowDidResignMainNotification object:[self window]];
}

#pragma mark -
#define OERoundNSPoint(p) (NSPoint){roundf(p.x), roundf(p.y)}
- (void)reloadData
{
    /*
     
     [self _deselectLayers:self.selectedLayers];
     for (IKSGridItemLayer *layer in self.content) {
     [layer removeFromSuperlayer];
     }
     [content release];
     if (![newContent count]) {
     // if there's no content make sure to set content height to 0
     [self layoutManager:layoutManager contentHeightChanged:0.0];
     }
     NSArray *sortedContent = [self embedInLayer:newContent];
     if (self.sortDescriptors) {
     sortedContent = [newContent sortedArrayUsingDescriptors:self.sortDescriptors];
     }
     content = [sortedContent retain];
     BOOL isFirstResponder = (([[self window] isKeyWindow]) && ([[self window] firstResponder] == self));
     for (IKSGridItemLayer* layer in content) {
     layer.selected = NO;
     layer.firstResponder = isFirstResponder;
     layer.parentGridView = self;
     }
     [self.gridLayer setSublayers:content];
     */
    [layoutManager reset];
    [self.gridLayer layoutIfNeeded];
    noItems = [[self dataSource] numberOfItemsInGridView:self]==0;
    
    if(![self dataSource]) return;
    
    NSArray* subviews = [NSArray arrayWithArray:[self subviews]];
    [subviews enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        if(![obj isKindOfClass:[OEGridViewFieldEditor class]])
        {
            [obj removeFromSuperview];
        }
    }];
    
    if(noItems)
    {
        if([self dataSource] && [[self dataSource] respondsToSelector:@selector(gridViewNoItemsView:)])
        {
            NSView* view = [[self dataSource] gridViewNoItemsView:self];
            if(!view) return;
            [view setAutoresizingMask:NSViewMinXMargin|NSViewMaxXMargin | NSViewMinYMargin|NSViewMaxYMargin];
            
            self.wantsLayer = NO;
            
            NSPoint center = NSMakePoint((self.frame.size.width-view.frame.size.width)/2, (self.frame.size.height-view.frame.size.height)/2);
            center = OERoundNSPoint(center);
            [view setFrameOrigin:center];
            [self addSubview:view];
        }
        self.selectionLayer.hidden = YES;
    }
    else
    {
        self.wantsLayer = YES;
    }
    
    [self setNeedsDisplay:YES];    
}
#pragma mark -
#pragma mark Decoration Views
- (void)layoutDecorationViews
{
    CGRect frame = layoutManager.visibleRect;
    frame.origin.y = self.frame.size.height - layoutManager.visibleRect.origin.y - layoutManager.visibleRect.size.height;
    frame.size.width--;
    frame.size.height--;
    if(self.backgroundLayer)
    {
        self.backgroundLayer.frame = frame;
    }
    
    if(self.foregroundLayer)
    {
        self.foregroundLayer.frame = frame;
    }
    self.dragIndicationLayer.frame = CGRectInset(frame, 1.0f, 1.0f);
    
    [[self subviews] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        if([obj isKindOfClass:[OEGridBlankSlateView class]])
        {
            [obj centerInSuperview];
        }
    }];
}
- (void)addBackgroundLayer:(CALayer*)newBackgroundLayer
{
    if(self.backgroundLayer)
    {
        [self.backgroundLayer removeFromSuperlayer];
        self.backgroundLayer = nil;
    }
    
    self.backgroundLayer = newBackgroundLayer;
    self.backgroundLayer.delegate = self;
    self.backgroundLayer.needsDisplayOnBoundsChange = YES;
    self.backgroundLayer.delegate = self;
    
    CALayer *rootLayer = [self layer];
    [rootLayer insertSublayer:self.backgroundLayer atIndex:0];
    
    [self layoutDecorationViews];
}

- (void)addForegroundLayer:(CALayer*)newForegroundLayer
{
    if(self.foregroundLayer)
    {
        [self.foregroundLayer removeFromSuperlayer];
        self.foregroundLayer = nil;
    }
    
    self.foregroundLayer = newForegroundLayer;
    self.foregroundLayer.delegate = self;
    self.foregroundLayer.needsDisplayOnBoundsChange = YES;
    self.foregroundLayer.delegate = self;
    
    CALayer *rootLayer = [self layer];
    [rootLayer insertSublayer:self.foregroundLayer atIndex:[[rootLayer sublayers] count]-2];
    
    [self layoutDecorationViews];
}


#pragma mark -
#pragma mark NSView Overrides

//
// Fill the background if a background color has been set
// This step isn't really necessary because the grid layer should handle background, but just in case..
//
- (void)drawRect:(NSRect)dirtyRect
{
    if (self.backgroundColor) 
    {
        [self.backgroundColor set];
        NSRectFill([self bounds]);
    }
}

- (void) viewWillMoveToWindow:(NSWindow *)newWindow 
{
    // Setup a new tracking area when the view is added to the window.
    NSTrackingAreaOptions options = (NSTrackingMouseMoved | NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow);
    self.trackingArea = [[[NSTrackingArea alloc] initWithRect:[self bounds] options:options owner:self userInfo:nil] autorelease];
    [self addTrackingArea:self.trackingArea];
}

- (void)updateTrackingAreas
{
    [super updateTrackingAreas];
    if (self.trackingArea)
    { 
        [self removeTrackingArea:self.trackingArea];
    }
    NSTrackingAreaOptions options = (NSTrackingMouseMoved | NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow);
    self.trackingArea = [[[NSTrackingArea alloc] initWithRect:[self bounds] options:options owner:self userInfo:nil] autorelease];
    [self addTrackingArea:self.trackingArea];
}

//
// Flipped coordinate system (origin on top left)
//
- (BOOL)isFlipped
{
    return YES;
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

#pragma mark -
#pragma mark NSResponder

//
// Accepts keyboard and mouse events
//
- (BOOL)acceptsFirstResponder
{
    return YES;
}


- (BOOL)becomeFirstResponder
{
    // When the view becomes first responder, set the first responder
    // property of all the layers to YES, because some layers may want
    // to implement drawing changes based on the first responder status of the view
    
    for (IKSGridItemLayer *layer in self.gridLayer.sublayers) 
    {
        layer.firstResponder = YES;
    }
    return YES;
}

- (BOOL)resignFirstResponder
{
    // Same as the becomeFirstResponder method, edxcept set the property to no
    for (IKSGridItemLayer *layer in self.gridLayer.sublayers) 
    {
        layer.firstResponder = NO;
    }
    return YES;
}

//
// This is an NSResponder method that is called when the user presses Command+A
// In this case, we want to select all the layers
//
- (void)selectAll:(id)sender
{
    if(!self.dataSource) return;
    NSUInteger itemCount = [self.dataSource numberOfItemsInGridView:self];
    self.selectedIndexes = [NSMutableIndexSet indexSetWithIndexesInRange:NSMakeRange(0, itemCount)];
    
    [self.gridLayer setNeedsLayout];
}

//
// Interpret key events to move selection up/down/left/right
//
- (void)keyDown:(NSEvent *)theEvent
{
    [self interpretKeyEvents:[NSArray arrayWithObject:theEvent]];
}

//
// NSResponder methods that respond to arrow key presses
// Invoke the method with the appropriate parameter to select the right layer
//
- (void)moveUp:(id)sender
{
    [self _moveSelectionToLayerAtPosition:IKSLayerPositionUp];
}

- (void)moveDown:(id)sender
{
    [self _moveSelectionToLayerAtPosition:IKSLayerPositionDown];
}

- (void)moveLeft:(id)sender
{
    [self _moveSelectionToLayerAtPosition:IKSLayerPositionLeft];
}

- (void)moveRight:(id)sender
{
    [self _moveSelectionToLayerAtPosition:IKSLayerPositionRight];
}

//
// Adds pinch to zoom capabilities to the grid using Snow Leopard's new multitouch APIs
//
- (void)magnifyWithEvent:(NSEvent *)event
{
    CGFloat magnification = [event magnification] + 1.0;
    NSSize newSize = self.itemSize;
    newSize.width = round(newSize.width * magnification);
    newSize.height = round(newSize.height * magnification);
    if (newSize.width < self.minimumItemSize.width || newSize.width > self.maximumItemSize.width 
        || newSize.height < self.minimumItemSize.height 
        || newSize.height > self.maximumItemSize.height) 
    { 
        return;
    }
    [self.delegate gridView:self itemsMagnifiedToSize:newSize];
    self.itemSize = newSize;
}

#pragma mark -
#pragma mark Clip View Notifications
//
// Notification when the clip view frame or bounds have changed
// This sets the layout manager's visible rect when the user scrolls
//
- (void)clipViewFrameChanged:(NSNotification*)notification
{
    CGRect rect;
    if ([self enclosingScrollView]) 
    {
        rect = NSRectToCGRect([[self enclosingScrollView] documentVisibleRect]);
    }
    else 
    {
        rect = NSRectToCGRect([self bounds]);
    }
    layoutManager.visibleRect = rect;
    if (![self inLiveResize]) 
    {
        [self.gridLayer setNeedsLayout];
    }
    
    [CATransaction begin];
    [CATransaction setValue:(id)kCFBooleanTrue
                     forKey:kCATransactionDisableActions];
    [self layoutDecorationViews];
    [CATransaction commit];
}

#pragma mark -
#pragma mark Memory Management

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [trackingArea release];
    [eventLayer release];
    [gridLayer release];
    [selectionLayer release];
    
    [selectedIndexes release];
    
    [sortDescriptors release];
    [layoutManager release];
    [backgroundColor release];
    [super dealloc];
}

#pragma mark -
#pragma mark Accessors

- (NSColor*)backgroundColor
{
    return backgroundColor;
}

//
// Sets the background color of the root layer, grid layer, and view
// Uses the NSColor+IKSAdditions category to convert colors to CGColorRefs
//
- (void)setBackgroundColor:(NSColor *)newColor
{
    if (backgroundColor != newColor) 
    {
        [backgroundColor release];
        backgroundColor = [newColor retain];
        CGColorRef aColor = backgroundColor.CGColor;
        self.layer.backgroundColor = aColor;
        self.gridLayer.backgroundColor = newColor.CGColor;
        [self setNeedsDisplay:YES];
    }
}

- (NSSize)minimumSpacing
{
    return layoutManager.minimumSpacing;
}

//
// Redo layout when minimum spacing has changed
//
- (void)setMinimumSpacing:(NSSize)newSpacing
{
    layoutManager.minimumSpacing = newSpacing;
    [self.gridLayer setNeedsLayout];
}

- (NSSize)itemSize
{
    return layoutManager.sublayerSize;
}

//
// Redo layout when sublayer size changes
//
- (void)setItemSize:(NSSize)newSize
{
    [fieldEditor setString:@""];
    [fieldEditor setHidden:YES];
    
    layoutManager.sublayerSize = newSize;
    [self.gridLayer setNeedsLayout];
}


- (NSArray*)embedInLayer:(NSArray*)items
{
    @throw @"embed in layer calles!!";
    
    
    NSMutableArray* layers = [[items mutableCopy] autorelease];
    
    NSUInteger i;
    for(i=0; i<[items count]; i++)
    {
        id anObj = [items objectAtIndex:i];
        if([[anObj className] isNotEqualTo:[self.cellClass className]])
        {
            IKSGridItemLayer* layer = [self.cellClass layer];
            //layer.representedObject = anObj;
            [layers replaceObjectAtIndex:i withObject:layer];
        }
    }
    
    return layers;
}


//
// Sort and set content
//
- (void)setSortDescriptors:(NSArray *)newDescriptors
{
    if (sortDescriptors != newDescriptors) 
    {
        /*
         [sortDescriptors release];
         sortDescriptors = [newDescriptors retain];
         NSArray *sortedContent = [self.content sortedArrayUsingDescriptors:sortDescriptors];
         // Manually set content property
         [content release];
         content = [sortedContent retain];
         [self.gridLayer setSublayers:content];
         */
    }
}

- (BOOL)layoutEnabled
{
    return (self.gridLayer.layoutManager != nil);
}

- (void)setLayoutEnabled:(BOOL)newLayoutEnabled
{
    self.gridLayer.layoutManager = (newLayoutEnabled) ? layoutManager : nil;
}

- (NSArray*)selectedLayers
{
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"selected == YES"];
    return [self.gridLayer.sublayers filteredArrayUsingPredicate:predicate];
}
#pragma mark -
#pragma mark Layout Manager Delegate

//
// If there is a scroll view, adjust the view frame when the content height changes
//
- (void)layoutManager:(IKSGridLayoutManager*)manager contentHeightChanged:(CGFloat)height;
{
    if (![self enclosingScrollView] || height==self.frame.size.height) 
    {
        return;
    }
    
    CGFloat newHeight = [[self enclosingScrollView] contentSize].height;
    if (height > newHeight) 
    {
        newHeight = height;
    }
    
    NSRect newFrame = [self frame];
    newFrame.size.height = newHeight;
    BOOL oldLayoutEnabled = self.layoutEnabled;
    self.layoutEnabled = NO; // Disable the layout manager 
    [self setFrame:newFrame];
    self.layoutEnabled = oldLayoutEnabled; // Reset the layout manager to its previous state after the frame has been set
}

- (id < CAAction >)actionForLayer:(CALayer *)layer forKey:(NSString *)key
{
    return (id < CAAction >)[NSNull null];
}
#pragma mark -
#pragma mark Mouse Events

//
// If the view allows multiple selection, then the mouse down point is set and mouseDragged called
// If not, it just grabs the currently clicked layer and selects it
// Checks whether the command key is being pressed, and if it is, it will select the layer without deselecting the others
//
- (void)mouseDown:(NSEvent *)theEvent
{    
    self.eventLayer = nil;
    self.draggedLayer = nil;
    self.draggedImage = nil;
        
    NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    CALayer* clickedLayer = [self _layerAtPoint:location];
    
    if([clickedLayer isKindOfClass:[IKSGridItemLayer class]] &&
       [(id <IKSGridItemLayerEventProtocol>)clickedLayer mouseDown:theEvent])
    {
        self.eventLayer = (IKSGridItemLayer*)clickedLayer;
        return;
    }
    
    mouseDownPoint = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    if([clickedLayer isKindOfClass:[IKSGridItemLayer class]] &&
       NSPointInRect(mouseDownPoint, [(IKSGridItemLayer*)clickedLayer hitRect]))
    {
        if(![(IKSGridItemLayer*)clickedLayer isSelected])
            [self _selectLayers:[NSArray arrayWithObject:clickedLayer] exclusively:YES];
        
        self.draggedLayer = (IKSGridItemLayer*)clickedLayer;
        self.draggedImage = [self _generateDraggedImage];
        
        if([theEvent clickCount]>=2 && [self target] && [self doubleAction]!=NULL && [[self target] respondsToSelector:[self doubleAction]])
            [[self target] performSelector:[self doubleAction] withObject:self];
        
        return;
    }
    
    [self _deselectLayers:[self selectedLayers]];
    [self mouseDragged:theEvent];
}
// 
// Autoscroll if the mouse is beyond view bounds
// If multiple selection is allowed, this will create a selection rectangle 
// Mouse down point and current mouse point is used to construct the rectangle
// Iterates through all the layers in the content array to see if they fall within the selection rectangle. 
//

- (void)timerFireMethod:(NSTimer*)theTimer
{
    if(!self.lastEvent) return;
    
    [self mouseDragged:self.lastEvent];
}
- (void)mouseDragged:(NSEvent *)theEvent
{
    if(self.draggedImage)
    {
        float offsetX = mouseDownPoint.x-(mouseDownPoint.x-[(IKSGridItemLayer*)self.draggedLayer hitRect].origin.x);
        float offsetY = mouseDownPoint.y-(mouseDownPoint.y-[(IKSGridItemLayer*)self.draggedLayer hitRect].origin.y)+[(IKSGridItemLayer*)self.draggedLayer hitRect].size.height;
        
        NSPasteboard* pBoard = [NSPasteboard pasteboardWithName:NSDragPboard];
        [pBoard clearContents];
        [self.selectedLayers enumerateObjectsUsingBlock:^(id layer, NSUInteger index, BOOL *stop){
            id obj = [[self dataSource] gridView:self objectValueForKey:nil withRepresentedObject:((IKSGridItemLayer*)layer).representedObject];           
            [pBoard writeObjects:[NSArray arrayWithObject:obj]];
        }];
        
        [self dragImage:self.draggedImage at:NSMakePoint(offsetX, offsetY) offset:NSZeroSize event:theEvent pasteboard:pBoard source:self slideBack:NO];
        self.draggedImage = nil;
    }
    
    if(self.eventLayer)
    {
        [self.eventLayer mouseDragged:theEvent];
        return;
    }
    
    if(self.draggedLayer)
    {
        return;
    }
    
    if(self.autoscrollTimer)
    { 
        [self.autoscrollTimer invalidate]; self.autoscrollTimer=nil; 
    }
    if([self autoscroll:theEvent])
    {
        self.lastEvent = theEvent;
        self.autoscrollTimer = [[[NSTimer alloc] initWithFireDate:[NSDate dateWithTimeIntervalSinceNow:0.05] interval:0 target:self selector:@selector(timerFireMethod:) userInfo:nil repeats:NO] autorelease];
        NSRunLoop* loop = [NSRunLoop currentRunLoop];
        [loop addTimer:self.autoscrollTimer forMode:NSDefaultRunLoopMode];
    }
    // If allows multiple selection isn't enabled, the method won't proceed
    if (!self.allowsMultipleSelection || NSEqualPoints(NSZeroPoint, mouseDownPoint))
    { 
        return; 
    }
    // Calculate a rectangle for the selection indicator
    
    NSPoint startPoint = mouseDownPoint;
    NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    
    // either visibleHeight or frame height....
    startPoint.y = self.frame.size.height-startPoint.y;
    location.y = self.frame.size.height-location.y;
    
    NSPoint distanceFromStart = NSMakePoint(location.x - startPoint.x, location.y-startPoint.y);
    
    CGRect selectionRect;
    if (location.x < startPoint.x)
    {
        selectionRect.origin.x = startPoint.x + distanceFromStart.x;
        selectionRect.size.width = fabs(distanceFromStart.x);
    } 
    else
    {
        selectionRect.origin.x = startPoint.x;
        selectionRect.size.width = distanceFromStart.x;
    }
    
    if (location.y < startPoint.y)
    {
        selectionRect.origin.y = startPoint.y + distanceFromStart.y;
        selectionRect.size.height = fabs(distanceFromStart.y);
    }
    else
    {
        selectionRect.origin.y = startPoint.y;
        selectionRect.size.height = distanceFromStart.y;
    }
    if(!noItems)
    {
        self.selectionLayer.hidden = NO;
        self.selectionLayer.frame = selectionRect;
    }
    
    // Check if any of the layers fall within the selection bounds
    CALayer *clickedLayer = [self _layerWithEvent:theEvent];
    
    NSArray *layers = self.gridLayer.sublayers;
    NSMutableArray *layersToSelect = [NSMutableArray array];
    for (IKSGridItemLayer *layer in layers) 
    {
        NSRect frameToCheck = [layer isKindOfClass:[IKSGridItemLayer class]] ? layer.hitRect : layer.frame;
        frameToCheck.origin.y = self.frame.size.height-frameToCheck.origin.y-frameToCheck.size.height; //layoutManager.visibleRect.origin.y - layoutManager.visibleRect.size.height;
        
        
        if (CGRectIntersectsRect(frameToCheck, selectionRect))
        {
            [layersToSelect addObject:layer];
        }
    }
    // Check which keys are being held (command, shift)
    NSUInteger modifierFlags = [[NSApp currentEvent] modifierFlags];
    BOOL shiftKeyDown = ((modifierFlags & NSShiftKeyMask) == NSShiftKeyMask);
    BOOL commandKeyDown = (((modifierFlags & NSCommandKeyMask) == NSCommandKeyMask) || shiftKeyDown);
    // Select layers exclusively only if the command key isn't being pressed and the layer doesn't already exist in the selected layers array
    BOOL exclusive = (!commandKeyDown && !(([clickedLayer isKindOfClass:[IKSGridItemLayer class]]) && ([self.selectedLayers containsObject:clickedLayer]))); 
    [self _selectLayers:layersToSelect exclusively:exclusive];
    
    // If the shift key is being held, we want to select all the l;ayers in that range
    NSArray *currentSelectedLayers = self.selectedLayers;
    if (shiftKeyDown && [currentSelectedLayers count])
    { 
        layersToSelect = [NSMutableArray array];
        // Figure out the first selected layer
        NSUInteger firstIndex = [self.gridLayer.sublayers indexOfObject:[currentSelectedLayers objectAtIndex:0]];
        // Get the layer at the clicked point and if it's the background layer then do nothng
        CALayer *clickedLayer = [self _layerAtPoint:location];
        if (![clickedLayer isKindOfClass:[IKSGridItemLayer class]]) 
        { 
            return; 
        }
        
        if(!NSPointInRect(location, [(IKSGridItemLayer*)clickedLayer hitRect])) return;
        
        // Find the index of the clicked layer in the content array
        NSUInteger clickedIndex = [self.gridLayer.sublayers indexOfObject:clickedLayer];
        // Calculate the range of indexes to select
        NSRange indexRange = (firstIndex < clickedIndex) ? NSMakeRange(firstIndex, (clickedIndex + 1) - firstIndex) : NSMakeRange(clickedIndex, firstIndex - (clickedIndex - 1));
        NSIndexSet *indexes = [NSIndexSet indexSetWithIndexesInRange:indexRange];
        [layersToSelect addObjectsFromArray:[self.gridLayer.sublayers objectsAtIndexes:indexes]];
        [self _selectLayers:layersToSelect exclusively:YES];
    }
}


//
// Clears mouse down point and removes selection rect layer
//
- (void)mouseUp:(NSEvent *)theEvent
{
    if(self.draggedLayer)
    {
        self.draggedLayer = nil;
        
        return;
    }
    
    if(self.eventLayer)
    {
        [self.eventLayer mouseUp:theEvent];
        self.eventLayer = nil;
        
        return;
    }
    
    self.lastEvent = nil;
    if(self.autoscrollTimer)
    { 
        [self.autoscrollTimer invalidate]; 
        self.autoscrollTimer = nil;
    }
    
    mouseDownPoint = NSZeroPoint;
    self.selectionLayer.frame = CGRectZero;
}

- (void)mouseMoved:(NSEvent *)theEvent
{
}

- (void)mouseExited:(NSEvent *)theEvent
{
    if(self.eventLayer)
    {
        [self.eventLayer mouseExited:theEvent];
    }
}
#pragma mark -
#pragma mark Drop
- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender
{
    self.draggedLayer = nil;
    
    NSPoint location = [self convertPoint:[sender draggingLocation] fromView:nil];
    CALayer <IKSGridItemLayerDragProtocol> * dragLayer = (CALayer <IKSGridItemLayerDragProtocol> *) [self _layerAtPoint:location];
    NSDragOperation layerOp = NSDragOperationNone;
    
    if([dragLayer isKindOfClass:[IKSGridItemLayer class]] &&
       NSPointInRect(location, [(IKSGridItemLayer*)dragLayer hitRect]))
    {
        self.draggedLayer = (IKSGridItemLayer*)dragLayer;
        
        NSDragOperation layerOp = [dragLayer draggingEntered:sender];
        if(layerOp!=NSDragOperationNone)
        {
            self.dragIndicationLayer.hidden = YES;
            return layerOp;
        }
    }

    if([delegate respondsToSelector:@selector(gridView:validateDrop:)])
        layerOp = [delegate gridView:self validateDrop:sender];

    self.dragIndicationLayer.hidden = (layerOp == NSDragOperationNone);
    return layerOp;
}

- (void)draggingExited:(id<NSDraggingInfo>)sender
{
    self.dragIndicationLayer.hidden = YES;
    
    if(self.draggedLayer)
    {
        [(CALayer <IKSGridItemLayerDragProtocol> *)self.draggedLayer draggingExited:sender];
        self.draggedLayer = nil;
        return;
    }
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender
{
    
    NSPoint location = [self convertPoint:[sender draggingLocation] fromView:nil];
    NSDragOperation layerOp = NSDragOperationNone;

    if(self.draggedLayer)
    {
        if(NSPointInRect(location, [self.draggedLayer frame]))
        {
            layerOp = [(CALayer <IKSGridItemLayerDragProtocol> *)self.draggedLayer draggingUpdated:sender];
            if(layerOp!=NSDragOperationNone)
            {
                self.dragIndicationLayer.hidden = YES;
                return layerOp;
            }
        }
        [(CALayer <IKSGridItemLayerDragProtocol> *)self.draggedLayer draggingExited:sender];
    }
    
    CALayer* dragLayer = [self _layerAtPoint:location];
    if(self.draggedLayer!=dragLayer && [dragLayer isKindOfClass:[IKSGridItemLayer class]] &&
       NSPointInRect(location, [(IKSGridItemLayer*)dragLayer hitRect]))
    {
        self.draggedLayer = (IKSGridItemLayer*)dragLayer;
        
        NSDragOperation layerOp = [(CALayer <IKSGridItemLayerDragProtocol> *)dragLayer draggingEntered:sender];
        if(layerOp!=NSDragOperationNone){
            self.dragIndicationLayer.hidden = YES;
            return layerOp;
        }
    }
    self.draggedLayer = nil;
    
    
    // determine if grid view can handle drop
    if([delegate respondsToSelector:@selector(gridView:validateDrop:)]) {
        layerOp = [delegate gridView:self validateDrop:sender];
        if(layerOp != NSDragOperationNone && [delegate respondsToSelector:@selector(gridView:updateDraggingItemsForDrag:)])
            [delegate gridView:self updateDraggingItemsForDrag:sender];
    }
    
    // indicate whatever we decided
    self.dragIndicationLayer.hidden = (layerOp == NSDragOperationNone);
    return layerOp;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    self.dragIndicationLayer.hidden = YES;

    if(self.draggedLayer)
        return [(CALayer <IKSGridItemLayerDragProtocol> *)self.draggedLayer performDragOperation:sender];
    else if([delegate respondsToSelector:@selector(gridView:acceptDrop:)])
        return [delegate gridView:self acceptDrop:sender];
    return NO;
}
#pragma mark -
#pragma mark Selection

//
// Selects or deselects layers
// If an array controller is bound to the content property, its selectedObjects will automatically be set
// The delegate is also called to be informed of the changes
// 

- (void)_selectLayers:(NSArray*)layers exclusively:(BOOL)exclusive
{
    if (!layers) 
    {
        return; 
    }
    if (exclusive) 
    {
        [self _deselectLayers:self.selectedLayers];
    }
    
    for(IKSGridItemLayer* aLayer in layers)
    {
        [selectedIndexes addIndex:aLayer.representedIndex];
    }
    
    
    [self.gridLayer setNeedsLayout];
    [self.delegate gridView:self selectionChanged:self.selectedLayers];
}

- (void)_deselectLayers:(NSArray*)layers
{
    if (!layers) 
    { 
        return; 
    }
    
    self.selectedIndexes = [NSMutableIndexSet indexSet];
    [self.gridLayer setNeedsLayout];
    [self.delegate gridView:self selectionChanged:self.selectedLayers];
}
- (BOOL)isItemSelectedAtIndex:(NSInteger)index
{
    if(index == -1) 
        return NO;
    
    return [self.selectedIndexes containsIndex:index];
}
#pragma mark -
- (void)windowChanged:(id)sender
{
    [self.gridLayer setNeedsLayout];
}

#pragma mark -
- (void)setObjectValue:(id)obj forKey:(NSString*)key atIndex:(NSUInteger)index fromLayer:(CALayer*)layer
{
    [self.dataSource gridView:self setObject:obj forKey:key atIndex:index];
    
    [layer needsLayout];
    [layer needsDisplay];
}
#pragma mark -
#pragma mark Private

- (void)_setupAutoresizing
{
    [self setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
}

- (void)_setupFieldEditor
{
    if(fieldEditor)
    {
        [fieldEditor removeFromSuperview];
        
        [fieldEditor release];
        fieldEditor = nil;
    }
    
    fieldEditor = [[OEGridViewFieldEditor alloc] initWithFrame:NSMakeRect(50, 50, 50, 50)];
    
    [self addSubview:fieldEditor];
}

- (void)_setupGridLayer
{
    if (self.gridLayer) 
    {
        [self.gridLayer removeFromSuperlayer];
        self.gridLayer = nil;
    }
    // Make the view layer backed
    if (![self wantsLayer]) 
    {
        self.layer = [CALayer layer];
        
        // Flip the coordinate system of the layer
        
        [self setWantsLayer:YES];
    }
    // Create new grid layer
    CALayer *grid = [CALayer layer];
    CALayer *rootLayer = [self layer];
    grid.frame = rootLayer.bounds;
    if (!layoutManager) 
    {
        layoutManager = [[IKSGridLayoutManager alloc] init];
        layoutManager.delegate = self;
        layoutManager.gridView = self;
    }
    
    grid.delegate = self;
    grid.sublayerTransform = CATransform3DMakeScale(1.0f, -1.0f, 1.0f);
    
    rootLayer.delegate = self;
    
    grid.layoutManager = layoutManager;
    grid.autoresizingMask = (kCALayerWidthSizable | kCALayerHeightSizable);
    grid.masksToBounds = YES;
    [rootLayer addSublayer:grid];
    self.gridLayer = grid;
}

- (void)_setupSelectionLayer
{
    if (self.selectionLayer) 
    {
        [self.selectionLayer removeFromSuperlayer];
        self.selectionLayer = nil;
    }
    // Make the view layer backed
    if (![self wantsLayer]) 
    {
        self.layer = [CALayer layer];
        // Flip the coordinate system of the layer
        //self.layer.sublayerTransform = CATransform3DMakeScale(1.0f, -1.0f, 1.0f);
        [self setWantsLayer:YES];
    }
    CALayer *selection = [CALayer layer];
    selection.backgroundColor = [[NSColor colorWithCalibratedWhite:1.0 alpha:0.3] CGColor];
    selection.borderWidth = 1.0;
    selection.borderColor = [[NSColor whiteColor] CGColor];
    
    // Disable implicit animations
    selection.actions = [NSDictionary dictionaryWithObjectsAndKeys:[NSNull null], @"frame", [NSNull null], @"bounds", [NSNull null], @"position", [NSNull null], @"sublayers", [NSNull null], @"onOrderOut", [NSNull null], @"onOrderIn", [NSNull null], @"contents", [NSNull null], @"hidden", nil];
    
    [self.layer addSublayer:selection];
    self.selectionLayer = selection;
    
    
    self.selectedIndexes = [NSMutableIndexSet indexSet];
}
- (void)_setupDragIndicationLayer
{
    CALayer* diLayer = [CALayer layer];
    diLayer.borderColor = [[NSColor colorWithDeviceRed:0.03f green:0.41f blue:0.85f alpha:1.0f] CGColor];
    diLayer.borderWidth = 2.0f;
    diLayer.cornerRadius = 8.0f;
    diLayer.hidden = YES;
    diLayer.delegate = self;

    [self.layer addSublayer:diLayer];
    self.dragIndicationLayer = diLayer;
}
//
// Register for frame/bounds change notifications from the clip view
//
- (void)_registerClipViewNotifications
{
    if (![self enclosingScrollView]) 
    {
        return;
    }
    NSClipView *clipView = [[self enclosingScrollView] contentView];
    [clipView setPostsBoundsChangedNotifications:YES];
    [clipView setPostsFrameChangedNotifications:YES];
    
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc addObserver:self selector:@selector(clipViewFrameChanged:) name:NSViewBoundsDidChangeNotification object:clipView];
    [nc addObserver:self selector:@selector(clipViewFrameChanged:) name:NSViewFrameDidChangeNotification object:clipView];
    [self clipViewFrameChanged:nil];
}

//
// Returns the layer by grabbing the mouse position from the given event
//
- (CALayer*)_layerWithEvent:(NSEvent*)event
{
    NSPoint location = [self convertPoint:[event locationInWindow] fromView:nil];
    location.y = [self bounds].size.height - location.y;
    
    CALayer* layer = [self.gridLayer hitTest:CGPointMake(location.x, location.y)];
    layer = layer && [layer isKindOfClass:[IKSGridItemLayer class]] && NSPointInRect(location, [(IKSGridItemLayer*)layer hitRect]) ? layer : nil;
    
    if(layer.superlayer && ![layer isKindOfClass:[IKSGridItemLayer class]]) layer = layer.superlayer;
    if(layer.superlayer && [layer.superlayer isKindOfClass:[IKSGridItemLayer class]]) layer = layer.superlayer;
    
    return layer;
}

//
// Returns the layer corresponding to the given point
//
- (CALayer*)_layerAtPoint:(NSPoint)point
{
    point.y = [self bounds].size.height - point.y;
    
    CALayer* layer = [self.gridLayer hitTest:point];
    
    if(layer.superlayer && ![layer isKindOfClass:[IKSGridItemLayer class]]) layer = layer.superlayer;
    if(layer.superlayer && [layer.superlayer isKindOfClass:[IKSGridItemLayer class]]) layer = layer.superlayer;
    
    return layer;
}

//
// Used for keyboard events to calculate where the layer at the given position would be
// Also scrolls to make sure that the layer is visible
//
- (void)_moveSelectionToLayerAtPosition:(IKSLayerPosition)position
{
    if (![self.selectedLayers count]) 
    {
        return; 
    }
    CALayer *firstLayer = [self.selectedLayers objectAtIndex:0];
    NSPoint nextLayerPoint = NSPointFromCGPoint(firstLayer.frame.origin);
    if (position == IKSLayerPositionUp) 
    {
        nextLayerPoint.x = CGRectGetMidX(firstLayer.frame);
        nextLayerPoint.y = firstLayer.frame.origin.y - firstLayer.frame.size.height;
    }
    else if (position == IKSLayerPositionDown)
    {
        nextLayerPoint.x = CGRectGetMidX(firstLayer.frame);
        nextLayerPoint.y = firstLayer.frame.origin.y + (firstLayer.frame.size.height * 2.0);
    } 
    else if (position == IKSLayerPositionLeft) 
    {
        nextLayerPoint.x = firstLayer.frame.origin.x - firstLayer.frame.size.width;
        nextLayerPoint.y = CGRectGetMidY(firstLayer.frame);
    } 
    else if (position == IKSLayerPositionRight)
    {
        nextLayerPoint.x = firstLayer.frame.origin.x + (firstLayer.frame.size.width * 2.0);
        nextLayerPoint.y = CGRectGetMidY(firstLayer.frame);
    }
    else {
        return;
    }
    
    CALayer *nextLayer = [self _layerAtPoint:nextLayerPoint];
    if (nextLayer && [nextLayer isKindOfClass:[IKSGridItemLayer class]]) 
    {
        [self _selectLayers:[NSArray arrayWithObject:nextLayer] exclusively:YES];
        if ([self enclosingScrollView]) 
        {
            NSScrollView *scrollView = [self enclosingScrollView];
            NSRect visibleRect = [scrollView documentVisibleRect];
            if (!NSPointInRect(nextLayerPoint, visibleRect)) 
            {
                NSClipView *contentView = [scrollView contentView];
                NSPoint scrollPoint = NSPointFromCGPoint(nextLayer.frame.origin);
                [contentView scrollToPoint:[contentView constrainScrollPoint:scrollPoint]];
                [scrollView reflectScrolledClipView:contentView];
            }
        }
    }
}


- (NSImage*)_generateDraggedImage
{
    NSImage* proposedImage = [(IKSGridItemLayer*)self.draggedLayer dragImage];
    
    
    NSImage* dragImage = [[NSImage alloc] initWithSize:proposedImage.size];
    
    [dragImage lockFocus];
    [proposedImage drawAtPoint:NSZeroPoint fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:0.5];
    
    if([self.selectedLayers count]>1)
    {
        NSString* countStr = [NSString stringWithFormat:@"%d", [self.selectedIndexes count]];
        
        NSString* badgeImageName = [NSString stringWithFormat:@"badge_%d", [countStr length]<4?[countStr length]:3];
        NSImage* badgeImage = [NSImage imageNamed:badgeImageName];
        
        NSPoint badgeLoc = NSMakePoint(proposedImage.size.width-5-badgeImage.size.width, 5);
        [badgeImage drawAtPoint:badgeLoc fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
        
        
        NSFont* font = [NSFont systemFontOfSize:13.0];
        NSDictionary* attr = [NSDictionary dictionaryWithObjectsAndKeys:
                              font, NSFontAttributeName,
                              [NSColor whiteColor], NSForegroundColorAttributeName,
                              nil];
        
        NSAttributedString* attributedCountString = [[NSAttributedString alloc] initWithString:countStr attributes:attr];
        
        badgeLoc.x += (badgeImage.size.width-[attributedCountString size].width)/2;
        badgeLoc.y += (badgeImage.size.height-[attributedCountString size].height)/2;
        badgeLoc.y += 1;
        
        [attributedCountString drawAtPoint:badgeLoc];
        [attributedCountString release];
    }
    
    [dragImage unlockFocus];
    
    return [dragImage autorelease];
}
#pragma mark -
- (OEGridViewFieldEditor*)fieldEditorForFrame:(NSRect)frame ofLayer:(CALayer*)layer
{
    frame.origin.x += layer.frame.origin.x;
    frame.origin.y += layer.frame.origin.y;
    
    [fieldEditor setFrame:frame];
    
    return fieldEditor;
}
@end
