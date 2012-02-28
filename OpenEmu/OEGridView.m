/*
 Copyright (c) 2012, OpenEmu Team

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

#import "OEGridView.h"
#import "OEGridViewCell+OEGridView.h"
#import "NSColor+OEAdditions.h"
#import "OEMenu.h"
const NSTimeInterval OEInitialPeriodicDelay = 0.4;      // Initial delay of a periodic events
const NSTimeInterval OEPeriodicInterval     = 0.075;    // Subsequent interval of periodic events

@interface OEGridView ()

- (void)OE_commonGridViewInit;

- (void)OE_updateSelectedCellsActiveSelectorWithFocus:(BOOL)focus;
- (void)OE_windowChangedKey:(NSNotification *)notification;
- (void)OE_clipViewFrameChanged:(NSNotification *)notification;

- (void)OE_moveKeyboardSelectionToIndex:(NSUInteger)index;

- (void)OE_setNeedsLayoutGridView;
- (void)OE_layoutGridViewIfNeeded;
- (void)OE_layoutGridView;

- (void)OE_enqueueCell:(OEGridViewCell *)cell;
- (void)OE_enqueueCells:(NSSet *)cells;
- (void)OE_enqueueCellsAtIndexes:(NSIndexSet *)indexes;
- (void)OE_calculateCachedValuesAndQueryForDataChanges:(BOOL)queryForDataChanges;
- (void)OE_checkForDataReload;

- (void)OE_setNeedsReloadData;
- (void)OE_reloadDataIfNeeded;

- (void)OE_centerNoItemsView;
- (void)OE_reorderSublayers;
- (void)OE_updateDecorativeLayers;

- (NSPoint)OE_pointInViewFromEvent:(NSEvent *)theEvent;
- (NSPoint)OE_convertPointToRootLayer:(const NSPoint)point;
- (OEGridLayer *)OE_layerForPoint:(const NSPoint)point;

- (NSDragOperation)OE_dragOperationForDestinationLayer:(id<NSDraggingInfo>)sender;

- (void)OE_setupFieldEditorForCell:(OEGridViewCell *)cell titleLayer:(CATextLayer *)textLayer;
- (void)OE_cancelFieldEditor;

@end

@implementation OEGridView
@synthesize delegate = _delegate, dataSource = _dataSource;

- (id)initWithFrame:(NSRect)frame
{
    if((self = [super initWithFrame:frame]))
    {
        [self OE_commonGridViewInit];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
    if((self = [super initWithCoder:aDecoder]))
    {
        [self OE_commonGridViewInit];
    }
    
    return self;
}

- (void)OE_commonGridViewInit
{
    // Set default values
    _minimumColumnSpacing = 24.0;
    _rowSpacing           = 20.0;
    _cellSize             = CGSizeMake(250.0, 250.0);

    // Allocate memory for objects
    _selectionIndexes    = [[NSMutableIndexSet alloc] init];
    _visibleCells        = [[NSMutableSet alloc] init];
    _visibleCellsIndexes = [[NSMutableIndexSet alloc] init];
    _reuseableCells      = [[NSMutableSet alloc] init];

    [self setWantsLayer:YES];
    [self setLayer:[[CALayer alloc] init]];
    [[self layer] setFrame:[self bounds]];

    _rootLayer = [[OEGridLayer alloc] init];
    [[self layer] addSublayer:_rootLayer];
    [_rootLayer setInteractive:YES];
    [_rootLayer setGeometryFlipped:YES];
    [_rootLayer setLayoutManager:[OEGridViewLayoutManager layoutManager]];
    [_rootLayer setDelegate:self];
    [_rootLayer setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
    [_rootLayer setFrame:[self bounds]];

    _dragIndicationLayer = [[CALayer alloc] init];
    [_dragIndicationLayer setBorderColor:[[NSColor colorWithDeviceRed:0.03 green:0.41 blue:0.85 alpha:1.0] CGColor]];
    [_dragIndicationLayer setBorderWidth:2.0];
    [_dragIndicationLayer setCornerRadius:8.0];
    [_dragIndicationLayer setHidden:YES];
    [_rootLayer addSublayer:_dragIndicationLayer];

    _fieldEditor = [[OEGridViewFieldEditor alloc] initWithFrame:NSMakeRect(50, 50, 50, 50)];
    [self addSubview:_fieldEditor];

    [self OE_reorderSublayers];
    [self OE_setNeedsReloadData];
}

#pragma mark -
#pragma mark Query Data Sources

- (id)dequeueReusableCell
{
    if([_reuseableCells count] == 0) return nil;

    OEGridViewCell *cell = [_reuseableCells anyObject];
    [_reuseableCells removeObject:cell];
    [cell prepareForReuse];

    return cell;
}

- (NSUInteger)numberOfItems
{
    return _cachedNumberOfItems;
}

- (OEGridViewCell *)cellForItemAtIndex:(NSUInteger)index makeIfNecessary:(BOOL)necessary
{
    __block OEGridViewCell *result = nil;

    // This is my attempt at being thread safe
    if([_visibleCells count] > 0)
    {
        // I'm not sure which is faster, iterate through the cells or use an NSPredicate:
        //   [visibleCells filteredSetUsingPredicate:[NSPredicate predicateWithFormat:@"_index == %d", index]]
        [_visibleCells enumerateObjectsUsingBlock:^(OEGridViewCell *obj, BOOL *stop)
         {
             if([obj _index] == index)
             {
                 result = obj;
                 *stop = YES;
             }
         }];
    }

    if(result == nil && necessary)
    {
        result = [_dataSource gridView:self cellForItemAtIndex:index];
        [result _setIndex:index];
        [result setSelected:[_selectionIndexes containsIndex:index] animated:NO];
        [result setFrame:[self rectForCellAtIndex:index]];
        [result layoutIfNeeded];
    }

    return result;
}

#pragma mark -
#pragma mark Query Cells

- (NSUInteger)indexForCell:(OEGridViewCell *)cell
{
    return [cell _index];
}

- (NSUInteger)indexForCellAtPoint:(NSPoint)point
{
    return [[self indexesForCellsInRect:NSMakeRect(point.x, point.y, 1.0, 1.0)] firstIndex];
}

- (NSIndexSet *)indexesForCellsInRect:(NSRect)rect
{
    // This needs to return both on and off screen cells, make sure that the rect requested is even within the bounds
    if(NSIsEmptyRect(rect) || _cachedNumberOfItems == 0) return [NSIndexSet indexSet];

    // Figure out the first row and column, and the number of cells and rows within the rect.
    NSMutableIndexSet *result   = [NSMutableIndexSet indexSet];
    const NSUInteger   firstCol = (NSUInteger)floor(NSMinX(rect) / _cachedCellSize.width);
    const NSUInteger   firstRow = (NSUInteger)floor(NSMinY(rect) / _cachedCellSize.height);
    const NSUInteger   numCols  = (NSUInteger)ceil(NSMaxX(rect) / _cachedCellSize.width) - firstCol;
    const NSUInteger   numRows  = (NSUInteger)ceil(NSMaxY(rect) / _cachedCellSize.height) - firstRow;

    // Calculate the starting index
    NSUInteger startIndex       = firstCol + (firstRow * _cachedNumberOfVisibleColumns);
    NSUInteger index;

    // As long as the start index is within the number of known items, then we can return some cells
    if(startIndex < _cachedNumberOfItems)
    {
        // Iterate through each row and column, as a row is iterated move the start index by the number of visible columns.
        OEGridViewCell *cell;
        NSRect hitRect, frame;

        for(NSUInteger row = 0; row < numRows; row++)
        {
            index = startIndex;
            for(NSUInteger col = 0; col < numCols; col++, index++)
            {
                if(index >= _cachedNumberOfItems) break;

                cell = [self cellForItemAtIndex:index makeIfNecessary:YES];
                frame = [cell frame];
                hitRect = NSOffsetRect([cell hitRect], NSMinX(frame), NSMinY(frame));

                if(NSIntersectsRect(rect, hitRect)) [result addIndex:index];
            }

            if(index >= _cachedNumberOfItems) break;

            startIndex += _cachedNumberOfVisibleColumns;
        }
    }
    else
    {
        result = [NSIndexSet indexSet];
    }

    // Return an immutable copy
    return [result copy];
}

- (NSArray *)visibleCells
{
    return [_visibleCells allObjects];
}

- (NSIndexSet *)indexesForVisibleCells
{
    // Return an immutable copy
    return [_visibleCellsIndexes copy];
}

- (NSRect)rectForCellAtIndex:(NSUInteger)index
{
    if(index >= _cachedNumberOfItems) return NSZeroRect;
    
    const NSUInteger col = index % _cachedNumberOfVisibleColumns;
    const NSUInteger row = index / _cachedNumberOfVisibleColumns;

    return NSMakeRect(floor(col * _cachedCellSize.width + _cachedColumnSpacing), floor(row * _cachedCellSize.height + _rowSpacing), _cellSize.width, _cellSize.height);
}

#pragma mark -
#pragma mark Selection

- (NSUInteger)indexForSelectedCell
{
    return [_selectionIndexes firstIndex];
}

- (NSIndexSet *)indexesForSelectedCells
{
    // Return an immutable copy
    return [_selectionIndexes copy];
}

- (void)selectCellAtIndex:(NSUInteger)index
{
    if(index == NSNotFound) return;

    OEGridViewCell *item = [self cellForItemAtIndex:index makeIfNecessary:NO];
    [item setSelected:YES animated:![CATransaction disableActions]];

    [_selectionIndexes addIndex:index];

    if(_delegateHas.selectionChanged) [_delegate selectionChangedInGridView:self];
}

- (void)deselectCellAtIndex:(NSUInteger)index
{
    if(index == NSNotFound) return;

    OEGridViewCell *item = [self cellForItemAtIndex:index makeIfNecessary:NO];
    [item setSelected:NO animated:![CATransaction disableActions]];

    [_selectionIndexes removeIndex:index];

    if(_delegateHas.selectionChanged) [_delegate selectionChangedInGridView:self];
}

- (void)selectAll:(id)sender
{
    // We add all the indexes immediately in case the visible cells shift while we are performing this operaiton
    [_selectionIndexes addIndexesInRange:NSMakeRange(0, _cachedNumberOfItems)];
    [_visibleCells enumerateObjectsUsingBlock:^(id obj, BOOL *stop)
     {
         [obj setSelected:YES animated:YES];
     }];

    if(_delegateHas.selectionChanged) [_delegate selectionChangedInGridView:self];
}

- (void)deselectAll:(id)sender
{
    _indexOfKeyboardSelection = NSNotFound;
    if([_selectionIndexes count] == 0) return;

    // We remove all the indexes immediately in case the visible cells shift while we are performing this operaiton
    [_selectionIndexes removeAllIndexes];
    [_visibleCells enumerateObjectsUsingBlock:^(id obj, BOOL *stop)
     {
         [obj setSelected:NO animated:YES];
     }];

    if(_delegateHas.selectionChanged) [_delegate selectionChangedInGridView:self];
}

#pragma mark -
#pragma mark Data Reload

- (void)OE_enqueueCell:(OEGridViewCell *)cell
{
    if([_fieldEditor delegate] == cell) [self OE_cancelFieldEditor];

    [_reuseableCells addObject:cell];
    [_visibleCells removeObject:cell];
    [cell removeFromSuperlayer];
}

- (void)OE_enqueueCells:(NSSet *)cells
{
    for(OEGridViewCell *cell in cells)
        [self OE_enqueueCell:cell];
}

- (void)OE_enqueueCellsAtIndexes:(NSIndexSet *)indexes
{
    [indexes enumerateIndexesUsingBlock:
     ^(NSUInteger idx, BOOL *stop)
     {
         [self OE_enqueueCell:[self cellForItemAtIndex:idx makeIfNecessary:NO]];
     }];
}

- (void)OE_calculateCachedValuesAndQueryForDataChanges:(BOOL)shouldQueryForDataChanges
{
    // OE_clipViewFrameChanged: will call OE_calculateCachedValuesAndQueryForDataChanges when the contentSize is modified, the following supresses
    // additional calls until the calculation is done
    static BOOL alreadyCalculatingCachedValues = NO;
    if(alreadyCalculatingCachedValues) return;

    alreadyCalculatingCachedValues = YES;

    // Collect some basic information of the current environment
    NSScrollView *enclosingScrollView = [self enclosingScrollView];
    NSRect        visibleRect         = (enclosingScrollView ? [enclosingScrollView documentVisibleRect] : [self bounds]);
    NSPoint       contentOffset       = visibleRect.origin;

    const NSSize cachedContentSize    = [self bounds].size;
    const NSSize viewSize             = visibleRect.size;

    // These variables help determine if the calculated values are different than their cached counter parts. These values
    // are recalculated only if needed, so they are all initialized with their cached counter parts. If the recalculated
    // values do not change from their cached counter parts, then there is nothing that we need to do.
    NSUInteger numberOfVisibleColumns = _cachedNumberOfVisibleColumns;  // Number of visible columns
    NSUInteger numberOfVisibleRows    = _cachedNumberOfVisibleRows;     // Number of visible rows
    NSUInteger numberOfItems          = _cachedNumberOfItems;           // Number of items in the data source
    NSSize     cellSize               = NSMakeSize(_cellSize.width + _minimumColumnSpacing, _cellSize.height + _rowSpacing);
                                                                        // Item Size (within minimumColumnSpacing and rowSpacing)
    NSSize contentSize                = cachedContentSize;              // The scroll view's content size
    BOOL   checkForDataReload         = FALSE;                          // Used to determine if we should consider reloading the data

    // Query the data source for the number of items it has, this is only done if the caller explicitly sets shouldQueryForDataChanges.
    if(shouldQueryForDataChanges && _dataSource) numberOfItems = [_dataSource numberOfItemsInGridView:self];

    // Check to see if the frame's width has changed to update the number of visible columns and the cached cell size
    if(cellSize.width == 0)
        numberOfVisibleColumns = 1;
    else if(_cachedViewSize.width != viewSize.width ||
            !NSEqualSizes(_cachedCellSize, cellSize))
    {
        // Set the number of visible columns based on the view's width, there must be at least 1 visible column and no more than the total number
        // of items within the data source.  Just because a column is potentially visible doesn't mean that there is enough data to populate it.
        numberOfVisibleColumns = MAX((NSUInteger)(floor(viewSize.width / cellSize.width)), 1);

        // The cell's height include the original itemSize.height + rowSpacing. The cell's column spacing is based on the number of visible columns.
        // The cell will be at least itemSize.width + minimumColumnSpacing, it could grow as larg as the width of the view
        cellSize = NSMakeSize(MAX(cellSize.width, round(viewSize.width / numberOfVisibleColumns)), cellSize.height);

        // Make sure that the scroll view's content width reflects the view's width. The scroll view's content height is be calculated later (if
        // needed).
        contentSize.width = viewSize.width;
    }

    // Check to see if the frame's height has changed to update the number of visible rows
    if(cellSize.height == 0)
        numberOfVisibleRows = 1;
    else if(_cachedViewSize.height != viewSize.height ||
            cellSize.height != _cachedCellSize.height)
    {
        // TODO: only add 1 to the number of visible rows if the first row is partially visible
        numberOfVisibleRows = ((NSUInteger)ceil(viewSize.height / cellSize.height)) + 1;
    }

    // Check to see if the number of items, number of visible columns, or cached cell size has changed
    if(_cachedNumberOfItems          != numberOfItems          ||
       _cachedNumberOfVisibleColumns != numberOfVisibleColumns ||
       !NSEqualSizes(_cachedCellSize, cellSize)                ||
       !NSEqualSizes(_cachedViewSize, viewSize))
    {
        // These three events may require a data reload but will most definitely cause the scroll view's content size to change
        checkForDataReload = YES;

        if(numberOfItems == 0)
            contentSize.height = viewSize.height;
        else
        {
            NSUInteger numberOfRows = ceil(((CGFloat)numberOfItems / (CGFloat)numberOfVisibleColumns));
            contentSize.height      = MAX(viewSize.height, ceil(numberOfRows * cellSize.height) + _rowSpacing);
        }
        [super setFrameSize:contentSize];

        // Changing the size of the frame may also change the contentOffset, recalculate that value
        visibleRect   = (enclosingScrollView ? [enclosingScrollView documentVisibleRect] : [self bounds]);
        contentOffset = visibleRect.origin;

        // Check to see if the number visible columns or the cell size has changed as these vents will cause the layout to be recalculated
        if(_cachedNumberOfVisibleColumns != numberOfVisibleColumns || !NSEqualSizes(_cachedCellSize, cellSize))
            [self OE_setNeedsLayoutGridView];
    }

    // Check to see if the number of visible rows have changed
    // Check to see if the scroll view's content offset or the view's height has changed
    if(_cachedNumberOfVisibleRows != numberOfVisibleRows ||
       _cachedContentOffset.y     != contentOffset.y     ||
       _cachedViewSize.height     != viewSize.height)
    {
        // This event may require a data reload
        checkForDataReload = YES;
    }

    // Update the cached values
    _cachedViewSize               = viewSize;
    _cachedCellSize               = cellSize;
    _cachedColumnSpacing          = round((cellSize.width - _cellSize.width) / 2.0);
    _cachedNumberOfVisibleColumns = numberOfVisibleColumns;
    _cachedNumberOfVisibleRows    = numberOfVisibleRows;
    _cachedNumberOfItems          = numberOfItems;
    _cachedContentOffset          = contentOffset;

    // We're done calculating all of the values, the following signals that the -OE_calculateCachedValuesAndQueryForDataChanges: can be unblocked
    alreadyCalculatingCachedValues = FALSE;

    // Reload data when appropriate
    if(checkForDataReload && _cachedNumberOfItems > 0) [self OE_checkForDataReload];
}

- (void)OE_checkForDataReload
{
    // Check to see if the visible cells have changed
    NSScrollView    *enclosingScrollView = [self enclosingScrollView];
    const NSRect     visibleRect         = (enclosingScrollView ? [enclosingScrollView documentVisibleRect] : [self bounds]);
    const NSSize     contentSize         = [self bounds].size;
    const NSSize     viewSize            = visibleRect.size;
    const CGFloat    maxContentOffset    = MAX(contentSize.height - viewSize.height, contentSize.height - _cachedCellSize.height);
    const CGFloat    contentOffsetY      = MAX(MIN(_cachedContentOffset.y, maxContentOffset), 0.0);
    const NSUInteger row                 = (NSUInteger)floor(contentOffsetY / _cachedCellSize.height);
    const NSUInteger firstVisibleIndex   = row * _cachedNumberOfVisibleColumns;
    const NSUInteger visibleIndexLength  = MIN(_cachedNumberOfVisibleColumns * _cachedNumberOfVisibleRows, _cachedNumberOfItems - firstVisibleIndex);
    const NSRange    visibleIndexRange   = NSMakeRange(firstVisibleIndex, visibleIndexLength);

    NSIndexSet *visibleCellsIndexSet = [NSIndexSet indexSetWithIndexesInRange:visibleIndexRange];
    if ([_visibleCellsIndexes isEqualToIndexSet:visibleCellsIndexSet]) return;

    // Calculate which cells to remove from view
    if([_visibleCellsIndexes count] != 0)
    {
        NSMutableIndexSet *removeIndexSet = [_visibleCellsIndexes mutableCopy];
        [removeIndexSet removeIndexes:visibleCellsIndexSet];

        if([removeIndexSet count] != 0) [self OE_enqueueCellsAtIndexes:removeIndexSet];
    }

    // Calculate which cells to add to view
    NSMutableIndexSet *addIndexSet = [visibleCellsIndexSet mutableCopy];
    if([_visibleCellsIndexes count] != 0)
    {
        [addIndexSet removeIndexes:_visibleCellsIndexes];
        [_visibleCellsIndexes removeAllIndexes];
    }

    // Update the visible cells index set
    [_visibleCellsIndexes addIndexes:visibleCellsIndexSet];

    if([addIndexSet count] != 0) [self reloadCellsAtIndexes:addIndexSet];
}

- (void)OE_setNeedsReloadData
{
    _needsReloadData = YES;
    [_rootLayer setNeedsLayout];
}

- (void)OE_reloadDataIfNeeded
{
    if(_needsReloadData) [self reloadData];
}

- (void)reloadData
{
    [_selectionIndexes removeAllIndexes];
    _indexOfKeyboardSelection = NSNotFound;

    [_visibleCells makeObjectsPerformSelector:@selector(removeFromSuperlayer)];
    [_visibleCells removeAllObjects];
    [_visibleCellsIndexes removeAllIndexes];
    [_reuseableCells removeAllObjects];

    _cachedNumberOfVisibleColumns = 0;
    _cachedNumberOfVisibleRows    = 0;
    _cachedNumberOfItems          = 0;

    _cachedContentOffset          = NSZeroPoint;
    _cachedViewSize               = NSZeroSize;
    _cachedCellSize               = NSZeroSize;
    _cachedColumnSpacing          = 0.0;

    [_noItemsView removeFromSuperview];
    _noItemsView = nil;

    [self setFrameSize:NSZeroSize];

    // Recalculate all of the required cached values
    [self OE_calculateCachedValuesAndQueryForDataChanges:YES];

    // If there are no items, then remove all the cells
    if(_cachedNumberOfItems == 0)
    {
        // Enqueue all the cells for later use and remove them from the view
        [self OE_enqueueCells:_visibleCells];
        [_visibleCellsIndexes removeAllIndexes];

        // Check to see if the dataSource has a view to display when there is nothing to display
        if(_dataSourceHas.viewForNoItemsInGridView)
        {
            _noItemsView = [_dataSource viewForNoItemsInGridView:self];
            if(_noItemsView)
            {
                [self addSubview:_noItemsView];
                [_noItemsView setHidden:NO];
                [self OE_centerNoItemsView];
            }
        }
    }
    else if(_noItemsView)
    {
        [_noItemsView removeFromSuperview];
        _noItemsView = nil;
    }

    _needsReloadData = NO;
}

- (void)reloadCellsAtIndexes:(NSIndexSet *)indexes
{
    // If there is no index set or no items in the index set, then there is nothing to update
    if([indexes count] == 0) return;

    [indexes enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop)
     {
         // If the cell is not already visible, then there is nothing to reload
         if([_visibleCellsIndexes containsIndex:idx])
         {
             OEGridViewCell *newCell = [_dataSource gridView:self cellForItemAtIndex:idx];
             OEGridViewCell *oldCell = [self cellForItemAtIndex:idx makeIfNecessary:NO];
             if(newCell != oldCell)
             {
                 if(oldCell) [newCell setFrame:[oldCell frame]];

                 // Prepare the new cell for insertion
                 if (newCell)
                 {
                     [newCell _setIndex:idx];
                     [newCell setSelected:[_selectionIndexes containsIndex:idx] animated:NO];

                     // Replace the old cell with the new cell
                     if(oldCell)
                     {
                         [oldCell removeFromSuperlayer];
                         [self OE_enqueueCell:oldCell];
                     }
                     [newCell setOpacity:1.0];
                     [newCell setHidden:NO];

                     if(!oldCell) [newCell setFrame:[self rectForCellAtIndex:idx]];

                     [_visibleCells addObject:newCell];
                     [_rootLayer addSublayer:newCell];
                 }

                 [self OE_setNeedsLayoutGridView];
             }
         }
     }];
    [self OE_reorderSublayers];
}

#pragma mark -
#pragma mark View Operations

- (BOOL)isFlipped
{
    return YES;
}

- (void)viewWillMoveToWindow:(NSWindow *)newWindow
{
    NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];
    NSWindow             *oldWindow          = [self window];

    if(oldWindow)
    {
        [notificationCenter removeObserver:self name:NSWindowDidBecomeKeyNotification object:oldWindow];
        [notificationCenter removeObserver:self name:NSWindowDidResignKeyNotification object:oldWindow];
    }

    if(newWindow)
    {
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_windowChangedKey:) name:NSWindowDidBecomeKeyNotification object:[self window]];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_windowChangedKey:) name:NSWindowDidResignKeyNotification object:[self window]];
    }
}

- (void)viewWillMoveToSuperview:(NSView *)newSuperview
{
    NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];
    NSClipView           *newClipView        = ([newSuperview isKindOfClass:[NSClipView class]] ? (NSClipView *)newSuperview : nil);
    NSClipView           *oldClipView        = [[self enclosingScrollView] contentView];

    if(oldClipView)
    {
        [notificationCenter removeObserver:self name:NSViewBoundsDidChangeNotification object:oldClipView];
        [notificationCenter removeObserver:self name:NSViewFrameDidChangeNotification object:oldClipView];
    }

    if(newClipView)
    {
        [self setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
        [notificationCenter addObserver:self selector:@selector(OE_clipViewFrameChanged:) name:NSViewBoundsDidChangeNotification object:newClipView];
        [notificationCenter addObserver:self selector:@selector(OE_clipViewFrameChanged:) name:NSViewFrameDidChangeNotification object:newClipView];
        [newClipView setPostsBoundsChangedNotifications:YES];
        [newClipView setPostsFrameChangedNotifications:YES];
    }
}

- (void)OE_updateSelectedCellsActiveSelectorWithFocus:(BOOL)focus
{
    if([_selectionIndexes count] == 0                                     ||
       [_selectionIndexes lastIndex]  < [_visibleCellsIndexes firstIndex] ||
       [_selectionIndexes firstIndex] > [_visibleCellsIndexes lastIndex])
        return;

    NSMutableIndexSet *visibleAndSelected = [_selectionIndexes mutableCopy];
    [visibleAndSelected removeIndexesInRange:NSMakeRange([_selectionIndexes firstIndex], [_visibleCellsIndexes firstIndex] - [_selectionIndexes firstIndex])];
    [visibleAndSelected removeIndexesInRange:NSMakeRange([_visibleCellsIndexes lastIndex] + 1, [_selectionIndexes lastIndex] - [_visibleCellsIndexes lastIndex])];

    if([visibleAndSelected count] > 0)
    {
        [visibleAndSelected enumerateIndexesUsingBlock:
         ^(NSUInteger idx, BOOL *stop)
         {
             OEGridViewCell *cell = [self cellForItemAtIndex:idx makeIfNecessary:NO];
             if(cell)
             {
                 if(focus)
                     [cell didBecomeFocused];
                 else
                     [cell willResignFocus];
             }
         }];
    }
}

- (void)OE_windowChangedKey:(NSNotification *)notification
{
    if([notification name] == NSWindowDidBecomeKeyNotification)
        [self OE_updateSelectedCellsActiveSelectorWithFocus:YES];
    else if([notification name] == NSWindowDidResignKeyNotification)
        [self OE_updateSelectedCellsActiveSelectorWithFocus:NO];
}

- (void)OE_clipViewFrameChanged:(NSNotification *)notification
{
    NSScrollView *enclosingScrollView = [self enclosingScrollView];

    if(_noItemsView)
    {
        [self setFrame:[enclosingScrollView bounds]];
        [self OE_centerNoItemsView];
    }
    else
    {
        const NSRect visibleRect = (enclosingScrollView ? [enclosingScrollView documentVisibleRect] : [self bounds]);

        if(!NSEqualSizes(_cachedViewSize, visibleRect.size))
        {
            [self OE_cancelFieldEditor];
            [self OE_calculateCachedValuesAndQueryForDataChanges:NO];
        }
        else if(!NSEqualPoints(_cachedContentOffset, visibleRect.origin))
        {
            _cachedContentOffset = visibleRect.origin;
            [self OE_checkForDataReload];
        }

        [CATransaction begin];
        [CATransaction setDisableActions:YES];
        [self OE_updateDecorativeLayers];
        [CATransaction commit];
    }
}

- (void)OE_centerNoItemsView
{
    if(!_noItemsView) return;

    NSView       *enclosingScrollView = [self enclosingScrollView] ? : self;
    const NSRect  visibleRect         = [enclosingScrollView visibleRect];
    const NSSize  viewSize            = [_noItemsView frame].size;
    const NSRect  viewFrame           = NSMakeRect(ceil((NSWidth(visibleRect) - viewSize.width) / 2.0),
                                                   ceil((NSHeight(visibleRect) - viewSize.height) / 2.0),
                                                   viewSize.width, viewSize.height);
    [_noItemsView setFrame:viewFrame];
}

#pragma mark -
#pragma mark Layer Operations

- (id)actionForLayer:(CALayer *)layer forKey:(NSString *)event
{
    return [NSNull null];
}

- (void)OE_reorderSublayers
{
    [_rootLayer insertSublayer:_backgroundLayer atIndex:0];

    unsigned int index = (unsigned int)[[_rootLayer sublayers] count];
    [_rootLayer insertSublayer:_foregroundLayer atIndex:index];
    [_rootLayer insertSublayer:_selectionLayer atIndex:index];
    [_rootLayer insertSublayer:_dragIndicationLayer atIndex:index];
}

- (void)OE_updateDecorativeLayers
{
    if(!_dragIndicationLayer && !_backgroundLayer && !_foregroundLayer) return;

    NSScrollView *enclosingScrollView   = [self enclosingScrollView];
    const NSRect visibleRect            = (enclosingScrollView ? [enclosingScrollView documentVisibleRect] : [self bounds]);
    const NSRect decorativeFrame        = NSIntegralRect(NSOffsetRect((enclosingScrollView ? [enclosingScrollView frame] : visibleRect), NSMinX(visibleRect), NSMinY(visibleRect)));

    [_backgroundLayer setFrame:decorativeFrame];
    [_foregroundLayer setFrame:decorativeFrame];
    [_dragIndicationLayer setFrame:NSInsetRect(decorativeFrame, 1.0, 1.0)];
}

- (void)OE_setNeedsLayoutGridView
{
    _needsLayoutGridView = YES;
    [_rootLayer setNeedsLayout];
}

- (void)OE_layoutGridViewIfNeeded
{
    // -layoutSublayers is called for every little thing, this checks to see if we really intended to adjust the location of the cells. This value can
    // be set using OE_setNeedsLayoutGridView
    if(_needsLayoutGridView) [self OE_layoutGridView];
}

- (void)OE_layoutGridView
{
    if([_visibleCells count] == 0) return;

    [_visibleCells enumerateObjectsUsingBlock:
     ^(id obj, BOOL *stop)
     {
         [obj setFrame:[self rectForCellAtIndex:[obj _index]]];
     }];

    _needsLayoutGridView = NO;
}

- (void)layoutSublayers
{
    [self OE_reloadDataIfNeeded];
    [self OE_updateDecorativeLayers];
    [self OE_layoutGridViewIfNeeded];
}

#pragma mark -
#pragma mark Responder Chain

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)becomeFirstResponder
{
    [self OE_updateSelectedCellsActiveSelectorWithFocus:YES];
    return YES;
}

- (BOOL)resignFirstResponder
{
    [self OE_updateSelectedCellsActiveSelectorWithFocus:NO];
    return YES;
}

#pragma mark -
#pragma mark Mouse Handling Operations

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
    return YES;
}

- (NSPoint)OE_pointInViewFromEvent:(NSEvent *)theEvent
{
    return [self convertPoint:[theEvent locationInWindow] fromView:nil];
}

- (NSPoint)OE_convertPointToRootLayer:(const NSPoint)point
{
    NSPoint result = point;

    if([self isFlipped]) result.y = CGRectGetMaxY([_rootLayer frame]) - result.y - 1.0;

    return result;
}

- (OEGridLayer *)OE_layerForPoint:(const NSPoint)point
{
    for(OEGridLayer *obj in [_rootLayer sublayers])
    {
        if(![obj isKindOfClass:[OEGridLayer class]]) continue;

        OEGridLayer *hitLayer = (OEGridLayer *)[obj hitTest:point];

        if([hitLayer isKindOfClass:[OEGridLayer class]]) return hitLayer;
    }

    return nil;
}

- (void)mouseDown:(NSEvent *)theEvent
{    
    const NSPoint pointInView = [self OE_pointInViewFromEvent:theEvent];
    _trackingLayer            = [self OE_layerForPoint:pointInView];

    if(![_trackingLayer isInteractive]) _trackingLayer = _rootLayer;

    OEGridViewCell *cell = nil;
    if ([_trackingLayer isKindOfClass:[OEGridViewCell class]])
        cell = (OEGridViewCell *)_trackingLayer;

    if(cell == nil && _trackingLayer != nil && _trackingLayer != _rootLayer)
    {
        const NSPoint pointInLayer = [_rootLayer convertPoint:pointInView toLayer:_trackingLayer];
        [_trackingLayer mouseDownAtPointInLayer:pointInLayer withEvent:theEvent];
        if(![_trackingLayer isTracking]) _trackingLayer = nil;
    }

    NSUInteger modifierFlags = [[NSApp currentEvent] modifierFlags];
    BOOL commandKeyDown      = ((modifierFlags & NSCommandKeyMask) == NSCommandKeyMask);
    BOOL shiftKeyDown        = ((modifierFlags & NSShiftKeyMask) == NSShiftKeyMask);
    BOOL invertSelection     = commandKeyDown || shiftKeyDown;

    // Figure out which cell was touched, inverse it's selection...
    if(cell != nil)
    {
        if(!invertSelection && ![cell isSelected]) [self deselectAll:self];

        NSUInteger idx = [cell _index];
        if(![_selectionIndexes containsIndex:idx])
        {
            [self selectCellAtIndex:idx];
            _indexOfKeyboardSelection = idx;
        }
        else if(invertSelection)
        {
            [self deselectCellAtIndex:idx];
            _indexOfKeyboardSelection = [_selectionIndexes lastIndex];
        }
    }
    else if(_trackingLayer == nil || _trackingLayer == _rootLayer)
    {
        _trackingLayer = _rootLayer;

        if(!invertSelection) [self deselectAll:self];

        // If the command key was pressed and there are already a list of selected indexes, then we may want to invert the items that are already selected
        if(invertSelection && [_selectionIndexes count] > 0)
            _originalSelectionIndexes = [_selectionIndexes copy];
    }

    // Start tracking mouse
    NSEvent *lastMouseDragEvent = nil;
    BOOL     periodicEvents     = (_trackingLayer == _rootLayer);

    _initialPoint = pointInView;

    if(periodicEvents) [NSEvent startPeriodicEventsAfterDelay:OEInitialPeriodicDelay withPeriod:OEPeriodicInterval];

    const NSUInteger mask = NSLeftMouseUpMask | NSLeftMouseDraggedMask | NSKeyDownMask | (periodicEvents ? NSPeriodicMask : 0);
    while(_trackingLayer && (theEvent = [[self window] nextEventMatchingMask:mask]))
    {
        if(periodicEvents && [theEvent type] == NSPeriodic)
        {
            if(lastMouseDragEvent)
            {
                [self mouseDragged:lastMouseDragEvent];

                const NSPoint point = [self convertPoint:[lastMouseDragEvent locationInWindow] fromView:nil];
                if(!NSPointInRect(point, [self bounds])) lastMouseDragEvent = nil;
            }
        }
        else if([theEvent type] == NSLeftMouseDragged)
        {
            const NSPoint point = [self convertPoint:[theEvent locationInWindow] fromView:nil];
            lastMouseDragEvent  = (NSPointInRect(point, [self visibleRect]) ? nil : theEvent);
            [self mouseDragged:theEvent];
        }
        else if([theEvent type] == NSLeftMouseUp)
        {
            [self mouseUp:theEvent];
            break;
        }
        else if([theEvent type] == NSKeyDown)
        {
            NSBeep();
        }
    }

    lastMouseDragEvent = nil;
    _trackingLayer     = nil;

    if(periodicEvents) [NSEvent stopPeriodicEvents];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    if(_trackingLayer == nil) return;

    const NSPoint pointInView = [self OE_pointInViewFromEvent:theEvent];

    if([_trackingLayer isKindOfClass:[OEGridViewCell class]])
    {
        if(_dataSourceHas.pasteboardWriterForIndex && [_selectionIndexes count] > 0)
        {
            const NSPoint draggedDistance = NSMakePoint(ABS(pointInView.x - _initialPoint.x), ABS(pointInView.y - _initialPoint.y));
            if(draggedDistance.x >= 5.0 || draggedDistance.y >= 5.0 ||
               (draggedDistance.x * draggedDistance.x + draggedDistance.y * draggedDistance.y) >= 25)
            {
                __block NSMutableArray *draggingItems = [NSMutableArray array];
                [_selectionIndexes enumerateIndexesUsingBlock:
                 ^(NSUInteger idx, BOOL *stop)
                 {
                     id<NSPasteboardWriting> item = [_dataSource gridView:self pasteboardWriterForIndex:idx];
                     if(item != nil)
                     {
                         NSDraggingItem *dragItem = [[NSDraggingItem alloc] initWithPasteboardWriter:item];
                         OEGridViewCell *cell = [self cellForItemAtIndex:idx makeIfNecessary:YES];
                         [dragItem setDraggingFrame:NSOffsetRect([cell hitRect], NSMinX([cell frame]), NSMinY([cell frame])) contents:[cell draggingImage]];
                         [draggingItems addObject:dragItem];
                     }
                 }];

                if([draggingItems count] > 0)
                {
                    _draggingSession = [self beginDraggingSessionWithItems:draggingItems event:theEvent source:self];
                    [_draggingSession setDraggingFormation:NSDraggingFormationStack];
                }
                _trackingLayer = nil;
            }
        }
        else
        {
            _trackingLayer = nil;
        }
    }
    else if(_trackingLayer != _rootLayer)
    {
        const NSPoint pointInLayer = [_rootLayer convertPoint:pointInView toLayer:_trackingLayer];
        [_trackingLayer mouseMovedAtPointInLayer:pointInLayer withEvent:theEvent];
    }
    else
    {
        // Make sure that the view is scrolled
        [self autoscroll:theEvent];

        // Calculate the selection rect
        const CGRect bounds        = [self bounds];

        CGRect selectionRect;
        selectionRect.origin = CGPointMake(MIN(pointInView.x, _initialPoint.x), MIN(pointInView.y, _initialPoint.y));
        selectionRect.size   = CGSizeMake(ABS(pointInView.x - _initialPoint.x), ABS(pointInView.y - _initialPoint.y));

        if(CGRectGetMinX(selectionRect) < 0.0)
        {
            selectionRect.size.width += CGRectGetMinX(selectionRect);
            selectionRect.origin.x = 0.0;
        }

        if(CGRectGetMaxX(selectionRect) > CGRectGetMaxX(bounds))
        {
            selectionRect.size.width = CGRectGetMaxX(bounds) - CGRectGetMinX(selectionRect);
        }

        if(CGRectGetMinY(selectionRect) < 1.0)
        {
            selectionRect.size.height += CGRectGetMinY(selectionRect);
            selectionRect.origin.y = 1.0;
        }

        if(CGRectGetMaxY(selectionRect) > CGRectGetMaxY(bounds))
        {
            selectionRect.size.height = CGRectGetMaxY(bounds) - CGRectGetMinY(selectionRect);
        }

        [CATransaction begin];
        [CATransaction setDisableActions:YES];

        // Create the selection view if it doesn't exisit...set the frame to the previous calculation
        if(_selectionLayer == nil)
        {
            _selectionLayer = [[CALayer alloc] init];
            [_selectionLayer setBackgroundColor:[[NSColor colorWithCalibratedWhite:1.0 alpha:0.3] CGColor]];
            [_selectionLayer setBorderColor:[[NSColor whiteColor] CGColor]];
            [_selectionLayer setBorderWidth:1.0];
            [_rootLayer addSublayer:_selectionLayer];
            [self OE_reorderSublayers];
        }

        [_selectionLayer setFrame:CGRectIntegral(selectionRect)];

        [CATransaction commit];

        // Determine which cells to select and which ones to deselect
        NSIndexSet        *indexesUnderSelectionRect = [self indexesForCellsInRect:selectionRect];
        NSMutableIndexSet *indexesToSelect           = nil;
        NSMutableIndexSet *indexesToDeselect         = nil;

        if(_originalSelectionIndexes)
        {
            /** Invert the selection */
            // Calculate the new indexes to select...it should be:
            //  indexesToSelect = (_originalSelectionIndexes + indexesUnderSelectionRect) - (_originalSelectionIndexes X indexesUnderSelectionRect).
            indexesToSelect = [_originalSelectionIndexes mutableCopy];
            [indexesToSelect addIndexes:indexesUnderSelectionRect];

            if([indexesUnderSelectionRect firstIndex] != NSNotFound)
            {
                const NSUInteger firstIndex = [indexesUnderSelectionRect firstIndex];
                const NSUInteger lastIndex  = [indexesUnderSelectionRect lastIndex];

                NSIndexSet *intersection = [_originalSelectionIndexes indexesInRange:NSMakeRange(firstIndex, lastIndex - firstIndex + 1)
                                                                             options:0
                                                                         passingTest:
                                            ^BOOL (NSUInteger idx, BOOL *stop)
                                            {
                                                return [indexesUnderSelectionRect containsIndex:idx];
                                            }];

                [indexesToSelect removeIndexes:intersection];
            }
        }
        else
        {
            /** Select the indexes under selection rect */
            indexesToSelect = [indexesUnderSelectionRect mutableCopy];
        }

        BOOL selectionChanged = NO;
        if([indexesToDeselect firstIndex] != NSNotFound)
        {
            // Figure out which indexes that are currently selected that need to be deslected: indexesToDeselect = _selectionIndexes - indexesToSelect
            indexesToDeselect = [_selectionIndexes mutableCopy];
            [indexesToDeselect removeIndexes:indexesToSelect];
            [indexesToDeselect enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop)
             {
                 [[self cellForItemAtIndex:idx makeIfNecessary:NO] setSelected:NO animated:YES];
             }];
            [_selectionIndexes removeIndexes:indexesToDeselect];
            selectionChanged = YES;
        }

        if([indexesToSelect firstIndex] != NSNotFound)
        {
            // Figure out which indexes that are not selected that need to be selected: indexesToSelect = _selectionIndexes - indexesToDeselect
            [indexesToSelect removeIndexes:_selectionIndexes];
            [indexesToSelect enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop)
             {
                 [[self cellForItemAtIndex:idx makeIfNecessary:NO] setSelected:YES animated:YES];
             }];
            [_selectionIndexes addIndexes:indexesToSelect];
            selectionChanged = YES;
        }

        if(selectionChanged && _delegateHas.selectionChanged) [_delegate selectionChangedInGridView:self];

        _indexOfKeyboardSelection = [_selectionIndexes lastIndex];
    }
}

