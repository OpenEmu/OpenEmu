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
#import <objc/runtime.h>

static IMP _OEReplaceMethodInClass(Class cls, SEL sel, IMP replace)
{
    Method m = class_getInstanceMethod(cls, sel);
    IMP ret = method_getImplementation(m);
    
    class_replaceMethod(cls, sel, (IMP)replace, method_getTypeEncoding(m));
    
    return ret;
}

@implementation NSViewController (OEAdditions)

static NSView *(*_old_NSViewController_view)(NSViewController *self, SEL _cmd);
static NSView *OE_NSViewController_view(NSViewController *self, SEL _cmd);

static NSView *_OENSViewControllerView(NSViewController *self)
{
    return object_getIvar(self, class_getInstanceVariable([NSView class], "view"));
}

+ (void)load
{
    _old_NSViewController_view = (NSView *(*)(NSViewController *, SEL))_OEReplaceMethodInClass(self, @selector(view), (IMP)OE_NSViewController_view);
}

- (void)viewWillLoad;
{
}

- (void)viewDidLoad;
{
}

- (void)viewWillAppear;
{
}

- (void)viewDidAppear;
{
    if([[self view] nextResponder] != self)
    {
        [self setNextResponder:[[self view] nextResponder]];
        [[self view] setNextResponder:self];
    }
}

- (void)viewWillDisappear;
{
}

- (void)viewDidDisappear;
{
}

static NSView *OE_NSViewController_view(NSViewController *self, SEL _cmd)
{
    NSView *ret = _OENSViewControllerView(self);
    
    BOOL willLoad = ret == nil;
    
    if(willLoad) [self viewWillLoad];
    
    ret = _old_NSViewController_view(self, _cmd);
    
    if(willLoad) [self viewDidLoad];
    
    return ret;
}

@end
