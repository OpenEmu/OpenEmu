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

#import "OEMultipassShader.h"
#import "OEShaderPlugin.h"

@implementation OEMultipassShader

- (id)initWithShadersInFilterDirectory:(NSString *)theShadersName forContext:(CGLContextObj)context
{
    if((self = [super initWithName:theShadersName forContext:context]))
    {
        NSString *openEmuSearchPath = [@"OpenEmu" stringByAppendingPathComponent:[OEShaderPlugin pluginFolder]];

        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);

        for(NSString *path in paths)
        {
            NSString *shaderPath = [path stringByAppendingPathComponent:[openEmuSearchPath stringByAppendingPathComponent:theShadersName]];
            shaderSource = [shaderPath stringByAppendingPathExtension:@"cgp"];
            shaderData = self;
            return self;
        }
    }
    return self;
}

- (void)compileShaders
{
    if(compiled == NO)
    {
        if(([self parseCgpFile] == NO))
            return;
        for (OECgShader *x in shaders) {
            [x compileShaders];
        }

        compiled = YES;
    }
}

- (NSTextCheckingResult *)checkRegularExpression:(NSString *)regexTerm inString:(NSString *)input withError:(NSError *)error
{
    NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:regexTerm options:NSRegularExpressionAnchorsMatchLines error:&error];
    NSRange   inputRange = NSMakeRange(0, [input length]);
    return [regex firstMatchInString:input options:0 range:inputRange];
}

- (BOOL)parseCgpFile
{
    NSError  *error = nil;
    NSString *input = [NSString stringWithContentsOfFile:shaderSource encoding:NSUTF8StringEncoding error:&error];
    if(input==nil)
    {
        NSLog(@"Couldn't read shader source file of %@: %@", shaderName, error);
        return NO;
    }

    // Remove whitespace
    NSArray  *seperateByWhitespace = [input componentsSeparatedByCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
    NSString *strippedInput = [seperateByWhitespace componentsJoinedByString:@""];

    // Parse the number of shaders
    NSTextCheckingResult *result = [self checkRegularExpression:@"(?<=shaders=).*$" inString:strippedInput withError:error];
    if(result.range.location == NSNotFound)
    {
        NSLog(@"Couldn't find \"shaders\" argument of %@: %@", shaderName, error);
        return NO;
    }
    numberOfPasses = [[strippedInput substringWithRange:result.range] integerValue];
    shaders = [NSMutableArray arrayWithCapacity:numberOfPasses];

    // We need to find that many shader sources
    for(int i=0; i<numberOfPasses; ++i)
    {
        result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=shader%d=).*(?=.cg)", i] inString:strippedInput withError:error];
        if(result.range.location == NSNotFound)
        {
            NSLog(@"Couldn't find \"shader%d\" argument of %@: %@", i, shaderName, error);
            return NO;
        }
        NSString *name = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];

        // Create shader
        OECgShader *shader = [[OECgShader alloc] initWithShadersInFilterDirectory:name forContext:shaderContext];

        // Check if linear filtering is to be used
        result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=filter_linear%d=).*", i] inString:strippedInput withError:error];
        NSString *otherArguments = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];
        if([otherArguments isEqualToString:@"true"] || [otherArguments isEqualToString:@"1"])
        {
            [shader setLinearFiltering:YES];
        }

        // Check how the shader should scale
        result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=scale_type%d=).*", i] inString:strippedInput withError:error];
        otherArguments = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];
        if(result.range.location != NSNotFound)
        {
            [shader setScaleType:otherArguments];
        }

        // Check for the scaling factor
        result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=scale%d=).*", i] inString:strippedInput withError:error];
        if(result.range.location != NSNotFound)
        {
            otherArguments = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];
            [shader setScaler:CGSizeMake([otherArguments floatValue], [otherArguments floatValue])];
        }

        // Add the shader to the shaders array
        [shaders addObject:shader];
    }

    result = [self checkRegularExpression:@"(?<=ntsc_filter=).*" inString:strippedInput withError:error];
    if(result.range.location != NSNotFound)
    {
        ntscFilter = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];
    }

    return YES;
}

- (NSUInteger)numberOfPasses
{
    return numberOfPasses;
}

- (NSMutableArray *)shaders
{
    return shaders;
}

- (NSString *)ntscFilter
{
    return ntscFilter;
}

@end
