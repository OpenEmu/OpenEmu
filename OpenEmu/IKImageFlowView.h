/*
 *  IKImageFlowView.h
 *  ShopManagerX
 *
 *  Created by David Gohara on 2/22/08.
 *  Copyright 2008 SmackFu-Master. All rights reserved.
 *
 */

#import <Cocoa/Cocoa.h>

@interface IKImageFlowView : NSOpenGLView
{
    id _dataSource;
    id _dragDestinationDelegate;
    id _delegate;
    void *_reserved;
}

+ (id)pixelFormat;
+ (BOOL)flowViewIsSupportedByCurrentHardware;
+ (void)initialize;
+ (void)setImportAnimationStyle:(unsigned int)fp8;
- (void)_setDefaultTextAttributes;
- (void)_ikCommonInit;
- (id)initWithFrame:(struct _NSRect)fp8;
- (void)dealloc;
- (void)finalize;
- (void)setValue:(id)fp8 forUndefinedKey:(id)fp12;
- (id)valueForUndefinedKey:(id)fp8;
- (id)allocateNewCell;
- (void)dataSourceDidChange;
- (void)_reloadCellDataAtIndex:(int)fp8;
- (void)reloadCellDataAtIndex:(int)fp8;
- (void)reloadAllCellsData;
- (void)reloadData;
- (id)loadCellAtIndex:(int)fp8;
- (void)didStabilize;
- (BOOL)isAnimating;
- (void)setAnimationsMask:(unsigned int)fp8;
- (unsigned int)animationsMask;
- (void)_cellFinishedImportAnimation:(id)fp8;
- (BOOL)itemAtIndexIsLoaded:(unsigned int)fp8;
- (void)keyWindowChanged:(id)fp8;
- (void)setSelectedIndex:(unsigned int)fp8;
- (BOOL)hitTestWithImage:(id)fp8 x:(float)fp12 y:(float)fp16;
- (unsigned int)cellIndexAtLocation:(struct _NSPoint)fp8;
- (void)_adjustScroller;
- (void)resetCursorRects;
- (void)frameDidChange:(id)fp8;
- (void)invalidateLayout;
- (float)offset;
- (int)cellIndexAtPosition:(float)fp8;
- (int)heightOfInfoSpace;
- (int)countOfVisibleCellsOnEachSide;
- (struct _NSRange)rangeOfVisibleIndexes;
- (struct _NSRange)rangeOfVisibleIndexesAtSelection;
- (id)visibleCellIndexesAtSelection;
- (id)visibleCellIndexes;
- (void)flipCellsWithOldSelectedIndex:(unsigned int)fp8 newSelectedIndex:(unsigned int)fp12;
- (void)flowLayout:(struct _NSRange)fp8;
- (void)zoomOnSelectedLayerLayout:(struct _NSRange)fp8;
- (void)updateLayoutInRange:(struct _NSRange)fp8;
- (void)updateLayout;
- (struct _NSRect)titleFrame;
- (struct _NSRect)subtitleFrame;
- (struct _NSRect)splitterFrame;
- (double)_viewAspectRatio;
- (double)_zScreen;
- (struct _NSSize)imageRenderedSize;
- (struct _NSRect)selectedImageFrame;
- (double)_computeCameraDZ;
- (double)cameraDZ;
- (double)_computeCameraDY;
- (double)cameraDY;
- (float)convertPixelUnitTo3DUnit:(float)fp8;
- (double)alignOnPixelValue;
- (BOOL)updatesCGSurfaceOnDrawRect;
- (void)setUpdatesCGSurfaceOnDrawRect:(BOOL)fp8;
- (BOOL)showSplitter;
- (void)setShowSplitter:(BOOL)fp8;
- (id)delegate;
- (void)setDelegate:(id)fp8;
- (id)dataSource;
- (void)setDataSource:(id)fp8;
- (void)setZoomOnSelectedLayer:(BOOL)fp8;
- (BOOL)zoomOnSelectedLayer;
- (unsigned int)itemsCount;
- (id)cells;
- (unsigned int)selectedIndex;
- (unsigned int)focusedIndex;
- (id)backgroundColor;
- (void)_setBackgroundColorWithRed:(float)fp8 green:(float)fp12 blue:(float)fp16 alpha:(float)fp20;
- (BOOL)backgroundIsLight;
- (BOOL)backgroundIsBlack;
- (BOOL)_convertColor:(id)fp8 toRed:(float *)fp12 green:(float *)fp16 blue:(float *)fp20 alpha:(float *)fp24;
- (void)_getBackgroundRed:(float *)fp8 green:(float *)fp12 blue:(float *)fp16 alpha:(float *)fp20;
- (void)setBackgroundColor:(id)fp8;
- (id)cellBackgroundColor;
- (void)setCellBackgroundColor:(id)fp8;
- (id)cellBorderColor;
- (void)setCellBorderColor:(id)fp8;
- (float)imageAspectRatio;
- (void)setImageAspectRatio:(float)fp8;
- (float)scaleFactor;
- (id)cacheManager;
- (BOOL)cellsAlignOnBaseline;
- (void)setCellsAlignOnBaseline:(BOOL)fp8;
- (void)startInlinePreview;
- (void)stopInlinePreview;
- (void)inlinePreviewDidRenderImage:(void *)fp8;
- (id)thumbnailImageAtIndex:(int)fp8;
- (id)previewImageAtIndex:(int)fp8;
- (void)initRenderingContext;
- (void *)fogShader;
- (void)renewGState;
- (void)setHidden:(BOOL)fp8;
- (id)renderer;
- (void)_setAutoscalesBoundsToPixelUnits:(BOOL)fp8;
- (void)setCacheManager:(id)fp8;
- (id)imageFlowContext;
- (void)setImageFlowContext:(id)fp8;
- (void)__ikSetupGLContext:(id)fp8;
- (id)openGLContext;
- (void)setOpenGLContext:(id)fp8;
- (void)_cacheWasFlushed:(id)fp8;
- (float)fogAtLocation:(float)fp8;
- (struct _NSRect)clampedBounds;
- (struct _NSRect)clampedFrame;
- (void)drawVisibleCells:(struct _NSRect)fp8;
- (void)drawBackground;
- (void)drawTitle;
- (BOOL)installViewport;
- (void)setupGLState;
- (void)installPerspetiveViewportForPicking:(BOOL)fp8 location:(struct _NSPoint)fp12;
- (void)drawFocusRing;
- (BOOL)drawWithCurrentRendererInRect:(struct _NSRect)fp8;
- (void)__copyPixels:(void *)fp8 withSize:(struct _NSSize)fp12 toCurrentFocusedViewAtPoint:(struct _NSPoint)fp20;
- (void)__copyGLToCurrentFocusedView;
- (BOOL)_createPBuffer;
- (void)_deletePBUffer;
- (BOOL)_installPBuffer;
- (void)_copyPBufferToCGSurface;
- (void)drawRect:(struct _NSRect)fp8;

@end
