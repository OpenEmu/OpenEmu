//
//  OEFilterPlugin.h
//  OpenEmu
//
//  Created by Remy Demarest on 15/06/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import "OEPlugin.h"
@class OEGameShader;

@interface OEFilterPlugin : OEPlugin
{
    OEGameShader *currentShader;
}

+ (OEGameShader *)gameShaderWithFilterName:(NSString *)aFilterName forContext:(CGLContextObj)aContext;
- (OEGameShader *)gameShaderForContext:(CGLContextObj)aContext;

@end