- (void)mouseUp:(NSEvent *)theEvent
{
    if(_trackingLayer == nil) return;

    const NSPoint pointInView = [self OE_pointInViewFromEvent:theEvent];

    if([_trackingLayer isKindOfClass:[OEGridViewCell class]])
    {
        if([theEvent clickCount] == 2 && _delegateHas.doubleClickedCellForItemAtIndex)
        {
            OEGridViewCell *cell = (OEGridViewCell *)[self OE_layerForPoint:pointInView];
            if ([cell isKindOfClass:[OEGridViewCell class]])
                [_delegate gridView:self doubleClickedCellForItemAtIndex:[cell _index]];
        }
    }
    else if(_trackingLayer != _rootLayer)
    {
        const NSPoint pointInLayer = [_rootLayer convertPoint:pointInView toLayer:_trackingLayer];
        [_trackingLayer mouseUpAtPointInLayer:pointInLayer withEvent:theEvent];
    }
    else
    {
        [_selectionLayer removeFromSuperlayer];
        _selectionLayer = nil;
        _originalSelectionIndexes = nil;

        if([theEvent clickCount] == 2)
        {
            CALayer *hitLayer = [_rootLayer hitTest:[self OE_convertPointToRootLayer:pointInView]];
            if(hitLayer && [hitLayer isKindOfClass:[CATextLayer class]])
            {
                CATextLayer *titleLayer = (CATextLayer *)hitLayer;
                CALayer     *superlayer = [titleLayer superlayer];
                while(superlayer)
                {
                    if([superlayer isKindOfClass:[OEGridViewCell class]]) break;

                    superlayer = [superlayer superlayer];
                }

                if(superlayer) [self OE_setupFieldEditorForCell:(OEGridViewCell *)superlayer titleLayer:titleLayer];
            }
        }
    }
    _trackingLayer = nil;
}

