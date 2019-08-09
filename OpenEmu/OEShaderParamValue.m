// Copyright (c) 2019, OpenEmu Team
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the OpenEmu Team nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#import "OEShaderParamValue.h"
#import <OpenEmuShaders/OpenEmuShaders.h>

#pragma mark -

@implementation OEShaderParamValue

+ (instancetype)fromParameter:(OEShaderParameter *)param atIndex:(NSUInteger)index atGroupIndex:(NSUInteger)groupIndex
{
    OEShaderParamValue *p = [OEShaderParamValue new];
    
    p.index      = index;
    p.groupIndex = groupIndex;
    p.name       = param.name;
    p.desc       = param.desc;
    p.group      = param.group;
    p.value      = @(param.value);
    p.initial    = @(param.initial);
    p.minimum    = @(param.minimum);
    p.maximum    = @(param.maximum);
    p.step       = @(param.step);
    
    return p;
}

+ (nonnull instancetype)groupWithName:(NSString *)name
{
    OEShaderParamValue *p = [OEShaderParamValue new];
    p.index = -1; // "group"
    p.group = name;
    return p;
}

+ (NSArray<OEShaderParamValue *> *)withParameters:(NSArray<OEShaderParameter *> *)params
{
    __block NSMutableArray<OEShaderParamValue *> *res = [[NSMutableArray alloc] initWithCapacity:params.count];

    [params enumerateObjectsUsingBlock:^(OEShaderParameter * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        [res addObject:[OEShaderParamValue fromParameter:obj atIndex:idx atGroupIndex:0]];
    }];
    
    return res;
}

+ (NSArray<OEShaderParamValue *> *)fromParameters:(NSArray<OEShaderParameter *> *)params atGroupIndex:(NSUInteger)index
{
    __block NSMutableArray<OEShaderParamValue *> *res = [[NSMutableArray alloc] initWithCapacity:params.count];

    [params enumerateObjectsUsingBlock:^(OEShaderParameter * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        [res addObject:[OEShaderParamValue fromParameter:obj atIndex:idx atGroupIndex:index]];
    }];
    
    return res;
}

static bool approximatelyEqual(double a, double b, double epsilon)
{
    return fabs(a - b) <= ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

static float newPrecision(double n, double i)
{
    return round(pow(10, i) * n) / pow(10, i);
}

- (BOOL)isInitial {
    return approximatelyEqual(_value.doubleValue, _initial.doubleValue, DBL_EPSILON);
}

- (void)setValue:(NSNumber *)value
{
    [self willChangeValueForKey:@"value"];
    
    if (_step.doubleValue > 0.0)
    {
        double d = 1.0 / _step.doubleValue;
        value = @(newPrecision(round(value.doubleValue * d) / d, 4));
    }
    
    _value = value;
    
    [self didChangeValueForKey:@"value"];
}

#pragma mark - NSSecureCoding

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if ((self = [super init]))
    {
        _index      = [coder decodeIntegerForKey:@"index"];
        _groupIndex = [coder decodeIntegerForKey:@"groupIndex"];
        _name       = [coder decodeObjectOfClass:NSString.class forKey:@"name"];
        _desc       = [coder decodeObjectOfClass:NSString.class forKey:@"desc"];
        _group      = [coder decodeObjectOfClass:NSString.class forKey:@"group"];
        _value      = [coder decodeObjectOfClass:NSNumber.class forKey:@"value"];
        _initial    = [coder decodeObjectOfClass:NSNumber.class forKey:@"initial"];
        _minimum    = [coder decodeObjectOfClass:NSNumber.class forKey:@"minimum"];
        _maximum    = [coder decodeObjectOfClass:NSNumber.class forKey:@"maximum"];
        _step       = [coder decodeObjectOfClass:NSNumber.class forKey:@"step"];
    }
    return self;
}

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeInteger:_index forKey:@"index"];
    [coder encodeInteger:_groupIndex forKey:@"groupIndex"];
    [coder encodeObject:_name forKey:@"name"];
    [coder encodeObject:_desc forKey:@"desc"];
    [coder encodeObject:_group forKey:@"group"];
    [coder encodeObject:_value forKey:@"value"];
    [coder encodeObject:_initial forKey:@"initial"];
    [coder encodeObject:_minimum forKey:@"minimum"];
    [coder encodeObject:_maximum forKey:@"maximum"];
    [coder encodeObject:_step forKey:@"step"];
}

@end

#pragma mark -

@implementation OEShaderParamGroupValue

+ (OEShaderParamGroupValue *)fromGroup:(OEShaderParamGroup *)group atIndex:(NSUInteger)index
{
    OEShaderParamGroupValue *g = [OEShaderParamGroupValue new];
    g.index      = index;
    g.name       = group.name;
    g.desc       = group.desc;
    g.hidden     = group.hidden;
    g.parameters = [OEShaderParamValue fromParameters:group.parameters atGroupIndex:index];
    
    return g;
}

+ (OEShaderParamGroups)fromGroups:(NSArray<OEShaderParamGroup *> *)groups
{
    __block NSMutableArray<OEShaderParamGroupValue *> *res = [[NSMutableArray alloc] initWithCapacity:groups.count];

    [groups enumerateObjectsUsingBlock:^(OEShaderParamGroup * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        [res addObject:[OEShaderParamGroupValue fromGroup:obj atIndex:idx]];
    }];
    
    return res;
}

#pragma mark - NSSecureCoding

+ (NSSet *)codingClasses
{
    static NSSet *classes;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        classes = [NSSet setWithObjects:NSArray.class, OEShaderParamValue.class, nil];
    });
    return classes;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if ((self = [super init]))
    {
        _index      = [coder decodeIntegerForKey:@"index"];
        _name       = [coder decodeObjectOfClass:NSString.class forKey:@"name"];
        _desc       = [coder decodeObjectOfClass:NSString.class forKey:@"desc"];
        _hidden     = [coder decodeBoolForKey:@"hidden"];
        _parameters = [coder decodeObjectOfClasses:OEShaderParamGroupValue.codingClasses
                                            forKey:@"parameters"];
    }
    return self;
}

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeInteger:_index forKey:@"index"];
    [coder encodeObject:_name forKey:@"name"];
    [coder encodeObject:_desc forKey:@"desc"];
    [coder encodeBool:_hidden forKey:@"hidden"];
    [coder encodeObject:_parameters forKey:@"parameters"];
}

@end
