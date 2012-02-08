//
//  IKSGridView.h
//  Sonora
//
//  Created by Indragie Karunaratne on 10-11-14.
//  Copyright 2010. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import "IKSGridLayoutManager.h"

/**
 An NSView subclass that displays a grid of CALayers (similar to NSCollectionView, but using CALayers instead of NSViews). The use of CALayers results in the ability to display many layers on screen at a time with no visible performance hit, and of course all the animation capabilities of the Core Animation framework. Features:
 
 - Renders layers using a background queue (Grand Central Dispatch)
 - Only renders layers that are visible to the user to improve performance (similar to what UITableView does)
 - Supports keyboard controls
 - Supports multitouch pinch-to-zoom on supported Apple trackpads & mice
 - Easily customizable drawing using subclasses of IKSGridItemLayer
 - Automatic grid layout with flexible horizontal and vertical spacing
 */

@class IKSGridLayoutManager, IKSGridItemLayer;
@protocol IKSGridViewDelegate;
@protocol IKSGridViewDataSource;

@class OEGridViewFieldEditor;
@interface IKSGridView : NSView <IKSGridLayoutManagerDelegate> {
    /**
     The layout manager used to layout the grid
     */
    IKSGridLayoutManager *layoutManager;
    /**
     Grid layer
     */
    CALayer *gridLayer;
    
    OEGridViewFieldEditor *fieldEditor;
    
    /**
     Keeps track of the point where the mouse first clicked (used for calculating the frame of the selection rectangle)
     */
    NSPoint mouseDownPoint;
    
    BOOL noItems;
}

- (IBAction)copy:(id)sender;
#pragma mark -
- (void)reloadData;
#pragma mark -
#pragma mark decoration views
- (void)layoutDecorationViews;
- (void)addBackgroundLayer:(CALayer*)newBackgroundLayer;
- (void)addForegroundLayer:(CALayer*)newForegroundLayer;
#pragma mark -
- (void)windowChanged:(id)sender;
#pragma mark -
- (BOOL)isItemSelectedAtIndex:(NSInteger)index;
#pragma mark -
@property (readwrite, unsafe_unretained) id target;
@property (readwrite) SEL doubleAction;
@property (readwrite, weak) Class cellClass;


@property (readwrite, strong) NSImage *draggedImage;

@property (nonatomic, strong) NSTimer *autoscrollTimer;
@property (nonatomic, strong) NSEvent *lastEvent;

- (OEGridViewFieldEditor*)fieldEditorForFrame:(NSRect)frame ofLayer:(id)layer;
/**
 Delegate of the grid view (an object conforming to the IKSGridViewDelegateProtocol
 */
@property (nonatomic, unsafe_unretained) IBOutlet id<IKSGridViewDelegate> delegate;
/**
 Tracking area for capturing mouseMoved events
 */
@property (nonatomic, strong) NSTrackingArea *trackingArea;
/**
 Layer that the mouse is currently hovering over
 */
@property (nonatomic, strong) CALayer *dragIndicationLayer;
@property (nonatomic, strong) IKSGridItemLayer *eventLayer;
@property (nonatomic, strong) IKSGridItemLayer *draggedLayer;
/** 
 Minimum horizontal and vertical spacing between layers
 Default: {10.0, 10.0} 
 */
@property (nonatomic, assign) NSSize minimumSpacing;


@property (nonatomic, strong) id<IKSGridViewDataSource> dataSource;
/** 
 Size of each item
 Default: {100.0, 100.0} 
 */
@property (nonatomic, assign) NSSize itemSize;
/**
 Background color of the view 
 */
@property (nonatomic, strong) NSColor *backgroundColor;

/**
 Sort descriptors used to sort the items
 */
@property (nonatomic, strong) NSArray *sortDescriptors;
/**
 Whether the user is allowed to select items in the grid or not
 */
@property (nonatomic, assign, getter=isSelectable) BOOL selectable;
/**
 Enable/disable layout manager
 */
@property (nonatomic, assign) BOOL layoutEnabled;
/**
 Whether the user can select more than one item at a time
 */
@property (nonatomic, assign) BOOL allowsMultipleSelection;
/**
 Maximum and minimum layer sizes (for resizing)
 Default minimum: {125.0, 125.0}
 Default maximum: {256.0, 256.0}
 */
@property (nonatomic, assign) NSSize minimumItemSize, maximumItemSize;
/**
 Selected indexes
 **/
@property (nonatomic, strong) NSMutableIndexSet *selectedIndexes;

/**
 Private properties
 */
@property (nonatomic, strong) CALayer *foregroundLayer, *backgroundLayer;
/**
 selectionLayer - layer used to draw the selection rectangle 
 */
@property (nonatomic, strong) CALayer *gridLayer, *selectionLayer;
@property (readonly) NSMutableArray *selectedLayers;
@end

/**
 The delegate protocol of IKSGridView. This protocol contains one method which is called when the selection has changed.
 Note: If the "content" property of the grid view is bound to an NSArrayController, the array controller's selectedObjects will be automatically updated, which means that this delegate method is not required.
 */

@protocol IKSGridViewDelegate <NSObject>
/**
 Called when the selected items in the grid have changed
 @param gridView the grid view
 @param selectedItems the item(s) that have been selected
 */
- (void)gridView:(IKSGridView*)gridView selectionChanged:(NSArray*)selectedItems;
- (void)gridView:(IKSGridView*)gridView itemsMagnifiedToSize:(NSSize)newSize;

@optional
- (NSDragOperation)gridView:(IKSGridView*)gridView validateDrop:(id<NSDraggingInfo>)draggingInfo;
- (BOOL)gridView:(IKSGridView*)gridView acceptDrop:(id<NSDraggingInfo>)draggingInfo;
- (void)gridView:(IKSGridView *)gridView updateDraggingItemsForDrag:(id<NSDraggingInfo>)draggingInfo;
@end
