//
//  OEFilterPlugin.m
//  OpenEmu
//
//  Created by Remy Demarest on 15/06/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEFilterPlugin.h"
#import "OEGameShader.h"

@implementation OEFilterPlugin

- (id)initWithBundle:(NSBundle *)aBundle
{
    if(self = [super initWithBundle:aBundle])
    {
        NSString *shaderSource = [aBundle pathForResource:[self displayName] ofType:@"frag"];
        NSString *fragmentSource = [NSString stringWithContentsOfFile:shaderSource];
        shaderSource = [aBundle pathForResource:[self displayName] ofType:@"vert"];
        NSString *vertexSource = [NSString stringWithContentsOfFile:shaderSource];
        currentShader = [[OEGameShader alloc] initWithFragmentSource:fragmentSource vertexSource:vertexSource];
    }
    return self;
}

- (void) dealloc
{
    [currentShader release];
    [super dealloc];
}


+ (OEGameShader *)gameShaderWithFilterName:(NSString *)aFilterName forContext:(CGLContextObj)aContext
{
    OEFilterPlugin *filter = [self pluginWithBundleName:aFilterName type:self];
    return [filter gameShaderForContext:aContext];
}

- (OEGameShader *)gameShaderForContext:(CGLContextObj)aContext
{
    [currentShader setShaderContext:aContext];
    if([currentShader programObject] == NULL)
        NSLog(@">> WARNING: Failed to load GLSL \"%@\" fragment & vertex shaders!\n", [self displayName]);
    return currentShader;
}

@end
