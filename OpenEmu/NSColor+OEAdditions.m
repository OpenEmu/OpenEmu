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

#import "NSColor+OEAdditions.h"
#import <objc/runtime.h>

// Note: this file is compiled under MRR. It cannot be ARC because there is no supported way
// to return an autoreleased CF object under ARC.

#if __has_feature(objc_arc)
#error This file cannot be compiled with ARC
#endif

@implementation NSColor (OEAdditionsDynamic)

static CGColorRef _NSColor_CGColor(NSColor *self, SEL _cmd);
static NSColor *_NSColor_colorWithCGColor_(Class self, SEL _cmd, CGColorRef color);

+ (void)load
{
    if(![self instancesRespondToSelector:@selector(CGColor)])
        class_addMethod(self, @selector(CGColor), (IMP)_NSColor_CGColor, "^{CGColor=}@:");
    
    if(![self respondsToSelector:@selector(colorWithCGColor:)])
        class_addMethod(object_getClass(self), @selector(colorWithCGColor:), (IMP)_NSColor_colorWithCGColor_, "@@:^{CGColor=}");
}

static NSColor *_NSColor_colorWithCGColor_(Class self, SEL _cmd, CGColorRef color)
{
    const CGFloat *components = CGColorGetComponents(color);
    NSColorSpace *colorSpace = [[NSColorSpace alloc] initWithCGColorSpace:CGColorGetColorSpace(color)];
    NSColor *result = [NSColor colorWithColorSpace:colorSpace components:components count:CGColorGetNumberOfComponents(color)];
    [colorSpace release];

    return result;
}

static CGColorRef _NSColor_CGColor(NSColor *self, SEL _cmd)
{
    if([self isEqualTo:[NSColor blackColor]]) return CGColorGetConstantColor(kCGColorBlack);
    if([self isEqualTo:[NSColor whiteColor]]) return CGColorGetConstantColor(kCGColorWhite);
    if([self isEqualTo:[NSColor clearColor]]) return CGColorGetConstantColor(kCGColorClear);

    NSColor *rgbColor = [self colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
    CGFloat components[4];
    [rgbColor getComponents:components];

    CGColorSpaceRef theColorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    CGColorRef theColor = CGColorCreate(theColorSpace, components);
    CGColorSpaceRelease(theColorSpace);

    return (CGColorRef)[(id)theColor autorelease];
}

@end