- (void)OE_setupFieldEditorForCell:(OEGridViewCell *)cell titleLayer:(CATextLayer *)textLayer
{
    NSRect fieldFrame = [_rootLayer convertRect:[textLayer bounds] fromLayer:textLayer];
    fieldFrame        = NSOffsetRect(NSInsetRect(fieldFrame, 0.0, -1.0), 0.0, -1.0);
    [_fieldEditor setFrame:fieldFrame];

    NSString *title = [textLayer string];
    [_fieldEditor setString:title];
    [_fieldEditor setDelegate:cell];
    [_fieldEditor setHidden:NO];
    [[self window] makeFirstResponder:[[_fieldEditor subviews] objectAtIndex:0]];
}

- (void)OE_cancelFieldEditor
{
    if([_fieldEditor isHidden]) return;

    OEGridViewCell *delegate = [_fieldEditor delegate];

    if([delegate isKindOfClass:[OEGridViewCell class]]) [delegate setEditing:NO];

    [_fieldEditor setHidden:YES];
    [[self window] makeFirstResponder:self];
}

- (NSMenu *)menuForEvent:(NSEvent *)theEvent
{
    [[self window] makeFirstResponder:self];
    
    NSPoint mouseLocationInWindow = [theEvent locationInWindow];
    NSPoint mouseLocationInView = [self convertPoint:mouseLocationInWindow fromView:nil];
    
    NSUInteger index = [self indexForCellAtPoint:mouseLocationInView];
    if(index != NSNotFound && _dataSourceHas.menuForItemsAtIndexes)
    {
        BOOL itemIsSelected = [[self selectionIndexes] containsIndex:index];
        OEGridViewCell *itemCell = [self cellForItemAtIndex:index makeIfNecessary:YES];
        
        NSIndexSet* indexes = itemIsSelected ? [self selectionIndexes] : [NSIndexSet indexSetWithIndex:index];
        
        NSRect hitRect = NSInsetRect([itemCell hitRect], 5, 5);
        NSRect hitRectOnWindow = [itemCell convertRect:hitRect toLayer:nil];
        NSRect visibleRectOnWindow = [self convertRect:[self visibleRect] toView:nil];
        NSRect visibleItemRect = NSIntersectionRect(hitRectOnWindow, visibleRectOnWindow);
        
        if(!itemIsSelected)
            [self setSelectionIndexes:[NSIndexSet indexSetWithIndex:index]];
        
        OEMenu *contextMenu = [[self dataSource] gridView:self menuForItemsAtIndexes:indexes];
        
        if([[NSUserDefaults standardUserDefaults] boolForKey:UDLightStyleGridViewMenu])
            [contextMenu setStyle:OEMenuStyleLight];
        
        OERectEdge edge = OEMaxXEdge;
        if( NSHeight(visibleItemRect) < 25.0 )
            edge = NSMinY(visibleItemRect) == NSMinY(visibleRectOnWindow) ? OEMaxYEdge : OEMinYEdge;
        
        [contextMenu openOnEdge:edge ofRect:visibleItemRect ofWindow:[self window]];
        
        return nil;
    }
    
    return [self menu];
}
#pragma mark -
#pragma mark Keyboard Handling Operations

