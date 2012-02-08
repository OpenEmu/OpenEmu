//
//  IKSGridLayoutManager.h
//  Sonora
//
//  Created by Indragie Karunaratne on 10-11-16.
//  Copyright 2010 PCWiz Computer. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>

/**
 Lays out sublayers in a grid style formation
 */

struct IKSGridLayoutState 
{
    NSInteger firstVisibleRow;
    NSUInteger rows;
    NSUInteger columns;
};
typedef struct IKSGridLayoutState IKSGridLayoutState;


@protocol IKSGridLayoutManagerDelegate;
@class IKSGridView;
@interface IKSGridLayoutManager : NSObject 
{
    BOOL reset;
    /**
     The GCD queue that is used to render all the layers
     */
    dispatch_queue_t renderQueue;
    
    IKSGridLayoutState currentGridState;
}

@property (nonatomic, strong) NSMutableArray *itemLayers;

@property (nonatomic, weak) IKSGridView *gridView;
@property (nonatomic, weak) id<IKSGridLayoutManagerDelegate> delegate;
/**
 Minimum vertical and horizontap spacing between layers
 Default: {10.0, 10.0}
 */
@property (nonatomic, assign) NSSize minimumSpacing;
/**
 Size of each item
 Default: {100.0, 100.0}
 */
@property (nonatomic, assign) NSSize sublayerSize;
/**
 Visible space (used by IKSGridView, optional)
 */
@property (nonatomic, assign) CGRect visibleRect;
/**
 Initializes and returns a new autoreleased layout manager object
 @returns a new layout manager
 */
+ (IKSGridLayoutManager*)layoutManager;
- (void)reset;
@end

/**
 The delegate protocol for IKSGridLayoutManager
 */
@protocol IKSGridLayoutManagerDelegate <NSObject>
/**
 Called by the layout manager when the total content height changes
 @param manager the layout manager
 @param height the new content height
 */
- (void)layoutManager:(IKSGridLayoutManager*)manager contentHeightChanged:(CGFloat)height;
@end