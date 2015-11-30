
#import <Foundation/Foundation.h>

@protocol IKRenderer <NSObject>
- (void)uninstallClipRect;
- (void)installClipRect:(struct CGRect)arg1;
- (void)clearViewport:(struct CGRect)arg1;
- (BOOL)renderBezelGroupWithPoints:(struct CGPoint *)arg1 count:(int)arg2 radius:(float)arg3 strokeColor:(float *)arg4 fillColor:(float *)arg5 lineWidth:(int)arg6;
- (int)rendererType;
- (void)setColorRed:(float)arg1 Green:(float)arg2 Blue:(float)arg3 Alpha:(float)arg4;
- (void)drawRectShadow:(struct CGRect)arg1 withAlpha:(float)arg2;
- (void)drawText:(id)arg1 inRect:(struct CGRect)arg2 withAttributes:(id)arg3 withAlpha:(float)arg4;
- (void)drawImage:(id)arg1 inRect:(struct CGRect)arg2 fromRect:(struct CGRect)arg3 alpha:(float)arg4;
- (void)drawRect:(struct CGRect)arg1 withLineWidth:(float)arg2;
- (void)drawLineFromPoint:(struct CGPoint)arg1 toPoint:(struct CGPoint)arg2;
- (void)drawPlaceHolderWithRect:(struct CGRect)arg1 withAlpha:(float)arg2;
- (BOOL)drawRoundedRect:(struct CGRect)arg1 radius:(float)arg2 strokeColor:(float *)arg3 fillColor:(float *)arg4 lineWidth:(int)arg5;
- (void)drawRoundedRect:(struct CGRect)arg1 radius:(float)arg2 lineWidth:(float)arg3 cacheIt:(BOOL)arg4;
- (void)fillRoundedRect:(struct CGRect)arg1 radius:(float)arg2 cacheIt:(BOOL)arg3;
- (void)fillRect:(struct CGRect)arg1;
- (void)fillGradientInRect:(struct CGRect)arg1 bottomColor:(id)arg2 topColor:(id)arg3;
- (void)endDrawing;
- (void)flushRenderer;
- (void)flushTextRenderer;
- (void)beginDrawingInView:(id)arg1;
@property unsigned long long scaleFactor;
- (void)setupViewportWithView:(id)arg1;
- (void)resetOffset;
- (id)textRenderer;
- (NSPoint)offset;
@property BOOL enableSubpixelAntialiasing;
- (void)setAutoInstallBlendMode:(BOOL)arg1;
- (BOOL)autoInstallBlendMode;
- (void)setEnableMagFilter:(BOOL)arg1;
- (BOOL)enableMagFilter;
- (void)emptyCaches;
- (void)dealloc;
- (id)init;
@end

@interface IKCGRenderer : NSObject <IKRenderer>
{
    struct CGContext *_ctx;
    float _currentAlpha;
    BOOL _smoothFonts;
    unsigned long long _currentScaleFactor;
}

- (void)uninstallClipRect;
- (void)installClipRect:(struct CGRect)arg1;
- (void)fillGradientInRect:(struct CGRect)arg1 bottomColor:(id)arg2 topColor:(id)arg3;
- (void)flushTextRenderer;
- (int)rendererType;
- (void)clearViewport:(struct CGRect)arg1;
- (void)setColorRed:(float)arg1 Green:(float)arg2 Blue:(float)arg3 Alpha:(float)arg4;
- (BOOL)renderBezelGroupWithPoints:(struct CGPoint *)arg1 count:(int)arg2 radius:(float)arg3 strokeColor:(float *)arg4 fillColor:(float *)arg5 lineWidth:(int)arg6;
- (void)drawRectShadow:(struct CGRect)arg1 withAlpha:(float)arg2;
- (void)drawText:(id)arg1 inRect:(struct CGRect)arg2 withAttributes:(id)arg3 withAlpha:(float)arg4;
- (void)drawImage:(id)arg1 inRect:(struct CGRect)arg2 fromRect:(struct CGRect)arg3 alpha:(float)arg4;
- (void)drawRect:(struct CGRect)arg1 withLineWidth:(float)arg2;
- (void)drawPlaceHolderWithRect:(struct CGRect)arg1 withAlpha:(float)arg2;
- (BOOL)drawRoundedRect:(struct CGRect)arg1 radius:(float)arg2 strokeColor:(float *)arg3 fillColor:(float *)arg4 lineWidth:(int)arg5;
- (void)fillRoundedRect:(struct CGRect)arg1 radius:(float)arg2 cacheIt:(BOOL)arg3;
- (void)drawRoundedRect:(struct CGRect)arg1 radius:(float)arg2 lineWidth:(float)arg3 cacheIt:(BOOL)arg4;
- (void)_drawRoundedRect:(struct CGRect)arg1 radius:(float)arg2 lineWidth:(float)arg3 drawingMode:(int)arg4;
- (void)drawLineFromPoint:(struct CGPoint)arg1 toPoint:(struct CGPoint)arg2;
- (void)fillRect:(struct CGRect)arg1;
- (void)emptyCaches;
- (void)flushRenderer;
- (void)setupViewportWithView:(id)arg1;
- (void)endDrawing;
- (void)beginDrawingInView:(id)arg1;
- (struct CGContext *)ctx;
- (void)setShouldSmoothFonts:(BOOL)arg1;
- (void)setContext:(struct CGContext *)arg1;
- (void)dealloc;
- (id)init;

