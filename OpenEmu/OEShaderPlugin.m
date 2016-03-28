/*
 Copyright (c) 2013, OpenEmu Team

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

#import "OEShaderPlugin.h"
#import "OEGLSLShader.h"

#ifdef CG_SUPPORT
#import "OECGShader.h"
#import "OEMultipassShader.h"
#endif

#import "OEBuiltInShader.h"

@implementation OEShaderPlugin

+ (void)initialize
{
    if(self == [OEShaderPlugin class])
    {
        [OEGLSLShaderPlugin      registerPluginClass];

#ifdef CG_SUPPORT
        [OECGShaderPlugin        registerPluginClass];
        [OEMultipassShaderPlugin registerPluginClass];
#endif

        [OEBuiltInShaderPlugin   registerPluginClass];
    }
}

+ (NSArray *)allPlugins
{
    if(self == [OEShaderPlugin class])
    {
        NSMutableArray *allPlugins = [NSMutableArray array];
        [allPlugins addObjectsFromArray:[self pluginsForType:[OEGLSLShaderPlugin class]]];

#ifdef CG_SUPPORT
        [allPlugins addObjectsFromArray:[self pluginsForType:[OECGShaderPlugin class]]];
        [allPlugins addObjectsFromArray:[self pluginsForType:[OEMultipassShaderPlugin class]]];
#endif

        [allPlugins addObjectsFromArray:[self pluginsForType:[OEBuiltInShaderPlugin class]]];

        return allPlugins;
    }

    return [super allPlugins];
}

+ (Class)shaderClass
{
    return Nil;
}

+ (NSString *)pluginFolder
{
    return @"Filters";
}

+ (NSString *)pluginExtension
{
    return nil;
}

- (id)shaderWithContext:(CGLContextObj)context;
{
    return [[[[self class] shaderClass] alloc] initWithFileAtPath:[self path] context:context];
}

@end

@implementation OEGLSLShaderPlugin

+ (Class)shaderClass
{
    return [OEGLSLShader class];
}

+ (NSString *)pluginExtension
{
    return @"vert";
}

- (id)initWithFileAtPath:(NSString *)aPath name:(NSString *)aName
{
    NSString *fragPath = [[aPath stringByDeletingPathExtension] stringByAppendingPathExtension:@"frag"];
    if(![[NSFileManager defaultManager] fileExistsAtPath:fragPath])
        return nil;

    return [super initWithFileAtPath:aPath name:aName];
}

@end

#ifdef CG_SUPPORT
@implementation OECGShaderPlugin

+ (Class)shaderClass
{
    return [OECGShader class];
}

+ (NSString *)pluginExtension
{
    return @"cg";
}

@end

@implementation OEMultipassShaderPlugin

+ (Class)shaderClass
{
    return [OEMultipassShader class];
}

+ (NSString *)pluginExtension
{
    return @"cgp";
}

@end
#endif

@implementation OEBuiltInShaderPlugin
{
    OEBuiltInShader *_shader;
}

+ (Class)shaderClass
{
    return [OEBuiltInShaderPlugin class];
}

+ (NSString *)pluginExtension
{
    return @"";
}

+ (void)registerPluginClass
{
    [super registerPluginClass];

    for(OEBuiltInShaderType type = 0; type < OEBuiltInShaderTypeCount; type++)
        [self pluginWithFileAtPath:[OEBuiltInShader shaderNameForBuiltInShaderType:type] type:self];
}

- (id)initWithFileAtPath:(NSString *)aPath name:(NSString *)aName
{
    return [self initWithBuiltInShaderType:[OEBuiltInShader builtInShaderTypeShaderName:aName ? : aPath]];
}

- (id)initWithBuiltInShaderType:(OEBuiltInShaderType)type
{
    if((self = [super initWithFileAtPath:nil name:[OEBuiltInShader shaderNameForBuiltInShaderType:type]]))
    {
        _shader = [[OEBuiltInShader alloc] initWithBuiltInShaderType:type];
    }
    return self;
}

- (id)shaderWithContext:(CGLContextObj)context
{
    return _shader;
}

@end
