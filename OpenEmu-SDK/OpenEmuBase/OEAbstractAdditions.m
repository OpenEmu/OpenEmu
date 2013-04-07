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

#import "OEAbstractAdditions.h"


@implementation NSObject (OEAbstractAdditions)

+ (void)doesNotImplementSelector:(SEL)aSel
{
    @throw [NSException exceptionWithName:NSInvalidArgumentException
                                   reason:[NSString stringWithFormat:@"*** +%s cannot be sent to the abstract class %@: Create a concrete subclass!", sel_getName(aSel), [self class]]
                                 userInfo:nil];
}

- (void)doesNotImplementSelector:(SEL)aSel
{
    @throw [NSException exceptionWithName:NSInvalidArgumentException
                                   reason:[NSString stringWithFormat:@"*** -%s cannot be sent to an abstract object of class %@: Create a concrete instance!", sel_getName(aSel), [self class]]
                                 userInfo:nil];
}

+ (void)doesNotImplementOptionalSelector:(SEL)aSel
{
    NSLog(@"*** +%s is an optional method and it is not implemented in %@!", sel_getName(aSel), NSStringFromClass([self class]));
}

- (void)doesNotImplementOptionalSelector:(SEL)aSel
{
    NSLog(@"*** -%s is an optional method and it is not implemented in %@!", sel_getName(aSel), NSStringFromClass([self class]));
}

@end
