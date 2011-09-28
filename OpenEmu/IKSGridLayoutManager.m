//
//  IKSGridLayoutManager.m
//  Sonora
//
//  Created by Indragie Karunaratne on 10-11-16.
//  Copyright 2010 PCWiz Computer. All rights reserved.
//

#import "IKSGridLayoutManager.h"
#import "IKSGridItemLayer.h"

@implementation IKSGridLayoutManager
@synthesize delegate, gridView, minimumSpacing, sublayerSize, visibleRect, itemLayers;

#pragma mark -
#pragma mark Initialization

+ (IKSGridLayoutManager*)layoutManager{
	return [[[self alloc] init] autorelease];
}

- (id)init{
	if ((self = [super init])) {
		// Set default values
		self.sublayerSize = NSMakeSize(100.0, 100.0);
		self.minimumSpacing = NSMakeSize(10.0, 10.0);
		
		self.itemLayers = [NSMutableArray array];
		// self.visibleIndexes = NSMakeRange(0, 0);
		
		// Create the queue that is used to render the layers
		renderQueue = dispatch_queue_create("com.iksgridview.renderlayers", NULL);
		dispatch_queue_t high = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);
		dispatch_set_target_queue(renderQueue, high);

	}
	return self;
}

#pragma mark -
#pragma mark Memory Management

- (void)dealloc{
	self.gridView = nil;
	self.itemLayers = nil;
	
	dispatch_release(renderQueue);
	[super dealloc];
}

