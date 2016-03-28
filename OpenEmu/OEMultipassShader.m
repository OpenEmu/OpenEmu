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

#ifdef CG_SUPPORT

#import "OEMultipassShader.h"
#import "OEShaderPlugin.h"
#import "OECGShader.h"
#import "OEGameShader_ForSubclassEyesOnly.h"
#import "OELUTTexture.h"

@implementation OEMultipassShader
{
    NSMutableArray *_shaders;
    NSMutableArray *_lutTextures;
}

- (void)compileShaders
{
    if(![self isCompiled])
    {
        if(![self parseCGPFile]) return;

        for(OECGShader *x in _shaders)
            [x compileShaders];

        for(OELUTTexture* x in _lutTextures)
            [x loadTexture];

        [self setCompiled:YES];
    }
}

- (NSTextCheckingResult *)checkRegularExpression:(NSString *)regexTerm inString:(NSString *)input withError:(NSError *)error
{
    NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:regexTerm options:NSRegularExpressionAnchorsMatchLines error:&error];
    NSRange inputRange = NSMakeRange(0, [input length]);
    return [regex firstMatchInString:input options:0 range:inputRange];
}

- (BOOL)parseCGPFile
{
    NSError  *error = nil;
    NSString *input = [NSString stringWithContentsOfFile:[self filePath] encoding:NSUTF8StringEncoding error:&error];

    if(input == nil)
    {
        NSLog(@"Couldn't read shader source file of %@: %@", [self shaderName], error);
        return NO;
    }

    // Remove whitespace
    NSArray  *seperateByWhitespace = [input componentsSeparatedByCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
    NSString *strippedInput = [seperateByWhitespace componentsJoinedByString:@""];
    NSTextCheckingResult *result = nil;
    NSArray *lutNames = nil;

    // Check for LUTs
    result = [self checkRegularExpression:@"(?<=textures=).*" inString:strippedInput withError:error];
    if(result.range.length != 0)
    {
        NSString *otherArguments = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];
        lutNames = [otherArguments componentsSeparatedByString: @";"];
        if([lutNames count] > OELUTTextures)
        {
            NSLog(@"Too many lut textures in %@: %@", [self shaderName], error);
            return NO;
        }

        _lutTextures = [NSMutableArray arrayWithCapacity:[lutNames count]];

        for(NSUInteger i = 0; i < [lutNames count]; ++i)
        {
            OELUTTexture *lut = [OELUTTexture new];
            [lut setName:lutNames[i]];

            result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=%s=).*", [[lut name] UTF8String]] inString:strippedInput withError:error];
            if(result.range.length == 0)
            {
                NSLog(@"Couldn't find \"%s\" argument of %@: %@", [[lut name] UTF8String], [self shaderName], error);
                return NO;
            }

            NSString *texture = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];

            // Find texture
            NSFileManager *manager = [NSFileManager defaultManager];
            [lut setPath: [[[self filePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:texture]];
            if(![manager fileExistsAtPath:[lut path]])
            {
                NSLog(@"Couldn't find texture named %@", texture);
                return NO;
            }

            // Check if linear filtering is to be used
            result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=%s_linear=).*", [[lut name] UTF8String]] inString:strippedInput withError:error];
            otherArguments = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];
            if([otherArguments isEqualToString:@"true"] || [otherArguments isEqualToString:@"1"])
            {
                [lut setLinearFiltering:YES];
            }

            // Check which wrap mode is to be used
            result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=%s_wrap_mode=).*", [[lut name] UTF8String]] inString:strippedInput withError:error];
            otherArguments = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];
            if([otherArguments isEqualToString:@"clamp_to_border"])
            {
                [lut setWrapType:GL_CLAMP_TO_BORDER];
            }
            else if([otherArguments isEqualToString:@"clamp_to_edge"])
            {
                [lut setWrapType:GL_CLAMP_TO_EDGE];
            }
            else if([otherArguments isEqualToString:@"repeat"])
            {
                [lut setWrapType:GL_REPEAT];
            }
            else if([otherArguments isEqualToString:@"mirrored_repeat"])
            {
                [lut setWrapType:GL_MIRRORED_REPEAT];
            }

            // Check if mipmapping is to be used
            result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=%s_mipmap=).*", [[lut name] UTF8String]] inString:strippedInput withError:error];
            otherArguments = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];
            if([otherArguments isEqualToString:@"true"] || [otherArguments isEqualToString:@"1"])
            {
                [lut setMipmap:YES];
            }
            
            [_lutTextures addObject:lut];
        }
    }

    // Parse the number of shaders
    result = [self checkRegularExpression:@"(?<=shaders=).*$" inString:strippedInput withError:error];
    if(result.range.length == 0)
    {
        NSLog(@"Couldn't find \"shaders\" argument of %@: %@", [self shaderName], error);
        return NO;
    }

    _numberOfPasses = [[[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""] integerValue];

    if(_numberOfPasses > OEMultipasses)
    {
        NSLog(@"Too many shader passes in %@: %@", [self shaderName], error);
        return NO;
    }

    _shaders = [NSMutableArray arrayWithCapacity:_numberOfPasses];

    // We need to find that many shader sources
    for(NSUInteger i = 0; i < _numberOfPasses; ++i)
    {
        result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=shader%ld=).*(?=.cg)", i] inString:strippedInput withError:error];
        if(result.range.length == 0)
        {
            NSLog(@"Couldn't find \"shader%ld\" argument of %@: %@", i, [self shaderName], error);
            return NO;
        }

        NSString *name = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];

        // Create shader
        OECGShader *shader = [[OECGShaderPlugin pluginWithName:name] shaderWithContext:[self shaderContext]];
        if(!shader)
        {
            NSLog(@"Couldn't find shader named %@", name);
            return NO;
        }

        // Check if linear filtering is to be used
        result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=filter_linear%ld=).*", i] inString:strippedInput withError:error];
        NSString *otherArguments = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];
        if([otherArguments isEqualToString:@"true"] || [otherArguments isEqualToString:@"1"])
        {
            [shader setLinearFiltering:YES];
        }

        // Check how the shader should scale
        result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=scale_type%ld=).*", i] inString:strippedInput withError:error];
        if(result.range.length != 0)
        {
            otherArguments = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];
            if([otherArguments isEqualToString:@"viewport"])
            {
                [shader setXScaleType:OEScaleTypeViewPort];
                [shader setYScaleType:OEScaleTypeViewPort];
            }
            else if([otherArguments isEqualToString:@"absolute"])
            {
                [shader setXScaleType:OEScaleTypeAbsolute];
                [shader setYScaleType:OEScaleTypeAbsolute];
            }
            else
            {
                [shader setXScaleType:OEScaleTypeSource];
                [shader setYScaleType:OEScaleTypeSource];
            }
        }
        else
        {
            result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=scale_type_x%ld=).*", i] inString:strippedInput withError:error];
            if(result.range.length != 0)
            {
                otherArguments = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];
                if([otherArguments isEqualToString:@"viewport"])
                    [shader setXScaleType:OEScaleTypeViewPort];
                else if([otherArguments isEqualToString:@"absolute"])
                    [shader setXScaleType:OEScaleTypeAbsolute];
                else
                    [shader setXScaleType:OEScaleTypeSource];
            }
            else
            {
                if(i != (_numberOfPasses - 1))
                    [shader setXScaleType:OEScaleTypeSource];
                else
                    [shader setXScaleType:OEScaleTypeViewPort];
            }

            result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=scale_type_y%ld=).*", i] inString:strippedInput withError:error];
            if(result.range.length != 0)
            {
                otherArguments = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];
                if([otherArguments isEqualToString:@"viewport"])
                    [shader setYScaleType:OEScaleTypeViewPort];
                else if([otherArguments isEqualToString:@"absolute"])
                    [shader setYScaleType:OEScaleTypeAbsolute];
                else
                    [shader setYScaleType:OEScaleTypeSource];
            }
            else
            {
                if(i != (_numberOfPasses - 1))
                    [shader setYScaleType:OEScaleTypeSource];
                else
                    [shader setYScaleType:OEScaleTypeViewPort];
            }
        }

        // Check for the scaling factor
        result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=scale%ld=).*", i] inString:strippedInput withError:error];
        if(result.range.length != 0)
        {
            otherArguments = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];
            [shader setScaler:CGSizeMake([otherArguments floatValue], [otherArguments floatValue])];
        }
        else
        {
            CGFloat x = 1;
            CGFloat y = 1;

            result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=scale_x%ld=).*", i] inString:strippedInput withError:error];
            if(result.range.length != 0)
            {
                otherArguments = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];
                x = [otherArguments floatValue];
            }

            result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=scale_y%ld=).*", i] inString:strippedInput withError:error];
            if(result.range.length != 0)
            {
                otherArguments = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];
                y = [otherArguments floatValue];
            }

            [shader setScaler:CGSizeMake(x,y)];
        }

        // Check if a floating point framebuffer is to be used
        result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=float_framebuffer%ld=).*", i] inString:strippedInput withError:error];
        otherArguments = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];
        if([otherArguments isEqualToString:@"true"] || [otherArguments isEqualToString:@"1"])
        {
            [shader setFloatFramebuffer:YES];
        }

        // Check if a modulo should be applied to the frame count
        result = [self checkRegularExpression:[NSString stringWithFormat:@"(?<=frame_count_mod%ld=).*", i] inString:strippedInput withError:error];
        if(result.range.length != 0)
        {
            otherArguments = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];
            [shader setFrameCountMod:[otherArguments integerValue]];
        }

        // Add the LUTs to the shader
        [shader setLutTextures:lutNames];

        // Add the shader to the shaders array
        [_shaders addObject:shader];
    }

    _NTSCFilter = OENTSCFilterTypeNone;

    result = [self checkRegularExpression:@"(?<=ntsc_filter=).*" inString:strippedInput withError:error];
    if(result.range.location != NSNotFound)
    {
         NSString *ntscString = [[strippedInput substringWithRange:result.range] stringByReplacingOccurrencesOfString:@"\"" withString:@""];

        if([ntscString isEqualToString:@"composite"])
            _NTSCFilter = OENTSCFilterTypeComposite;
        else if([ntscString isEqualToString:@"svideo"])
            _NTSCFilter = OENTSCFilterTypeSVideo;
        else if([ntscString isEqualToString:@"rgb"])
            _NTSCFilter = OENTSCFilterTypeRGB;
    }

    return YES;
}

- (NSArray *)shaders
{
    return _shaders;
}

- (NSArray *)lutTextures
{
    return _lutTextures;
}

@end

#endif
