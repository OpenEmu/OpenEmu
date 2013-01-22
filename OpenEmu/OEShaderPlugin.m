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

@interface OEShaderPlugin ()
+ (void)OE_addPluginWithPath:(NSString *)aPath;
- (NSComparisonResult)OE_compare:(OEShaderPlugin *)value;
- (id)initWithShaderAtPath:(NSString *)aPath;
@end


@implementation OEShaderPlugin

@synthesize shader, name, path;


+ (void)initialize{
}

+ (NSString *)pluginFolder
{
    return @"Filters";
}

+ (NSString *)pluginExtension
{
    return @"cg";
}

- (NSComparisonResult)OE_compare:(OEShaderPlugin *)value
{
    return [[self name] caseInsensitiveCompare:[value name]];
} 

static NSMutableDictionary *plugins = nil;

+ (void)OE_addPluginWithPath:(NSString *)aPath
{
    OEShaderPlugin *plugin = [[self alloc] initWithShaderAtPath:aPath];
    if(plugin != nil) [plugins setObject:plugin forKey:[plugin name]];
}

+ (NSArray *)allPluginNames
{
    if(plugins == nil)
    {
        plugins = [[NSMutableDictionary alloc] init];
        
        NSString *folder = [self pluginFolder];
        NSString *extension = [self pluginExtension];
        
        NSString *openEmuSearchPath = [@"OpenEmu" stringByAppendingPathComponent:folder];
        
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
        
        NSFileManager *manager = [NSFileManager defaultManager];
        
        for(NSString *path in paths)
        {
            NSString *subpath = [path stringByAppendingPathComponent:openEmuSearchPath];
            NSArray  *subpaths = [manager contentsOfDirectoryAtPath:subpath error:nil];
            for(NSString *bundlePath in subpaths)
            {
                if([extension isEqualToString:[bundlePath pathExtension]])
                {
                    [self OE_addPluginWithPath:[subpath stringByAppendingPathComponent:bundlePath]];
                }
            }
        }
        
        paths = [[NSBundle mainBundle] pathsForResourcesOfType:extension inDirectory:folder];
        for(NSString *path in paths) [self OE_addPluginWithPath:path];
    }
    
    return [plugins allKeys];
}

+ (id)shaderPluginWithName:(NSString *)aName
{
    return [plugins objectForKey:aName];
}

- (id)init
{
    return [self initWithShaderAtPath:nil];
}

- (id)initWithShaderAtPath:(NSString *)aPath
{
    if((self = [super init]))
    {
        if(![[aPath pathExtension] isEqualToString:@"cg"])
        {
            return nil;
        }
        path = [aPath copy];

        name = [[path lastPathComponent] stringByDeletingPathExtension];
    }
    return self;
}

- (BOOL)isEqual:(id)object
{
    if([object isKindOfClass:[OEShaderPlugin class]])
        return [[self name] isEqualToString:[(OEShaderPlugin *)object name]];
    else if([object isKindOfClass:[NSString class]])
        return [[self name] isEqualToString:object];
    return [super isEqual:object];
}

- (NSUInteger)hash
{
    return [[self name] hash];
}

@end