- (void)OE_moveKeyboardSelectionToIndex:(NSUInteger)index
{
    NSUInteger modifierFlags = [[NSApp currentEvent] modifierFlags];
    BOOL       multiSelect   = ((modifierFlags & NSCommandKeyMask) == NSCommandKeyMask) || ((modifierFlags & NSShiftKeyMask) == NSShiftKeyMask);

    if(!multiSelect) [self deselectAll:self];

    if(index != NSNotFound)
    {
        [self selectCellAtIndex:index];
        [self scrollRectToVisible:NSIntegralRect(NSInsetRect([self rectForCellAtIndex:index], 0.0, -_rowSpacing))];
    }

    _indexOfKeyboardSelection = index;
}

- (void)cancelOperation:(id)sender
{
    [self OE_moveKeyboardSelectionToIndex:NSNotFound];
}

- (void)moveUp:(id)sender
{
    if(_cachedNumberOfItems == 0) return;

    NSUInteger index = 0;
    if(_indexOfKeyboardSelection == NSNotFound)
        index = (_cachedNumberOfItems / _cachedNumberOfVisibleColumns) * _cachedNumberOfVisibleColumns;
    else
        index = MIN(_indexOfKeyboardSelection, _indexOfKeyboardSelection - _cachedNumberOfVisibleColumns);

    [self OE_moveKeyboardSelectionToIndex:index];
}

