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

#import "OEBuiltInShader.h"

static NSString *const _OELinearFilterName          = @"Linear";
static NSString *const _OENearestNeighborFilterName = @"Nearest Neighbor";

@implementation OEBuiltInShader

static OEBuiltInShader *_builtInShaders[OEBuiltInShaderTypeCount] = { nil };

+ (NSString *)shaderNameForBuiltInShaderType:(OEBuiltInShaderType)type;
{
    switch(type) {
        case OEBuiltInShaderTypeLinear          : return _OELinearFilterName;
        case OEBuiltInShaderTypeNearestNeighbor : return _OENearestNeighborFilterName;
        default : break;
    }

    return nil;
}

+ (OEBuiltInShaderType)builtInShaderTypeShaderName:(NSString *)type;
{
    static NSArray *types = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        types = @[ _OELinearFilterName, _OENearestNeighborFilterName ];
    });

    return [types indexOfObject:type];
}

- (id)initWithFileAtPath:(NSString *)filePath context:(CGLContextObj)context
{
    return [self initWithBuiltInShaderType:[[self class] builtInShaderTypeShaderName:filePath]];
}

- (id)initWithBuiltInShaderType:(OEBuiltInShaderType)type
{
    if(type >= OEBuiltInShaderTypeCount) return nil;

    if(_builtInShaders[type] == nil &&
       (self = [super initWithFileAtPath:[[self class] shaderNameForBuiltInShaderType:type] context:nil]))
    {
        _type = type;
        _builtInShaders[type] = self;
    }

    return _builtInShaders[type];
}

- (BOOL)isBuiltIn;
{
    return YES;
}

@end
