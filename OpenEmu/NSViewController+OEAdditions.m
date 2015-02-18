/*
 Copyright (c) 2012, OpenEmu Team

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

#import "NSViewController+OEAdditions.h"
#import "OEUtilities.h"
#include <objc/runtime.h>

void OEMethodCopy(SEL oldMethod, SEL newMethod);
void OEMethodCopy(SEL oldMethod, SEL newMethod)
{
    Class class = [NSViewController class];
    Method origMethod = class_getInstanceMethod(class, oldMethod);
    IMP implementaion = method_getImplementation(origMethod);
    const char *types = method_getTypeEncoding(origMethod);
    class_addMethod(class, newMethod, implementaion, types);
}

@implementation NSViewController (OEAdditions)

+ (void)load
{
    int major, minor;

    if(GetSystemVersion(&major, &minor, NULL) && major == 10 && minor < 10)
    {
        OEMethodCopy(@selector(OE_viewWillAppear),    @selector(viewWillAppear));
        OEMethodCopy(@selector(OE_viewDidAppear),     @selector(viewDidAppear));
        OEMethodCopy(@selector(OE_viewWillDisappear), @selector(viewWillDisappear));
        OEMethodCopy(@selector(OE_viewDidDisappear),  @selector(viewDidDisappear));
    }
}


- (void)OE_viewWillAppear;
{
}

- (void)OE_viewDidAppear;
{
    if([[self view] nextResponder] != self)
    {
        [self setNextResponder:[[self view] nextResponder]];
        [[self view] setNextResponder:self];
    }
}

- (void)OE_viewWillDisappear;
{
}

- (void)OE_viewDidDisappear;
{
}

@end