@end

@interface IKOpenGLRenderer : NSObject <IKRenderer>
{
    struct __CFDictionary *_textRenderers;
    float _color[4];
    void *_programObject;
    BOOL _autoInstallBlendMode;
    BOOL _enableMagFilter;
    BOOL _perspectivePushed;
    unsigned long long _currentScaleFactor;
    BOOL _enableSubpixelAntialiasing;
}

@property BOOL perspectivePushed; // @synthesize perspectivePushed=_perspectivePushed;
- (void)uninstallClipRect;
- (void)installClipRect:(struct CGRect)arg1;
- (void)clearViewport:(struct CGRect)arg1;
- (BOOL)renderBezelGroupWithPoints:(struct CGPoint *)arg1 count:(int)arg2 radius:(float)arg3 strokeColor:(float *)arg4 fillColor:(float *)arg5 lineWidth:(int)arg6;
- (int)rendererType;
- (void)setColorRed:(float)arg1 Green:(float)arg2 Blue:(float)arg3 Alpha:(float)arg4;
- (void)drawRectShadow:(struct CGRect)arg1 withAlpha:(float)arg2;
- (void)drawText:(id)arg1 inRect:(struct CGRect)arg2 withAttributes:(id)arg3 withAlpha:(float)arg4;
- (void)drawImage:(id)arg1 inRect:(struct CGRect)arg2 fromRect:(struct CGRect)arg3 alpha:(float)arg4;
- (void)drawRect:(struct CGRect)arg1 withLineWidth:(float)arg2;
- (void)drawLineFromPoint:(struct CGPoint)arg1 toPoint:(struct CGPoint)arg2;
- (void)drawPlaceHolderWithRect:(struct CGRect)arg1 withAlpha:(float)arg2;
- (BOOL)drawRoundedRect:(struct CGRect)arg1 radius:(float)arg2 strokeColor:(float *)arg3 fillColor:(float *)arg4 lineWidth:(int)arg5;
- (void)drawRoundedRect:(struct CGRect)arg1 radius:(float)arg2 lineWidth:(float)arg3 cacheIt:(BOOL)arg4;
- (void)fillRoundedRect:(struct CGRect)arg1 radius:(float)arg2 cacheIt:(BOOL)arg3;
- (void)fillRect:(struct CGRect)arg1;
- (void)fillGradientInRect:(struct CGRect)arg1 bottomColor:(id)arg2 topColor:(id)arg3;
- (void)endDrawing;
- (void)flushRenderer;
- (void)flushTextRenderer;
- (void)beginDrawingInView:(id)arg1;
@property unsigned long long scaleFactor;
- (void)setupViewportWithView:(id)arg1;
- (void)resetOffset;
- (id)textRenderer;
@property BOOL enableSubpixelAntialiasing;
- (void)setAutoInstallBlendMode:(BOOL)arg1;
- (BOOL)autoInstallBlendMode;
- (void)setEnableMagFilter:(BOOL)arg1;
- (BOOL)enableMagFilter;
- (void)emptyCaches;
- (void)dealloc;
- (id)init;

@end