- (void)moveDown:(id)sender
{
    if(_cachedNumberOfItems == 0) return;

    NSUInteger index = 0;
    if(_indexOfKeyboardSelection == NSNotFound)
        index = 0;
    else
    {
        index = _indexOfKeyboardSelection + _cachedNumberOfVisibleColumns;
        if(index >= _cachedNumberOfItems) index = _indexOfKeyboardSelection;
    }

    [self OE_moveKeyboardSelectionToIndex:index];
}

- (void)moveLeft:(id)sender
{
    if(_cachedNumberOfItems == 0) return;

    NSUInteger index = 0;
    if(_indexOfKeyboardSelection == NSNotFound)
        index = MIN(_cachedNumberOfVisibleColumns, _cachedNumberOfItems) - 1;
    else
    {
        if(_indexOfKeyboardSelection > 0)
        {
            const NSUInteger rowFirstIndex = (_indexOfKeyboardSelection / _cachedNumberOfVisibleColumns) * _cachedNumberOfVisibleColumns;
            index = MAX(rowFirstIndex, _indexOfKeyboardSelection - 1);
        }
        else
            index = 0;
    }

    [self OE_moveKeyboardSelectionToIndex:index];
}

- (void)moveRight:(id)sender
{
    if(_cachedNumberOfItems == 0) return;

    NSUInteger index = 0;
    if(_indexOfKeyboardSelection == NSNotFound)
        index = 0;
    else
    {
        const NSUInteger rowLastIndex = MIN((((_indexOfKeyboardSelection / _cachedNumberOfVisibleColumns) + 1) * _cachedNumberOfVisibleColumns), _cachedNumberOfItems);
        index = MIN(rowLastIndex - 1, _indexOfKeyboardSelection + 1);
    }

    [self OE_moveKeyboardSelectionToIndex:index];
}

