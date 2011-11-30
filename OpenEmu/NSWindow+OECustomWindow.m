//
//  NSWindow+OECustomWindow.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 25.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "NSWindow+OECustomWindow.h"
#import <objc/objc-runtime.h>

// This code is based on Patrick Geiller's article on custom NSThemeFrames
// check it out at http://parmanoir.com/Custom_NSThemeFrame

@interface NSWindow (OEOriginalThemeFrameDrawing)
- (void)drawRectOriginal:(NSRect)dirtyRect;
@end
@implementation NSWindow (OECustomWindow)

+ (void)registerWindowClassForCustomThemeFrameDrawing:(Class)windowClass
{
    static BOOL customDrawingWasSetup = NO;
    DLog(@"customDrawingWasSetup before: %d", customDrawingWasSetup);
    Class NSThemeFrameClass = NSClassFromString(@"NSThemeFrame");
    
    if(customDrawingWasSetup == NO)
    {       
        customDrawingWasSetup = YES;

        // exchange method implementation of NSThemeFrame's drawRect:
        Method customDrawingMethod = class_getInstanceMethod([NSWindow class], @selector(drawThemeFrame:));
        IMP customDrawingImplementation = method_getImplementation(customDrawingMethod);
        
        Method originalDrawingMethod = class_getInstanceMethod(NSThemeFrameClass, @selector(drawRect:));
        const char* typeEncoding = method_getTypeEncoding(originalDrawingMethod);
        
        // make original implementation of drawRect: available as drawRectOriginal:
        class_addMethod(NSThemeFrameClass, @selector(drawRectOriginal:), customDrawingImplementation, typeEncoding);
        Method customDrawingMethodOnThemeFrame = class_getInstanceMethod(NSThemeFrameClass, @selector(drawRectOriginal:));
        method_exchangeImplementations(originalDrawingMethod, customDrawingMethodOnThemeFrame);
    }
    
    // add instance method drawRect: of themeViewClass to instances of NSThemeFrameClass
    NSString* winodwClassName = NSStringFromClass(windowClass);
    NSString* selectorName = [NSString stringWithFormat:@"draw%@ThemeRect:", winodwClassName];
    SEL newDrawingSelector = NSSelectorFromString(selectorName);
    
    Method themeDrawingMethod = class_getInstanceMethod(windowClass, @selector(drawThemeFrame:));
    IMP themeDrawingImplementation = method_getImplementation(themeDrawingMethod);
    const char* typeEncoding = method_getTypeEncoding(themeDrawingMethod);

    class_addMethod(NSThemeFrameClass, newDrawingSelector, themeDrawingImplementation, typeEncoding);
}

- (void)drawThemeFrame:(NSRect)dirtyRect
{
    id window = [(NSView*)self window];
    
    BOOL drawingCustomWindow = [window conformsToProtocol:@protocol(OECustomWindow)];
    NSLog(@"draw theme frame: %@ | %d", window, drawingCustomWindow);

    if(!drawingCustomWindow || [window drawsAboveDefaultThemeFrame])
    {
        [self drawRectOriginal:dirtyRect];
    }

    if(!drawingCustomWindow) return;
    // create drawing selector based on window class
    NSString* winodwClassName = NSStringFromClass([window class]);
    NSString* selectorName = [NSString stringWithFormat:@"draw%@ThemeRect:", winodwClassName];
    SEL customDrawingSelector = NSSelectorFromString(selectorName);

    // finally draw the custom theme frame
    [self performSelector:customDrawingSelector withObject:[NSValue valueWithRect:dirtyRect]];    
}
@end
