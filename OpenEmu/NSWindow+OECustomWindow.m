/*
 Copyright (c) 2011, OpenEmu Team
 
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
    Class NSThemeFrameClass = NSClassFromString(@"NSThemeFrame");
    
    if(customDrawingWasSetup == NO)
    {       
        customDrawingWasSetup = YES;

        // exchange method implementation of NSThemeFrame's drawRect:
        Method customDrawingMethod = class_getInstanceMethod([NSWindow class], @selector(drawThemeFrame:));
        IMP customDrawingImplementation = method_getImplementation(customDrawingMethod);
        
        Method originalDrawingMethod = class_getInstanceMethod(NSThemeFrameClass, @selector(drawRect:));
        const char *typeEncoding = method_getTypeEncoding(originalDrawingMethod);
        
        // make original implementation of drawRect: available as drawRectOriginal:
        class_addMethod(NSThemeFrameClass, @selector(drawRectOriginal:), customDrawingImplementation, typeEncoding);
        Method customDrawingMethodOnThemeFrame = class_getInstanceMethod(NSThemeFrameClass, @selector(drawRectOriginal:));
        method_exchangeImplementations(originalDrawingMethod, customDrawingMethodOnThemeFrame);
    }
    
    // add instance method drawRect: of themeViewClass to instances of NSThemeFrameClass
    NSString *winodwClassName = NSStringFromClass(windowClass);
    NSString *selectorName = [NSString stringWithFormat:@"draw%@ThemeRect:", winodwClassName];
    SEL newDrawingSelector = NSSelectorFromString(selectorName);
    
    Method themeDrawingMethod = class_getInstanceMethod(windowClass, @selector(drawThemeFrame:));
    IMP themeDrawingImplementation = method_getImplementation(themeDrawingMethod);
    const char *typeEncoding = method_getTypeEncoding(themeDrawingMethod);

    class_addMethod(NSThemeFrameClass, newDrawingSelector, themeDrawingImplementation, typeEncoding);
}

- (void)drawThemeFrame:(NSRect)dirtyRect
{
    id window = [(NSView*)self window];
    
    BOOL drawingCustomWindow = [window conformsToProtocol:@protocol(OECustomWindow)];
    if(!drawingCustomWindow || [window drawsAboveDefaultThemeFrame])
    {
        [self drawRectOriginal:dirtyRect];
    }

    if(!drawingCustomWindow) return;
    // create drawing selector based on window class
    NSString *windowClassName = NSStringFromClass([window class]);
    NSString *selectorName = [NSString stringWithFormat:@"draw%@ThemeRect:", windowClassName];
    SEL customDrawingSelector = NSSelectorFromString(selectorName);

    // finally draw the custom theme frame
    ((void (*)(id, SEL, NSRect))objc_msgSend)(self, customDrawingSelector, dirtyRect);
}
@end