#pragma mark -
#pragma mark Layout
- (void)layoutSublayersOfLayer:(CALayer *)layer{
	// check if manager is associated with a grid view, and the gridview has a datasource
	if(!self.gridView || !self.gridView.dataSource) return;
	
	// get number of items
	NSUInteger numberOfItems = [self.gridView.dataSource numberOfItemsInGridView:self.gridView];

	// Calculate size of item
	CGSize paddedSize = CGSizeMake(self.sublayerSize.width + self.minimumSpacing.width, self.sublayerSize.height + self.minimumSpacing.height);
	if (CGRectIsNull(self.visibleRect)) { self.visibleRect = layer.bounds; }
	
	//
	// Calculate the number of rows and columns
	// Columns are calculated by dividing the total width (subtracting the spacing for the far edge) divided by the padded size
	// We make sure that there is at least one column, otherwise it will attempt to divide by zero (EXC_ARITHMETIC)
	// Calculate rows by taking the total number of layers / number of columns and rounding it to the highest integer
	//
	NSInteger columns = floor((self.visibleRect.size.width - self.minimumSpacing.width) / paddedSize.width);
	if (columns < 1) { columns = 1; } // Make sure there's at least one column, otherwise it will divide by zero
	NSUInteger rows = ceil((CGFloat)numberOfItems / (CGFloat)columns);
	
	// Calculate contentHeight
	CGFloat contentHeight = rows * paddedSize.height + self.minimumSpacing.height;
	
	//
	// When there is only one row, we use a different layout method (to prevent it from being centered)
	// Calculate the extra horizontal padding using the remaining content space
	// The vertical padding is equivalent to the horizontal padding
	// Calculate new padded size using the extra space
	//
	CGFloat contentWidth = paddedSize.width*columns + self.minimumSpacing.width;
	
	CGFloat additionalWidth = (self.visibleRect.size.width - contentWidth) / columns;
	paddedSize.width += additionalWidth;

	// calc rows per "page":  + 1 half visible on top + 1 half visible on bottom
	NSInteger visibleRows = (self.visibleRect.size.height / paddedSize.height)+ 1 + 1;

	
	// calc first visible row
	NSInteger firstVisibleRow = ((self.visibleRect.origin.y) / paddedSize.height);	
	
	// set object values for visible layers
	if(!reset && currentGridState.columns == columns){
		if(currentGridState.rows == visibleRows){
			if(currentGridState.firstVisibleRow != firstVisibleRow){
				// do some magic to reduce drawing effort:				
				NSInteger rowMovement = firstVisibleRow-currentGridState.firstVisibleRow;
				NSInteger affectedItems = rowMovement*columns;
				if(rowMovement > 0 && affectedItems < [self.itemLayers count]){
					NSIndexSet* indexSet = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, affectedItems)];
					NSArray* recylcedLayers = [self.itemLayers objectsAtIndexes:indexSet];
					[self.itemLayers removeObjectsAtIndexes:indexSet];
					[self.itemLayers addObjectsFromArray:recylcedLayers];
				} else if(rowMovement < 0 && affectedItems > -1*[self.itemLayers count]){
					NSIndexSet* indexSet = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange([self.itemLayers count]+affectedItems, -affectedItems)];
					NSArray* recylcedLayers = [self.itemLayers objectsAtIndexes:indexSet];
					[self.itemLayers removeObjectsAtIndexes:indexSet];
					indexSet = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, -affectedItems)];
					[self.itemLayers insertObjects:recylcedLayers atIndexes:indexSet];
				}
			}
		}
	} else {
		NSUInteger requiredLayers = visibleRows*columns;
		while(requiredLayers < [self.itemLayers count]){			
			[[self.itemLayers objectAtIndex:0] removeFromSuperlayer];
			[self.itemLayers removeObjectAtIndex:0];
		}
		while(requiredLayers > [self.itemLayers count]){
			IKSGridItemLayer* itemLayer = [self.gridView.cellClass layer];
			itemLayer.gridView = self.gridView;
			itemLayer.representedIndex = -1;
			[self.itemLayers addObject:itemLayer];
			[layer addSublayer:itemLayer];
		}
	}
	
	// update state
	currentGridState.rows = visibleRows;
	currentGridState.columns = columns;
	
	currentGridState.firstVisibleRow = firstVisibleRow;
	
	NSUInteger currentLayerIndex = 0, j, k;
	NSUInteger currentItemIndex;
	IKSGridItemLayer *currentLayer = nil;
	
	currentItemIndex = firstVisibleRow*columns;
		
	// iterate though columns and rows	
	for (j = firstVisibleRow; j < firstVisibleRow+visibleRows; j++){
        for (k = 0; k < columns; k++){
			
			// self.itemLayers can change during this method, if layoutSublayersOfLayer: is called again before it's finishes
			// if that case occures, we cancel
			if(currentLayerIndex >= [self.itemLayers count]){ return; }
			
			// get the layer we want to update
			currentLayer = [self.itemLayers objectAtIndex:currentLayerIndex];
			
			// if there is no object associated with the layer
			if(currentItemIndex >= numberOfItems){
				currentLayer.representedIndex = -1;
				
				// layer gets hidden
				[CATransaction begin];
				[CATransaction setDisableActions:YES];
				currentLayer.hidden = YES;
				[CATransaction commit];
				
				currentItemIndex++;
				currentLayerIndex++;

				// skip the rest
				continue;
			}
			
			// get the object the layer is supposed to represent
			
			// we need to redraw if layers current object value and the one it's supposed to have differ
// TODO: check implementation
	//		BOOL redrawLayer = currentLayer.representedObject != objectValue;
			BOOL redrawLayer = currentLayer.representedIndex != currentItemIndex;
			
			if(redrawLayer){
				// tell the layer that it's values are about to change (layer should disable animations, hide parts that might take long to update)
				[currentLayer beginValueChange];
			}
			
			// set selection for layer
			currentLayer.selected = [self.gridView isItemSelectedAtIndex:currentItemIndex];
			
			// calculate new frame size
			CGRect newFrame;
			newFrame.size = CGSizeMake(self.sublayerSize.width, self.sublayerSize.height);
			
			// redraw layer if we assign new size
			redrawLayer |= (!CGSizeEqualToSize(currentLayer.frame.size, newFrame.size));
			
			// calculate origin for current layer, round it to prevent subpixel stuff and have nice border lines
			CGFloat currentOriginX = roundf( additionalWidth/2+ self.minimumSpacing.width+k*paddedSize.width );
			CGFloat currentOriginY = roundf( self.minimumSpacing.height+j*paddedSize.height );
			
			newFrame.origin = CGPointMake(currentOriginX, currentOriginY);
			
			// tell the layer which item it currently represents			
			// TODO: check implementation
			currentLayer.representedIndex = currentItemIndex;
//			currentLayer.representedObject = objectValue;
			
			
			[CATransaction begin];
			[CATransaction setDisableActions:YES];
			currentLayer.frame = newFrame;
			currentLayer.hidden = NO;
			[CATransaction commit];

			//
			// Rendering
			// If there are multiple layers, they are rendered on a separate GCD queue which prevents the main application from stalling
			// If there is only one layer, it is rendered on the main thread to avoid lag
			//
			if (redrawLayer) {
				dispatch_async(renderQueue, ^{
					
						[CATransaction begin];
						[CATransaction setDisableActions:YES];
						[currentLayer reloadData];
						[CATransaction commit];
						[currentLayer endValueChange];
				});
#warning Imortant to fix this sooon!!!
#warning fixed already??????
				// TODO: check why coredata and thumbnails don't work in an async queue
				//	);
			}
			
			currentItemIndex++;
            currentLayerIndex++;
		}
	}
	
	[self.delegate layoutManager:self contentHeightChanged:contentHeight];
	[self.gridView layoutDecorationViews];
	
	reset = NO;
}
- (void)reset{	
	while(gridView.gridLayer.sublayers.count)
		[gridView.gridLayer.sublayers.lastObject removeFromSuperlayer];
	self.itemLayers  = [NSMutableArray array];
	
	reset = YES;
}
@end
