/*
 Copyright (c) 2009, OpenEmu Team

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

#import "OECompositionPlugin.h"

#define OEPreviewImageOutputKey @"OEPreviewImage"

@implementation OECompositionPlugin

+ (NSString *)pluginFolder
{
    return @"Filters";
}

+ (NSString *)pluginExtension
{
    return @"qtz";
}

- (id)initWithFileAtPath:(NSString *)aPath name:(NSString *)aName
{
    QCComposition *composition = [QCComposition compositionWithFile:aPath];
    if(composition == nil) return nil;

    if(![[composition inputKeys] containsObject:@"OEImageInput"]) return nil;

    aName = (    [[_composition attributes] objectForKey:QCCompositionAttributeNameKey]
             ? : [[_composition attributes] objectForKey:@"name"]);

    if((self = [super initWithFileAtPath:aPath name:aName]))
        _composition = composition;

    return self;
}

- (NSString *)description
{
    return [[_composition attributes] objectForKey:QCCompositionAttributeDescriptionKey];
}

- (NSString *)copyright
{
    return [[_composition attributes] objectForKey:QCCompositionAttributeCopyrightKey];
}

- (BOOL)isBuiltIn
{
    return [[[_composition attributes] objectForKey:QCCompositionAttributeBuiltInKey] boolValue];
}
- (BOOL)isTimeDependent
{
    return [[[_composition attributes] objectForKey:@"QCCompositionAttributeTimeDependentKey"] boolValue];
}

- (BOOL)hasConsumers
{
    return [[[_composition attributes] objectForKey:QCCompositionAttributeHasConsumersKey] boolValue];
}

- (NSString *)category
{
    return [[_composition attributes] objectForKey:QCCompositionAttributeCategoryKey];
}

- (NSImage *)previewImage
{
    if(![[[self composition] outputKeys] containsObject:OEPreviewImageOutputKey])
        return nil;

    NSOpenGLPixelFormatAttribute attributes[] = {
        (NSOpenGLPixelFormatAttribute) 0
    };

    NSOpenGLContext     *openGLContext = nil;
    QCRenderer          *renderer      = nil;
    NSOpenGLPixelFormat *format        = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];

    openGLContext = [[NSOpenGLContext alloc] initWithFormat:format shareContext:nil];
    if(openGLContext == nil)
    {
        DLog(@"Cannot create OpenGL context");
        return nil;
    }

    //Create the QuartzComposer Renderer with that OpenGL context and the specified composition file
    renderer = [[QCRenderer alloc] initWithOpenGLContext:openGLContext pixelFormat:format file:[self path]];
    if(renderer == nil)
    {
        DLog(@"Cannot create QCRenderer");
        return nil;
    }

    NSImage *previewImage = [renderer valueForOutputKey:@"OEPreviewImage" ofType:@"NSImage"];
    if(previewImage == nil)
    {
        DLog(@"Did not get a preview image");
        return nil;
    }

    return previewImage;
}

@end