#pragma mark -
#pragma mark NSDraggingDestination

- (NSDragOperation)OE_dragOperationForDestinationLayer:(id<NSDraggingInfo>)sender
{
    const NSPoint pointInView             = [self convertPoint:[sender draggingLocation] fromView:nil];
    OEGridLayer  *newDragDestinationLayer = [self OE_layerForPoint:pointInView];
    if(_dragDestinationLayer != newDragDestinationLayer)
    {
        if(newDragDestinationLayer == _rootLayer)
        {
            _prevDragDestinationLayer = nil;
            _dragDestinationLayer = nil;
        }
        else if(newDragDestinationLayer != _prevDragDestinationLayer)
        {
            NSDragOperation result = [newDragDestinationLayer draggingEntered:sender];
            if(result != NSDragOperationNone)
            {
                [_dragIndicationLayer setHidden:YES];
                _dragDestinationLayer = newDragDestinationLayer;
                return result;
            }
            _prevDragDestinationLayer = newDragDestinationLayer;
        }
    }
    return NSDragOperationNone;
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    _lastDragOperation = NSDragOperationNone;
    
    NSDragOperation result = [self OE_dragOperationForDestinationLayer:sender];
    if(result != NSDragOperationNone) return result;
    
    // The delegate has to be able to validate and accept drops, if it can't do then then there is no need to drag anything around
    if(_delegateHas.validateDrop && _delegateHas.acceptDrop)
    {
        _lastDragOperation = [_delegate gridView:self validateDrop:sender];
        [_dragIndicationLayer setHidden:(_lastDragOperation == NSDragOperationNone)];

        if(![_dragIndicationLayer isHidden])
            [_rootLayer setNeedsLayout];
    }
    
    return _lastDragOperation;
}

- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender
{
    const NSPoint pointInView = [self convertPoint:[sender draggingLocation] fromView:nil];
    BOOL hadDragDestinationLayer = (_dragDestinationLayer != nil);
    
    if(_dragDestinationLayer)
    {
        CGPoint pointInSuperlayer = [_rootLayer convertPoint:pointInView toLayer:[_dragDestinationLayer superlayer]];
        if([_dragDestinationLayer hitTest:pointInSuperlayer] == _dragDestinationLayer)
        {
            NSDragOperation result = [_dragDestinationLayer draggingUpdated:sender];
            if(result != NSDragOperationNone) return result;
        }
        [_dragDestinationLayer draggingExited:sender];
        _dragDestinationLayer = nil;
    }
    
    NSDragOperation result = [self OE_dragOperationForDestinationLayer:sender];
    if(result != NSDragOperationNone) return result;
    
    if(hadDragDestinationLayer)
    {
        // If we were targeting the drag destination layer and now we are not, then its the same as running draggingEntered:
        _lastDragOperation = [self draggingEntered:sender];
    }
    else if(_delegateHas.draggingUpdated)
    {
        _lastDragOperation = [_delegate gridView:self draggingUpdated:sender];
        [_dragIndicationLayer setHidden:(_lastDragOperation == NSDragOperationNone)];

        if(![_dragIndicationLayer isHidden]) [_rootLayer setNeedsLayout];
    }
    
    return _lastDragOperation;
}

