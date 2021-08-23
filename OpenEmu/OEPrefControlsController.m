/*
 Copyright (c) 2011, OpenEmu Team

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

#import "OEPrefControlsController.h"
#import <OpenEmuSystem/OEHIDEvent.h>

static Boolean _OEHIDEventIsEqualSetCallback(OEHIDEvent *value1, OEHIDEvent *value2)
{
    return [value1 isUsageEqualToEvent:value2];
}

static CFHashCode _OEHIDEventHashSetCallback(OEHIDEvent *value)
{
    return [value controlIdentifier];
}

@implementation OEPrefControlsControllerObjC

- (void)awakeFromNib
{
    [super awakeFromNib];

    // We're using CFSet here because NSSet is confused by the changing state of OEHIDEvents
    CFSetCallBacks callbacks = kCFTypeSetCallBacks;
    callbacks.equal = (CFSetEqualCallBack)_OEHIDEventIsEqualSetCallback;
    callbacks.hash  = (CFSetHashCallBack)_OEHIDEventHashSetCallback;

    _ignoredEvents = (__bridge_transfer NSMutableSet *)CFSetCreateMutable(NULL, 0, &callbacks);
}

@end