- (void)draggingExited:(id<NSDraggingInfo>)sender
{
    if(_dragDestinationLayer)
    {
        [_dragDestinationLayer draggingExited:sender];
        _dragDestinationLayer = nil;
    }
    else
        [_dragIndicationLayer setHidden:YES];
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    [_dragIndicationLayer setHidden:YES];

    if(_dragDestinationLayer != nil)
        return [_dragDestinationLayer performDragOperation:sender];
    else
        return _delegateHas.acceptDrop && [_delegate gridView:self acceptDrop:sender];
}

#pragma mark -
#pragma mark NSDraggingSource

- (NSDragOperation)draggingSession:(NSDraggingSession *)session sourceOperationMaskForDraggingContext:(NSDraggingContext)context
{
    return context == NSDraggingContextWithinApplication ? NSDragOperationCopy : NSDragOperationNone;
}

- (void)draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
    _draggingSession = nil;
}

#pragma mark -
#pragma mark Properties

- (void)setForegroundLayer:(CALayer *)foregroundLayer
{
    if(_foregroundLayer == foregroundLayer) return;
    
    [_foregroundLayer removeFromSuperlayer];
    _foregroundLayer = foregroundLayer;
    
    if(_foregroundLayer) [self OE_reorderSublayers];
}

- (CALayer *)foregroundLayer
{
    return _foregroundLayer;
}

- (void)setBackgroundLayer:(CALayer *)backgroundLayer
{
    if(_backgroundLayer == backgroundLayer) return;

    [_backgroundLayer removeFromSuperlayer];
    _backgroundLayer = backgroundLayer;

    if(_backgroundLayer) [self OE_reorderSublayers];
}

- (CALayer *)backgroundLayer
{
    return _backgroundLayer;
}

- (void)setMinimumColumnSpacing:(CGFloat)minimumColumnSpacing
{
    if(_minimumColumnSpacing == minimumColumnSpacing) return;
    
    _minimumColumnSpacing = minimumColumnSpacing;
    [self OE_calculateCachedValuesAndQueryForDataChanges:NO];
}

- (CGFloat)minimumColumnSpacing
{
    return _minimumColumnSpacing;
}

- (void)setRowSpacing:(CGFloat)rowSpacing
{
    if(_rowSpacing == rowSpacing) return;
    
    _rowSpacing = rowSpacing;
    [self OE_calculateCachedValuesAndQueryForDataChanges:NO];
}

- (CGFloat)rowSpacing
{
    return _rowSpacing;
}

- (void)setItemSize:(NSSize)itemSize
{
    if(NSEqualSizes(_cellSize, itemSize)) return;
    
    [self OE_cancelFieldEditor];
    
    _cellSize = itemSize;
    [[self enclosingScrollView] flashScrollers];
    [self OE_calculateCachedValuesAndQueryForDataChanges:NO];
}

- (NSSize)itemSize
{
    return _cellSize;
}

- (void)setDataSource:(id<OEGridViewDataSource>)dataSource
{
    if(_dataSource != dataSource)
    {
        _dataSource = dataSource;
        _dataSourceHas.viewForNoItemsInGridView           = [_dataSource respondsToSelector:@selector(viewForNoItemsInGridView:)];
        _dataSourceHas.willBeginEditingCellForItemAtIndex = [_dataSource respondsToSelector:@selector(gridView:willBeginEditingCellForItemAtIndex:)];
        _dataSourceHas.didEndEditingCellForItemAtIndex    = [_dataSource respondsToSelector:@selector(gridView:didEndEditingCellForItemAtIndex:)];
        _dataSourceHas.pasteboardWriterForIndex           = [_dataSource respondsToSelector:@selector(gridView:pasteboardWriterForIndex:)];
        _dataSourceHas.menuForItemsAtIndexes                 = [_dataSource respondsToSelector:@selector(gridView:menuForItemsAtIndexes:)];
        [self OE_setNeedsReloadData];
    }
}

- (void)setDelegate:(id<OEGridViewDelegate>)delegate
{
    if(_delegate != delegate)
    {
        _delegate = delegate;
        _delegateHas.selectionChanged                = [_delegate respondsToSelector:@selector(selectionChangedInGridView:)];
        _delegateHas.doubleClickedCellForItemAtIndex = [_delegate respondsToSelector:@selector(gridView:doubleClickedCellForItemAtIndex:)];
        _delegateHas.validateDrop                    = [_delegate respondsToSelector:@selector(gridView:validateDrop:)];
        _delegateHas.draggingUpdated                 = [_delegate respondsToSelector:@selector(gridView:draggingUpdated:)];
        _delegateHas.acceptDrop                      = [_delegate respondsToSelector:@selector(gridView:acceptDrop:)];
    }
}

- (void)setSelectionIndexes:(NSIndexSet *)selectionIndexes
{
    if([_selectionIndexes isEqualToIndexSet:selectionIndexes]) return;
    
    [_selectionIndexes removeAllIndexes];
    [_selectionIndexes addIndexes:selectionIndexes];
    [_visibleCells enumerateObjectsUsingBlock:^(id obj, BOOL *stop)
     {
         [obj setSelected:[_selectionIndexes containsIndex:[obj _index]] animated:![CATransaction disableActions]];
     }];

    if(_delegateHas.selectionChanged) [_delegate selectionChangedInGridView:self];
}

- (NSIndexSet *)selectionIndexes
{
    return [_selectionIndexes copy];
}

@end

@implementation OEGridView (OEGridViewCell)

- (void)OE_willBeginEditingCell:(OEGridViewCell *)cell
{
    if(_dataSourceHas.willBeginEditingCellForItemAtIndex) [_dataSource gridView:self willBeginEditingCellForItemAtIndex:[cell _index]];
}

- (void)OE_didEndEditingCell:(OEGridViewCell *)cell
{
    if(_dataSourceHas.didEndEditingCellForItemAtIndex) [_dataSource gridView:self didEndEditingCellForItemAtIndex:[cell _index]];
}

@end
